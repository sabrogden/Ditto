param(
    [string]$BuiltExe = (Join-Path $PSScriptRoot '..\Release64\Ditto.exe'),
    [string]$OriginalDir = (Join-Path $env:ProgramFiles 'Ditto'),
    [string]$StartupRegistryPath = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'
)
$ErrorActionPreference = 'Stop'
$programsRoot = [IO.Path]::GetFullPath((Join-Path $env:LOCALAPPDATA 'Programs'))
$target = [IO.Path]::GetFullPath((Join-Path $programsRoot 'Ditto-UIA-Compat'))
$originalDirPath = (Resolve-Path -LiteralPath $OriginalDir).Path
$originalExe = Join-Path $originalDirPath 'Ditto.exe'
$builtExePath = (Resolve-Path -LiteralPath $BuiltExe).Path
$runPath = $StartupRegistryPath
$shortcutDir = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Ditto'
$shortcutPath = Join-Path $shortcutDir 'Ditto.lnk'

function Start-Interactive([string]$Exe, [string]$Arguments = '') {
    $taskName = 'Ditto-UIA-' + [guid]::NewGuid().ToString('N')
    $principal = New-ScheduledTaskPrincipal -UserId "$env:USERDOMAIN\$env:USERNAME" -LogonType Interactive -RunLevel Limited
    $options = @{ Execute = $Exe; WorkingDirectory = (Split-Path -Parent $Exe) }
    if ($Arguments) { $options.Argument = $Arguments }
    $task = New-ScheduledTask -Action (New-ScheduledTaskAction @options) -Principal $principal
    Register-ScheduledTask -TaskName $taskName -InputObject $task | Out-Null
    try { Start-ScheduledTask -TaskName $taskName } finally { Unregister-ScheduledTask -TaskName $taskName -Confirm:$false }
}

if (-not $target.StartsWith($programsRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Unexpected installation path.' }
if (-not (Test-Path -LiteralPath $originalExe -PathType Leaf)) { throw 'Original Ditto executable not found.' }
if ((Get-Item -LiteralPath $originalExe).VersionInfo.FileVersion -ne '3.25.113.0') { throw 'The original Ditto version differs from the build base.' }
if ((Get-Item -LiteralPath $builtExePath).VersionInfo.FileVersion -ne '3.25.113.1') { throw 'The compatibility build has an unexpected version.' }
if (Test-Path -LiteralPath $target) { throw 'Compatibility installation already exists; use the rollback script first.' }
if (Test-Path -LiteralPath (Join-Path $originalDirPath 'Ditto.Settings')) { throw 'INI-based installations require manual migration.' }
$runKey = Get-Item -Path $runPath
$runExists = $runKey.GetValueNames() -contains 'Ditto'
$oldRun = if ($runExists) { $runKey.GetValue('Ditto', $null, [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames) } else { $null }
$oldRunKind = if ($runExists) { $runKey.GetValueKind('Ditto').ToString() } else { $null }
$shortcutExists = Test-Path -LiteralPath $shortcutPath -PathType Leaf
if ($runExists -and $oldRun.Trim().Trim('"') -ne $originalExe) { throw 'Existing Ditto startup points elsewhere; refusing to overwrite it.' }
if ($shortcutExists) {
    $existingShell = New-Object -ComObject WScript.Shell
    if ($existingShell.CreateShortcut($shortcutPath).TargetPath -ne $originalExe) { throw 'Existing user shortcut points elsewhere; refusing to overwrite it.' }
}

New-Item -ItemType Directory -Path $target | Out-Null
Get-ChildItem -LiteralPath $originalDirPath -Force | Where-Object Name -NotLike 'unins*' | Copy-Item -Destination $target -Recurse -Force
Copy-Item -LiteralPath $builtExePath -Destination (Join-Path $target 'Ditto.exe') -Force
if ($shortcutExists) { Copy-Item -LiteralPath $shortcutPath -Destination (Join-Path $target 'previous-Ditto.lnk') }
$state = @{ originalExe = $originalExe; compatExe = (Join-Path $target 'Ditto.exe'); runExists = $runExists; oldRun = $oldRun; oldRunKind = $oldRunKind; shortcutPath = $shortcutPath; shortcutExists = $shortcutExists }
$state | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $target 'rollback.json') -Encoding utf8

try {
    Start-Interactive $originalExe '/Exit'
    for ($i = 0; $i -lt 20; $i++) {
        $old = @(Get-CimInstance Win32_Process -Filter "Name='Ditto.exe'" | Where-Object { $_.ExecutablePath -eq $originalExe })
        if (-not $old.Count) { break }
        Start-Sleep -Milliseconds 250
    }
    if ($old.Count) { throw 'The original Ditto did not exit; startup settings remain unchanged.' }
    if ($runExists) { Set-ItemProperty -Path $runPath -Name Ditto -Value ('"{0}"' -f $state.compatExe) }
    New-Item -ItemType Directory -Path $shortcutDir -Force | Out-Null
    $shell = New-Object -ComObject WScript.Shell
    $shortcut = $shell.CreateShortcut($shortcutPath)
    $shortcut.TargetPath = $state.compatExe
    $shortcut.WorkingDirectory = $target
    $shortcut.Description = 'Ditto - UI Automation compatibility build'
    $shortcut.Save()
    Start-Interactive $state.compatExe
    $started = $false
    for ($i = 0; $i -lt 20; $i++) {
        if (Get-CimInstance Win32_Process -Filter "Name='Ditto.exe'" | Where-Object { $_.ExecutablePath -eq $state.compatExe }) { $started = $true; break }
        Start-Sleep -Milliseconds 250
    }
    if (-not $started) { throw 'The compatibility build did not stay running.' }
    Write-Output "Started compatibility build: $($state.compatExe)"
} catch {
    $current = $runKey.GetValue('Ditto', $null, [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames)
    if ($runExists -and $current -and $current.Trim().Trim('"') -eq $state.compatExe) {
        New-ItemProperty -Path $runPath -Name Ditto -Value $oldRun -PropertyType $oldRunKind -Force | Out-Null
    }
    if ($shortcutExists) { Copy-Item -LiteralPath (Join-Path $target 'previous-Ditto.lnk') -Destination $shortcutPath -Force }
    elseif (Test-Path -LiteralPath $shortcutPath) {
        $shell = New-Object -ComObject WScript.Shell
        if ($shell.CreateShortcut($shortcutPath).TargetPath -eq $state.compatExe) { Remove-Item -LiteralPath $shortcutPath -Force }
    }
    Start-Interactive $originalExe
    throw
}
