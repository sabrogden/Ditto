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
#include "MainTableFunctions.h"
#include "Path.h"

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
#define QPASTE_HEIGHT			200

#define TIMER_FILL_CACHE		1
#define TIMER_DO_SEARCH			2
#define TIMER_PASTE_FROM_MODIFER	3

#define THREAD_DO_QUERY				0
#define THREAD_EXIT_THREAD			1
#define THREAD_FILL_ACCELERATORS	2
#define THREAD_DESTROY_ACCELERATORS	3
#define THREAD_LOAD_ITEMS			4
#define THREAD_LOAD_EXTRA_DATA		5


/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd

CQPasteWnd::CQPasteWnd()
{
    m_Title = QPASTE_TITLE;
    m_bHideWnd = true;
    m_strSQLSearch = "";
    m_bAllowRepaintImmediately = true;
    m_bHandleSearchTextChange = true;
    m_lItemsPerPage = 0;
    m_bModifersMoveActive = false;
}

CQPasteWnd::~CQPasteWnd(){}

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
ON_EN_CHANGE(ID_EDIT_SEARCH, OnSearchEditChange)
ON_COMMAND(ID_MENU_QUICKOPTIONS_FINDASYOUTYPE, OnMenuQuickoptionsFindasyoutype)
ON_COMMAND(ID_MENU_QUICKOPTIONS_ENSUREENTIREWINDOWISVISIBLE, OnMenuQuickoptionsEnsureentirewindowisvisible)
ON_COMMAND(ID_MENU_QUICKOPTIONS_SHOWCLIPSTHATAREINGROUPSINMAINLIST, OnMenuQuickoptionsShowclipsthatareingroupsinmainlist)
ON_UPDATE_COMMAND_UI(ID_MENU_NEWGROUP, OnUpdateMenuNewgroup)
ON_UPDATE_COMMAND_UI(ID_MENU_NEWGROUPSELECTION, OnUpdateMenuNewgroupselection)
ON_UPDATE_COMMAND_UI(ID_MENU_ALLWAYSONTOP, OnUpdateMenuAllwaysontop)
ON_UPDATE_COMMAND_UI(ID_MENU_VIEWFULLDESCRIPTION, OnUpdateMenuViewfulldescription)
ON_UPDATE_COMMAND_UI(ID_MENU_VIEWGROUPS, OnUpdateMenuViewgroups)
ON_UPDATE_COMMAND_UI(ID_MENU_PASTEPLAINTEXTONLY, OnUpdateMenuPasteplaintextonly)
ON_UPDATE_COMMAND_UI(ID_MENU_DELETE, OnUpdateMenuDelete)
ON_UPDATE_COMMAND_UI(ID_MENU_PROPERTIES, OnUpdateMenuProperties)
ON_COMMAND(ID_QUICKOPTIONS_PROMPTTODELETECLIP, OnPromptToDeleteClip)
ON_WM_DESTROY()

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
ON_MESSAGE(CB_UPDOWN, OnUpDown)
ON_MESSAGE(NM_INACTIVE_TOOLTIPWND, OnToolTipWndInactive)
ON_MESSAGE(NM_SET_LIST_COUNT, OnSetListCount)
ON_MESSAGE(NM_REFRESH_ROW, OnRefeshRow)
ON_MESSAGE(NM_ITEM_DELETED, OnItemDeleted)
ON_WM_TIMER()ON_COMMAND(ID_MENU_EXPORT, OnMenuExport)
ON_COMMAND(ID_MENU_IMPORT, OnMenuImport)
ON_COMMAND(ID_QUICKPROPERTIES_REMOVEQUICKPASTE, OnQuickpropertiesRemovequickpaste)
ON_COMMAND(ID_MENU_EDITITEM, OnMenuEdititem)
ON_COMMAND(ID_MENU_NEWCLIP, OnMenuNewclip)
ON_UPDATE_COMMAND_UI(ID_MENU_EDITITEM, OnUpdateMenuEdititem)
ON_UPDATE_COMMAND_UI(ID_MENU_NEWCLIP, OnUpdateMenuNewclip)
ON_WM_CTLCOLOR_REFLECT()
ON_COMMAND_RANGE(3000, 4000, OnAddinSelect)
ON_MESSAGE(NM_ALL_SELECTED, OnSelectAll)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd message handlers

HBRUSH CQPasteWnd::CtlColor(CDC *pDC, UINT nCtlColor)
{
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetBkColor(RGB(255, 0, 0));

    return (HBRUSH)GetStockObject(NULL_BRUSH);
}

BOOL CQPasteWnd::Create(const POINT &ptStart, CWnd *pParentWnd)
{
    // Get the previous size of the QPaste window
    CSize szWnd;
    CGetSetOptions::GetQuickPasteSize(szWnd);

    return CWndEx::Create(CRect(ptStart, szWnd), pParentWnd);
}

int CQPasteWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if(CWndEx::OnCreate(lpCreateStruct) ==  - 1)
    {
        return  - 1;
    }

    SetWindowText(_T(QPASTE_TITLE));

    m_Search.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER, CRect(0, 0, 0, 0), this, ID_EDIT_SEARCH);

    // Create the header control
    if(!m_lstHeader.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA, CRect(0, 0, 0, 0), this, ID_LIST_HEADER))
    {
        ASSERT(FALSE);
        return  - 1;
    }

    ((CWnd*) &m_GroupTree)->CreateEx(NULL, _T("SysTreeView32"), NULL, WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS, CRect(0, 0, 100, 100), this, 0);
    m_GroupTree.ModifyStyle(WS_CAPTION, WS_BORDER);

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

    m_btCancel.Create(_T("&C"), WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP /*|BS_FLAT*/, CRect(0, 0, 0, 0), this, ID_CANCEL);

    m_stGroup.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, ID_GROUP_TEXT);

    //Set the z-order
    m_lstHeader.SetWindowPos(this, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    m_Search.SetWindowPos(&m_lstHeader, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    m_btCancel.SetWindowPos(&m_Search, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    m_ShowGroupsFolderBottom.SetWindowPos(&m_Search, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    //LVS_EX_FLATSB
    m_lstHeader.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

    // Create the columns
    if(m_lstHeader.InsertColumn(0, _T(""), LVCFMT_LEFT, 2500, 0) != 0)
    {
        ASSERT(FALSE);
        return  - 1;
    }

    #ifdef AFTER_98
        m_Alpha.SetWindowHandle(m_hWnd);
    #endif 

    m_TitleFont.CreateFont(14, 0,  - 900, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));

    m_SearchFont.CreatePointFont(80, _T("@Arial Unicode MS"));

    GroupFont.CreateFont( - 11, 0, 0, 0, 400, 0, 1, 0, DEFAULT_CHARSET, 3, 2, 1, 34, _T("MS Sans Serif"));

    m_Search.SetFont(&m_SearchFont);
    m_btCancel.SetFont(&m_SearchFont);
    m_stGroup.SetFont(&GroupFont);

    UpdateFont();

    m_thread.Start(this);

    return 0;
}

void CQPasteWnd::OnSize(UINT nType, int cx, int cy)
{
    CWndEx::OnSize(nType, cx, cy);

    if(!IsWindow(m_lstHeader.m_hWnd))
    {
        return ;
    }

    MoveControls();
}

void CQPasteWnd::MoveControls()
{
    CRect crRect;
    GetClientRect(crRect);
    int cx = crRect.Width();
    int cy = crRect.Height();

    long lTopOfListBox = 0;

    if(theApp.m_GroupID > 0)
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
    m_lstHeader.MoveWindow(0, lTopOfListBox, cx, cy - 25-lTopOfListBox);

    int nWidth = cx;

    if(m_strSQLSearch.IsEmpty() == FALSE)
    {
        m_btCancel.ShowWindow(SW_SHOW);
        m_btCancel.MoveWindow(cx - 17, cy - 18, 15, 15);
        nWidth -= 19;
    }
    else
    {
        m_btCancel.ShowWindow(SW_HIDE);
    }

    m_Search.MoveWindow(18, cy - 20, nWidth - 20, 19);

    m_ShowGroupsFolderBottom.MoveWindow(0, cy - 19, 18, 16);

    // Set the column widths
    //m_lstHeader.SetColumnWidth(0, cx);
}

void CQPasteWnd::OnSetFocus(CWnd *pOldWnd)
{
    CWndEx::OnSetFocus(pOldWnd);

    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

    // Set the focus to the list control
    if(::IsWindow(m_lstHeader.m_hWnd))
    {
        m_lstHeader.SetFocus();
    }
}

void CQPasteWnd::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
    CWndEx::OnActivate(nState, pWndOther, bMinimized);

    if(m_bHideWnd == false || m_lstHeader.GetToolTipHWnd() == pWndOther->GetSafeHwnd())
    {
        return ;
    }

    if(nState == WA_INACTIVE)
    {
        m_bModifersMoveActive = false;

        if(!g_Opt.m_bShowPersistent)
        {
            HideQPasteWindow();
        }

        //re register the global hot keys for the last ten
        if(theApp.m_bAppExiting == false)
        {
            g_HotKeys.RegisterAll();
        }

        m_lstHeader.HidePopup();
    }
    else if(nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
    {
        if(theApp.m_bShowingQuickPaste == false)
        {
            ShowQPasteWindow(m_mapCache.size() == 0);
        }

        //Unregister the global hot keys for the last ten copies
        g_HotKeys.UnregisterAll(false, true);
    }
}

BOOL CQPasteWnd::HideQPasteWindow()
{
    Log(_T("Start of HideQPasteWindow"));

    if(!theApp.m_bShowingQuickPaste)
    {
        Log(_T("End of HideQPasteWindow, !theApp.m_bShowingQuickPaste"));
        return FALSE;
    }

	{
		ATL::CCritSecLock csLock(m_CritSection.m_sect);
    	
		m_bStopQuery = true;
	}

    //Reset the flagShell_TrayWnd
    theApp.m_bShowingQuickPaste = false;
    theApp.m_activeWnd.ReleaseFocus();

    m_thread.FireUnloadAccelerators();

    KillTimer(TIMER_FILL_CACHE);

    //Save the size
    CRect rect;
    GetWindowRectEx(&rect);
    CGetSetOptions::SetQuickPasteSize(rect.Size());
    CGetSetOptions::SetQuickPastePoint(rect.TopLeft());

    // Hide the window when the focus is lost
    ShowWindow(SW_HIDE);

    //Reset the selection in the search combo
    m_bHandleSearchTextChange = false;
    m_Search.SetWindowText(_T(""));
    m_bHandleSearchTextChange = true;

    if(m_strSQLSearch.IsEmpty() == FALSE)
    {
        {
			ATL::CCritSecLock csLock(m_CritSection.m_sect);
          
			m_bStopQuery = true;
        }

        //Wait for the thread to stop fill the cache so we can clear it
        WaitForSingleObject(m_thread.m_SearchingEvent, 5000);

        {
			ATL::CCritSecLock csLock(m_CritSection.m_sect);
            
			m_mapCache.clear();
            m_lstHeader.SetItemCountEx(0);
        }
    }

    Log(_T("End of HideQPasteWindow"));

    return TRUE;
}

BOOL CQPasteWnd::ShowQPasteWindow(BOOL bFillList)
{
    theApp.m_bShowingQuickPaste = true;

    Log(StrF(_T("ShowQPasteWindow - Fill List: %d"), bFillList));

    //Ensure we have the latest theme file, this checks the last write time so it doesn't read the file each time
    g_Opt.m_Theme.Load(g_Opt.GetTheme(), false, true);

    SetCaptionColorActive(g_Opt.m_bShowPersistent, theApp.GetConnectCV());

    // use invalidation to avoid unnecessary repainting
    m_bAllowRepaintImmediately = false;
    UpdateStatus();

    m_thread.FireLoadAccelerators();

    m_bHideWnd = true;

    #ifdef AFTER_98
        //Set the transparency
        if(CGetSetOptions::GetEnableTransparency())
        {
            m_Alpha.SetTransparent(TRUE);

            float fPercent = CGetSetOptions::GetTransparencyPercent() / (float)100.0;

            m_Alpha.SetOpacity(OPACITY_MAX - (int)(fPercent *OPACITY_MAX));
        }
        else
        {
            m_Alpha.SetTransparent(FALSE);
        }
    #endif 

    m_lstHeader.SetNumberOfLinesPerRow(CGetSetOptions::GetLinesPerRow());

    m_lstHeader.SetShowTextForFirstTenHotKeys(CGetSetOptions::GetShowTextForFirstTenHotKeys());

    if(bFillList)
    {
        FillList();
    }
    else
    {
        //MoveControls();
    }

    // from now on, for interactive use, we can repaint immediately
    m_bAllowRepaintImmediately = true;

    // always on top... for persistent showing (g_Opt.m_bShowPersistent)
    // SHOWWINDOW was also integrated into this function rather than calling it separately
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

    SetKeyModiferState(true);

    return TRUE;
}

bool CQPasteWnd::Add(const CString &csHeader, const CString &csText, int nID)
{
    int nNewIndex;

    if(g_Opt.m_bHistoryStartTop)
    {
        // Insert the item in the list control
        if((nNewIndex = m_lstHeader.InsertItem(m_lstHeader.GetItemCount(), csHeader)) ==  - 1)
        {
            return false;
        }
    }
    else
    {
        if((nNewIndex = m_lstHeader.InsertItem(m_lstHeader.GetItemCount(), csHeader)) ==  - 1)
        {
            return false;
        }
    }

    m_lstHeader.SetItemData(nNewIndex, nID);

    return true;
}

BOOL CQPasteWnd::OpenID(long lID, bool bOnlyLoad_CF_TEXT, CClipFormats *pPasteFormats)
{
    Log(StrF(_T("Start OpenId, Id: %d, Only CF_TEXT: %d"), lID, bOnlyLoad_CF_TEXT));

    if(pPasteFormats == NULL)
    {
        if(theApp.EnterGroupID(lID))
        {
            Log(_T("Entered group"));
            return TRUE;
        }
    }

    if(GetKeyState(VK_SHIFT) &0x8000)
    {
        Log(_T("Shift key is down, pasting CF_TEXT only"));
        bOnlyLoad_CF_TEXT = true;
    }

    // else, it is a clip, so paste it
    CProcessPaste paste;

    paste.m_bSendPaste = g_Opt.m_bSendPasteMessageAfterSelection == TRUE ? true : false;
    paste.m_bOnlyPaste_CF_TEXT = bOnlyLoad_CF_TEXT;

    if(pPasteFormats != NULL)
    {
        paste.SetCustomPasteFormats(pPasteFormats);
    }
    else
    {
        paste.GetClipIDs().Add(lID);
    }
    paste.DoPaste();
    theApp.OnPasteCompleted();

    if(g_Opt.m_bSendPasteMessageAfterSelection == FALSE)
    {
        theApp.m_activeWnd.ActivateTarget();
    }

    if(g_Opt.m_bShowPersistent && g_Opt.GetAutoHide())
    {
        MinMaxWindow(FORCE_MIN);
    }

    Log(StrF(_T("End OpenId, Id: %d, Only CF_TEXT: %d"), lID, bOnlyLoad_CF_TEXT));

    return TRUE;
}

BOOL CQPasteWnd::OpenSelection(bool bOnlyLoad_CF_TEXT)
{
    Log(_T("Start Open Selection"));
    ARRAY IDs;
    m_lstHeader.GetSelectionItemData(IDs);

    int count = IDs.GetSize();

    if(count <= 0)
    {
        return FALSE;
    }

    if(count == 1)
    {
        return OpenID(IDs[0], bOnlyLoad_CF_TEXT);
    }

    if(GetKeyState(VK_SHIFT) &0x8000)
    {
        bOnlyLoad_CF_TEXT = true;
    }

    CProcessPaste paste;

    paste.m_bSendPaste = g_Opt.m_bSendPasteMessageAfterSelection == TRUE ? true : false;
    paste.m_bOnlyPaste_CF_TEXT = bOnlyLoad_CF_TEXT;

    paste.GetClipIDs().Copy(IDs);
    paste.DoPaste();
    theApp.OnPasteCompleted();

    if(g_Opt.m_bSendPasteMessageAfterSelection == FALSE)
    {
        theApp.m_activeWnd.ActivateTarget();
    }

    if(g_Opt.m_bShowPersistent && g_Opt.GetAutoHide())
    {
        MinMaxWindow(FORCE_MIN);
    }

    Log(_T("End Open Selection"));
    return TRUE;
}

BOOL CQPasteWnd::OpenIndex(long nItem)
{
    return OpenID(m_lstHeader.GetItemData(nItem));
}

BOOL CQPasteWnd::NewGroup(bool bGroupSelection)
{
    //Get the selected ids
    CClipIDs IDs;
    m_lstHeader.GetSelectionItemData(IDs);

    CGroupName Name;
    CString csName("");

    if(g_Opt.m_bPrompForNewGroupName)
    {
        m_bHideWnd = false;

        int nRet = Name.DoModal();

        m_bHideWnd = true;

        if(nRet == IDOK)
        {
            csName = Name.m_csName;
        }
        else
        {
            return false;
        }
    }

    long lID = NewGroupID(theApp.GetValidGroupID(), csName);

    if(lID <= 0)
    {
        return FALSE;
    }

    if(!bGroupSelection)
    {
        theApp.m_FocusID = lID; // focus on the new group
        FillList();
        return TRUE;
    }

    IDs.MoveTo(lID);
    theApp.EnterGroupID(lID);
    return TRUE;
}

LRESULT CQPasteWnd::OnListSelect_DB_ID(WPARAM wParam, LPARAM lParam)
{
    OpenID(wParam);
    return TRUE;
}

LRESULT CQPasteWnd::OnListSelect_Index(WPARAM wParam, LPARAM lParam)
{
    if((int)wParam >= m_lstHeader.GetItemCount())
    {
        return FALSE;
    }

    OpenIndex(wParam);

    return TRUE;
}

LRESULT CQPasteWnd::OnListSelect(WPARAM wParam, LPARAM lParam)
{
    int nCount = (int)wParam;
    long *pItems = (long*)lParam;

    OpenSelection(false);

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
	while(::PeekMessage(&msg, m_hWnd, WM_REFRESH_VIEW, WM_REFRESH_VIEW, PM_REMOVE)){}

	Log(_T("OnRefreshView - Start"));

	theApp.m_FocusID =  - 1;

	if(theApp.m_bShowingQuickPaste)
	{
		FillList();
	}
    else
    {
        //Wait for the thread to stop fill the cache so we can clear it
        WaitForSingleObject(m_thread.m_SearchingEvent, 5000);

		{
			ATL::CCritSecLock csLock(m_CritSection.m_sect);
			m_mapCache.clear();
		}

		m_lstHeader.SetItemCountEx(0);
		m_lstHeader.RefreshVisibleRows();
    }

    Log(_T("OnRefreshView - End"));

    return TRUE;
}

void CQPasteWnd::RefreshNc(bool bRepaintImmediately)
{
    if(!theApp.m_bShowingQuickPaste)
    {
        return ;
    }

    if(bRepaintImmediately && m_bAllowRepaintImmediately)
    {
        OnNcPaint();
    }
    else
    {
        InvalidateNc();
    }
}

void CQPasteWnd::UpdateStatus(bool bRepaintImmediately)
{
    CString title = m_Title;
    CString prev;

    GetWindowText(prev);

    CString cs;
    cs.Format(_T(" - %d/%d"), m_lstHeader.GetSelectedCount(), m_lstHeader.GetItemCount());
    title += cs;

    if(theApp.m_Status != "")
    {
        title += " [ ";
        title += theApp.m_Status;
        title += " ] - ";
    }
    else
    {
        title += " - ";
    }

    if(theApp.m_IC_IDs.GetSize() > 0)
    {
        if(theApp.m_IC_bCopy)
        {
            title += "Copying";
        }
        else
        {
            title += "Moving";
        }

        title += " - ";
    }

    if(::IsWindow(theApp.m_activeWnd.ActiveWnd()))
    {
        title += theApp.m_activeWnd.ActiveWndName();
    }
    else
    {
        title += theApp.m_Language.GetString("No_Target", "No target");
    }

    if(title != prev)
    {
        SetWindowText(title);
        RefreshNc(bRepaintImmediately);
    }
}

BOOL CQPasteWnd::FillList(CString csSQLSearch /*=""*/)
{
    KillTimer(TIMER_DO_SEARCH);

    Log(StrF(_T("Fill List - %s"), csSQLSearch));

	{
	    ATL::CCritSecLock csLock(m_CritSection.m_sect);
	    m_bStopQuery = true;
	}

    CString strFilter;
    CString strParentFilter;
    CString csSort;

    // History Group
    if(theApp.m_GroupID < 0)
    {
        m_lstHeader.m_bStartTop = g_Opt.m_bHistoryStartTop;
        if(g_Opt.m_bHistoryStartTop)
        {
            csSort = "bIsGroup ASC, lDate DESC";
        }
        else
        {
            csSort = "bIsGroup ASC, lDate ASC";
        }

        if(g_Opt.m_bShowAllClipsInMainList)
        {
            strFilter = "((bIsGroup = 1 AND lParentID = -1) OR bIsGroup = 0)";
        }
        else
        {
            strFilter = "((bIsGroup = 1 AND lParentID = -1) OR (bIsGroup = 0 AND lParentID = -1))";
        }
    }
    else
    // it's some other group
    {
        m_lstHeader.m_bStartTop = true;

        if(g_Opt.m_bHistoryStartTop)
        {
            csSort = "bIsGroup DESC, lDate DESC";
        }
        else
        {
            csSort = "bIsGroup ASC, lDate ASC";
        }

        if(theApp.m_GroupID >= 0)
        {
            strFilter.Format(_T("lParentID = %d"), theApp.m_GroupID);
            strParentFilter = strFilter;
        }

        m_stGroup.SetWindowText(theApp.m_GroupText);
    }

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

        strFilter = SQLFormat.GetSQLString();

        if(strParentFilter.IsEmpty() == FALSE)
        {
            strFilter += " AND ";
            strFilter += strParentFilter;
        }

        m_strSQLSearch = strFilter;
    }

	{
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

	    //Format the count and select sql queries for the thread
	    m_CountSQL.Format(_T("SELECT COUNT(lID) FROM Main where %s"), strFilter);

	    m_SQL.Format(_T("SELECT lID, mText, lParentID, lDontAutoDelete, ")_T("lShortCut, bIsGroup, QuickPasteText FROM Main where %s order by %s"), strFilter, csSort);

	    m_lItemsPerPage = m_lstHeader.GetCountPerPage();
	}

    m_thread.FireDoQuery();

    return TRUE;
}


void CQPasteWnd::OnRclickQuickPaste(NMHDR *pNMHDR, LRESULT *pResult)
{
    POINT pp;
    CMenu cmPopUp;
    CMenu *cmSubMenu = NULL;

    GetCursorPos(&pp);
    if(cmPopUp.LoadMenu(IDR_QUICK_PASTE) != 0)
    {
        cmSubMenu = cmPopUp.GetSubMenu(0);
        if(!cmSubMenu)
        {
            return ;
        }

        if(pNMHDR == NULL)
        {
            int nItem = m_lstHeader.GetCaret();
            CRect rc;
            m_lstHeader.GetItemRect(nItem, rc, LVIR_BOUNDS);
            ClientToScreen(rc);
            pp.x = rc.left;
            pp.y = rc.bottom;
        }

        theApp.m_Addins.AddPrePasteAddinsToMenu(cmSubMenu);

        theApp.m_Language.UpdateRightClickMenu(cmSubMenu);

        SetMenuChecks(cmSubMenu);

        cmSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pp.x, pp.y, this, NULL);
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
        int nCheckID =  - 1;
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
        {
            pMenu->CheckMenuItem(nCheckID, MF_CHECKED);
        }
    }

    //Set the lines per row check
    int nLinesPerRow = CGetSetOptions::GetLinesPerRow();
    int nCheckID =  - 1;
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
    {
        pMenu->CheckMenuItem(nCheckID, MF_CHECKED);
    }

    //Set the position check
    int nPosition = CGetSetOptions::GetQuickPastePosition();
    nCheckID =  - 1;
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
    {
        pMenu->CheckMenuItem(nCheckID, MF_CHECKED);
    }

    theApp.UpdateMenuConnectCV(pMenu, ID_MENU_TOGGLECONNECTCV);

    if(CGetSetOptions::GetShowTextForFirstTenHotKeys())
    {
        pMenu->CheckMenuItem(ID_MENU_FIRSTTENHOTKEYS_SHOWHOTKEYTEXT, MF_CHECKED);
    }

    if(CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys())
    {
        pMenu->CheckMenuItem(ID_MENU_FIRSTTENHOTKEYS_USECTRLNUM, MF_CHECKED);
    }

    if(g_Opt.m_bShowPersistent)
    {
        pMenu->CheckMenuItem(ID_MENU_ALLWAYSONTOP, MF_CHECKED);
    }

    if(CGetSetOptions::GetAutoHide())
    {
        pMenu->CheckMenuItem(ID_MENU_AUTOHIDE, MF_CHECKED);
    }

    if(g_Opt.m_bHistoryStartTop)
    {
        pMenu->CheckMenuItem(ID_SORT_ASCENDING, MF_CHECKED);
    }
    else
    {
        pMenu->CheckMenuItem(ID_SORT_DESCENDING, MF_CHECKED);
    }

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
    {
        pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_PROMPTFORNEWGROUPNAMES, MF_CHECKED);
    }

    if(g_Opt.m_bDrawThumbnail)
    {
        pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_SHOWTHUMBNAILS, MF_CHECKED);
    }

    if(g_Opt.m_bDrawRTF)
    {
        pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_DRAWRTFTEXT, MF_CHECKED);
    }

    if(g_Opt.m_bSendPasteMessageAfterSelection)
    {
        pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_PASTECLIPAFTERSELECTION, MF_CHECKED);
    }

    if(g_Opt.m_bFindAsYouType)
    {
        pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_FINDASYOUTYPE, MF_CHECKED);
    }

    if(g_Opt.m_bEnsureEntireWindowCanBeSeen)
    {
        pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_ENSUREENTIREWINDOWISVISIBLE, MF_CHECKED);
    }

    if(g_Opt.m_bShowAllClipsInMainList)
    {
        pMenu->CheckMenuItem(ID_MENU_QUICKOPTIONS_SHOWCLIPSTHATAREINGROUPSINMAINLIST, MF_CHECKED);
    }

    if(g_Opt.GetPromptWhenDeletingClips())
    {
        pMenu->CheckMenuItem(ID_QUICKOPTIONS_PROMPTTODELETECLIP, MF_CHECKED);
    }

    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIENDONE, 0);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_TWO, 1);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_THREE, 2);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FORE, 3);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FIVE, 4);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_SIX, 5);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_SEVEN, 6);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_EIGHT, 7);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_NINE, 8);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_TEN, 9);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_ELEVEN, 10);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_TWELVE, 11);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_THIRTEEN, 12);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FORETEEN, 13);
    SetSendToMenu(pMenu, ID_MENU_SENTTO_FRIEND_FIFTEEN, 14);

    if(g_Opt.GetAllowFriends() == false)
    {
        CString csText("Send To");
        int nPos =  - 1;
        CMultiLanguage::GetMenuPos(pMenu, csText, nPos);
        if(nPos >= 0)
        {
            pMenu->DeleteMenu(nPos, MF_BYPOSITION);
        }
    }

    pMenu->DeleteMenu(ID_MENU_SENTTO_PROMPTFORIP, MF_BYCOMMAND);
}

void CQPasteWnd::SetSendToMenu(CMenu *pMenu, int nMenuID, int nArrayPos)
{
    if(g_Opt.m_SendClients[nArrayPos].csIP.GetLength() > 0)
    {
        CString cs;
        cs.Format(_T("(%s) - %s"), g_Opt.m_SendClients[nArrayPos].csIP, g_Opt.m_SendClients[nArrayPos].csDescription);
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
    m_Search.GetWindowText(csText);

    if(csText == "")
    {
        return FALSE;
    }

    FillList(csText);

    m_lstHeader.SetFocus();

    MoveControls();

    m_Search.SetSel( - 1, 0);

    return TRUE;
}


///////////////////////////////////////////////////////////////////////
//Menu Stuff
///////////////////////////////////////////////////////////////////////
void CQPasteWnd::OnMenuLinesperrow1()
{
    SetLinesPerRow(1);
}

void CQPasteWnd::OnMenuLinesperrow2()
{
    SetLinesPerRow(2);
}

void CQPasteWnd::OnMenuLinesperrow3()
{
    SetLinesPerRow(3);
}

void CQPasteWnd::OnMenuLinesperrow4()
{
    SetLinesPerRow(4);
}

void CQPasteWnd::OnMenuLinesperrow5()
{
    SetLinesPerRow(5);
}

void CQPasteWnd::SetLinesPerRow(long lLines)
{
    CGetSetOptions::SetLinesPerRow(lLines);
    m_lstHeader.SetNumberOfLinesPerRow(lLines);

    FillList();
}

void CQPasteWnd::OnMenuTransparencyNone()
{
    SetTransparency(0);
}

void CQPasteWnd::OnMenuTransparency5()
{
    SetTransparency(5);
}

void CQPasteWnd::OnMenuTransparency10()
{
    SetTransparency(10);
}

void CQPasteWnd::OnMenuTransparency15()
{
    SetTransparency(15);
}

void CQPasteWnd::OnMenuTransparency20()
{
    SetTransparency(20);
}

void CQPasteWnd::OnMenuTransparency25()
{
    SetTransparency(25);
}

void CQPasteWnd::OnMenuTransparency30()
{
    SetTransparency(25);
}

void CQPasteWnd::OnMenuTransparency40()
{
    SetTransparency(40);
}

void CQPasteWnd::SetTransparency(long lPercent)
{
    #ifdef AFTER_98
        if(lPercent)
        {
            CGetSetOptions::SetTransparencyPercent(lPercent);
            CGetSetOptions::SetEnableTransparency(TRUE);

            m_Alpha.SetTransparent(TRUE);

            float fPercent = lPercent / (float)100.0;

            m_Alpha.SetOpacity(OPACITY_MAX - (int)(fPercent *OPACITY_MAX));
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
    theApp.ShowOptionsDlg();
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
    {
        return ;
    }

    long lID = IDs[0];
    int nRow = Indexes[0];

    if(lID < 0)
    {
        return ;
    }

    m_lstHeader.RemoveAllSelection();
    m_lstHeader.SetSelection(nRow);

    CCopyProperties props(lID, this);
    int nDo = props.DoModal();

    if(nDo == IDOK)
    {
        {
			ATL::CCritSecLock csLock(m_CritSection.m_sect);

            MainTypeMap::iterator iter = m_mapCache.find(nRow);
            if(iter != m_mapCache.end())
            {
                CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT * FROM Main WHERE lID = %d"), lID);
                if(!q.eof())
                {
                    FillMainTable(iter->second, q);
                }
            }
        }

        m_thread.FireLoadAccelerators();

        m_lstHeader.RefreshVisibleRows();

        if(props.m_lGroupChangedTo >= 0)
        {
            OpenID(props.m_lGroupChangedTo);
        }

        m_lstHeader.SetFocus();
        m_lstHeader.SetListPos(nRow);
    }

    m_bHideWnd = true;
}

void CQPasteWnd::UpdateFont()
{
    LOGFONT lf;
    CGetSetOptions::GetFont(lf);
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
    theApp.ShowPersistent(!g_Opt.m_bShowPersistent);
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
    NewGroup(false);
}

void CQPasteWnd::OnMenuNewGroupSelection()
{
    NewGroup(true);
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
    g_Opt.SetPrompForNewGroupName(!g_Opt.m_bPrompForNewGroupName);
}

void CQPasteWnd::OnMenuViewgroups()
{
    OnShowGroupsTop();
}

void CQPasteWnd::OnMenuQuickpropertiesSettoneverautodelete()
{
    CWaitCursor wait;
    ARRAY IDs;
    ARRAY Indexs;
    m_lstHeader.GetSelectionItemData(IDs);
    m_lstHeader.GetSelectionIndexes(Indexs);

    int count = IDs.GetSize();

    for(int i = 0; i < count; i++)
    {
        try
        {
            theApp.m_db.execDMLEx(_T("UPDATE Main SET lDontAutoDelete = %d where lID = %d;"), (long)CTime::GetCurrentTime().GetTime(), IDs[i]);
        }
        CATCH_SQLITE_EXCEPTION
    }

    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        count = Indexs.GetSize();
        for(int row = 0; row < count; row++)
        {
            MainTypeMap::iterator iter = m_mapCache.find(row);
            if(iter != m_mapCache.end())
            {
                iter->second.m_bDontAutoDelete = true;
            }
        }
    }

    m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuQuickpropertiesAutodelete()
{
    CWaitCursor wait;
    ARRAY IDs;
    ARRAY Indexs;
    m_lstHeader.GetSelectionItemData(IDs);
    m_lstHeader.GetSelectionIndexes(Indexs);

    int count = IDs.GetSize();

    for(int i = 0; i < count; i++)
    {
        try
        {
            theApp.m_db.execDMLEx(_T("UPDATE Main SET lDontAutoDelete = 0 where lID = %d;"), IDs[i]);
        }
        CATCH_SQLITE_EXCEPTION
    }

    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);
        count = Indexs.GetSize();
        for(int row = 0; row < count; row++)
        {
            MainTypeMap::iterator iter = m_mapCache.find(row);
            if(iter != m_mapCache.end())
            {
                iter->second.m_bDontAutoDelete = false;
            }
        }
    }

    m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuQuickpropertiesRemovehotkey()
{
    CWaitCursor wait;
    ARRAY IDs;
    ARRAY Indexs;
    m_lstHeader.GetSelectionItemData(IDs);
    m_lstHeader.GetSelectionIndexes(Indexs);

    int count = IDs.GetSize();

    for(int i = 0; i < count; i++)
    {
        try
        {
            theApp.m_db.execDMLEx(_T("UPDATE Main SET lShortCut = 0 where lID = %d;"), IDs[i]);
        }
        CATCH_SQLITE_EXCEPTION
    }

    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        count = Indexs.GetSize();
        for(int row = 0; row < count; row++)
        {
            MainTypeMap::iterator iter = m_mapCache.find(row);
            if(iter != m_mapCache.end())
            {
                iter->second.m_bHasShortCut = false;
            }
        }
    }

    m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnQuickpropertiesRemovequickpaste()
{
    CWaitCursor wait;
    ARRAY IDs;
    ARRAY Indexs;
    m_lstHeader.GetSelectionItemData(IDs);
    m_lstHeader.GetSelectionIndexes(Indexs);

    int count = IDs.GetSize();

    for(int i = 0; i < count; i++)
    {
        try
        {
            theApp.m_db.execDMLEx(_T("UPDATE Main SET QuickPasteText = '' where lID = %d;"), IDs[i]);
        }
        CATCH_SQLITE_EXCEPTION
    }

    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        count = Indexs.GetSize();
        for(int row = 0; row < count; row++)
        {
            MainTypeMap::iterator iter = m_mapCache.find(row);
            if(iter != m_mapCache.end())
            {
                iter->second.m_QuickPaste.Empty();
            }
        }
    }

    m_lstHeader.RefreshVisibleRows();
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

    int nRet = dlg.DoModal();
    if(nRet == IDOK)
    {
        int nGroup = dlg.GetSelectedGroup();
        if(nGroup >= 0)
        {
            CClipIDs IDs;
            m_lstHeader.GetSelectionItemData(IDs);

            IDs.MoveTo(nGroup);
        }
        FillList();
    }

    m_bHideWnd = true;
}

void CQPasteWnd::OnMenuPasteplaintextonly()
{
    OpenSelection(true);
}

void CQPasteWnd::OnPromptToDeleteClip()
{
    CGetSetOptions::SetPromptWhenDeletingClips(!CGetSetOptions::GetPromptWhenDeletingClips());
}

void CQPasteWnd::OnMenuExport()
{
    CClipIDs IDs;
    long lCount = m_lstHeader.GetSelectedCount();
    if(lCount <= 0)
    {
        return ;
    }

    m_lstHeader.GetSelectionItemData(IDs);
    lCount = IDs.GetSize();
    if(lCount <= 0)
    {
        return ;
    }

    OPENFILENAME ofn;
    TCHAR szFile[400];
    TCHAR szDir[400];

    memset(&szFile, 0, sizeof(szFile));
    memset(szDir, 0, sizeof(szDir));
    memset(&ofn, 0, sizeof(ofn));

    CString csInitialDir = CGetSetOptions::GetLastImportDir();
    STRCPY(szDir, csInitialDir);

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("Exported Ditto Clips (.dto)\0*.dto\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = szDir;
    ofn.lpstrDefExt = _T("dto");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    m_bHideWnd = false;

    if(GetSaveFileName(&ofn))
    {
        using namespace nsPath;
        CPath path(ofn.lpstrFile);
        CString csPath = path.GetPath();
        CGetSetOptions::SetLastExportDir(csPath);

        CString csFile(ofn.lpstrFile);
        IDs.Export(csFile);
    }

    m_bHideWnd = true;
}

void CQPasteWnd::OnMenuImport()
{
    m_bHideWnd = false;
    theApp.ImportClips(m_hWnd);
    m_bHideWnd = true;
}

void CQPasteWnd::OnMenuHelp()
{
    CString csFile = CGetSetOptions::GetPath(PATH_HELP);
    csFile += "DittoGettingStarted.htm";

    CHyperLink::GotoURL(csFile, SW_SHOW);
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
    m_Search.GetWindowText(csText);
    FillList(csText);
}

void CQPasteWnd::OnMenuEdititem()
{
    if(m_lstHeader.GetSelectedCount() == 0)
    {
        return ;
    }

    CClipIDs IDs;
    m_lstHeader.GetSelectionItemData(IDs);
    theApp.EditItems(IDs, true);

    HideQPasteWindow();
}

void CQPasteWnd::OnMenuNewclip()
{
    CClipIDs IDs;
    IDs.Add( - 1);
    theApp.EditItems(IDs, true);

    HideQPasteWindow();
}


///////////////////////////////////////////////////////////////////////
//END END Menu Stuff
///////////////////////////////////////////////////////////////////////


BOOL CQPasteWnd::SendToFriendbyPos(int nPos)
{
    if(g_Opt.GetAllowFriends() == false)
    {
        return FALSE;
    }

    CWaitCursor wait;

    m_bHideWnd = false;

    CClipIDs IDs;
    long lCount = m_lstHeader.GetSelectedCount();
    if(lCount <= 0)
    {
        return FALSE;
    }

    m_lstHeader.GetSelectionItemData(IDs);
    lCount = IDs.GetSize();
    if(lCount <= 0)
    {
        return FALSE;
    }

    CSendToFriendInfo Info;
    Info.m_lPos = nPos;

    BOOL bRet = FALSE;

    try
    {
        CPopup Popup(0, 0, m_hWnd);
        Popup.Show(StrF(_T("Sending clip to %s"), g_Opt.m_SendClients[nPos].csIP));

        Info.m_pPopup = &Popup;

        Info.m_pClipList = new CClipList;
        for(int i = 0; i < lCount; i++)
        {
            CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT mText FROM Main WHERE lID = %d"), IDs[i]);
            if(q.eof() == false)
            {
                CClip *pClip = new CClip;
                if(pClip)
                {
                    pClip->LoadFormats(IDs[i]);
                    pClip->m_Desc = q.getStringField(0);
                    pClip->m_ID = IDs[i];
                    Info.m_pClipList->AddTail(pClip);
                }
            }
        }

        if(SendToFriend(Info) == FALSE)
        {
            MessageBox(StrF(_T("Error Sending data to %s\n\n%s"), g_Opt.m_SendClients[nPos].csIP, Info.m_csErrorText), _T("Ditto"));
        }
        else
        {
            bRet = TRUE;
        }
    }
    CATCH_SQLITE_EXCEPTION 

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
    if(g_Opt.GetPromptWhenDeletingClips())
    {
        bool bStartValue = m_bHideWnd;
        m_bHideWnd = false;

        int nRet = MessageBox(theApp.m_Language.GetString("Delete_Clip", "Delete Selected Clips?"), _T("Ditto"), MB_YESNO);

        m_bHideWnd = bStartValue;

        if(nRet == IDNO)
        {
            return ;
        }
    }

    CClipIDs IDs;
    ARRAY Indexs;

    if(m_lstHeader.GetSelectedCount() == 0)
    {
        return ;
    }

    POSITION pos = m_lstHeader.GetFirstSelectedItemPosition();
    int nFirstSel = m_lstHeader.GetNextSelectedItem(pos);

    m_lstHeader.GetSelectionItemData(IDs);
    m_lstHeader.GetSelectionIndexes(Indexs);

    IDs.DeleteIDs(true, theApp.m_db);

    Indexs.SortDescending();
    int nCount = Indexs.GetSize();

    int erasedCount = 0;

    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        for(int i = 0; i < nCount; i++)
        {
            MainTypeMap::iterator iter = m_mapCache.find(Indexs[i]);
            if(iter != m_mapCache.end() && iter->second.m_lID > 0)
            {
                m_mapCache.erase(iter);
                erasedCount++;
            }
        }
    }

    m_lstHeader.SetItemCountEx(m_lstHeader.GetItemCount() - erasedCount);

    // if there are no items after the one we deleted, then select the last one.
    if(nFirstSel >= m_lstHeader.GetItemCount())
    {
        nFirstSel = m_lstHeader.GetItemCount() - 1;
    }

    m_lstHeader.SetListPos(nFirstSel);

    m_lstHeader.RefreshVisibleRows();
    RedrawWindow(0, 0, RDW_INVALIDATE);
}

CString CQPasteWnd::LoadDescription(int nItem)
{
    if(nItem < 0 || nItem >= m_lstHeader.GetItemCount())
    {
        return "";
    }

    CString cs;
    try
    {
        long lID = m_lstHeader.GetItemData(nItem);

        CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT mText FROM Main WHERE lID = %d"), lID);
        if(q.eof() == false)
        {
            cs = q.getStringField(0);
        }
    }
    CATCH_SQLITE_EXCEPTION 

    return cs;
}

void CQPasteWnd::MoveSelection(bool down)
{
    if(m_bModifersMoveActive)
    {
        ARRAY arr;
        m_lstHeader.GetSelectionIndexes(arr);
        if(arr.GetCount() > 0)
        {
            int index = arr[0];

            if(down)
            {
                if(index < m_lstHeader.GetItemCount() - 1)
                {
                    m_lstHeader.SetListPos(index + 1);
                }
            }
            else
            {
                if(index > 0)
                {
                    m_lstHeader.SetListPos(index - 1);
                }
            }
        }
        else
        {
            if(m_lstHeader.GetItemCount() > 0)
            {
                m_lstHeader.SetListPos(0);
            }
        }
    }
}

void CQPasteWnd::OnKeyStateUp()
{
    if(m_bModifersMoveActive)
    {
        Log(_T("OnKeyStateUp"));
        SetTimer(TIMER_PASTE_FROM_MODIFER, g_Opt.GetKeyStatePasteDelay(), NULL);
    }
    else
    {
        Log(_T("OnKeyStateUp - Modifers not active"));
    }
}

void CQPasteWnd::SetKeyModiferState(bool bActive)
{
    Log(StrF(_T("SetKeyModiferState %d"), bActive));
    m_bModifersMoveActive = bActive;
}

BOOL CQPasteWnd::PreTranslateMessage(MSG *pMsg)
{
    switch(pMsg->message)
    {
        case WM_KEYDOWN:

            switch(pMsg->wParam)
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
                    if(GetKeyState(VK_CONTROL) &0x8000)
                    {
                        NewGroup(false);
                    }
                    else
                    {
                        NewGroup(true);
                    }
                    return TRUE;
                }
                break;

            case VK_F5:
                //toggle outputing text to outputdebugstring
                if(GetKeyState(VK_CONTROL) &0x8000)
                {
                    if(CGetSetOptions::m_bEnableDebugLogging)
                    {
                        Log(_T("turning file logging OFF"));
                    }

                    CGetSetOptions::m_bEnableDebugLogging = !CGetSetOptions::m_bEnableDebugLogging;

                    if(CGetSetOptions::m_bEnableDebugLogging)
                    {
                        Log(_T("turning file logging ON"));
                    }
                }
                else
                {
                    if(CGetSetOptions::m_bEnableDebugLogging)
                    {
                        Log(_T("turning DebugString logging OFF"));
                    }

                    CGetSetOptions::m_bOutputDebugString = !CGetSetOptions::m_bOutputDebugString;

                    if(CGetSetOptions::m_bEnableDebugLogging)
                    {
                        Log(_T("turning DebugString logging ON"));
                    }
                }
                return TRUE;

            case VK_ESCAPE:
                {
                    if(m_bModifersMoveActive)
                    {
                        Log(_T("Escape key hit setting modifers to NOT active"));
                        m_bModifersMoveActive = false;
                        return TRUE;
                    }
                    else
                    {
                        if(m_lstHeader.HandleKeyDown(pMsg->wParam, pMsg->lParam) == FALSE)
                        {
                            if(m_strSQLSearch.IsEmpty() == FALSE)
                            {
                                OnCancelFilter();
                                return TRUE;
                            }
                            else
                            {
                                if(m_GroupTree.IsWindowVisible() == FALSE)
                                {
                                    HideQPasteWindow();
                                    return TRUE;
                                }
                            }
                        }
                    }
                    break;
                }
            case VK_TAB:
                {
                    BOOL bPrev = FALSE;

                    if(GetKeyState(VK_SHIFT) &0x8000)
                    {
                        bPrev = TRUE;
                    }

                    CWnd *pFocus = GetFocus();
                    if(pFocus)
                    {
                        CWnd *pNextWnd = GetNextDlgTabItem(pFocus, bPrev);
                        if(pNextWnd)
                        {
                            pNextWnd->SetFocus();
                        }
                    }
                    return TRUE;
                }
            case 'G':
                if(GetKeyState(VK_CONTROL) &0x8000)
                {
                    OnShowGroupsTop();
                    return TRUE;
                }
                break;
            case 'N':
                if(GetKeyState(VK_CONTROL) &0x8000)
                {
                    OnMenuNewclip();
                    return TRUE;
                }
                break;
            case 'E':
                if(GetKeyState(VK_CONTROL) &0x8000)
                {
                    OnMenuEdititem();
                    return TRUE;
                }
                break;

            case VK_UP:
                if(m_bModifersMoveActive)
                {
                    MoveSelection(false);
                    return TRUE;
                }
                break;

            case VK_DOWN:
                if(m_bModifersMoveActive)
                {
                    MoveSelection(true);
                    return TRUE;
                }
                break;

            case VK_HOME:
                if(m_bModifersMoveActive)
                {
                    m_lstHeader.SetListPos(0);
                    return TRUE;
                }
                break;
            case VK_END:
                if(m_bModifersMoveActive)
                {
                    if(m_lstHeader.GetItemCount() > 0)
                    {
                        m_lstHeader.SetListPos(m_lstHeader.GetItemCount() - 1);
                    }
                    return TRUE;
                }
                break;
            }
            // end switch( pMsg->wParam )

            break; // end case WM_KEYDOWN 

        case WM_SYSKEYDOWN:
            // ALT key is held down

            switch(pMsg->wParam)
            {
            case 'C':
                // switch to the filter combobox
                BYTE key[256];
                GetKeyboardState((LPBYTE)(&key));
                if(key[VK_MENU] &128)
                {
                    OnCancelFilter();
                }
                return TRUE;

            case VK_HOME:
                theApp.EnterGroupID( - 1); // History
                return TRUE;
            }
            // end switch( pMsg->wParam )

            break; // end case WM_SYSKEYDOWN
    }

    return CWndEx::PreTranslateMessage(pMsg);
}

void CQPasteWnd::OnCancelFilter()
{
    FillList();

    m_bHandleSearchTextChange = false;
    m_Search.SetWindowText(_T(""));
    m_bHandleSearchTextChange = true;

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

void CQPasteWnd::OnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;
    CProcessPaste paste;
    CClipIDs &clips = paste.GetClipIDs();

    m_lstHeader.GetSelectionItemData(clips);

    if(clips.GetSize() <= 0)
    {
        ASSERT(0); // does this ever happen ??
        clips.Add(m_lstHeader.GetItemData(pLV->iItem));
    }

    paste.DoDrag();

    *pResult = 0;
}

void CQPasteWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CWndEx::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CQPasteWnd::GetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    LV_DISPINFO *pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM *pItem = &(pDispInfo)->item;

    if(pItem->mask &LVIF_TEXT)
    {
        switch(pItem->iSubItem)
        {
            case 0:
                try
                {
					ATL::CCritSecLock csLock(m_CritSection.m_sect);

                    int c = m_lstHeader.GetItemCount();
                    MainTypeMap::iterator iter = m_mapCache.find(pItem->iItem);
                    if(iter != m_mapCache.end())
                    {
                        CString cs;
                        if(iter->second.m_bDontAutoDelete)
                        {
                            cs += "*";
                        }

                        if(iter->second.m_bHasShortCut)
                        {
                            cs += "s";
                        }

                        if(iter->second.m_bIsGroup)
                        {
                            cs += "G";
                        }

                        // attached to a group
                        if(iter->second.m_bHasParent)
                        {
                            cs += "!";
                        }

                        if(iter->second.m_QuickPaste.IsEmpty() == FALSE)
                        {
                            cs += "Q";
                        }

                        // pipe is the "end of symbols" marker
                        cs += "|" + CMainTableFunctions::GetDisplayText(g_Opt.m_nLinesPerRow, iter->second.m_Desc);

                        lstrcpyn(pItem->pszText, cs, pItem->cchTextMax);
                        pItem->pszText[pItem->cchTextMax - 1] = '\0';
                    }
                    else
                    {
						
                        bool addToLoadItems = true;

						for (std::list<CPoint>::iterator it = m_loadItems.begin(); it != m_loadItems.end(); it++)
						{
							if(pItem->iItem >= it->x && pItem->iItem <= it->y)
							{
								addToLoadItems = false;
								break;
							}
						}

						//Log(StrF(_T("DrawItem index %d, add: %d"), pItem->iItem, addToLoadItems));

                        if(addToLoadItems)
                        {
                            CPoint loadItem(pItem->iItem, (m_lstHeader.GetTopIndex() + m_lstHeader.GetCountPerPage() + 2));
                            m_loadItems.push_back(loadItem);
                        }

                        m_thread.FireLoadItems();
                    }
                }
                CATCH_SQLITE_EXCEPTION 

                break;
        }
    }

    if(pItem->mask &LVIF_PARAM)
    {
        switch(pItem->iSubItem)
        {
            case 0:
                {
					ATL::CCritSecLock csLock(m_CritSection.m_sect);

                    MainTypeMap::iterator iter = m_mapCache.find(pItem->iItem);
                    if(iter != m_mapCache.end())
                    {
                        pItem->lParam = iter->second.m_lID;
                    }
                }

                break;
        }
    }

    if(pItem->mask & LVIF_CF_DIB && g_Opt.m_bDrawThumbnail)
    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);
     
		MainTypeMap::iterator iter = m_mapCache.find(pItem->iItem);
        if(iter != m_mapCache.end())
        {
            CF_DibTypeMap::iterator iterDib = m_cf_dibCache.find(iter->second.m_lID);
            if(iterDib == m_cf_dibCache.end())
            {
                bool exists = false;
				for (std::list<CClipFormatQListCtrl>::iterator it = m_ExtraDataLoadItems.begin(); it != m_ExtraDataLoadItems.end(); it++)
				{
					if(it->m_cfType == CF_DIB && it->m_lDBID == iter->second.m_lID)
					{
						exists = true;
						break;
					}
				}

                if(exists == false)
                {
                    CClipFormatQListCtrl format;
                    format.m_cfType = CF_DIB;
                    format.m_lDBID = iter->second.m_lID;
                    format.m_clipRow = pItem->iItem;
                    format.m_autoDeleteData = false;
                    m_ExtraDataLoadItems.push_back(format);

                    m_thread.FireLoadExtraData();
                }
            }
            else
            {
                if(iterDib->second.m_hgData != NULL)
                {
                    pItem->lParam = (LPARAM) &(iterDib->second);
                }
            }
        }
    }

    if(pItem->mask & LVIF_CF_RICHTEXT && g_Opt.m_bDrawRTF)
    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        MainTypeMap::iterator iter = m_mapCache.find(pItem->iItem);
        if(iter != m_mapCache.end())
        {
            CF_DibTypeMap::iterator iterDib = m_cf_rtfCache.find(iter->second.m_lID);
            if(iterDib == m_cf_rtfCache.end())
            {
                bool exists = false;
				for (std::list<CClipFormatQListCtrl>::iterator it = m_ExtraDataLoadItems.begin(); it != m_ExtraDataLoadItems.end(); it++)
				{
					if(it->m_cfType == CF_DIB && it->m_lDBID == iter->second.m_lID)
					{
						exists = true;
						break;
					}
				}

                if(exists == false)
                {
                    CClipFormatQListCtrl format;
                    format.m_cfType = theApp.m_RTFFormat;
                    format.m_lDBID = iter->second.m_lID;
                    format.m_clipRow = pItem->iItem;
                    format.m_autoDeleteData = false;
                    m_ExtraDataLoadItems.push_back(format);

                    m_thread.FireLoadExtraData();
                }
            }
            else
            {
                if(iterDib->second.m_hgData != NULL)
                {
                    pItem->lParam = (LPARAM) &(iterDib->second);
                }
            }
        }
    }
}

CString CQPasteWnd::GetDisplayText(long lDontAutoDelete, long lShortCut, bool bIsGroup, long lParentID, CString csText)
{
    CString cs;
    if(lDontAutoDelete)
    {
        cs += "*";
    }

    if(lShortCut > 0)
    {
        cs += "s";
    }

    if(bIsGroup)
    {
        cs += "G";
    }

    // attached to a group
    if(lParentID > 0)
    {
        cs += "!";
    }

    // pipe is the "end of symbols" marker
    cs += "|" + CMainTableFunctions::GetDisplayText(g_Opt.m_nLinesPerRow, csText);

    return cs;
}

void CQPasteWnd::OnGetToolTipText(NMHDR *pNMHDR, LRESULT *pResult)
{
    CQListToolTipText *pInfo = (CQListToolTipText*)pNMHDR;
    if(!pInfo)
    {
        return ;
    }

    if(pInfo->lItem < 0)
    {
        CString cs("no item selected");

        lstrcpyn(pInfo->pszText, cs, pInfo->cchTextMax);
        if(cs.GetLength() > pInfo->cchTextMax)
        {
            pInfo->pszText[pInfo->cchTextMax - 1] = 0;
        }

        return ;
    }

    try
    {
        CString cs;

        long lID = m_lstHeader.GetItemData(pInfo->lItem);
        CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, mText, lDate, lShortCut, lDontAutoDelete, QuickPasteText FROM Main WHERE lID = %d"), lID);
        if(q.eof() == false)
        {
            cs = q.getStringField(1);
            cs += "\n\n";

            #ifdef _DEBUG
                cs += StrF(_T("(Index = %d) (DB ID = %d)\n"), pInfo->lItem, q.getIntField(_T("lID")));
            #endif 

            COleDateTime time((time_t)q.getIntField(_T("lDate")));
            cs += time.Format();

            if(q.getIntField(_T("lDontAutoDelete")) > 0)
            {
                cs += "\n";
                cs += "Never Auto Delete";
            }

            CString csQuickPaste = q.getStringField(_T("QuickPasteText"));

            if(csQuickPaste.IsEmpty() == FALSE)
            {
                cs += "\nQuick Paste = ";
                cs += csQuickPaste;
            }

            long lShortCut = q.getIntField(_T("lShortCut"));
            if(lShortCut > 0)
            {
                cs += "\n";

                if(HIBYTE(lShortCut) &HOTKEYF_CONTROL)
                {
                    cs += "Ctrl + ";
                }
                if(HIBYTE(lShortCut) &HOTKEYF_SHIFT)
                {
                    cs += "Shift + ";
                }
                if(HIBYTE(lShortCut) &HOTKEYF_ALT)
                {
                    cs += "Alt + ";
                }

                cs += (char)LOBYTE(lShortCut);
            }
        }

        lstrcpyn(pInfo->pszText, cs, pInfo->cchTextMax);
        pInfo->pszText[pInfo->cchTextMax - 1] = '\0';
    }
    CATCH_SQLITE_EXCEPTION
}

void CQPasteWnd::OnFindItem(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVFINDITEM *pFindInfo = (NMLVFINDITEM*)pNMHDR;
    LVFINDINFO fndItem = pFindInfo->lvfi;

    if(fndItem.flags &LVFI_STRING)
    {
        m_Search.SetWindowText(fndItem.psz);
        m_Search.SetFocus();
        m_Search.SetSel(1, 1);

        *pResult = m_lstHeader.GetCaret();
        return ;
    }

    *pResult =  - 1; // Default action.
}

void CQPasteWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
    // toggle ShowPersistent when we double click the caption
    if(nHitTest == HTCAPTION)
    {
        switch(g_Opt.m_bDoubleClickingOnCaptionDoes)
        {
            case TOGGLES_ALLWAYS_ON_TOP:
                theApp.ShowPersistent(!g_Opt.m_bShowPersistent);
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

void CQPasteWnd::OnWindowPosChanging(WINDOWPOS *lpwndpos)
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
    if(abs(lpwndpos->x + lpwndpos->cx - rcScreen.right) <= WNDSNAP_ALLOWANCE)
    {
        lpwndpos->x = rcScreen.right - lpwndpos->cx;
    }

    // Snap Y axis to top
    if(abs(lpwndpos->y - rcScreen.top) <= WNDSNAP_ALLOWANCE)
    {
        // Assign new cordinate
        lpwndpos->y = rcScreen.top;
    }

    // Snap Y axis to bottom
    if(abs(lpwndpos->y + lpwndpos->cy - rcScreen.bottom) <= WNDSNAP_ALLOWANCE)
    {
        lpwndpos->y = rcScreen.bottom - lpwndpos->cy;
    }
}

void CQPasteWnd::OnShowGroupsTop()
{
    OnShowGroupsBottom();
    return ;
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

    m_bHandleSearchTextChange = false;
    m_Search.SetWindowText(_T(""));
    m_bHandleSearchTextChange = true;

    MoveControls();

    if(lID >=  - 1)
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

void CQPasteWnd::OnSearchEditChange()
{
    if(g_Opt.m_bFindAsYouType == FALSE)
    {
        return ;
    }

    if(m_bHandleSearchTextChange == false)
    {
        //Log(_T("Handle text change is NOT set"));
        return ;
    }

    KillTimer(TIMER_DO_SEARCH);
    SetTimer(TIMER_DO_SEARCH, 100, NULL);

    return ;
}

LRESULT CQPasteWnd::OnUpDown(WPARAM wParam, LPARAM lParam)
{
    m_lstHeader.HidePopup();

    if(m_lstHeader.HandleKeyDown(wParam, lParam) == FALSE)
    {
        m_lstHeader.SendMessage(WM_KEYDOWN, wParam, lParam);
    }

    return TRUE;
}

LRESULT CQPasteWnd::OnToolTipWndInactive(WPARAM wParam, LPARAM lParam)
{
    if(!g_Opt.m_bShowPersistent)
    {
        CWnd *p = GetFocus();
        if(p == NULL)
        {
            HideQPasteWindow();
        }
    }

    return TRUE;
}

void CQPasteWnd::OnUpdateMenuNewgroup(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
    if(cs.Find(_T("Ctrl-F7")) < 0)
    {
        cs += "\tCtrl-F7";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuNewgroupselection(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
    if(cs.Find(_T("F7")) < 0)
    {
        cs += "\tF7";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuAllwaysontop(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
    if(cs.Find(_T("Ctrl-Space")) < 0)
    {
        cs += "\tCtrl-Space";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuViewfulldescription(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
    if(cs.Find(_T("F3")) < 0)
    {
        cs += "\tF3";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuViewgroups(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
    if(cs.Find(_T("Ctrl-G")) < 0)
    {
        cs += "\tCtrl-G";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuPasteplaintextonly(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
    if(cs.Find(_T("Shift-Enter")) < 0)
    {
        cs += "\tShift-Enter";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuDelete(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);

    if(cs.Find(_T("Del")) < 0)
    {
        cs += "\tDel";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuProperties(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);

    if(cs.Find(_T("tAlt-Enter")) < 0)
    {
        cs += "\tAlt-Enter";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuEdititem(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);

    if(cs.Find(_T("tCtrl-E")) < 0)
    {
        cs += "\tCtrl-E";
        pCmdUI->SetText(cs);
    }
}

void CQPasteWnd::OnUpdateMenuNewclip(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

    CString cs;
    pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);

    if(cs.Find(_T("tCtrl-N")) < 0)
    {
        cs += "\tCtrl-N";
        pCmdUI->SetText(cs);
    }
}

LRESULT CQPasteWnd::OnSetListCount(WPARAM wParam, LPARAM lParam)
{
    m_lstHeader.SetItemCountEx(wParam);
    m_lstHeader.RefreshVisibleRows();

    MoveControls();
    UpdateStatus(false);
    RedrawWindow();

    return TRUE;
}

LRESULT CQPasteWnd::OnItemDeleted(WPARAM wParam, LPARAM lParam)
{
    m_lstHeader.OnItemDeleted(wParam);
    return TRUE;
}

LRESULT CQPasteWnd::OnRefeshRow(WPARAM wParam, LPARAM lParam)
{
    int clipId = wParam;
    int listPos = lParam;

    if(m_bFoundClipToSetFocusTo == false)
    {
        long lFocusIndex =  - 1;
        if(theApp.m_FocusID == clipId || clipId ==  - 1)
        {
            m_bFoundClipToSetFocusTo = true;
            theApp.m_FocusID =  - 1;
            m_lstHeader.SetListPos(listPos);
            UpdateStatus(false);
        }
    }

    int topIndex = m_lstHeader.GetTopIndex();
    int lastIndex = topIndex + m_lstHeader.GetCountPerPage();

    if(listPos >= topIndex && listPos <= lastIndex)
    {
        m_lstHeader.RefreshRow(listPos);
    }

    return true;
}

void CQPasteWnd::FillMainTable(CMainTable &table, CppSQLite3Query &q)
{
    table.m_lID = q.getIntField(_T("lID"));
    table.m_Desc = q.fieldValue(_T("mText"));
    table.m_bHasParent = q.getIntField(_T("lParentID")) >= 0;
    table.m_bDontAutoDelete = q.getIntField(_T("lDontAutoDelete")) > 0;
    table.m_bHasShortCut = q.getIntField(_T("lShortCut")) > 0;
    table.m_bIsGroup = q.getIntField(_T("bIsGroup")) > 0;
    table.m_QuickPaste = q.fieldValue(_T("QuickPasteText"));
}

void CQPasteWnd::OnDestroy()
{
    CWndEx::OnDestroy();
    m_thread.Stop();
}

void CQPasteWnd::OnTimer(UINT_PTR nIDEvent)
{
    if(nIDEvent == TIMER_DO_SEARCH)
    {
        KillTimer(TIMER_DO_SEARCH);

        CString csText;
        m_Search.GetWindowText(csText);

        int nCaretPos = m_lstHeader.GetCaret();
        if(nCaretPos >= 0 && theApp.m_FocusID < 0)
        {
            theApp.m_FocusID = m_lstHeader.GetItemData(nCaretPos);
        }

        FillList(csText);
    }
    if(nIDEvent == TIMER_PASTE_FROM_MODIFER)
    {
        Log(_T("TIMER_PASTE_FROM_MODIFER timer\n"));
        KillTimer(TIMER_PASTE_FROM_MODIFER);
        if(m_bModifersMoveActive)
        {
            Log(_T("Open Selection\n"));
            OpenSelection(false);
        }
        else
        {
            Log(_T("m_bModifersMoveActive set to false\n"));
        }
    }

    CWndEx::OnTimer(nIDEvent);
}

void CQPasteWnd::OnAddinSelect(UINT id)
{
    ARRAY IDs;
    m_lstHeader.GetSelectionItemData(IDs);

    if(IDs.GetCount() > 0)
    {
        long lID = IDs[0];
        CClip clip;
        if(clip.LoadMainTable(lID))
        {
            if(clip.LoadFormats(lID, false))
            {
                bool bCont = theApp.m_Addins.CallPrePasteFunction(id, &clip);
                if(bCont)
                {
                    OpenID( - 1, false, &clip.m_Formats);
                }
            }
        }
    }
}

LRESULT CQPasteWnd::OnSelectAll(WPARAM wParam, LPARAM lParam)
{
    BOOL ret = FALSE;
    ATL::CCritSecLock csLock(m_CritSection.m_sect);

    if((int)m_mapCache.size() < m_lstHeader.GetItemCount())
    {
        Log(_T("All items selected loading all items from the db"));

        CPoint loadItem(0, m_lstHeader.GetItemCount());
        m_loadItems.push_back(loadItem);

        m_thread.FireLoadItems();

        ret = TRUE;

        UpdateStatus(false);
    }

    return ret;
}
