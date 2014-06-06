#define MyAppName "Ditto"
#define MyAppVersion GetFileVersion("..\Release\DittoU.exe")
#define MyAppVerName MyAppName + " " + MyAppVersion

;#define bit64
  

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
OutputBaseFilename=DittoSetup_{#MyAppVersion}
AppPublisher=Scott Brogden
AppPublisherURL=ditto-cp.sourceforge.net
AppSupportURL=ditto-cp.sourceforge.net
AppUpdatesURL=ditto-cp.sourceforge.net
#ifdef bit64
  ArchitecturesInstallIn64BitMode=x64
  ArchitecturesAllowed=x64
#endif
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


Name: Deutsch; MessagesFile: German.isl
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
Name: Russian; MessagesFile: Russian.isl
Name: Slovenian; MessagesFile: Slovenian.isl
Name: Czech; MessagesFile: Czech.isl
Name: Danish; MessagesFile: Danish.isl
Name: Greek; MessagesFile: Greek.isl
Name: Ukrainian; MessagesFile: Ukrainian.isl

[Tasks]
Name: RunAtStartup; Description: Run Ditto on Windows Startup

[Files]
#ifdef bit64
	Source: ..\Release64\Ditto.exe; DestDir: {app}; DestName: Ditto.exe; Flags: ignoreversion;
	Source: ..\Release64\Addins\DittoUtil.dll; DestDir: {app}\Addins; Flags: ignoreversion
	Source: mfc-crt64\*; DestDir: {app}      
#endif
#ifndef bit64
	Source: ..\Release\Ditto.exe; DestDir: {app}; DestName: Ditto.exe; Flags: ignoreversion;
	Source: ..\Addins\DittoUtil\Release\DittoUtil.dll; DestDir: {app}\Addins; Flags: ignoreversion
	Source: mfc-crt_10\*; DestDir: {app}
#endif

Source: Changes.txt; DestDir: {app}
Source: ..\Debug\Language\*; DestDir: {app}\Language; BeforeInstall: BeforeLanguageInstall()
Source: ..\Debug\Themes\*; DestDir: {app}\Themes
Source: ..\Help\*.htm; DestDir: {app}\Help; Flags: ignoreversion

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

Root: HKCU; Subkey: Software\Ditto; ValueType: dword; ValueName: SetFocus_iexplore.exe; ValueData: 00000001

Root: HKCU; Subkey: Software\Ditto; ValueType: string; ValueName: LanguageFile; ValueData: {language}

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
procedure BeforeLanguageInstall();
var
  sDir: String;
begin
    sDir := ExpandConstant('{app}');

    RenameFile(sDir+'\Language\Italian.xml', sDir+'\Language\Italian.xml.old')
end;






