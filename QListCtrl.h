 #if !defined(AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_)
#define AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QListCtrl.h : header file
//
#include "ArrayEx.h"

#define NM_SELECT					WM_USER+0x100
#define NM_RIGHT					WM_USER+0x101
#define NM_LEFT						WM_USER+0x102
#define NM_END						WM_USER+0x103
#define NM_DELETE					WM_USER+0x105
#define NM_PROPERTIES				WM_USER+0x106
#define NM_LBUTTONDOWN				WM_USER+0x107
#define NM_GETTOOLTIPTEXT	        WM_USER+0x108
#define NM_SELECT_DB_ID		        WM_USER+0x109
#define NM_SELECT_INDEX		        WM_USER+0x110

class CQListToolTipText
{
public:
	CQListToolTipText()
	{
		memset(&cText, 0, sizeof(cText));
	}

	NMHDR hdr;
	char cText[500];
	long lItem;
};

class CQListCtrl : public CListCtrl
{
// Construction
public:
	CQListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQListCtrl)
	public:
	virtual int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam); //!!!!!!
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CQListCtrl();

	void SetNumberOfLinesPerRow(int nLines);
	void GetSelectionIndexes(ARRAY &arr);
	void GetSelectionItemData(ARRAY &arr);
	void RefreshVisibleRows();
	void RemoveAllSelection();
	BOOL SetSelection(int nRow, BOOL bSelect = TRUE);
	BOOL SetText(int nRow, int nCol, CString cs);
	BOOL SetFormattedText(int nRow, int nCol, LPCTSTR lpszFormat,...);
	BOOL SetCaret(int nRow, BOOL bFocus = TRUE);
	long GetCaret();
	DWORD GetItemData(int nItem);
	CString GetToolTipText(int nItem);

	void SetShowTextForFirstTenHotKeys(BOOL bVal)	{ m_bShowTextForFirstTenHotKeys = bVal;	}

	void DestroyAndCreateAccelerator(BOOL bCreate);

protected:
	void SendSelection(int nItem);;
	void SendSelection(ARRAY &arrItems);
	
	WCHAR *m_pwchTip;
	TCHAR *m_pchTip;

	HFONT m_SmallFont;
	BOOL  m_bShowTextForFirstTenHotKeys;

	//Accelerator
	CAccels	m_Accels;
//	HACCEL	m_Accelerator; // !!!!!
//	bool	m_CheckingAccelerator;
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CQListCtrl)
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	afx_msg BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_)
