#define bit64

#define MyAppName               "Ditto"
#ifdef bit64
  #define MyAppVersion            GetFileVersion("..\Release64\Ditto.exe")
#endif
#ifndef bit64
  #define MyAppVersion            GetFileVersion("..\Release\Ditto.exe")
#endif
#define MyAppVerName            MyAppName + " " + MyAppVersion
#define MyAppPublisher          "Scott Brogden"
#define MyAppSupportURL         "ditto-cp.sourceforge.net"
#define MyAppCopyrighEndYear    GetDateTimeString('yyyy','','')

;#define bit64
 
[Setup]
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppVerName}

AppPublisher={# MyAppPublisher}

AppPublisherURL={#MyAppSupportURL}
AppSupportURL={#MyAppSupportURL}
AppUpdatesURL={#MyAppSupportURL}

WizardStyle=modern

UninstallDisplayIcon={app}\Ditto.exe
UninstallDisplayName={#MyAppName}

VersionInfoDescription={#MyAppName} installer
VersionInfoVersion={#MyAppVersion}
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

AppCopyright={#MyAppPublisher} {#MyAppCopyrighEndYear}

OutputBaseFilename=DittoSetup_{#MyAppVersion}
#ifdef bit64
  ArchitecturesInstallIn64BitMode=x64compatible
  ArchitecturesAllowed=x64compatible
#endif
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
;UsePreviousTasks=no
;DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DirExistsWarning=no
UninstallLogMode=overwrite
ChangesAssociations=yes
CloseApplications=yes
MinVersion=6.1
SetupLogging=yes

[Languages]
Name: English; MessagesFile: compiler:Default.isl

;Inno Setup Official translations
Name: Czech;               MessagesFile: compiler:Languages\Czech.isl
Name: Danish;              MessagesFile: compiler:Languages\Danish.isl
Name: Dutch;               MessagesFile: compiler:Languages\Dutch.isl
Name: Finnish;             MessagesFile: compiler:Languages\Finnish.isl
Name: French;              MessagesFile: compiler:Languages\French.isl
Name: Deutsch;             MessagesFile: compiler:Languages\German.isl
Name: Hebrew;              MessagesFile: compiler:Languages\Hebrew.isl
Name: Italiano;            MessagesFile: compiler:Languages\Italian.isl
Name: Japanese;            MessagesFile: compiler:Languages\Japanese.isl
Name: Polski;              MessagesFile: compiler:Languages\Polish.isl
Name: Portuguese;          MessagesFile: compiler:Languages\Portuguese.isl
Name: Russian;             MessagesFile: compiler:Languages\Russian.isl
Name: Slovak;              MessagesFile: compiler:Languages\Slovak.isl
Name: Slovenian;           MessagesFile: compiler:Languages\Slovenian.isl
Name: Spanish;             MessagesFile: compiler:Languages\Spanish.isl
Name: Turkish;             MessagesFile: compiler:Languages\Turkish.isl
Name: Ukrainian;           MessagesFile: compiler:Languages\Ukrainian.isl

;Inno Setup Unofficial translations
Name: ChineseSimplified;   MessagesFile: ChineseSimplified.isl
Name: ChineseTraditional;  MessagesFile: ChineseTraditional.isl
Name: Croatian;            MessagesFile: Croatian.isl
Name: Greek;               MessagesFile: Greek.isl
Name: Hungarian;           MessagesFile: Hungarian.isl
Name: Korean;              MessagesFile: Korean.isl
Name: Romanian;            MessagesFile: Romanian.isl
Name: Swedish;             MessagesFile: Swedish.isl

[CustomMessages]
English.RunDittoOnStartup=Run Ditto on Windows startup
English.AddFirewallException=Add Windows Firewall exception for Ditto on port 23443
English.LaunchDitto=Launch Ditto
English.ViewHelp=View Help
English.ViewChangeHistory=View Change History
English.UninstallDitto=Uninstall Ditto
English.VCRuntimeInstallFailed=VCRuntime prerequisite install failed.

Italiano.RunDittoOnStartup=Esegui Ditto all'avvio di Windows
Italiano.AddFirewallException=Aggiungi eccezione a Windows Firewall per Ditto e la porta 23443
Italiano.LaunchDitto=Esegui Ditto
Italiano.ViewHelp=Visualizza guida in linea
Italiano.ViewChangeHistory=Visualizza cronologia versioni programma
Italiano.UninstallDitto=Disinstalla Ditto
Italiano.VCRuntimeInstallFailed=Installazione prerequisito VCRuntime non riuscita.

[Tasks]
Name: RunAtStartup; Description: {cm:RunDittoOnStartup}
Name: AddFireWallException; Description: {cm:AddFirewallException};  Flags: unchecked

[Files]
#ifdef bit64
  Source: ..\Release64\Ditto.exe; DestDir: {app}; DestName: Ditto.exe; Flags: ignoreversion; AfterInstall: AddProgramToFirewall(ExpandConstant('{app}\Ditto.exe'), 'Ditto_FromInstaller_64');
  Source: ..\Release64\ICU_Loader.dll; DestDir: {app}; Flags: ignoreversion
  Source: ..\Release64\Addins\DittoUtil.dll; DestDir: {app}\Addins; Flags: ignoreversion

  ; "C:\Windows\sysnative" will be converted to "C:\Windows\System32"
  ; System32 stores a 64-bit DLL on x64 system
  Source: C:\Windows\sysnative\vcruntime140.dll;  DestDir: {app}; Flags: ignoreversion
  Source: C:\Windows\sysnative\vcruntime140_1.dll;  DestDir: {app}; Flags: ignoreversion 
  Source: C:\Windows\sysnative\msvcp140.dll;  DestDir: {app}; Flags: ignoreversion
  Source: C:\Windows\sysnative\mfc140u.dll;  DestDir: {app}; Flags: ignoreversion
#endif
#ifndef bit64
  Source: ..\Release\Ditto.exe; DestDir: {app}; DestName: Ditto.exe; Flags: ignoreversion; AfterInstall: AddProgramToFirewall(ExpandConstant('{app}\Ditto.exe'), 'Ditto_FromInstaller_32');
  Source: ..\Release\ICU_Loader.dll; DestDir: {app}; Flags: ignoreversion
  Source: ..\Release\Addins\DittoUtil.dll; DestDir: {app}\Addins; Flags: ignoreversion

  Source: C:\Windows\SysWOW64\vcruntime140.dll;  DestDir: {app}; Flags: ignoreversion
  Source: C:\Windows\SysWOW64\msvcp140.dll;  DestDir: {app}; Flags: ignoreversion
  Source: C:\Windows\SysWOW64\mfc140u.dll;  DestDir: {app}; Flags: ignoreversion
#endif

Source: ..\Debug\Language\*; DestDir: {app}\Language; BeforeInstall: BeforeLanguageInstall()
Source: ..\Debug\Themes\*; DestDir: {app}\Themes

[Icons]
Name: {group}\Ditto; Filename: {app}\Ditto.exe
Name: {group}\{cm:UninstallDitto}; Filename: {uninstallexe}

[Run]
Filename: {app}\Ditto.exe; Description: {cm:LaunchDitto}; Flags: nowait postinstall
Filename: https://sourceforge.net/p/ditto-cp/wiki/Getting%20Started; Description: {cm:ViewHelp}; Flags: nowait postinstall skipifsilent shellexec unchecked
Filename: https://ditto-cp.sourceforge.io/changeHistory.php; Description: {cm:ViewChangeHistory}; Flags: nowait postinstall skipifsilent shellexec unchecked

[Registry]
Root: HKCU; Subkey: Software\Ditto; Flags: uninsdeletekey
Root: HKCU; Subkey: SOFTWARE\Microsoft\Windows\CurrentVersion\Run; ValueType: string; ValueName: Ditto; flags: uninsdeletevalue; ValueData: {app}\Ditto.exe; Tasks: RunAtStartup

Root: HKCU; Subkey: Software\Ditto; ValueType: dword; ValueName: SetFocus_iexplore.exe; ValueData: 00000001

Root: HKCU; Subkey: Software\Ditto; ValueType: string; ValueName: LanguageFile; ValueData: "{code:AdjustedLanguage|{language}}"

Root: HKCU; Subkey: Software\Ditto\PasteStrings; ValueType: string; ValueName: gvim.exe; ValueData: """{{PLUS}gP"
Root: HKCU; Subkey: Software\Ditto\CopyStrings; ValueType: string; ValueName: gvim.exe; ValueData: """{{PLUS}y"
Root: HKCU; Subkey: Software\Ditto\CutStrings; ValueType: string; ValueName: gvim.exe; ValueData: """{{PLUS}x"

Root: HKCU; Subkey: Software\Ditto\PasteStrings; ValueType: string; ValueName: cmd.exe; OnlyBelowVersion: 10; ValueData: % {{Delay100}ep
Root: HKCU; Subkey: Software\Ditto\CopyStrings; ValueType: string; ValueName: cmd.exe; OnlyBelowVersion: 10; ValueData: % {{Delay100}ey

Root: HKCU; Subkey: Software\Ditto\PasteStrings; ValueName: cmd.exe; MinVersion: 10; Flags: deletevalue
Root: HKCU; Subkey: Software\Ditto\CopyStrings; ValueName: cmd.exe; MinVersion: 10; Flags: deletevalue

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
end;

function AdjustedLanguage(Param: String): String;  
begin
  Case Param of
    'ChineseSimplified' : Result := 'Chinese Simplified';
    'ChineseTraditional' : Result := 'Chinese Traditional';
  else
    Result := Param;
  end;
end;

procedure CleanupOldFiles();
var
  sDir: String;
  begin
    sDir := ExpandConstant('{app}');

    DeleteFile(sDir+'\mfc100u.dll')
    DeleteFile(sDir+'\mfcm100u.dll')
    DeleteFile(sDir+'\msvcp100.dll')
    DeleteFile(sDir+'\msvcr100.dll')

    DeleteFile(sDir+'\iculx55.dll')
    DeleteFile(sDir+'\icule55.dll')
    DeleteFile(sDir+'\icuuc55.dll')
    DeleteFile(sDir+'\icutu55.dll')
    DeleteFile(sDir+'\icuio55.dll')
    DeleteFile(sDir+'\icuin55.dll')
    DeleteFile(sDir+'\icudt55.dll')


    //moved to use the windows included dll
    DeleteFile(sDir+'\icuuc58.dll')
    DeleteFile(sDir+'\icuin58.dll')
    DeleteFile(sDir+'\icutu58.dll')
    DeleteFile(sDir+'\icuio58.dll')
    DeleteFile(sDir+'\icudt58.dll')

    DelTree(sDir+'\Help', TRUE, TRUE, TRUE)
end;


procedure RegisterForCrashDump(theApp : String);
var
   theExe : String;
begin
  theExe := theApp + '.exe';
  if IsWin64() then
    begin
      if RegValueExists(HKLM64, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpFolder') = false then
        RegWriteStringValue(HKLM64, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpFolder', ExpandConstant('{userappdata}') + '\Ditto\Dumps');
      if RegValueExists(HKLM64, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpType') = false then
        RegWriteDWordValue(HKLM64, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpType', 2);
      if RegValueExists(HKLM64, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpCount') = false then
        RegWriteDWordValue(HKLM64, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpCount', 3);
    end
  else
    begin
      if RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpFolder') = false then
        RegWriteStringValue(HKEY_LOCAL_MACHINE, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpFolder', ExpandConstant('{userappdata}') + '\Ditto\Dumps');
      if RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpType') = false then
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpType', 2);
      if RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpCount') = false then
        RegWriteDWordValue(HKEY_LOCAL_MACHINE, 'Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\' + theExe, 'DumpCount', 3);
    end;
end;

function IsVC2017CRuntimeInstalled(): Boolean;
var
  Installed: Boolean;
  IsInstalled: Cardinal;
begin
  Installed := false
  IsInstalled := 0;

  #ifdef bit64

    if RegQueryDWordValue(HKLM, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', IsInstalled) then
    begin
      if (IsInstalled = 1) then
      begin
        Installed := true;
      end;
    end;  

    //double check the HKLM64 key
    if (IsInstalled <> 1) and IsWin64() then 
    begin
      if RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', IsInstalled) then
      begin
        if (IsInstalled = 1) then
        begin
          Installed := true;
        end;
      end;  
    end;

  #endif
  #ifndef bit64
    if RegQueryDWordValue(HKLM, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86', 'Installed', IsInstalled) then
    begin
      if (IsInstalled = 1) then
      begin
        Installed := true;
      end;
    end;  

    //double check the HKLM64 key
    if (IsInstalled <> 1) and IsWin64() then 
    begin
      if RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86', 'Installed', IsInstalled) then
      begin
        if (IsInstalled = 1) then
        begin
          Installed := true;
        end;
      end;  
    end;
  #endif

  Result := Installed;
end;

procedure CheckForPreReqs();
var
  nReturnCode: Integer;
begin
 

end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ProgressPage: TOutputProgressWizardPage;
  ErrorCode : Integer;
  AbortNeeded: Boolean;
begin
  AbortNeeded := false;
  case CurStep of
    ssInstall:      
    begin
      CheckForPreReqs();
    end;

	  ssPostInstall:
  	begin            
    end;
	
    ssDone:
    begin
      RegisterForCrashDump('Ditto')
      CleanupOldFiles()
    end;

  end;
end;


function RuleExistsInFirewall(RuleName : String) : Boolean;
var
  ErrorCode : Integer;
begin
  Exec('>', 'netsh advfirewall firewall show rule name="' + RuleName + '"', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
  if ErrorCode = 0 then
    Result := True
  else
    Result := False;
end;

procedure AddProgramToFirewall(ProgramName : String; RuleName : String);
var
  ErrorCode : Integer;
  Success : Boolean;
  WindowsVersion : TWindowsVersion;
begin
  if IsTaskSelected('AddFireWallException') then
    begin
    GetWindowsVersionEx(WindowsVersion);
    if (WindowsVersion.Major < 6) then
      begin
        Success := Exec('>', 'netsh firewall add allowedprogram "' + ProgramName + '" "' + RuleName + '" ENABLE ALL', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
      end
    else
      begin
        if (not RuleExistsInFirewall(RuleName)) then
          begin
            Success := Exec('>', 'netsh advfirewall firewall add rule name="' + RuleName + '" dir=in action=allow protocol=TCP localport=23443 program="' + ProgramName + '" enable=yes', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
            Success := Exec('>', 'netsh advfirewall firewall add rule name="' + RuleName + '" dir=out action=allow protocol=TCP localport=23443 program="' + ProgramName + '" enable=yes', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
          end
      end;
     
    if not Success then
       Log('Error - Unable to add ' + RuleName + ' to List of Windows firewall exceptions. ErrorCode: ' + IntToStr(ErrorCode))
    else
       Log(RuleName + ' successfully added to list of Windows firewall exceptions. ErrorCode: ' + IntToStr(ErrorCode))
  end
end;

