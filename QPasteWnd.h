#if !defined(AFX_QPASTEWND_H__65261F0F_FEFD_48CF_A0CD_01D8BFEB353B__INCLUDED_)
#define AFX_QPASTEWND_H__65261F0F_FEFD_48CF_A0CD_01D8BFEB353B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QPasteWnd.h : header file
//

#include "QListCtrl.h"
#include "ComboBoxSearch.h"
#include "WndEx.h"
#include "GroupStatic.h"
#include "GroupTree.h"

#ifdef AFTER_98
	#include "AlphaBlend.h"
#endif


/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd window

class CQPasteWnd : public CWndEx
{
// Construction
public:
	CQPasteWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQPasteWnd)
	public:
	virtual BOOL Create(const POINT& ptStart, CWnd* pParentWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	bool Add(const CString &csHeader, const CString &csText, int nID);
	virtual ~CQPasteWnd();

//protected:
	CQListCtrl		m_lstHeader;

	#ifdef AFTER_98
		CAlphaBlend		m_Alpha;
	#endif
	
	CFont			m_TitleFont;
	CComboBoxSearch	m_cbSearch;
	CFont			m_SearchFont;
	CButton			m_btCancel;
	bool			m_bHideWnd;
	CMainTable		m_Recset;
	CString			m_strSQLSearch;
	CGroupStatic	m_stGroup;
	CFont			GroupFont;
	CString			m_Title;
	CGroupTree		m_GroupTree;
	CBitmapButton	m_ShowGroupsFolderBottom;
	CBitmapButton	m_ShowGroupsFolderTop;

	void RefreshNc( bool bRepaintImmediately = false );
	void UpdateStatus( bool bRepaintImmediately = false );  // regenerates the status (caption) text
	BOOL FillList(CString csSQLSearch = "");
	BOOL HideQPasteWindow();
	BOOL ShowQPasteWindow(BOOL bFillList = TRUE);
	void MoveControls();

	void DeleteSelectedRows();

	BOOL OpenID( long lID );
	BOOL OpenSelection();
	BOOL OpenIndex( long nItem );
	BOOL NewGroup( bool bGroupSelection = true );
	// moves the caret to the given ID, selects it, and ensures it is visible.
	BOOL SetListID( long lID );

	CString LoadDescription( int nItem );
	bool SaveDescription( int nItem, CString text );

	//Menu Items
	void SetLinesPerRow(long lLines);
	void SetTransparency(long lPercent);
	void OnUpdateLinesPerRow(CCmdUI* pCmdUI, int nValue);
	void OnUpdateTransparency(CCmdUI* pCmdUI, int nValue);
	void SetMenuChecks(CMenu *pMenu);
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CQPasteWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnMenuLinesperrow1();
	afx_msg void OnMenuLinesperrow2();
	afx_msg void OnMenuLinesperrow3();
	afx_msg void OnMenuLinesperrow4();
	afx_msg void OnMenuLinesperrow5();
	afx_msg void OnMenuTransparency10();
	afx_msg void OnMenuTransparency15();
	afx_msg void OnMenuTransparency20();
	afx_msg void OnMenuTransparency25();
	afx_msg void OnMenuTransparency30();
	afx_msg void OnMenuTransparency40();
	afx_msg void OnMenuTransparency5();
	afx_msg void OnMenuTransparencyNone();
	afx_msg void OnRclickQuickPaste(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMenuDelete();
	afx_msg void OnMenuPositioningAtcaret();
	afx_msg void OnMenuPositioningAtcursor();
	afx_msg void OnMenuPositioningAtpreviousposition();
	afx_msg void OnMenuOptions();
	afx_msg void OnCancelFilter();
	afx_msg void OnMenuExitprogram();
	afx_msg void OnMenuReconnecttoclipboardchain();
	afx_msg void OnMenuProperties();
	afx_msg void OnClose();
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void GetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFindItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMenuFirsttenhotkeysUsectrlnum();
	afx_msg void OnMenuFirsttenhotkeysShowhotkeytext();
	afx_msg void OnMenuQuickoptionsAllwaysshowdescription();
	afx_msg void OnMenuQuickoptionsDoubleclickingoncaptionTogglesalwaysontop();
	afx_msg void OnMenuQuickoptionsDoubleclickingoncaptionRollupwindow();
	afx_msg void OnMenuQuickoptionsDoubleclickingoncaptionTogglesshowdescription();
	afx_msg void OnMenuQuickoptionsPromptfornewgroupnames();
	afx_msg void OnShowGroupsBottom();
	afx_msg void OnShowGroupsTop();
	afx_msg void OnMenuViewgroups();
	//}}AFX_MSG
	afx_msg LRESULT OnListSelect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSearch(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDelete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProperties(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGetToolTipText(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnListSelect_DB_ID(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListSelect_Index(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRefreshView(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGroupTreeMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnViewcaptionbaronRight();
	afx_msg void OnViewcaptionbaronBottom();
	afx_msg void OnViewcaptionbaronLeft();
	afx_msg void OnViewcaptionbaronTop();
	afx_msg void OnMenuAutohide();
	afx_msg void OnMenuViewfulldescription();
	afx_msg void OnMenuAllwaysontop();
	afx_msg void OnSortAscending();
	afx_msg void OnSortDescending();
	afx_msg void OnMenuNewGroup();
	afx_msg void OnMenuNewGroupSelection();
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QPASTEWND_H__65261F0F_FEFD_48CF_A0CD_01D8BFEB353B__INCLUDED_)
