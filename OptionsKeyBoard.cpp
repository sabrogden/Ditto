// OptionsKeyBoard.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsKeyBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsKeyBoard property page

IMPLEMENT_DYNCREATE(COptionsKeyBoard, CPropertyPage)

COptionsKeyBoard::COptionsKeyBoard() : CPropertyPage(COptionsKeyBoard::IDD)
{
	m_csTitle = theApp.m_Language.GetString("KeyboardShortcutsTitle", "Keyboard Shortcuts");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE;
	
	//{{AFX_DATA_INIT(COptionsKeyBoard)
	//}}AFX_DATA_INIT
}

COptionsKeyBoard::~COptionsKeyBoard()
{
}

void COptionsKeyBoard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsKeyBoard)
	DDX_Control(pDX, IDC_CHECK_SEND_PASTE, m_btSendPaste);
	DDX_Control(pDX, IDC_CHECK_USE_UI_GROUP_LAST_10, m_UseUiGroupForLastTen);
	DDX_Control(pDX, IDC_HOTKEY9, m_Nine);
	DDX_Control(pDX, IDC_HOTKEY8, m_Eight);
	DDX_Control(pDX, IDC_HOTKEY7, m_Seven);
	DDX_Control(pDX, IDC_HOTKEY6, m_Six);
	DDX_Control(pDX, IDC_HOTKEY5, m_Five);
	DDX_Control(pDX, IDC_HOTKEY4, m_Four);
	DDX_Control(pDX, IDC_HOTKEY3, m_Three);
	DDX_Control(pDX, IDC_HOTKEY2, m_Two);
	DDX_Control(pDX, IDC_HOTKEY10, m_Ten);
	DDX_Control(pDX, IDC_HOTKEY1, m_One);
	DDX_Control(pDX, IDC_HOTKEY, m_HotKey);
	DDX_Control(pDX, IDC_HOTKEY_ACTIVATE_2, m_HotKey2);
	DDX_Control(pDX, IDC_HOTKEY_ACTIVATE_3, m_HotKey3);
	DDX_Control(pDX, IDC_HOTKEY_TEXT_ONLY, m_TextOnlyKey);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_CUSTOM_KEYS, m_CustomeKeysHelp);
	DDX_Control(pDX, IDC_CHECK_MOVE_CLIPS_ON_PASTE, m_btMoveClipOnGlobal10);
	DDX_Control(pDX, IDC_HOTKEY_SAVE_CLIPBOARD, m_saveClipboardHotKey);
	DDX_Control(pDX, IDC_HOTKEY_COPYSAVECLIPBOARD, m_copyAndSaveClipboardCtrl);
}

BEGIN_MESSAGE_MAP(COptionsKeyBoard, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsKeyBoard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsKeyBoard message handlers

BOOL COptionsKeyBoard::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_CustomeKeysHelp.SetURL(_T("https://github.com/sabrogden/Ditto/wiki/Custom-Key-Strokes"));

	m_pParent = (COptionsSheet *)GetParent();

	theApp.m_pDittoHotKey->CopyToCtrl(m_HotKey, m_hWnd, IDC_CHECK_WIN_DITTO);
	theApp.m_pDittoHotKey2->CopyToCtrl(m_HotKey2, m_hWnd, IDC_CHECK_WIN_DITTO2);
	theApp.m_pDittoHotKey3->CopyToCtrl(m_HotKey3, m_hWnd, IDC_CHECK_WIN_DITTO3);

	theApp.m_pPosOne->CopyToCtrl(m_One, m_hWnd, IDC_CHECK_WIN1);
	theApp.m_pPosTwo->CopyToCtrl(m_Two, m_hWnd, IDC_CHECK_WIN2);
	theApp.m_pPosThree->CopyToCtrl(m_Three, m_hWnd, IDC_CHECK_WIN3);
	theApp.m_pPosFour->CopyToCtrl(m_Four, m_hWnd, IDC_CHECK_WIN4);
	theApp.m_pPosFive->CopyToCtrl(m_Five, m_hWnd, IDC_CHECK_WIN5);
	theApp.m_pPosSix->CopyToCtrl(m_Six, m_hWnd, IDC_CHECK_WIN6);
	theApp.m_pPosSeven->CopyToCtrl(m_Seven, m_hWnd, IDC_CHECK_WIN7);
	theApp.m_pPosEight->CopyToCtrl(m_Eight, m_hWnd, IDC_CHECK_WIN8);
	theApp.m_pPosNine->CopyToCtrl(m_Nine, m_hWnd, IDC_CHECK_WIN9);
	theApp.m_pPosTen->CopyToCtrl(m_Ten, m_hWnd, IDC_CHECK_WIN10);
	theApp.m_pTextOnlyPaste->CopyToCtrl(m_TextOnlyKey, m_hWnd, IDC_CHECK_WIN_TEXT_ONLY);
	theApp.m_pSaveClipboard->CopyToCtrl(m_saveClipboardHotKey, m_hWnd, IDC_CHECK_WIN_SAVE_CLIPBOARD);
	theApp.m_pCopyAndSaveClipboard->CopyToCtrl(m_copyAndSaveClipboardCtrl, m_hWnd, IDC_CHECK_WIN_COPY_SAVE_CLIPBOARD);


	//Unregister hotkeys and Reregister them on cancel or ok
	g_HotKeys.UnregisterAll();

	m_btSendPaste.SetCheck(CGetSetOptions::m_bSendPasteOnFirstTenHotKeys);
	m_UseUiGroupForLastTen.SetCheck(CGetSetOptions::GetUseUISelectedGroupForLastTenCopies());

	m_btMoveClipOnGlobal10.SetCheck(CGetSetOptions::GetMoveClipsOnGlobal10());

	m_HotKey.SetFocus();

	theApp.m_Language.UpdateOptionShortcuts(this);	
		
	return FALSE;
}

LRESULT COptionsKeyBoard::OnWizardNext() 
{
	return CPropertyPage::OnWizardNext();
}

BOOL COptionsKeyBoard::OnWizardFinish() 
{
	return CPropertyPage::OnWizardFinish();
}

BOOL COptionsKeyBoard::OnApply()
{
	CGetSetOptions::SetSendPasteOnFirstTenHotKeys(m_btSendPaste.GetCheck());
	CGetSetOptions::SetMoveClipsOnGlobal10(m_btMoveClipOnGlobal10.GetCheck());
	CGetSetOptions::SetUseUISelectedGroupForLastTenCopies(m_UseUiGroupForLastTen.GetCheck());
					
	INT_PTR x,y;
	CString str;
	ARRAY keys;
	
	g_HotKeys.GetKeys( keys ); // save old keys just in case new ones are invalid
	
	theApp.m_pDittoHotKey->CopyFromCtrl(m_HotKey, m_hWnd, IDC_CHECK_WIN_DITTO);
	theApp.m_pDittoHotKey2->CopyFromCtrl(m_HotKey2, m_hWnd, IDC_CHECK_WIN_DITTO2);
	theApp.m_pDittoHotKey3->CopyFromCtrl(m_HotKey3, m_hWnd, IDC_CHECK_WIN_DITTO3);
	
	theApp.m_pPosOne->CopyFromCtrl(m_One, m_hWnd, IDC_CHECK_WIN1);
	theApp.m_pPosTwo->CopyFromCtrl(m_Two, m_hWnd, IDC_CHECK_WIN2);
	theApp.m_pPosThree->CopyFromCtrl(m_Three, m_hWnd, IDC_CHECK_WIN3);
	theApp.m_pPosFour->CopyFromCtrl(m_Four, m_hWnd, IDC_CHECK_WIN4);
	theApp.m_pPosFive->CopyFromCtrl(m_Five, m_hWnd, IDC_CHECK_WIN5);
	theApp.m_pPosSix->CopyFromCtrl(m_Six, m_hWnd, IDC_CHECK_WIN6);
	theApp.m_pPosSeven->CopyFromCtrl(m_Seven, m_hWnd, IDC_CHECK_WIN7);
	theApp.m_pPosEight->CopyFromCtrl(m_Eight, m_hWnd, IDC_CHECK_WIN8);
	theApp.m_pPosNine->CopyFromCtrl(m_Nine, m_hWnd, IDC_CHECK_WIN9);
	theApp.m_pPosTen->CopyFromCtrl(m_Ten, m_hWnd, IDC_CHECK_WIN10);
	theApp.m_pTextOnlyPaste->CopyFromCtrl(m_TextOnlyKey, m_hWnd, IDC_CHECK_WIN_TEXT_ONLY);
	theApp.m_pSaveClipboard->CopyFromCtrl(m_saveClipboardHotKey, m_hWnd, IDC_CHECK_WIN_SAVE_CLIPBOARD);
	theApp.m_pCopyAndSaveClipboard->CopyFromCtrl(m_copyAndSaveClipboardCtrl, m_hWnd, IDC_CHECK_WIN_COPY_SAVE_CLIPBOARD);

	ARRAY NewKeys;
	g_HotKeys.GetKeys(NewKeys);
	
	if(g_HotKeys.FindFirstConflict(NewKeys, &x, &y))
	{
		str =  g_HotKeys.ElementAt(x)->GetName();
		str += " and ";
		str += g_HotKeys.ElementAt(y)->GetName();
		str += " cannot be the same.";
		MessageBox(str);
		g_HotKeys.SetKeys(keys); // restore the original values
		return FALSE;
	}
	
	g_HotKeys.SaveAllKeys();
	g_HotKeys.RegisterAll(true);
	
	return CPropertyPage::OnApply();
}

void COptionsKeyBoard::OnCancel() 
{
	g_HotKeys.RegisterAll( true );
	CPropertyPage::OnCancel();
}
