; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Portable Ditto
AppVerName=Ditto Portable 3.15.0.0
OutputBaseFilename=DittoPortableSetup_3_15_0_0
AppPublisher=Scott Brogden
AppPublisherURL=ditto-cp.sourceforge.net
AppSupportURL=ditto-cp.sourceforge.net
AppUpdatesURL=ditto-cp.sourceforge.net
DefaultDirName= C:\Portable Device\Ditto
DefaultGroupName=Ditto
UsePreviousTasks=no
DisableProgramGroupPage=yes
DisableReadyPage=yes
DirExistsWarning=no
CreateUninstallRegKey=no

[Languages]
Name: "English"; MessagesFile: "compiler:Default.isl"
;Inno Setup Official translations
Name: Czech; MessagesFile: compiler:Languages\Czech.isl
Name: Danish; MessagesFile: compiler:Languages\Danish.isl
Name: Dutch; MessagesFile: compiler:Languages\Dutch.isl
Name: Finnish; MessagesFile: compiler:Languages\Finnish.isl
Name: French; MessagesFile: compiler:Languages\French.isl
Name: German; MessagesFile: compiler:Languages\German.isl
Name: Hebrew; MessagesFile: compiler:Languages\Hebrew.isl
Name: Italiano; MessagesFile: compiler:Languages\Italian.isl
Name: Japanese; MessagesFile: compiler:Languages\Japanese.isl
Name: Polish; MessagesFile: compiler:Languages\Polish.isl
Name: Portuguese_Brazil; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: Portuguese_Portugal; MessagesFile: compiler:Languages\Portuguese.isl
Name: Russian; MessagesFile: compiler:Languages\Russian.isl
Name: Slovak; MessagesFile: compiler:Languages\Slovak.isl
Name: Slovenian; MessagesFile: compiler:Languages\Slovenian.isl
Name: Spanish; MessagesFile: compiler:Languages\Spanish.isl
Name: Turkish; MessagesFile: compiler:Languages\Turkish.isl
Name: Ukrainian; MessagesFile: compiler:Languages\Ukrainian.isl

;Inno Setup Unofficial translations
Name: Simplified_Chinese; MessagesFile: ChineseSimplified.isl
Name: Traditional_Chinese; MessagesFile: ChineseTraditional.isl
Name: Croatian; MessagesFile: Croatian.isl
Name: Greek; MessagesFile: Greek.isl
Name: Hungarian; MessagesFile: Hungarian.isl
Name: Korean; MessagesFile: Korean.isl
Name: Romanian; MessagesFile: Romanian.isl
Name: Swedish; MessagesFile: Swedish.isl

[Files]
Source: "..\Release\DittoU.exe"; DestDir: "{app}"; DestName: "Ditto.exe"; Flags: ignoreversion;

Source: "..\Release\focus.dll"; DestDir: "{app}"; BeforeInstall: BeforeFocusInstall(); Flags: ignoreversion restartreplace
Source: "..\Release\sqlite3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\AccessToSqlite.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\zlib\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "Changes.txt"; DestDir: "{app}"

Source: "..\Debug\Language\*"; DestDir: "{app}\Language";

Source: "mfc-crt\*"; DestDir: "{app}"

;portable app settings stored in an .ini file
Source: "Ditto.Settings"; DestDir: "{app}"; Flags: onlyifdoesntexist

;Add help files
Source: "..\Help\*.htm"; DestDir: "{app}\Help"; Flags: ignoreversion

[Run]
Filename: "{app}\Ditto.exe"; Description: "Launch Ditto"; Flags: nowait postinstall skipifsilent
Filename: "{app}\Help\DittoGettingStarted.htm"; Description: "View Help"; Flags: nowait postinstall skipifsilent shellexec
Filename: "{app}\Changes.txt"; Description: "View Change History"; Flags: nowait postinstall skipifsilent shellexec unchecked

[INI]
Filename: "{app}\Ditto.Settings"; Section: "Ditto"; Key: "LanguageFile"; String: {language}

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
