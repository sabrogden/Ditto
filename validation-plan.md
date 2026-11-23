# Validation Plan — Preserve custom clip title when editing content by default (+ double-click tab rename)

**Revision:** 2 — adds validation for the new double-click-to-rename feature on the Edit window tab strip. Revision 1 covered only the title-preservation default.
**Upstream issue:** sabrogden/Ditto#1167
**Branch:** `havra/preserve-custom-clip-title`
**Changes:**
- `src/Options.cpp` — default of `CGetSetOptions::GetUpdateDescWhenSavingClip()` changed from `TRUE` to `FALSE` (registry key `UpdateDescWhenSaving`). Users who never opt in no longer lose a manually set description/title when editing a clip's content. The EditWnd checkbox "Update clip description on save?" still lets users opt in to regeneration per-edit and persists their choice. *(rev 1, unchanged)*
- `src/TabCtrl.h/.cpp`, `src/EditWnd.h/.cpp` — double-clicking a tab's title in the Edit window opens a rename prompt (reuses the existing `CGroupName` one-field dialog). On confirm, the new title is persisted to the DB (`Main.mText` via `CClip::ModifyDescription()`), the tab title updates immediately, and the Quick Paste list is refreshed. New (unsaved) clips only get their tab title changed locally. *(new in rev 2)*

## Observable checks

| # | Check | User actions | Expected result | Evidence |
|---|-------|--------------|-----------------|----------|
| 1 | **Baseline: app launches** | Launch the built `CP_Main.exe` | Ditto tray icon appears; Quick Paste window opens on hotkey without crash | Screenshot of Quick Paste window open |
| 2 | **Baseline: string capture appears in Quick Paste UI** | Copy a distinctive text string (e.g. `DITTO-VAL-2026`) from Notepad; open Quick Paste | New entry at top of list whose description starts with the copied text | Screenshot of list entry |
| 3 | **Baseline: image capture appears in Quick Paste UI** | Copy an image (e.g. via Snipping Tool or PrintScreen); open Quick Paste | New thumbnail entry appears at top of list | Screenshot of thumbnail |
| 4 | **Change-specific (rev 1): custom title preserved on edit (default path)** | a) Ensure no prior `UpdateDescWhenSaving` registry value exists (fresh/never-touched install state). b) Copy text `Alpha original content`. c) Set its description/title to a custom string (e.g. via clip properties/Edit, set description to `CUSTOM-TITLE-XYZ`). d) Open the clip in Edit (content editing), change the text to `Beta edited content`, save. e) Reopen Quick Paste / view entry | Description/title still reads `CUSTOM-TITLE-XYZ`; it is NOT overwritten with the start of `Beta edited content` | Screenshot of the entry showing preserved custom title after edit+save |
| 5 | **Change-specific (rev 1): checkbox still regenerates description when checked** | a) Open the same clip in Edit. b) Confirm the "Update clip description on save?" checkbox is checked. c) Change content to `Gamma regenerated`, save | Description updates to the start of `Gamma regenerated` (opt-in regeneration still works) | Screenshot of entry with regenerated description |
| 6 | **Change-specific (rev 1): option persistence round-trip** | In the Edit window, uncheck "Update clip description on save?", save an edit | Choice persists: description stays custom after save; reopening Edit shows the checkbox state matching the saved setting | Screenshot of Edit window checkbox + resulting entry |
| 7 | **New in rev 2: double-click tab title opens rename UI, persists, and refreshes Quick Paste** | a) Copy text `Rename source content` and open the clip in Edit. b) Double-click the clip's title in the Edit window's tab strip. c) A rename prompt appears pre-filled with the current title. d) Replace the text with `NEW-TITLE-ABC` and confirm (OK). e) Observe the tab strip, the Quick Paste list, then close the editor and reopen the clip in Edit (and close/reopen Quick Paste) | c) Rename prompt appears. d) After confirm: tab title updates to `NEW-TITLE-ABC` immediately; the Quick Paste list entry shows `NEW-TITLE-ABC`; the new title persists after closing and reopening the editor (DB-backed, survives re-open) | Screenshots: (i) rename prompt open with pre-filled title, (ii) tab strip showing `NEW-TITLE-ABC` after confirm, (iii) Quick Paste list showing `NEW-TITLE-ABC`, (iv) editor reopened still showing `NEW-TITLE-ABC` |
| 8 | **New in rev 2: rename combines with title-preservation** | Continuing from check 7: with the "Update clip description on save?" checkbox unchecked, edit the clip's content (e.g. to `Post-rename edited content`) and save | Content saves; the title remains `NEW-TITLE-ABC` (edit+save does not overwrite the rename) | Screenshot of Quick Paste entry after edit+save still showing `NEW-TITLE-ABC` |

## Notes / pass criteria

- Checks 1–3 must pass before checks 4–8 are meaningful (baseline health).
- Check 4 is the core regression check for issue #1167. Pass requires the description to be byte-identical to the custom string the user set.
- Check 5 guards against breaking the existing opt-in behavior that the task explicitly requires to keep working.
- Check 7 is the core new-behavior check for revision 2. Cancel path is implicitly covered: pressing Cancel in the rename prompt must leave title/tab/list unchanged (spot-check if time allows).
- Check 8 ties the two changes together: a renamed title must survive a subsequent content edit under the new default (checkbox off).
- All evidence: screenshots captured by the validator subagent into `validation/` alongside `validation-results.json` and `validation-report.html`.
