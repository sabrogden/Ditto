Describe 'Per-user installation and rollback' {
BeforeEach {
    $script:oldLocal = $env:LOCALAPPDATA
    $script:oldRoaming = $env:APPDATA
    $caseRoot = Join-Path $TestDrive ([guid]::NewGuid().ToString('N'))
    $env:LOCALAPPDATA = Join-Path $caseRoot 'Local'
    $env:APPDATA = Join-Path $caseRoot 'Roaming'
    New-Item -ItemType Directory -Path $env:LOCALAPPDATA, $env:APPDATA -Force | Out-Null
    $script:testRegistry = 'HKCU:\Software\Codex\DittoCompatTest-' + [guid]::NewGuid().ToString('N')
    New-Item -Path $script:testRegistry -Force | Out-Null
    $script:originalExe = Join-Path $env:ProgramFiles 'Ditto\Ditto.exe'
    New-ItemProperty -Path $script:testRegistry -Name Ditto -Value $script:originalExe -PropertyType ExpandString -Force | Out-Null
    $script:shortcutDir = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Ditto'
    New-Item -ItemType Directory -Path $script:shortcutDir -Force | Out-Null
    $script:shortcutPath = Join-Path $script:shortcutDir 'Ditto.lnk'
    $shell = New-Object -ComObject WScript.Shell
    $shortcut = $shell.CreateShortcut($script:shortcutPath)
    $shortcut.TargetPath = $script:originalExe
    $shortcut.Description = 'Original shortcut'
    $shortcut.Save()
    Mock Register-ScheduledTask { }
    Mock Start-ScheduledTask { }
    Mock Unregister-ScheduledTask { }
    Mock Start-Sleep { }
}

AfterEach {
    $env:LOCALAPPDATA = $script:oldLocal
    $env:APPDATA = $script:oldRoaming
    if ($script:testRegistry -match '^HKCU:\\Software\\Codex\\DittoCompatTest-[0-9a-f]{32}$') {
        Remove-Item -LiteralPath $script:testRegistry -Recurse -Force -ErrorAction SilentlyContinue
    }
}

    It 'restores the original startup value, value kind and shortcut' {
        Mock Get-CimInstance { [pscustomobject]@{ ExecutablePath = (Join-Path $env:LOCALAPPDATA 'Programs\Ditto-UIA-Compat\Ditto.exe') } }
        & "$PSScriptRoot\..\scripts\install-uia-compat.ps1" -StartupRegistryPath $script:testRegistry
        $compat = Join-Path $env:LOCALAPPDATA 'Programs\Ditto-UIA-Compat\Ditto.exe'
        Test-Path -LiteralPath $compat | Should -BeTrue
        (Get-ItemProperty -Path $script:testRegistry).Ditto.Trim('"') | Should -Be $compat
        $shell = New-Object -ComObject WScript.Shell
        $shell.CreateShortcut($script:shortcutPath).TargetPath | Should -Be $compat

        Mock Get-CimInstance { }
        & "$PSScriptRoot\..\scripts\rollback-uia-compat.ps1" -StartupRegistryPath $script:testRegistry
        $key = Get-Item -Path $script:testRegistry
        $key.GetValue('Ditto', $null, [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames) | Should -Be $script:originalExe
        $key.GetValueKind('Ditto').ToString() | Should -Be 'ExpandString'
        $shell.CreateShortcut($script:shortcutPath).TargetPath | Should -Be $script:originalExe
        $shell.CreateShortcut($script:shortcutPath).Description | Should -Be 'Original shortcut'
    }

    It 'preserves a disabled startup entry and an absent user shortcut' {
        Remove-ItemProperty -Path $script:testRegistry -Name Ditto
        Remove-Item -LiteralPath $script:shortcutPath -Force
        Mock Get-CimInstance { [pscustomobject]@{ ExecutablePath = (Join-Path $env:LOCALAPPDATA 'Programs\Ditto-UIA-Compat\Ditto.exe') } }
        & "$PSScriptRoot\..\scripts\install-uia-compat.ps1" -StartupRegistryPath $script:testRegistry
        (Get-Item -Path $script:testRegistry).GetValueNames() | Should -Not -Contain 'Ditto'
        Mock Get-CimInstance { }
        & "$PSScriptRoot\..\scripts\rollback-uia-compat.ps1" -StartupRegistryPath $script:testRegistry
        (Get-Item -Path $script:testRegistry).GetValueNames() | Should -Not -Contain 'Ditto'
        Test-Path -LiteralPath $script:shortcutPath | Should -BeFalse
    }

    It 'preserves startup and shortcut changes made after installation' {
        Mock Get-CimInstance { [pscustomobject]@{ ExecutablePath = (Join-Path $env:LOCALAPPDATA 'Programs\Ditto-UIA-Compat\Ditto.exe') } }
        & "$PSScriptRoot\..\scripts\install-uia-compat.ps1" -StartupRegistryPath $script:testRegistry
        $other = 'C:\Different\Ditto.exe'
        Set-ItemProperty -Path $script:testRegistry -Name Ditto -Value $other
        $shell = New-Object -ComObject WScript.Shell
        $link = $shell.CreateShortcut($script:shortcutPath); $link.TargetPath = $other; $link.Save()
        Mock Get-CimInstance { }
        & "$PSScriptRoot\..\scripts\rollback-uia-compat.ps1" -StartupRegistryPath $script:testRegistry
        (Get-ItemProperty -Path $script:testRegistry).Ditto | Should -Be $other
        $shell.CreateShortcut($script:shortcutPath).TargetPath | Should -Be $other
    }

    It 'restores settings when the compatibility process fails to start' {
        Mock Get-CimInstance { }
        { & "$PSScriptRoot\..\scripts\install-uia-compat.ps1" -StartupRegistryPath $script:testRegistry } | Should -Throw '*did not stay running*'
        $key = Get-Item -Path $script:testRegistry
        $key.GetValue('Ditto', $null, [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames) | Should -Be $script:originalExe
        $key.GetValueKind('Ditto').ToString() | Should -Be 'ExpandString'
        $shell = New-Object -ComObject WScript.Shell
        $shell.CreateShortcut($script:shortcutPath).TargetPath | Should -Be $script:originalExe
    }
}
