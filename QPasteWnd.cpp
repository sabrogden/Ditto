// QPasteWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QPasteWnd.h"
#include "ProcessPaste.h"
#include "CopyProperties.h"
#include "GroupName.h"
#include ".\qpastewnd.h"
#include "MoveToGroupDlg.h"
#include "HyperLink.h"
#include "FormatSQL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define QPASTE_TITLE			"Ditto"

#define ID_LIST_HEADER			0x201
#define ID_EDIT_SEARCH			0x202
#define ID_CANCEL				0x203
#define ID_GROUP_TEXT			0x204
#define ID_SHOW_GROUPS_BOTTOM	0x205
#define ID_SHOW_GROUPS_TOP		0x206
#define ID_BACK_BUTTON			0x207


#define QPASTE_WIDTH			200
#define	QPASTE_HEIGHT			200

/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd

CQPasteWnd::CQPasteWnd()
{	
	m_Title = QPASTE_TITLE;
	m_bHideWnd = true;
	m_strSQLSearch = "";
	m_bAllowRepaintImmediately = true;
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
	ON_COMMAND(ID_MENU_TOGGLECONNECTCV, OnMenuToggleConnectCV)
	ON_COMMAND(ID_MENU_PROPERTIES, OnMenuProperties)
	ON_WM_CLOSE()
	ON_NOTIFY(LVN_BEGINDRAG, ID_LIST_HEADER, OnBegindrag)
	ON_WM_SYSKEYDOWN()
	ON_NOTIFY(LVN_GETDISPINFO, ID_LIST_HEADER, GetDispInfo)
	ON_NOTIFY(LVN_ODFINDITEM, ID_LIST_HEADER, OnFindItem)
	ON_COMMAND(ID_MENU_FIRSTTENHOTKEYS_USECTRLNUM, OnMenuFirsttenhotkeysUsectrlnum)
	ON_COMMAND(ID_MENU_FIRSTTENHOTKEYS_SHOWHOTKEYTEXT, OnMenuFirsttenhotkeysShowhotkeytext)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_ALLWAYSSHOWDESCRIPTION, OnMenuQuickoptionsAllwaysshowdescription)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_DOUBLECLICKINGONCAPTION_TOGGLESALWAYSONTOP, OnMenuQuickoptionsDoubleclickingoncaptionTogglesalwaysontop)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_DOUBLECLICKINGONCAPTION_ROLLUPWINDOW, OnMenuQuickoptionsDoubleclickingoncaptionRollupwindow)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_DOUBLECLICKINGONCAPTION_TOGGLESALWAYSSHOWDESCRIPTION, OnMenuQuickoptionsDoubleclickingoncaptionTogglesshowdescription)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_PROMPTFORNEWGROUPNAMES, OnMenuQuickoptionsPromptfornewgroupnames)
	ON_BN_CLICKED(ID_SHOW_GROUPS_BOTTOM, OnShowGroupsBottom)
	ON_BN_CLICKED(ID_SHOW_GROUPS_TOP, OnShowGroupsTop)
	ON_COMMAND(ID_MENU_VIEWGROUPS, OnMenuViewgroups)
	ON_COMMAND(ID_MENU_QUICKPROPERTIES_SETTONEVERAUTODELETE, OnMenuQuickpropertiesSettoneverautodelete)
	ON_COMMAND(ID_MENU_QUICKPROPERTIES_AUTODELETE, OnMenuQuickpropertiesAutodelete)
	ON_COMMAND(ID_MENU_QUICKPROPERTIES_REMOVEHOTKEY, OnMenuQuickpropertiesRemovehotkey)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_EIGHT, OnMenuSenttoFriendEight)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_ELEVEN, OnMenuSenttoFriendEleven)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_FIFTEEN, OnMenuSenttoFriendFifteen)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_FIVE, OnMenuSenttoFriendFive)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_FORE, OnMenuSenttoFriendFore)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_FORETEEN, OnMenuSenttoFriendForeteen)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_NINE, OnMenuSenttoFriendNine)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_SEVEN, OnMenuSenttoFriendSeven)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_SIX, OnMenuSenttoFriendSix)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_TEN, OnMenuSenttoFriendTen)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_THIRTEEN, OnMenuSenttoFriendThirteen)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_THREE, OnMenuSenttoFriendThree)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_TWELVE, OnMenuSenttoFriendTwelve)
	ON_COMMAND(ID_MENU_SENTTO_FRIEND_TWO, OnMenuSenttoFriendTwo)
	ON_COMMAND(ID_MENU_SENTTO_FRIENDONE, OnMenuSenttoFriendone)
	ON_COMMAND(ID_MENU_SENTTO_PROMPTFORIP, OnMenuSenttoPromptforip)
	ON_COMMAND(ID_MENU_GROUPS_MOVETOGROUP, OnMenuGroupsMovetogroup)
	ON_COMMAND(ID_MENU_PASTEPLAINTEXTONLY, OnMenuPasteplaintextonly)
	ON_COMMAND(ID_MENU_HELP, OnMenuHelp)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_FONT, OnMenuQuickoptionsFont)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_SHOWTHUMBNAILS, OnMenuQuickoptionsShowthumbnails)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_DRAWRTFTEXT, OnMenuQuickoptionsDrawrtftext)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_PASTECLIPAFTERSELECTION, OnMenuQuickoptionsPasteclipafterselection)
	ON_CBN_EDITCHANGE(ID_EDIT_SEARCH, OnSearchEditChange)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_FINDASYOUTYPE, OnMenuQuickoptionsFindasyoutype)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_ENSUREENTIREWINDOWISVISIBLE, OnMenuQuickoptionsEnsureentirewindowisvisible)
	ON_COMMAND(ID_MENU_QUICKOPTIONS_SHOWCLIPSTHATAREINGROUPSINMAINLIST, OnMenuQuickoptionsShowclipsthatareingroupsinmainlist)
	ON_COMMAND(ID_MENU_PASTEHTMLASPLAINTEXT, OnMenuPastehtmlasplaintext)
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
	ON_COMMAND(ID_MENU_NEWGROUP, OnMenuNewGroup)
	ON_COMMAND(ID_MENU_NEWGROUPSELECTION, OnMenuNewGroupSelection)
	ON_MESSAGE(NM_GROUP_TREE_MESSAGE, OnGroupTreeMessage)
	ON_COMMAND(ID_BACK_BUTTON, OnBackButton)
	ON_MESSAGE(NM_GET_CLIP_DATA, OnGetClipData)
	ON_MESSAGE(CB_UPDOWN, OnUpDown)

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
	
	((CWnd *)&m_GroupTree)->CreateEx(NULL, "SysTreeView32", NULL, WS_BORDER|WS_POPUP|TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS, CRect(0, 0, 100, 100), this, 0);
	
	m_GroupTree.SetNotificationWndEx(m_hWnd);
	m_GroupTree.ShowWindow(SW_HIDE);
	
	m_ShowGroupsFolderBottom.Create(NULL, WS_CHILD | BS_OWNERDRAW | WS_TABSTOP, CRect(0, 0, 0, 0), this, ID_SHOW_GROUPS_BOTTOM);
	m_ShowGroupsFolderBottom.LoadBitmaps(IDB_CLOSED_FOLDER, IDB_CLOSED_FOLDER_PRESSED, IDB_CLOSED_FOLDER_FOCUSED);
	m_ShowGroupsFolderBottom.ShowWindow(SW_SHOW);
	
	m_ShowGroupsFolderTop.Create(NULL, WS_CHILD | BS_OWNERDRAW | WS_TABSTOP, CRect(0, 0, 0, 0), this, ID_SHOW_GROUPS_TOP);
	m_ShowGroupsFolderTop.LoadBitmaps(IDB_OPEN_FOLDER, IDB_OPEN_FOLDER_PRESSED, IDB_OPEN_FOLDER_FOCUSED);
	m_ShowGroupsFolderTop.ShowWindow(SW_SHOW);

	m_BackButton.Create(NULL, WS_CHILD | BS_OWNERDRAW | WS_TABSTOP, CRect(0, 0, 0, 0), this, ID_BACK_BUTTON);
	m_BackButton.LoadBitmaps(IDB_BACK, IDB_BACK_PRESSED, IDB_BACK_FOCUSED);
	m_BackButton.ShowWindow(SW_SHOW);
	
	m_btCancel.Create("&C", WS_CHILD|BS_PUSHBUTTON|WS_TABSTOP/*|BS_FLAT*/, CRect(0, 0, 0, 0), this, ID_CANCEL);
	
	m_stGroup.Create("", WS_CHILD|WS_VISIBLE, CRect(0, 0, 0, 0), this, ID_GROUP_TEXT);
	
	//Set the z-order
	m_lstHeader.SetWindowPos(this, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	m_cbSearch.SetWindowPos(&m_lstHeader, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	m_btCancel.SetWindowPos(&m_cbSearch, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	m_ShowGroupsFolderBottom.SetWindowPos(&m_cbSearch, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
	
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
	
	GroupFont.CreateFont(-11, 0, 0, 0, 400, 0, 1, 0, DEFAULT_CHARSET, 3, 2, 1, 34, "MS Sans Serif");
	
	m_cbSearch.SetFont(&m_SearchFont);
	m_btCancel.SetFont(&m_SearchFont);
	m_stGroup.SetFont(&GroupFont);

	UpdateFont();
		
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
	
	long lTopOfListBox = 0;
	
	if( theApp.m_GroupID > 0 )
	{
		m_stGroup.ShowWindow(SW_SHOW);
		m_ShowGroupsFolderTop.ShowWindow(SW_SHOW);
		m_BackButton.ShowWindow(SW_SHOW);

		m_BackButton.MoveWindow(0, 0, 18, 16);
		m_ShowGroupsFolderTop.MoveWindow(22, 0, 18, 16);
		m_stGroup.MoveWindow(44, 0, cx, 16);
		
		lTopOfListBox = 16;	
	}
	else
	{
		m_BackButton.ShowWindow(SW_HIDE);
		m_stGroup.ShowWindow(SW_HIDE);
		m_ShowGroupsFolderTop.ShowWindow(SW_HIDE);
	}
	
	// Resize the list control
	m_lstHeader.MoveWindow(0, lTopOfListBox, cx, cy - 22 - lTopOfListBox);
	
	int nWidth = cx;
	
	if( m_strSQLSearch.IsEmpty() == FALSE )
	{
		m_btCancel.ShowWindow(SW_SHOW);
		m_btCancel.MoveWindow(cx - 20, cy - 20, 20, 20);
		nWidth -= 19;
	}
	else
		m_btCancel.ShowWindow(SW_HIDE);
	
	m_cbSearch.MoveWindow(18, cy - 22, nWidth-18, 100);
	
	m_ShowGroupsFolderBottom.MoveWindow(0, cy - 19, 18, 16);
	
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
	
	if(m_bHideWnd == false || m_lstHeader.GetToolTipHWnd() == pWndOther->GetSafeHwnd())
		return;
	
	if (nState == WA_INACTIVE)
	{
		if(!g_Opt.m_bShowPersistent)
		{
			HideQPasteWindow();
		}
		//re register the global hot keys for the last ten
		if(theApp.m_bAppExiting == false)
			g_HotKeys.RegisterAll();

		m_lstHeader.HidePopup();
	}
	else if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
	{
		if(!theApp.m_bShowingQuickPaste)
		{
			ShowQPasteWindow();
		}

		//Unregister the global hot keys for the last ten copies
		g_HotKeys.UnregisterAll(false, true);
	}
}

BOOL CQPasteWnd::HideQPasteWindow()
{
	if( !theApp.m_bShowingQuickPaste || m_cbSearch.GetShowingDropDown() )
		return FALSE;

	//Reset the flag
	theApp.m_bShowingQuickPaste = false;
	theApp.ReleaseFocus();
	
	// Profile optimization -- is it safe to comment this out???
	//	if(m_Recset.IsOpen())
	//		m_Recset.Close();
	
	m_lstHeader.DestroyAndCreateAccelerator(FALSE);
	
	// save the caret position
	int nCaretPos = m_lstHeader.GetCaret();
	if( nCaretPos >= 0 )
		theApp.m_FocusID = m_lstHeader.GetItemData( nCaretPos );
	
	//Save the size
	CRect rect;
	GetWindowRectEx(&rect);
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
	SetCaptionColorActive(!g_Opt.m_bShowPersistent, theApp.GetConnectCV());
	
	// use invalidation to avoid unnecessary repainting
	m_bAllowRepaintImmediately = false;
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
		FillList();	// FillList calls MoveControls
	else
		MoveControls();
	
	// from now on, for interactive use, we can repaint immediately
	m_bAllowRepaintImmediately = true;
	
	// always on top... for persistent showing (g_Opt.m_bShowPersistent)
	// SHOWWINDOW was also integrated into this function rather than calling it separately
	::SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW );
	
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

BOOL CQPasteWnd::OpenID(long lID, bool bOnlyLoad_CF_TEXT, bool bPasteHTMLAs_CF_TEXT)
{
	if( theApp.EnterGroupID(lID) )
		return TRUE;

	if(GetKeyState(VK_SHIFT) & 0x8000)
	{
		bOnlyLoad_CF_TEXT = true;
		keybd_event(VK_SHIFT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		//Sleep(300);
	}

	// else, it is a clip, so paste it
	CProcessPaste paste;
	
	paste.m_bSendPaste = g_Opt.m_bSendPasteMessageAfterSelection == TRUE ? true : false;
	paste.m_bOnlyPaste_CF_TEXT = bOnlyLoad_CF_TEXT;
	paste.m_bPasteHTMLFormatAs_CF_TEXT = bPasteHTMLAs_CF_TEXT;

	paste.GetClipIDs().Add( lID );
	paste.DoPaste();
	theApp.OnPasteCompleted();

	if(g_Opt.m_bSendPasteMessageAfterSelection == FALSE)
		theApp.ActivateTarget();

	return TRUE;
}

BOOL CQPasteWnd::OpenSelection(bool bOnlyLoad_CF_TEXT, bool bPasteHTMLAs_CF_TEXT)
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData( IDs );
    
	int count = IDs.GetSize();
	
	if(count <= 0)
		return FALSE;
	
	if(count == 1)
		return OpenID(IDs[0], bOnlyLoad_CF_TEXT, bPasteHTMLAs_CF_TEXT);
	
	CProcessPaste paste;

	paste.m_bSendPaste = g_Opt.m_bSendPasteMessageAfterSelection == TRUE ? true : false;
	paste.m_bOnlyPaste_CF_TEXT = bOnlyLoad_CF_TEXT;
	paste.m_bPasteHTMLFormatAs_CF_TEXT = bPasteHTMLAs_CF_TEXT;
	
	paste.GetClipIDs().Copy(IDs);
	paste.DoPaste();
	theApp.OnPasteCompleted();

	if(g_Opt.m_bSendPasteMessageAfterSelection == FALSE)
		theApp.ActivateTarget();

	return TRUE;
}

BOOL CQPasteWnd::OpenIndex( long nItem )
{
	return OpenID( m_lstHeader.GetItemData(nItem) );
}

BOOL CQPasteWnd::NewGroup( bool bGroupSelection )
{	
	//Get the selected ids
	CClipIDs IDs;
	m_lstHeader.GetSelectionItemData( IDs );
	
	CGroupName Name;
	CString csName("");
	
	if(g_Opt.m_bPrompForNewGroupName)
	{
		m_bHideWnd = false;
		
		int nRet = Name.DoModal();
		
		m_bHideWnd = true;
		
		if(nRet == IDOK)
			csName = Name.m_csName;
		else
			return false;
	}
	
	long lID = NewGroupID(theApp.GetValidGroupID(), csName);
	
	if(lID <= 0)
		return FALSE;
	
	if(!bGroupSelection )
	{
		theApp.m_FocusID = lID; // focus on the new group
		FillList();
		return TRUE;
	}
	
	IDs.MoveTo( lID );
	theApp.EnterGroupID( lID );
	return TRUE;
}

BOOL CQPasteWnd::SetListID( long lID )
{
	int index;
	if( !m_Recset.FindFirst( StrF("lID = %d",lID) ) )
		return FALSE;
	index = m_Recset.GetAbsolutePosition();
	m_lstHeader.SetListPos( index );
	return TRUE;
}


LRESULT CQPasteWnd::OnListSelect_DB_ID(WPARAM wParam, LPARAM lParam)
{
	OpenID( wParam );
	return TRUE;
}

LRESULT CQPasteWnd::OnListSelect_Index(WPARAM wParam, LPARAM lParam)
{
	if( (int) wParam >= m_lstHeader.GetItemCount() )
		return FALSE;
	
	OpenIndex( wParam );
	
	return TRUE;
}

LRESULT CQPasteWnd::OnListSelect(WPARAM wParam, LPARAM lParam)
{
	int nCount = (int) wParam;
	long *pItems = (long*) lParam;
	
	OpenSelection(false, false);
	
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
	{
	}
	if( theApp.m_bShowingQuickPaste )
	{
		HWND hFocus = GetFocus()->GetSafeHwnd();
		if(hFocus == NULL || hFocus == m_lstHeader.GetSafeHwnd())
			FillList();
	}
	
	return TRUE;
}

void CQPasteWnd::RefreshNc( bool bRepaintImmediately )
{
	if( !theApp.m_bShowingQuickPaste )
		return;
	
	if( bRepaintImmediately && m_bAllowRepaintImmediately )
		OnNcPaint();
	else
		InvalidateNc();
}

void CQPasteWnd::UpdateStatus( bool bRepaintImmediately )
{
	CString title = m_Title;
	CString prev;
	
	GetWindowText(prev);
	
	if(m_Recset.IsOpen())
	{
		CString cs;
		cs.Format(" - %d/%d", m_lstHeader.GetSelectedCount(), m_Recset.GetRecordCount());
		title += cs;
	}
	
	if( theApp.m_Status != "" )
	{
		title += " [ ";
		title += theApp.m_Status;
		title += " ] - ";
	}
	else
		title += " - ";
	
	if( theApp.m_IC_IDs.GetSize() > 0 )
	{
		if( theApp.m_IC_bCopy )
			title += "Copying";
		else
			title += "Moving";
		
		title += " - ";
	}
	
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
	CString strFilter;
	
	// History Group
	if( theApp.m_GroupID == 0 )
	{
		m_lstHeader.m_bStartTop = g_Opt.m_bHistoryStartTop;
		if( g_Opt.m_bHistoryStartTop )
			m_Recset.m_strSort = "bIsGroup DESC, lDate DESC";
		else
			m_Recset.m_strSort = "bIsGroup ASC, lDate ASC";
		
		if(g_Opt.m_bShowAllClipsInMainList)
		{
			strFilter = "((bIsGroup = TRUE AND lParentID = 0) OR bIsGroup = FALSE)";
		}
		else
		{
			strFilter = "((bIsGroup = TRUE AND lParentID = 0) OR (bIsGroup = FALSE AND lParentID = 0))";
		}
	}
	else // it's some other group
	{
		m_lstHeader.m_bStartTop = true;
		
		if( g_Opt.m_bHistoryStartTop )
			m_Recset.m_strSort = "bIsGroup DESC, lDate DESC";
		else
			m_Recset.m_strSort = "bIsGroup ASC, lDate ASC";
		
		
		if( theApp.m_GroupID > 0 )
			strFilter.Format( "lParentID = %d", theApp.m_GroupID );
		else // All top-level groups
			strFilter = "bIsGroup = TRUE AND lParentID = 0";
		
		m_stGroup.SetWindowText(theApp.m_GroupText);
	}
	
	// maintain the previous position if theApp.m_FocusID == -1
	int nCaretPos = m_lstHeader.GetCaret();
	if( theApp.m_FocusID == -1 && nCaretPos >= 0 )
		theApp.m_FocusID = m_lstHeader.GetItemData( nCaretPos );
	
	m_lstHeader.DeleteAllItems();
	
	CRect crRect;
	GetClientRect(crRect);
	
	CString csSQL;
	if(csSQLSearch == "")
	{
		m_strSQLSearch = "";
	}
	else
	{
		CFormatSQL SQLFormat;
		SQLFormat.SetVariable("mText");
		SQLFormat.Parse(csSQLSearch);

		strFilter = m_strSQLSearch = SQLFormat.GetSQLString();
	}
	
	try
	{
		m_Recset.m_strFilter = strFilter;
		if(m_Recset.IsOpen())
			m_Recset.Requery();
		else
			m_Recset.Open("");
		
		if(!m_Recset.IsEOF())
		{
			m_Recset.MoveLast();
			m_lstHeader.SetItemCountEx(m_Recset.GetRecordCount());
		}
		else
			m_lstHeader.SetItemCountEx(0);
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		ASSERT(0);
		e->Delete();
	}
	
	// if the caret position can't be set to the focus ID requested
	if( theApp.m_FocusID <= 0 || !SetListID( theApp.m_FocusID ) )
	{
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
	}
	
	theApp.m_FocusID = -1; // maintain previous position from now on.
	
	theApp.SetStatus(NULL, true);
	
	MoveControls();
	
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
		
		if(pNMHDR == NULL)
		{
			int nItem = m_lstHeader.GetCaret();
			CRect rc;
			m_lstHeader.GetItemRect(nItem, rc, LVIR_BOUNDS);
			ClientToScreen(rc);
			pp.x = rc.left;
			pp.y = rc.bottom;
		}
		
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

	theApp.UpdateMenuConnectCV(pMenu, ID_MENU_TOGGLECONNECTCV);

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
	
	if(CGetSetOptions::GetAllwaysShowDescription())
	{
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_ALLWAYSSHOWDESCRIPTION, MF_CHECKED);
	}
	
	switch(CGetSetOptions::GetDoubleClickingOnCaptionDoes())
	{
	case TOGGLES_ALLWAYS_ON_TOP:
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_DOUBLECLICKINGONCAPTION_TOGGLESALWAYSONTOP, MF_CHECKED);
		break;
	case TOGGLES_ALLWAYS_SHOW_DESCRIPTION:
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_DOUBLECLICKINGONCAPTION_TOGGLESALWAYSSHOWDESCRIPTION, MF_CHECKED);
		break;
	case ROLLES_UP_WINDOW:
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_DOUBLECLICKINGONCAPTION_ROLLUPWINDOW, MF_CHECKED);
		break;
	}
	
	if(CGetSetOptions::m_bPrompForNewGroupName)
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_PROMPTFORNEWGROUPNAMES, MF_CHECKED);

	if(g_Opt.m_bDrawThumbnail)
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_SHOWTHUMBNAILS, MF_CHECKED);

	if(g_Opt.m_bDrawRTF)
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_DRAWRTFTEXT, MF_CHECKED);

	if(g_Opt.m_bSendPasteMessageAfterSelection)
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_PASTECLIPAFTERSELECTION, MF_CHECKED);

	if(g_Opt.m_bFindAsYouType)
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_FINDASYOUTYPE, MF_CHECKED);

	if(g_Opt.m_bEnsureEntireWindowCanBeSeen)
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_ENSUREENTIREWINDOWISVISIBLE, MF_CHECKED);

	if(g_Opt.m_bShowAllClipsInMainList)
		pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_SHOWCLIPSTHATAREINGROUPSINMAINLIST, MF_CHECKED);

	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIENDONE, 0);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_TWO, 1);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_THREE, 2);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FORE, 3);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FIVE, 4);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_SIX, 5);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_SEVEN,6);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_EIGHT,7);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_NINE,8);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_TEN,9);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_ELEVEN,10);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_TWELVE,11);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_THIRTEEN,12);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FORETEEN,13);
	SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FIFTEEN,14);

	pMenu->DeleteMenu(ID_MENU_SENTTO_PROMPTFORIP, MF_BYCOMMAND);
}

void CQPasteWnd::SetSendToMenu(CMenu *pMenu, int nMenuID, int nArrayPos)
{
	if(g_Opt.m_SendClients[nArrayPos].csIP.GetLength() > 0)
	{
		CString cs;
		cs.Format("(%s) - %s", g_Opt.m_SendClients[nArrayPos].csIP, g_Opt.m_SendClients[nArrayPos].csDescription);
		pMenu->ModifyMenu(nMenuID, MF_BYCOMMAND, nMenuID, cs);
	}
	else
	{
		pMenu->DeleteMenu(nMenuID, MF_BYCOMMAND);
	}
}

LRESULT CQPasteWnd::OnSearch(WPARAM wParam, LPARAM lParam)
{
	m_lstHeader.HidePopup();

	CString csText;
	m_cbSearch.GetWindowText(csText);
	
	if(csText == "")
		return FALSE;
	
	FillList(csText);
	
	m_lstHeader.SetFocus();
	
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
	
	UpdateFont();

	ShowQPasteWindow(TRUE);
	
	m_bHideWnd = true;
	m_lstHeader.SetFocus();
}

void CQPasteWnd::OnMenuExitprogram() 
{
	::SendMessage(theApp.m_MainhWnd, WM_CLOSE, 0, 0);
}

void CQPasteWnd::OnMenuToggleConnectCV() 
{
	theApp.ToggleConnectCV();
}

#include "client.h"

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
		
		if(props.m_lGroupChangedTo >= 0)
		{
			OpenID(props.m_lGroupChangedTo);
		}
	}
	
	SetFocus();
	m_bHideWnd = true;
	
	if(nDo == IDOK || nDo == IDCANCEL)
	{
		m_lstHeader.SetFocus();
		m_lstHeader.SetListPos(lID);
	}
}

void CQPasteWnd::UpdateFont()
{
	LOGFONT lf;
	if( CGetSetOptions::GetFont(lf) )
		m_lstHeader.SetLogFont(lf);
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

void CQPasteWnd::OnMenuNewGroup()
{
	NewGroup( false );
}

void CQPasteWnd::OnMenuNewGroupSelection()
{
	NewGroup( true );
}

void CQPasteWnd::OnMenuQuickoptionsAllwaysshowdescription() 
{
	CGetSetOptions::SetAllwaysShowDescription(!g_Opt.m_bAllwaysShowDescription);
	
}

void CQPasteWnd::OnMenuQuickoptionsDoubleclickingoncaptionTogglesalwaysontop() 
{
	CGetSetOptions::SetDoubleClickingOnCaptionDoes(TOGGLES_ALLWAYS_ON_TOP);
	
}

void CQPasteWnd::OnMenuQuickoptionsDoubleclickingoncaptionRollupwindow() 
{
	CGetSetOptions::SetDoubleClickingOnCaptionDoes(ROLLES_UP_WINDOW);
	
}

void CQPasteWnd::OnMenuQuickoptionsDoubleclickingoncaptionTogglesshowdescription() 
{
	CGetSetOptions::SetDoubleClickingOnCaptionDoes(TOGGLES_ALLWAYS_SHOW_DESCRIPTION);	
}

void CQPasteWnd::OnMenuQuickoptionsPromptfornewgroupnames() 
{
	g_Opt.SetPrompForNewGroupName( !g_Opt.m_bPrompForNewGroupName );	
}

void CQPasteWnd::OnMenuViewgroups() 
{
	OnShowGroupsTop();
}

void CQPasteWnd::OnMenuQuickpropertiesSettoneverautodelete() 
{
	CWaitCursor wait;
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData( IDs );
    
	int count = IDs.GetSize();

	for(int i = 0; i < count; i++)
	{
		try
		{
			CMainTable recset;
			recset.Open("SELECT * FROM Main WHERE lID = %d", IDs[i]);
			recset.Edit();
			recset.m_lDontAutoDelete = (long)CTime::GetCurrentTime().GetTime();
			recset.Update();
		}
		CATCHDAO
	}

	m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuQuickpropertiesAutodelete() 
{
	CWaitCursor wait;
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData( IDs );
    
	int count = IDs.GetSize();

	for(int i = 0; i < count; i++)
	{
		try
		{
			CMainTable recset;
			recset.Open("SELECT * FROM Main WHERE lID = %d", IDs[i]);
			recset.Edit();
			recset.m_lDontAutoDelete = 0;
			recset.Update();
		}
		CATCHDAO
	}
	m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuQuickpropertiesRemovehotkey() 
{
	CWaitCursor wait;
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData( IDs );
    
	int count = IDs.GetSize();

	for(int i = 0; i < count; i++)
	{
		try
		{
			CMainTable recset;
			recset.Open("SELECT * FROM Main WHERE lID = %d", IDs[i]);
			recset.Edit();
			recset.m_lShortCut = 0;
			recset.Update();
		}
		CATCHDAO
	}	
	m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnUpdateMenuGroupsMovetothegroupBlank(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}


void CQPasteWnd::OnMenuSenttoFriendFifteen() 
{
	SendToFriendbyPos(14);
}

void CQPasteWnd::OnMenuSenttoFriendForeteen() 
{
	SendToFriendbyPos(13);
}

void CQPasteWnd::OnMenuSenttoFriendThirteen() 
{
	SendToFriendbyPos(12);
}

void CQPasteWnd::OnMenuSenttoFriendTwelve() 
{
	SendToFriendbyPos(11);
}

void CQPasteWnd::OnMenuSenttoFriendEleven() 
{
	SendToFriendbyPos(10);
}

void CQPasteWnd::OnMenuSenttoFriendTen() 
{
	SendToFriendbyPos(9);
}

void CQPasteWnd::OnMenuSenttoFriendNine() 
{
	SendToFriendbyPos(8);
}

void CQPasteWnd::OnMenuSenttoFriendEight() 
{
	SendToFriendbyPos(7);
}

void CQPasteWnd::OnMenuSenttoFriendSeven() 
{
	SendToFriendbyPos(6);
}

void CQPasteWnd::OnMenuSenttoFriendSix() 
{
	SendToFriendbyPos(5);
}

void CQPasteWnd::OnMenuSenttoFriendFive() 
{
	SendToFriendbyPos(4);
}

void CQPasteWnd::OnMenuSenttoFriendFore() 
{
	SendToFriendbyPos(3);
}

void CQPasteWnd::OnMenuSenttoFriendThree() 
{
	SendToFriendbyPos(2);	
}

void CQPasteWnd::OnMenuSenttoFriendTwo() 
{
	SendToFriendbyPos(1);
}

void CQPasteWnd::OnMenuSenttoFriendone() 
{
	SendToFriendbyPos(0);
}

void CQPasteWnd::OnMenuSenttoPromptforip() 
{
	// TODO: Add your command handler code here
	
}

void CQPasteWnd::OnMenuGroupsMovetogroup() 
{
	m_bHideWnd = false;

	CMoveToGroupDlg dlg;
	
	if(dlg.DoModal() == IDOK)
	{
		int nGroup = dlg.GetSelectedGroup();
		if(nGroup >= 0)
		{
			CClipIDs IDs;
			m_lstHeader.GetSelectionItemData( IDs );

			IDs.MoveTo(nGroup);
		}		
	}

	m_bHideWnd = true;

	FillList();
}

void CQPasteWnd::OnMenuPasteplaintextonly() 
{
	OpenSelection(true, false);	
}

void CQPasteWnd::OnMenuPastehtmlasplaintext() 
{
	OpenSelection(false, true);
}

void CQPasteWnd::OnMenuHelp() 
{
	CHyperLink::GotoURL("Help\\DittoGettingStarted.htm", SW_SHOW);
}

void CQPasteWnd::OnMenuQuickoptionsFont() 
{
	m_bHideWnd = false;

	CFont *pFont = m_lstHeader.GetFont();
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	CFontDialog dlg(&lf);
	if(dlg.DoModal() == IDOK)
	{	
		CGetSetOptions::SetFont(*dlg.m_cf.lpLogFont);
		m_lstHeader.SetLogFont(*dlg.m_cf.lpLogFont);
	}
	
	m_bHideWnd = true;
}

void CQPasteWnd::OnMenuQuickoptionsShowthumbnails() 
{
	CGetSetOptions::SetDrawThumbnail(!g_Opt.m_bDrawThumbnail);
	m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuQuickoptionsDrawrtftext() 
{
	CGetSetOptions::SetDrawRTF(!g_Opt.m_bDrawRTF);
	m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuQuickoptionsPasteclipafterselection() 
{
	CGetSetOptions::SetSendPasteAfterSelection(!g_Opt.m_bSendPasteMessageAfterSelection);	
}

void CQPasteWnd::OnMenuQuickoptionsFindasyoutype() 
{
	CGetSetOptions::SetFindAsYouType(!g_Opt.m_bFindAsYouType);
}

void CQPasteWnd::OnMenuQuickoptionsEnsureentirewindowisvisible() 
{
	CGetSetOptions::SetEnsureEntireWindowCanBeSeen(!g_Opt.m_bEnsureEntireWindowCanBeSeen);
}

void CQPasteWnd::OnMenuQuickoptionsShowclipsthatareingroupsinmainlist() 
{
	CGetSetOptions::SetShowAllClipsInMainList(!g_Opt.m_bShowAllClipsInMainList);

	CString csText;
	m_cbSearch.GetWindowText(csText);
	FillList(csText);
}

///////////////////////////////////////////////////////////////////////
//END END Menu Stuff
///////////////////////////////////////////////////////////////////////


BOOL CQPasteWnd::SendToFriendbyPos(int nPos)
{
	CWaitCursor wait;

	m_bHideWnd = false;

	CClipIDs IDs;
	long lCount = m_lstHeader.GetSelectedCount();
	if(lCount <= 0)
		return FALSE;

	m_lstHeader.GetSelectionIndexes(IDs);

	CSendToFriendInfo Info;
	Info.m_lPos = nPos;

	BOOL bRet = FALSE;

	try
	{
		CPopup Popup(0, 0, m_hWnd);
		Popup.Show(StrF("Sending clip to %s", g_Opt.m_SendClients[nPos].csIP));

		Info.m_pPopup = &Popup;
		
		Info.m_pClipList =  new CClipList;
		for(int i = 0; i < lCount; i++)
		{
			m_Recset.SetAbsolutePosition(IDs[i]);
			CClip *pClip = new CClip;
			CClip::LoadFormats(m_Recset.m_lID, pClip->m_Formats);
			pClip->m_Desc = m_Recset.m_strText;
			pClip->m_ID = m_Recset.m_lID;
			Info.m_pClipList->AddTail(pClip);
		}
		
		if(SendToFriend(Info) == FALSE)
		{
			MessageBox(StrF("Error Sending data to %s\n\n%s", g_Opt.m_SendClients[nPos].csIP, Info.m_csErrorText), "Ditto");
		}
		else
		{
			bRet = TRUE;
		}
	}
	catch(CDaoException* e)
	{
		e->ReportError();
		ASSERT(0);
		e->Delete();
	}	

	m_bHideWnd = true;

	return bRet;
}

LRESULT CQPasteWnd::OnDelete(WPARAM wParam, LPARAM lParam)
{
	DeleteSelectedRows();
	
	return TRUE;
}

void CQPasteWnd::DeleteSelectedRows()
{
	CClipIDs IDs;
	long lCount = 0;
	
	if( m_lstHeader.GetSelectedCount() == 0 )
		return;
	
	POSITION pos = m_lstHeader.GetFirstSelectedItemPosition();
	int nFirstSel = m_lstHeader.GetNextSelectedItem( pos );
	
	m_lstHeader.GetSelectionItemData( IDs );
	IDs.DeleteIDs();
	
	try
	{
		m_Recset.Requery();
		
		// set lCount to current number of records
		if( m_Recset.IsBOF() && m_Recset.IsEOF() )
			lCount = 0;
		else
		{
			m_Recset.MoveLast();
			lCount = m_Recset.GetRecordCount();
		}
	}
	CATCHDAO
		
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
		case VK_APPS:
			{
				long lRet;
				OnRclickQuickPaste(NULL, &lRet);
				return 0;
			}
		case VK_F7:
			if(pMsg->hwnd == m_lstHeader.m_hWnd)
			{
				if(GetKeyState(VK_CONTROL) & 0x8000)
					NewGroup( false );
				else
					NewGroup( true );
				return TRUE;
			}
			break;
			
		case VK_ESCAPE:
			{	
				if(m_strSQLSearch.IsEmpty() == FALSE)
				{
					OnCancelFilter();
					return TRUE;
				}
				else
				{	
					if(!m_cbSearch.GetShowingDropDown() &&
						m_GroupTree.IsWindowVisible() == FALSE)
					{
						HideQPasteWindow();
						return TRUE;
					}
				}
				break;
			}
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
		case 'G':
			{
				if(GetKeyState(VK_CONTROL) & 0x8000)
				{
					OnShowGroupsTop();
				}
			}
			
		} // end switch( pMsg->wParam )
		
		break; // end case WM_KEYDOWN 
		
		case WM_SYSKEYDOWN: // ALT key is held down
			
			switch( pMsg->wParam )
			{
			case 'C': // switch to the filter combobox
				BYTE key[256];
				GetKeyboardState((LPBYTE)(&key));
				if(key[VK_MENU]&128)
				{
					OnCancelFilter();
				}
				return TRUE;
				
			case VK_HOME:
				theApp.EnterGroupID( 0 );  // History
				return TRUE;
				
			case VK_END:
				theApp.EnterGroupID( -1 ); // All Groups
				return TRUE;
				
			} // end switch( pMsg->wParam )
			
			break; // end case WM_SYSKEYDOWN
	}
	
	return CWndEx::PreTranslateMessage(pMsg);
}

void CQPasteWnd::OnCancelFilter()
{
	FillList();
	
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
		ASSERT(0); // does this ever happen ??
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
					cs += "*";
				
				if(m_Recset.m_lShortCut > 0)
					cs += "s";
                
				if(m_Recset.m_bIsGroup)
					cs += "G";
				
				// attached to a group
				if(m_Recset.m_lParentID > 0 )
					cs += "!";
				
				// pipe is the "end of symbols" marker
				cs += "|" + m_Recset.GetDisplayText( g_Opt.m_nLinesPerRow );
				
				lstrcpyn(pItem->pszText, cs, pItem->cchTextMax);
				pItem->pszText[pItem->cchTextMax-1] = '\0';
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
		cs += StrF("(Index = %d) (DB ID = %d) (Size = %d)\n", pInfo->lItem, m_Recset.m_lID, m_Recset.m_strText.GetLength() );
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
		pInfo->pszText[pInfo->cchTextMax-1] = '\0';
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
		{
			m_cbSearch.SetWindowText(fndItem.psz);
			m_cbSearch.SetFocus();
			m_cbSearch.SetEditSel(strlen(fndItem.psz), strlen(fndItem.psz));
			*pResult = -1;
			return;
		}
		
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
	{
		switch(g_Opt.m_bDoubleClickingOnCaptionDoes)
		{
		case TOGGLES_ALLWAYS_ON_TOP:
			theApp.ShowPersistent( !g_Opt.m_bShowPersistent );
			break;
		case TOGGLES_ALLWAYS_SHOW_DESCRIPTION:
			CGetSetOptions::SetAllwaysShowDescription(!g_Opt.m_bAllwaysShowDescription);
			break;
		case ROLLES_UP_WINDOW:
			MinMaxWindow();
			break;
		}
	}
	
	CWndEx::OnNcLButtonDblClk(nHitTest, point);
}

#define WNDSNAP_ALLOWANCE 12

void CQPasteWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CWndEx::OnWindowPosChanging(lpwndpos);

//	m_lstHeader.HidePopup();
	
	CRect rcScreen;
	
	CRect cr(lpwndpos->x, lpwndpos->y, lpwndpos->x + lpwndpos->cx, lpwndpos->y + lpwndpos->cy);
	int nMonitor = GetMonitorFromRect(&cr);
	GetMonitorRect(nMonitor, &rcScreen);
	
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

void CQPasteWnd::OnShowGroupsTop()
{
	OnShowGroupsBottom();
	return;
	m_GroupTree.m_bHide = false;
	m_bHideWnd = false;
	
	CRect crList;
	m_lstHeader.GetWindowRect(crList);
	
	CRect cr(crList.left, crList.top, crList.left + crList.Width(), crList.top + 200);
	
	m_GroupTree.MoveWindow(cr);
	m_GroupTree.m_lSelectedFolderID = theApp.m_GroupID;
	m_GroupTree.FillTree();
	m_GroupTree.ShowWindow(SW_SHOW);
	
	m_GroupTree.m_bHide = true;
	m_bHideWnd = true;
}

void CQPasteWnd::OnShowGroupsBottom()
{
	m_GroupTree.m_bHide = false;
	m_bHideWnd = false;
	
	CRect crWindow, crList;
	m_lstHeader.GetWindowRect(crList);
	GetWindowRect(crWindow);
	
	CRect cr(crWindow.left, crWindow.bottom, crWindow.left + crWindow.Width(), crWindow.bottom + 200);
	
	m_GroupTree.MoveWindow(cr);
	m_GroupTree.m_lSelectedFolderID = theApp.m_GroupID;
	m_GroupTree.FillTree();
	m_GroupTree.ShowWindow(SW_SHOW);
	
	m_GroupTree.m_bHide = true;
	m_bHideWnd = true;
}

LRESULT CQPasteWnd::OnGroupTreeMessage(WPARAM wParam, LPARAM lParam)
{
	m_bHideWnd = false;
	
	long lID = (long)wParam;
	
	m_GroupTree.ShowWindow(SW_HIDE);
	
	m_cbSearch.SetCurSel(-1);
	MoveControls();
	
	if(lID >= 0)
	{	
		//Set the app flag so it does a send message to refresh the list
		//We need to do this because we set the list pos to 0 and with Post
		//the list is not filled up yet
		bool bItWas = theApp.m_bAsynchronousRefreshView;
		theApp.m_bAsynchronousRefreshView = false;
		
		OpenID(lID);
		
		theApp.m_bAsynchronousRefreshView = bItWas;
		
		m_lstHeader.SetListPos(0);
	}
	
	CWnd *p = GetFocus();
	if(p == NULL)
	{
		HideQPasteWindow();
	}
	
	m_bHideWnd = true;
	
	return TRUE;
}

void CQPasteWnd::OnBackButton()
{
	theApp.EnterGroupID(theApp.m_GroupParentID);
}

LRESULT CQPasteWnd::OnGetClipData(WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = FALSE;
	CClipFormat *pClip = (CClipFormat *)lParam;
	int nItem = (int)wParam;
	try
	{
		ASSERT( pClip->m_hgData == NULL);

		m_Recset.SetAbsolutePosition(nItem);

		CDataTable recset;
		recset.Open("SELECT * FROM Data WHERE lDataID = %d AND strClipBoardFormat = \"%s\"", m_Recset.m_lDataID, GetFormatName(pClip->m_cfType));
		if(recset.IsEOF() == false)
		{			
			pClip->m_hgData = NewGlobal(recset.m_ooData.m_dwDataLength);
	
			::CopyToGlobalHH(pClip->m_hgData, recset.m_ooData.m_hData, recset.m_ooData.m_dwDataLength);

			bRet = TRUE;
		}
	}
	CATCHDAO

	return bRet;
}

void CQPasteWnd::OnSearchEditChange()
{
	if(g_Opt.m_bFindAsYouType == FALSE)
		return;

	CString csText;
	m_cbSearch.GetWindowText(csText);
	
	if(csText == "")
		return;

	m_lstHeader.HidePopup();
	
	FillList(csText);
	
	return;
	
	POSITION pos = m_lstHeader.GetFirstSelectedItemPosition();
	int nFirstSel = m_lstHeader.GetNextSelectedItem(pos);
	int nCount = m_lstHeader.GetItemCount();
	CString cs;
	m_cbSearch.GetWindowText(cs);

	m_Recset.SetAbsolutePosition(nFirstSel);

	for(int i = nFirstSel; i < nCount; i++)
	{
		if(m_Recset.m_strText.Find(cs) >= 0)
		{
			m_lstHeader.SetListPos(i);
			return;
		}

		m_Recset.MoveNext();
	}

	m_Recset.SetAbsolutePosition(0);

	for(i = 0; i < nFirstSel; i++)
	{
		if(m_Recset.m_strText.Find(cs) >= 0)
		{
			m_lstHeader.SetListPos(i);
			return;
		}

		m_Recset.MoveNext();
	}
}

LRESULT CQPasteWnd::OnUpDown(WPARAM wParam, LPARAM lParam)
{
	if(wParam == VK_F3)
	{
		m_lstHeader.ShowFullDescription();
	}
	else
	{
		m_lstHeader.HidePopup();

		m_lstHeader.SendMessage(WM_KEYDOWN, wParam, lParam);
	}

	return TRUE;
}
