$ErrorActionPreference = "Stop"

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path

function Assert-SourcePattern {
    param(
        [Parameter(Mandatory = $true)]
        [string] $RelativePath,

        [Parameter(Mandatory = $true)]
        [string] $Pattern,

        [Parameter(Mandatory = $true)]
        [string] $FailureMessage
    )

    $path = Join-Path $repositoryRoot $RelativePath
    $text = Get-Content -LiteralPath $path -Raw
    if ($text -notmatch $Pattern) {
        throw "$FailureMessage ($RelativePath)"
    }
}

function Assert-SourceNotPattern {
    param(
        [Parameter(Mandatory = $true)]
        [string] $RelativePath,

        [Parameter(Mandatory = $true)]
        [string] $Pattern,

        [Parameter(Mandatory = $true)]
        [string] $FailureMessage
    )

    $path = Join-Path $repositoryRoot $RelativePath
    $text = Get-Content -LiteralPath $path -Raw
    if ($text -match $Pattern) {
        throw "$FailureMessage ($RelativePath)"
    }
}

# Stable enum values: new commands must remain appended immediately before LAST_ACTION.
Assert-SourcePattern "src\ActionEnums.h" '(?s)SHOW_STARRED_CLIPS,\s*RENAME_CLIP,\s*ASSIGN_CLIP_HOTKEY,\s*LAST_ACTION' "Clip commands are not appended to the action enum"

$actionEnumText = Get-Content -LiteralPath (Join-Path $repositoryRoot "src\ActionEnums.h") -Raw
$actionEnumBody = [regex]::Match($actionEnumText, '(?s)enum ActionEnumValues\s*\{(?<body>.*?)LAST_ACTION').Groups['body'].Value
$actionNames = @([regex]::Matches($actionEnumBody, '(?m)^\s*([A-Z][A-Z0-9_]*),?\s*(?://.*)?$') | ForEach-Object { $_.Groups[1].Value })
$renameActionId = [Array]::IndexOf($actionNames, "RENAME_CLIP")
$assignHotKeyActionId = [Array]::IndexOf($actionNames, "ASSIGN_CLIP_HOTKEY")
if ($renameActionId -lt 0 -or $assignHotKeyActionId -lt 0) {
    throw "Clip command action IDs could not be derived from ActionEnums.h"
}

# User-facing command names and their default local shortcuts.
Assert-SourcePattern "src\ActionEnums.cpp" '(?s)case RENAME_CLIP:\s*val = "Rename Clip";' "Rename Clip has no user-facing action name"
Assert-SourcePattern "src\ActionEnums.cpp" '(?s)case ASSIGN_CLIP_HOTKEY:\s*val = "Assign Clip Hot Key";' "Assign Clip Hot Key has no user-facing action name"
Assert-SourcePattern "src\ActionEnums.cpp" '(?s)case ActionEnums::RENAME_CLIP:\s*return VK_F2;' "Rename Clip does not default to F2"
Assert-SourcePattern "src\ActionEnums.cpp" "(?s)case ActionEnums::ASSIGN_CLIP_HOTKEY:\s*return ACCEL_MAKEKEY\('S', HOTKEYF_CONTROL \| HOTKEYF_SHIFT\);" "Assign Clip Hot Key does not default to Ctrl+Shift+S"

# Keyboard and context-menu routing must use the dedicated operations.
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)case ActionEnums::RENAME_CLIP:\s*ret = DoActionRenameClip\(\);' "Rename Clip is not routed to its dedicated operation"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)case ActionEnums::ASSIGN_CLIP_HOTKEY:\s*ret = DoActionAssignClipHotKey\(\);' "Assign Clip Hot Key is not routed to its dedicated operation"
Assert-SourceNotPattern "src\QPasteWnd.cpp" '(?s)case ActionEnums::(?:RENAME_CLIP|ASSIGN_CLIP_HOTKEY):\s*ret = DoActionClipProperties' "A dedicated clip command still opens the full Properties dialog"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)ON_COMMAND\(ID_MENU_RENAME_CLIP, OnMenuRenameClip\).*ON_COMMAND\(ID_MENU_ASSIGN_CLIP_HOTKEY, OnMenuAssignClipHotKey\)' "Clip commands are missing menu routing"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)OnUpdateMenuRenameClip.*UpdateMenuShortCut\(pCmdUI, ActionEnums::RENAME_CLIP\);' "Rename Clip menu does not expose its configured shortcut"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)OnUpdateMenuAssignClipHotKey.*UpdateMenuShortCut\(pCmdUI, ActionEnums::ASSIGN_CLIP_HOTKEY\);' "Assign Clip Hot Key menu does not expose its configured shortcut"

# Both commands must be visible with intentional labels in the menu and keyboard settings.
Assert-SourcePattern "CP_Main.rc" '(?s)"Rename Clip",\s+ID_MENU_RENAME_CLIP.*"Assign Clip Hot Key",\s+ID_MENU_ASSIGN_CLIP_HOTKEY' "Clip commands are missing from the Quick Paste menu"
Assert-SourcePattern "Debug\Language\English.xml" "<Item English_Text = `"Rename Clip`" ID = `"$renameActionId`">" "Rename Clip is missing from Quick Paste keyboard translations"
Assert-SourcePattern "Debug\Language\English.xml" "<Item English_Text = `"Assign Clip Hot Key`" ID = `"$assignHotKeyActionId`">" "Assign Clip Hot Key is missing from Quick Paste keyboard translations"
Assert-SourcePattern "Debug\Language\English.xml" '<Item English_Text = "Rename Clip" ID = "33003">' "Rename Clip is missing from context-menu translations"
Assert-SourcePattern "Debug\Language\English.xml" '<Item English_Text = "Assign Clip Hot Key" ID = "33004">' "Assign Clip Hot Key is missing from context-menu translations"

# Each operation has a small, purpose-specific dialog with an intentional label.
$resourceText = Get-Content -LiteralPath (Join-Path $repositoryRoot "CP_Main.rc") -Raw
$renameDialog = [regex]::Match($resourceText, '(?s)IDD_RENAME_CLIP DIALOGEX.*?\r?\nEND').Value
$hotKeyDialog = [regex]::Match($resourceText, '(?s)IDD_ASSIGN_CLIP_HOTKEY DIALOGEX.*?\r?\nEND').Value
if ([string]::IsNullOrWhiteSpace($renameDialog) -or $renameDialog -notmatch 'IDC_STATIC_CLIP_NAME' -or $renameDialog -notmatch 'IDC_CLIP_NAME') {
    throw "Rename Clip does not have its dedicated labelled edit dialog (CP_Main.rc)"
}
if ($renameDialog -notmatch 'DEFPUSHBUTTON\s+"OK",IDOK' -or $renameDialog -notmatch 'IDCANCEL') {
    throw "Rename Clip does not expose standard Enter and Escape dialog actions (CP_Main.rc)"
}
if ($renameDialog -match 'IDC_CLIP_HOTKEY|IDC_EDIT_PROPERTIES') {
    throw "Rename Clip dialog exposes unrelated properties (CP_Main.rc)"
}
if ([string]::IsNullOrWhiteSpace($hotKeyDialog) -or $hotKeyDialog -notmatch 'IDC_STATIC_CLIP_HOTKEY' -or $hotKeyDialog -notmatch 'IDC_CLIP_HOTKEY') {
    throw "Assign Clip Hot Key does not have its dedicated labelled hot-key dialog (CP_Main.rc)"
}
if ($hotKeyDialog -notmatch 'DEFPUSHBUTTON\s+"OK",IDOK' -or $hotKeyDialog -notmatch 'IDCANCEL') {
    throw "Assign Clip Hot Key does not expose standard Enter and Escape dialog actions (CP_Main.rc)"
}
if ($hotKeyDialog -match 'IDC_CLIP_NAME|IDC_EDIT_PROPERTIES') {
    throw "Assign Clip Hot Key dialog exposes unrelated properties (CP_Main.rc)"
}

# Initial focus, cancellation and return focus are part of the keyboard and screen-reader contract.
Assert-SourcePattern "src\ClipEditDialogs.cpp" '(?s)CClipNameDialog::OnInitDialog.*m_nameEdit.SetFocus\(\);\s*m_nameEdit.SetSel\(0, -1\);' "Rename Clip does not focus and select the clip name"
Assert-SourcePattern "src\ClipEditDialogs.cpp" '(?s)CClipHotKeyDialog::OnInitDialog.*m_hotKey.SetFocus\(\);' "Assign Clip Hot Key does not initially focus the shortcut field"
Assert-SourcePattern "src\ClipEditDialogs.cpp" 'ClipHotKeyGlobal", "Use this shortcut from any application"' "Global shortcut choice has no user-facing explanation"
Assert-SourcePattern "src\ClipEditDialogs.cpp" '(?s)OnRemoveShortcut.*m_hotKey.SetHotKey\(0, 0\);' "The dedicated shortcut dialog cannot remove an existing shortcut"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)DoActionRenameClip\(\).*m_lstHeader.SetFocus\(\);.*DoActionAssignClipHotKey\(\).*m_lstHeader.SetFocus\(\);' "Clip commands do not restore focus to the clip list"
Assert-SourcePattern "CP_Main.vcxproj" 'ClCompile Include="src\\ClipEditDialogs\.cpp"' "The dedicated dialog implementation is not part of the build"
Assert-SourcePattern "CP_Main.vcxproj" 'ClInclude Include="src\\ClipEditDialogs\.h"' "The dedicated dialog header is not part of the build"

# Persist only the requested field and keep local/global shortcut caches synchronized.
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)DoActionRenameClip\(\).*clip.ModifyDescription\(\)' "Rename Clip does not save only the clip description"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)SaveClipHotKey\(.*UPDATE Main SET.*lShortCut = CASE.*globalShortCut = CASE' "Assign Clip Hot Key does not use a targeted shortcut update"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)SaveClipHotKey\(.*g_HotKeys.Remove\(conflictingIds\[index\].*g_HotKeys.Remove\(clip.m_id.*g_HotKeys.ValidateClip' "Runtime global shortcut registrations are not synchronized after assignment"
Assert-SourcePattern "src\QPasteWnd.cpp" '(?s)DoActionAssignClipHotKey\(\).*RefreshClipRow\(id, row, true\);' "Shortcut assignment does not reload the local accelerator map"

Write-Host "Dedicated clip command parity and accessibility checks passed."
