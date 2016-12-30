#!/bin/bash
# @file perform-tests.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-11-26
# @version 2016-12-30

[ "x${base}"  == "x" ] && base="."
[ "x${pp}"    == "x" ] && pp="${PWD}/${base}/bin/pp.exe"
[ "x${dir}"   == "x" ] && dir="${PWD}/${base}/test"
[ "x${clean}" == "x" ] && clean="1"

# to absolute path
pp="$(readlink -e ${pp})"
dir="$(readlink -e ${dir})"

if [ ! -d "${base}" ]; then
	echo "Error: Base directory '${base}' not found." >&2
	exit 1
fi
if [ ! -f "${pp}" ]; then
	echo "Error: Parallel Processor executable '${pp}' not found." >&2
	exit 1
fi
if [ ! -d "${dir}" -a ! -f "${dir}" ]; then
	echo "Error: Test directory/file '${dir}' not found." >&2
	exit 1
fi
case "${clean}" in
"0"|"1")
	;; # ok
*)
	echo "Error: Invalid value '${clean}' for clean." >&2
	exit 1
esac

wdir="${PWD}"

c_ok=$'\033[32m'
c_nok=$'\033[31m'
c_r=$'\033[0m'

# statistics
t_all=0
t_ok=0
t_nok=0

# params: <test-case>
sub_ok() {
	let "t_all++"
	let "t_ok++"
	echo "${c_ok}[PASSED]${c_r} ${1}"
}

# params: <test-case>
sub_nok() {
	let "t_all++"
	let "t_nok++"
	echo "${c_nok}[FAILED]${c_r} ${1}"
}

# params: <function-list>
sub_function() {
	[ "x${1}" == "x" ] && return 0
	local result=0
	for f in ${1}; do
		${f} || result=1
	done
	return ${result}
}

# params: <type-path-list-per-line>
sub_create_input() {
	[ "x${1}" == "x" ] && return 0
	local result=0
	while read type path; do
		[ "x${type}" == "x" ] && continue
		[ "x${path}" == "x" ] && continue
		case "${type}" in
		'f')
			if [ ! -d "$(dirname "${path}")" ]; then
				mkdir -p "$(dirname "${path}")" || result=1
			fi
			touch "${path}" || result=1
			;;
		'd')
			if [ ! -d "${path}" ]; then
				mkdir -p "${path}" || result=1
			fi
			touch "${path}" || result=1
			;;
		*)
			echo "Warning: Ignoring invalid path type '${type}' for '${path}'." >&2
			;;
		esac
	done <<_END
${1}
_END
	return ${result}
}

# params: <script-content>
sub_create_script() {
	[ "x${1}" == "x" ] && return 0
	echo "${1}" > "process.parallel"
	return $?
}

# params: <name> <file> <result> <checks>
sub_check_pattern() {
	local check="-eq"
	local part=" "
	[ "x${4}" = "x" ] && return 0
	if [ "${3}" -ne 0 ]; then
		check="-ne"
		part=" not "
	fi
	if [ ! -f "${2}" ]; then
		sub_nok "${1}${part}pattern: ${pattern}"
		return 0
	fi
	if [ "x${4}" != "x" ]; then
		while read pattern; do
			grep -q "${pattern}" < ${2}
			if [ $? ${check} 0 ]; then
				sub_ok "${1}${part}pattern: ${pattern}"
			else
				sub_nok "${1}${part}pattern: ${pattern}"
			fi
		done <<_END
${4}
_END
	fi
	return 0
}

# params: <test-name> <command-line> <runs> <exit-check> <stdout-checks> <stderr-checks> <stdout-nchecks> <stderr-nchecks>
sub_execute() {
	local runs="${3}"
	local run="1"
	local res=""
	local t_touch
	while [ "${runs}" -gt 0 ]; do
		${pp} ${2} "TEST_RUN=${run}" >${1}.stdout 2>${1}.stderr
		res=$?
		eval "t_touch=\"\${t_touch${run}}\""
		if [ "x${t_touch}" != "x" ]; then
			[ "${runs}" -gt 1 ] && sleep 1 # wait a second to ensure the files get a new time stamp
			sub_create_input "${t_touch}"
		fi
		let "runs--"
		let "run++"
	done
	if [ "x${4}" != "x" ]; then
		if [ ${res} -eq ${4} ]; then
			sub_ok "exit code - ${pp} ${2}"
		else
			sub_nok "exit code - ${pp} ${2}"
		fi
	fi
	sub_check_pattern "stdout" ${1}.stdout 0 "${5}"
	sub_check_pattern "stderr" ${1}.stderr 0 "${6}"
	sub_check_pattern "stdout" ${1}.stdout 1 "${7}"
	sub_check_pattern "stderr" ${1}.stderr 1 "${8}"
	return 0
}

# params: <result> <type-path-list-per-line>
sub_check_output() {
	[ "x${2}" == "x" ] && return 0
	local check=" "
	[ ${1} -eq 1 ] && check=" not "
	while read type path; do
		[ "x${type}" == "x" ] && continue
		[ "x${path}" == "x" ] && continue
		local result=0
		case "${type}" in
		'f')
			[ ! -f "${path}" ] && result=1
			;;
		'd')
			[ ! -d "${path}" ] && result=1
			;;
		*)
			echo "Warning: Ignoring invalid path type '${type}' for '${path}'." >&2
			;;
		esac
		if [ ${result} -eq ${1} ]; then
			sub_ok "output path does${check}exist: ${path}"
		else
			sub_nok "output path does${check}exist: ${path}"
		fi
	done <<_END
${2}
_END
	return 0
}

# params:
sub_clean() {
	if [ "${clean}" -eq 1 ]; then
		rm -rf */
		find . -type f ! -name "*.sh" -exec rm -f "{}" ";"
	fi
	return 0
}

# params: <exit-code> <info>
ok_or_fail() {
	if [ ${1} -ne 0 ]; then
		shift 1
		echo "Error: Failed at $@"
		exit 1
	fi
	return 0
}

# params: <var-name>
to_var() {
	local cmd="s|\\([\\\\\\\"\$]\\)|\\\\\1|g"
	eval "${1}=\"`sed \"${cmd}\"`\""
}

# main loop
while read file; do
	t_before=""  # execute these functions first (optional)
	t_after=""   # execute these functions last (optional)
	t_create=""  # create these directories and files (optional)
	# touch these paths after the given run (optional)
	unset t_touch1 t_touch2 t_touch3 t_touch4 t_touch5 t_touch6 t_touch7 t_touch8 t_touch9
	t_script=""  # create this script
	t_runs="1"   # execute the script this often, environment variable TEST_RUN is set accordingly (optional)
	t_cmdline="" # pass these parameters on command-line when executing (optional)
	t_text=""    # check if pp standard output contained these strings (separated by line) (optional)
	t_ntext=""   # check if pp standard output did not contain these strings (separated by line) (optional)
	t_error=""   # check if pp error output contained these strings (separated by line) (optional)
	t_nerror=""  # check if pp error output did not contain these strings (separated by line) (optional)
	t_log=""     # check if pp log file contained these strings (separated by line) (optional)
	t_nlog=""    # check if pp log file did not contain these strings (separated by line) (optional)
	t_check=""   # check if these files are present after execution (optional)
	t_ncheck=""  # check if these files are not present after execution (optional)
	t_exit="0"   # check this exit code (optional)
	cd "${wdir}"
		ok_or_fail $? line $LINENO
	t_name="$(basename \"${file}\")"
	t_name="${t_name%.*}"
	echo "Test: ${file#${dir}/}"
	source "${file}"
		ok_or_fail $? line $LINENO
	cd "$(dirname "${file}")"
		ok_or_fail $? line $LINENO
	sub_clean
		ok_or_fail $? line $LINENO
	sub_function "${t_before}"
		ok_or_fail $? line $LINENO
	sub_create_input "${t_create}"
		ok_or_fail $? line $LINENO
	sub_create_script "${t_script}"
		ok_or_fail $? line $LINENO
	sub_execute "${t_name}" "${t_cmdline}" "${t_runs}" "${t_exit}" "${t_text}" "${t_error}" "${t_ntext}" "${t_nerror}"
		ok_or_fail $? line $LINENO
	sub_check_pattern "log file" "log.txt" 0 "${t_log}"
		ok_or_fail $? line $LINENO
	sub_check_pattern "log file" "log.txt" 1 "${t_nlog}"
		ok_or_fail $? line $LINENO
	sub_check_output 0 "${t_check}"
		ok_or_fail $? line $LINENO
	sub_check_output 1 "${t_ncheck}"
		ok_or_fail $? line $LINENO
	sub_function "${t_after}"
		ok_or_fail $? line $LINENO
	sub_clean
		ok_or_fail $? line $LINENO
done <<_END
$(find "${dir}" -type f -name "*.sh")
_END

# output statistics
cat <<_END

TOTAL:  ${t_all}
${c_ok}PASSED: ${t_ok}${c_r}
${c_nok}FAILED: ${t_nok}${c_r}
_END

[ "${t_nok}" -gt 0 ] && exit 1
exit 0