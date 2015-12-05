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

	void Create(CWnd *pParent);
	void ShowQPasteWnd(CWnd *pParent, bool bAtPrevPos, bool bFromKeyboard, BOOL bReFillList);
	void HideQPasteWnd();
	BOOL CloseQPasteWnd();
	BOOL IsWindowVisibleEx();
	void MoveSelection(bool down);
	void OnKeyStateUp();
	void SetKeyModiferState(bool bActive);
	bool IsWindowTopLevel();

	void UpdateFont()		{ if(m_pwndPaste) m_pwndPaste->UpdateFont();	}

	void OnScreenResolutionChange();

//protected:
	CQPasteWnd *m_pwndPaste;

};

#endif // !defined(AFX_QUICKPASTE_H__1B4A98E6_B719_402C_BDD4_7F3F97CD0EB0__INCLUDED_)
