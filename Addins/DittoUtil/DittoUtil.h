// DittoUtil.h : main header file for the DittoUtil DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDittoUtilApp
// See DittoUtil.cpp for the implementation of this class
//

class CDittoUtilApp : public CWinApp
{
public:
	CDittoUtilApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

	DECLARE_MESSAGE_MAP()
};
