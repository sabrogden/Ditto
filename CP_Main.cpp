// CP_Main.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CP_Main.h"
#include "MainFrm.h"
#include "Misc.h"
#include "SelectDB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp

BEGIN_MESSAGE_MAP(CCP_MainApp, CWinApp)
	//{{AFX_MSG_MAP(CCP_MainApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp construction

CCP_MainApp::CCP_MainApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCP_MainApp object

CCP_MainApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp initialization

BOOL CCP_MainApp::InitInstance()
{
	AfxEnableControlContainer();

	m_hMutex = CreateMutex(NULL, FALSE, "Ditto Is Now Running");
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = (HWND)CGetSetOptions::GetMainHWND();
		if(hWnd)
			::SendMessage(hWnd, WM_SHOW_TRAY_ICON, TRUE, TRUE);

		return TRUE;
	}

	m_bShowingOptions = false;
	m_bShowingQuickPaste = false;
	m_bHandleClipboardDataChange = true;
	m_bReloadTypes = true;
	m_atomHotKey = GlobalAddAtom(HK_ACTIVATE);
	m_atomNamedCopy = GlobalAddAtom(HK_NAMED_COPY);
	m_MainhWnd = NULL;
	m_pDatabase = NULL;
	ShowCopyProperties = NULL;

	if(CheckDBExists(CGetSetOptions::GetDBPath()) == FALSE)
	{
		AfxMessageBox("Error Opening Database.");
		return TRUE;
	}

	AfxOleInit();

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp message handlers

int CCP_MainApp::ExitInstance() 
{
	if(CGetSetOptions::GetCompactAndRepairOnExit())
		CompactDatabase();
	
	CloseDB();

	return CWinApp::ExitInstance();
}

CDaoDatabase* CCP_MainApp::EnsureOpenDB(CString csName)
{
	if(!m_pDatabase)
		m_pDatabase = new CDaoDatabase;

	if(!m_pDatabase->IsOpen())
	{
		if(csName == "")
			m_pDatabase->Open(GetDBName());
		else
			m_pDatabase->Open(csName);
	}

	if(m_pMainWnd)
		((CMainFrame *)m_pMainWnd)->ResetKillDBTimer();

	return m_pDatabase;
}

BOOL CCP_MainApp::CloseDB()
{
	if(m_pDatabase)
	{
		if(m_pDatabase->IsOpen())
			m_pDatabase->Close();

		delete m_pDatabase;
		m_pDatabase = NULL;
	}

	return TRUE;
}
