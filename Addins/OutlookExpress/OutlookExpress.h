// OutlookExpress.h : main header file for the OutlookExpress DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// COutlookExpressApp
// See OutlookExpress.cpp for the implementation of this class
//

class COutlookExpressApp : public CWinApp
{
public:
	COutlookExpressApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};
