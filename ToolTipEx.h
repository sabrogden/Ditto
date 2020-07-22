#pragma once

#include "RichEditCtrlEx.h"
#include "WndEx.h"
#include "DittoWindow.h"
#include "GdipButton.h"
#include "ImageViewer.h"
#include "GroupStatic.h"
#include "Accels.h"
#include "SnapWindow.h"
#include "SimpleBrowser.h"

class CToolTipEx : public CWnd
{
// Construction
public:
	CToolTipEx();

// Attributes
public:

// Operations
public:
	BOOL OnMsg(MSG* pMsg);
	BOOL Create(CWnd* pParentWnd);
	BOOL Show(CPoint point);
	BOOL Hide();
	void SetToolTipText(const CString &csText);
	void SetRTFText(const CStringA &rtf);
	void SetHtmlText(const CString &html);
	void SetGdiplusBitmap(Gdiplus::Bitmap *gdiplusBitmap);
	void SetNotifyWnd(CWnd *pNotify)		{ m_pNotifyWnd = pNotify;	}
	void HideWindowInXMilliSeconds(long lms);
	CRect GetBoundsRect();

	void SetClipId(int clipId) { m_clipId = clipId; }
	int GetClipId() { return m_clipId; }

	void SetClipRow(int clipRow) { m_clipRow = clipRow; }
	int GetClipRow() { return m_clipRow; }

	void SetSearchText(CString text) { m_searchText = text; }

	void SetClipData(CString data) { m_clipData = data; m_originalClipData = data; }
	void SetFolderPath(CString path) { m_folderPath = path; }

	bool GetShowPersistant() { return m_showPersistant; }
	void ToggleShowPersistant() { OnFirstAlwaysontop(); }
	bool ToggleWordWrap();
	void SetTooltipActions(CAccels *pToolTipActions) { m_pToolTipActions = pToolTipActions; }

	void GetWindowRectEx(LPRECT lpRect);

	void UpdateMenuShortCut(CMenu *subMenu, int id, DWORD action);

	void DoSearch();
	void MoveControls();

	BOOL SetLogFont(LPLOGFONT lpLogFont, BOOL bRedraw /*=TRUE*/);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolTipEx)
	protected:
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolTipEx();
	
protected:
	DWORD m_dwTextStyle;
	CRect m_rectMargin;
	CString m_csText;
	CFont m_Font;
	int m_fontHeight;
	CString m_csRTF;
	CString m_html;
	CRichEditCtrlEx m_RichEdit;
	SimpleBrowser m_browser;
	CWnd *m_pNotifyWnd;
	CGdipButton m_optionsButton;
	int m_clipId;
	CString m_searchText;
	CScrollBar m_vScroll;
	CScrollBar m_hScroll;
	CDittoWindow m_DittoWindow;
	CImageViewer m_imageViewer;
	CGroupStatic m_clipDataStatic;
	CGroupStatic m_folderPathStatic;
	CString m_clipData;
	CString m_originalClipData;
	CFont m_clipDataFont;
	bool m_saveWindowLockout;
	int m_clipRow;
	bool m_showPersistant;
	CAccels *m_pToolTipActions;
	bool m_bMaxSetTimer;
	int m_lDelayMaxSeconds;
	SnapWindow m_snap;
	CString m_folderPath;

protected:
	CString GetFieldFromString(CString ref, int nIndex, TCHAR ch);	
	BOOL IsCursorInToolTip();
	void HighlightSearchText();	
	void ApplyWordWrap();
	void SaveWindowSize();
	CString m_mouseDownOnLink;

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolTipEx)	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HITTEST_RET OnNcHitTest(CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point); 
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point); 
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp); 
	afx_msg void OnNcPaint();
	afx_msg void OnOptions();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRememberwindowposition();
	afx_msg void OnSizewindowtocontent();
	afx_msg void OnScaleimagestofitwindow();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();	
	afx_msg void OnFirstHidedescriptionwindowonm();
	afx_msg void OnFirstWraptext();
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnFirstAlwaysontop();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void OnEnMsgfilterRichedit21(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnEnterSizeMove();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT OnRefreshFooter(WPARAM wParam, LPARAM lParam);
};