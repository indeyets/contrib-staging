# installer generator script for Freenet:

Name "Freenet 0.3.9.3"
OutFile "Freenet_setup0.3.9.test.exe"
ComponentText "This will install Freenet 0.3.9.3 on your system."

LicenseText "Freenet is published under the GNU general public license:"
LicenseData GNU.txt

UninstallText "This uninstalls Freenet and all files on this node. (You may need to shut down running nodes before proceeding)"
UninstallExeName Uninstall-Freenet.exe

DirText "No files will be placed outside this directory (e.g. Windows\system)"

 InstType Minimal
 InstType Full

EnabledBitmap Yes.bmp
DisabledBitmap No.bmp
BGGradient
;AutoCloseWindow true
;!packhdr will further optimize your installer package if you have upx.exe in your directory
!packhdr temp.dat "upx.exe -9 temp.dat"

InstallDir "$PROGRAMFILES\Freenet"
InstallDirRegKey HKEY_LOCAL_MACHINE "Software\Freenet" "instpath"

;-----------------------------------------------------------------------------------
Function DetectJava
# this function detects Sun Java from registry and calls the JavaFind utility otherwise

  # First look for the current version of Java and get the correct path in $2,
  # then test if its empty (nonexisting)
StartCheck:
  StrCpy $0 "SOFTWARE\JavaSoft\Java Runtime Environment\1.3"	; JRE key into $0
  ReadRegStr $2 HKLM $0 "JavaHome"				; read JRE path in $2
  StrCmp $2 "" 0 EndCheck

  # Check for 1.2
  StrCpy $0 "SOFTWARE\JavaSoft\Java Runtime Environment\1.2"	; JRE key into $0
  ReadRegStr $2 HKLM $0 "JavaHome"				; read JRE path in $2
  StrCmp $2 "" RunJavaFind

EndCheck:
  StrCpy $3 "$2\bin\java.exe"
  StrCpy $4 "$2\bin\javaw.exe"

  IfFileExists $3 0 RunJavaFind
  WriteINIStr "$INSTDIR\FLaunch.ini" "Freenet Launcher" "JavaExec" $3
  IfFileExists $4 0 RunJavaFind
  WriteINIStr "$INSTDIR\FLaunch.ini" "Freenet Launcher" "Javaw" $4

  #jump to the end if we did the Java recognition correctly
  Goto End

RunJavaFind:
  # running the good ol' Java detection utility on unsuccess
  MessageBox MB_YESNO "I did not find Sun's Java Runtime Environment which is needed for Freenet.$\r$\nHit 'Yes' to open the download page for Java (http://java.sun.com),$\r$\n'No' to look for an alternative Java interpreter on your disks." IDYES GetJava
  Execwait "$INSTDIR\findjava.exe"
  Goto End

GetJava:
  # Open the download page for Sun's Java
  ExecShell "open" "http://java.sun.com/j2se/1.3/jre/download-windows.html#software"
  Sleep 5000
  MessageBox MB_OKCANCEL "Press OK to continue the Freenet installation AFTER having installed Java,$\r$\nCANCEL to abort the installation." IDOK StartCheck
  Abort

End:
FunctionEnd
;-----------------------------------------------------------------------------------
Section "Freenet (required)"

  # Copying the actual Freenet files to the install dir
  SetOutPath "$INSTDIR\ref"
  File "Freenet\ref\*.ref"

  SetOutPath "$INSTDIR"
  File freenet\*.*
  CopyFiles "$INSTDIR\fserve.exe" "$INSTDIR\frequest.exe" 6
  CopyFiles "$INSTDIR\fserve.exe" "$INSTDIR\finsert.exe" 6
  CopyFiles "$INSTDIR\fserve.exe" "$INSTDIR\FProxy.exe" 6
  CopyFiles "$INSTDIR\fserve.exe" "$INSTDIR\cfgnode.exe" 6

  # possibly embed full Java runtime
  !ifdef embedJava
  File /r jre
  !endif

  HideWindow
  Call DetectJava
  Delete "$INSTDIR\findjava.exe"

  ClearErrors
  ExecWait "$INSTDIR\portcfg.exe"
  IfErrors ConfigError
  Delete "$INSTDIR\portcfg.exe"
  BringToFront
  ExecWait '"$INSTDIR\cfgnode.exe" silent'
  IfErrors CfgnodeError ; this does not work yet. It never returns an error message?!
  Delete "$INSTDIR\cfgnode.exe"
  ExecWait "$INSTDIR\cfgclient.exe"
  IfErrors ConfigError ; this does not work yet. It never returns an error message?!
  Delete "$INSTDIR\cfgclient.exe"
  Goto End

 CfgnodeError:
  BringToFront
  MessageBox MB_OK "There was an error while creating the Freenet configuration file. Do you really have Java already installed? Aborting installation now!"
  Abort
 ConfigError:
  BringToFront
  MessageBox MB_OK "There was an error while configuring Freenet.$\r$\nDo you really have Java already installed?$\r$\nAborting installation!"
  Abort
 End:
SectionEnd
;--------------------------------------------------------------------------------------
 
Section "Startmenu and Desktop Icons"
SectionIn 1,2

   CreateShortCut "$DESKTOP\Freenet.lnk" "$INSTDIR\freenet.exe" "" "$INSTDIR\freenet.exe" 0
   CreateDirectory "$SMPROGRAMS\Freenet"
   CreateShortCut "$SMPROGRAMS\Freenet\Freenet.lnk" "$INSTDIR\freenet.exe" "" "$INSTDIR\freenet.exe" 0
   WriteINIStr "$SMPROGRAMS\Freenet\FN Homepage.url" "InternetShortcut" "URL" "http://www.freenetproject.org"  
   WriteINIStr "$SMPROGRAMS\Freenet\FNGuide.url" "InternetShortcut" "URL" "http://www.freenetproject.org/quickguide" 
   ;CreateShortcut "$SMPROGRAMS\Freenet\FNGuide.url" "" "" "$SYSDIR\url.dll" 0
   CreateShortCut "$SMPROGRAMS\Freenet\Uninstall.lnk" "$INSTDIR\Uninstall-Freenet.exe" "" "$INSTDIR\Uninstall-Freenet.exe" 0
 SectionEnd
;-------------------------------------------------------------------------------
 SectionDivider
;-------------------------------------------------------------------------------
#Plugin for Internet-Explorer
#Section "IE browser plugin"
#SectionIn 2
#SetOutPath $INSTDIR
#File freenet\IEplugin\*.*
#WriteRegStr HKEY_CLASSES_ROOT PROTOCOLS\Handler\freenet CLSID {CDDCA3BE-697E-4BEB-BCE4-5650C1580BCE}
#WriteRegStr HKEY_CLASSES_ROOT PROTOCOLS\Handler\freenet '' 'freenet: Asychronous Pluggable Protocol Handler'
#WriteRegStr HKEY_CLASSES_ROOT freenet '' 'URL:freenet protocol'
#WriteRegStr HKEY_CLASSES_ROOT freenet 'URL Protocol' ''
#RegDLL $INSTDIR\IEFreenetPlugin.dll
#SectionEnd

#Section "Mozilla plugin"
#SectionIn 2
#SetOutPath $TEMP
# The next files are not yet deleted anywhere, need to do this somewhere!
#File freenet\NSplugin\launch.exe
#File freenet\NSplugin\mozinst.html
#File freenet\NSplugin\protozilla-0.3-other.xpi
#Exec '"$TEMP\launch.exe" Mozilla "$TEMP\mozinst.html"'
##need to delete the tempfiles again
#SectionEnd
;---------------------------------------------------------------------------------------
Section "Launch Freenet on Startup"
SectionIn 2
  # WriteRegStr HKEY_CURRENT_USER "Software\Microsoft\Windows\CurrentVersion\Run" "Freenet server" '"$INSTDIR\fserve.exe"'
  CreateShortCut "$SMSTARTUP\Freenet.lnk" "$INSTDIR\freenet.exe" "" "$INSTDIR\freenet.exe" 0
SectionEnd

;---------------------------------------------------------------------------------------

Section "Launch Freenet node now"
SectionIn 2
  Exec "$INSTDIR\freenet.exe"
SectionEnd
;---------------------------------------------------------------------------------------

Section "View Readme.txt"
SectionIn 2
  ExecShell "open" "$INSTDIR\Readme.txt"
SectionEnd
;---------------------------------------------------------------------------------------

Section -PostInstall
  # Registering install path, so future installs will use the same path
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Freenet" "instpath" $INSTDIR
  # Registering the unistall information
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Freenet" "DisplayName" "Freenet"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Freenet" "UninstallString" '"$INSTDIR\Uninstall-Freenet.exe"'
SectionEnd
;------------------------------------------------------------------------------------------
# Uninstall part begins here:
Section Uninstall

  #First trying to shut down the node, the system tray Window class is called: TrayIconFreenetClass
  FindWindow "prompt" "TrayIconFreenetClass" "You are still running Freenet, please shut it down first and retry then."

  DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Freenet"
  DeleteRegKey HKEY_LOCAL_MACHINE "Software\Freenet"

  # Now deleting the rest
  RMDir /r $INSTDIR

  # remove IE plugin
  UnRegDLL $INSTDIR\IEplugin\IEFreenetPlugin.dll
  Delete $INSTDIR\IEplugin\*.*
  RMDir $INSTDIR\IEplugin
  DeleteRegKey HKEY_CLASSES_ROOT PROTOCOLS\Handler\freenet
  DeleteRegKey HKEY_CLASSES_ROOT freenet

  # remove the desktop and startmenu icons
  Delete "$SMSTARTUP\Freenet.lnk"
  Delete "$DESKTOP\Freenet.lnk"
  Delete "$SMPROGRAMS\Freenet\Freenet.lnk"
  Delete "$SMPROGRAMS\Freenet\FN Homepage.url"
  Delete "$SMPROGRAMS\Freenet\FNGuide.url"
  Delete "$SMPROGRAMS\Freenet\Uninstall.lnk" 
  RMDir "$SMPROGRAMS\Freenet"

  #delete "$SMPROGRAMS\Start FProxy.lnk"
  #Delete "$QUICKLAUNCH\Start FProxy.lnk"
SectionEnd
;-----------------------------------------------------------------------------------------

Function .onInit
  #First trying to shut down the node, the system tray Window class is called: TrayIconFreenetClass
  FindWindow "prompt" "TrayIconFreenetClass" "You are still running Freenet, please shut it down first and retry then."
FunctionEnd
;-----------------------------------------------------------------------------------------

Function .onInstFailed
  DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Freenet"
  DeleteRegKey HKEY_LOCAL_MACHINE "Software\Freenet"
  RMDir /r $INSTDIR
FunctionEnd

;-----------------------------------------------------------------------------------------