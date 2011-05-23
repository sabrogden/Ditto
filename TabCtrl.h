#pragma once

#include <afxtempl.h>

#define SHEET_CLASSNAME		_T("_TabCtrlClass_")
#define SHEET_CLASSTYLE		(CS_DBLCLKS)

// Sheet styles
#define SCS_SHEET			0x0000
#define SCS_TAB				0x0001
#define SCS_TOP				0x0002
#define SCS_BOTTOM			0x0004
#define SCS_BOLD			0x0008
#define SCS_BORDER			0x0010

#define SN_SETACTIVETAB		WM_APP + 1122

typedef struct tagNMTABCHANGE{
    NMHDR hdr;
    long lOldTab;
    long lNewTab;
	long lOldItemData;
	long lNewItemData;
} NMTABCHANGE, FAR *LPNMTABCHANGE;

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx window

class CTabCtrlEx : public CWnd
{
// Construction
public:
	CTabCtrlEx();

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabCtrlEx)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTabCtrlEx();

	// Create the sheet control
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	// Add a new window to the sheet. Returns the index of the new tab, -1 on error
	bool AddItem(const CString& csTabTitle, CWnd* pTabWnd);

	// Insert a new window in the sheet. Returns the index of the new tab, -1 on error
	bool InsertItem(int nTab, const CString& csTabTitle, CWnd* pTabWnd);
	
	//Replaces the window in nTab position -- Calls HideWindow for the old and ShowWindow for the new
	bool ReplaceItem(int nTab, const CString &csTabTitle, CWnd* pTabWnd);

	// Delete a tab in the sheet
	bool DeleteItem(int nTab);

	// Delete all tabs in the sheet
	bool DeleteAllItems();
	
	// Get/Set the tab title
	CString GetTabTitle(int nTab);
	bool SetTabTitle(int nTab, const CString& csTabTitle);

	// Get the active sheet
	int GetActiveTab();

	// Get the number of tabs
	int GetTabCount();

	// Set the active tab and make the tab title visible
	void SetActiveTab(int nTab, bool bNotify = true);

	bool SetTabItemData(int nTab, long lItemData);
	long GetTabItemData(int nTab);

	// Make a tab title visible
	void MakeTabVisible(int nTab);
	
	// Set the tab height
	void SetTabHeight(int nTabHeight);

	// Relay keyboard events for the sheet to process,
	// returns TRUE if the message was processed, DO NOT process this message
	// return FALSE if the message was not processed
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Underline a tab title, use color -1 to remove underline
	void UnderlineTabTitle(int nTab, COLORREF clr);

	void SetFocusToNewlySelectedTab(bool bVal)	{ m_bSetFocusToNewlySelectedTab = bVal;	}
	bool GetFocusToNewlySelectedTab()			{ return m_bSetFocusToNewlySelectedTab;	}

	void SetTabColors(COLORREF Selected, COLORREF NonSelected)	{ m_SelectedColor = Selected; m_NonSelectedColor = NonSelected;}

protected:
	class CTab
	{
	public:
		CTab()
		{
			lItemData = -1;
		}
		CString csTitle;
		CWnd* pWnd;
		long lWidth;
		COLORREF clrUnderline;
		long lItemData;
	};

	typedef enum {ArrowLeft, ArrowRight} ButtonStyle;
	typedef enum {BtnDown, BtnUp/*, BtnHover*/} ButtonState;
		

protected:
	short m_nStyle;
	CArray <CTab, CTab&> m_Tabs;
	int m_nActiveTab;
	int m_nTabHeight;
	CBrush m_brSelectedTab;
	CBrush m_brNonSelectedTab;
	CPen m_penGray;
	CPen m_penBlack;
	CFont *m_pFntText, *m_pFntBoldText;
	ButtonState m_btnState[2];
	bool m_bBtnEnabled[2];
	int m_nLeftShifted;
	int m_nPrevWidth;
	bool m_bSetFocusToNewlySelectedTab;
	COLORREF m_SelectedColor;
	COLORREF m_NonSelectedColor;

	
protected:
	int GetTextWidth(const CString& csText);
	int GetDisplayWidth();
	int GetTabsWidth();
	int GetSpinnerWidth();
	
	void GetFullRect(CRect& rcTab);
	void GetTabListRect(CRect& rcTab);
	void GetTabRect(int nTab, CRect& rcTab);
	void GetSpinnerRect(CRect& rcButton);
	void GetButtonRect(int nBtn, CRect& rcBtn);

	void EnableSpinners();
	void DrawTabs(CDC *pDC);
	void DrawBar(CDC* pDC);
	void DrawTab(int nTab, CDC *pDC, CRect& rcTab);
	void DrawTabEx(int nTab, CDC *pDC, CRect& rcTab);
	void DrawSpinner(CDC *pDC);
	void DrawButton(CDC *pDC, CRect& rcBtn, ButtonState btnState, ButtonStyle btnStyle, bool bEnable);
	void ScrollTab(CPoint point);
	
	void ActivateTab(int nTab, bool bNotify, bool bOnSize = false);
	void ResizeTabWindow(int nOldTab, int nNewTab, bool bNotify, bool bOnSize);
	void SwitchTabs(bool bNext = true);
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CTabCtrlEx)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};