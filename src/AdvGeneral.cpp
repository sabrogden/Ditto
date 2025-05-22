// AdvGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "AdvGeneral.h"
#include "afxdialogex.h"
#include "ScriptEditor.h"
#include "DimWnd.h"


// CAdvGeneral dialog

IMPLEMENT_DYNAMIC(CAdvGeneral, CDialogEx)

CAdvGeneral::CAdvGeneral(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAdvGeneral::IDD, pParent)
{

}

CAdvGeneral::~CAdvGeneral()
{
}

void CAdvGeneral::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID1, m_propertyGrid);
	DDX_Control(pDX, IDC_EDIT_ADV_FILTER, m_editFilter);
}


BEGIN_MESSAGE_MAP(CAdvGeneral, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAdvGeneral::OnBnClickedOk)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BT_COMPACT_AND_REPAIR, &CAdvGeneral::OnBnClickedBtCompactAndRepair)
	ON_BN_CLICKED(IDC_BUTTON_COPY_SCRIPTS, &CAdvGeneral::OnBnClickedButtonCopyScripts)
	ON_BN_CLICKED(IDC_BUTTON_PASTE_SCRIPTS, &CAdvGeneral::OnBnClickedButtonPasteScripts2)
	ON_WM_GETMINMAXINFO()
	ON_WM_NCLBUTTONDOWN()
	ON_EN_CHANGE(IDC_EDIT_ADV_FILTER, &CAdvGeneral::OnEnChangeAdvFilter)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_MATCH, &CAdvGeneral::OnBnClickedButtonNextMatch)
END_MESSAGE_MAP()


// CAdvGeneral message handlers

#define SETTING_DESC_SIZE 1
#define SETTING_SHOW_TASKBAR_ICON 2
#define SETTING_SAVE_MULTI_PASTE 3
#define SETTING_HIDE_ON_HOTKEY_IF_VISIBLE 4
#define SETTING_PASTE_IN_ACTIVE_WINDOW 5
#define SETTING_MAX_CLIP_SIZE 6
#define SETTING_CLIP_SEPARATOR 7
#define SETTING_ENSURE_CONNECTED 8
#define SETTING_COPY_PLAY_SOUND 9
#define SETTING_TEXT_FIRST_TEN 10 
#define SETTING_SHOW_LEADING_WHITESPACE 11
#define SETTING_LINES_PER_ROW 12
#define SETTING_ENABLE_TRANSPARENCY 13
#define SETTING_DRAW_THUMBNAILS 14
#define SETTING_DRAW_RTF 15
#define SETTING_FIND_AS_TYPE 16
#define SETTING_ENSURE_WINDOW_IS_VISIBLE 17
#define SETTING_SHOW_GROUP_CLIPS_IN_LIST 18
#define SETTING_PROMPT_ON_DELETE 19
#define SETTING_ALWAYS_SHOW_SCROLL_BAR 20
#define SETTING_PASTE_AS_ADMIN 21
#define SETTTING_SHOW_IN_TASKBAR 22
#define SETTING_SHOW_CLIP_PASTED 23
#define SETTING_DIFF_APP 24
#define SETTING_TRANSPARENCY 25
#define SETTING_UPDATE_ORDER_ON_PASTE 26
#define SETTING_ALLOW_DUPLICATES 27
#define SETTING_REGEX_FILTERING_1 28
#define SETTING_REGEX_FILTERING_2 29
#define SETTING_REGEX_FILTERING_3 30
#define SETTING_REGEX_FILTERING_4 31
#define SETTING_REGEX_FILTERING_5 32
#define SETTING_REGEX_FILTERING_6 33
#define SETTING_REGEX_FILTERING_7 34
#define SETTING_REGEX_FILTERING_8 35
#define SETTING_REGEX_FILTERING_9 36
#define SETTING_REGEX_FILTERING_10 37
#define SETTING_REGEX_FILTERING_11 38
#define SETTING_REGEX_FILTERING_12 39
#define SETTING_REGEX_FILTERING_13 40
#define SETTING_REGEX_FILTERING_14 41
#define SETTING_REGEX_FILTERING_15 42

#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_1 43
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_2 44
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_3 45
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_4 46
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_5 47
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_6 48
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_7 49
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_8 50
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_9 51
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_10 52
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_11 53
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_12 54
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_13 55
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_14 56
#define SETTING_REGEX_FILTERING_BY_PROCESS_NAME_15 57

#define SETTING_SHOW_STARTUP_MESSAGE 58
#define SETTING_TOOLTIP_TIMEOUT 59
#define SETTING_SELECTED_INDEX 60
#define SETTING_CLIPBOARD_SAVE_DELAY 61
#define SETTING_SHOW_MSG_WHEN_RECEIVING_MANUAL_SENT_CLIP 62
#define SETTING_MULTIPASTE_REVERSE_ORDER 63
#define SETTING_DEFAULT_PASTE_STRING 64
#define SETTING_DEFAULT_COPY_STRING 65
#define SETTING_DEFAULT_CUT_STRING 66
#define SETTING_REVERT_TO_TOP_LEVEL_GROUP 67
#define SETTING_UPDATE_ORDER_ON_CTRL_C 68

#define SETTING_TOOLTIP_LINES 69
#define SETTING_TOOLTIP_CHARACTERS 70


#define SETTING_ACTIVATE_WINDOW_DELAY 71
#define SETTING_DOUBLE_KEYSTROKE_TIMEOUT 72
#define SETTING_SEND_KEYS_DELAY 73
#define SETTING_FIRST_TEN_HOTKEYS_START 74
#define SETTING_FIRST_TEN_HOTKEYS_FONT_SIZE 75

#define SETTING_OPEN_TO_GROUP_AS_ACTIVE_EXE 76
#define SETTING_ADD_CF_HDROP_ON_DRAG 77
#define SETTING_COPY_SAVE_DELAY 78
#define SETTING_EDITOR_FONT_SIZE 79
#define SETTING_MOVE_SELECTION_ON_OPEN_HOTKEY 80
#define SETTING_ALOW_BACK_TO_BACK_DUPLICATES 81
#define SETTING_MAINTAIN_SEARCH_VIEW 82
#define SETTING_SEND_RECV_PORT 83
#define SETTING_DEBUG_TO_FILE 84
#define SETTING_DEBUG_TO_OUTPUT_STRING 85
#define SETTING_NETWORK_BIND_IP_ADDRESS 86
#define SETTING_DISABLE_FRIENDS 87
#define SETTING_IGNORE_FALSE_COPIES_DELAY 88
#define SETTING_REFRESH_VIEW_AFTER_PASTE 89
#define SETTING_SLUGIFY_SEPARATOR 90
#define SETTING_FAST_THUMBNAIL_MODE 91
#define SETTING_CLIPBOARD_RESTORE_AFTER_COPY_BUFFER_DELAY 92
#define SETTING_SUPPORT_ALL_TYPES 93
#define SETTING_IGNORE_ANNOYING_CF_DIB 94
#define SETTING_REGEX_CASE_INSENSITIVE 95
#define SETTING_DRAW_COPIED_COLOR_CODE 96
#define SETTING_CENTER_WINDOW_BELOW_CURSOR_CARET 97
#define SETTING_TEXT_EDITOR_PATH 98
#define SETTING_RTF_EDITOR_PATH 99
#define SETTING_UPDATE_DESC_ON_CLIP_EDIT 100
#define SETTING_QR_CODE_URL 101
#define SETTING_APPEND_NAME_IP 102
#define SETTING_USE_UTF8_FOR_DIFF 103
#define SETTING_IMAGE_EDITOR_PATH 104
#define SETTING_CLIP_EDIT_SAVE_DELAY_AFTER_LOAD 105
#define SETTING_ClIP_EDIT_SAVE_DELAY_AFTER_SAVE 106

BOOL CAdvGeneral::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_propertyGrid.ModifyStyle(0, WS_CLIPCHILDREN);

	HICON b = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 64, 64, LR_SHARED);
	SetIcon(b, TRUE);

	CMFCPropertyGridProperty * pGroupTest = new CMFCPropertyGridProperty( _T( "Ditto" ) );
	m_propertyGrid.AddProperty(pGroupTest);

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_MFCPROPERTYGRID1, DR_SizeWidth | DR_SizeHeight);
	m_Resize.AddControl(IDOK, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDC_BT_COMPACT_AND_REPAIR, DR_MoveTop);
	m_Resize.AddControl(IDC_BUTTON_COPY_SCRIPTS, DR_MoveTop);
	m_Resize.AddControl(IDC_BUTTON_PASTE_SCRIPTS, DR_MoveTop);
	m_Resize.AddControl(IDC_EDIT_ADV_FILTER, DR_SizeWidth);
	m_Resize.AddControl(IDC_BUTTON_NEXT_MATCH, DR_MoveLeft);

	HDITEM hdItem;
	hdItem.mask = HDI_WIDTH; // indicating cxy is width
	CDPI dpi(m_hWnd);
	hdItem.cxy = dpi.Scale(400); // whatever you want the property name column width to be
	m_propertyGrid.GetHeaderCtrl().SetItem(0, &hdItem);

	m_propertyGrid.SetFont(this->GetFont());	

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Activate window delay (100ms default)"), (long)CGetSetOptions::SendKeysDelay(), _T(""), SETTING_ACTIVATE_WINDOW_DELAY));

	AddTrueFalse(pGroupTest, _T("Add file drop when dragging clips"), CGetSetOptions::GetAddCFHDROP_OnDrag(), SETTING_ADD_CF_HDROP_ON_DRAG);

	AddTrueFalse(pGroupTest, _T("Allow duplicates"), CGetSetOptions::GetAllowDuplicates(), SETTING_ALLOW_DUPLICATES);
	AddTrueFalse(pGroupTest, _T("Allow back to back duplicates (if allowing duplicates)"), CGetSetOptions::GetAllowBackToBackDuplicates(), SETTING_ALOW_BACK_TO_BACK_DUPLICATES);

	AddTrueFalse(pGroupTest, _T("Always show scroll bar"), CGetSetOptions::GetShowScrollBar(), SETTING_ALWAYS_SHOW_SCROLL_BAR);
	AddTrueFalse(pGroupTest, _T("Append Computer Name and IP when receiving clips"), CGetSetOptions::GetAppendRemoveComputerNameAndIPToDescription(), SETTING_APPEND_NAME_IP);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Amount of text to save for description"), CGetSetOptions::m_bDescTextSize, _T(""), SETTING_DESC_SIZE));
	AddTrueFalse(pGroupTest, _T("Center window below cursor or caret"), CGetSetOptions::GetCenterWindowBelowCursorOrCaret(), SETTING_CENTER_WINDOW_BELOW_CURSOR_CARET);
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Copy and save clipboard delay (ms)"), (long)CGetSetOptions::GetCopyAndSveDelay(), _T(""), SETTING_COPY_SAVE_DELAY));

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Clip edit save delay after load"), (long)(CGetSetOptions::GetClipEditSaveDelayAfterLoadSeconds()), _T(""), SETTING_CLIP_EDIT_SAVE_DELAY_AFTER_LOAD));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Clip edit save delay after Save"), (long)(CGetSetOptions::GetClipEditSaveDelayAfterSaveSeconds()), _T(""), SETTING_ClIP_EDIT_SAVE_DELAY_AFTER_SAVE));

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Clipboard restore delay after copy buffer sent paste (ms, default: 750)"), (long)(CGetSetOptions::GetDittoRestoreClipboardDelay()), _T(""), SETTING_CLIPBOARD_RESTORE_AFTER_COPY_BUFFER_DELAY));

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Default paste string"), CGetSetOptions::GetDefaultPasteString(), _T(""), SETTING_DEFAULT_PASTE_STRING));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Default copy string"), CGetSetOptions::GetDefaultCopyString(), _T(""), SETTING_DEFAULT_COPY_STRING));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Default cut string"), CGetSetOptions::GetDefaultCutString(), _T(""), SETTING_DEFAULT_CUT_STRING));
	
	static TCHAR BASED_CODE szDiffFilter[] = _T("Diff Applications(*.exe)|*.exe||");
	CMFCPropertyGridFileProperty* pDiffProp = new CMFCPropertyGridFileProperty(_T("Diff application path"), TRUE, CGetSetOptions::GetDiffApp(), _T("exe"), 0, szDiffFilter, (LPCTSTR)0, SETTING_DIFF_APP);
	pGroupTest->AddSubItem(pDiffProp);

	AddTrueFalse(pGroupTest, _T("Diff save compare files as utf8"), CGetSetOptions::GetPreferUtf8ForCompare(), SETTING_USE_UTF8_FOR_DIFF);

	AddTrueFalse(pGroupTest, _T("Disable friends"), !CGetSetOptions::GetAllowFriends(), SETTING_DISABLE_FRIENDS);

	AddTrueFalse(pGroupTest, _T("Display icon in system tray"), CGetSetOptions::GetShowIconInSysTray(), SETTING_SHOW_TASKBAR_ICON);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Double shortcut keystroke timeout)"), (long)CGetSetOptions::GetDoubleKeyStrokeTimeout(), _T(""), SETTING_DOUBLE_KEYSTROKE_TIMEOUT));

	AddTrueFalse(pGroupTest, _T("Draw swatch for hex, RGB, and HSL colors"), CGetSetOptions::GetDrawCopiedColorCode(), SETTING_DRAW_COPIED_COLOR_CODE);

	AddTrueFalse(pGroupTest, _T("Draw RTF text in list (for RTF types) (could increase memory usage an display speed)"), CGetSetOptions::GetDrawRTF(), SETTING_DRAW_RTF);
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Editor default font size"), (long)CGetSetOptions::GetEditorDefaultFontSize(), _T(""), SETTING_EDITOR_FONT_SIZE));
	AddTrueFalse(pGroupTest, _T("Elevated privileges to paste into elevated apps"), CGetSetOptions::GetPasteAsAdmin(), SETTING_PASTE_AS_ADMIN);
	AddTrueFalse(pGroupTest, _T("Ensure Ditto is always connected to the clipboard"), CGetSetOptions::GetEnsureConnectToClipboard(), SETTING_ENSURE_CONNECTED);
	AddTrueFalse(pGroupTest, _T("Ensure entire window is visible"), CGetSetOptions::GetEnsureEntireWindowCanBeSeen(), SETTING_ENSURE_WINDOW_IS_VISIBLE);

	AddTrueFalse(pGroupTest, _T("Fast thumbnails (True = fast / low quality (default). False = slow / high quality)"), CGetSetOptions::GetFastThumbnailMode(), SETTING_FAST_THUMBNAIL_MODE);

	AddTrueFalse(pGroupTest, _T("Find as you type"), CGetSetOptions::GetFindAsYouType(), SETTING_FIND_AS_TYPE);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("First ten hot keys start index"), (long)CGetSetOptions::GetFirstTenHotKeysStart(), _T(""), SETTING_FIRST_TEN_HOTKEYS_START));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("First ten hot keys font size"), (long)CGetSetOptions::GetFirstTenHotKeysFontSize(), _T(""), SETTING_FIRST_TEN_HOTKEYS_FONT_SIZE));

	AddTrueFalse(pGroupTest, _T("Hide Ditto on hot key if Ditto is visible"), CGetSetOptions::GetHideDittoOnHotKeyIfAlreadyShown(), SETTING_HIDE_ON_HOTKEY_IF_VISIBLE);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Ignore copies faster than (ms) (default: 500)"), (long)CGetSetOptions::GetSaveClipDelay(), _T(""), SETTING_IGNORE_FALSE_COPIES_DELAY));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Ignore CF_DIB when a clip is detected as text content"), CGetSetOptions::GetIgnoreAnnoyingCFDIB(), _T("Case insensitive. Recommended option is \"excel.exe; onenote.exe; powerpnt.exe\" "), SETTING_IGNORE_ANNOYING_CF_DIB));

	static TCHAR BASED_CODE szImageEditorFilter[] = _T("Applications(*.exe)|*.exe||");
	CMFCPropertyGridFileProperty* pImageEditorProp = new CMFCPropertyGridFileProperty(_T("Image editor path (empty for system mapping)"), TRUE, CGetSetOptions::GetImageEditorPath(), _T("exe"), 0, szImageEditorFilter, (LPCTSTR)0, SETTING_IMAGE_EDITOR_PATH);
	pGroupTest->AddSubItem(pImageEditorProp);

	pGroupTest->AddSubItem( new CMFCPropertyGridProperty(_T("Maximum clip size in bytes (0 for no limit)"), CGetSetOptions::m_lMaxClipSizeInBytes, _T(""), SETTING_MAX_CLIP_SIZE));
		
	AddTrueFalse(pGroupTest, _T("Maintain search view"), CGetSetOptions::GetMaintainSearchView(), SETTING_MAINTAIN_SEARCH_VIEW);

	AddTrueFalse(pGroupTest, _T("Move selection on open hot key"), CGetSetOptions::GetMoveSelectionOnOpenHotkey(), SETTING_MOVE_SELECTION_ON_OPEN_HOTKEY);
	
	pGroupTest->AddSubItem( new CMFCPropertyGridProperty(_T("Multi-paste clip separator ([LF] = line feed)"), CGetSetOptions::GetMultiPasteSeparator(false), _T(""), SETTING_CLIP_SEPARATOR));

	AddTrueFalse(pGroupTest, _T("Multi-paste in reverse order"), CGetSetOptions::m_bMultiPasteReverse, SETTING_MULTIPASTE_REVERSE_ORDER);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Network send receive port (default: 23443)"), (long)CGetSetOptions::GetPort(), _T(""), SETTING_SEND_RECV_PORT));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Network server bind IP (default: *)"), CGetSetOptions::GetNetworkBindIPAddress(), _T(""), SETTING_NETWORK_BIND_IP_ADDRESS));

	AddTrueFalse(pGroupTest, _T("Open to group same as active exe"), CGetSetOptions::GetOpenToGroupByActiveExe(), SETTING_OPEN_TO_GROUP_AS_ACTIVE_EXE);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("QRCode Url"), CGetSetOptions::GetQRCodeUrl(), _T(""), SETTING_QR_CODE_URL));

	static TCHAR BASED_CODE szFilter[] = _T("Sounds(*.wav)|*.wav||");
	CMFCPropertyGridFileProperty* pFileProp = new CMFCPropertyGridFileProperty(_T("On copy play the sound"), TRUE, CGetSetOptions::GetPlaySoundOnCopy(), _T("wav"), 0, szFilter, (LPCTSTR)0, SETTING_COPY_PLAY_SOUND);
	pGroupTest->AddSubItem(pFileProp);

	static TCHAR BASED_CODE szTextEditorFilter[] = _T("Applications(*.exe)|*.exe||");
	CMFCPropertyGridFileProperty* pTextEditorProp = new CMFCPropertyGridFileProperty(_T("Text editor path (empty for system mapping)"), TRUE, CGetSetOptions::GetTextEditorPath(), _T("exe"), 0, szTextEditorFilter, (LPCTSTR)0, SETTING_TEXT_EDITOR_PATH);
	pGroupTest->AddSubItem(pTextEditorProp);

	AddTrueFalse(pGroupTest, _T("Paste clip in active window after selection"), CGetSetOptions::GetSendPasteAfterSelection(), SETTING_PASTE_IN_ACTIVE_WINDOW);
	AddTrueFalse(pGroupTest, _T("Prompt when deleting clips"), CGetSetOptions::GetPromptWhenDeletingClips(), SETTING_PROMPT_ON_DELETE);

	AddTrueFalse(pGroupTest, _T("Revert to top level group on close"), CGetSetOptions::GetRevertToTopLevelGroup(), SETTING_REVERT_TO_TOP_LEVEL_GROUP);

	AddTrueFalse(pGroupTest, _T("Refresh view after paste"), CGetSetOptions::GetRefreshViewAfterPasting(), SETTING_REFRESH_VIEW_AFTER_PASTE);

	AddTrueFalse(pGroupTest, _T("Regex case insensitive search"), CGetSetOptions::GetRegexCaseInsensitive(), SETTING_REGEX_CASE_INSENSITIVE);

	static TCHAR BASED_CODE szRTFEditorFilter[] = _T("Applications(*.exe)|*.exe||");
	CMFCPropertyGridFileProperty* pRTFEditorProp = new CMFCPropertyGridFileProperty(_T("RTF editor path"), TRUE, CGetSetOptions::GetRTFEditorPath(), _T("exe"), 0, szRTFEditorFilter, (LPCTSTR)0, SETTING_RTF_EDITOR_PATH);
	pGroupTest->AddSubItem(pRTFEditorProp);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Save clipboard delay (ms, default: 100)"), (long)(CGetSetOptions::GetProcessDrawClipboardDelay()), _T(""), SETTING_CLIPBOARD_SAVE_DELAY));

	AddTrueFalse(pGroupTest, _T("Save multi-pastes"), CGetSetOptions::GetSaveMultiPaste(), SETTING_SAVE_MULTI_PASTE);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Selected index"), (long)(CGetSetOptions::SelectedIndex()+1), _T(""), SETTING_SELECTED_INDEX));

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Send keys delay (ms)"), (long)CGetSetOptions::RealSendKeysDelay(), _T(""), SETTING_SEND_KEYS_DELAY));


	AddTrueFalse(pGroupTest, _T("Show clips that are in groups in main list"), CGetSetOptions::GetShowAllClipsInMainList(), SETTING_SHOW_GROUP_CLIPS_IN_LIST);
	AddTrueFalse(pGroupTest, _T("Show leading whitespace"), CGetSetOptions::GetDescShowLeadingWhiteSpace(), SETTING_SHOW_LEADING_WHITESPACE);
	AddTrueFalse(pGroupTest, _T("Show in taskbar"), CGetSetOptions::GetShowInTaskBar(), SETTTING_SHOW_IN_TASKBAR);
	AddTrueFalse(pGroupTest, _T("Show indicator a clip has been pasted"), CGetSetOptions::GetShowIfClipWasPasted(), SETTING_SHOW_CLIP_PASTED);

	AddTrueFalse(pGroupTest, _T("Show message that we received a manual sent clip"), CGetSetOptions::GetShowMsgWhenReceivingManualSentClip(), SETTING_SHOW_MSG_WHEN_RECEIVING_MANUAL_SENT_CLIP);	

	AddTrueFalse(pGroupTest, _T("Show startup tooltip message"), CGetSetOptions::GetShowStartupMessage(), SETTING_SHOW_STARTUP_MESSAGE);

	AddTrueFalse(pGroupTest, _T("Show text for first ten copy hot keys"), CGetSetOptions::GetShowTextForFirstTenHotKeys(), SETTING_TEXT_FIRST_TEN);
	AddTrueFalse(pGroupTest, _T("Show thumbnails(for CF_DIB and PNG types) (could increase memory usage and display speed)"), CGetSetOptions::GetDrawThumbnail(), SETTING_DRAW_THUMBNAILS);
	
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Slugify Separator (default: -)"), CGetSetOptions::GetSlugifySeparator(), _T(""), SETTING_SLUGIFY_SEPARATOR));

	AddTrueFalse(pGroupTest, _T("Support all types ignoring supported type list (default: false))"), CGetSetOptions::GetSupportAllTypes(), SETTING_SUPPORT_ALL_TYPES);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Text lines per clip"), CGetSetOptions::GetLinesPerRow(), _T(""), SETTING_LINES_PER_ROW));

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Tooltip display time(ms) max of 32000 (-1 default (5 seconds), 0 to turn off)"), CGetSetOptions::m_tooltipTimeout, _T(""), SETTING_TOOLTIP_TIMEOUT));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Tooltip maximum display lines"), (long)CGetSetOptions::GetMaxToolTipLines(), _T(""), SETTING_TOOLTIP_LINES));
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Tooltip display characters"), (long)CGetSetOptions::GetMaxToolTipCharacters(), _T(""), SETTING_TOOLTIP_CHARACTERS));

	AddTrueFalse(pGroupTest, _T("Transparency enabled"), CGetSetOptions::GetEnableTransparency(), SETTING_ENABLE_TRANSPARENCY);
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Transparency percentage"), CGetSetOptions::GetTransparencyPercent(), _T(""), SETTING_TRANSPARENCY));
	AddTrueFalse(pGroupTest, _T("Update description on clip edit"), CGetSetOptions::GetUpdateDescWhenSavingClip(), SETTING_UPDATE_DESC_ON_CLIP_EDIT);
	AddTrueFalse(pGroupTest, _T("Update clip order on paste"), CGetSetOptions::GetUpdateTimeOnPaste(), SETTING_UPDATE_ORDER_ON_PASTE);
	AddTrueFalse(pGroupTest, _T("Update clip Order on ctrl-c"), CGetSetOptions::GetUpdateClipOrderOnCtrlC(), SETTING_UPDATE_ORDER_ON_CTRL_C);

	AddTrueFalse(pGroupTest, _T("Write debug to file"), CGetSetOptions::GetEnableDebugLogging(), SETTING_DEBUG_TO_FILE);
	AddTrueFalse(pGroupTest, _T("Write debug to OutputDebugString"), CGetSetOptions::GetEnableDebugLogging(), SETTING_DEBUG_TO_OUTPUT_STRING);

	CMFCPropertyGridProperty * regexFilterGroup = new CMFCPropertyGridProperty(_T("Exclude clips by Regular Expressions"));
	m_propertyGrid.AddProperty(regexFilterGroup);

	CString processFilterDesc = _T("Process making the copy first must match this before the Regex will be applied (empty or * for all processes) (separate multiples by ;)");
	CString regexFilterDesc = _T("If copied text matches this regular expression then the clip will not be saved to Ditto");

	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("1 Regex"), CGetSetOptions::GetRegexFilter(0), regexFilterDesc, SETTING_REGEX_FILTERING_1));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("1 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(0), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_1));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("2 Regex"), CGetSetOptions::GetRegexFilter(1), regexFilterDesc, SETTING_REGEX_FILTERING_2));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("2 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(1), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_2));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("3 Regex"), CGetSetOptions::GetRegexFilter(2), regexFilterDesc, SETTING_REGEX_FILTERING_3));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("3 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(2), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_3));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("4 Regex"), CGetSetOptions::GetRegexFilter(3), regexFilterDesc, SETTING_REGEX_FILTERING_4));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("4 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(3), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_4));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("5 Regex"), CGetSetOptions::GetRegexFilter(4), regexFilterDesc, SETTING_REGEX_FILTERING_5));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("5 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(4), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_5));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("6 Regex"), CGetSetOptions::GetRegexFilter(5), regexFilterDesc, SETTING_REGEX_FILTERING_6));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("6 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(5), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_6));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("7 Regex"), CGetSetOptions::GetRegexFilter(6), regexFilterDesc, SETTING_REGEX_FILTERING_7));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("7 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(6), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_7));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("8 Regex"), CGetSetOptions::GetRegexFilter(7), regexFilterDesc, SETTING_REGEX_FILTERING_8));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("8 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(7), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_8));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("9 Regex"), CGetSetOptions::GetRegexFilter(8), regexFilterDesc, SETTING_REGEX_FILTERING_9));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("9 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(8), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_9));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("10 Regex"), CGetSetOptions::GetRegexFilter(9), regexFilterDesc, SETTING_REGEX_FILTERING_10));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("10 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(9), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_10));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("11 Regex"), CGetSetOptions::GetRegexFilter(10), regexFilterDesc, SETTING_REGEX_FILTERING_11));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("11 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(10), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_11));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("12 Regex"), CGetSetOptions::GetRegexFilter(11), regexFilterDesc, SETTING_REGEX_FILTERING_12));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("12 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(11), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_12));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("13 Regex"), CGetSetOptions::GetRegexFilter(12), regexFilterDesc, SETTING_REGEX_FILTERING_13));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("13 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(12), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_13));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("14 Regex"), CGetSetOptions::GetRegexFilter(13), regexFilterDesc, SETTING_REGEX_FILTERING_14));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("14 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(13), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_14));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("15 Regex"), CGetSetOptions::GetRegexFilter(14), regexFilterDesc, SETTING_REGEX_FILTERING_15));
	regexFilterGroup->AddSubItem(new CMFCPropertyGridProperty(_T("15 Process Name"), CGetSetOptions::GetRegexFilterByProcessName(14), processFilterDesc, SETTING_REGEX_FILTERING_BY_PROCESS_NAME_14));

	regexFilterGroup->Expand(FALSE);

	return TRUE;
}

void CAdvGeneral::AddTrueFalse(CMFCPropertyGridProperty * pGroupTest, CString desc, BOOL value, int settingId)
{
	CString stringValue = _T("False");
	if(value)
	{
		stringValue = _T("True");
	}

	CMFCPropertyGridProperty *pCombo = new CMFCPropertyGridProperty(desc, stringValue, _T(""), settingId);
	pCombo->AddOption(_T("True"));
	pCombo->AddOption(_T("False"));
	pCombo->AllowEdit(FALSE);
	pGroupTest->AddSubItem(pCombo);
}

void CAdvGeneral::OnBnClickedOk()
{
	int topLevelCount = m_propertyGrid.GetPropertyCount();
	for (int topLevel = 0; topLevel < topLevelCount; topLevel++)
	{
		int count = m_propertyGrid.GetProperty(topLevel)->GetSubItemsCount();
		for (int row = 0; row < count; row++)
		{
			CMFCPropertyGridProperty* prop = m_propertyGrid.GetProperty(topLevel)->GetSubItem(row);

			COleVariant i = prop->GetValue();
			LPVARIANT pNewValue = (LPVARIANT)i;

			COleVariant iOrig = prop->GetOriginalValue();
			LPVARIANT pOrigValue = (LPVARIANT)iOrig;

			switch ((int)prop->GetData())
			{
			case SETTING_DESC_SIZE:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetDescTextSize(pNewValue->lVal);
				}
				break;
			case SETTING_SELECTED_INDEX:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetSelectedIndex(max((pNewValue->lVal-1), 0));
				}
				break;
			case SETTING_CLIPBOARD_SAVE_DELAY:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetProcessDrawClipboardDelay(max(pNewValue->lVal, 0));
				}
				break;
			case SETTING_SHOW_TASKBAR_ICON:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowIconInSysTray(val);
				}
				break;
			case SETTING_SAVE_MULTI_PASTE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetSaveMultiPaste(val);
				}
				break;
			case SETTING_HIDE_ON_HOTKEY_IF_VISIBLE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetHideDittoOnHotKeyIfAlreadyShown(val);
				}
				break;
			case SETTING_PASTE_IN_ACTIVE_WINDOW:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetSendPasteAfterSelection(val);
				}
				break;
			case SETTING_MAX_CLIP_SIZE:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetMaxClipSizeInBytes(pNewValue->lVal);
				}
				break;
			case SETTING_CLIP_SEPARATOR:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetMultiPasteSeparator(pNewValue->bstrVal);
				}
				break;

			case SETTING_ENSURE_CONNECTED:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetEnsureConnectToClipboard(val);
				}
				break;
			case SETTING_COPY_PLAY_SOUND:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetPlaySoundOnCopy(pNewValue->bstrVal);
				}
				break;
			case SETTING_TEXT_FIRST_TEN:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowTextForFirstTenHotKeys(val);
				}
				break;
			case SETTING_SHOW_LEADING_WHITESPACE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetDescShowLeadingWhiteSpace(val);
				}
				break;
			case SETTING_LINES_PER_ROW:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetLinesPerRow(pNewValue->lVal);
				}
				break;
			case SETTING_ENABLE_TRANSPARENCY:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetEnableTransparency(val);
				}
				break;
			case SETTING_TRANSPARENCY:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					int value = 100;
					if (pNewValue->lVal <= 100 && pNewValue->lVal > 0)
					{
						value = pNewValue->lVal;
					}

					CGetSetOptions::SetTransparencyPercent(value);
				}
				break;
			case SETTING_DRAW_THUMBNAILS:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetDrawThumbnail(val);
				}
				break;
			case SETTING_FAST_THUMBNAIL_MODE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetFastThumbnailMode(val);
				}
				break;
			case SETTING_DRAW_RTF:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetDrawRTF(val);
				}
				break;
			case SETTING_FIND_AS_TYPE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetFindAsYouType(val);
				}
				break;
			case SETTING_ENSURE_WINDOW_IS_VISIBLE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetEnsureEntireWindowCanBeSeen(val);
				}
				break;
			case SETTING_SHOW_GROUP_CLIPS_IN_LIST:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowAllClipsInMainList(val);
				}
				break;
			case SETTING_PROMPT_ON_DELETE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetPromptWhenDeletingClips(val);
				}
				break;
			case SETTING_ALWAYS_SHOW_SCROLL_BAR:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowScrollBar(val);
				}
				break;
			case SETTING_PASTE_AS_ADMIN:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetPasteAsAdmin(val);
				}
				break;
			case SETTTING_SHOW_IN_TASKBAR:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowInTaskBar(val);
				}
				break;
			case SETTING_SHOW_CLIP_PASTED:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowIfClipWasPasted(val);
				}
				break;
			case SETTING_SHOW_MSG_WHEN_RECEIVING_MANUAL_SENT_CLIP:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowMsgWhenReceivingManualSentClip(val);
				}
				break;
			case SETTING_DIFF_APP:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetDiffApp(pNewValue->bstrVal);
				}
				break;
			case SETTING_UPDATE_ORDER_ON_PASTE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetUpdateTimeOnPaste(val);
				}
				break;
			case SETTING_UPDATE_ORDER_ON_CTRL_C:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetUpdateClipOrderOnCtrlC(val);
				}
				break;
			case SETTING_MULTIPASTE_REVERSE_ORDER:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetMultiPasteReverse(val);
				}
				break;
			case SETTING_ALLOW_DUPLICATES:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetAllowDuplicates(val);
				}
				break;
			case SETTING_ALOW_BACK_TO_BACK_DUPLICATES:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetAllowBackToBackDuplicates(val);
				}
				break;
			case SETTING_REGEX_FILTERING_1:
			case SETTING_REGEX_FILTERING_2:
			case SETTING_REGEX_FILTERING_3:
			case SETTING_REGEX_FILTERING_4:
			case SETTING_REGEX_FILTERING_5:
			case SETTING_REGEX_FILTERING_6:
			case SETTING_REGEX_FILTERING_7:
			case SETTING_REGEX_FILTERING_8:
			case SETTING_REGEX_FILTERING_9:
			case SETTING_REGEX_FILTERING_10:
			case SETTING_REGEX_FILTERING_11:
			case SETTING_REGEX_FILTERING_12:
			case SETTING_REGEX_FILTERING_13:
			case SETTING_REGEX_FILTERING_14:
			case SETTING_REGEX_FILTERING_15:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetRegexFilter(pNewValue->bstrVal, (((int)prop->GetData()) - SETTING_REGEX_FILTERING_1));
				}
				break;
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_1:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_2:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_3:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_4:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_5:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_6:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_7:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_8:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_9:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_10:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_11:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_12:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_13:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_14:
			case SETTING_REGEX_FILTERING_BY_PROCESS_NAME_15:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetRegexFilterByProcessName(pNewValue->bstrVal, (((int)prop->GetData()) - SETTING_REGEX_FILTERING_BY_PROCESS_NAME_1));
				}
				break;

			case SETTING_SHOW_STARTUP_MESSAGE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetShowStartupMessage(val);
				}
				break;

			case SETTING_TOOLTIP_TIMEOUT:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetToolTipTimeout(pNewValue->lVal);
				}
				break;

			case SETTING_TOOLTIP_LINES:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetMaxToolTipLines(pNewValue->lVal);
				}
				break;

			case SETTING_TOOLTIP_CHARACTERS:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetMaxToolTipCharacters(pNewValue->lVal);
				}
				break;

			case SETTING_DEFAULT_PASTE_STRING:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetDefaultPasteString(pNewValue->bstrVal);
				}
				break;
			case SETTING_DEFAULT_COPY_STRING:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetDefaultCopyString(pNewValue->bstrVal);
				}
				break;
			case SETTING_DEFAULT_CUT_STRING:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetDefaultCutString(pNewValue->bstrVal);
				}
				break;
			case SETTING_SLUGIFY_SEPARATOR:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetSlugifySeparator(pNewValue->bstrVal);
				}
				break;
			case SETTING_REVERT_TO_TOP_LEVEL_GROUP:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetRevertToTopLevelGroup(val);
				}
				break;
			case SETTING_ACTIVATE_WINDOW_DELAY:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetSendKeysDelay(pNewValue->lVal);
				}
				break;
			case SETTING_SEND_KEYS_DELAY:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetRealSendKeysDelay(pNewValue->lVal);
				}
				break;
			case SETTING_CLIPBOARD_RESTORE_AFTER_COPY_BUFFER_DELAY:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetDittoRestoreClipboardDelay(pNewValue->lVal);
				}
				break;
			case SETTING_DOUBLE_KEYSTROKE_TIMEOUT:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetDoubleKeyStrokeTimeout(pNewValue->lVal);
				}
				break;
			case SETTING_FIRST_TEN_HOTKEYS_START:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetFirstTenHotKeysStart(pNewValue->lVal);
				}
				break;
			case SETTING_FIRST_TEN_HOTKEYS_FONT_SIZE:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetFirstTenHotKeysFontSize(pNewValue->lVal);
				}
				break;
			case SETTING_OPEN_TO_GROUP_AS_ACTIVE_EXE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetOpenToGroupByActiveExe(val);
				}
				break;

			case SETTING_ADD_CF_HDROP_ON_DRAG:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetAddCFHDROP_OnDrag(val);
				}
				break;
			case SETTING_COPY_SAVE_DELAY:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetCopyAndSveDelay(pNewValue->lVal);
				}
				break;
			case SETTING_EDITOR_FONT_SIZE:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetEditorDefaultFontSize(pNewValue->lVal);
				}
				break;
			case SETTING_MOVE_SELECTION_ON_OPEN_HOTKEY:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetMoveSelectionOnOpenHotkey(val);
				}
				break;
			case SETTING_MAINTAIN_SEARCH_VIEW:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetMaintainSearchView(val);
				}
				break;
			case SETTING_SEND_RECV_PORT:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetPort(pNewValue->lVal);
				}
				break;
			case SETTING_DEBUG_TO_FILE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetEnableDebugLogging(val);
				}
				break;
			case SETTING_DEBUG_TO_OUTPUT_STRING:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetEnableOutputDebugStringLogging(val);
				}
				break;
			case SETTING_NETWORK_BIND_IP_ADDRESS:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetNetworkBindIPAddress(pNewValue->bstrVal);
				}
				break;
			case SETTING_DISABLE_FRIENDS:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetAllowFriends(!val);
				}
				break;
			case SETTING_IGNORE_FALSE_COPIES_DELAY:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetSaveClipDelay(pNewValue->lVal);
				}
				break;
			case SETTING_REFRESH_VIEW_AFTER_PASTE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetRefreshViewAfterPasting(val);
				}
				break;
			case SETTING_SUPPORT_ALL_TYPES:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetSupportAllTypes(val);
				}
				break;
			case SETTING_IGNORE_ANNOYING_CF_DIB:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetIgnoreAnnoyingCFDIB(pNewValue->bstrVal);
				}
				break;
			case SETTING_REGEX_CASE_INSENSITIVE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetRegexCaseInsensitive(val);
				}
				break;
			case SETTING_DRAW_COPIED_COLOR_CODE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetDrawCopiedColorCode(val);
				}
				break;
			case SETTING_CENTER_WINDOW_BELOW_CURSOR_CARET:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetCenterWindowBelowCursorOrCaret(val);
				}
				break;
			case SETTING_TEXT_EDITOR_PATH:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetTextEditorPath(pNewValue->bstrVal);
				}
				break;
			case SETTING_IMAGE_EDITOR_PATH:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetImageEditorPath(pNewValue->bstrVal);
				}
				break;
			case SETTING_RTF_EDITOR_PATH:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetRTFEditorPath(pNewValue->bstrVal);
				}
				break;
			case SETTING_UPDATE_DESC_ON_CLIP_EDIT:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetUpdateDescWhenSavingClip(val);
				}
				break;
			case SETTING_QR_CODE_URL:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetQRCodeUrl(pNewValue->bstrVal);
				}
				break;
			case SETTING_APPEND_NAME_IP:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetAppendRemoveComputerNameAndIPToDescription(val);
				}
				break;
			case SETTING_USE_UTF8_FOR_DIFF:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = wcscmp(pNewValue->bstrVal, L"True") == 0;
					CGetSetOptions::SetPreferUtf8ForCompare(val);
				}
				break;
			case SETTING_CLIP_EDIT_SAVE_DELAY_AFTER_LOAD:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetClipEditSaveDelayAfterLoadSeconds(pNewValue->lVal);
				}
				break;
			case SETTING_ClIP_EDIT_SAVE_DELAY_AFTER_SAVE:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetClipEditSaveDelayAfterSaveSeconds(pNewValue->lVal);
				}
				break;
			}
		}
	}
	CDialogEx::OnOK();
}

void CAdvGeneral::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (((GetKeyState(VK_LBUTTON) & 0x100) != 0) &&
		m_mouseDownOnCaption == false)
	{
		m_Resize.MoveControls(CSize(cx, cy));
	}
	else
	{
		m_Resize.SetParent(m_hWnd);
	}
}

void CAdvGeneral::OnBnClickedBtCompactAndRepair()
{
	auto msg = theApp.m_Language.GetString("CompactRepairWarning", "Warning this can take quite a long time and require up to double the hard drive space as your current database size, Continue?");
	int ret = MessageBox(msg, _T("Ditto"), MB_YESNO);

	if (ret == IDYES)
	{
		CWaitCursor wait;

		try
		{
			try
			{
				for (int i = 0; i < 100; i++)
				{
					int toDeleteCount = theApp.m_db.execScalar(_T("SELECT COUNT(clipID) FROM MainDeletes"));
					if (toDeleteCount <= 0)
						break;

					RemoveOldEntries(false);
				}
			}
			CATCH_SQLITE_EXCEPTION

			theApp.m_db.execDML(_T("PRAGMA auto_vacuum = 1"));
			theApp.m_db.execQuery(_T("VACUUM"));
		}
		CATCH_SQLITE_EXCEPTION
	}
}

void CAdvGeneral::OnBnClickedButtonCopyScripts()
{
	CDimWnd dim(this);

	CScriptEditor e(this);
	e.m_title = _T("Copy Scripts");
	e.m_xml.Load(CGetSetOptions::GetCopyScriptsXml());
	if (e.DoModal() == IDOK)
	{
		CGetSetOptions::SetCopyScriptsXml(e.m_xml.Save());
	}
}

void CAdvGeneral::OnBnClickedButtonPasteScripts2()
{
	CDimWnd dim(this);

	CScriptEditor e(this);
	e.m_title = _T("Paste Scripts");
	e.m_xml.Load(CGetSetOptions::GetPasteScriptsXml());
	if (e.DoModal() == IDOK)
	{
		CGetSetOptions::SetPasteScriptsXml(e.m_xml.Save());
	}
}

void CAdvGeneral::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 450;
	lpMMI->ptMinTrackSize.y = 450;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CAdvGeneral::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	m_mouseDownOnCaption = false;

	if (nHitTest == HTCAPTION)
	{
		m_mouseDownOnCaption = true;
	}

	CDialog::OnNcLButtonDown(nHitTest, point);
}

void CAdvGeneral::OnEnChangeAdvFilter()
{
	Search(false);
}

void CAdvGeneral::Search(bool fromSelection)
{
	CString filterText;
	m_editFilter.GetWindowText(filterText);
	filterText.MakeLower();

	if (filterText == _T(""))
	{
		m_propertyGrid.SetCurSel(m_propertyGrid.GetProperty(0));
		m_propertyGrid.EnsureVisible(m_propertyGrid.GetProperty(0), TRUE);
		return;
	}

	auto selection = m_propertyGrid.GetCurSel();
	bool foundSelection = false;

	for (int i = 0; i < m_propertyGrid.GetPropertyCount(); ++i)
	{
		CMFCPropertyGridProperty* pProp = m_propertyGrid.GetProperty(i);
		if (pProp != nullptr)
		{
			CString name = pProp->GetName();
			name.MakeLower();

			for (int row = 0; row < pProp->GetSubItemsCount(); ++row)
			{
				auto pSubItem = pProp->GetSubItem(row);
				if (pSubItem != nullptr)
				{
					if (fromSelection && selection != nullptr && foundSelection == false)
					{
						if (selection == pSubItem)
						{
							foundSelection = true;
						}
						continue;
					}

					CString subName = pSubItem->GetName();
					subName.MakeLower();
					if (subName.Find(filterText) >= 0)
					{
						pSubItem->Show();
						m_propertyGrid.SetCurSel(pSubItem);

						//calling EnsureVisible mutliple times seemed to show it better otherwise it would randomly not work
						if (row > 2)
						{
							m_propertyGrid.EnsureVisible(pProp->GetSubItem(row - 2), TRUE);
							m_propertyGrid.EnsureVisible(pProp->GetSubItem(row - 2), TRUE);
							m_propertyGrid.EnsureVisible(pProp->GetSubItem(row - 2), TRUE);
						}
						else if (row > 1)
						{
							m_propertyGrid.EnsureVisible(pProp->GetSubItem(row - 1), TRUE);
							m_propertyGrid.EnsureVisible(pProp->GetSubItem(row - 1), TRUE);
							m_propertyGrid.EnsureVisible(pProp->GetSubItem(row - 1), TRUE);
						}
						else
						{
							m_propertyGrid.EnsureVisible(pSubItem, TRUE);
							m_propertyGrid.EnsureVisible(pSubItem, TRUE);
							m_propertyGrid.EnsureVisible(pSubItem, TRUE);
						}
						
						break;
					}
				}
			}
		}
	}
}

BOOL CAdvGeneral::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		int idCtrl = this->GetFocus()->GetDlgCtrlID();
		if (idCtrl == IDC_EDIT_ADV_FILTER)
		{
			Search(true);
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CAdvGeneral::OnBnClickedButtonNextMatch()
{
	Search(true);	
}
