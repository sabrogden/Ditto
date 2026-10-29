param([string]$StartupRegistryPath = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run')
$ErrorActionPreference = 'Stop'
$programsRoot = [IO.Path]::GetFullPath((Join-Path $env:LOCALAPPDATA 'Programs'))
$target = [IO.Path]::GetFullPath((Join-Path $programsRoot 'Ditto-UIA-Compat'))
if (-not $target.StartsWith($programsRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Unexpected installation path.' }
$statePath = Join-Path $target 'rollback.json'
if (-not (Test-Path -LiteralPath $statePath -PathType Leaf)) { throw 'Compatibility rollback metadata not found.' }
$state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
if ($state.compatExe -ne (Join-Path $target 'Ditto.exe') -or -not (Test-Path -LiteralPath $state.originalExe -PathType Leaf)) { throw 'Rollback metadata does not match this installation.' }
$expectedShortcut = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Ditto\Ditto.lnk'
if ($state.shortcutPath -ne $expectedShortcut) { throw 'Unexpected shortcut path in rollback metadata.' }
$runPath = $StartupRegistryPath
function Start-Interactive([string]$Exe, [string]$Arguments = '') {
    $name = 'Ditto-UIA-' + [guid]::NewGuid().ToString('N')
    $principal = New-ScheduledTaskPrincipal -UserId "$env:USERDOMAIN\$env:USERNAME" -LogonType Interactive -RunLevel Limited
    $options = @{ Execute = $Exe; WorkingDirectory = (Split-Path -Parent $Exe) }
    if ($Arguments) { $options.Argument = $Arguments }
    $task = New-ScheduledTask -Action (New-ScheduledTaskAction @options) -Principal $principal
    Register-ScheduledTask -TaskName $name -InputObject $task | Out-Null
    try { Start-ScheduledTask -TaskName $name } finally { Unregister-ScheduledTask -TaskName $name -Confirm:$false }
}

Start-Interactive $state.compatExe '/Exit'
for ($i = 0; $i -lt 20; $i++) {
    $running = @(Get-CimInstance Win32_Process -Filter "Name='Ditto.exe'" | Where-Object { $_.ExecutablePath -eq $state.compatExe })
    if (-not $running.Count) { break }
    Start-Sleep -Milliseconds 250
}
if ($running.Count) { throw 'The compatibility build did not exit; startup settings remain unchanged.' }
try {
    $runKey = Get-Item -Path $runPath
    $current = $runKey.GetValue('Ditto', $null, [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames)
    if ($state.runExists -and $current -and $current.Trim().Trim('"') -eq $state.compatExe) {
        New-ItemProperty -Path $runPath -Name Ditto -Value ([string]$state.oldRun) -PropertyType ([string]$state.oldRunKind) -Force | Out-Null
    }
    if (Test-Path -LiteralPath $state.shortcutPath -PathType Leaf) {
        $shell = New-Object -ComObject WScript.Shell
        if ($shell.CreateShortcut($state.shortcutPath).TargetPath -eq $state.compatExe) {
            if ($state.shortcutExists) { Copy-Item -LiteralPath (Join-Path $target 'previous-Ditto.lnk') -Destination $state.shortcutPath -Force }
            else { Remove-Item -LiteralPath $state.shortcutPath -Force }
        }
    }
    Start-Interactive $state.originalExe
    Write-Output "Started original Ditto: $($state.originalExe)"
} catch {
    Start-Interactive $state.compatExe
    throw
}
