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
	//{{AFX_DATA_INIT(COptionsKeyBoard)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COptionsKeyBoard::~COptionsKeyBoard()
{
}

void COptionsKeyBoard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsKeyBoard)
	DDX_Control(pDX, IDC_NAMED_COPY, m_NamedCopy);
	DDX_Control(pDX, IDC_HOTKEY, m_HotKey);
	//}}AFX_DATA_MAP
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

	m_pParent = (COptionsSheet *)GetParent();

	theApp.m_pDittoHotKey->CopyToCtrl( m_HotKey );
	theApp.m_pCopyHotKey->CopyToCtrl( m_NamedCopy );

	//Unregister hotkeys and Reregister them on cancel or ok
	g_HotKeys.UnregisterAll();

	m_HotKey.SetFocus();

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
int x,y;
CString str;
ARRAY keys;

	g_HotKeys.GetKeys( keys ); // save old keys just in case new ones are invalid

	theApp.m_pDittoHotKey->CopyFromCtrl( m_HotKey );
	theApp.m_pCopyHotKey->CopyFromCtrl( m_NamedCopy );

	if( g_HotKeys.FindFirstConflict(keys,&x,&y) )
	{
		str =  g_HotKeys.GetAt(x)->GetName();
		str += " and ";
		str += g_HotKeys.GetAt(y)->GetName();
		str += " cannot be the same.";
		MessageBox(str);
		g_HotKeys.SetKeys( keys ); // restore the original values
		return FALSE;
	}

	g_HotKeys.SaveAllKeys();
	g_HotKeys.RegisterAll(true);
		
	return CPropertyPage::OnApply();
}

/*
BOOL COptionsKeyBoard::ValidateHotKey(WORD wHotKey)
{
	ATOM id = GlobalAddAtom("HK_VALIDATE");
	BOOL bResult = CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, wHotKey, FALSE);
	
	if(bResult)
		UnregisterHotKey(GetSafeHwnd(), id);

	GlobalDeleteAtom(id);

	return bResult;
}
*/

void COptionsKeyBoard::OnCancel() 
{
	g_HotKeys.RegisterAll( true );
	CPropertyPage::OnCancel();
}
