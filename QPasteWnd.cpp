// QPasteWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QPasteWnd.h"
#include "ProcessPaste.h"
#include "CopyProperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_LIST_HEADER			0x201
#define ID_EDIT_SEARCH			0x202
#define ID_CANCEL				0x203


#define QPASTE_WIDTH			200
#define	QPASTE_HEIGHT			200

/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd

CQPasteWnd::CQPasteWnd()
{	
	m_bHideWnd = true;
	m_Recset.m_strSort = "lDate DESC";
}

CQPasteWnd::~CQPasteWnd()
{
	
}


BEGIN_MESSAGE_MAP(CQPasteWnd, CWndEx)
	//{{AFX_MSG_MAP(CQPasteWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_MENU_LINESPERROW_1, OnMenuLinesperrow1)
	ON_COMMAND(ID_MENU_LINESPERROW_2, OnMenuLinesperrow2)
	ON_COMMAND(ID_MENU_LINESPERROW_3, OnMenuLinesperrow3)
	ON_COMMAND(ID_MENU_LINESPERROW_4, OnMenuLinesperrow4)
	ON_COMMAND(ID_MENU_LINESPERROW_5, OnMenuLinesperrow5)
	ON_COMMAND(ID_MENU_TRANSPARENCY_10, OnMenuTransparency10)
	ON_COMMAND(ID_MENU_TRANSPARENCY_15, OnMenuTransparency15)
	ON_COMMAND(ID_MENU_TRANSPARENCY_20, OnMenuTransparency20)
	ON_COMMAND(ID_MENU_TRANSPARENCY_25, OnMenuTransparency25)
	ON_COMMAND(ID_MENU_TRANSPARENCY_30, OnMenuTransparency30)
	ON_COMMAND(ID_MENU_TRANSPARENCY_40, OnMenuTransparency40)
	ON_COMMAND(ID_MENU_TRANSPARENCY_5, OnMenuTransparency5)
	ON_COMMAND(ID_MENU_TRANSPARENCY_NONE, OnMenuTransparencyNone)
	ON_NOTIFY(NM_RCLICK, ID_LIST_HEADER, OnRclickQuickPaste)
	ON_COMMAND(ID_MENU_DELETE, OnMenuDelete)
	ON_COMMAND(ID_MENU_POSITIONING_ATCARET, OnMenuPositioningAtcaret)
	ON_COMMAND(ID_MENU_POSITIONING_ATCURSOR, OnMenuPositioningAtcursor)
	ON_COMMAND(ID_MENU_POSITIONING_ATPREVIOUSPOSITION, OnMenuPositioningAtpreviousposition)
	ON_COMMAND(ID_MENU_OPTIONS, OnMenuOptions)
	ON_BN_CLICKED(ID_CANCEL, OnCancelFilter)
	ON_COMMAND(ID_MENU_EXITPROGRAM, OnMenuExitprogram)
	ON_COMMAND(ID_MENU_RECONNECTTOCLIPBOARDCHAIN, OnMenuReconnecttoclipboardchain)
	ON_COMMAND(ID_MENU_PROPERTIES, OnMenuProperties)
	ON_WM_CLOSE()
	ON_NOTIFY(LVN_BEGINDRAG, ID_LIST_HEADER, OnBegindrag)
	ON_WM_SYSKEYDOWN()
	ON_NOTIFY(LVN_GETDISPINFO, ID_LIST_HEADER, GetDispInfo)
	ON_NOTIFY(LVN_ODFINDITEM, ID_LIST_HEADER, OnFindItem)
	ON_COMMAND(ID_MENU_FIRSTTENHOTKEYS_USECTRLNUM, OnMenuFirsttenhotkeysUsectrlnum)
	ON_COMMAND(ID_MENU_FIRSTTENHOTKEYS_SHOWHOTKEYTEXT, OnMenuFirsttenhotkeysShowhotkeytext)
	//}}AFX_MSG_MAP
	ON_MESSAGE(NM_SELECT, OnListSelect)
	ON_MESSAGE(NM_END, OnListEnd)
	ON_MESSAGE(CB_SEARCH, OnSearch)
	ON_MESSAGE(NM_DELETE, OnDelete)
	ON_MESSAGE(NM_PROPERTIES, OnProperties)
	ON_NOTIFY(NM_GETTOOLTIPTEXT, ID_LIST_HEADER, OnGetToolTipText)
	ON_MESSAGE(NM_SELECT_DB_ID, OnListSelect_DB_ID)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd message handlers

BOOL CQPasteWnd::Create(const POINT& ptStart, CWnd* pParentWnd) 
{
	// Get the previous size of the QPaste window
	CSize szWnd;
	CGetSetOptions::GetQuickPasteSize(szWnd);
	
	return CWndEx::Create(CRect(ptStart, szWnd), pParentWnd);
}

int CQPasteWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SetWindowText("Ditto");
	
	m_cbSearch.Create(CBS_DROPDOWN | WS_VSCROLL | WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL, 
						CRect(0, 0, 0, 0), this, ID_EDIT_SEARCH);
	
	// Create the header control
	if (!m_lstHeader.Create(WS_TABSTOP|WS_CHILD|WS_VISIBLE|LVS_NOCOLUMNHEADER|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_OWNERDATA,
		CRect(0, 0, 0, 0),	this, ID_LIST_HEADER))
	{
		ASSERT(FALSE);
		return -1;
	}

	m_btCancel.Create("&C", WS_CHILD|BS_PUSHBUTTON|WS_TABSTOP/*|BS_FLAT*/, CRect(0, 0, 0, 0), this, ID_CANCEL);

	//Set the z-order
	m_lstHeader.SetWindowPos(this, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	m_cbSearch.SetWindowPos(&m_lstHeader, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	m_btCancel.SetWindowPos(&m_cbSearch, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

	//LVS_EX_FLATSB
	m_lstHeader.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

	// Create the columns
	if (m_lstHeader.InsertColumn(0, "", LVCFMT_LEFT, 0, 0) != 0)
	{
		ASSERT(FALSE);
		return -1;
	}

	#ifdef AFTER_98
		m_Alpha.SetWindowHandle(m_hWnd);	
	#endif
	
	//Show the window
	ShowQPasteWindow(FALSE);
	
	m_TitleFont.CreateFont(14,0,-900,0,400,FALSE,FALSE,0,ANSI_CHARSET,
	OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
	DEFAULT_PITCH|FF_SWISS,"Arial");
	
	m_SearchFont.CreatePointFont(80, "MS Sans Serif");

	m_cbSearch.SetFont(&m_SearchFont);
	m_btCancel.SetFont(&m_SearchFont);

	
	return 0;
}

void CQPasteWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWndEx::OnSize(nType, cx, cy);

	if(!IsWindow(m_lstHeader.m_hWnd))
		return;

	//Create the region for drawing the rounded top edge
	CRect rect;
	GetWindowRect(rect);
	CRgn rgnRect, rgnRect2, rgnRound, rgnFinalA, rgnFinalB;

	rgnRect.CreateRectRgn(0, 0, 20, rect.Height());
	rgnRound.CreateRoundRectRgn(0, 0, rect.Width(), rect.Height()+1, 15, 15);

	rgnFinalB.CreateRectRgn(0, 0, 0, 0);
	rgnFinalB.CombineRgn(&rgnRect, &rgnRound, RGN_OR);

	rgnRect2.CreateRectRgn(0, 20, rect.Width()-1, rect.Height());
	rgnFinalA.CreateRectRgn(0, 0, 0, 0);
	rgnFinalA.CombineRgn(&rgnRect2, &rgnFinalB, RGN_OR);

	//Set the region
	SetWindowRgn(rgnFinalA, TRUE);

	MoveControls();
}

void CQPasteWnd::MoveControls()
{
	CRect crRect;
	GetClientRect(crRect);
	int cx = crRect.Width();
	int cy = crRect.Height();

	// Resize the list control
	m_lstHeader.MoveWindow(0, 0, cx, cy - 22);

	int nWidth = cx;

	if(m_Recset.m_strFilter.IsEmpty() == FALSE)
	{
		m_btCancel.MoveWindow(cx - 20, cy - 20, 20, 20);
		nWidth -= 19;
	}

	m_cbSearch.MoveWindow(0, cy - 22, nWidth, 100);

	// Set the column widths
	//m_lstHeader.SetColumnWidth(0, cx);
}

void CQPasteWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWndEx::OnSetFocus(pOldWnd);

	// Set the focus to the list control
	m_lstHeader.SetFocus();
}

void CQPasteWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWndEx::OnActivate(nState, pWndOther, bMinimized);
	
	if(m_bHideWnd == false)
		return;

	if (nState == WA_INACTIVE)
	{
		HideQPasteWindow();
	}
	else if (nState == WA_ACTIVE)
	{
		ShowQPasteWindow();
	}
}

BOOL CQPasteWnd::HideQPasteWindow()
{
	if(m_cbSearch.GetShowingDropDown())
		return FALSE;

	if(m_Recset.IsOpen())
		m_Recset.Close();

	//Reset the flag
	theApp.m_bShowingQuickPaste = false;

	m_lstHeader.DestroyAndCreateAccelerator(FALSE);

	//Save the size
	CRect rect;
	GetWindowRect(rect);
	CGetSetOptions::SetQuickPasteSize(rect.Size());
	CGetSetOptions::SetQuickPastePoint(rect.TopLeft());

	// Hide the window when the focus is lost
	ShowWindow(SW_HIDE);

	//Rest the selection on the search combo
	m_cbSearch.SetCurSel(-1);

	//Set focus back
	if(m_hWndFocus)
	{
		::SetForegroundWindow(m_hWndFocus);
		::SetFocus(m_hWndFocus);
	}

	return TRUE;
}

BOOL CQPasteWnd::ShowQPasteWindow(BOOL bFillList)
{
	//Set the flag so we can't open this up again
	theApp.m_bShowingQuickPaste = true;

	m_lstHeader.DestroyAndCreateAccelerator(TRUE);

	m_bHideWnd = true;

	#ifdef AFTER_98
		//Set the transparency
		if(CGetSetOptions::GetEnableTransparency())
		{
			m_Alpha.SetTransparent(TRUE);

			float fPercent =  CGetSetOptions::GetTransparencyPercent() / (float)100.0;

			m_Alpha.SetOpacity(OPACITY_MAX - (int)(fPercent * OPACITY_MAX));
		}
		else
		{
			m_Alpha.SetTransparent(FALSE);
		}
	#endif

	m_lstHeader.SetNumberOfLinesPerRow(CGetSetOptions::GetLinesPerRow());

	m_lstHeader.SetShowTextForFirstTenHotKeys(CGetSetOptions::GetShowTextForFirstTenHotKeys());
	
	if(bFillList)
		FillList();

	m_btCancel.ShowWindow(SW_HIDE);
	
	MoveControls();
	
	ShowWindow(SW_SHOW);

	return TRUE;
}

bool CQPasteWnd::Add(const CString &csHeader, const CString &csText, int nID)
{
	int nNewIndex;
	
	// Insert the item in the list control
	if ((nNewIndex = m_lstHeader.InsertItem(m_lstHeader.GetItemCount(), csHeader)) == -1)
		return false;

	m_lstHeader.SetItemData(nNewIndex, nID);
	
	return true;
}

LRESULT CQPasteWnd::OnListSelect_DB_ID(WPARAM wParam, LPARAM lParam)
{
	ShowWindow(SW_HIDE);

	CProcessPaste past;
	past.LoadDataAndPaste(wParam, m_hWndFocus);

	return TRUE;
}

LRESULT CQPasteWnd::OnListSelect(WPARAM wParam, LPARAM lParam)
{
	int nCount = (int) wParam;
	long *pItems = (long*) lParam;

	
	if(nCount)
	{			
		int nID = m_lstHeader.GetItemData(pItems[0]);

		ShowWindow(SW_HIDE);

		CProcessPaste past;
		past.LoadDataAndPaste(nID, m_hWndFocus);
	}

	return TRUE;
}


LRESULT CQPasteWnd::OnListEnd(WPARAM wParam, LPARAM lParam)
{
	HideQPasteWindow();

	return 0;
}

BOOL CQPasteWnd::FillList(CString csSQLSearch/*=""*/)
{
//	if(m_Recset.IsOpen())
//		m_Recset.Close();

	m_lstHeader.DeleteAllItems();

	CRect crRect;
	GetClientRect(crRect);

	// Create the columns
	if(m_lstHeader.InsertColumn(0, "", LVCFMT_LEFT, 2500, 0) != 0)
	{	
		ASSERT(FALSE);	
		return -1;	
	}

	CString csSQL;
	if(csSQLSearch == "")
	{
		m_Recset.m_strFilter = "";
		if(m_Recset.IsOpen())
			m_Recset.Requery();		
	}
	else
	{
		//Replace all single ' with a double '
		csSQLSearch.Replace("'", "''");

		//Can't query of strings that have '|' in them
		//this should be removed later
		if(csSQLSearch.Find("|") >= 0)
			return FALSE;

		m_Recset.m_strFilter.Format("strText LIKE \'*%s*\'", csSQLSearch);
		if(m_Recset.IsOpen())
			m_Recset.Requery();
	}

	try
	{
		if(m_Recset.IsOpen() == FALSE)
			m_Recset.Open("");
					
		if(!m_Recset.IsEOF())
		{
			m_Recset.MoveLast();
			m_lstHeader.SetItemCountEx(m_Recset.GetRecordCount());

			m_lstHeader.LoadFirstTenHotKeys(m_Recset);
		}	
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
	}

	m_lstHeader.SetSelection(0);
	m_lstHeader.SetCaret(0);

	m_lstHeader.Invalidate();
		
	return TRUE;
}



void CQPasteWnd::OnRclickQuickPaste(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POINT pp;
	CMenu cmPopUp;
	CMenu *cmSubMenu = NULL;
	
	GetCursorPos(&pp);
	if(cmPopUp.LoadMenu(IDR_QUICK_PASTE) != 0)
	{
		cmSubMenu = cmPopUp.GetSubMenu(0);
		if(!cmSubMenu)
			return;

		SetMenuChecks(cmSubMenu);

		cmSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
				pp.x, pp.y, this, NULL);
	}
	
	*pResult = 0;
}

void CQPasteWnd::SetMenuChecks(CMenu *pMenu)
{
	//Set the transparency Check
	if(!CGetSetOptions::GetEnableTransparency())
	{
		pMenu->CheckMenuItem(ID_MENU_TRANSPARENCY_NONE, MF_CHECKED);
	}
	else
	{
		int nTrans = CGetSetOptions::GetTransparencyPercent();
		int nCheckID = -1;
		switch(nTrans)
		{
		case 5:
			nCheckID = ID_MENU_TRANSPARENCY_5;
			break;
		case 10:
			nCheckID = ID_MENU_TRANSPARENCY_10;
			break;
		case 15:
			nCheckID = ID_MENU_TRANSPARENCY_15;
			break;
		case 20:
			nCheckID = ID_MENU_TRANSPARENCY_20;
			break;
		case 25:
			nCheckID = ID_MENU_TRANSPARENCY_25;
			break;
		case 30:
			nCheckID = ID_MENU_TRANSPARENCY_30;
			break;
		case 40:
			nCheckID = ID_MENU_TRANSPARENCY_40;
			break;
		}
		if(nCheckID >= 0)
			pMenu->CheckMenuItem(nCheckID, MF_CHECKED);
	}

	//Set the lines per row check
	int nLinesPerRow = CGetSetOptions::GetLinesPerRow();
	int nCheckID = -1;
	switch(nLinesPerRow)
	{
	case 1:
		nCheckID = ID_MENU_LINESPERROW_1;
		break;
	case 2:
		nCheckID = ID_MENU_LINESPERROW_2;
		break;
	case 3:
		nCheckID = ID_MENU_LINESPERROW_3;
		break;
	case 4:
		nCheckID = ID_MENU_LINESPERROW_4;
		break;
	case 5:
		nCheckID = ID_MENU_LINESPERROW_5;
		break;
	}
	if(nCheckID >= 0)
		pMenu->CheckMenuItem(nCheckID, MF_CHECKED);

	//Set the position check
	int nPosition = CGetSetOptions::GetQuickPastePosition();
	nCheckID = -1;
	switch(nPosition)
	{
	case POS_AT_CARET:
		nCheckID = ID_MENU_POSITIONING_ATCARET;
		break;
	case POS_AT_CURSOR:
		nCheckID = ID_MENU_POSITIONING_ATCURSOR;
		break;
	case POS_AT_PREVIOUS:
		nCheckID = ID_MENU_POSITIONING_ATPREVIOUSPOSITION;
		break;
	}
	if(nCheckID >= 0)
		pMenu->CheckMenuItem(nCheckID, MF_CHECKED);

	if(::SendMessage(theApp.m_MainhWnd, WM_IS_TOP_VIEWER, 0, 0))
		pMenu->DeleteMenu(ID_MENU_RECONNECTTOCLIPBOARDCHAIN, MF_BYCOMMAND);

	if(CGetSetOptions::GetShowTextForFirstTenHotKeys())
		pMenu->CheckMenuItem(ID_MENU_FIRSTTENHOTKEYS_SHOWHOTKEYTEXT, MF_CHECKED);

	if(CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys())
		pMenu->CheckMenuItem(ID_MENU_FIRSTTENHOTKEYS_USECTRLNUM, MF_CHECKED);
}

LRESULT CQPasteWnd::OnSearch(WPARAM wParam, LPARAM lParam)
{
	CString csText;
	m_cbSearch.GetWindowText(csText);
	
	if(csText == "")
		return FALSE;

	FillList(csText);

	m_lstHeader.SetFocus();

	m_btCancel.ShowWindow(SW_SHOW);

	MoveControls();

	m_cbSearch.SetEditSel(-1, 0);

	return TRUE;
}


///////////////////////////////////////////////////////////////////////
//Menu Stuff
///////////////////////////////////////////////////////////////////////
void CQPasteWnd::OnMenuLinesperrow1()	{ SetLinesPerRow(1);	}
void CQPasteWnd::OnMenuLinesperrow2()	{ SetLinesPerRow(2);	}
void CQPasteWnd::OnMenuLinesperrow3()	{ SetLinesPerRow(3);	}
void CQPasteWnd::OnMenuLinesperrow4()	{ SetLinesPerRow(4);	}
void CQPasteWnd::OnMenuLinesperrow5()	{ SetLinesPerRow(5);	}
void CQPasteWnd::SetLinesPerRow(long lLines)
{
	CGetSetOptions::SetLinesPerRow(lLines);
	m_lstHeader.SetNumberOfLinesPerRow(lLines);
	
	FillList();
}
void CQPasteWnd::OnMenuTransparencyNone()	{ SetTransparency(0);	}
void CQPasteWnd::OnMenuTransparency5()		{ SetTransparency(5);	}
void CQPasteWnd::OnMenuTransparency10()		{ SetTransparency(10);	}
void CQPasteWnd::OnMenuTransparency15()		{ SetTransparency(15);	}
void CQPasteWnd::OnMenuTransparency20()		{ SetTransparency(20);	}
void CQPasteWnd::OnMenuTransparency25()		{ SetTransparency(25);	}
void CQPasteWnd::OnMenuTransparency30()		{ SetTransparency(25);	}
void CQPasteWnd::OnMenuTransparency40()		{ SetTransparency(40);	}

void CQPasteWnd::SetTransparency(long lPercent)
{
	#ifdef AFTER_98
		if(lPercent)
		{
			CGetSetOptions::SetTransparencyPercent(lPercent);
			CGetSetOptions::SetEnableTransparency(TRUE);

			m_Alpha.SetTransparent(TRUE);

			float fPercent =  lPercent / (float)100.0;

			m_Alpha.SetOpacity(OPACITY_MAX - (int)(fPercent * OPACITY_MAX));
		}
		else
		{
			CGetSetOptions::SetEnableTransparency(FALSE);
			m_Alpha.SetTransparent(FALSE);
		}
	#endif
}

void CQPasteWnd::OnMenuDelete() 
{
	DeleteSelectedRows();
}

void CQPasteWnd::OnMenuPositioningAtcaret() 
{
	CGetSetOptions::SetQuickPastePosition(POS_AT_CARET);
}

void CQPasteWnd::OnMenuPositioningAtcursor() 
{
	CGetSetOptions::SetQuickPastePosition(POS_AT_CURSOR);
}

void CQPasteWnd::OnMenuPositioningAtpreviousposition() 
{
	CGetSetOptions::SetQuickPastePosition(POS_AT_PREVIOUS);
}

void CQPasteWnd::OnMenuOptions() 
{
	m_bHideWnd = false;

	DoOptions(this);

	ShowQPasteWindow(TRUE);
	
	m_bHideWnd = true;
	m_lstHeader.SetFocus();
}

void CQPasteWnd::OnMenuExitprogram() 
{
	::SendMessage(theApp.m_MainhWnd, WM_CLOSE, 0, 0);
}

void CQPasteWnd::OnMenuReconnecttoclipboardchain() 
{
	::SendMessage(theApp.m_MainhWnd, WM_RECONNECT_TO_COPY_CHAIN, 0, 0);
}

void CQPasteWnd::OnMenuProperties() 
{
	m_bHideWnd = false;

	ARRAY IDs, Indexes;
	m_lstHeader.GetSelectionItemData(IDs);
	m_lstHeader.GetSelectionIndexes(Indexes);

	int nSize = IDs.GetSize();
	if(nSize < 1)
		return;

	long lID = IDs[0];
	int nRow = Indexes[0];

	if(lID < 0)
		return;
	
	m_lstHeader.RemoveAllSelection();
	m_lstHeader.SetSelection(nRow);

	CCopyProperties props(lID, this);
	int nDo = props.DoModal();

	if(nDo == IDOK)
	{
		m_lstHeader.DestroyAndCreateAccelerator(TRUE);
		m_lstHeader.RefreshVisibleRows();
	}
		
	SetFocus();
	m_bHideWnd = true;

	if(nDo == IDOK || nDo == IDCANCEL)
	{
		m_lstHeader.SetFocus();
		m_lstHeader.SetCaret(nRow);
	}
}

void CQPasteWnd::OnMenuFirsttenhotkeysUsectrlnum() 
{	
	CGetSetOptions::SetUseCtrlNumForFirstTenHotKeys(!CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys());
	m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuFirsttenhotkeysShowhotkeytext() 
{
	CGetSetOptions::SetShowTextForFirstTenHotKeys(!CGetSetOptions::GetShowTextForFirstTenHotKeys());

	m_lstHeader.SetShowTextForFirstTenHotKeys(CGetSetOptions::GetShowTextForFirstTenHotKeys());

	m_lstHeader.RefreshVisibleRows();
}

///////////////////////////////////////////////////////////////////////
//END END Menu Stuff
///////////////////////////////////////////////////////////////////////


LRESULT CQPasteWnd::OnDelete(WPARAM wParam, LPARAM lParam)
{
	DeleteSelectedRows();
	
	return TRUE;
}

void CQPasteWnd::DeleteSelectedRows()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if(m_Recset.DeleteRows(IDs))
	{
		m_Recset.Requery();

		long lCount = 0;
		if(m_Recset.IsEOF() == FALSE)
		{
			m_Recset.MoveLast();
			lCount = m_Recset.GetRecordCount();
			m_lstHeader.LoadFirstTenHotKeys(m_Recset);
		}

		m_lstHeader.SetItemCountEx(lCount);
		m_lstHeader.Invalidate();
	}

	int nCurSel = m_lstHeader.GetCaret();
	if(nCurSel < 0) 
		nCurSel = 0;

	m_lstHeader.RemoveAllSelection();

	m_lstHeader.SetSelection(nCurSel);
	m_lstHeader.SetCaret(nCurSel);

	m_lstHeader.RefreshVisibleRows();
}


BOOL CQPasteWnd::PreTranslateMessage(MSG* pMsg) 
{	
	switch(pMsg->message) 
	{
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_ESCAPE)
		{
			if(!m_cbSearch.GetShowingDropDown())
			{
				HideQPasteWindow();

				return TRUE;
			}
		}
		else if(pMsg->wParam == VK_TAB)
		{
			BOOL bPrev = FALSE;

			if(GetKeyState(VK_SHIFT) & 0x8000)
				bPrev = TRUE;

			CWnd *pFocus = GetFocus();
			if(pFocus)
			{
				CWnd *pNextWnd = GetNextDlgTabItem(pFocus, bPrev);
				if(pNextWnd)
					pNextWnd->SetFocus();
			}

			return TRUE;
		}
		else if(pMsg->wParam == 'A')
		{
			if(GetKeyState(VK_CONTROL) & 0x8000)
			{
				int nCount = m_lstHeader.GetItemCount();
				for(int i = 0; i < nCount; i++)
				{
					m_lstHeader.SetSelection(i);
				}
				return TRUE;
			}
		}
		break;
	case WM_SYSKEYDOWN:
		if(pMsg->wParam == 'C')
		{
			BYTE key[256];
			GetKeyboardState((LPBYTE)(&key));
			if(key[VK_MENU]&128)
			{
				OnCancelFilter();				
			}
		}
		break;
	}

	
	return CWndEx::PreTranslateMessage(pMsg);
}

void CQPasteWnd::OnCancelFilter()
{
	FillList();

	m_btCancel.ShowWindow(SW_HIDE);

	m_cbSearch.SetCurSel(-1);
	MoveControls();

	m_lstHeader.SetFocus();
}

LRESULT CQPasteWnd::OnProperties(WPARAM wParam, LPARAM lParam)
{
	OnMenuProperties();
	return TRUE;
}

void CQPasteWnd::OnClose() 
{
	HideQPasteWindow();
}

void CQPasteWnd::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
    int nItem =  ((NM_LISTVIEW*) pNMHDR)->iItem;

	long lID = m_lstHeader.GetItemData(nItem);

	if(lID >= 0)
	{
		CProcessPaste past;
		past.LoadDataAndDrag(lID);
	}

	*pResult = 0;
}

void CQPasteWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{	
	CWndEx::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CQPasteWnd::GetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(m_Recset.IsOpen() == FALSE)
		return;
	
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;
	
	if(pItem->mask & LVIF_TEXT) //valid text buffer?
	{		
		switch(pItem->iSubItem)
		{
		case 0:
			try
			{
				m_Recset.SetAbsolutePosition(pItem->iItem);

				CString cs;
				if(m_Recset.m_lDontAutoDelete)
					cs = "* ";

				if(m_Recset.m_lShortCut > 0)
					cs += "** ";

				cs += m_Recset.m_strText;
				lstrcpy(pItem->pszText, cs);
			}
			catch(CDaoException *e)
			{
				ASSERT(FALSE);
				e->Delete();
			}
		break;
		}
	}

	if(pItem->mask & LVIF_PARAM)
	{
		switch(pItem->iSubItem)
		{
		case 0:
			try
			{
				m_Recset.SetAbsolutePosition(pItem->iItem);
				pItem->lParam = m_Recset.m_lID;
			}
			catch(CDaoException *e)
			{
				ASSERT(FALSE);
				e->Delete();
			}			
		break;
		}
	}
}

void CQPasteWnd::OnGetToolTipText(NMHDR* pNMHDR, LRESULT* pResult)
{
	CQListToolTipText* pInfo = (CQListToolTipText*)pNMHDR;
	if(!pInfo)
		return;

	try
	{
		CString cs;
		
		m_Recset.SetAbsolutePosition(pInfo->lItem);

		cs = m_Recset.m_strText;
		cs += "\n\n";
		CTime time(m_Recset.m_lDate);
		cs += time.Format("%m/%d/%Y %I:%M %p");

		if(m_Recset.m_lDontAutoDelete)
		{
			cs += "\n";
			cs += "Never Auto Delete";
		}

		if(m_Recset.m_lShortCut > 0)
		{
			cs += "\n";
			
			if( HIBYTE(m_Recset.m_lShortCut) & HOTKEYF_CONTROL )
				cs += "Ctrl + ";
			if( HIBYTE(m_Recset.m_lShortCut) & HOTKEYF_SHIFT )
				cs += "Shift + ";
			if( HIBYTE(m_Recset.m_lShortCut) & HOTKEYF_ALT )
				cs += "Alt + ";

			cs += LOBYTE(m_Recset.m_lShortCut);
		}

		strcpy(pInfo->cText, cs);

	}
	catch(CDaoException *e)
	{
		ASSERT(FALSE);
		e->Delete();
	}
}

void CQPasteWnd::OnFindItem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)pNMHDR;
	LVFINDINFO    fndItem   = pFindInfo->lvfi;

	if(fndItem.flags & LVFI_STRING)
	{
        int   nLength = strlen(fndItem.psz);
        long lArraySize = m_Recset.GetRecordCount();

		m_Recset.SetAbsolutePosition(pFindInfo->iStart);
		// Search to end.
		for(int i = pFindInfo->iStart; i < lArraySize; i++ )
		{
			if(m_Recset.IsEOF() == FALSE)
			{
				if(_strnicmp(fndItem.psz, m_Recset.m_strText, nLength) == 0)
				{
					*pResult = i;
					return;
				}
				m_Recset.MoveNext();
			}
		}

		m_Recset.MoveFirst();

		// Search from 0 to start.
		for( i = 0; i < pFindInfo->iStart; i++ )
		{
			if(m_Recset.IsEOF() == FALSE)
			{
				if(_strnicmp(fndItem.psz, m_Recset.m_strText, nLength) == 0)
				{
					*pResult = i;
					return;
				}

				m_Recset.MoveNext();
			}
		}
	}

	*pResult = -1;	// Default action.
}
