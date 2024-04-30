 #if !defined(AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_)
#define AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QListCtrl.h : header file
//
#include "Shared/ArrayEx.h"
#include "ToolTipEx.h"
#include "FormattedTextDraw.h"
#include "sqlite/CppSQLite3.h"
#include "ClipFormatQListCtrl.h"
#include "Accels.h"
#include "GdiImageDrawer.h"
#include "DPI.h"

#define NM_SEARCH_ENTER_PRESSED		WM_USER+0x100
#define NM_RIGHT					WM_USER+0x101
#define NM_LEFT						WM_USER+0x102
#define NM_END						WM_USER+0x103
#define NM_DELETE					WM_USER+0x104
#define NM_PROPERTIES				WM_USER+0x105
#define NM_LBUTTONDOWN				WM_USER+0x106
#define NM_GETTOOLTIPTEXT	        WM_USER+0x107
#define NM_SELECT_DB_ID		        WM_USER+0x108
#define NM_GROUP_TREE_MESSAGE       WM_USER+0x110
#define CB_SEARCH					WM_USER+0x112
#define CB_UPDOWN					WM_USER+0x113
#define NM_INACTIVE_TOOLTIPWND		WM_USER+0x114
#define NM_FILL_REST_OF_LIST		WM_USER+0x115

#define NM_SET_LIST_COUNT			WM_USER+0x116
#define NM_ITEM_DELETED				WM_USER+0x118
#define NM_ALL_SELECTED				WM_USER+0x119
#define NM_REFRESH_ROW				WM_USER+0x120
#define NM_REFRESH_ROW_EXTRA_DATA	WM_USER+0x121
#define NM_SHOW_HIDE_SCROLLBARS		WM_USER+0x122
#define NM_CANCEL_SEARCH			WM_USER+0x123
#define NM_POST_OPTIONS_WINDOW		WM_USER+0x124
#define NM_SHOW_PROPERTIES			WM_USER+0x125
#define NM_NEW_GROUP				WM_USER+0x126
#define NM_DELETE_ID				WM_USER+0x127
#define NM_MOVE_TO_GROUP			WM_USER+0x128
#define NM_FOCUS_ON_SEARCH			WM_USER+0x129
#define NM_COPY_CLIP				WM_USER+0x130



#define COPY_BUFFER_HOT_KEY_1_ID	-100
#define COPY_BUFFER_HOT_KEY_2_ID	-101
#define COPY_BUFFER_HOT_KEY_3_ID	-102

#define LVIF_CF_DIB 0x10000000
#define LVIF_CF_RICHTEXT 0x10000000


//#define NM_LIST_CUT			        WM_USER+0x111
//#define NM_LIST_COPY		        WM_USER+0x112
//#define NM_LIST_PASTE		        WM_USER+0x113

class CQListToolTipText
{
public:
	NMHDR hdr;
	long lItem;
	LPTSTR pszText; 
	int cchTextMax; 
};



typedef CMap<long, long, CClipFormat, CClipFormat&> CMapIDtoCF;

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
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CQListCtrl();

	BOOL	m_bShowTextForFirstTenHotKeys;
	// returns the position 1-10 if the index is in the FirstTen block else -1
	int GetFirstTenNum( int index );

	void SetNumberOfLinesPerRow(int nLines, bool force);
	void GetSelectionIndexes(ARRAY &arr);
	void GetSelectionItemData(ARRAY &arr);
	void RefreshVisibleRows();
	void RefreshRow(int row);
	void RemoveAllSelection();
	BOOL SetSelection(int nRow, BOOL bSelect = TRUE);
	BOOL SetText(int nRow, int nCol, CString cs);
	BOOL SetFormattedText(int nRow, int nCol, LPCTSTR lpszFormat,...);
	BOOL SetCaret(int nRow, BOOL bFocus = TRUE);
	long GetCaret();
	// moves the caret to the given index, selects it, and ensures it is visible.
	BOOL SetListPos(int index);
	bool PutSelectedItemOnDittoCopyBuffer(long lBuffer);

	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	DWORD GetItemData(int nItem);
	CClipFormatQListCtrl* GetItem_CF_DIB_ClipFormat(int nItem);
	CClipFormatQListCtrl* GetItem_CF_RTF_ClipFormat(int nItem);
	void GetToolTipText(int nItem, CString &csText);

	void SetShowTextForFirstTenHotKeys(BOOL bVal)	{ m_bShowTextForFirstTenHotKeys = bVal;	}
	void SetShowIfClipWasPasted(BOOL val) { m_showIfClipWasPasted = val; }

	void DestroyAndCreateAccelerator(BOOL bCreate, CppSQLite3DB &db);

	bool PostEventLoadedCheckDescription(int updatedRow);
	bool ShowFullDescription(bool bFromAuto = false, bool fromNextPrev = false);
	BOOL SetItemCountEx(int iCount, DWORD dwFlags = 0);

	void HidePopup(bool checkShowPersistant);
	void ToggleToolTipShowPersistant();
	bool ToggleToolTipWordWrap();
	void SetTooltipActions(CAccels *pToolTipActions) { m_pToolTipActions = pToolTipActions; }
	bool IsToolTipShowPersistant();
	void DoToolTipSearch();
	void HideToolTip();

	void SetLogFont(LOGFONT &font);

	HWND GetToolTipHWnd();

	BOOL HandleKeyDown(WPARAM wParam, LPARAM lParam);

	BOOL OnItemDeleted(long lID);

	BOOL IsToolTipWindowVisible();
	BOOL IsToolTipWindowFocus();

	int GetRowHeight() { return m_rowHeight; }

	void SetSearchText(CString text);

	void SetDpiInfo(CDPI *dpi);

	void CreateSmallFont();

	void OnDpiChanged();
	
	void LoadCopyOrCutToClipboard();

protected:
	BOOL GetClipData(int nItem, CClipFormat &Clip);
	BOOL DrawBitMap(int nItem, CRect &crRect, CDC *pDC, const CString &csDescription);
	void LoadDittoCopyBufferHotkeys();
	bool MouseInScrollBarArea(CRect crWindow, CPoint point);
	BOOL DrawRtfText(int nItem, CRect &crRect, CDC *pDC);
	void StopHideScrollBarTimer();
		
	WCHAR *m_pwchTip;
	TCHAR *m_pchTip;
	HFONT m_SmallFont;
	CAccels	m_Accels;
	CMapIDtoCF m_RTFData;
	CToolTipEx *m_pToolTip;
	HWND m_toolTipHwnd;
	CFont m_Font;
	CFont m_boldFont;
	IFormattedTextDraw *m_pFormatter;
	bool m_allSelected;
	int m_linesPerRow;
	DWORD m_mouseOverScrollAreaStart;
	bool m_timerToHideScrollAreaSet;
	CGdiImageDrawer m_groupFolder;
	CGdiImageDrawer m_dontDeleteImage;
	CGdiImageDrawer m_inFolderImage;
	CGdiImageDrawer m_shortCutImage;
	CGdiImageDrawer m_stickyImage;
	int m_rowHeight;
	CString m_searchText;
	BOOL m_showIfClipWasPasted;
	CAccels *m_pToolTipActions;
	CRichEditCtrlEx m_rtfFormater;
	CDPI *m_windowDpi;


	// Generated message map functions
protected:
	//{{AFX_MSG(CQListCtrl)
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
	void DrawCopiedColorCode(CString& csText, CRect& rcText, CDC* pDC);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	afx_msg BOOL OnToolTipText(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_)
