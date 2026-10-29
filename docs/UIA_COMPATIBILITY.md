# Large history UI Automation compatibility

On Windows, an accessibility client can cause the standard `SysListView32`
proxy to traverse every row of Ditto's owner-data history list. With a very
large history, the UI thread can become unresponsive. The compatibility
provider exposes the visible viewport to UI Automation, supports scrolling,
selection and invocation, and leaves the actual list and database intact.
Previously returned row elements reject actions if the clip ID has changed.

The visible UI Automation subtree changes as the list scrolls. Offscreen
selected items can be realized through the VirtualizedItem pattern. Clients
that assume the entire history is present in the UI Automation tree must scroll
the list to inspect more rows.

## Build and test

Build `CP_Main_10.sln` as `Release|x64` with Visual Studio C++ and MFC.
Restore `packages.config` first. The test project uses an invisible synthetic
216,025-row owner-data list and a real UI Automation client:

```powershell
msbuild tests/VisibleListProviderTests.vcxproj /p:Configuration=Debug /p:Platform=x64
& tests/x64/Debug/VisibleListProviderTests.exe
pwsh -NoLogo -NoProfile -NonInteractive -File tests/run-install-tests.ps1
```

## Local compatibility installation

`scripts/install-uia-compat.ps1` expects the release build at
`Release64/Ditto.exe` and the official 3.25.113.0 installation at
`%ProgramFiles%\Ditto`. It leaves the official installation and clipboard
database in place, stages a per-user copy, updates the user's existing Ditto
startup entry, and adds a per-user Start menu shortcut. It records the prior
startup value and shortcut for rollback.

`scripts/rollback-uia-compat.ps1` restores the recorded startup entry and
shortcut when they still point to this compatibility build, then starts the
original installation. It retains the compatibility directory and does not
alter the database. The compatibility binary is locally built and unsigned;
it does not receive updates to the official installation automatically.
