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
}


BEGIN_MESSAGE_MAP(CAdvGeneral, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAdvGeneral::OnBnClickedOk)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BT_COMPACT_AND_REPAIR, &CAdvGeneral::OnBnClickedBtCompactAndRepair)
	ON_BN_CLICKED(IDC_BUTTON_COPY_SCRIPTS, &CAdvGeneral::OnBnClickedButtonCopyScripts)
	ON_BN_CLICKED(IDC_BUTTON_PASTE_SCRIPTS, &CAdvGeneral::OnBnClickedButtonPasteScripts2)
	ON_WM_GETMINMAXINFO()
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
#define SETTING_PROMP_ON_DELETE 19
#define SETTING_ALWAYS_SHOW_SCROLL_BAR 20
#define SETTING_PASTE_AS_ADMIN 21
#define SETTTING_SHOW_IN_TASKBAR 22
#define SETTING_SHOW_CLIP_PASTED 23
#define SETTING_DIFF_APP 24
#define SETTING_TRANSPARENCY 25
#define SETTING_UPDATE_ORDER 26
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

BOOL CAdvGeneral::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	HICON b = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 64, 64, LR_SHARED);
	SetIcon(b, TRUE);

	m_propertyGrid.ModifyStyle(0, WS_CLIPCHILDREN);

	CMFCPropertyGridProperty * pGroupTest = new CMFCPropertyGridProperty( _T( "Ditto" ) );
	m_propertyGrid.AddProperty(pGroupTest);	

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_MFCPROPERTYGRID1, DR_SizeWidth | DR_SizeHeight);
	m_Resize.AddControl(IDOK, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDC_BT_COMPACT_AND_REPAIR, DR_MoveTop);
	m_Resize.AddControl(IDC_BUTTON_COPY_SCRIPTS, DR_MoveTop);
	m_Resize.AddControl(IDC_BUTTON_PASTE_SCRIPTS, DR_MoveTop);

	HDITEM hdItem;
	hdItem.mask = HDI_WIDTH; // indicating cxy is width
	hdItem.cxy = 300; // whatever you want the property name column width to be
	m_propertyGrid.GetHeaderCtrl().SetItem(0, &hdItem);

	m_propertyGrid.SetFont(this->GetFont());
	
	
	AddTrueFalse(pGroupTest, _T("Allow Duplicates"), CGetSetOptions::GetAllowDuplicates(), SETTING_ALLOW_DUPLICATES);
	AddTrueFalse(pGroupTest, _T("Always Show Scroll Bar"), CGetSetOptions::GetShowScrollBar(), SETTING_ALWAYS_SHOW_SCROLL_BAR);
	pGroupTest->AddSubItem( new CMFCPropertyGridProperty(_T("Amount of text to save for description"), g_Opt.m_bDescTextSize, _T(""), SETTING_DESC_SIZE));
	
	static TCHAR BASED_CODE szDiffFilter[] = _T("Diff Applications(*.exe)|*.exe||");
	CMFCPropertyGridFileProperty* pDiffProp = new CMFCPropertyGridFileProperty(_T("Diff Application Path"), TRUE, CGetSetOptions::GetDiffApp(), _T("exe"), 0, szDiffFilter, (LPCTSTR)0, SETTING_DIFF_APP);
	pGroupTest->AddSubItem(pDiffProp);


	AddTrueFalse(pGroupTest, _T("Display Icon in System Tray"), CGetSetOptions::GetShowIconInSysTray(), SETTING_SHOW_TASKBAR_ICON);
	AddTrueFalse(pGroupTest, _T("Draw RTF Text in List(for RTF types) (Could Increase Memory Usage an Display Speed)"), CGetSetOptions::GetDrawRTF(), SETTING_DRAW_RTF);
	AddTrueFalse(pGroupTest, _T("Elevated privileges to paste into elevated apps"), CGetSetOptions::GetPasteAsAdmin(), SETTING_PASTE_AS_ADMIN);
	AddTrueFalse(pGroupTest, _T("Ensure Ditto is always connected to the clipboard"), CGetSetOptions::GetEnsureConnectToClipboard(), SETTING_ENSURE_CONNECTED);
	AddTrueFalse(pGroupTest, _T("Ensure Entire Window is Visible"), CGetSetOptions::GetEnsureEntireWindowCanBeSeen(), SETTING_ENSURE_WINDOW_IS_VISIBLE);
	AddTrueFalse(pGroupTest, _T("Find As You Type"), CGetSetOptions::GetFindAsYouType(), SETTING_FIND_AS_TYPE);
	AddTrueFalse(pGroupTest, _T("Hide Ditto on Hot Key if Ditto is Visible"), CGetSetOptions::GetHideDittoOnHotKeyIfAlreadyShown(), SETTING_HIDE_ON_HOTKEY_IF_VISIBLE);
	pGroupTest->AddSubItem( new CMFCPropertyGridProperty(_T("Maximum Clip Size in Bytes (0 for no limit)"), g_Opt.m_lMaxClipSizeInBytes, _T(""), SETTING_MAX_CLIP_SIZE));
	pGroupTest->AddSubItem( new CMFCPropertyGridProperty(_T("Multi-Paste clip separator ([LF] = line feed)"), g_Opt.GetMultiPasteSeparator(false), _T(""), SETTING_CLIP_SEPARATOR));

	static TCHAR BASED_CODE szFilter[] = _T("Sounds(*.wav)|*.wav||");
	CMFCPropertyGridFileProperty* pFileProp = new CMFCPropertyGridFileProperty(_T("On copy play the sound"), TRUE, CGetSetOptions::GetPlaySoundOnCopy(), _T("wav"), 0, szFilter, (LPCTSTR)0, SETTING_COPY_PLAY_SOUND);
	pGroupTest->AddSubItem(pFileProp);

	AddTrueFalse(pGroupTest, _T("Paste Clip in active window after selection"), CGetSetOptions::GetSendPasteAfterSelection(), SETTING_PASTE_IN_ACTIVE_WINDOW);
	AddTrueFalse(pGroupTest, _T("Prompt when deleting clips"), CGetSetOptions::GetPromptWhenDeletingClips(), SETTING_PROMP_ON_DELETE);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Save Clipboard Delay (ms, default: 100))"), (long)(CGetSetOptions::GetProcessDrawClipboardDelay()), _T(""), SETTING_CLIPBOARD_SAVE_DELAY));

	AddTrueFalse(pGroupTest, _T("Save Multi-Pastes"), CGetSetOptions::GetSaveMultiPaste(), SETTING_SAVE_MULTI_PASTE);

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Selected Index"), (long)(CGetSetOptions::SelectedIndex()+1), _T(""), SETTING_SELECTED_INDEX));

	AddTrueFalse(pGroupTest, _T("Show Clips That are in Groups in Main List"), CGetSetOptions::GetShowAllClipsInMainList(), SETTING_SHOW_GROUP_CLIPS_IN_LIST);
	AddTrueFalse(pGroupTest, _T("Show leading whitespace"), CGetSetOptions::GetDescShowLeadingWhiteSpace(), SETTING_SHOW_LEADING_WHITESPACE);
	AddTrueFalse(pGroupTest, _T("Show In Taskbar"), CGetSetOptions::GetShowInTaskBar(), SETTTING_SHOW_IN_TASKBAR);
	AddTrueFalse(pGroupTest, _T("Show indicator a clip has been pasted"), CGetSetOptions::GetShowIfClipWasPasted(), SETTING_SHOW_CLIP_PASTED);

	AddTrueFalse(pGroupTest, _T("Show message that we received a manual sent clip"), CGetSetOptions::GetShowMsgWhenReceivingManualSentClip(), SETTING_SHOW_MSG_WHEN_RECEIVING_MANUAL_SENT_CLIP);	

	AddTrueFalse(pGroupTest, _T("Show startup tooltip message"), CGetSetOptions::GetShowStartupMessage(), SETTING_SHOW_STARTUP_MESSAGE);

	AddTrueFalse(pGroupTest, _T("Show text for first ten copy hot keys"), CGetSetOptions::GetShowTextForFirstTenHotKeys(), SETTING_TEXT_FIRST_TEN);
	AddTrueFalse(pGroupTest, _T("Show thumbnails(for CF_DIB types) (Could Increase Memory Usage and Display Speed)"), CGetSetOptions::GetDrawThumbnail(), SETTING_DRAW_THUMBNAILS);
	
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Text Lines per Clip"), CGetSetOptions::GetLinesPerRow(), _T(""), SETTING_LINES_PER_ROW));

	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Tooltip display time(ms) max of 32000 (-1 default (5 seconds), 0 to turn off)"), g_Opt.m_tooltipTimeout, _T(""), SETTING_TOOLTIP_TIMEOUT));

	AddTrueFalse(pGroupTest, _T("Transparency Enabled"), CGetSetOptions::GetEnableTransparency(), SETTING_ENABLE_TRANSPARENCY);
	pGroupTest->AddSubItem(new CMFCPropertyGridProperty(_T("Transparency Percentage"), CGetSetOptions::GetTransparencyPercent(), _T(""), SETTING_TRANSPARENCY));
	AddTrueFalse(pGroupTest, _T("Update Clip Order On Paste"), CGetSetOptions::GetUpdateTimeOnPaste(), SETTING_UPDATE_ORDER);

	CMFCPropertyGridProperty * regexFilterGroup = new CMFCPropertyGridProperty(_T("Exlude clips by Regular Expressions"));
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
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowIconInSysTray(val);
				}
				break;
			case SETTING_SAVE_MULTI_PASTE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetSaveMultiPaste(val);
				}
				break;
			case SETTING_HIDE_ON_HOTKEY_IF_VISIBLE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetHideDittoOnHotKeyIfAlreadyShown(val);
				}
				break;
			case SETTING_PASTE_IN_ACTIVE_WINDOW:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
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
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
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
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowTextForFirstTenHotKeys(val);
				}
				break;
			case SETTING_SHOW_LEADING_WHITESPACE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
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
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
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
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetDrawThumbnail(val);
				}
				break;
			case SETTING_DRAW_RTF:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetDrawRTF(val);
				}
				break;
			case SETTING_FIND_AS_TYPE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetFindAsYouType(val);
				}
				break;
			case SETTING_ENSURE_WINDOW_IS_VISIBLE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetEnsureEntireWindowCanBeSeen(val);
				}
				break;
			case SETTING_SHOW_GROUP_CLIPS_IN_LIST:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowAllClipsInMainList(val);
				}
				break;
			case SETTING_PROMP_ON_DELETE:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetPromptWhenDeletingClips(val);
				}
				break;
			case SETTING_ALWAYS_SHOW_SCROLL_BAR:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowScrollBar(val);
				}
				break;
			case SETTING_PASTE_AS_ADMIN:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetPasteAsAdmin(val);
				}
				break;
			case SETTTING_SHOW_IN_TASKBAR:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowInTaskBar(val);
				}
				break;
			case SETTING_SHOW_CLIP_PASTED:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowIfClipWasPasted(val);
				}
				break;
			case SETTING_SHOW_MSG_WHEN_RECEIVING_MANUAL_SENT_CLIP:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowMsgWhenReceivingManualSentClip(val);
				}
				break;
			case SETTING_DIFF_APP:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					CGetSetOptions::SetDiffApp(pNewValue->bstrVal);
				}
				break;
			case SETTING_UPDATE_ORDER:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetUpdateTimeOnPaste(val);
				}
				break;
			case SETTING_ALLOW_DUPLICATES:
				if (wcscmp(pNewValue->bstrVal, pOrigValue->bstrVal) != 0)
				{
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetAllowDuplicates(val);
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
					BOOL val = false;
					if (wcscmp(pNewValue->bstrVal, L"True") == 0)
					{
						val = true;
					}
					CGetSetOptions::SetShowStartupMessage(val);
				}
				break;

			case SETTING_TOOLTIP_TIMEOUT:
				if (pNewValue->lVal != pOrigValue->lVal)
				{
					CGetSetOptions::SetToolTipTimeout(pNewValue->lVal);
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

	m_Resize.MoveControls(CSize(cx, cy));
}


void CAdvGeneral::OnBnClickedBtCompactAndRepair()
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
