// QPasteWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QPasteWnd.h"
#include "ProcessPaste.h"
#include "CopyProperties.h"
#include ".\qpastewnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define QPASTE_TITLE			"Ditto"

#define ID_LIST_HEADER			0x201
#define ID_EDIT_SEARCH			0x202
#define ID_CANCEL				0x203


#define QPASTE_WIDTH			200
#define	QPASTE_HEIGHT			200

/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd

CQPasteWnd::CQPasteWnd()
{	
	m_Title = QPASTE_TITLE;
	m_bHideWnd = true;
	m_bAscending = false;
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
	ON_MESSAGE(NM_SELECT_INDEX, OnListSelect_Index)
	ON_MESSAGE(WM_REFRESH_VIEW, OnRefreshView)
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_WINDOWPOSCHANGING()
	ON_COMMAND(ID_VIEWCAPTIONBARON_RIGHT, OnViewcaptionbaronRight)
	ON_COMMAND(ID_VIEWCAPTIONBARON_BOTTOM, OnViewcaptionbaronBottom)
	ON_COMMAND(ID_VIEWCAPTIONBARON_LEFT, OnViewcaptionbaronLeft)
	ON_COMMAND(ID_VIEWCAPTIONBARON_TOP, OnViewcaptionbaronTop)
	ON_COMMAND(ID_MENU_AUTOHIDE, OnMenuAutohide)
	ON_COMMAND(ID_MENU_VIEWFULLDESCRIPTION, OnMenuViewfulldescription)
	ON_COMMAND(ID_MENU_ALLWAYSONTOP, OnMenuAllwaysontop)
	ON_COMMAND(ID_SORT_ASCENDING, OnSortAscending)
	ON_COMMAND(ID_SORT_DESCENDING, OnSortDescending)
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
	
	SetWindowText( QPASTE_TITLE );

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
	if (m_lstHeader.InsertColumn(0, "", LVCFMT_LEFT, 2500, 0) != 0)
	{
		ASSERT(FALSE);
		return -1;
	}

	#ifdef AFTER_98
		m_Alpha.SetWindowHandle(m_hWnd);	
	#endif
	
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
	if(::IsWindow(m_lstHeader.m_hWnd))
		m_lstHeader.SetFocus();
}

void CQPasteWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWndEx::OnActivate(nState, pWndOther, bMinimized);
	
	if(m_bHideWnd == false)
		return;

	if (nState == WA_INACTIVE)
	{
		if( !g_Opt.m_bShowPersistent )
			HideQPasteWindow();
	}
	else if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
	{
		if( !theApp.m_bShowingQuickPaste )
			ShowQPasteWindow();
	}
}

BOOL CQPasteWnd::HideQPasteWindow()
{
	if( !theApp.m_bShowingQuickPaste || m_cbSearch.GetShowingDropDown() )
		return FALSE;
	//Reset the flag
	theApp.m_bShowingQuickPaste = false;
	theApp.ReleaseFocus();

	if(m_Recset.IsOpen())
		m_Recset.Close();

	m_lstHeader.DestroyAndCreateAccelerator(FALSE);

	//Save the size
	CRect rect;
	GetWindowRectEx(rect);
	CGetSetOptions::SetQuickPasteSize(rect.Size());
	CGetSetOptions::SetQuickPastePoint(rect.TopLeft());

	// Hide the window when the focus is lost
	ShowWindow(SW_HIDE);

	//Reset the selection in the search combo
	m_cbSearch.SetCurSel(-1);

	return TRUE;
}

BOOL CQPasteWnd::ShowQPasteWindow(BOOL bFillList)
{
	//Set the flag so we can't open this up again
	theApp.m_bShowingQuickPaste = true;
	SetCaptionColorActive( !g_Opt.m_bShowPersistent );
	UpdateStatus();

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

	// always on top... for persistent showing (g_Opt.m_bShowPersistent)
	::SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE );

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
	CProcessPaste paste;
	paste.GetClipIDs().Add(wParam);
	paste.DoPaste();
	theApp.OnPasteCompleted();

	return TRUE;
}

LRESULT CQPasteWnd::OnListSelect_Index(WPARAM wParam, LPARAM lParam)
{
	if( (int) wParam >= m_lstHeader.GetItemCount() )
		return FALSE;

	CProcessPaste paste;
	paste.GetClipIDs().Add( m_lstHeader.GetItemData(wParam) );
	paste.DoPaste();
	theApp.OnPasteCompleted();

	return TRUE;
}

LRESULT CQPasteWnd::OnListSelect(WPARAM wParam, LPARAM lParam)
{
	int nCount = (int) wParam;
	long *pItems = (long*) lParam;

	if(nCount <= 0)
		return TRUE;

	CProcessPaste paste;
	m_lstHeader.GetSelectionItemData( paste.GetClipIDs() );

	paste.DoPaste();

	theApp.OnPasteCompleted();

	return TRUE;
}

LRESULT CQPasteWnd::OnListEnd(WPARAM wParam, LPARAM lParam)
{
	HideQPasteWindow();

	return 0;
}

LRESULT CQPasteWnd::OnRefreshView(WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	// remove all additional refresh view messages from the queue
	while( ::PeekMessage( &msg, m_hWnd, WM_REFRESH_VIEW, WM_REFRESH_VIEW, PM_REMOVE ) )
	{}
	if( theApp.m_bShowingQuickPaste )
		FillList();
	return TRUE;
}

void CQPasteWnd::RefreshNc( bool bRepaintImmediately )
{
	if( !theApp.m_bShowingQuickPaste )
		return;

	if( bRepaintImmediately )
		OnNcPaint();
	else
		InvalidateNc();
}

void CQPasteWnd::UpdateStatus( bool bRepaintImmediately )
{
CString title = m_Title;
CString prev;

	GetWindowText(prev);

	if( theApp.m_Status != "" )
	{
		title += " [ ";
		title += theApp.m_Status;
		title += " ] - ";
	}
	else
		title += " - ";

	if( ::IsWindow(theApp.m_hTargetWnd) )
		title += theApp.GetTargetName();
	else
		title += "NO TARGET!";

	if( title != prev )
	{
		SetWindowText( title );
		RefreshNc( bRepaintImmediately );
	}
}

BOOL CQPasteWnd::FillList(CString csSQLSearch/*=""*/)
{
//	if(m_Recset.IsOpen())
//		m_Recset.Close();

	// currently, we only have a History Group, so assign it directly.
	m_lstHeader.m_bStartTop = g_Opt.m_bHistoryStartTop;
	m_bAscending = !g_Opt.m_bHistoryStartTop;

	if( m_bAscending )
		m_Recset.m_strSort = "lDate ASC";
	else
		m_Recset.m_strSort = "lDate DESC";

	m_lstHeader.DeleteAllItems();

	CRect crRect;
	GetClientRect(crRect);
 
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
		}	
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
	}

	// set the caret based upon which end we're starting from
	if( m_lstHeader.m_bStartTop )
	{
		m_lstHeader.SetListPos( 0 );
	}
	else
	{
	int idx = m_lstHeader.GetItemCount() - 1;
		// if there are elements
		if( idx >= 0 )
			m_lstHeader.SetListPos( idx );
	}

//	m_lstHeader.Invalidate();
	RedrawWindow(0,0,RDW_INVALIDATE);

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

	if(theApp.IsClipboardViewerConnected())
		pMenu->DeleteMenu(ID_MENU_RECONNECTTOCLIPBOARDCHAIN, MF_BYCOMMAND);

	if(CGetSetOptions::GetShowTextForFirstTenHotKeys())
		pMenu->CheckMenuItem(ID_MENU_FIRSTTENHOTKEYS_SHOWHOTKEYTEXT, MF_CHECKED);

	if(CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys())
		pMenu->CheckMenuItem(ID_MENU_FIRSTTENHOTKEYS_USECTRLNUM, MF_CHECKED);

	if(g_Opt.m_bShowPersistent)
		pMenu->CheckMenuItem(ID_MENU_ALLWAYSONTOP, MF_CHECKED);

	if(CGetSetOptions::GetAutoHide())
		pMenu->CheckMenuItem(ID_MENU_AUTOHIDE, MF_CHECKED);

	if(g_Opt.m_bHistoryStartTop)
		pMenu->CheckMenuItem(ID_SORT_ASCENDING, MF_CHECKED);
	else
		pMenu->CheckMenuItem(ID_SORT_DESCENDING, MF_CHECKED);

	switch(CGetSetOptions::GetCaptionPos())
	{
	case 1:
		pMenu->CheckMenuItem(ID_VIEWCAPTIONBARON_RIGHT, MF_CHECKED);
		break;
	case 2:
		pMenu->CheckMenuItem(ID_VIEWCAPTIONBARON_BOTTOM, MF_CHECKED);
		break;
	case 3:
		pMenu->CheckMenuItem(ID_VIEWCAPTIONBARON_LEFT, MF_CHECKED);
		break;
	case 4:
		pMenu->CheckMenuItem(ID_VIEWCAPTIONBARON_TOP, MF_CHECKED);
		break;
	}
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
	::SendMessage(theApp.GetClipboardViewer(), WM_CV_RECONNECT, 0, 0);
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
		m_lstHeader.SetListPos(nRow);
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

void CQPasteWnd::OnViewcaptionbaronRight()
{
	SetCaptionOn(CAPTION_RIGHT);
	CGetSetOptions::SetCaptionPos(CAPTION_RIGHT);
}

void CQPasteWnd::OnViewcaptionbaronBottom()
{
	SetCaptionOn(CAPTION_BOTTOM);
	CGetSetOptions::SetCaptionPos(CAPTION_BOTTOM);
}

void CQPasteWnd::OnViewcaptionbaronLeft()
{
	SetCaptionOn(CAPTION_LEFT);
	CGetSetOptions::SetCaptionPos(CAPTION_LEFT);
}

void CQPasteWnd::OnViewcaptionbaronTop()
{
	SetCaptionOn(CAPTION_TOP);
	CGetSetOptions::SetCaptionPos(CAPTION_TOP);
}

void CQPasteWnd::OnMenuAutohide()
{
	bool bAutoHide = !CGetSetOptions::GetAutoHide();
	CGetSetOptions::SetAutoHide(bAutoHide);
	SetAutoHide(bAutoHide);
}

void CQPasteWnd::OnMenuViewfulldescription()
{
	m_lstHeader.ShowFullDescription();	
}

void CQPasteWnd::OnMenuAllwaysontop()
{
	theApp.ShowPersistent( !g_Opt.m_bShowPersistent );
}

void CQPasteWnd::OnSortAscending()
{
	g_Opt.SetHistoryStartTop(TRUE);
	FillList();
}

void CQPasteWnd::OnSortDescending()
{
	g_Opt.SetHistoryStartTop(FALSE);
	FillList();
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
	long lCount = 0;

	if( m_lstHeader.GetSelectedCount() == 0 )
		return;

	POSITION pos = m_lstHeader.GetFirstSelectedItemPosition();
	int nFirstSel = m_lstHeader.GetNextSelectedItem( pos );

	m_Recset.MoveLast();
	lCount = m_Recset.GetRecordCount();

	if( lCount == m_lstHeader.GetSelectedCount() )
		CClip::DeleteAll();
	else
	{
		m_lstHeader.GetSelectionItemData(IDs);
		CClip::Delete(IDs);
	}

	m_Recset.Requery();

	// set lCount to current number of records
	if( m_Recset.IsBOF() && m_Recset.IsEOF() )
		lCount = 0;
	else
	{
		m_Recset.MoveLast();
		lCount = m_Recset.GetRecordCount();
	}

	m_lstHeader.SetItemCountEx(lCount);
	if(lCount == 0)
		m_lstHeader.Invalidate();

	m_lstHeader.RemoveAllSelection();

	// adjust new cursor position to the first item we deleted.
	if( lCount > 0 )
	{
		// if there are no items after the one we deleted, then select the last one.
		if( nFirstSel >= lCount )
			nFirstSel = lCount - 1;

		m_lstHeader.SetListPos(nFirstSel);
	}

	m_lstHeader.RefreshVisibleRows();
}

CString CQPasteWnd::LoadDescription( int nItem )
{
	if( nItem < 0 || nItem >= m_lstHeader.GetItemCount() )
		return "";

CString cs;
	try
	{
		m_Recset.SetAbsolutePosition( nItem );
		cs = m_Recset.m_strText;
	}
	CATCHDAO

	return cs;
}

bool CQPasteWnd::SaveDescription( int nItem, CString text )
{
	if( nItem < 0 || nItem >= m_lstHeader.GetItemCount() )
		return false;

	try
	{
		m_Recset.SetAbsolutePosition( nItem );
		m_Recset.Edit();
		m_Recset.m_strText = text;
		m_Recset.Update();
	}
	CATCHDAO

	return true;
}

BOOL CQPasteWnd::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message) 
	{
	case WM_KEYDOWN:

		switch( pMsg->wParam )
		{
		case VK_SPACE:
			if(GetKeyState(VK_CONTROL) & 0x8000)
				theApp.ShowPersistent( !g_Opt.m_bShowPersistent );
			break;

		case VK_ESCAPE:
			if(!m_cbSearch.GetShowingDropDown())
			{
				HideQPasteWindow();
				return TRUE;
			}
			break;

		case VK_TAB:
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

		case 'A': // Ctrl-A = Select All
			if(GetKeyState(VK_CONTROL) & 0x8000)
			{
				int nCount = m_lstHeader.GetItemCount();
				for(int i = 0; i < nCount; i++)
				{
					m_lstHeader.SetSelection(i);
				}
				return TRUE;
			}
			break;

		} // end switch( pMsg->wParam )

		break; // end case WM_KEYDOWN 

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
NM_LISTVIEW* pLV = (NM_LISTVIEW*)pNMHDR;
CProcessPaste paste;
CClipIDs& clips = paste.GetClipIDs();
	m_lstHeader.GetSelectionItemData( clips );
	if( clips.GetSize() <= 0 )
	{
		ASSERT(0); // does this ever happen ?????
		clips.Add( m_lstHeader.GetItemData(pLV->iItem) );
	}
	paste.DoDrag();
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
				
				lstrcpyn(pItem->pszText, cs, pItem->cchTextMax);
				if(cs.GetLength() > pItem->cchTextMax)
					pItem->pszText[pItem->cchTextMax-1] = 0;
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
	if( !pInfo )
		return;

	if( pInfo->lItem < 0 )
	{
		CString cs("no item selected");

		lstrcpyn(pInfo->pszText, cs, pInfo->cchTextMax);
		if(cs.GetLength() > pInfo->cchTextMax)
			pInfo->pszText[pInfo->cchTextMax-1] = 0;

		return;
	}

	try
	{
		CString cs;
	
		m_Recset.SetAbsolutePosition(pInfo->lItem);

		cs = m_Recset.m_strText;
		cs += "\n\n";

#ifdef _DEBUG
		cs += StrF("%d (%d) ", pInfo->lItem, m_Recset.m_lID );
#endif

		CTime time(m_Recset.m_lDate);
		cs += time.Format("%m/%d/%Y %I:%M:%S %p");

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

		lstrcpyn(pInfo->pszText, cs, pInfo->cchTextMax);
		if(cs.GetLength() > pInfo->cchTextMax)
			pInfo->pszText[pInfo->cchTextMax-1] = 0;
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

void CQPasteWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	// toggle ShowPersistent when we double click the caption
	if( nHitTest == HTCAPTION )
		theApp.ShowPersistent( !g_Opt.m_bShowPersistent );

	CWndEx::OnNcLButtonDblClk(nHitTest, point);
}

#define WNDSNAP_ALLOWANCE 12

void CQPasteWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CWndEx::OnWindowPosChanging(lpwndpos);

	RECT rcScreen;

	// Get cordinates of the working area on the screen
	SystemParametersInfo (SPI_GETWORKAREA, 0, &rcScreen, 0);

	// Snap X axis to left
	if(abs(lpwndpos->x - rcScreen.left) <= WNDSNAP_ALLOWANCE)
	{
		lpwndpos->x = rcScreen.left;
	}

	// Snap X axis to right
	if (abs(lpwndpos->x + lpwndpos->cx - rcScreen.right) <= WNDSNAP_ALLOWANCE)
	{
		lpwndpos->x = rcScreen.right - lpwndpos->cx;
	}

	// Snap Y axis to top
	if (abs(lpwndpos->y - rcScreen.top) <= WNDSNAP_ALLOWANCE)
	{
		// Assign new cordinate
		lpwndpos->y = rcScreen.top;
	} 

	// Snap Y axis to bottom
	if (abs(lpwndpos->y + lpwndpos->cy - rcScreen.bottom) <= WNDSNAP_ALLOWANCE)
	{
		lpwndpos->y = rcScreen.bottom - lpwndpos->cy;
	}
}
