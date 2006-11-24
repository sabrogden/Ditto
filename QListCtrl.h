 #if !defined(AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_)
#define AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QListCtrl.h : header file
//
#include "ArrayEx.h"
#include "ToolTipEx.h"
#include "FormattedTextDraw.h"
#include "sqlite/CppSQLite3.h"

#define NM_SELECT					WM_USER+0x100
#define NM_RIGHT					WM_USER+0x101
#define NM_LEFT						WM_USER+0x102
#define NM_END						WM_USER+0x103
#define NM_DELETE					WM_USER+0x104
#define NM_PROPERTIES				WM_USER+0x105
#define NM_LBUTTONDOWN				WM_USER+0x106
#define NM_GETTOOLTIPTEXT	        WM_USER+0x107
#define NM_SELECT_DB_ID		        WM_USER+0x108
#define NM_SELECT_INDEX		        WM_USER+0x109
#define NM_GROUP_TREE_MESSAGE       WM_USER+0x110
#define CB_SEARCH					WM_USER+0x112
#define CB_UPDOWN					WM_USER+0x113
#define NM_INACTIVE_TOOLTIPWND		WM_USER+0x114
#define NM_FILL_REST_OF_LIST		WM_USER+0x115

#define NM_SET_LIST_COUNT			WM_USER+0x116
#define NM_REFRESH_VISIBLE_ROWS		WM_USER+0x117
#define NM_ITEM_DELETED				WM_USER+0x118

#define COPY_BUFFER_HOT_KEY_1_ID	-100
#define COPY_BUFFER_HOT_KEY_2_ID	-101
#define COPY_BUFFER_HOT_KEY_3_ID	-102


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
	virtual int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CQListCtrl();

	// The "FirstTen" block is either at the top or the bottom
	//  of the list based upon m_bStartTop.
	BOOL	m_bShowTextForFirstTenHotKeys;
	BOOL	m_bStartTop;
	// returns the position 1-10 if the index is in the FirstTen block else -1
	int GetFirstTenNum( int index );
	// returns the list index corresponding to the given FirstTen position number.
	int GetFirstTenIndex( int num );

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
	// moves the caret to the given index, selects it, and ensures it is visible.
	BOOL SetListPos(int index);
	bool PutSelectedItemOnDittoCopyBuffer(long lBuffer);

	DWORD GetItemData(int nItem);
	void GetToolTipText(int nItem, CString &csText);

	void SetShowTextForFirstTenHotKeys(BOOL bVal)	{ m_bShowTextForFirstTenHotKeys = bVal;	}

	void DestroyAndCreateAccelerator(BOOL bCreate, CppSQLite3DB &db);

	void ShowFullDescription(bool bFromAuto = false);
	BOOL SetItemCountEx(int iCount, DWORD dwFlags = LVSICF_NOINVALIDATEALL);

	void HidePopup()	{ if(m_pToolTip) m_pToolTip->Hide();	}

	void SetLogFont(LOGFONT &font);

	HWND GetToolTipHWnd();

	BOOL HandleKeyDown(WPARAM wParam, LPARAM lParam);

	BOOL OnItemDeleted(long lID);

protected:
	void SendSelection(int nItem);;
	void LoadCopyOrCutToClipboard();
	void SendSelection(ARRAY &arrItems);
	BOOL GetClipData(int nItem, CClipFormat &Clip);
	BOOL DrawBitMap(int nItem, CRect &crRect, CDC *pDC);
	void LoadDittoCopyBufferHotkeys();

	BOOL DrawText(int nItem, CRect &crRect, CDC *pDC);
		
	WCHAR *m_pwchTip;
	TCHAR *m_pchTip;

	HFONT m_SmallFont;

	//Accelerator
	CAccels	m_Accels;

	CMapIDtoCF m_ThumbNails;
	CMapIDtoCF m_RTFData;

	CToolTipEx *m_pToolTip;

	CFont m_Font;

	IFormattedTextDraw *m_pFormatter;
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CQListCtrl)
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QLISTCTRL_H__30BEB04A_4B97_4943_BB73_C5128E66B4ED__INCLUDED_)
