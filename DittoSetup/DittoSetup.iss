#define MyAppName "Ditto"
#define MyAppVersion GetFileVersion("..\Release\Ditto.exe")
#define MyAppVerName MyAppName + " " + MyAppVersion

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
OutputBaseFilename=DittoSetup_{#MyAppVersion}
AppPublisher=Scott Brogden
AppPublisherURL=ditto-cp.sourceforge.net
AppSupportURL=ditto-cp.sourceforge.net
AppUpdatesURL=ditto-cp.sourceforge.net
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AppMutex=Ditto Is Now Running
;UsePreviousTasks=no
;DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DirExistsWarning=no
UninstallLogMode=overwrite
ChangesAssociations=yes

[Languages]
Name: English; MessagesFile: compiler:Default.isl
Name: German; MessagesFile: German.isl
Name: Italiano; MessagesFile: Italian.isl
Name: French; MessagesFile: French.isl
Name: Portuguese; MessagesFile: Portuguese.isl
Name: Spanish; MessagesFile: Spanish.isl
Name: Polski; MessagesFile: Polish.isl
Name: Dutch; MessagesFile: Dutch.isl
Name: Swedish; MessagesFile: Swedish.isl
Name: Croatian; MessagesFile: Croatian.isl
Name: Turkish; MessagesFile: Turkish.isl
Name: Japanese; MessagesFile: Japanese.isl
Name: Chinese; MessagesFile: ChineseSimp.isl
Name: Romanian; MessagesFile: Romanian.isl
Name: Korean; MessagesFile: Korean.isl


[Tasks]
Name: RunAtStartup; Description: Run Ditto on Windows Startup
;Name: UseFocusDll; Description: Use System Hook to track currently focused window

Name: OutlookExpressAddIn; Description: Outlook Express Add-in; GroupDescription: Add-Ins

[Files]
;Unicode for 2000 and later
Source: ..\Release\DittoU.exe; DestDir: {app}; DestName: Ditto.exe; Flags: ignoreversion; MinVersion: 0, 4.0

;Non unicode for pre 2000
Source: ..\Release\Ditto.exe; DestDir: {app}; DestName: Ditto.exe; Flags: ignoreversion; MinVersion: 4.0, 0

Source: ..\Release\focus.dll; DestDir: {app}; BeforeInstall: BeforeFocusInstall(); Flags: ignoreversion restartreplace
Source: ..\Release\sqlite3.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\Release\AccessToSqlite.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\zlib\zlib1.dll; DestDir: {app}; Flags: ignoreversion

Source: Changes.txt; DestDir: {app}

Source: ..\Debug\Language\*; DestDir: {app}\Language; BeforeInstall: BeforeLanguageInstall()
Source: ..\Debug\Themes\*; DestDir: {app}\Themes

Source: mfc-crt\*; DestDir: {app}

;Add help files
Source: ..\Help\*.htm; DestDir: {app}\Help; Flags: ignoreversion

Source: ..\Addins\OutlookExpress\Release\OutlookExpress.dll; DestDir: {app}\Addins; Flags: ignoreversion; Tasks: OutlookExpressAddIn

[Icons]
Name: {group}\Ditto; Filename: {app}\Ditto.exe
Name: {group}\Ditto Help; Filename: {app}\Help\DittoGettingStarted.htm
Name: {group}\Uninstall; Filename: {uninstallexe}

[Run]
Filename: {app}\Ditto.exe; Description: Launch Ditto; Flags: nowait postinstall skipifsilent
Filename: {app}\Help\DittoGettingStarted.htm; Description: View Help; Flags: nowait postinstall skipifsilent shellexec
Filename: {app}\Changes.txt; Description: View Change History; Flags: nowait postinstall skipifsilent shellexec unchecked

[Registry]
Root: HKCU; Subkey: Software\Ditto; Flags: uninsdeletekey
Root: HKCU; Subkey: SOFTWARE\Microsoft\Windows\CurrentVersion\Run; ValueType: string; ValueName: Ditto; flags: uninsdeletevalue; ValueData: {app}\Ditto.exe; Tasks: RunAtStartup
Root: HKCU; Subkey: Software\Ditto; ValueType: string; ValueName: LanguageFile; ValueData: {language}

;Always set UseHookdll to false then to true only if the task is set
;Root: HKCU; Subkey: Software\Ditto; ValueType: dword; ValueName: UseHookDllForFocus; ValueData: 0
;Root: HKCU; Subkey: Software\Ditto; ValueType: dword; ValueName: UseHookDllForFocus; ValueData: 1; Tasks: UseFocusDll

Root: HKCU; Subkey: Software\Ditto\PasteStrings; ValueType: string; ValueName: gvim.exe; ValueData: """{{PLUS}gP"
Root: HKCU; Subkey: Software\Ditto\CopyStrings; ValueType: string; ValueName: gvim.exe; ValueData: """{{PLUS}y"
Root: HKCU; Subkey: Software\Ditto\CutStrings; ValueType: string; ValueName: gvim.exe; ValueData: """{{PLUS}x"

Root: HKCU; Subkey: Software\Ditto\PasteStrings; ValueType: string; ValueName: cmd.exe; ValueData: % {{Delay100}ep
Root: HKCU; Subkey: Software\Ditto\CopyStrings; ValueType: string; ValueName: cmd.exe; ValueData: % {{Delay100}ey

;associate .dto with Ditto
Root: HKCR; Subkey: .dto; ValueType: string; ValueName: ; ValueData: Ditto; Flags: uninsdeletevalue
Root: HKCR; Subkey: Ditto; ValueType: string; ValueName: ; ValueData: Ditto; Flags: uninsdeletekey
Root: HKCR; Subkey: Ditto\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\Ditto.exe,0
Root: HKCR; Subkey: Ditto\shell\open\command; ValueType: string; ValueName: ; ValueData: """{app}\Ditto.exe"" ""%1"""


[Code]
procedure BeforeFocusInstall();
var
  sDir: String;
begin
    sDir := ExpandConstant('{app}');

    DeleteFile(sDir+'\focus.dll')
    DeleteFile(sDir+'\focus.dll.old')
    DeleteFile(sDir+'\focus.dll.old.old')
    DeleteFile(sDir+'\focus.dll.old.old.old')

    RenameFile(sDir+'\focus.dll', sDir+'\focus.dll.old')
    RenameFile(sDir+'\focus.dll', sDir+'\focus.dll.old.old')
    RenameFile(sDir+'\focus.dll', sDir+'\focus.dll.old.old.old')
end;

procedure BeforeLanguageInstall();
var
  sDir: String;
begin
    sDir := ExpandConstant('{app}');

    RenameFile(sDir+'\Language\Italian.xml', sDir+'\Language\Italian.xml.old')
end;
