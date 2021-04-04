
Write-Host "Stopping running instances of Ditto"

# Kill ditto before updating
Get-Process | Where { $_.name -eq 'ditto' } | Where-Object {$_.Path -like "*chocolatey*"} | Stop-Process

Write-Host "Removing app data settings and database files"

#remove the app data where the portable choco app was written, settings and database
Remove-Item -Recurse -Force $env:LOCALAPPDATA\Ditto_ChocolateyApp