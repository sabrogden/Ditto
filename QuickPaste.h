// QuickPaste.h: interface for the CQuickPaste class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUICKPASTE_H__1B4A98E6_B719_402C_BDD4_7F3F97CD0EB0__INCLUDED_)
#define AFX_QUICKPASTE_H__1B4A98E6_B719_402C_BDD4_7F3F97CD0EB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QPasteWnd.h"

class CQuickPaste  
{
public:
	CQuickPaste();
	virtual ~CQuickPaste();

	void ShowQPasteWnd(CWnd *pParent, BOOL bAtPrevPos = FALSE);
	void HideQPasteWnd();
	BOOL CloseQPasteWnd();

protected:
	CQPasteWnd *m_pwndPaste;
};

#endif // !defined(AFX_QUICKPASTE_H__1B4A98E6_B719_402C_BDD4_7F3F97CD0EB0__INCLUDED_)
