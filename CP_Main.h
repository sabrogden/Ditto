// CP_Main.h : main header file for the CP_MAIN application
//

#if !defined(AFX_CP_MAIN_H__DAB2F753_2CC1_4FED_8095_763987961026__INCLUDED_)
#define AFX_CP_MAIN_H__DAB2F753_2CC1_4FED_8095_763987961026__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "ProcessCopy.h"
#include "DatabaseUtilities.h"
#include "Misc.h"
#include "DataTable.h"
#include "MainTable.h"
#include "TypesTable.h"
#include "ArrayEx.h"

#define MAIN_WND_TITLE		"Ditto MainWnd"
//#define GET_APP    ((CCP_MainApp *)AfxGetApp())	

//#define GET_APP ((CMainWnd*)theApp)
extern class CCP_MainApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCP_MainApp:
// See CP_Main.cpp for the implementation of this class
//
class CCP_MainApp : public CWinApp
{
public:
	CCP_MainApp();

	bool m_bShowingOptions;
	bool m_bShowingQuickPaste;
	bool m_bHandleClipboardDataChange;
	bool m_bReloadTypes;
	bool ShowCopyProperties;

	HANDLE m_hMutex;

	ATOM m_atomHotKey;

	ATOM m_atomNamedCopy;
	
	HWND m_MainhWnd;

	CDaoDatabase	*m_pDatabase;

	CDaoDatabase	*EnsureOpenDB(CString csName = "");
	BOOL			CloseDB();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCP_MainApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
	

// Implementation
	//{{AFX_MSG(CCP_MainApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CP_MAIN_H__DAB2F753_2CC1_4FED_8095_763987961026__INCLUDED_)
