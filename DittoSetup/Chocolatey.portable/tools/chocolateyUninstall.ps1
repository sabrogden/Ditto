$ErrorActionPreference = 'Stop'

# Kill ditto before updating
Get-Process | Where { $_.name -eq 'ditto' } | Where-Object {$_.Path -like "*chocolatey*"} | Stop-Process
 
$toolsPath = Split-Path -Parent $MyInvocation.MyCommand.Definition
$fileName  = 'DittoPortable__%versionFileName%.zip'

Write-Host "Removing old Ditto.Portable files"
Remove-Item $toolsPath\* -Recurse -Force -Exclude $fileName

$zip_path = "$toolsPath\$fileName"
 
$packageArgs = @{
    PackageName  = 'ditto.portable'
    FileFullPath = $zip_path
    Destination  = $toolsPath
}

Write-Host "Installing Ditto.Portable"
Get-ChocolateyUnzip @packageArgs

#tell ditto this is a chocolatey portable app, will use a the directory in AppData/Local/Ditto_ChocolateyApp to store it's data
Rename-Item -Path "$(Join-Path "$toolsPath" "Ditto\portable")" -NewName "chocolatey"

Remove-Item $toolsPath\*.zip -ea 0