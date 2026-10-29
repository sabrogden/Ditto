$result = Invoke-Pester -Path (Join-Path $PSScriptRoot 'InstallCompat.Tests.ps1') -Output Detailed -PassThru
if ($result.FailedCount -gt 0 -or $result.FailedContainersCount -gt 0) { exit 1 }
