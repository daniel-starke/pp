/**
 * @file fdio.i
 * @author Daniel Starke
 * @copyright Copyright 2015-2016 Daniel Starke
 * @see fdios.h
 * @see fdious.h
 * @date 2015-08-23
 * @internal This file is never used or compiled directly but only included.
 * @remarks Define CHAR_T to the character type before including this file.
 * @remarks See FPOPEN_FUNC() and FPCLOSE_FUNC() for further notes.
 */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <libpcf/target.h>

#if PCF_IS_WIN
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else /* ! PCF_IS_WIN */
#include <unistd.h>
#ifdef FPOPEN_UNICODE
#include <wchar.h>
#endif
#include <sys/wait.h>
#endif /* PCF_IS_WIN */


#ifndef READ_PIPE
#define READ_PIPE 0
#endif
#ifndef WRITE_PIPE
#define WRITE_PIPE 1
#endif


/**
 * @fn tFdioPHandle * FPOPEN_FUNC(const CHAR_T ** shell, const CHAR_T * command, FILE * input, const char * mode)
 * Executes the passed program and opens its stdin or stdout for reading
 * or writing. The difference to popen is, that the command excepts a shell
 * which shall be used for execution.
 * The first argument of 'shell' is the program path and any other following except NULL are the
 * arguments that shall be passed to shell for executing the given command.
 *
 * @param[in] shellPath - path the shell binary
 * @param[in] shell - execute the given command with this shell (null terminated array)
 * @param[in] command - command to execute (optional)
 * @param[in] input - use this for the standard input of the new process (optional)
 * @param[in] mode - open mode, see tFdioPMode
 * @return NULL on error or the handle to the child process
 * @remarks Define FPOPEN_FUNC for name of the function.
 * @remarks Define FPOPEN_UNICODE for Unicode Windows build.
 * @remarks Windows implementation is based on https://msdn.microsoft.com/en-us/library/17w5ykft(v=vs.85).aspx
 * except if FDIO_RAW_CMDLINE is passed, which forces the function to pass the arguments separated by space.
 */


/**
 * @fn int FPCLOSE_FUNC(tFdioPHandle * fd)
 * Closes the read/write file descriptor of the passed handle and waits until the child
 * process terminates. Returns the exit code of the child process.
 *
 * @param[in,out] fd - tFdioPHandle pointer to the FPOPEN handle
 * @return -1 on error or the exit code of the child process
 * @remarks Define FPCLOSE_FUNC for name of the function.
 */


#ifdef FPOPEN_UNICODE
#define CHAR_T wchar_t
#ifndef TCHAR_STLEN
#define TCHAR_STLEN wcslen
#endif /* TCHAR_STLEN */
#else /* ! FPOPEN_UNICODE */
#define CHAR_T char
#ifndef TCHAR_STLEN
#define TCHAR_STLEN strlen
#endif /* TCHAR_STLEN */
#endif /* FPOPEN_UNICODE */


#ifdef PCF_IS_WIN
tFdioPHandle * FPOPEN_FUNC(const CHAR_T * shellPath, const CHAR_T ** shell, const CHAR_T * command, FILE * input, const tFdioPMode mode) {
	HANDLE pStandardInput[2];
	HANDLE pStandardOutput[2];
	HANDLE pStandardError[2];
	int hasPStdIn, hasPStdOut, hasPStdErr;
	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION pi;
#ifdef FPOPEN_UNICODE
	STARTUPINFOW si;
#else /* ! FPOPEN_UNICODE */
	STARTUPINFOA si;
#endif /* FPOPEN_UNICODE */
	BOOL bSuccess = FALSE;
	tFdioPHandle * fd;
	const CHAR_T * cPtr;
	CHAR_T * cmdLine, * clPtr;
	size_t numArgs;
	const CHAR_T * arg, ** args;
	int * hasSpaces;
	size_t cmdLineLen, escCount;
	int i, flags;

	if (shellPath == NULL || shell == NULL || *shell == NULL) return NULL;
	/* cannot access the standard input of the sub process and use a different file descriptor
	   as standard input for it */
	if ((((int)mode) & FDIO_USE_STDIN) != 0 && input != NULL) return NULL;
	hasPStdIn = 0;
	hasPStdOut = 0;
	hasPStdErr = 0;
	fd = NULL;
	cmdLine = NULL;
	hasSpaces = NULL;
	
	if ((((int)mode) & FDIO_RAW_CMDLINE) != 0) {
		/* check the needed size for the command-line string */
		cmdLineLen = 1; /* null-termination */
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			cmdLineLen += TCHAR_STLEN(arg);
			if (arg == command) break;
			cmdLineLen++; /* argument separating space */
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		/* create the command-line */
		cmdLine = (CHAR_T *)malloc(sizeof(CHAR_T) * cmdLineLen);
		if (cmdLine == NULL) {
			goto onerror;
		}
		clPtr = cmdLine;
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			cmdLineLen = TCHAR_STLEN(arg);
			memcpy(clPtr, arg, cmdLineLen * sizeof(CHAR_T));
			clPtr += cmdLineLen;
			if (arg == command) break;
			/* spaces between two arguments */
			*clPtr = ' ';
			clPtr++;
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		*clPtr = 0;
	} else {
		/* calculate passed number of arguments */
		numArgs = 1;
		args = shell;
		for (arg = *args; arg != NULL; args++, arg = *args, numArgs++);
		/* create array to save whether a arguments needs quoting or not */
		hasSpaces = (int *)malloc(sizeof(int) * numArgs);
		if (hasSpaces == NULL) goto onerror;
		/* check the needed size for the command-line string */
		/* see https://msdn.microsoft.com/en-us/library/17w5ykft(v=vs.85).aspx */
		cmdLineLen = 3; /* first quote, last quote and null-termination */
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			hasSpaces[i] = 0;
			escCount = 0;
			for (cPtr = arg; *cPtr != 0; cPtr++) {
				switch (*cPtr) {
				case '\\':
					escCount++;
					break;
				case '"':
					if (escCount > 0) {
						cmdLineLen = cmdLineLen + escCount + 1;
						escCount = 0;
					} else {
						cmdLineLen++;
					}
				default:
					if (*cPtr == ' ' || *cPtr == '\t') hasSpaces[i] = 1;
					escCount = 0;
					break;
				}
				cmdLineLen++;
			}
			cmdLineLen += escCount; /* handle trailing escape characters */
			if (hasSpaces[i] != 0) cmdLineLen += 2;
			if (arg == command) break;
			cmdLineLen++; /* argument separating space */
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		/* create the command-line */
		cmdLine = (CHAR_T *)malloc(sizeof(CHAR_T) * cmdLineLen);
		if (cmdLine == NULL) {
			goto onerror;
		}
		clPtr = cmdLine;
		args = shell;
		arg = *args;
		for (i = 0; ; i++) {
			if (hasSpaces[i] != 0) {
				/* quote of the beginning of an argument */
				*clPtr = '"';
				clPtr++;
			}
			escCount = 0;
			for (cPtr = arg; *cPtr != 0; cPtr++) {
				switch (*cPtr) {
				case '\\':
					escCount++;
					break;
				case '"':
					if (escCount > 0) {
						for (; escCount > 0; escCount--) {
							*clPtr = '\\';
							clPtr++;
						}
						escCount = 0;
					}
					*clPtr = '\\';
					clPtr++;
				default:
					escCount = 0;
					break;
				}
				*clPtr = *cPtr;
				clPtr++;
			}
			for (; escCount > 0; escCount--) {
				/* handle trailing escape characters */
				*clPtr = '\\';
				clPtr++;
			}
			if (hasSpaces[i] != 0) {
				/* quote of the end of an argument */
				*clPtr = '"';
				clPtr++;
			}
			if (arg == command) break;
			/* spaces between two arguments */
			*clPtr = ' ';
			clPtr++;
			args++;
			arg = *args;
			if (arg == NULL) {
				if (command != NULL) {
					arg = command;
				} else {
					break;
				}
			}
		}
		*clPtr = 0;
	}
	
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		if ( ! CreatePipe(&(pStandardInput[READ_PIPE]), &(pStandardInput[WRITE_PIPE]), &sa, 0 /* default buffer size */) ) {
			goto onerror;
		}
		hasPStdIn = 1;
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		if ( ! CreatePipe(&(pStandardOutput[READ_PIPE]), &(pStandardOutput[WRITE_PIPE]), &sa, 0 /* default buffer size */) ) {
			goto onerror;
		}
		hasPStdOut = 1;
	}
	
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		if ( ! CreatePipe(&(pStandardError[READ_PIPE]), &(pStandardError[WRITE_PIPE]), &sa, 0 /* default buffer size */) ) {
			goto onerror;
		}
		hasPStdErr = 1;
	}
	
	if ((fd = (tFdioPHandle *)malloc(sizeof(tFdioPHandle))) == NULL) {
		goto onerror;
    }
	
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		if ( ! SetHandleInformation(pStandardInput[WRITE_PIPE], HANDLE_FLAG_INHERIT, 0) ) {
			goto onerror;
		}
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		if ( ! SetHandleInformation(pStandardOutput[READ_PIPE], HANDLE_FLAG_INHERIT, 0) ) {
			goto onerror;
		}
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		if ( ! SetHandleInformation(pStandardError[READ_PIPE], HANDLE_FLAG_INHERIT, 0) ) {
			goto onerror;
		}
	}
	
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO); 
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		si.hStdInput = pStandardInput[READ_PIPE];
	} else if (input != NULL) {
		si.hStdInput = (HANDLE)_get_osfhandle(_fileno(input));
	} else {
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		si.hStdOutput = pStandardOutput[WRITE_PIPE];
	} else {
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0) {
		if ((((int)mode) & FDIO_COMBINE) == 0) {
			si.hStdError = pStandardError[WRITE_PIPE];
		} else {
			si.hStdError = si.hStdOutput;
		}
	} else {
		if ((((int)mode) & FDIO_COMBINE) == 0) {
			si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		} else {
			si.hStdError = si.hStdOutput;
		}
	}
	si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	
#ifdef FPOPEN_UNICODE
	bSuccess = CreateProcessW(shellPath, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
#else /* ! FPOPEN_UNICODE */
	bSuccess = CreateProcessA(shellPath, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
#endif /* FPOPEN_UNICODE */
	
	free(hasSpaces);
	hasSpaces = NULL;
	free(cmdLine);
	cmdLine = NULL;
	if ( ! bSuccess ) {
		goto onerror;
	}
	/* close pipe handles after child process inherited them */
	/* see https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365782(v=vs.85).aspx */
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		CloseHandle(si.hStdInput);
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		CloseHandle(si.hStdOutput);
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		CloseHandle(si.hStdError);
	}
	
	CloseHandle(pi.hThread);
	flags = 0;
	if ((((int)mode) & FDIO_BINARY_PIPE) == 0) flags |= _O_TEXT;
	fd->pid = pi.hProcess;
	if ((((int)mode) & FDIO_USE_STDIN) != 0) {
		fd->in = _fdopen(
			_open_osfhandle((intptr_t)pStandardInput[WRITE_PIPE], flags),
			(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "wb" : "w"
		);
	} else {
		fd->in = NULL;
	}
	if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
		fd->out = _fdopen(
			_open_osfhandle((intptr_t)pStandardOutput[READ_PIPE], flags | _O_RDONLY),
			(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
		);
	} else {
		fd->out = NULL;
	}
	if ((((int)mode) & FDIO_USE_STDERR) != 0 && (((int)mode) & FDIO_COMBINE) == 0) {
		fd->err = _fdopen(
			_open_osfhandle((intptr_t)pStandardError[READ_PIPE], flags | _O_RDONLY),
			(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
		);
	} else {
		fd->err = NULL;
	}
	
	return fd;
onerror:
	if (hasPStdIn != 0) {
		CloseHandle(pStandardInput[READ_PIPE]);
		CloseHandle(pStandardInput[WRITE_PIPE]);
	}
	if (hasPStdOut != 0) {
		CloseHandle(pStandardOutput[READ_PIPE]);
		CloseHandle(pStandardOutput[WRITE_PIPE]);
	}
	if (hasPStdErr != 0) {
		CloseHandle(pStandardError[READ_PIPE]);
		CloseHandle(pStandardError[WRITE_PIPE]);
	}
	if (hasSpaces != NULL) free(hasSpaces);
	if (fd != NULL) free(fd);
	if (cmdLine != NULL) free(cmdLine);
	return NULL;
}


int FPCLOSE_FUNC(tFdioPHandle * fd) {
	HANDLE pid;
	int exitCode;
	DWORD dwExitCode;
	if (fd == NULL) return -1;
	pid = fd->pid;
	
	if (fd->in != NULL) fclose(fd->in);
	
	exitCode = -1;
	if (WaitForSingleObject(pid, INFINITE) != WAIT_FAILED) {
		if ( GetExitCodeProcess(pid, &dwExitCode) ) {
			exitCode = (int)dwExitCode;
		}
	}
	
	/* standard outputs need to be closed last or the child process
	 * may fail with an error code while trying to write to them */
	if (fd->out != NULL) fclose(fd->out);
	if (fd->err != NULL) fclose(fd->err);
	free(fd);
	
	CloseHandle(pid);
	return exitCode;
}
#else /* ! PCF_IS_WIN */
tFdioPHandle * FPOPEN_FUNC(const CHAR_T * shellPath, const char ** shell, const char * command, FILE * input, const tFdioPMode mode) {
	int pStandardInput[2];
	int pStandardOutput[2];
	int pStandardError[2];
	int hasPStdIn, hasPStdOut, hasPStdErr;
	tFdioPHandle * fd;
	size_t numArgs, strLen, totalStrLen;
	const CHAR_T * arg, ** args;
	CHAR_T * out;
	CHAR_T ** argv;
	int i;

	if (shellPath == NULL || shell == NULL || *shell == NULL) return NULL;
	/* cannot access the standard input of the sub process and use a different file descriptor
	   as standard input for it */
	if ((((int)mode) & FDIO_USE_STDIN) != 0 && input != NULL) return NULL;
	
	hasPStdIn = 0;
	hasPStdOut = 0;
	hasPStdErr = 0;
	fd = NULL;
	argv = NULL;
	
	/* calculate passed number of arguments and final memory size */
	numArgs = (command == NULL) ? 1 : 2;
	args = shell;
	totalStrLen = 0;
	for (arg = *args; arg != NULL; args++, arg = *args, numArgs++) {
		totalStrLen += TCHAR_STLEN(*args) + 1;
	}
	
	/* build new command array */
	argv = (CHAR_T **)malloc((sizeof(CHAR_T *) * numArgs) + (sizeof(CHAR_T) * totalStrLen));
	if (argv == NULL) goto onerror;
	out = (CHAR_T *)(&(argv[numArgs]));
	args = shell;
	for (i = 0; *args != NULL; args++, i++) {
		strLen = TCHAR_STLEN(*args) + 1;
		memcpy(out, *args, sizeof(CHAR_T) * strLen);
		argv[i] = out;
		out += strLen;
	}
	if (command != NULL) {
		strLen = TCHAR_STLEN(command) + 1;
		memcpy(out, command, sizeof(CHAR_T) * strLen);
		argv[i] = out;
		i++;
	}
	argv[i] = NULL;
	
	if (pipe(pStandardInput) != 0) goto onerror;
	hasPStdIn = 1;
	if (pipe(pStandardOutput) != 0) goto onerror;
	hasPStdOut = 1;
	if (pipe(pStandardError) != 0) goto onerror;
	hasPStdErr = 1;

    if ((fd = (tFdioPHandle *)malloc(sizeof(tFdioPHandle))) == NULL) goto onerror;
	
	fd->pid = fork();

	switch (fd->pid) {
	case -1: /* fork failed */
		goto onerror;
		break;
	case 0: /* child */
		if ((((int)mode) & FDIO_USE_STDIN) != 0) {
			dup2(pStandardInput[READ_PIPE], STDIN_FILENO);
		} else if (input != NULL) {
			dup2(fileno(input), STDIN_FILENO);
		}
		if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
			dup2(pStandardOutput[WRITE_PIPE], STDOUT_FILENO);
		}
		if ((((int)mode) & FDIO_USE_STDERR) != 0) {
			if ((((int)mode) & FDIO_COMBINE) == 0) {
				dup2(pStandardError[WRITE_PIPE], STDERR_FILENO);
			} else {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			}
		} else {
			if ((((int)mode) & FDIO_COMBINE) != 0) {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			}
		}
		
		/* close other pipe fds */
		if (hasPStdIn != 0) close(pStandardInput[WRITE_PIPE]);
		if (hasPStdOut != 0) close(pStandardOutput[READ_PIPE]);
		if (hasPStdErr != 0) close(pStandardError[READ_PIPE]);
		
		/* can change to any exec* function family */
		/* see why argv cannot be const char **: http://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html#tag_16_111 */
		execvp(shellPath, argv);
		exit(1); /* something went wrong */
		
		break;
	default: /* parent */
		/* close other pipe fds */
		if (hasPStdIn != 0) close(pStandardInput[READ_PIPE]);
		if (hasPStdOut != 0) close(pStandardOutput[WRITE_PIPE]);
		if (hasPStdErr != 0) close(pStandardError[WRITE_PIPE]);
		
		if ((((int)mode) & FDIO_USE_STDIN) != 0) {
			if ((fd->in = fdopen(
				pStandardInput[WRITE_PIPE],
				(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "wb" : "w"
			)) == NULL) {
				close(pStandardInput[WRITE_PIPE]);
			}
		} else {
			fd->in = NULL;
		}
		if ((((int)mode) & FDIO_USE_STDOUT) != 0) {
			if ((fd->out = fdopen(
				pStandardOutput[READ_PIPE],
				(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
			)) == NULL) {
				close(pStandardOutput[READ_PIPE]);
			}
		} else {
			fd->out = NULL;
		}
		if ((((int)mode) & FDIO_USE_STDERR) != 0) {
			if ((fd->err = fdopen(
				pStandardError[READ_PIPE],
				(((int)mode) & FDIO_BINARY_PIPE) != 0 ? "rb" : "r"
			)) == NULL) {
				close(pStandardError[READ_PIPE]);
			}
		} else {
			fd->err = NULL;
		}
		if (argv != NULL) free(argv);
		break;
	}
	
	return fd;
onerror:
	if (hasPStdIn != 0) {
		close(pStandardInput[READ_PIPE]);
		close(pStandardInput[WRITE_PIPE]);
	}
	if (hasPStdOut != 0) {
		close(pStandardOutput[READ_PIPE]);
		close(pStandardOutput[WRITE_PIPE]);
	}
	if (hasPStdErr != 0) {
		close(pStandardError[READ_PIPE]);
		close(pStandardError[WRITE_PIPE]);
	}
	if (fd != NULL) free(fd);
	if (argv != NULL) free(argv);
	return NULL;
}


int FPCLOSE_FUNC(tFdioPHandle * fd) {
	int status, result;
	if (fd == NULL) return -1;
	
	if (fd->in != NULL) fclose(fd->in);
	
	result = -1;
	waitpid(fd->pid, &status, 0);
	free(fd);
	
	if ( WIFEXITED(status) ) {
		result = WEXITSTATUS(status);
	}
	
	/* standard outputs need to be closed last or the child process
	 * may fail with an error code while trying to write to them */
	if (fd->out != NULL) fclose(fd->out);
	if (fd->err != NULL) fclose(fd->err);
	
	return result;
}
#endif /* PCF_IS_WIN */
