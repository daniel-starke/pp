# @file setup.nsi
# @author Daniel Starke
# @copyright Copyright 2015-2017 Daniel Starke
# @date 2015-12-02
# @version 1.0.0 (2015-12-04)

!ifndef VERSION
!define VERSION "unknown"
!endif
!define APPNAME "Parallel Processor"
!define NAME "Parallel Processor ${VERSION}"
!define SETUP "pp-${VERSION}-setup"
!define REGISTRY_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
Var /GLOBAL SIZE
Var /GLOBAL OLDINSTALLDIR

Name "${NAME}"
OutFile "..\bin\${SETUP}.exe"
SetCompressor /SOLID lzma

RequestExecutionLevel admin
InstallDir "$PROGRAMFILES64\${APPNAME}"
LicenseData "..\doc\COPYING"

# Get previous install location as default
InstallDirRegKey HKLM "${REGISTRY_PATH}" "InstallLocation"

!include LogicLib.nsh
!include WinMessages.nsh

Page license "" "ChangeFont" ""
Page directory
Page instfiles

Function "ChangeFont"
	FindWindow $0 "#32770" "" $HWNDPARENT
	GetDlgItem $0 $0 1000
	CreateFont $1 "Liberation Mono" 7 0
	SendMessage $0 ${WM_SETFONT} $1 0
FunctionEnd

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        MessageBox MB_ICONSTOP "Administrator rights required!"
        SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        Quit
${EndIf}
!macroend

!macro VerifyPreviousVersion
ReadRegStr $0 HKLM "${REGISTRY_PATH}" "QuietUninstallString"
ReadRegStr $OLDINSTALLDIR HKLM "${REGISTRY_PATH}" "InstallLocation"
${If} $OLDINSTALLDIR == ""
	StrCpy $OLDINSTALLDIR "$INSTDIR"
${EndIf}
${If} $0 != ""
	ExecWait "$0 _?=$INSTDIR" $0
	${If} $0 != 0
		MessageBox MB_ICONSTOP "Failed to uninstall previous version!"
		SetErrorLevel 1603 ;ERROR_INSTALL_FAILURE
		Quit
	${EndIf}
	${If} $OLDINSTALLDIR != "$INSTDIR"
		# Uninstall in different location
		Delete "$OLDINSTALLDIR\bin\uninstall.exe"
		RMDir $OLDINSTALLDIR\bin
		RMDir $OLDINSTALLDIR
	${EndIf}
${EndIf}
!macroend


Function .onInit
	SetShellVarContext all
	!insertmacro VerifyUserIsAdmin
FunctionEnd

Section "install"
	# Uninstall previous version first
	!insertmacro VerifyPreviousVersion
	
	# Files for the install directory
	SetOutPath '$INSTDIR\bin'
	File '..\bin\pp.exe'
	SetOutPath '$INSTDIR\doc'
	File '..\doc\Changelog.txt'
	File '..\doc\pp_notepad++_syntax.xml'
	File '..\doc\pp-user-manual.pdf'
	SetOutPath '$INSTDIR\license'
	File '..\doc\COPYING'
	
	# Uninstaller
	WriteUninstaller "$INSTDIR\bin\uninstall.exe"
	
	# Start Menu
	CreateDirectory "$SMPROGRAMS\${APPNAME}"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\User Manual.lnk" "$INSTDIR\doc\pp-user-manual.pdf"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\License.lnk" "$WINDIR\notepad.exe" "$\"$INSTDIR\license\COPYING$\"" "$WINDIR\notepad.exe" 0
	CreateShortCut "$SMPROGRAMS\${APPNAME}\Uninstall.lnk" "$INSTDIR\bin\uninstall.exe"
	
	# Registry information for add/remove programs
	WriteRegStr HKLM "${REGISTRY_PATH}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "${REGISTRY_PATH}" "UninstallString" "$\"$INSTDIR\bin\uninstall.exe$\""
	WriteRegStr HKLM "${REGISTRY_PATH}" "QuietUninstallString" "$\"$INSTDIR\bin\uninstall.exe$\" /S"
	WriteRegStr HKLM "${REGISTRY_PATH}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "${REGISTRY_PATH}" "Publisher" "Daniel Starke"
	WriteRegStr HKLM "${REGISTRY_PATH}" "DisplayVersion" "${VERSION}"
	# There is no option for modifying or repairing the install
	WriteRegDWORD HKLM "${REGISTRY_PATH}" "NoModify" 1
	WriteRegDWORD HKLM "${REGISTRY_PATH}" "NoRepair" 1
	# Set the INSTALLSIZE constant
	SectionGetSize "install" $SIZE
	IntOp $SIZE $SIZE + 1023
	IntOp $SIZE $SIZE >> 10
	WriteRegDWORD HKLM "${REGISTRY_PATH}" "EstimatedSize" $SIZE
SectionEnd


Function un.onInit
	SetShellVarContext all
	!insertmacro VerifyUserIsAdmin
FunctionEnd
 
Section "uninstall"
	# Remove Start Menu
	RMDir /r "$SMPROGRAMS\${APPNAME}"
 
	# Get old installation directory
	ReadRegStr $OLDINSTALLDIR HKLM "${REGISTRY_PATH}" "InstallLocation"
	${If} $OLDINSTALLDIR == ""
		StrCpy $OLDINSTALLDIR "$INSTDIR"
	${EndIf}
 
	# Remove files
	Delete "$OLDINSTALLDIR\bin\pp.exe"
	RMDir "$OLDINSTALLDIR\bin"
	Delete "$OLDINSTALLDIR\doc\Changelog.txt"
	Delete "$OLDINSTALLDIR\doc\pp_notepad++_syntax.xml"
	Delete "$OLDINSTALLDIR\doc\pp-user-manual.pdf"
	RMDir "$OLDINSTALLDIR\doc"
	Delete "$OLDINSTALLDIR\license\COPYING"
	RMDir "$OLDINSTALLDIR\license"
 
	# Always delete uninstaller as the last action
	Delete $OLDINSTALLDIR\bin\uninstall.exe
 
	# Try to remove the install directory - this will only happen if it is empty
	RMDir $OLDINSTALLDIR\bin
	RMDir $OLDINSTALLDIR
 
	# Remove uninstaller information from the registry
	DeleteRegKey HKLM "${REGISTRY_PATH}"
SectionEnd
