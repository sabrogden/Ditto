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

	DWORD wHotKey = CGetSetOptions::GetHotKey();
	
	m_HotKey.SetHotKey(LOBYTE(wHotKey),HIBYTE(wHotKey));

	//Unregister the hotkey
	//Re regester it on cancel or ok
	UnregisterHotKey(theApp.m_MainhWnd, theApp.m_atomHotKey);
	
	wHotKey = CGetSetOptions::GetNamedCopyHotKey();
	
	m_NamedCopy.SetHotKey(LOBYTE(wHotKey),HIBYTE(wHotKey));

	//Unregister the hotkey
	//Re regester it on cancel or ok
	UnregisterHotKey(theApp.m_MainhWnd, theApp.m_atomNamedCopy);

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
	DWORD wHotKey = m_HotKey.GetHotKey();
	DWORD wNamedCopy = m_NamedCopy.GetHotKey();

	if((wHotKey == wNamedCopy) && (wHotKey != 0))
	{
		MessageBox("Activate Hot Key and Named Copy Hot Key cannot be the same.");
		return FALSE;
	}

	CGetSetOptions::SetNamedCopyHotKey(wNamedCopy);
	CGetSetOptions::SetHotKey(wHotKey);

	if(wHotKey > 0)
	{
		if(!CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, wHotKey, theApp.m_atomHotKey))
		{
			MessageBox("Error Registering HotKey to Activate Ditto");
			return FALSE;
		}
	}

	if(wNamedCopy > 0)
	{
		if(!CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, wNamedCopy, theApp.m_atomNamedCopy))
		{
			MessageBox("Error Registering HotKey for Named Copy");
			return FALSE;
		}
	}
		
	return CPropertyPage::OnApply();
}

BOOL COptionsKeyBoard::ValidateHotKey(WORD wHotKey)
{
	ATOM id = GlobalAddAtom("HK_VALIDATE");
	BOOL bResult = CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, wHotKey, FALSE);
	
	if(bResult)
		UnregisterHotKey(GetSafeHwnd(), id);

	GlobalDeleteAtom(id);

	return bResult;
}

void COptionsKeyBoard::OnCancel() 
{
	if(CGetSetOptions::GetHotKey())
	{
		if(!CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, 
										CGetSetOptions::GetHotKey(), 
										theApp.m_atomHotKey))
		{
			MessageBox("Error Registering HotKey to Activate Ditto");
			return;
		}
	}

	if(CGetSetOptions::GetNamedCopyHotKey())
	{
		if(!CGetSetOptions::RegisterHotKey(theApp.m_MainhWnd, 
										CGetSetOptions::GetNamedCopyHotKey(), 
										theApp.m_atomNamedCopy))
		{
			MessageBox("Error Registering HotKey for Named Copy");
			return;
		}
	}
	
	
	CPropertyPage::OnCancel();
}
