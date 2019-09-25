// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "ActionEnums.h"
#include <algorithm>
#include "QRCodeViewer.h"
#include "CreateQRCodeImage.h"
#include "ClipCompare.h"
#include "Misc.h"
#include "FriendPromptDlg.h"
#include "DimWnd.h"
#include "client.h"
#include "SendMail.h"
#include "CF_UnicodeTextAggregator.h"
#include "CF_TextAggregator.h"
#include "htmlformataggregator.h"
#include "shared/Tokenizer.h"

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
#define ID_SEARCH_DESCRIPTION_BUTTON 0x208
#define ON_TOP_WARNING 0x209
#define ID_SYSTEM_BUTTON		0x210
#define ID_NO_SEARCH_RESULTS	0x211


#define QPASTE_WIDTH			200
#define QPASTE_HEIGHT			200

#define TIMER_FILL_CACHE		1
#define TIMER_DO_SEARCH			2
#define TIMER_PASTE_FROM_MODIFER	3
#define TIMER_ERROR_MSG			4
#define TIMER_DRAG_HIDE_WINDOW	6
#define TIMER_DO_ACTION	7

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
	m_strSearch = "";
    m_bHandleSearchTextChange = true;
    m_bModifersMoveActive = false;
	m_showScrollBars = false;
	m_leftSelectedCompareId = 0;
	m_extraDataCounter = 0;
	m_noSearchResults = false;
}

CQPasteWnd::~CQPasteWnd()
{
}

BEGIN_MESSAGE_MAP(CQPasteWnd, CWndEx)
//{{AFX_MSG_MAP(CQPasteWnd)
ON_WM_ERASEBKGND()
ON_WM_CREATE()
ON_WM_SIZE()
ON_WM_SETFOCUS()
ON_WM_KILLFOCUS()
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
ON_COMMAND(ID_MENU_DELETE, OnMenuDelete)
ON_COMMAND(ID_MENU_POSITIONING_ATCARET, OnMenuPositioningAtcaret)
ON_COMMAND(ID_MENU_POSITIONING_ATCURSOR, OnMenuPositioningAtcursor)
ON_COMMAND(ID_MENU_POSITIONING_ATPREVIOUSPOSITION, OnMenuPositioningAtpreviousposition)
ON_COMMAND(ID_MENU_OPTIONS, OnMenuOptions)
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
ON_COMMAND(ID_STICKYCLIPS_MAKETOPSTICKYCLIP, OnMakeTopStickyClip)
ON_COMMAND(ID_STICKYCLIPS_MAKELASTSTICKYCLIP, OnMakeLastStickyClip)
ON_COMMAND(ID_STICKYCLIPS_REMOVESTICKYSETTING, OnRemoveSticky)
ON_COMMAND(ID_QUICKOPTIONS_ELEVATEPREVILEGESTOPASTEINTOELEVATEDAPPS, OnElevateAppToPasteIntoElevatedApp)

ON_WM_DESTROY()

//}}AFX_MSG_MAP
ON_MESSAGE(NM_SEARCH_ENTER_PRESSED, OnSearchEnterKeyPressed)
ON_MESSAGE(NM_COPY_CLIP, OnCopyClip)
ON_MESSAGE(NM_END, OnListEnd)
ON_MESSAGE(CB_SEARCH, OnSearch)
ON_MESSAGE(NM_DELETE, OnDelete)
ON_NOTIFY(NM_GETTOOLTIPTEXT, ID_LIST_HEADER, OnGetToolTipText)
ON_MESSAGE(NM_SELECT_DB_ID, OnListSelect_DB_ID)
ON_MESSAGE(WM_REFRESH_VIEW, OnRefreshView)
ON_MESSAGE(WM_RELOAD_CLIP_AFTER_PASTE, OnReloadClipAfterPaste)
ON_WM_NCLBUTTONDBLCLK()
ON_WM_WINDOWPOSCHANGING()
ON_COMMAND(ID_VIEWCAPTIONBARON_RIGHT, OnViewcaptionbaronRight)
ON_COMMAND(ID_VIEWCAPTIONBARON_BOTTOM, OnViewcaptionbaronBottom)
ON_COMMAND(ID_VIEWCAPTIONBARON_LEFT, OnViewcaptionbaronLeft)
ON_COMMAND(ID_VIEWCAPTIONBARON_TOP, OnViewcaptionbaronTop)
ON_COMMAND(ID_MENU_AUTOHIDE, OnMenuAutohide)
ON_COMMAND(ID_MENU_VIEWFULLDESCRIPTION, OnMenuViewfulldescription)
ON_COMMAND(ID_MENU_ALLWAYSONTOP, OnMenuAllwaysontop)
ON_COMMAND(ID_MENU_NEWGROUP, OnMenuNewGroup)
ON_COMMAND(ID_MENU_NEWGROUPSELECTION, OnMenuNewGroupSelection)
ON_MESSAGE(NM_GROUP_TREE_MESSAGE, OnGroupTreeMessage)
ON_COMMAND(ID_BACK_BUTTON, OnBackButton)
ON_COMMAND(ID_SYSTEM_BUTTON, OnSystemButton)
ON_MESSAGE(CB_UPDOWN, OnUpDown)
ON_MESSAGE(NM_INACTIVE_TOOLTIPWND, OnToolTipWndInactive)
ON_MESSAGE(NM_SET_LIST_COUNT, OnSetListCount)
ON_MESSAGE(NM_REFRESH_ROW, OnRefeshRow)
ON_MESSAGE(NM_ITEM_DELETED, OnItemDeleted)
ON_WM_TIMER()
ON_COMMAND(ID_MENU_EXPORT, OnMenuExport)
ON_COMMAND(ID_MENU_IMPORT, OnMenuImport)
ON_COMMAND(ID_QUICKPROPERTIES_REMOVEQUICKPASTE, OnQuickpropertiesRemovequickpaste)
ON_COMMAND(ID_MENU_EDITITEM, OnMenuEdititem)
ON_COMMAND(ID_MENU_NEWCLIP, OnMenuNewclip)
ON_UPDATE_COMMAND_UI(ID_MENU_EDITITEM, OnUpdateMenuEdititem)
ON_UPDATE_COMMAND_UI(ID_MENU_NEWCLIP, OnUpdateMenuNewclip)
ON_WM_CTLCOLOR_REFLECT()
ON_COMMAND_RANGE(3000, 4000, OnAddinSelect)
ON_MESSAGE(NM_ALL_SELECTED, OnSelectAll)
ON_MESSAGE(NM_SHOW_HIDE_SCROLLBARS, OnShowHideScrollBar)
ON_MESSAGE(NM_CANCEL_SEARCH, OnCancelFilter)
ON_MESSAGE(NM_POST_OPTIONS_WINDOW, OnPostOptions)
ON_COMMAND(ID_MENU_SEARCHDESCRIPTION, OnMenuSearchDescription)
ON_COMMAND(ID_MENU_SEARCHFULLTEXT, OnMenuSearchFullText)
ON_COMMAND(ID_MENU_SEARCHQUICKPASTE, OnMenuSearchQuickPaste)
ON_COMMAND(ID_MENU_CONTAINSTEXTSEARCHONLY, OnMenuSimpleTextSearch)
//ON_WM_CTLCOLOR()
//ON_WM_ERASEBKGND()
//ON_WM_PAINT()
ON_COMMAND(ID_QUICKOPTIONS_SHOWINTASKBAR, &CQPasteWnd::OnQuickoptionsShowintaskbar)
ON_COMMAND(ID_MENU_VIEWASQRCODE, &CQPasteWnd::OnMenuViewasqrcode)
ON_COMMAND(ID_EXPORT_EXPORTTOTEXTFILE, &CQPasteWnd::OnExportExporttotextfile)
ON_COMMAND(ID_COMPARE_COMPARE, &CQPasteWnd::OnCompareCompare)
ON_COMMAND(ID_COMPARE_SELECTLEFTCOMPARE, &CQPasteWnd::OnCompareSelectleftcompare)
ON_COMMAND(ID_COMPARE_COMPAREAGAINST, &CQPasteWnd::OnCompareCompareagainst)
ON_UPDATE_COMMAND_UI(ID_COMPARE_COMPARE, &CQPasteWnd::OnUpdateCompareCompare)
ON_MESSAGE(NM_SHOW_PROPERTIES, OnShowProperties)
ON_MESSAGE(NM_NEW_GROUP, OnNewGroup)
ON_MESSAGE(NM_DELETE_ID, OnDeleteId)
ON_COMMAND(ID_MENU_REGULAREXPRESSIONSEARCH, &CQPasteWnd::OnMenuRegularexpressionsearch)

ON_COMMAND(ID_IMPORT_EXPORTTOGOOGLETRANSLATE, &CQPasteWnd::OnImportExporttogoogletranslate)
ON_UPDATE_COMMAND_UI(ID_IMPORT_EXPORTTOGOOGLETRANSLATE, &CQPasteWnd::OnUpdateImportExporttogoogletranslate)
ON_COMMAND(ID_IMPORT_EXPORTCLIP_BITMAP, &CQPasteWnd::OnImportExportclipBitmap)
ON_UPDATE_COMMAND_UI(ID_IMPORT_EXPORTCLIP_BITMAP, &CQPasteWnd::OnUpdateImportExportclipBitmap)

ON_COMMAND(ID_MENU_WILDCARDSEARCH, &CQPasteWnd::OnMenuWildcardsearch)

ON_COMMAND(ID_MENU_SAVECURRENTCLIPBOARD, &CQPasteWnd::OnMenuSavecurrentclipboard)
ON_UPDATE_COMMAND_UI(ID_MENU_SAVECURRENTCLIPBOARD, &CQPasteWnd::OnUpdateMenuSavecurrentclipboard)
ON_MESSAGE(NM_MOVE_TO_GROUP, OnListMoveSelectionToGroup)
ON_COMMAND(ID_CLIPORDER_MOVEUP, &CQPasteWnd::OnCliporderMoveup)
ON_UPDATE_COMMAND_UI(ID_CLIPORDER_MOVEUP, &CQPasteWnd::OnUpdateCliporderMoveup)
ON_COMMAND(ID_CLIPORDER_MOVEDOWN, &CQPasteWnd::OnCliporderMovedown)
ON_UPDATE_COMMAND_UI(ID_CLIPORDER_MOVEDOWN, &CQPasteWnd::OnUpdateCliporderMovedown)
ON_COMMAND(ID_CLIPORDER_MOVETOTOP, &CQPasteWnd::OnCliporderMovetotop)
ON_UPDATE_COMMAND_UI(ID_CLIPORDER_MOVETOTOP, &CQPasteWnd::OnUpdateCliporderMovetotop)
ON_COMMAND(ID_MENU_FILTERON, &CQPasteWnd::OnMenuFilteron)
ON_UPDATE_COMMAND_UI(ID_MENU_FILTERON, &CQPasteWnd::OnUpdateMenuFilteron)
ON_BN_CLICKED(ON_TOP_WARNING, OnAlwaysOnTopClicked)
//ON_WM_CTLCOLOR()
ON_COMMAND(ID_SPECIALPASTE_UPPERCASE, &CQPasteWnd::OnSpecialpasteUppercase)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_UPPERCASE, &CQPasteWnd::OnUpdateSpecialpasteUppercase)
ON_COMMAND(ID_SPECIALPASTE_LOWERCASE, &CQPasteWnd::OnSpecialpasteLowercase)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_LOWERCASE, &CQPasteWnd::OnUpdateSpecialpasteLowercase)
ON_COMMAND(ID_SPECIALPASTE_CAPITALIZE, &CQPasteWnd::OnSpecialpasteCapitalize)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_CAPITALIZE, &CQPasteWnd::OnUpdateSpecialpasteCapitalize)
ON_COMMAND(ID_SPECIALPASTE_SENTENCE, &CQPasteWnd::OnSpecialpasteSentence)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_SENTENCE, &CQPasteWnd::OnUpdateSpecialpasteSentence)
ON_COMMAND(ID_SPECIALPASTE_REMOVELINEFEEDS, &CQPasteWnd::OnSpecialpasteRemovelinefeeds)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_REMOVELINEFEEDS, &CQPasteWnd::OnUpdateSpecialpasteRemovelinefeeds)
ON_COMMAND(ID_SPECIALPASTE_PASTE, &CQPasteWnd::OnSpecialpastePaste)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_PASTE, &CQPasteWnd::OnUpdateSpecialpastePaste)
ON_COMMAND(ID_SPECIALPASTE_PASTE32919, &CQPasteWnd::OnSpecialpastePaste32919)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_PASTE32919, &CQPasteWnd::OnUpdateSpecialpastePaste32919)
ON_COMMAND(ID_SPECIALPASTE_TYPOGLYCEMIA, &CQPasteWnd::OnSpecialpasteTypoglycemia)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_TYPOGLYCEMIA, &CQPasteWnd::OnUpdateSpecialpasteTypoglycemia)
ON_NOTIFY(NM_CLICK, ID_LIST_HEADER, &CQPasteWnd::OnNMClickList1)
ON_NOTIFY(NM_DBLCLK, ID_LIST_HEADER, &CQPasteWnd::OnNMDblclkList1)
ON_NOTIFY(NM_RCLICK, ID_LIST_HEADER, &CQPasteWnd::OnNMRClickList1)
ON_NOTIFY(NM_RDBLCLK, ID_LIST_HEADER, &CQPasteWnd::OnNMRDblclkList1)
ON_COMMAND(ID_QUICKOPTIONS_SHOWTEXTFORFIRSTTENCOPYHOTKEYS, &CQPasteWnd::OnQuickoptionsShowtextforfirsttencopyhotkeys)
ON_UPDATE_COMMAND_UI(ID_QUICKOPTIONS_SHOWTEXTFORFIRSTTENCOPYHOTKEYS, &CQPasteWnd::OnUpdateQuickoptionsShowtextforfirsttencopyhotkeys)
ON_COMMAND(ID_QUICKOPTIONS_SHOWINDICATORACLIPHASBEENPASTED, &CQPasteWnd::OnQuickoptionsShowindicatoracliphasbeenpasted)
ON_UPDATE_COMMAND_UI(ID_QUICKOPTIONS_SHOWINDICATORACLIPHASBEENPASTED, &CQPasteWnd::OnUpdateQuickoptionsShowindicatoracliphasbeenpasted)
ON_COMMAND(ID_GROUPS_TOGGLELASTGROUP, &CQPasteWnd::OnGroupsTogglelastgroup)
ON_UPDATE_COMMAND_UI(ID_GROUPS_TOGGLELASTGROUP, &CQPasteWnd::OnUpdateGroupsTogglelastgroup)
ON_UPDATE_COMMAND_UI(ID_STICKYCLIPS_MAKETOPSTICKYCLIP, &CQPasteWnd::OnUpdateStickyclipsMaketopstickyclip)
ON_UPDATE_COMMAND_UI(ID_STICKYCLIPS_MAKELASTSTICKYCLIP, &CQPasteWnd::OnUpdateStickyclipsMakelaststickyclip)
ON_UPDATE_COMMAND_UI(ID_STICKYCLIPS_REMOVESTICKYSETTING, &CQPasteWnd::OnUpdateStickyclipsRemovestickysetting)
ON_COMMAND(ID_SPECIALPASTE_PASTE32927, &CQPasteWnd::OnSpecialpastePaste32927)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_PASTE32927, &CQPasteWnd::OnUpdateSpecialpastePaste32927)
ON_COMMAND(ID_MENU_GLOBALHOTKEYS32933, &CQPasteWnd::OnMenuGlobalhotkeys32933)
ON_COMMAND(ID_MENU_DELETECLIPDATA32934, &CQPasteWnd::OnMenuDeleteclipdata32934)
ON_COMMAND(ID_MENU_IMPORTCLIP32935, &CQPasteWnd::OnMenuImportclip32935)
ON_COMMAND(ID_MENU_NEWCLIP32937, &CQPasteWnd::OnMenuNewclip32937)
ON_UPDATE_COMMAND_UI(ID_MENU_IMPORTCLIP32935, &CQPasteWnd::OnUpdateMenuImportclip32935)
ON_UPDATE_COMMAND_UI(ID_MENU_NEWCLIP32937, &CQPasteWnd::OnUpdateMenuNewclip32937)
ON_UPDATE_COMMAND_UI(ID_MENU_GLOBALHOTKEYS32933, &CQPasteWnd::OnUpdateMenuGlobalhotkeys32933)
ON_UPDATE_COMMAND_UI(ID_MENU_DELETECLIPDATA32934, &CQPasteWnd::OnUpdateMenuDeleteclipdata32934)
ON_MESSAGE(NM_FOCUS_ON_SEARCH, OnSearchFocused)
ON_COMMAND(ID_CLIPORDER_REPLACETOPSTICKYCLIP, &CQPasteWnd::OnCliporderReplacetopstickyclip)
ON_UPDATE_COMMAND_UI(ID_CLIPORDER_REPLACETOPSTICKYCLIP, &CQPasteWnd::OnUpdateCliporderReplacetopstickyclip)
ON_COMMAND(ID_SENDTO_PROMPTFORNAME, &CQPasteWnd::OnSendtoPromptforname)
ON_UPDATE_COMMAND_UI(ID_SENDTO_PROMPTFORNAME, &CQPasteWnd::OnUpdateSendtoPromptforname)
ON_COMMAND(ID_IMPORT_IMPORTCOPIEDFILE, &CQPasteWnd::OnImportImportcopiedfile)
ON_UPDATE_COMMAND_UI(ID_IMPORT_IMPORTCOPIEDFILE, &CQPasteWnd::OnUpdateImportImportcopiedfile)
ON_UPDATE_COMMAND_UI(32775, &CQPasteWnd::OnUpdate32775)
ON_COMMAND_RANGE(CustomFriendStartId, (CustomFriendStartId+ MaxCustomFriends+1), OnCustomSendToFriend)
ON_COMMAND_RANGE(ChaiScriptMenuStartId, (ChaiScriptMenuStartId + MaxChaiScripts + 1), OnChaiScriptPaste)
ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
ON_COMMAND(ID_CLIPORDER_MOVETOLAST, &CQPasteWnd::OnCliporderMovetolast)
ON_UPDATE_COMMAND_UI(ID_CLIPORDER_MOVETOLAST, &CQPasteWnd::OnUpdateCliporderMovetolast)
ON_COMMAND(ID_SPECIALPASTE_PASTE32945, &CQPasteWnd::OnSpecialpastePasteDontUpdateOrder)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_PASTE32945, &CQPasteWnd::OnUpdateOnSpecialPasteDontUpdateOrder)
ON_COMMAND(ID_SPECIALPASTE_TRIM, &CQPasteWnd::OnSpecialpasteTrim)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_TRIM, &CQPasteWnd::OnUpdateSpecialpasteTrim)
ON_COMMAND(ID_TRANSPARENCY_INCREASE, &CQPasteWnd::OnTransparencyIncrease)
ON_UPDATE_COMMAND_UI(ID_TRANSPARENCY_INCREASE, &CQPasteWnd::OnUpdateTransparencyIncrease)
ON_COMMAND(ID_TRANSPARENCY_DECREASE, &CQPasteWnd::OnTransparencyDecrease)
ON_UPDATE_COMMAND_UI(ID_TRANSPARENCY_DECREASE, &CQPasteWnd::OnUpdateTransparencyDecrease)
ON_COMMAND(ID_TRANSPARENCY_TOGGLE, &CQPasteWnd::OnTransparencyToggle)
ON_UPDATE_COMMAND_UI(ID_TRANSPARENCY_TOGGLE, &CQPasteWnd::OnUpdateTransparencyToggle)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_NONE, &CQPasteWnd::OnUpdateTransparencyNone)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_5, &CQPasteWnd::OnUpdateTransparency5)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_10, &CQPasteWnd::OnUpdateTransparency10)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_15, &CQPasteWnd::OnUpdateTransparency15)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_20, &CQPasteWnd::OnUpdateTransparency20)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_25, &CQPasteWnd::OnUpdateTransparency25)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_30, &CQPasteWnd::OnUpdateTransparency30)
ON_UPDATE_COMMAND_UI(ID_TRANSPARENCY_35, &CQPasteWnd::OnUpdateTransparency35)
ON_UPDATE_COMMAND_UI(ID_MENU_TRANSPARENCY_40, &CQPasteWnd::OnUpdateTransparency40)
ON_COMMAND(ID_TRANSPARENCY_35, &CQPasteWnd::OnTransparency35)
ON_COMMAND(ID_IMPORT_EMAILTO, &CQPasteWnd::OnImportEmailto)
ON_UPDATE_COMMAND_UI(ID_IMPORT_EMAILTO, &CQPasteWnd::OnUpdateImportEmailto)
ON_COMMAND(ID_IMPORT_GMAIL, &CQPasteWnd::OnImportGmail)
ON_UPDATE_COMMAND_UI(ID_IMPORT_GMAIL, &CQPasteWnd::OnUpdateImportGmail)
ON_COMMAND(ID_IMPORT_EMAILTOASATTACHMENT, &CQPasteWnd::OnImportEmailtoasattachment)
ON_UPDATE_COMMAND_UI(ID_IMPORT_EMAILTOASATTACHMENT, &CQPasteWnd::OnUpdateImportEmailtoasattachment)
ON_COMMAND(ID_SPECIALPASTE_SLUGIFY, &CQPasteWnd::OnSpecialpasteSlugify)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_SLUGIFY, &CQPasteWnd::OnUpdateSpecialpasteSlugify)
ON_COMMAND(ID_IMPORT_EMAIL_CONTENT_ATTACH, &CQPasteWnd::OnImportEmailContentAttach)
ON_UPDATE_COMMAND_UI(ID_IMPORT_EMAIL_CONTENT_ATTACH, &CQPasteWnd::OnUpdateImportEmailContentAttach)
ON_COMMAND(ID_SPECIALPASTE_TOGGLECASE, &CQPasteWnd::OnSpecialpasteTogglecase)
ON_UPDATE_COMMAND_UI(ID_SPECIALPASTE_TOGGLECASE, &CQPasteWnd::OnUpdateSpecialpasteTogglecase)
ON_COMMAND(ID_FIRST_SHOWSTARTUPMESSAGE, &CQPasteWnd::OnFirstShowstartupmessage)

ON_COMMAND(ID_MENU_RESTOREDATABSAE, &CQPasteWnd::OnFirstRestoreDb)
ON_COMMAND(ID_MENU_BACKUPDATABASE, &CQPasteWnd::OnFirstBackupDb)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd message handlers

HBRUSH CQPasteWnd::CtlColor(CDC *pDC, UINT nCtlColor)
{
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetBkColor(RGB(255, 0, 0));

    return (HBRUSH)GetStockObject(NULL_BRUSH);
}

BOOL CQPasteWnd::Create(CRect rect, CWnd *pParentWnd)
{
    return CWndEx::Create(rect, pParentWnd);
}

int CQPasteWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if(CWndEx::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

	HICON b = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 64, 64, LR_SHARED);	
	SetIcon(b, TRUE);

	//BOOL b = this->Register(this);

    SetWindowText(_T(QPASTE_TITLE));

    m_search.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, ID_EDIT_SEARCH);
	m_search.SetDpiInfo(&m_DittoWindow.m_dpi);
	m_search.SetPromptText(theApp.m_Language.GetString(_T("Search"), _T("Search")));
	SetSearchImages();
	m_search.LoadPastSearches(CGetSetOptions::GetPastSearchXml());

	CRect rcEditArea(m_DittoWindow.m_dpi.Scale(4), m_DittoWindow.m_dpi.Scale(2), m_DittoWindow.m_dpi.Scale(20), m_DittoWindow.m_dpi.Scale(2));
	//m_search.SetBorder(rcEditArea);

	CRect rcCloseArea(m_DittoWindow.m_dpi.Scale(85), m_DittoWindow.m_dpi.Scale(3), m_DittoWindow.m_dpi.Scale(99), m_DittoWindow.m_dpi.Scale(15));
	//m_search.SetButtonArea(rcCloseArea);

    // Create the header control
    if(!m_lstHeader.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_OWNERDRAWFIXED, CRect(0, 0, 0, 0), this, ID_LIST_HEADER))
    {
        ASSERT(FALSE);
        return -1;
    }
	m_lstHeader.SetDpiInfo(&m_DittoWindow.m_dpi);
	m_lstHeader.ShowWindow(SW_SHOW);

    ((CWnd*) &m_GroupTree)->CreateEx(NULL, _T("SysTreeView32"), NULL, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS, CRect(0, 0, 100, 100), this, 0);
	m_GroupTree.ModifyStyle(WS_CAPTION | WS_TABSTOP, 0);

    m_GroupTree.SetNotificationWndEx(m_hWnd);
    m_GroupTree.ShowWindow(SW_HIDE);
	m_GroupTree.m_showRightClickMenu = true;

    m_ShowGroupsFolderBottom.Create(NULL, WS_CHILD | BS_OWNERDRAW | WS_TABSTOP, CRect(0, 0, 0, 0), this, ID_SHOW_GROUPS_BOTTOM);
    //m_ShowGroupsFolderBottom.LoadBitmaps(IDB_CLOSED_FOLDER, IDB_CLOSED_FOLDER_PRESSED, IDB_CLOSED_FOLDER_FOCUSED);
	m_ShowGroupsFolderBottom.LoadStdImageDPI(m_DittoWindow.m_dpi.GetDPI(), open_folder_24, open_folder_30, open_folder_36, open_folder_42, open_folder_48, _T("PNG"), open_folder_54, open_folder_60, open_folder_66, open_folder_72, open_folder_78, open_folder_84);
    m_ShowGroupsFolderBottom.ShowWindow(SW_SHOW);
	m_ShowGroupsFolderBottom.SetToolTipText(theApp.m_Language.GetString(_T("GroupsTooltip"), _T("Groups")));
	m_ShowGroupsFolderBottom.ModifyStyle(WS_TABSTOP, 0);
	
    m_BackButton.Create(NULL, WS_CHILD | BS_OWNERDRAW | WS_TABSTOP, CRect(0, 0, 0, 0), this, ID_BACK_BUTTON);
	m_BackButton.LoadStdImageDPI(m_DittoWindow.m_dpi.GetDPI(), return_16, return_20, return_24, return_28, return_32, _T("PNG"));
	m_BackButton.ModifyStyle(WS_TABSTOP, 0);
    m_BackButton.ShowWindow(SW_SHOW);

	m_systemMenu.Create(NULL, WS_CHILD | BS_OWNERDRAW | WS_TABSTOP, CRect(0, 0, 0, 0), this, ID_SYSTEM_BUTTON);
	m_systemMenu.LoadStdImageDPI(m_DittoWindow.m_dpi.GetDPI(), system_menu_2_24, system_menu_2_30, system_menu_2_36, system_menu_2_42, system_menu_2_48, _T("PNG"), system_menu_54, system_menu_60, system_menu_66, system_menu_72, system_menu_78, system_menu_84);
	m_systemMenu.ModifyStyle(WS_TABSTOP, 0);
	m_systemMenu.ShowWindow(SW_SHOW);
		
    m_stGroup.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, ID_GROUP_TEXT);
	
    //Set the z-order
    m_lstHeader.SetWindowPos(this, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    m_search.SetWindowPos(&m_lstHeader, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    m_ShowGroupsFolderBottom.SetWindowPos(&m_search, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    //LVS_EX_FLATSB
    m_lstHeader.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

    // Create the columns
    if(m_lstHeader.InsertColumn(0, _T(""), LVCFMT_LEFT, 2500, 0) != 0)
    {
        ASSERT(FALSE);
        return -1;
    }

    m_Alpha.SetWindowHandle(m_hWnd);	
	    
	CString onTopMsg = theApp.m_Language.GetString(_T("TurnOfAlwaysOntop"), _T("Always on Top Enabled"));
	CString shortcutText = m_actions.GetCmdKeyText(ActionEnums::TOGGLESHOWPERSISTANT);
	if (shortcutText != _T("") &&
		shortcutText.Find("\t" + shortcutText) < 0)
	{
		onTopMsg += "\t(";
		onTopMsg += shortcutText;
		onTopMsg += ")";
	}

	m_alwaysOnToWarningStatic.Create(onTopMsg, WS_CHILD | SS_CENTERIMAGE | SS_NOTIFY, CRect(0, 0, 0, 0), this, ON_TOP_WARNING);
	m_alwaysOnToWarningStatic.SetBkColor(COLORREF(RGB(255, 255, 0)));
	m_alwaysOnToWarningStatic.SetTextColor(COLORREF(RGB(0, 0, 255)));
	m_alwaysOnToWarningStatic.SetToggleCursor(true);
	m_alwaysOnToWarningStatic.SetFont(&m_groupFont);

	m_noSearchResultsStatic.Create(onTopMsg, WS_CHILD, CRect(0, 0, 0, 0), this, ID_NO_SEARCH_RESULTS);

	m_popupMsg.m_hWndPosRelativeTo = m_hWnd;

	UpdateFont();

	m_thread.Start(this);
	m_extraDataThread.Start(this);

	LoadShortcuts();

	InvalidateNc();
	
    return 0;
}

void CQPasteWnd::LoadShortcuts()
{
	m_modifierKeyActions.RemoveAll();

	m_modifierKeyActions.m_checkModifierKeys = false;
	m_modifierKeyActions.AddAccel(ActionEnums::MODIFIER_ACTVE_SELECTIONUP, VK_UP);
	m_modifierKeyActions.AddAccel(ActionEnums::MODIFIER_ACTVE_SELECTIONDOWN, VK_DOWN);
	m_modifierKeyActions.AddAccel(ActionEnums::MODIFIER_ACTVE_MOVEFIRST, VK_HOME);
	m_modifierKeyActions.AddAccel(ActionEnums::MODIFIER_ACTVE_MOVELAST, VK_END);

	m_actions.RemoveAll();
	m_toolTipActions.RemoveAll();

	m_actions.AddAccel(ActionEnums::NEXTTABCONTROL, VK_TAB);
	m_actions.AddAccel(ActionEnums::PREVTABCONTROL, ACCEL_MAKEKEY(VK_TAB, HOTKEYF_CONTROL));
	m_actions.AddAccel(ActionEnums::BACKGRROUP, VK_BACK);
	m_actions.AddAccel(ActionEnums::DELETE_SELECTED, VK_DELETE);
	
	m_actions.AddAccel(ActionEnums::HOMELIST, VK_HOME);
	m_actions.AddAccel(ActionEnums::SHOWMENU, VK_APPS);

	m_search.SetLastSearchAccel(CAccel(0, ActionEnums::APPLY_LAST_SEARCH, 0));

	for (DWORD i = ActionEnums::FIRST_ACTION + 1; i < ActionEnums::LAST_ACTION; i++)
	{
		ActionEnums::ActionEnumValues action = (ActionEnums::ActionEnumValues) i;

		if (ActionEnums::UserConfigurable(action))
		{
			for (int i = 0; i < 10; i++)
			{
				int a = g_Opt.GetActionShortCutA(action, i);
				if (a > 0)
				{
					int b = g_Opt.GetActionShortCutB(action, i);
					m_actions.AddAccel(action, a, b);

					//always add a shift variation to show description F3 so it will search backwards in the text search
					if (action == ActionEnums::SHOWDESCRIPTION)
					{
						int shift = HOTKEYF_SHIFT;
						if ((HIBYTE(a) & HOTKEYF_SHIFT))
						{
							shift = 0;
						}

						m_actions.AddAccel(action, ACCEL_MAKEKEY(LOBYTE(a), shift), b);
					}
					else if (action == ActionEnums::APPLY_LAST_SEARCH)
					{
						m_search.SetLastSearchAccel(CAccel(a, action, b));
					}

					if (ActionEnums::ToolTipAction(action))
					{
						m_toolTipActions.AddAccel(action, a, b);

						if (action == ActionEnums::SHOWDESCRIPTION)
						{
							int shift = HOTKEYF_SHIFT;
							if ((HIBYTE(a) & HOTKEYF_SHIFT))
							{
								shift = 0;
							}

							m_toolTipActions.AddAccel(action, ACCEL_MAKEKEY(LOBYTE(a), shift), b);
						}
					}
				}
			}
		}
	}

	for (auto & element : g_Opt.m_pasteScripts.m_list)
	{
		for (int i = 0; i < 10; i++)
		{
			int a = g_Opt.GetActionShortCutA(ActionEnums::PASTE_SCRIPT, i, element.m_guid);
			if (a > 0)
			{
				int b = g_Opt.GetActionShortCutB(ActionEnums::PASTE_SCRIPT, i, element.m_guid);
				m_actions.AddAccel(ActionEnums::PASTE_SCRIPT, a, b, element.m_guid);
			}
		}
	}

	m_actions.AddAccel(ActionEnums::TOGGLEFILELOGGING, ACCEL_MAKEKEY('D', HOTKEYF_CONTROL), ACCEL_MAKEKEY('F', HOTKEYF_CONTROL));
	m_actions.AddAccel(ActionEnums::TOGGLEOUTPUTDEBUGSTRING, ACCEL_MAKEKEY('D', HOTKEYF_CONTROL), ACCEL_MAKEKEY('O', HOTKEYF_CONTROL));

	m_lstHeader.SetTooltipActions(&m_toolTipActions);
}

void CQPasteWnd::SetSearchImages()
{
	//int iSourceImageDPIToUse = 96; // We will assume 96 by default.

	//if (m_DittoWindow.m_dpi.GetDPI() > 144) 
	//	iSourceImageDPIToUse = 192;
	//else if (m_DittoWindow.m_dpi.GetDPI() > 120) 
	//	iSourceImageDPIToUse = 144;
	//else if (m_DittoWindow.m_dpi.GetDPI() > 96) 
	//	iSourceImageDPIToUse = 120;

	//// Now select the right resource to load.
	//switch(iSourceImageDPIToUse)
	//{
	//case 120: 
	//	m_search.SetBitmaps(IDB_BITMAP_SEARCH_NORMAL_125, IDB_BITMAP_SEARCH_CLOSE_125);
	//	break;
	//case 144: 
	//	m_search.SetBitmaps(IDB_BITMAP_SEARCH_NORMAL_150, IDB_BITMAP_SEARCH_CLOSE_150);
	//	break;
	//case 192: 
	//	m_search.SetBitmaps(IDB_BITMAP_SEARCH_NORMAL_200, IDB_BITMAP_SEARCH_CLOSE_200);
	//	break;						
	//default: // default to 96 DPI
	//	m_search.SetBitmaps(IDB_BITMAP_SEARCH_NORMAL, IDB_BITMAP_SEARCH_CLOSE);
	//	break;
	//}
}

void CQPasteWnd::OnSize(UINT nType, int cx, int cy)
{
    CWndEx::OnSize(nType, cx, cy);

    if(!IsWindow(m_lstHeader.m_hWnd))
    {
        return ;
    }

	m_popupMsg.Hide();

    MoveControls();
}

void CQPasteWnd::MoveControls()
{
    CRect crRect;
    GetClientRect(crRect);
    int cx = crRect.Width();
    int cy = crRect.Height();

    //Hide the two pixels of space at the top, not sure where this is coming from
	int topOfListBox = 0;
	
    if(theApp.m_GroupID > 0)
    {
        m_stGroup.ShowWindow(SW_SHOW);
        m_BackButton.ShowWindow(SW_SHOW);

		m_BackButton.MoveWindow(m_DittoWindow.m_dpi.Scale(2), m_DittoWindow.m_dpi.Scale(2), m_DittoWindow.m_dpi.Scale(16), m_DittoWindow.m_dpi.Scale(16));
		m_stGroup.MoveWindow(m_DittoWindow.m_dpi.Scale(24), m_DittoWindow.m_dpi.Scale(2), cx - m_DittoWindow.m_dpi.Scale(20), m_DittoWindow.m_dpi.Scale(16));

		topOfListBox = m_DittoWindow.m_dpi.Scale(20);
	}
	else
	{
		m_BackButton.ShowWindow(SW_HIDE);
		m_stGroup.ShowWindow(SW_HIDE);
	}
	
	int searchRowStart = 33;

	/*if(g_Opt.m_bShowPersistent)
	{
		searchRowStart = 41;
	}*/

    int nWidth = cx;
	int listBoxBottomOffset = m_DittoWindow.m_dpi.Scale(searchRowStart);
	
	int extraSize = 0;

	if(m_showScrollBars == false &&
		g_Opt.m_showScrollBar == false)
	{
		extraSize = m_DittoWindow.m_dpi.Scale(::GetSystemMetrics(SM_CXVSCROLL));

		CRgn rgnRect;
		CRect r;
		m_lstHeader.GetWindowRect(&r) ;

		rgnRect.CreateRectRgn(0, 0, cx, (cy - listBoxBottomOffset-topOfListBox)+1);

		m_lstHeader.SetWindowRgn(rgnRect, TRUE);
	}


	if(m_noSearchResults &&
		m_strSearch != _T(""))
	{
		m_lstHeader.ShowWindow(SW_HIDE);
		m_noSearchResultsStatic.ShowWindow(SW_SHOW);

		auto border = m_DittoWindow.m_dpi.Scale(10);
		m_noSearchResultsStatic.MoveWindow(border, topOfListBox + border, cx - border, cy - listBoxBottomOffset - topOfListBox + 1 - border);
	}
	else
	{
		m_lstHeader.ShowWindow(SW_SHOW);
		m_noSearchResultsStatic.ShowWindow(SW_HIDE);

		m_lstHeader.MoveWindow(0, topOfListBox, cx + extraSize, cy - listBoxBottomOffset - topOfListBox + extraSize + 1);
	}
	m_search.MoveWindow(m_DittoWindow.m_dpi.Scale(34), cy - m_DittoWindow.m_dpi.Scale(searchRowStart-5), cx - m_DittoWindow.m_dpi.Scale(70), m_DittoWindow.m_dpi.Scale(25));

	m_systemMenu.MoveWindow(cx - m_DittoWindow.m_dpi.Scale(30), cy - m_DittoWindow.m_dpi.Scale(28), m_DittoWindow.m_dpi.Scale(24), m_DittoWindow.m_dpi.Scale(24));

	m_ShowGroupsFolderBottom.MoveWindow(m_DittoWindow.m_dpi.Scale(4), cy - m_DittoWindow.m_dpi.Scale(28), m_DittoWindow.m_dpi.Scale(24), m_DittoWindow.m_dpi.Scale(24));

	/*if (g_Opt.m_bShowPersistent &&
		g_Opt.m_bShowAlwaysOnTopWarning)
	{
		m_alwaysOnToWarningStatic.ShowWindow(SW_SHOW);
		m_alwaysOnToWarningStatic.MoveWindow(m_DittoWindow.m_dpi.Scale(2), cy - m_DittoWindow.m_dpi.Scale(18), cx - m_DittoWindow.m_dpi.Scale(4), m_DittoWindow.m_dpi.Scale(17));
	}
	else*/
	{
		m_alwaysOnToWarningStatic.ShowWindow(SW_HIDE);
	}
}

void CQPasteWnd::OnSetFocus(CWnd *pOldWnd)
{
    CWndEx::OnSetFocus(pOldWnd);

    if(::IsWindow(m_lstHeader.m_hWnd))
    {
        m_lstHeader.SetFocus();
    }
}

void CQPasteWnd::OnKillFocus(CWnd *pOldWnd)
{
	CWndEx::OnKillFocus(pOldWnd);	
}

void CQPasteWnd::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
    CWndEx::OnActivate(nState, pWndOther, bMinimized);

    if(m_bHideWnd == false || (m_lstHeader.GetToolTipHWnd() != NULL && m_lstHeader.GetToolTipHWnd() == pWndOther->GetSafeHwnd()))
    {
        return ;
    }

	Log(StrF(_T("CQPasteWnd::OnActivate, nState: %d, Other: %d, Minimized: %d"), nState, pWndOther, bMinimized));

    if(nState == WA_INACTIVE)
    {
		SaveWindowSize();

        m_bModifersMoveActive = false;

        if(!g_Opt.m_bShowPersistent)
        {
            HideQPasteWindow(false);
        }
		else if(g_Opt.GetAutoHide())
		{
			MinMaxWindow(FORCE_MIN);
		}

        //re register the global hot keys for the last ten
        if(theApp.m_bAppExiting == false)
        {
            g_HotKeys.RegisterAll();
        }

        m_lstHeader.HidePopup(true);
    }
    else if(nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
    {
		if(bMinimized == FALSE)
		{
			if(theApp.m_bShowingQuickPaste == false)
			{
				ShowQPasteWindow(m_listItems.size() == 0);
			}

			//Unregister the global hot keys for the last ten copies
			g_HotKeys.UnregisterAll(false, true);
		}
    }
}

BOOL CQPasteWnd::HideQPasteWindow (bool releaseFocus, bool clearSearchData)
{
    Log(_T("Start of HideQPasteWindow"));
	DWORD startTick = GetTickCount();

    if(!theApp.m_bShowingQuickPaste)
    {
        Log(_T("End of HideQPasteWindow, !theApp.m_bShowingQuickPaste"));
    }

	{
		ATL::CCritSecLock csLock(m_CritSection.m_sect);
    	
		m_bStopQuery = true;
	}

    theApp.m_bShowingQuickPaste = false;

	//needs to be before we hide our window - inorder to set focus to another window we need to be the foreground window
	//http://msdn.microsoft.com/en-us/library/windows/desktop/ms632668%28v=vs.85%29.aspx
	if(releaseFocus)
	{
		theApp.m_activeWnd.ReleaseFocus();
	}

    KillTimer(TIMER_FILL_CACHE);

	m_lstHeader.HidePopup(true);

	m_popupMsg.Hide();

    //Save the size
    SaveWindowSize();

	if (CGetSetOptions::GetShowInTaskBar())
	{
		ShowWindow(SW_MINIMIZE);
	}
	else
	{
		ShowWindow(SW_HIDE);
	}
	
	if(clearSearchData)
	{
		//Reset the selection in the search combo
		m_bHandleSearchTextChange = false;
		m_search.SetWindowText(_T(""));
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
            
				m_listItems.clear();
				m_lstHeader.SetItemCountEx(0);
			}
		}
	}

	if (theApp.m_GroupID > 0 &&
		CGetSetOptions::GetRevertToTopLevelGroup())
	{
		theApp.EnterGroupID(-1);
	}
	else
	{
		theApp.TryEnterOldGroupState();
	}

	DWORD endTick = GetTickCount();
	if((endTick-startTick) > 150)
		Log(StrF(_T("Paste Timing HideQPasteWindow: %d"), endTick-startTick));

    Log(StrF(_T("End of HideQPasteWindow, ItemCount: %d"), m_listItems.size()));

    return TRUE;
}

void CQPasteWnd::SaveWindowSize()
{
	if (this->IsIconic() == FALSE)
	{
		CRect rect;
		GetWindowRectEx(&rect);
		CSize s = rect.Size();
		CGetSetOptions::SetQuickPasteSize(CSize(m_DittoWindow.m_dpi.UnScale(s.cx), m_DittoWindow.m_dpi.UnScale(s.cy)));
		CGetSetOptions::SetQuickPastePoint(rect.TopLeft());
	}
}

BOOL CQPasteWnd::ShowQPasteWindow(BOOL bFillList)
{
    theApp.m_bShowingQuickPaste = true;

    Log(StrF(_T("Start - ShowQPasteWindow - Fill List: %d, array count: %d"), bFillList, m_listItems.size()));

    //Ensure we have the latest theme file, this checks the last write time so it doesn't read the file each time
    g_Opt.m_Theme.Load(g_Opt.GetTheme(), false, true);

    SetCaptionColorActive(g_Opt.m_bShowPersistent, theApp.GetConnectCV());
	SetCaptionOn(CGetSetOptions::GetCaptionPos(), true, g_Opt.m_Theme.GetCaptionSize(), g_Opt.m_Theme.GetCaptionFontSize());

	UpdateStatus();

    m_bHideWnd = true;

	SetCurrentTransparency();

    m_lstHeader.SetNumberOfLinesPerRow(CGetSetOptions::GetLinesPerRow(), false);
    m_lstHeader.SetShowTextForFirstTenHotKeys(CGetSetOptions::GetShowTextForFirstTenHotKeys());
	m_lstHeader.SetShowIfClipWasPasted(CGetSetOptions::GetShowIfClipWasPasted());

    if(bFillList)
    {
        FillList();
    }
    else
    {
        MoveControls();
    }

    // always on top... for persistent showing (g_Opt.m_bShowPersistent)
    // SHOWWINDOW was also integrated into this function rather than calling it separately
	if(g_Opt.GetShowPersistent())
	{
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}

    //SetKeyModiferState(true);

	Log(StrF(_T("END - ShowQPasteWindow - Fill List: %d, array count: %d"), bFillList, m_listItems.size()));

    return TRUE;
}

bool CQPasteWnd::Add(const CString &csHeader, const CString &csText, int nID)
{
    int nNewIndex;

    if((nNewIndex = m_lstHeader.InsertItem(m_lstHeader.GetItemCount(), csHeader)) == -1)
	{
		return false;
	}

    m_lstHeader.SetItemData(nNewIndex, nID);

    return true;
}

BOOL CQPasteWnd::OpenID(int id, CSpecialPasteOptions pasteOptions)
{
	Log(StrF(_T("Start OpenId, Id: %d, Only CF_TEXT: %s"), id, pasteOptions.ToString()));

    if(pasteOptions.m_pPasteFormats == NULL)
    {
        if(theApp.EnterGroupID(id, FALSE, FALSE))
        {
            Log(_T("Entered group"));
            return TRUE;
        }
    }

    // else, it is a clip, so paste it
    CProcessPaste paste;

    paste.m_bSendPaste = g_Opt.m_bSendPasteMessageAfterSelection == TRUE ? true : false;
	paste.m_pasteOptions = pasteOptions;
	paste.m_pastedFromGroup = (theApp.m_GroupID > 0);

    paste.GetClipIDs().Add(id);
    
	if (paste.DoPaste())
	{
		theApp.OnPasteCompleted();

		if (g_Opt.m_bSendPasteMessageAfterSelection == FALSE)
		{
			theApp.m_activeWnd.ActivateTarget();
		}

		if (g_Opt.m_bShowPersistent && g_Opt.GetAutoHide())
		{
			MinMaxWindow(FORCE_MIN);
		}
	}
	else
	{
		CString errorMessage;
		errorMessage.Format(_T("Paste Error - %s"), paste.m_lastErrorMessage);
		m_popupMsg.Show(errorMessage, CPoint(0, 0), true);
		SetTimer(TIMER_ERROR_MSG, CGetSetOptions::GetErrorMsgPopupTimeout(), NULL);
	}

	Log(StrF(_T("End OpenId, Id: %d, Only CF_TEXT: %s"), id, pasteOptions.ToString()));

    return TRUE;
}

BOOL CQPasteWnd::OpenSelection(CSpecialPasteOptions pasteOptions)
{
    Log(_T("Start Open Selection"));
    ARRAY IDs;
    m_lstHeader.GetSelectionItemData(IDs);

    INT_PTR count = IDs.GetSize();

    if(count <= 0)
    {
        return FALSE;
    }

    if(count == 1)
    {
		return OpenID(IDs[0], pasteOptions);
    }

    CProcessPaste paste;

    paste.m_bSendPaste = g_Opt.m_bSendPasteMessageAfterSelection == TRUE ? true : false;
	paste.m_pasteOptions = pasteOptions;
	paste.m_pastedFromGroup = (theApp.m_GroupID > 0);

    paste.GetClipIDs().Copy(IDs);
	if (paste.DoPaste())
	{
		theApp.OnPasteCompleted();

		if (g_Opt.m_bSendPasteMessageAfterSelection == FALSE)
		{
			theApp.m_activeWnd.ActivateTarget();
		}

		if (g_Opt.m_bShowPersistent && g_Opt.GetAutoHide())
		{
			MinMaxWindow(FORCE_MIN);
		}
	}
	else
	{
		CString errorMessage;
		errorMessage.Format(_T("Paste Error - %s"), paste.m_lastErrorMessage);
		m_popupMsg.Show(errorMessage, CPoint(0, 0), true);
		SetTimer(TIMER_ERROR_MSG, CGetSetOptions::GetErrorMsgPopupTimeout(), NULL);
	}

    Log(_T("End Open Selection"));
    return TRUE;
}

BOOL CQPasteWnd::OpenIndex(int item)
{
	if (item >= m_lstHeader.GetItemCount())
	{
		return FALSE;
	}

	CSpecialPasteOptions pasteOptions;
    return OpenID(m_lstHeader.GetItemData(item), pasteOptions);
}

BOOL CQPasteWnd::NewGroup(bool bGroupSelection, int parentId)
{
    CGroupName Name;
    CString csName("");

    if(g_Opt.m_bPrompForNewGroupName)
    {
        m_bHideWnd = false;

		CDimWnd dimmer(this);

        INT_PTR nRet = Name.DoModal();

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

    int id = NewGroupID(parentId, csName);

    if(id <= 0)
    {
        return FALSE;
    }

    if(!bGroupSelection)
    {
        theApp.m_FocusID = id; // focus on the new group
        FillList();
        return TRUE;
    }

	CClipIDs IDs;
	m_lstHeader.GetSelectionItemData(IDs);
    IDs.MoveTo(id);
    theApp.EnterGroupID(id);
    return TRUE;
}

LRESULT CQPasteWnd::OnListSelect_DB_ID(WPARAM wParam, LPARAM lParam)
{
	CSpecialPasteOptions pasteOptions;
	OpenID((int) wParam, pasteOptions);
    return TRUE;
}

LRESULT CQPasteWnd::OnListMoveSelectionToGroup(WPARAM wParam, LPARAM lParam)
{
	int groupId = (int)wParam;
	if(groupId >= -1)
	{
		CClipIDs IDs;
		m_lstHeader.GetSelectionItemData(IDs);

		IDs.MoveTo(groupId);
	}
	return TRUE;
}

LRESULT CQPasteWnd::OnCopyClip(WPARAM wParam, LPARAM lParam)
{
	DoCopySelection();
	return TRUE;
}

LRESULT CQPasteWnd::OnSearchEnterKeyPressed(WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	msg.lParam = 0;
	msg.wParam = VK_RETURN;
	msg.message = WM_KEYDOWN;
	if (CheckActions(&msg) == false)
	{
	}
    return TRUE;
}

LRESULT CQPasteWnd::OnListEnd(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CQPasteWnd::OnReloadClipAfterPaste(WPARAM wParam, LPARAM lParam)
{
	DWORD startTick = GetTickCount();

	BOOL foundClip = FALSE;
	int clipId = (int)wParam;
	int updateFlags = (int)lParam;

	CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT clipOrder, clipGroupOrder, lastPasteDate FROM Main WHERE lID = %d"), clipId);			
	if(q.eof() == false)
	{
		double order = q.getFloatField(_T("clipOrder"));
		double orderGroup = q.getFloatField(_T("clipGroupOrder"));
		int lastPasted = q.getIntField(_T("lastPasteDate"));

		std::vector<CMainTable>::iterator iter = m_listItems.begin();
		while(iter != m_listItems.end())
		{
			if(iter->m_lID == clipId)
			{
				iter->m_datePasted = lastPasted;
				
				if (iter->m_clipOrder != order || iter->m_clipGroupOrder != orderGroup)
				{
					iter->m_clipOrder = order;
					iter->m_clipGroupOrder = orderGroup;

					if (theApp.m_GroupID > 0)
					{
						std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
					}
					else
					{
						std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
					}
				}
				
				foundClip = TRUE;
				
				if (updateFlags & UPDATE_AFTER_PASTE_SELECT_CLIP)
				{
					theApp.m_FocusID = clipId;
					SelectFocusID();
				}

				if (updateFlags & UPDATE_AFTER_PASTE_REFRESH_VISIBLE)
				{
					m_lstHeader.RefreshVisibleRows();
					m_lstHeader.RedrawWindow();
				}

				break;
			}

			iter++;
		}
	}

	DWORD endTick = GetTickCount();
	if((endTick-startTick) > 150)
		Log(StrF(_T("Paste Timing OnReloadClipAfterPaste: %d, ClipId: %d"), endTick-startTick, clipId));

	return foundClip;
}

LRESULT CQPasteWnd::OnRefreshView(WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    // remove all additional refresh view messages from the queue
	while(::PeekMessage(&msg, m_hWnd, WM_REFRESH_VIEW, WM_REFRESH_VIEW, PM_REMOVE)){}

	Log(_T("OnRefreshView - Start"));
	CString action;

	theApp.m_FocusID = -1;

	m_bHandleSearchTextChange = false;
	m_search.SetWindowText(_T(""));
	m_bHandleSearchTextChange = true;

	if(theApp.m_bShowingQuickPaste)
	{	
		FillList();
		action = _T("Filled List");
	}
    else
    {
        //Wait for the thread to stop fill the cache so we can clear it
        WaitForSingleObject(m_thread.m_SearchingEvent, 5000);

		{
			ATL::CCritSecLock csLock(m_CritSection.m_sect);
			m_listItems.clear();
		}

		m_lstHeader.SetItemCountEx(0);
		UpdateStatus();

		action = _T("Cleared Items");
    }

    Log(StrF(_T("OnRefreshView - End - Count: %d, Action: %s"), m_listItems.size(), action));

    return TRUE;
}

void CQPasteWnd::RefreshNc()
{
    if(!theApp.m_bShowingQuickPaste)
    {
        return ;
    }

	InvalidateNc();
}

void CQPasteWnd::UpdateStatus(bool bRepaintImmediately)
{
    CString title = m_Title;    

	if (g_Opt.m_bShowPersistent)
	{
		title = (StrF(_T("%s %s"), _T(QPASTE_TITLE), theApp.m_Language.GetString("top_window", "[Always on top]")));
	}

	if (theApp.IsClipboardViewerConnected() == FALSE)
	{
		title += _T(" ");
		title += theApp.m_Language.GetString("disconnected", "[Disconnected]");
	}

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

    if(::IsWindow(theApp.m_activeWnd.ActiveWnd()))
    {
        title += theApp.m_activeWnd.ActiveWndName();
    }
    else
    {
        title += theApp.m_Language.GetString("No_Target", "No target");
    }

    SetToolTipText(title);

	CString windowTitle = _T(QPASTE_TITLE);

	if (g_Opt.m_bShowPersistent)
	{
		windowTitle += StrF(_T(" %s"), theApp.m_Language.GetString("top_window", "[Always on top]"));
	}

	if (theApp.IsClipboardViewerConnected() == FALSE)
	{
		windowTitle += StrF(_T(" %s"), theApp.m_Language.GetString("disconnected", "[Disconnected]"));
	}

	SetCustomWindowTitle(windowTitle);
}

BOOL CQPasteWnd::FillList(CString csSQLSearch /*=""*/)
{
    KillTimer(TIMER_DO_SEARCH);
	m_lstHeader.HidePopup(true);

    Log(StrF(_T("Start Fill List - %s"), csSQLSearch));

	m_lstHeader.SetSearchText(csSQLSearch);

	{
	    ATL::CCritSecLock csLock(m_CritSection.m_sect);
	    m_bStopQuery = true;
	}

    CString strFilter;
    CString strParentFilter;
    CString csSort;

    // History Groupiter->m_stickyClipGroupOrder = clip.m_stickyClipGroupOrder;
    if(theApp.m_GroupID < 0)
    {
        m_lstHeader.m_bStartTop = true;
        
		//do not change this this directly relates to the views in the Main table
		csSort = "Main.bIsGroup ASC, " 
				 "Main.stickyClipOrder DESC, "				 
				 "Main.clipOrder DESC";

        if(g_Opt.m_bShowAllClipsInMainList)
        {
			if (CGetSetOptions::GetShowGroupsInMainList())
			{
				//found to be slower on large databases
				strFilter = "((Main.bIsGroup = 1 AND Main.lParentID = -1) OR Main.bIsGroup = 0)";
			}
			else
			{
				strFilter = "(Main.bIsGroup = 0)";
			}
        }
        else
        {
            strFilter = "((Main.bIsGroup = 1 AND Main.lParentID = -1) OR (Main.bIsGroup = 0 AND Main.lParentID = -1))";
        }
    }
    else
    // it's some other group
    {
        m_lstHeader.m_bStartTop = true;

		//do not change this this directly relates to the views in the Main table
		csSort = "Main.bIsGroup ASC, "
				 "Main.stickyClipGroupOrder DESC, "				 
				 "Main.clipGroupOrder DESC";
			
			//Main.stickyClipGroupOrder DESC, Main.clipGroupOrder DESC";//
        
        if(theApp.m_GroupID >= 0)
        {
            strFilter.Format(_T("Main.lParentID = %d"), theApp.m_GroupID);
            strParentFilter = strFilter;
        }

        m_stGroup.SetWindowText(theApp.m_GroupText);
    }

    CRect crRect;
    GetClientRect(crRect);

    CString csSQL;
	CString dataJoin;

    if(csSQLSearch == "")
    {
        m_strSQLSearch = "";
		m_strSearch = "";
    }
    else
    {
        CFormatSQL descriptionFormat;
		CString descriptionSql;
		CFormatSQL quickPasteFormat;
		CString quickPasteSql;
		CFormatSQL fullTextFormat;
		CString fullTextSql;

		//If other are off then always search the description
		if(CGetSetOptions::GetSearchDescription() ||
			(CGetSetOptions::GetSearchFullText() == FALSE && CGetSetOptions::GetSearchQuickPaste() == FALSE))
		{
			descriptionFormat.SetVariable("Main.mText");

			descriptionFormat.Parse(csSQLSearch);
			descriptionSql = descriptionFormat.GetSQLString();
		}

		if (csSQLSearch.Left(3) == _T("/q ") ||
			csSQLSearch.Left(3) == _T("\\q ") ||
			CGetSetOptions::GetSearchQuickPaste())
		{
			quickPasteFormat.SetVariable("Main.QuickPasteText");

			if (csSQLSearch.Left(3) == _T("/q ") ||
				csSQLSearch.Left(3) == _T("\\q "))
			{
				csSQLSearch = csSQLSearch.Mid(3);
			}

			quickPasteFormat.Parse(csSQLSearch);
			quickPasteSql = quickPasteFormat.GetSQLString();
		}
		
		if(csSQLSearch.Left(3) == _T("/f ") ||
			csSQLSearch.Left(3) == _T("\\f ") ||
			CGetSetOptions::GetSearchFullText())
		{
			dataJoin = _T("INNER JOIN Data on Data.lParentID = Main.lID");

			if (csSQLSearch.Left(3) == _T("/f ") ||
				csSQLSearch.Left(3) == _T("\\f "))
			{
				csSQLSearch = csSQLSearch.Mid(3);
			}

			fullTextFormat.SetVariable("Data.ooData");
			fullTextFormat.Parse(csSQLSearch);
			fullTextSql = fullTextFormat.GetSQLString();

			fullTextSql.Insert(1, _T("Data.strClipBoardFormat = 'CF_UNICODETEXT' AND "));

			//If we are also search for other formats limit it them to unicode text, otherwise multiple rows could be returned
			if(descriptionSql != _T(""))
			{
				descriptionSql.Insert(1, _T("Data.strClipBoardFormat = 'CF_UNICODETEXT' AND "));
			}

			if(quickPasteSql != _T(""))
			{
				quickPasteSql.Insert(1, _T("Data.strClipBoardFormat = 'CF_UNICODETEXT' AND "));
			}
		}

		strFilter = _T("(");

		if(descriptionSql != _T(""))
		{
			strFilter += descriptionSql;
		}

		if(quickPasteSql != _T(""))
		{
			if(descriptionSql != _T(""))
			{
				strFilter += _T(" OR ");
			}

			strFilter += quickPasteSql;
		}

		if(fullTextSql != _T(""))
		{
			if(descriptionSql != _T("") ||
				quickPasteSql != _T(""))
			{
				strFilter += _T(" OR ");
			}

			strFilter += fullTextSql;
		}

		strFilter += _T(")");		

        if(strParentFilter.IsEmpty() == FALSE)
        {
            strFilter += " AND ";
            strFilter += strParentFilter;
        }

        m_strSQLSearch = strFilter;
		m_strSearch = csSQLSearch;
    }

	{
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

	    //Format the count and select sql queries for the thread
	    m_CountSQL.Format(_T("SELECT COUNT(Main.lID) FROM Main %s where %s"), dataJoin, strFilter);

	    m_SQL.Format(_T("SELECT Main.lID, Main.mText, Main.lParentID, Main.lDontAutoDelete, ")
			_T("Main.lShortCut, Main.bIsGroup, Main.QuickPasteText, Main.clipOrder, Main.clipGroupOrder, ")
				_T("Main.stickyClipOrder, Main.stickyClipGroupOrder, Main.lDate, Main.lastPasteDate FROM Main %s ")
			_T("where %s order by %s"), dataJoin, strFilter, csSort);
	}

	{
		ATL::CCritSecLock csLock(m_CritSection.m_sect);
		m_listItems.clear();
	}

	m_noSearchResults = false;
	m_lstHeader.SetItemCount(0);
	m_lstHeader.RefreshVisibleRows();
	
	CPoint loadItem(-1, m_lstHeader.GetCountPerPage() + 2);
	m_loadItems.push_back(loadItem);
	m_thread.FireLoadItems(true);

	MoveControls();

	m_CountSQL.Replace(_T("%"), _T("%%"));
	m_SQL.Replace(_T("%"), _T("%%"));
	Log(StrF(_T("Start Fill List - Count SQL: %s, Query SQL: %s"), m_CountSQL, m_SQL));

    return TRUE;
}

void CQPasteWnd::ShowRightClickMenu()
{
    POINT pp;
    CMenu cmPopUp;
    CMenu *cmSubMenu = NULL;

    GetCursorPos(&pp);
	if (cmPopUp.LoadMenu(IDR_QUICK_PASTE) != 0)
	{
		cmSubMenu = cmPopUp.GetSubMenu(0);
		if (!cmSubMenu)
		{
			return;
		}

		int nItem = m_lstHeader.GetCaret();

		CRect rc;
		m_lstHeader.GetItemRect(nItem, rc, LVIR_BOUNDS);
		ClientToScreen(rc);

		if (rc.PtInRect(pp) == FALSE)
		{
			pp.x = rc.left;
			pp.y = rc.bottom;
		}

		theApp.m_Addins.AddPrePasteAddinsToMenu(cmSubMenu);

		SetFriendChecks(cmSubMenu);

		CString specialPaste("Special Paste");
		int nPos = -1;
		CMenu *sendToMenu = CMultiLanguage::GetMenuPos(cmSubMenu, specialPaste, nPos, TRUE);
		if (sendToMenu != NULL)
		{
			g_Opt.m_pasteScripts.AddToMenu(sendToMenu, &m_actions);
		}
		
		theApp.m_Language.UpdateRightClickMenu(cmSubMenu);

        cmSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pp.x, pp.y, this, NULL);
    }
}

void CQPasteWnd::SetFriendChecks(CMenu *pMenu)
{
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

	if (g_Opt.GetAllowFriends() == false)
	{
		CString csText("Send To");
		int nPos = -1;
		CMultiLanguage::GetMenuPos(pMenu, csText, nPos);
		if (nPos >= 0)
		{
			pMenu->DeleteMenu(nPos, MF_BYPOSITION);
		}
	}
	else
	{
		CString csText("Send To");
		int nPos = -1;
		CMenu *sendToMenu = CMultiLanguage::GetMenuPos(pMenu, csText, nPos, TRUE);

		if (sendToMenu != NULL)
		{
			m_customFriendsHelper.Load();
			m_customFriendsHelper.AddToMenu(sendToMenu);
		}
	}
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
			case 35:
				nCheckID = ID_TRANSPARENCY_35;
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
    {
        pMenu->CheckMenuItem(nCheckID, MF_CHECKED);
    }

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

	if (g_Opt.GetPasteAsAdmin())
	{
		pMenu->CheckMenuItem(ID_QUICKOPTIONS_ELEVATEPREVILEGESTOPASTEINTOELEVATEDAPPS, MF_CHECKED);
	}

	if(g_Opt.GetShowInTaskBar())
	{
		pMenu->CheckMenuItem(ID_QUICKOPTIONS_SHOWINTASKBAR, MF_CHECKED);
	}

	if (CGetSetOptions::GetShowTextForFirstTenHotKeys())
	{
		pMenu->CheckMenuItem(ID_QUICKOPTIONS_SHOWTEXTFORFIRSTTENCOPYHOTKEYS, MF_CHECKED);
	}

	if (CGetSetOptions::GetShowIfClipWasPasted())
	{
		pMenu->CheckMenuItem(ID_QUICKOPTIONS_SHOWINDICATORACLIPHASBEENPASTED, MF_CHECKED);
	}

	if (CGetSetOptions::GetSearchDescription())
		pMenu->CheckMenuItem(ID_MENU_SEARCHDESCRIPTION, MF_CHECKED);

	if (CGetSetOptions::GetSearchFullText())
		pMenu->CheckMenuItem(ID_MENU_SEARCHFULLTEXT, MF_CHECKED);

	if (CGetSetOptions::GetSearchQuickPaste())
		pMenu->CheckMenuItem(ID_MENU_SEARCHQUICKPASTE, MF_CHECKED);

	if (CGetSetOptions::GetSimpleTextSearch())
		pMenu->CheckMenuItem(ID_MENU_CONTAINSTEXTSEARCHONLY, MF_CHECKED);

	if (CGetSetOptions::GetRegExTextSearch())
		pMenu->CheckMenuItem(ID_MENU_REGULAREXPRESSIONSEARCH, MF_CHECKED);

	if (CGetSetOptions::GetSimpleTextSearch() == FALSE &&
		CGetSetOptions::GetRegExTextSearch() == FALSE)
	{
		pMenu->CheckMenuItem(ID_MENU_WILDCARDSEARCH, MF_CHECKED);
	}
}

void CQPasteWnd::SetSendToMenu(CMenu *pMenu, int nMenuID, int nArrayPos)
{
    if(g_Opt.m_SendClients[nArrayPos].csIP.GetLength() > 0)
    {
        CString cs;
		if (g_Opt.m_SendClients[nArrayPos].csDescription != _T(""))
		{
			cs.Format(_T("(%s) - %s"), g_Opt.m_SendClients[nArrayPos].csIP, g_Opt.m_SendClients[nArrayPos].csDescription);
		}
		else
		{
			cs.Format(_T("%s"), g_Opt.m_SendClients[nArrayPos].csIP);
		}
        pMenu->ModifyMenu(nMenuID, MF_BYCOMMAND, nMenuID, cs);
    }
    else
    {
        pMenu->DeleteMenu(nMenuID, MF_BYCOMMAND);
    }
}

LRESULT CQPasteWnd::OnSearch(WPARAM wParam, LPARAM lParam)
{
    CString csText;
    m_search.GetWindowText(csText);

	FillList(csText);

	m_lstHeader.SetFocus();

	MoveControls();

	m_search.SetSel(-1, 0);

	return TRUE;
}


///////////////////////////////////////////////////////////////////////
//Menu Stuff
///////////////////////////////////////////////////////////////////////
void CQPasteWnd::OnMenuLinesperrow1()
{
    SetLinesPerRow(1, false, true);
}

void CQPasteWnd::OnMenuLinesperrow2()
{
    SetLinesPerRow(2, false, true);
}

void CQPasteWnd::OnMenuLinesperrow3()
{
    SetLinesPerRow(3, false, true);
}

void CQPasteWnd::OnMenuLinesperrow4()
{
    SetLinesPerRow(4, false, true);
}

void CQPasteWnd::OnMenuLinesperrow5()
{
    SetLinesPerRow(5, false, true);
}

void CQPasteWnd::SetLinesPerRow(int lines, bool force, bool resetListCount)
{
	ARRAY Indexs;
	int listCount = 0;

	if (resetListCount)
	{
		//save and restore state, list box seems to pain funny (gap at header) if items are not reset
		m_lstHeader.GetSelectionIndexes(Indexs);
		if (Indexs.GetCount() <= 0)
		{
			Indexs.Add(0);
		}
		listCount = m_lstHeader.GetItemCount();
		m_lstHeader.SetItemCountEx(0);
	}

    CGetSetOptions::SetLinesPerRow(lines);
    m_lstHeader.SetNumberOfLinesPerRow(lines, force);

	ATL::CCritSecLock csLock(m_CritSection.m_sect);

	m_cf_dibCache.clear();
	m_cf_NO_dibCache.clear();
	m_cf_rtfCache.clear();
	m_cf_NO_rtfCache.clear();

	if (resetListCount)
	{
		m_lstHeader.SetItemCountEx(listCount);
		m_lstHeader.SetListPos(Indexs[0]);
		m_lstHeader.RefreshVisibleRows();
	}
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
	theApp.m_pMainFrame->SendMessage(WM_SHOW_OPTIONS, 0, 0);
}

void CQPasteWnd::OnMenuExitprogram()
{
    ::SendMessage(theApp.m_MainhWnd, WM_CLOSE, 0, 0);
}

void CQPasteWnd::OnMenuToggleConnectCV()
{
	this->DoAction(ActionEnums::TOGGLE_CLIPBOARD_CONNECTION);
}

void CQPasteWnd::OnUpdate32775(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TOGGLE_CLIPBOARD_CONNECTION);
}

void CQPasteWnd::OnMenuProperties()
{
    this->DoAction(ActionEnums::CLIP_PROPERTIES);
}

void CQPasteWnd::UpdateFont()
{
    LOGFONT lf;
    CGetSetOptions::GetFont(lf);
	lf.lfHeight = m_DittoWindow.m_dpi.Scale(lf.lfHeight);
    m_lstHeader.SetLogFont(lf);

	m_SearchFont.Detach();
	m_SearchFont.CreateFont(-m_DittoWindow.m_dpi.Scale(15), 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, 3, 2, 1, 34, _T("Segoe UI"));
	m_search.SetFont(&m_SearchFont);
	m_search.SetPromptFont(m_SearchFont);

	m_GroupTree.SetFont(&m_SearchFont);

	m_groupFont.Detach();
	m_groupFont.CreateFont(-m_DittoWindow.m_dpi.Scale(12), 0, 0, 0, 400, 0, 1, 0, DEFAULT_CHARSET, 3, 2, 1, 34, _T("Segoe UI"));
	m_stGroup.SetFont(&m_groupFont);
	m_stGroup.SetBkColor(g_Opt.m_Theme.MainWindowBG());
	m_stGroup.SetTextColor(g_Opt.m_Theme.ListBoxEvenRowsText());

	m_noSearchResultsStatic.SetBkColor(g_Opt.m_Theme.MainWindowBG());
	m_noSearchResultsStatic.SetTextColor(g_Opt.m_Theme.ListBoxEvenRowsText());
	m_noSearchResultsStatic.SetFont(&m_SearchFont);
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
    SetCaptionOn(CAPTION_RIGHT, false, g_Opt.m_Theme.GetCaptionSize(), g_Opt.m_Theme.GetCaptionFontSize());
    CGetSetOptions::SetCaptionPos(CAPTION_RIGHT);
}

void CQPasteWnd::OnViewcaptionbaronBottom()
{
    SetCaptionOn(CAPTION_BOTTOM, false, g_Opt.m_Theme.GetCaptionSize(), g_Opt.m_Theme.GetCaptionFontSize());
    CGetSetOptions::SetCaptionPos(CAPTION_BOTTOM);
}

void CQPasteWnd::OnViewcaptionbaronLeft()
{
    SetCaptionOn(CAPTION_LEFT, false, g_Opt.m_Theme.GetCaptionSize(), g_Opt.m_Theme.GetCaptionFontSize());
    CGetSetOptions::SetCaptionPos(CAPTION_LEFT);
}

void CQPasteWnd::OnViewcaptionbaronTop()
{
    SetCaptionOn(CAPTION_TOP, false, g_Opt.m_Theme.GetCaptionSize(), g_Opt.m_Theme.GetCaptionFontSize());
    CGetSetOptions::SetCaptionPos(CAPTION_TOP);
}

void CQPasteWnd::OnMenuAutohide()
{
    bool bAutoHide = !CGetSetOptions::GetAutoHide();
    CGetSetOptions::SetAutoHide(bAutoHide);
}

void CQPasteWnd::OnMenuViewfulldescription()
{
    this->DoAction(ActionEnums::SHOWDESCRIPTION);
}

void CQPasteWnd::OnMenuAllwaysontop()
{
	this->DoAction(ActionEnums::TOGGLESHOWPERSISTANT);
}

void CQPasteWnd::OnMenuNewGroup()
{
	this->DoAction(ActionEnums::NEWGROUP);
}

void CQPasteWnd::OnMenuNewGroupSelection()
{
	this->DoAction(ActionEnums::NEWGROUPSELECTION);
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
	this->DoAction(ActionEnums::SHOWGROUPS);
}

void CQPasteWnd::OnMenuQuickpropertiesSettoneverautodelete()
{
    CWaitCursor wait;
    ARRAY IDs;
    ARRAY Indexs;
    m_lstHeader.GetSelectionItemData(IDs);
    m_lstHeader.GetSelectionIndexes(Indexs);

    INT_PTR count = IDs.GetSize();

    for(int i = 0; i < count; i++)
    {
        try
        {
            theApp.m_db.execDMLEx(_T("UPDATE Main SET lDontAutoDelete = %d where lID = %d;"), (int)CTime::GetCurrentTime().GetTime(), IDs[i]);
        }
        CATCH_SQLITE_EXCEPTION
    }

    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        count = Indexs.GetSize();
        for(int row = 0; row < count; row++)
        {
			if(Indexs[row] < (int)m_listItems.size())
			{
				m_listItems[Indexs[row]].m_bDontAutoDelete = true;
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

    INT_PTR count = IDs.GetSize();

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
			if(Indexs[row] < (int)m_listItems.size())
			{
				m_listItems[Indexs[row]].m_bDontAutoDelete = false;
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

    INT_PTR count = IDs.GetSize();

    for(int i = 0; i < count; i++)
    {
        try
        {
            theApp.m_db.execDMLEx(_T("UPDATE Main SET lShortCut = 0, globalShortCut = 0 where lID = %d;"), IDs[i]);
			g_HotKeys.Remove(IDs[i], CHotKey::PASTE_OPEN_CLIP);
			theApp.m_db.execDMLEx(_T("UPDATE Main SET MoveToGroupShortCut = 0, GlobalMoveToGroupShortCut = 0 where lID = %d;"), IDs[i]);
			g_HotKeys.Remove(IDs[i], CHotKey::MOVE_TO_GROUP);
        }
        CATCH_SQLITE_EXCEPTION
    }

    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        count = Indexs.GetSize();
        for(int row = 0; row < count; row++)
        {
			if(Indexs[row] < (int)m_listItems.size())
			{
				m_listItems[Indexs[row]].m_bHasShortCut = false;
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

    INT_PTR count = IDs.GetSize();

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
			if(Indexs[row] < (int)m_listItems.size())
			{
				m_listItems[Indexs[row]].m_QuickPaste.Empty();
			}
        }
    }

    m_lstHeader.RefreshVisibleRows();
}

void CQPasteWnd::OnMenuSenttoFriendFifteen()
{
    SendToFriendbyPos(14, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendForeteen()
{
    SendToFriendbyPos(13, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendThirteen()
{
    SendToFriendbyPos(12, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendTwelve()
{
    SendToFriendbyPos(11, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendEleven()
{
    SendToFriendbyPos(10, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendTen()
{
    SendToFriendbyPos(9, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendNine()
{
    SendToFriendbyPos(8, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendEight()
{
    SendToFriendbyPos(7, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendSeven()
{
    SendToFriendbyPos(6, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendSix()
{
    SendToFriendbyPos(5, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendFive()
{
    SendToFriendbyPos(4, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendFore()
{
    SendToFriendbyPos(3, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendThree()
{
    SendToFriendbyPos(2, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendTwo()
{
    SendToFriendbyPos(1, _T(""));
}

void CQPasteWnd::OnMenuSenttoFriendone()
{
    SendToFriendbyPos(0, _T(""));
}

void CQPasteWnd::OnMenuSenttoPromptforip()
{
    // TODO: Add your command handler code here

}

void CQPasteWnd::OnMenuGroupsMovetogroup()
{
    this->DoAction(ActionEnums::MOVE_CLIP_TO_GROUP);
}

void CQPasteWnd::OnMenuPasteplaintextonly()
{
    this->DoAction(ActionEnums::PASTE_SELECTED_PLAIN_TEXT);
}

void CQPasteWnd::OnPromptToDeleteClip()
{
    CGetSetOptions::SetPromptWhenDeletingClips(!CGetSetOptions::GetPromptWhenDeletingClips());
}

void CQPasteWnd::OnMakeTopStickyClip()
{
	this->DoAction(ActionEnums::MAKE_TOP_STICKY);
}

void CQPasteWnd::OnMakeLastStickyClip()
{
	this->DoAction(ActionEnums::MAKE_LAST_STICKY);
}

void CQPasteWnd::OnRemoveSticky()
{
	this->DoAction(ActionEnums::REMOVE_STICKY);
}

void CQPasteWnd::OnUpdateStickyclipsMaketopstickyclip(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::MAKE_TOP_STICKY);
}

void CQPasteWnd::OnUpdateStickyclipsMakelaststickyclip(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::MAKE_LAST_STICKY);
}

void CQPasteWnd::OnUpdateStickyclipsRemovestickysetting(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::REMOVE_STICKY);
}

void CQPasteWnd::OnElevateAppToPasteIntoElevatedApp()
{
	this->DoAction(ActionEnums::ELEVATE_PRIVlEGES);
}

void CQPasteWnd::OnMenuExport()
{
    CClipIDs IDs;
    INT_PTR lCount = m_lstHeader.GetSelectedCount();
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
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

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
	CHyperLink::GotoURL(_T("https://sourceforge.net/p/ditto-cp/wiki/"), SW_SHOW);
}

void CQPasteWnd::OnMenuQuickoptionsFont()
{
    m_bHideWnd = false;

	CDimWnd dimmer(this);

    CFont *pFont = m_lstHeader.GetFont();
    LOGFONT lf;
    pFont->GetLogFont(&lf);

	lf.lfHeight = m_DittoWindow.m_dpi.UnScale(lf.lfHeight);

    CFontDialog dlg(&lf);
    if(dlg.DoModal() == IDOK)
    {
        CGetSetOptions::SetFont(*dlg.m_cf.lpLogFont);
		(*dlg.m_cf.lpLogFont).lfHeight = m_DittoWindow.m_dpi.Scale((*dlg.m_cf.lpLogFont).lfHeight);
        m_lstHeader.SetLogFont(*dlg.m_cf.lpLogFont);
		this->SetLinesPerRow(CGetSetOptions::GetLinesPerRow(), true, true);
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
    m_search.GetWindowText(csText);
    FillList(csText);
}

void CQPasteWnd::OnMenuEdititem()
{
	this->DoAction(ActionEnums::EDITCLIP);
}

void CQPasteWnd::OnMenuNewclip()
{
	this->DoAction(ActionEnums::NEWCLIP);
}


///////////////////////////////////////////////////////////////////////
//END END Menu Stuff
///////////////////////////////////////////////////////////////////////


BOOL CQPasteWnd::SendToFriendbyPos(int nPos, CString override_IP_Host)
{
    if(g_Opt.GetAllowFriends() == false)
    {
        return FALSE;
    }

    CWaitCursor wait;

    m_bHideWnd = false;

    CClipIDs IDs;
    INT_PTR count = m_lstHeader.GetSelectedCount();
    if(count <= 0)
    {
        return FALSE;
    }

    m_lstHeader.GetSelectionItemData(IDs);
    count = IDs.GetSize();
    if(count <= 0)
    {
        return FALSE;
    }

    CSendToFriendInfo Info;

    BOOL bRet = FALSE;

    try
    {
		Info.m_manualSend = true;
		Info.m_csIP = override_IP_Host;
		if (Info.m_csIP == _T(""))
		{
			Info.m_csIP = g_Opt.m_SendClients[nPos].csIP;
		}

		if (Info.m_csIP != _T(""))
		{
			CPopup Popup(0, 0, m_hWnd);
			Popup.Show(StrF(_T("Sending clip to %s"), Info.m_csIP));

			Info.m_pPopup = &Popup;

			Info.m_pClipList = new CClipList;
			for (int i = 0; i < count; i++)
			{
				CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT mText FROM Main WHERE lID = %d"), IDs[i]);
				if (q.eof() == false)
				{
					CClip *pClip = new CClip;
					if (pClip)
					{
						pClip->LoadFormats(IDs[i]);
						pClip->m_Desc = q.getStringField(0);
						pClip->m_id = IDs[i];
						Info.m_pClipList->AddTail(pClip);
					}
				}
			}

			if (SendToFriend(Info) == FALSE)
			{
				MessageBox(StrF(_T("Error Sending data to %s\n\n%s"), Info.m_csIP, Info.m_csErrorText), _T("Ditto"), MB_OK|MB_TOPMOST);
			}
			else
			{
				bRet = TRUE;
			}
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

        int nRet = MessageBox(theApp.m_Language.GetString("Delete_Clip", "Delete Selected Clips?"), _T("Ditto"), MB_OKCANCEL|MB_TOPMOST);

        m_bHideWnd = bStartValue;

        if(nRet != IDOK)
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
	   

    m_lstHeader.GetSelectionItemData(IDs);
    m_lstHeader.GetSelectionIndexes(Indexs);

	DeleteClips(IDs, Indexs);   
}

bool CQPasteWnd::DeleteClips(CClipIDs &IDs, ARRAY &Indexs)
{
	POSITION pos = m_lstHeader.GetFirstSelectedItemPosition();
	int nFirstSel = m_lstHeader.GetNextSelectedItem(pos);

	IDs.DeleteIDs(true, theApp.m_db);

	Indexs.SortDescending();
	INT_PTR count = Indexs.GetSize();

	int erasedCount = 0;

	{
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

		for (int i = 0; i < count; i++)
		{
			if (Indexs[i] < (int) m_listItems.size())
			{
				RemoveFromImageRtfCache(Indexs[i]);
				g_HotKeys.Remove(m_lstHeader.GetItemData(Indexs[i]), CHotKey::PASTE_OPEN_CLIP);

				m_listItems.erase(m_listItems.begin() + Indexs[i]);
				erasedCount++;
			}
		}
	}

	CClip::m_LastAddedCRC = 0;

	m_extraDataThread.FireLoadAccelerators();

	m_lstHeader.SetItemCountEx(m_lstHeader.GetItemCount() - erasedCount);

	// if there are no items after the one we deleted, then select the last one.
	if (nFirstSel >= m_lstHeader.GetItemCount())
	{
		nFirstSel = m_lstHeader.GetItemCount() - 1;
	}

	m_lstHeader.SetListPos(nFirstSel);
	UpdateStatus();

	if (m_lstHeader.IsToolTipWindowVisible())
	{
		m_lstHeader.ShowFullDescription(false, true);
	}

	return true;
}

void CQPasteWnd::RemoveFromImageRtfCache(int id)
{
	ATL::CCritSecLock csLock(m_CritSection.m_sect);

	CF_DibTypeMap::iterator iterDib = m_cf_dibCache.find(m_lstHeader.GetItemData(id));
	if (iterDib != m_cf_dibCache.end())
	{
		m_cf_dibCache.erase(iterDib);
	}

	CF_NoDibTypeMap::iterator iterNoDib = m_cf_NO_dibCache.find(m_lstHeader.GetItemData(id));
	if (iterNoDib != m_cf_NO_dibCache.end())
	{
		m_cf_NO_dibCache.erase(iterNoDib);
	}

	CF_DibTypeMap::iterator iterRtf = m_cf_rtfCache.find(m_lstHeader.GetItemData(id));
	if (iterRtf != m_cf_rtfCache.end())
	{
		m_cf_rtfCache.erase(iterRtf);
	}

	CF_NoDibTypeMap::iterator iterNoRtf = m_cf_NO_rtfCache.find(m_lstHeader.GetItemData(id));
	if (iterNoRtf != m_cf_NO_rtfCache.end())
	{
		m_cf_NO_rtfCache.erase(iterNoRtf);
	}
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
        int id = m_lstHeader.GetItemData(nItem);

        CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT mText FROM Main WHERE lID = %d"), id);
        if(q.eof() == false)
        {
            cs = q.getStringField(0);
        }
    }
    CATCH_SQLITE_EXCEPTION 

    return cs;
}

void CQPasteWnd::MoveSelection(bool down, bool requireModifersActive)
{
    if(m_bModifersMoveActive || requireModifersActive == false)
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
	switch (pMsg->message)
	{
	case WM_MBUTTONUP:
	{
		MSG msg;
		msg.lParam = 0;
		msg.wParam = VK_MOUSE_MIDDLE_CLICK;
		msg.message = WM_KEYDOWN;
		if (CheckActions(&msg) == false)
		{
		}
	}
	break;
	default:
		if (CheckActions(pMsg))
		{
			return TRUE;
		}
		break;
	}
    return CWndEx::PreTranslateMessage(pMsg);
}

bool CQPasteWnd::CheckActions(MSG * pMsg)
{
	bool ret = false;
	CAccel a;

	if (m_bModifersMoveActive)
	{
		if (m_modifierKeyActions.OnMsg(pMsg, a))
		{
			ret = DoAction(a);
		}
	}

	if (ret == false)
	{
		if (m_actions.OnMsg(pMsg, a))
		{
			KillTimer(TIMER_DO_ACTION);
			ret = DoAction(a);
		}
		else if (a.Cmd > 0)
		{
			m_timerAction = a;
			SetTimer(TIMER_DO_ACTION, CGetSetOptions::m_doubleKeyStrokeTimeout, NULL);
		}
	}

	return ret;
}

bool CQPasteWnd::DoAction(DWORD cmd)
{
	CAccel a(0, cmd);
	return DoAction(a);
}

bool CQPasteWnd::DoAction(CAccel a)
{
	bool ret = false;

	switch (a.Cmd)
	{
	case ActionEnums::SHOWDESCRIPTION:
		ret = DoActionShowDescription();
		break;
	case ActionEnums::NEXTDESCRIPTION:
		ret = DoActionNextDescription();
		break;
	case ActionEnums::PREVDESCRIPTION:
		ret = DoActionPrevDescription();
		break;
	case ActionEnums::SHOWMENU:
		ret = DoActionShowMenu();
		break;
	case ActionEnums::NEWGROUP:
		ret = DoActionNewGroup();
		break;
	case ActionEnums::NEWGROUPSELECTION:
		ret = DoActionNewGroupSelection();
		break;
	case ActionEnums::TOGGLEFILELOGGING:
		ret = DoActionToggleFileLogging();
		break;
	case ActionEnums::TOGGLEOUTPUTDEBUGSTRING:
		ret = DoActionToggleOutputDebugString();
		break;
	case ActionEnums::CLOSEWINDOW:
		ret = DoActionCloseWindow();
		break;
	case ActionEnums::FORCE_CLOSE_WINDOW:
		ret = DoActionForceCloseWindow();
		break;
	case ActionEnums::NEXTTABCONTROL:
		ret = DoActionNextTabControl();
		break;
	case ActionEnums::PREVTABCONTROL:
		ret = DoActionPrevTabControl();
		break;
	case ActionEnums::SHOWGROUPS:
		ret = DoActionShowGroups();
		break;
	case ActionEnums::NEWCLIP:
		ret = DoActionNewClip();
		break;
	case ActionEnums::EDITCLIP:
		ret = DoActionEditClip();
		break;
	case ActionEnums::MODIFIER_ACTVE_SELECTIONUP:
		ret = DoModifierActiveActionSelectionUp();
		break;
	case ActionEnums::MODIFIER_ACTVE_SELECTIONDOWN:
		ret = DoModifierActiveActionSelectionDown();
		break;
	case ActionEnums::MODIFIER_ACTVE_MOVEFIRST:
		ret = DoModifierActiveActionMoveFirst();
		break;
	case ActionEnums::MODIFIER_ACTVE_MOVELAST:
		ret = DoModifierActiveActionMoveLast();
		break;
	case ActionEnums::CANCELFILTER:
		ret = DoActionCancelFilter();
		break;
	case ActionEnums::HOMELIST:
		ret = DoActionHomeList();
		break;
	case ActionEnums::BACKGRROUP:
		ret = DoActionBackGroup();
		break;
	case ActionEnums::TOGGLESHOWPERSISTANT:
		ret = DoActionToggleShowPersistant();
		break;
	case ActionEnums::PASTE_SELECTED:
		ret = DoActionPasteSelected();
		break;
	case ActionEnums::DELETE_SELECTED:
		ret = DoActionDeleteSelected();
		break;
	case ActionEnums::CLIP_PROPERTIES:
		ret = DoActionClipProperties();
		break;
	case ActionEnums::PASTE_SELECTED_PLAIN_TEXT:
		ret = DoActionPasteSelectedPlainText();
		break;
	case ActionEnums::MOVE_CLIP_TO_GROUP:
		ret = DoActionMoveClipToGroup();
		break;
	case ActionEnums::ELEVATE_PRIVlEGES:
		ret = DoActionElevatePrivleges();
		break;
	case ActionEnums::SHOW_IN_TASKBAR:
		ret = DoShowInTaskBar();
		break;
	case ActionEnums::COMPARE_SELECTED_CLIPS:
		ret = DoClipCompare();
		break;
	case ActionEnums::SELECT_LEFT_SIDE_COMPARE:
		ret = DoSelectLeftSideCompare();
		break;
	case ActionEnums::SELECT_RIGHT_SITE_AND_DO_COMPARE:
		ret = DoSelectRightSideAndDoCompare();
		break;
	case ActionEnums::EXPORT_TO_TEXT_FILE:
		ret = DoExportToTextFile();
		break;
	case ActionEnums::EXPORT_TO_QR_CODE:
		ret = DoExportToQRCode();
		break;
	case ActionEnums::EXPORT_TO_GOOGLE_TRANSLATE:
		ret = DoExportToGoogleTranslate();
		break;
	case ActionEnums::EXPORT_TO_BITMAP_FILE:
		ret = DoExportToBitMapFile();
		break;
	case ActionEnums::SAVE_CURRENT_CLIPBOARD:
		ret = DoSaveCurrentClipboard();
		break;
	case ActionEnums::MOVE_CLIP_DOWN:
		ret = DoMoveClipDown();
		break;
	case ActionEnums::MOVE_CLIP_UP:
		ret = DoMoveClipUp();
		break;
	case ActionEnums::MOVE_CLIP_TOP:
		ret = DoMoveClipTOP();
		break;
	case ActionEnums::MOVE_CLIP_LAST:
		ret = DoMoveClipLast();
		break;
	case ActionEnums::FILTER_ON_SELECTED_CLIP:
		ret = DoFilterOnSelectedClip();
		break;
	case ActionEnums::PASTE_UPPER_CASE:
		ret = DoPasteUpperCase();
		break;
	case ActionEnums::PASTE_LOWER_CASE:
		ret = DoPasteLowerCase();
		break;
	case ActionEnums::PASTE_CAPITALiZE:
		ret = DoPasteCapitalize();
		break;
	case ActionEnums::PASTE_SENTENCE_CASE:
		ret = DoPasteSentenceCase();
		break;
	case ActionEnums::INVERT_CASE:
		ret = DoInvertCase();
		break;
	case ActionEnums::PASTE_REMOVE_LINE_FEEDS:
		ret = DoPasteRemoveLineFeeds();
		break;
	case ActionEnums::PASTE_ADD_ONE_LINE_FEED:
		ret = DoPastePlusAddLineFeed();
		break;
	case ActionEnums::PASTE_ADD_TWO_LINE_FEEDS:
		ret = DoPasteAddTwoLineFeeds();
		break;
	case ActionEnums::PASTE_TYPOGLYCEMIA:
		ret = DoPasteTypoglycemia();
		break;
	case ActionEnums::PASTE_ADD_CURRENT_TIME:
		ret = DoPasteAddCurrentTime();
		break;
	case ActionEnums::SEND_TO_FRIEND_1:
		ret = SendToFriendbyPos(0, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_2:
		ret = SendToFriendbyPos(1, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_3:
		ret = SendToFriendbyPos(2, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_4:
		ret = SendToFriendbyPos(3, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_5:
		ret = SendToFriendbyPos(4, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_6:
		ret = SendToFriendbyPos(5, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_7:
		ret = SendToFriendbyPos(6, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_8:
		ret = SendToFriendbyPos(7, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_9:
		ret = SendToFriendbyPos(8, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_10:
		ret = SendToFriendbyPos(9, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_11:
		ret = SendToFriendbyPos(10, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_12:
		ret = SendToFriendbyPos(11, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_13:
		ret = SendToFriendbyPos(12, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_14:
		ret = SendToFriendbyPos(13, _T(""));
		break;
	case ActionEnums::SEND_TO_FRIEND_15:
		ret = SendToFriendbyPos(14, _T(""));
		break;
	case ActionEnums::PASTE_POSITION_1:
		ret = OpenIndex(0);
		break;
	case ActionEnums::PASTE_POSITION_2:
		ret = OpenIndex(1);
		break;
	case ActionEnums::PASTE_POSITION_3:
		ret = OpenIndex(2);
		break;
	case ActionEnums::PASTE_POSITION_4:
		ret = OpenIndex(3);
		break;
	case ActionEnums::PASTE_POSITION_5:
		ret = OpenIndex(4);
		break;
	case ActionEnums::PASTE_POSITION_6:
		ret = OpenIndex(5);
		break;
	case ActionEnums::PASTE_POSITION_7:
		ret = OpenIndex(6);
		break;
	case ActionEnums::PASTE_POSITION_8:
		ret = OpenIndex(7);
		break;
	case ActionEnums::PASTE_POSITION_9:
		ret = OpenIndex(8);
		break;
	case ActionEnums::PASTE_POSITION_10:
		ret = OpenIndex(9);
		break;
	case ActionEnums::CONFIG_SHOW_FIRST_TEN_TEXT:
		ret = OnShowFirstTenText();
		break;
	case ActionEnums::CONFIG_SHOW_CLIP_WAS_PASTED:
		ret = OnShowClipWasPasted();
		break;
	case ActionEnums::TOGGLE_LAST_GROUP_TOGGLE:
		ret = OnToggleLastGroupToggle();
		break;
	case ActionEnums::MAKE_TOP_STICKY:
		ret = OnMakeTopSticky(false);
		break;
	case ActionEnums::MAKE_LAST_STICKY:
		ret = OnMakeLastSticky();
		break;
	case ActionEnums::REMOVE_STICKY:
		ret = OnRemoveStickySetting();
		break;
	case ActionEnums::GLOBAl_HOTKEYS:
		ret = OnGlobalHotkyes();
		break;
	case ActionEnums::DELETE_CLIP_DATA:
		ret = OnDeleteClipData();
		break;
	case ActionEnums::IMPORT_CLIP:
		ret = OnImportClip();
		break;
	case ActionEnums::REPLACE_TOP_STICKY_CLIP:
		ret = DoActionReplaceTopStickyClip();
		break;
	case ActionEnums::PROMPT_SEND_TO_FRIEND:
		ret = DoActionPromptSendToFriend();
		break;
	case ActionEnums::SAVE_CF_HDROP_FIlE_DATA:
		ret = DoActionSaveCF_HDROP_FileData();
		break;
	case ActionEnums::TOGGLE_CLIPBOARD_CONNECTION:
		ret = DoActionToggleClipboardConnection();
		break;
	case ActionEnums::MOVE_SELECTION_UP:
		ret = DoActionMoveSelectionUp();
		break;
	case ActionEnums::MOVE_SELECTION_DOWN:
		ret = DoActionMoveSelectionDown();
		break;
	case ActionEnums::TOGGLE_DESCRIPTION_WORD_WRAP:
		ret = DoActionToggleDescriptionWordWrap();
		break;
	case ActionEnums::APPLY_LAST_SEARCH:
		ret = DoActionApplyLastSearch();
		break;
	case ActionEnums::TOGGLE_SEARCH_METHOD:
		ret = DoActionToggleSearchMethod();
		break;
	case ActionEnums::PASTE_SCRIPT:
		ret = DoActionPasteScript(a.RefData);
		break;
	case ActionEnums::PASTE_DONT_MOVE_CLIP:
		ret = DoActionPasteDontMoveClip();
		break;
	case ActionEnums::PASTE_TRIM_WHITE_SPACE:
		ret = DoActionPasteTrimWhiteSpace();
		break;

	case ActionEnums::TRANSPARENCY_NONE:
		SetTransparency(0);
		break;
	case ActionEnums::TRANSPARENCY_5:
		SetTransparency(5);
		break;
	case ActionEnums::TRANSPARENCY_10:
		SetTransparency(10);
		break;
	case ActionEnums::TRANSPARENCY_15:
		SetTransparency(15);
		break;
	case ActionEnums::TRANSPARENCY_20:
		SetTransparency(20);
		break;
	case ActionEnums::TRANSPARENCY_25:
		SetTransparency(25);
		break;
	case ActionEnums::TRANSPARENCY_30:
		SetTransparency(30);
		break;
	case ActionEnums::TRANSPARENCY_35:
		SetTransparency(35);
		break;
	case ActionEnums::TRANSPARENCY_40:
		SetTransparency(40);
		break;
	case ActionEnums::TRANSPARENCY_TOGGLE:
		DoActionToggleTransparency();
		break;
	case ActionEnums::TRANSPARENCY_INCREASE:
		DoActionIncreaseTransparency();
		break;
	case ActionEnums::TRANSPARENCY_DECREASE:
		DoActionDecreaseTransparency();
		break;
	case ActionEnums::EMAILTO_BODY:
		DoActionEmailTo();
		break;
	case ActionEnums::GMAIL:
		DoActionGmail();
		break;
	case ActionEnums::EMAILTO_ATTACH_EXPORT:
		DoActionEmailToAttachExport();
		break;
	case ActionEnums::EMAILTO_ATTACH_CONTENT:
		DoActionEmailToAttachContent();
		break;
	case ActionEnums::SLUGIFY:
		DoActionSlugify();
		break;
	case ActionEnums::COPY_SELECTION:
		ret = DoCopySelection();
		break;
	}

	return ret;
}

bool CQPasteWnd::DoActionPasteDontMoveClip()
{
	CSpecialPasteOptions pasteOptions;
	pasteOptions.m_updateClipOrder = false;
	OpenSelection(pasteOptions);

	return true;
}

bool CQPasteWnd::DoActionPasteTrimWhiteSpace()
{
	CSpecialPasteOptions pasteOptions;
	pasteOptions.m_trimWhiteSpace = true;
	OpenSelection(pasteOptions);

	return true;
}

bool CQPasteWnd::DoActionPasteScript(CString scriptGuid)
{
	CSpecialPasteOptions pasteOptions;
	pasteOptions.m_pasteScriptGuid = scriptGuid;
	OpenSelection(pasteOptions);

	return true;
}

bool CQPasteWnd::DoActionToggleSearchMethod()
{
	if (CGetSetOptions::GetRegExTextSearch())
	{
		//if regex go back to wildcard
		CGetSetOptions::SetSimpleTextSearch(FALSE);
		CGetSetOptions::SetRegExTextSearch(FALSE);
	}
	else if (CGetSetOptions::GetSimpleTextSearch())
	{
		//if contains search go to regex
		CGetSetOptions::SetSimpleTextSearch(FALSE);
		CGetSetOptions::SetRegExTextSearch(TRUE);
	}
	else
	{
		//if wildcard to to contains
		CGetSetOptions::SetSimpleTextSearch(TRUE);
		CGetSetOptions::SetRegExTextSearch(FALSE);
	}
	return true;
}

bool CQPasteWnd::DoActionApplyLastSearch()
{
	return m_search.ApplyLastSearch();
}

bool CQPasteWnd::DoActionToggleDescriptionWordWrap()
{
	if (m_lstHeader.IsToolTipWindowVisible() == FALSE)
		return false;

	bool ret = m_lstHeader.ToggleToolTipWordWrap();
	return (ret == true);
}

bool CQPasteWnd::DoActionShowDescription()
{
	bool ret = false;

	CString csText;
	m_search.GetWindowText(csText);
	if (csText != _T(""))
	{
		m_search.AddToSearchHistory();
	}

	if (m_lstHeader.IsToolTipWindowVisible() == false)
	{
		ret = m_lstHeader.ShowFullDescription(false, false);
	}
	else
	{
		if (csText != _T(""))
		{
			m_lstHeader.DoToolTipSearch();
			ret = true;
		}
		else
		{
			m_lstHeader.HideToolTip();
			ret = true;
		}
	}

	return (ret == true);
}

bool CQPasteWnd::DoActionNextDescription()
{	
	if (m_lstHeader.IsToolTipWindowVisible() == FALSE)
		return false;

	if (g_Opt.m_bAllwaysShowDescription)
		return false;

	m_actions.m_handleRepeatKeys = true;

	ARRAY Indexes;
	m_lstHeader.GetSelectionIndexes(Indexes);

	long caret = m_lstHeader.GetCaret();

	if(Indexes.GetCount() > 1)
	{
		for (int i = 0; i < Indexes.GetCount(); i++)
		{
			int index = Indexes[i];
			if(index == caret)
			{
				if(i < Indexes.GetCount()-1)
				{
					caret = Indexes[i + 1];
					break;
				}
				else
				{
					caret = Indexes[0];
				}
			}
		}

		m_lstHeader.SetCaret(caret);
	}
	else
	{
		caret++;
		m_lstHeader.SetListPos(caret);
	}
	
	m_lstHeader.ShowFullDescription(false, true);

	return true;
}

bool CQPasteWnd::DoActionPrevDescription()
{
	if (m_lstHeader.IsToolTipWindowVisible() == FALSE)
		return false;

	if (g_Opt.m_bAllwaysShowDescription)
		return false;

	m_actions.m_handleRepeatKeys = true;

	ARRAY Indexes;
	m_lstHeader.GetSelectionIndexes(Indexes);

	long caret = m_lstHeader.GetCaret();
	
	if (Indexes.GetCount() > 1)
	{
		for (int i = ((int)Indexes.GetCount())-1; i >= 0; i--)
		{
			int index = Indexes[i];
			if (index == caret)
			{
				if (i > 0)
				{
					caret = Indexes[i - 1];
					break;
				}
				else
				{
					caret = Indexes[((int)Indexes.GetCount()) - 1];
				}
			}
		}

		m_lstHeader.SetCaret(caret);
	}
	else
	{
		caret--;
		m_lstHeader.SetListPos(caret);
	}

	m_lstHeader.ShowFullDescription(false, true);

	return true;
}

bool CQPasteWnd::DoActionShowMenu()
{
	ShowRightClickMenu();

	return true;
}

bool CQPasteWnd::DoActionNewGroup()
{
	NewGroup(false, theApp.GetValidGroupID());

	return true;
}

bool CQPasteWnd::DoActionNewGroupSelection()
{
	NewGroup(true, theApp.GetValidGroupID());

	return true;
}

bool CQPasteWnd::DoActionToggleFileLogging()
{
	if (CGetSetOptions::m_bEnableDebugLogging)
	{
		Log(_T("turning file logging OFF"));
	}

	CGetSetOptions::m_bEnableDebugLogging = !CGetSetOptions::m_bEnableDebugLogging;

	if (CGetSetOptions::m_bEnableDebugLogging)
	{
		Log(_T("turning file logging ON"));
	}

	return true;
}

bool CQPasteWnd::DoActionToggleOutputDebugString()
{
	if (CGetSetOptions::m_bEnableDebugLogging)
	{
		Log(_T("turning DebugString logging OFF"));
	}

	CGetSetOptions::m_bOutputDebugString = !CGetSetOptions::m_bOutputDebugString;

	if (CGetSetOptions::m_bEnableDebugLogging)
	{
		Log(_T("turning DebugString logging ON"));
	}

	return true;
}

bool CQPasteWnd::DoActionForceCloseWindow()
{
	Log(_T("Force closing window from hot keys"));
	HideQPasteWindow(true);

	return true;
}

bool CQPasteWnd::DoActionCloseWindow()
{
	bool ret = false;
	if (m_bModifersMoveActive)
	{
		Log(_T("Escape key hit setting modifers to NOT active"));
		m_bModifersMoveActive = false;
		ret = true;
	}
	else
	{
		Log(_T("close 1"));

		if (m_lstHeader.IsToolTipShowPersistant() == false &&
			m_lstHeader.IsToolTipWindowVisible())
		{
			m_lstHeader.HidePopup(true);
			Log(_T("close 2"));
		}
		else if (m_strSQLSearch.IsEmpty() == FALSE)
		{
			OnCancelFilter(0, 0);
			Log(_T("close 3"));
			ret = true;
		}
		else
		{
			if (g_Opt.GetShowPersistent() && this->GetMinimized() == false)
			{
				MinMaxWindow(FORCE_MIN);
				theApp.m_activeWnd.ReleaseFocus();

				Log(_T("close 4"));
			}
			else
			{
				if (m_GroupTree.IsWindowVisible() == FALSE)
				{
					HideQPasteWindow(true);
					ret = true;
					Log(_T("close 5"));
				}
			}
		}
	}

	Log(_T("close 6"));

	return ret;
}

bool CQPasteWnd::DoActionNextTabControl()
{
	BOOL bPrev = FALSE;

	CWnd *pFocus = GetFocus();
	if (pFocus)
	{
		CWnd *pNextWnd = GetNextDlgTabItem(pFocus, bPrev);
		if (pNextWnd)
		{
			pNextWnd->SetFocus();
		}
	}
	return true;
}

bool CQPasteWnd::DoActionPrevTabControl()
{
	BOOL bPrev = TRUE;

	CWnd *pFocus = GetFocus();
	if (pFocus)
	{
		CWnd *pNextWnd = GetNextDlgTabItem(pFocus, bPrev);
		if (pNextWnd)
		{
			pNextWnd->SetFocus();
		}
	}
	return true;
}

bool CQPasteWnd::DoActionShowGroups()
{
	OnShowGroupsTop();

	return true;
}

bool CQPasteWnd::DoActionNewClip()
{
	CClipIDs IDs;
	IDs.Add(-1);
	theApp.EditItems(IDs, true);

	HideQPasteWindow(false);
	return true;
}

bool CQPasteWnd::DoActionEditClip()
{
	if(m_lstHeader.GetSelectedCount() == 0)
	{
		return false;
	}

	CClipIDs IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	theApp.EditItems(IDs, true);

	HideQPasteWindow(false);
	return true;
}

bool CQPasteWnd::DoActionMoveSelectionUp()
{
	MoveSelection(false, false);
	m_actions.m_handleRepeatKeys = true;
	return true;
}

bool CQPasteWnd::DoActionMoveSelectionDown()
{
	MoveSelection(true, false);
	m_actions.m_handleRepeatKeys = true;
	return true;
}

bool CQPasteWnd::DoModifierActiveActionSelectionUp()
{
	if (m_bModifersMoveActive)
	{
		MoveSelection(false, true);
		m_modifierKeyActions.m_handleRepeatKeys = true;
		return true;
	}

	return false;
}

bool CQPasteWnd::DoModifierActiveActionSelectionDown()
{
	if (m_bModifersMoveActive)
	{
		MoveSelection(true, true);
		m_modifierKeyActions.m_handleRepeatKeys = true;
		return true;
	}

	return false;
}

bool CQPasteWnd::DoModifierActiveActionMoveFirst()
{
	if (m_bModifersMoveActive)
	{
		m_lstHeader.SetListPos(0);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoModifierActiveActionMoveLast()
{
	if (m_bModifersMoveActive)
	{
		if (m_lstHeader.GetItemCount() > 0)
		{
			m_lstHeader.SetListPos(m_lstHeader.GetItemCount() - 1);
		}
		return true;
	}

	return false;
}

bool CQPasteWnd::DoActionCancelFilter()
{
	FillList();

	m_bHandleSearchTextChange = false;
	m_search.SetWindowText(_T(""));
	m_bHandleSearchTextChange = true;

	MoveControls();

	m_lstHeader.SetFocus();

	return true;
}

bool CQPasteWnd::DoActionHomeList()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		theApp.EnterGroupID(-1); // History
		return true;
	}

	return false;
}

bool CQPasteWnd::DoActionBackGroup()
{
	if(::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		theApp.EnterGroupID(theApp.m_GroupParentID);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoActionToggleShowPersistant()
{
	if (m_lstHeader.IsToolTipWindowVisible())
	{
		m_lstHeader.ToggleToolTipShowPersistant();
	}
	else
	{
		theApp.ShowPersistent(!g_Opt.m_bShowPersistent);
		if (g_Opt.m_bShowPersistent)
		{
			::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
		}

		MoveControls();

		UpdateStatus();
	}
	return true;
}

bool CQPasteWnd::DoActionPasteSelected()
{
	CSpecialPasteOptions pasteOptions;
	OpenSelection(pasteOptions);
	return true;	
}

bool CQPasteWnd::DoActionDeleteSelected()
{
	if(::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		DeleteSelectedRows();
		return true;
	}

	return false;
}

bool CQPasteWnd::DoActionClipProperties()
{
	bool ret = false;

	if(::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		m_bHideWnd = false;

		ARRAY IDs, Indexes;
		m_lstHeader.GetSelectionItemData(IDs);
		m_lstHeader.GetSelectionIndexes(Indexes);

		INT_PTR size = IDs.GetSize();
		if(size < 1)
		{
			return ret;
		}

		int id = IDs[0];
		int row = Indexes[0];

		if(id < 0)
		{
			return ret;
		}

		m_lstHeader.RemoveAllSelection();
		m_lstHeader.SetSelection(row);

		ret = ShowProperties(id, row);

		m_lstHeader.SetListPos(row);
	}

	return false;
}

bool CQPasteWnd::ShowProperties(int id, int row)
{
	if (id < 0)
	{
		return false;
	}

	m_bHideWnd = false;	

	CDimWnd dimmer(this);

	CCopyProperties props(id, this);
	INT_PTR doModalRet = props.DoModal();

	if (doModalRet == IDOK)
	{
		{
			ATL::CCritSecLock csLock(m_CritSection.m_sect);

			if(row < 0)
			{
				bool selectedItem = false;
				int index = 0;
				std::vector<CMainTable>::iterator iter = m_listItems.begin();
				while (iter != m_listItems.end())
				{
					if (iter->m_lID == id)
					{
						row = index;
						break;
					}
					iter++;
					index++;
				}
			}

			if (row >= 0 &&
				row < (int) m_listItems.size())
			{
				CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT * FROM Main WHERE lID = %d"), id);
				if (!q.eof())
				{
					FillMainTable(m_listItems[row], q);
				}

				RemoveFromImageRtfCache(row);
			}
		}

		m_extraDataThread.FireLoadAccelerators();

		m_lstHeader.RefreshVisibleRows();

		if (props.m_lGroupChangedTo >= 0)
		{
			CSpecialPasteOptions pasteOptions;
			OpenID(props.m_lGroupChangedTo, pasteOptions);
		}

		m_lstHeader.SetFocus();		
	}

	m_bHideWnd = true;
	return true;
}

bool CQPasteWnd::DoActionPasteSelectedPlainText()
{
	CSpecialPasteOptions pasteOptions;
	pasteOptions.m_pasteAsPlainText = true;			
	OpenSelection(pasteOptions);
	return true;
}

bool CQPasteWnd::DoActionMoveClipToGroup()
{
	if(::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		m_bHideWnd = false;

		CDimWnd dimmer(this);

		CMoveToGroupDlg dlg;

		INT_PTR nRet = dlg.DoModal();
		if(nRet == IDOK)
		{
			int nGroup = dlg.GetSelectedGroup();

			Log(StrF(_T("Move to Group, GroupId: %d"), nGroup));

			if(nGroup >= -1)
			{
				CClipIDs IDs;
				m_lstHeader.GetSelectionItemData(IDs);

				IDs.MoveTo(nGroup);
			}
			FillList();
		}

		m_bHideWnd = true;
		return true;
	}

	return false;
}

bool CQPasteWnd::DoActionElevatePrivleges()
{
	g_Opt.SetPasteAsAdmin(!g_Opt.GetPasteAsAdmin());

	return true;
}

bool CQPasteWnd::DoShowInTaskBar()
{
	g_Opt.SetShowInTaskBar(!g_Opt.GetShowInTaskBar());

	theApp.RefreshShowInTaskBar();

	return true;
}

bool CQPasteWnd::DoClipCompare()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if(IDs.GetCount() > 1)
	{
		if(!g_Opt.m_bShowPersistent)
		{
			HideQPasteWindow(false, false);
		}
		else if(g_Opt.GetAutoHide())
		{
			MinMaxWindow(FORCE_MIN);
		}

		CClipCompare compare;
		compare.Compare(IDs[0], IDs[1]);
			
		return true;
	}
	else
	{
		Log(StrF(_T("DoClipCompare, at least 2 clips need to be selected, count: %d"), IDs.GetCount()));
	}

	return false;
}

bool CQPasteWnd::DoSelectLeftSideCompare()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if(IDs.GetCount() > 0)
	{
		m_leftSelectedCompareId = IDs[0];

		return true;
	}
	else
	{
		Log(StrF(_T("DoSelectLeftSideCompare, no selected clip, not assigning left side")));
	}

	return false;
}

bool CQPasteWnd::DoSelectRightSideAndDoCompare()
{
	if(m_leftSelectedCompareId > 0)
	{
		ARRAY IDs;
		m_lstHeader.GetSelectionItemData(IDs);

		if(IDs.GetCount() > 0)
		{
			int rightId = IDs[0];

			if(!g_Opt.m_bShowPersistent)
			{
				HideQPasteWindow(false, false);
			}
			else if(g_Opt.GetAutoHide())
			{
				MinMaxWindow(FORCE_MIN);
			}

			CClipCompare compare;
			compare.Compare(m_leftSelectedCompareId, rightId);

			return true;
		}
		else
		{
			Log(StrF(_T("DoSelectRightSideAndDoCompare, no selected clips")));
		}
	}
	else
	{
		Log(StrF(_T("DoSelectRightSideAndDoCompare, no left side selected, select left side first")));
	}

	return false;
}

bool CQPasteWnd::DoExportToTextFile()
{
	bool ret = false;

	CClipIDs IDs;
	INT_PTR lCount = m_lstHeader.GetSelectedCount();
	if(lCount <= 0)
	{
		return ret;
	}

	m_lstHeader.GetSelectionItemData(IDs);
	lCount = IDs.GetSize();
	if(lCount <= 0)
	{
		return ret;
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
	ofn.lpstrFilter = _T("Exported Ditto Clips (.txt)\0*.txt\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = szDir;
	ofn.lpstrDefExt = _T("txt");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	m_bHideWnd = false;

	if(GetSaveFileName(&ofn))
	{
		using namespace nsPath;
		CString startingFilePath = ofn.lpstrFile;
		CPath path(ofn.lpstrFile);
		CString csPath = path.GetPath();
		CString csExt = path.GetExtension();
		path.RemoveExtension();
		CString csFileName = path.GetName();

		CGetSetOptions::SetLastExportDir(csPath);

		for(int i = 0; i < IDs.GetCount(); i++)
		{
			int id = IDs[i];

			CClip clip;
			if(clip.LoadFormats(id, true))
			{	
				CString savePath = startingFilePath;
				if(IDs.GetCount() > 1 ||
					FileExists(startingFilePath))
				{
					savePath = _T("");

					for(int y = 1; y < 1001; y++)
					{			
						CString testFilePath;
						testFilePath.Format(_T("%s%s_%d.%s"), csPath, csFileName, y, csExt);
						if(FileExists(testFilePath) == FALSE)
						{				
							savePath = testFilePath;
							break;
						}
					}
				}

				if(savePath != _T(""))
				{
					clip.WriteTextToFile(savePath, true, true, false);

					ret = true;
				}
				else
				{
					Log(StrF(_T("Failed to find a valid file name for starting path: %s"), startingFilePath));
				}
			}
		}		
	}

	m_bHideWnd = true;

	return ret;
}

bool CQPasteWnd::DoExportToQRCode()
{
	bool ret = false;

	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if(IDs.GetCount() > 0)
	{
		int id = IDs[0];
		CClip clip;
		if(clip.LoadMainTable(id))
		{
			if(clip.LoadFormats(id, true))
			{
				CString clipText = clip.GetUnicodeTextFormat();				

				CCreateQRCodeImage p;
				int imageSize = 0;
				unsigned char* bitmapData = p.CreateImage(clipText, imageSize);

				if(bitmapData != NULL)
				{
					QRCodeViewer *viewer = new QRCodeViewer();

					LOGFONT lf;
					CGetSetOptions::GetFont(lf);

					viewer->CreateEx(this, bitmapData, imageSize, clip.Description(), m_lstHeader.GetRowHeight(), lf);
					viewer->ShowWindow(SW_SHOW);

					ret = true;
				}
			}
		}
	}	

	return ret;
}

bool CQPasteWnd::DoExportToGoogleTranslate()
{
	bool ret = false;

	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		int id = IDs[0];
		CClip clip;
		if (clip.LoadMainTable(id))
		{
			if (clip.LoadFormats(id, true))
			{
				CString clipText = clip.GetUnicodeTextFormat();
				if(clipText == _T(""))
				{
					CStringA aText = clip.GetCFTextTextFormat();
					if(aText != _T(""))
					{
						clipText = CTextConvert::MultiByteToUnicodeString(aText);
					}
				}

				if (clipText != _T(""))
				{
					CString clipTextUrlEncoded = InternetEncode(clipText);

					CString url;
					url.Format(CGetSetOptions::GetTranslateUrl(), clipTextUrlEncoded);

					if(!g_Opt.m_bShowPersistent)
					{
						HideQPasteWindow(false, false);
					}
					else if(g_Opt.GetAutoHide())
					{
						MinMaxWindow(FORCE_MIN);
					}

					CHyperLink::GotoURL(url, SW_SHOW);

					ret = true;
				}
			}
		}
	}

	return true;
} 

bool CQPasteWnd::DoSaveCurrentClipboard()
{
	theApp.m_pMainFrame->PostMessage(WM_SAVE_CLIPBOARD, 0, 0);

	return true;
}

bool CQPasteWnd::DoMoveClipDown()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		bool sort = false;
		for (int i = ((int)IDs.GetCount()) - 1; i >= 0; i--)
		{
			int id = IDs[i];
			CClip clip;
			if (clip.LoadMainTable(id))
			{
				clip.MoveDown(theApp.m_GroupID);
				clip.ModifyMainTable();

				sort = SyncClipDataToArrayData(clip);
			}
		}
		
		if (sort)
		{
			if (theApp.m_GroupID > 0)
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
			}
			else
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
			}

			SelectIds(IDs);

			m_lstHeader.RefreshVisibleRows();
			m_lstHeader.RedrawWindow();
		}
	}

	return true;
}

bool CQPasteWnd::DoMoveClipUp()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		bool sort = false;
		for (int i = 0; i < IDs.GetCount(); i++)
		{
			int id = IDs[i];
			CClip clip;
			if (clip.LoadMainTable(id))
			{
				clip.MoveUp(theApp.m_GroupID);
				clip.ModifyMainTable();

				sort = SyncClipDataToArrayData(clip);
			}
		}
		
		if (sort)
		{
			if (theApp.m_GroupID > 0)
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
			}
			else
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
			}

			SelectIds(IDs);

			m_lstHeader.RefreshVisibleRows();
			m_lstHeader.RedrawWindow();
		}
	}

	return true;
}

bool CQPasteWnd::DoMoveClipLast()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		bool sort = false;
		for (int i = 0; i < IDs.GetCount(); i++)
		{
			int id = IDs[i];
			CClip clip;
			if (clip.LoadMainTable(id))
			{
				if (theApp.m_GroupID > 0)
				{
					clip.MakeLastGroupOrder();
				}
				else
				{
					clip.MakeLastOrder();
				}
				clip.ModifyMainTable();

				//have we loaded all clips, if so then sort and select
				if (m_listItems.size() == m_lstHeader.GetItemCount())
				{
					sort = SyncClipDataToArrayData(clip);
				}
				else
				{
					//haven't loaded all clips so this will be out of what we have loaded
					//remove this from the list, will be shown as they scroll down the list
					std::vector<CMainTable>::iterator iter = m_listItems.begin();
					while (iter != m_listItems.end())
					{
						if (iter->m_lID == clip.ID())
						{
							m_listItems.erase(iter);
							break;
						}
						iter++;
					}
				}
			}
		}

		if (sort)
		{
			if (theApp.m_GroupID > 0)
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
			}
			else
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
			}

			SelectIds(IDs);			
		}

		m_lstHeader.RefreshVisibleRows();
		m_lstHeader.RedrawWindow();
	}

	return true;
}

bool CQPasteWnd::DoMoveClipTOP()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		bool sort = false;
		for (int i = 0; i < IDs.GetCount(); i++)
		{
			int id = IDs[i];
			CClip clip;
			if (clip.LoadMainTable(id))
			{
				if (theApp.m_GroupID > 0)
				{
					clip.MakeLatestGroupOrder();
				}
				else
				{
					clip.MakeLatestOrder();
				}
				clip.ModifyMainTable();

				sort = SyncClipDataToArrayData(clip);
			}
		}
		
		if (sort)
		{
			if (theApp.m_GroupID > 0)
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
			}
			else
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
			}
			
			SelectIds(IDs);

			m_lstHeader.RefreshVisibleRows();
			m_lstHeader.RedrawWindow();
		}
	}

	return true;
}

bool CQPasteWnd::DoFilterOnSelectedClip()
{
	bool ret = false;
	ARRAY IDs, Indexes;
	m_lstHeader.GetSelectionItemData(IDs);

	INT_PTR size = IDs.GetSize();
	if (size > 0)
	{
		int id = IDs[0];

		ATL::CCritSecLock csLock(m_CritSection.m_sect);
		std::vector<CMainTable>::iterator iter = m_listItems.begin();
		while (iter != m_listItems.end())
		{
			if (iter->m_lID == id)
			{
				m_bHandleSearchTextChange = false;
				m_search.SetWindowText(iter->m_Desc);
				m_bHandleSearchTextChange = true;
				OnSearch(0, 0);				
				ret = true;
				break;
			}
			iter++;
		}
	}

	return ret;
}

bool CQPasteWnd::DoPasteUpperCase()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteUpperCase = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPasteLowerCase()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteLowerCase = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPasteCapitalize()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteCapitalize = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPasteSentenceCase()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteSentenceCase = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoInvertCase()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_invertCase = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPasteRemoveLineFeeds()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteRemoveLineFeeds = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPastePlusAddLineFeed()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteAddOneLineFeed = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPasteAddTwoLineFeeds()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteAddTwoLineFeeds = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPasteTypoglycemia()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteTypoglycemia = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::DoPasteAddCurrentTime()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteAddingDateTime = true;
		OpenSelection(pasteOptions);
		return true;
	}

	return false;
}

bool CQPasteWnd::OnShowFirstTenText()
{
	CGetSetOptions::SetShowTextForFirstTenHotKeys(!CGetSetOptions::GetShowTextForFirstTenHotKeys());
	m_lstHeader.SetShowTextForFirstTenHotKeys(CGetSetOptions::GetShowTextForFirstTenHotKeys());

	m_lstHeader.RefreshVisibleRows();
	m_lstHeader.RedrawWindow();
	return true;
}

bool CQPasteWnd::OnShowClipWasPasted()
{
	CGetSetOptions::SetShowIfClipWasPasted(!CGetSetOptions::GetShowIfClipWasPasted());
	m_lstHeader.SetShowIfClipWasPasted(CGetSetOptions::GetShowIfClipWasPasted());

	m_lstHeader.RefreshVisibleRows();
	m_lstHeader.RedrawWindow();
	return true;
}

bool CQPasteWnd::OnToggleLastGroupToggle()
{
	int newGroupId = -2;
	if (theApp.m_GroupID > 0)
	{
		CGetSetOptions::SetLastGroupToggle(theApp.m_GroupID);
		newGroupId = -1;		
	}
	else
	{
		newGroupId = CGetSetOptions::GetLastGroupToggle();
	}

	if (newGroupId >= -1)
	{
		theApp.EnterGroupID(newGroupId);
	}

	return true;
}

bool CQPasteWnd::OnMakeTopSticky(bool forceSort)
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		bool sort = forceSort;
		for (int i = ((int)IDs.GetCount()) - 1; i >= 0; i--)
		{
			int id = IDs[i];
			CClip clip;
			if (clip.LoadMainTable(id))
			{
				clip.MakeStickyTop(theApp.m_GroupID);
				clip.ModifyMainTable();

				sort = SyncClipDataToArrayData(clip);
			}
		}

		if (sort)
		{
			if (theApp.m_GroupID > 0)
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
			}
			else
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
			}

			SelectIds(IDs);

			m_lstHeader.RefreshVisibleRows();
			m_lstHeader.RedrawWindow();
		}
	}

	return true;
}

bool CQPasteWnd::OnMakeLastSticky()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		bool sort = false;
		for (int i = ((int)IDs.GetCount()) - 1; i >= 0; i--)
		{
			int id = IDs[i];
			CClip clip;
			if (clip.LoadMainTable(id))
			{
				clip.MakeStickyLast(theApp.m_GroupID);
				clip.ModifyMainTable();

				sort = SyncClipDataToArrayData(clip);
			}
		}

		if (sort)
		{
			if (theApp.m_GroupID > 0)
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
			}
			else
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
			}

			SelectIds(IDs);

			m_lstHeader.RefreshVisibleRows();
			m_lstHeader.RedrawWindow();
		}
	}

	return true;
}

bool CQPasteWnd::OnRemoveStickySetting()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	if (IDs.GetCount() > 0)
	{
		bool sort = false;
		for (int i = ((int)IDs.GetCount()) - 1; i >= 0; i--)
		{
			RemoveStickyInternal(IDs[i], sort);
		}

		//theApp.m_FocusID = id;

		if (sort)
		{
			if (theApp.m_GroupID > 0)
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::GroupSortDesc);
			}
			else
			{
				std::sort(m_listItems.begin(), m_listItems.end(), CMainTable::SortDesc);
			}

			//SelectFocusID();

			m_lstHeader.RefreshVisibleRows();
			m_lstHeader.RedrawWindow();
		}
	}

	return true;
}

void CQPasteWnd::RemoveStickyInternal(int id, bool &sort)
{
	CClip clip;
	if (clip.LoadMainTable(id))
	{
		if (clip.RemoveStickySetting(theApp.m_GroupID))
		{
			clip.ModifyMainTable();

			std::vector<CMainTable>::iterator iter = m_listItems.begin();
			while (iter != m_listItems.end())
			{
				if (iter->m_lID == id)
				{
					if (theApp.m_GroupID > 0)
					{
						iter->m_stickyClipGroupOrder = clip.m_stickyClipGroupOrder;
					}
					else
					{
						iter->m_stickyClipOrder = clip.m_stickyClipOrder;
					}
					sort = true;
					break;
				}
				iter++;
			}
		}
	}
}

bool CQPasteWnd::OnNewClip()
{
	CWnd *pWnd = AfxGetMainWnd();
	if (pWnd != NULL)
	{
		pWnd->SendMessage(WM_COMMAND, ID_FIRST_NEWCLIP, 0);
	}

	return true;
}

bool CQPasteWnd::OnImportClip()
{
	CWnd *pWnd = AfxGetMainWnd();
	if (pWnd != NULL)
	{
		pWnd->SendMessage(WM_COMMAND, ID_FIRST_IMPORT, 0);
	}

	return true;
}

bool CQPasteWnd::DoActionReplaceTopStickyClip()
{
	ARRAY IDs;
	IDs.Add(m_lstHeader.GetItemData(0));

	if (IDs.GetCount() > 0)
	{
		bool sort = false;
		for (int i = ((int)IDs.GetCount()) - 1; i >= 0; i--)
		{
			RemoveStickyInternal(IDs[i], sort);
		}

		OnMakeTopSticky(true);
	}

	return true;
}

bool CQPasteWnd::DoActionSaveCF_HDROP_FileData()
{
	if (m_lstHeader.GetSelectedCount() == 0)
	{
		return false;
	}

	CWaitCursor wait;

	CString errorMessage;

	CClipIDs IDs;
	m_lstHeader.GetSelectionItemData(IDs);
	ARRAY Indexs;
	m_lstHeader.GetSelectionIndexes(Indexs);

	if (IDs.GetCount() > 0)
	{
		for(int i = 0; i < min(Indexs.GetCount(), IDs.GetCount()); i++)
		{
			int row = Indexs[i];
			int id = IDs[i];
			CClip clip;
			if (clip.LoadMainTable(id))
			{
				if (clip.LoadFormats(id))
				{
					CString localErrorMessage;
					if (clip.AddFileDataToData(localErrorMessage))
					{
						if (row >= 0 &&
							row < (int)m_listItems.size())
						{
							CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT * FROM Main WHERE lID = %d"), id);
							if (!q.eof())
							{
								FillMainTable(m_listItems[row], q);
							}
						}
					}
					
					errorMessage += localErrorMessage;
				}
			}
		}
	}

	if (errorMessage.GetLength() > 0)
	{
		m_popupMsg.Show(errorMessage, CPoint(0, 0), true);
		SetTimer(TIMER_ERROR_MSG, CGetSetOptions::GetErrorMsgPopupTimeout(), NULL);
	}

	m_lstHeader.RefreshVisibleRows();

	return true;
}

bool CQPasteWnd::DoActionPromptSendToFriend()
{
	m_bHideWnd = false;

	CDimWnd dimmer(this);

	CFriendPromptDlg dlg(this);
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.GetClearList())
		{
			m_customFriendsHelper.ClearList();
		}
		else
		{
			CString name = dlg.GetName();
			if (name != _T(""))
			{
				if (dlg.GetSave())
				{
					m_customFriendsHelper.Add(name, dlg.GetDesc());
				}
				SendToFriendbyPos(0, name);
			}
		}
	}

	m_bHideWnd = true;

	return true;
}

bool CQPasteWnd::DoActionToggleClipboardConnection()
{
	theApp.ToggleConnectCV();
	UpdateStatus();

	return true;
}

bool CQPasteWnd::OnDeleteClipData()
{
	CWnd *pWnd = AfxGetMainWnd();
	if (pWnd != NULL)
	{
		pWnd->SendMessage(WM_COMMAND, ID_FIRST_DELETECLIPDATA, 0);
	}

	return true;
}

bool CQPasteWnd::OnGlobalHotkyes()
{
	CWnd *pWnd = AfxGetMainWnd();
	if (pWnd != NULL)
	{
		pWnd->SendMessage(WM_COMMAND, ID_FIRST_GLOBALHOTKEYS, 0);
	}

	return true;
}

bool CQPasteWnd::DoExportToBitMapFile()
{
	bool ret = false;

	CClipIDs IDs;
	INT_PTR lCount = m_lstHeader.GetSelectedCount();
	if (lCount <= 0)
	{
		return ret;
	}

	m_lstHeader.GetSelectionItemData(IDs);
	lCount = IDs.GetSize();
	if (lCount <= 0)
	{
		return ret;
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
	ofn.lpstrFilter = _T("PNG (*.png)\0*.png\0BMP (*.bmp)\0*.bmp\0JPEG (*.jpeg)\0*.jpeg");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = szDir;
	ofn.lpstrDefExt = _T("png");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_NOCHANGEDIR;

	m_bHideWnd = false;
	
	if (GetSaveFileName(&ofn))
	{
		CWaitCursor wait;

		using namespace nsPath;
		CString startingFilePath = ofn.lpstrFile;
		CPath path(ofn.lpstrFile);
		CString csPath = path.GetPath();
		CString csExt = path.GetExtension();
		path.RemoveExtension();
		CString csFileName = path.GetName();

		CGetSetOptions::SetLastExportDir(csPath);

		for (int i = 0; i < IDs.GetCount(); i++)
		{
			int id = IDs[i];

			CClip toSave;
			toSave.LoadFormats(id);

			CClipFormat *png = NULL;
			CClipFormat *bitmap = toSave.m_Formats.FindFormat(CF_DIB);
			if (bitmap == NULL)
			{
				png = toSave.m_Formats.FindFormat(theApp.m_PNG_Format);
			}

			if (bitmap != NULL ||
				png != NULL)
			{
				CString savePath = startingFilePath;
				if (IDs.GetCount() > 1 ||
					FileExists(startingFilePath))
				{
					savePath = _T("");

					for (int y = 1; y < 1001; y++)
					{
						CString testFilePath;
						testFilePath.Format(_T("%s%s_%d.%s"), csPath, csFileName, y, csExt);
						if (FileExists(testFilePath) == FALSE)
						{
							savePath = testFilePath;
							break;
						}
					}
				}

				if (savePath != _T(""))
				{
					toSave.WriteImageToFile(savePath);

					ret = true;
				}
				else
				{
					Log(StrF(_T("Failed to find a valid file name for starting path: %s"), startingFilePath));
				}
			}
		}
	}

	m_bHideWnd = true;

	return ret;
}

LRESULT CQPasteWnd::OnCancelFilter(WPARAM wParam, LPARAM lParam)
{
	this->DoAction(ActionEnums::CANCELFILTER);
	return 1;
}

LRESULT CQPasteWnd::OnPostOptions(WPARAM wParam, LPARAM lParam)
{
	UpdateFont();
	LoadShortcuts();

	m_lstHeader.SetShowTextForFirstTenHotKeys(CGetSetOptions::GetShowTextForFirstTenHotKeys());
	m_lstHeader.SetShowIfClipWasPasted(CGetSetOptions::GetShowIfClipWasPasted());
	m_lstHeader.SetNumberOfLinesPerRow(CGetSetOptions::GetLinesPerRow(), true);

	SetCurrentTransparency();

	if (g_Opt.m_tooltipTimeout > 0 ||
		g_Opt.m_tooltipTimeout == -1)
	{
		m_lstHeader.EnableToolTips();
	}
	else
	{
		m_lstHeader.EnableToolTips(FALSE);
	}

	return 1;
}

void CQPasteWnd::OnClose()
{
    HideQPasteWindow(true);
}

void CQPasteWnd::OnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;
    CProcessPaste paste;
	paste.m_pastedFromGroup = (theApp.m_GroupID > 0);

	if (CONTROL_PRESSED)
	{
		paste.m_pasteOptions.m_dragDropFilesOnly = true;
	}

    CClipIDs &clips = paste.GetClipIDs();

    m_lstHeader.GetSelectionItemData(clips);

    if(clips.GetSize() <= 0)
    {
        ASSERT(0); // does this ever happen ??
        clips.Add(m_lstHeader.GetItemData(pLV->iItem));
    }

	this->SetTimer(TIMER_DRAG_HIDE_WINDOW, 500, NULL);
    
	paste.DoDrag();

	KillTimer(TIMER_DRAG_HIDE_WINDOW);

    *pResult = 0;
}

DROPEFFECT CQPasteWnd::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	int k = 0;

	return DROPEFFECT_COPY;
}

DROPEFFECT CQPasteWnd::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return DROPEFFECT_COPY;
}

BOOL CQPasteWnd::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	return TRUE;
}

void CQPasteWnd::OnDragLeave()
{

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
					if((int)m_listItems.size() > pItem->iItem &&
						m_listItems[pItem->iItem].m_lID > 0)
                    {
                        CString cs;
                        if(m_listItems[pItem->iItem].m_bDontAutoDelete)
                        {
							cs += _T("<noautodelete>");
                        }

                        if(m_listItems[pItem->iItem].m_bHasShortCut)
                        {
							cs += _T("<shortcut>");
                        }

                        if(m_listItems[pItem->iItem].m_bIsGroup)
                        {
							cs += _T("<group>");
                        }

						if (theApp.m_GroupID > 0)
						{
							if (m_listItems[pItem->iItem].m_stickyClipGroupOrder != INVALID_STICKY)
							{
								cs += _T("<sticky>");
							}
						}
						else
						{
							if (m_listItems[pItem->iItem].m_stickyClipOrder != INVALID_STICKY)
							{
								cs += _T("<sticky>");
							}
						}

                        // attached to a group
                        if(m_listItems[pItem->iItem].m_bHasParent)
                        {
							cs += _T("<ingroup>");
                        }

                        if(m_listItems[pItem->iItem].m_QuickPaste.IsEmpty() == FALSE)
                        {
							cs += _T("<qpastetext>");
                        }

						if (m_listItems[pItem->iItem].m_dateCopied != m_listItems[pItem->iItem].m_datePasted)
						{
							cs += "<pasted>";
						}

                        // pipe is the "end of symbols" marker
                        cs += "|" + CMainTableFunctions::GetDisplayText(g_Opt.m_nLinesPerRow, m_listItems[pItem->iItem].m_Desc);

                        lstrcpyn(pItem->pszText, cs, pItem->cchTextMax);
                        pItem->pszText[pItem->cchTextMax - 1] = '\0';

//						Log(StrF(_T("DrawItem index %d - "), pItem->iItem));//, pItem->pszText));
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

                        if(addToLoadItems)
                        {
                            CPoint loadItem(pItem->iItem, (m_lstHeader.GetTopIndex() + (m_lstHeader.GetCountPerPage() * 2)));

							//Log(StrF(_T("DrawItem index %d, add: %d"), loadItem.x, loadItem.y));
                            m_loadItems.push_back(loadItem);
                        }

                        m_thread.FireLoadItems(false);
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

                    if((int)m_listItems.size() > pItem->iItem)
                    {
                        pItem->lParam = m_listItems[pItem->iItem].m_lID;
                    }
                }

                break;
        }
    }

    if(pItem->mask & LVIF_CF_DIB && g_Opt.m_bDrawThumbnail)
    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);
     
		if((int)m_listItems.size() > pItem->iItem)
        {
			CF_NoDibTypeMap::iterator iterNoDib = m_cf_NO_dibCache.find(m_listItems[pItem->iItem].m_lID);
			if (iterNoDib == m_cf_NO_dibCache.end())
			{
				CF_DibTypeMap::iterator iterDib = m_cf_dibCache.find(m_listItems[pItem->iItem].m_lID);
				if (iterDib == m_cf_dibCache.end())
				{
					bool exists = false;
					for (std::list<CClipFormatQListCtrl>::iterator it = m_ExtraDataLoadItems.begin(); it != m_ExtraDataLoadItems.end(); it++)
					{
						if (it->m_cfType == CF_DIB && it->m_parentId == m_listItems[pItem->iItem].m_lID)
						{
							exists = true;
							break;
						}
					}

					if (exists == false)
					{
						CClipFormatQListCtrl format;
						format.m_cfType = CF_DIB;
						format.m_parentId = m_listItems[pItem->iItem].m_lID;
						format.m_clipRow = pItem->iItem;
						format.m_autoDeleteData = true;
						format.m_counter = m_extraDataCounter++;
						m_ExtraDataLoadItems.push_back(format);

						m_extraDataThread.FireLoadExtraData(m_lstHeader.GetRowHeight());
					}
				}
				else
				{
					if (iterDib->second.m_hgData != NULL)
					{
						pItem->lParam = (LPARAM) &(iterDib->second);
					}
				}
			}
        }
    }

    if(pItem->mask & LVIF_CF_RICHTEXT && g_Opt.m_bDrawRTF)
    {
		ATL::CCritSecLock csLock(m_CritSection.m_sect);

        if((int)m_listItems.size() > pItem->iItem)
        {
			CF_NoDibTypeMap::iterator iterNoRtf = m_cf_NO_rtfCache.find(m_listItems[pItem->iItem].m_lID);
			if (iterNoRtf == m_cf_NO_rtfCache.end())
			{
				CF_DibTypeMap::iterator iterRTF = m_cf_rtfCache.find(m_listItems[pItem->iItem].m_lID);
				if (iterRTF == m_cf_rtfCache.end())
				{
					bool exists = false;
					for (std::list<CClipFormatQListCtrl>::iterator it = m_ExtraDataLoadItems.begin(); it != m_ExtraDataLoadItems.end(); it++)
					{
						if (it->m_cfType == theApp.m_RTFFormat && it->m_parentId == m_listItems[pItem->iItem].m_lID)
						{
							exists = true;
							break;
						}
					}

					if (exists == false)
					{
						CClipFormatQListCtrl format;
						format.m_cfType = theApp.m_RTFFormat;
						format.m_parentId = m_listItems[pItem->iItem].m_lID;
						format.m_clipRow = pItem->iItem;
						format.m_autoDeleteData = true;
						format.m_counter = m_extraDataCounter++;
						m_ExtraDataLoadItems.push_back(format);

						m_extraDataThread.FireLoadExtraData(m_lstHeader.GetRowHeight());
					}
				}
				else
				{
					if (iterRTF->second.m_hgData != NULL)
					{
						pItem->lParam = (LPARAM) &(iterRTF->second);
					}
				}
			}
        }
    }
}

CString CQPasteWnd::GetDisplayText(int dontAutoDelete, int shortCut, bool isGroup, int parentID, CString text)
{
    CString cs;
    if(dontAutoDelete)
    {
        cs += "*";
    }

    if(shortCut > 0)
    {
        cs += "s";
    }

    if(isGroup)
    {
        cs += "G";
    }

    // attached to a group
    if(parentID > 0)
    {
        cs += "!";
    }

    // pipe is the "end of symbols" marker
    cs += "|" + CMainTableFunctions::GetDisplayText(g_Opt.m_nLinesPerRow, text);

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
		CString clipData = _T("");

        int id = m_lstHeader.GetItemData(pInfo->lItem);
        CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, mText, lDate, lShortCut, clipOrder, clipGroupOrder, stickyClipOrder, stickyClipGroupOrder, lDontAutoDelete, QuickPasteText, lastPasteDate, globalShortCut, lParentID FROM Main WHERE lID = %d"), id);
        if(q.eof() == false)
        {
            CString clipText = q.getStringField(1);

			int lines = 0;
			int maxLines = CGetSetOptions::GetMaxToolTipLines();
			CTokenizer tokenizer(clipText, "\r\n");
			CString token;
			while (tokenizer.Next(token))
			{
				cs += token + "\r\n";
				if (lines > maxLines)
					break;
				lines++;
			}
			
            #ifdef _DEBUG
			clipData += StrF(_T("(Index = %d) (DB ID = %d) (Seq = %f) (Group Seq = %f) (Sticky Seq = %f) (Sticky Group Seq = %f)\n"),
								pInfo->lItem, q.getIntField(_T("lID")), 
													q.getFloatField(_T("clipOrder")), q.getFloatField(_T("clipGroupOrder")), 
													q.getFloatField(_T("stickyClipOrder")), q.getFloatField(_T("stickyClipGroupOrder")));
            #endif 

            COleDateTime time((time_t)q.getIntField(_T("lDate")));
			clipData += "\nAdded: " + time.Format();

			COleDateTime modified((time_t)q.getIntField(_T("lastPasteDate")));
			clipData += "\nLast Used: " + modified.Format();

            if(q.getIntField(_T("lDontAutoDelete")) > 0)
            {
				clipData += "\nNever Auto Delete";
            }

            CString csQuickPaste = q.getStringField(_T("QuickPasteText"));

            if(csQuickPaste.IsEmpty() == FALSE)
            {
				clipData += "\nQuick Paste = ";
				clipData += csQuickPaste;
            }

            int shortCut = q.getIntField(_T("lShortCut"));
            if(shortCut > 0)
            {
				clipData += "\r\n";
				clipData += CHotKey::GetHotKeyDisplayStatic(shortCut);

				BOOL globalShortCut = q.getIntField(_T("globalShortCut"));
				if(globalShortCut)
				{
					clipData += " - Global Shortcut Key";
				}
            }

			if (theApp.m_GroupID > 0)
			{
				int sticky = q.getIntField(_T("stickyClipGroupOrder"));
				if (sticky != INVALID_STICKY)
				{
					clipData += "\r\n";
					clipData += _T(" - Sticky In Group");
				}
			}
			else
			{
				int sticky = q.getIntField(_T("stickyClipOrder"));
				if (sticky != INVALID_STICKY)
				{
					clipData += "\r\n";
					clipData += _T(" - Sticky");
				}
			}

			int parentId = q.getIntField(_T("lParentID"));
			if (parentId > 0)
			{
				clipData += "\r\n";
				clipData += FolderPath(parentId);
			}
        }

		cs = cs.Left(pInfo->cchTextMax - clipData.GetLength());

		cs += "\r\n\r\n";
		cs += clipData;

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
        m_search.SetWindowText(fndItem.psz);
        m_search.SetFocus();
        m_search.SetSel(1, 1);

		OnSearchEditChange();

        *pResult = m_lstHeader.GetCaret();
        return ;
    }

    *pResult = -1; // Default action.
}

void CQPasteWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
    // toggle ShowPersistent when we double click the caption
    if(nHitTest == HTCAPTION)
    {
        switch(g_Opt.m_bDoubleClickingOnCaptionDoes)
        {
            case TOGGLES_ALLWAYS_ON_TOP:				
			{
				theApp.ShowPersistent(!g_Opt.m_bShowPersistent);
				if (g_Opt.m_bShowPersistent)
				{
					::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
				}

				MoveControls();

				UpdateStatus();
			}
                break;
            case TOGGLES_ALLWAYS_SHOW_DESCRIPTION:
				DoAction(ActionEnums::SHOWDESCRIPTION);    
                break;
            case ROLLES_UP_WINDOW:
                MinMaxWindow();
                break;
        }
    }

    CWndEx::OnNcLButtonDblClk(nHitTest, point);
}

void CQPasteWnd::OnShowGroupsTop()
{
	m_lstHeader.HidePopup(true);

    OnShowGroupsBottom();
    return ;
    m_GroupTree.m_bHide = false;
    m_bHideWnd = false;

    CRect crList;
    m_lstHeader.GetWindowRect(crList);

    CRect cr(crList.left, crList.top, crList.left + crList.Width(), crList.top + 200);	

    m_GroupTree.MoveWindow(cr);
    m_GroupTree.m_selectedFolderID = theApp.m_GroupID;
    m_GroupTree.FillTree();
    m_GroupTree.ShowWindow(SW_SHOW);
	

    m_GroupTree.m_bHide = true;
    m_bHideWnd = true;
}

void CQPasteWnd::OnShowGroupsBottom()
{
	m_lstHeader.HidePopup(true);

	if (m_GroupTree.IsWindowVisible())
	{
		m_GroupTree.ShowWindow(SW_HIDE);
		return;
	}

    m_GroupTree.m_bHide = false;
    m_bHideWnd = false;

    CRect crWindow, crList;
    m_lstHeader.GetWindowRect(crList);
    GetWindowRect(crWindow);

    CRect cr(crWindow.left, crWindow.bottom, crWindow.left + crWindow.Width(), crWindow.bottom + 200);

	EnsureWindowVisible(&cr);

    m_GroupTree.MoveWindow(cr);
    m_GroupTree.m_selectedFolderID = theApp.m_GroupID;
    m_GroupTree.FillTree();
    m_GroupTree.ShowWindow(SW_SHOW);

    m_GroupTree.m_bHide = true;
    m_bHideWnd = true;
}

LRESULT CQPasteWnd::OnGroupTreeMessage(WPARAM wParam, LPARAM lParam)
{
    m_bHideWnd = false;

    int id = (int)wParam;

    m_GroupTree.ShowWindow(SW_HIDE);

    m_bHandleSearchTextChange = false;
    m_search.SetWindowText(_T(""));
    m_bHandleSearchTextChange = true;

    MoveControls();

	if(id == -1)
	{
		//go back to the main list
		theApp.EnterGroupID(-1);
	}
	else if(id >= 0)
    {
        //Set the app flag so it does a send message to refresh the list
        //We need to do this because we set the list pos to 0 and with Post
        //the list is not filled up yet
        bool bItWas = theApp.m_bAsynchronousRefreshView;
        theApp.m_bAsynchronousRefreshView = false;

		CSpecialPasteOptions pasteOptions;
        OpenID(id, pasteOptions);

        theApp.m_bAsynchronousRefreshView = bItWas;

        m_lstHeader.SetListPos(0);
		m_lstHeader.SetFocus();
    }

    CWnd *p = GetFocus();
    if(p == NULL)
    {
        HideQPasteWindow(false);
    }

    m_bHideWnd = true;

    return TRUE;
}

void CQPasteWnd::OnBackButton()
{
    theApp.EnterGroupID(theApp.m_GroupParentID);
}

void CQPasteWnd::OnMenuSearchDescription()
{
	CGetSetOptions::SetSearchDescription(!CGetSetOptions::GetSearchDescription());

	CString csText;
	m_search.GetWindowText(csText);

	if(csText != _T(""))
	{
		FillList(csText);
	}
}

void CQPasteWnd::OnMenuSearchFullText()
{
	CGetSetOptions::SetSearchFullText(!CGetSetOptions::GetSearchFullText());

	CString csText;
	m_search.GetWindowText(csText);

	if(csText != _T(""))
	{
		FillList(csText);
	}
}

void CQPasteWnd::OnMenuSearchQuickPaste()
{
	CGetSetOptions::SetSearchQuickPaste(!CGetSetOptions::GetSearchQuickPaste());

	CString csText;
	m_search.GetWindowText(csText);

	if(csText != _T(""))
	{
		FillList(csText);
	}
}

void CQPasteWnd::OnSearchEditChange()
{
	m_search.Invalidate();
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
    SetTimer(TIMER_DO_SEARCH, 250, NULL);	

    return ;
}

LRESULT CQPasteWnd::OnUpDown(WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	//Workaround for allow holding down arrow keys while in the search control
	msg.lParam = lParam & (~0x40000000);
	msg.wParam = wParam;
	msg.message = WM_KEYDOWN;
	if(CheckActions(&msg) == false)
	{
		if (m_lstHeader.HandleKeyDown(wParam, lParam) == FALSE)
		{
			LRESULT res = m_lstHeader.SendMessage(WM_KEYDOWN, wParam, lParam);
		}
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
            HideQPasteWindow(false);
			m_lstHeader.HidePopup(true);
        }
    }
	else
	{
		CWnd *p = GetFocus();
		if (p == NULL)
		{
			m_lstHeader.HidePopup(true);
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

	UpdateMenuShortCut(pCmdUI, ActionEnums::NEWGROUP);
}

void CQPasteWnd::OnUpdateMenuNewgroupselection(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::NEWGROUPSELECTION);
}

void CQPasteWnd::OnUpdateMenuAllwaysontop(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::TOGGLESHOWPERSISTANT);
}

void CQPasteWnd::OnUpdateMenuViewfulldescription(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::SHOWDESCRIPTION);
}

void CQPasteWnd::OnUpdateMenuViewgroups(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::SHOWGROUPS);
}

void CQPasteWnd::OnUpdateMenuPasteplaintextonly(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_SELECTED_PLAIN_TEXT);
}

void CQPasteWnd::OnUpdateMenuDelete(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::DELETE_SELECTED);
}

void CQPasteWnd::OnUpdateMenuProperties(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::CLIP_PROPERTIES);
}

void CQPasteWnd::OnUpdateMenuEdititem(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::EDITCLIP);
}

void CQPasteWnd::OnUpdateMenuNewclip(CCmdUI *pCmdUI)
{
    if(!pCmdUI->m_pMenu)
    {
        return ;
    }

	UpdateMenuShortCut(pCmdUI, ActionEnums::NEWCLIP);
}

LRESULT CQPasteWnd::OnSetListCount(WPARAM wParam, LPARAM lParam)
{
	m_noSearchResults = false;

	int x = m_lstHeader.GetScrollPos(SB_HORZ);
	int y = m_lstHeader.GetScrollPos(SB_VERT);
	m_lstHeader.Scroll(CSize(-x, -y));

    m_lstHeader.SetItemCountEx((int)wParam);

	if ((int)wParam == 0 &&
		m_strSearch != _T(""))
	{
		m_noSearchResults = true;
		CString text = theApp.m_Language.GetString("NoSearchResults", "There are no results for");
		m_noSearchResultsStatic.SetWindowText(StrF(_T("%s \"%s\""), text, m_strSearch));
	}

	SelectFocusID();
    UpdateStatus(false);	

	MoveControls();

    return TRUE;
}

LRESULT CQPasteWnd::OnItemDeleted(WPARAM wParam, LPARAM lParam)
{
    m_lstHeader.OnItemDeleted((int)wParam);
    return TRUE;
}

LRESULT CQPasteWnd::OnRefeshRow(WPARAM wParam, LPARAM lParam)
{
    int clipId = (int)wParam;
    int listPos = (int)lParam;

	int topIndex = m_lstHeader.GetTopIndex();
	int lastIndex = topIndex + m_lstHeader.GetCountPerPage();

	if(listPos >= topIndex && listPos <= lastIndex)
	{
		m_lstHeader.RefreshRow(listPos);
		m_lstHeader.PostEventLoadedCheckDescription(listPos);
	}

	if(clipId == -2)
	{
		m_lstHeader.Invalidate();
		m_lstHeader.RedrawWindow();

		//Log(_T("End of first load, showing listbox and loading actual count, then accelerators"));
	}

    return true;
}

void CQPasteWnd::SelectFocusID() 
{
	ATL::CCritSecLock csLock(m_CritSection.m_sect);

	bool selectedItem = false;
	int index = 0;
	std::vector<CMainTable>::iterator iter = m_listItems.begin();
	while(iter != m_listItems.end())
	{
		if(iter->m_lID == theApp.m_FocusID)
		{
			m_lstHeader.SetListPos(index);
			selectedItem = true;
			break;
		}
		iter++;
		index++;
	}

	if(selectedItem == false)
	{
		m_lstHeader.SetListPos(g_Opt.SelectedIndex());
	}
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
	table.m_clipOrder = q.getFloatField(_T("clipOrder"));
	table.m_clipGroupOrder = q.getFloatField(_T("clipGroupOrder"));
	table.m_stickyClipOrder = q.getFloatField(_T("stickyClipOrder"));
	table.m_stickyClipGroupOrder = q.getFloatField(_T("stickyClipGroupOrder"));
	table.m_dateCopied = q.getIntField(_T("lDate"));
	table.m_datePasted = q.getIntField(_T("lastPasteDate"));
}

void CQPasteWnd::OnDestroy()
{
	CGetSetOptions::SetPastSearchXml(m_search.SavePastSearches());

    CWndEx::OnDestroy();
    m_thread.Stop();
	m_extraDataThread.Stop();
}

void CQPasteWnd::OnTimer(UINT_PTR nIDEvent)
{
    if(nIDEvent == TIMER_DO_SEARCH)
    {
		Log(_T("TIMER_DO_SEARCH timer\n"));

		KillTimer(TIMER_DO_SEARCH);

        CString csText;
        m_search.GetWindowText(csText);

        int nCaretPos = m_lstHeader.GetCaret();
        if(nCaretPos >= 0)
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
			CSpecialPasteOptions pasteOptions;
            OpenSelection(pasteOptions);
        }
        else
        {
            Log(_T("m_bModifersMoveActive set to false\n"));
        }
    }
	else if (nIDEvent == TIMER_ERROR_MSG)
	{
		KillTimer(TIMER_ERROR_MSG);
		m_popupMsg.Hide();
	}
	else if (nIDEvent == TIMER_DRAG_HIDE_WINDOW)
	{
		OutputDebugString(_T("drag timer\n"));

		CPoint mouse;
		GetCursorPos(&mouse);

		CRect windowRect;
		this->GetWindowRect(&windowRect);

		if(PtInRect(&windowRect, mouse) == FALSE)
		{
			HideQPasteWindow(false, false);
			KillTimer(TIMER_DRAG_HIDE_WINDOW);
		}
	}
	else if (nIDEvent == TIMER_DO_ACTION)
	{
		KillTimer(TIMER_DO_ACTION);

		OutputDebugString(StrF(_T("DoActionTimer, cmd: %d"), m_timerAction.Cmd));

		if (m_timerAction.Cmd > 0)
		{
			DoAction(m_timerAction);
		}
		m_timerAction = CAccel();		
	}

    CWndEx::OnTimer(nIDEvent);
}

void CQPasteWnd::OnAddinSelect(UINT idIn)
{
    ARRAY IDs;
    m_lstHeader.GetSelectionItemData(IDs);

    if(IDs.GetCount() > 0)
    {
        int id = IDs[0];
        CClip clip;
        if(clip.LoadMainTable(id))
        {
            if(clip.LoadFormats(id, false))
            {
                bool bCont = theApp.m_Addins.CallPrePasteFunction(idIn, &clip);
                if(bCont)
                {
					CSpecialPasteOptions pasteOptions;
					pasteOptions.m_pPasteFormats = &clip.m_Formats;
                    OpenID(-1, pasteOptions);
                }
            }
        }
    }
}

LRESULT CQPasteWnd::OnSelectAll(WPARAM wParam, LPARAM lParam)
{
    BOOL ret = FALSE;
    ATL::CCritSecLock csLock(m_CritSection.m_sect);

    if((int)m_listItems.size() < m_lstHeader.GetItemCount())
    {
        Log(_T("All items selected loading all items from the db"));

        CPoint loadItem(0, m_lstHeader.GetItemCount());
        m_loadItems.push_back(loadItem);

        m_thread.FireLoadItems(false);

        ret = TRUE;

        UpdateStatus(false);
    }

    return ret;
}

LRESULT CQPasteWnd::OnShowHideScrollBar(WPARAM wParam, LPARAM lParam)
{
	if(wParam == 1)
	{
		Log(_T("OnShowHideScrollBar Showing ScrollBars"));

		m_showScrollBars = true;

		MoveControls();
	}
	else
	{
		Log(_T("OnShowHideScrollBar Hiding ScrollBars"));

		m_showScrollBars = false;
		MoveControls();
	}

	return 1;
}

//HBRUSH CQPasteWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
//{
//	// Call the base class implementation first! Otherwise, it may 
//	// undo what we're trying to accomplish here.
//	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
//
//	switch (nCtlColor) 
//	{
//	case CTLCOLOR_STATIC:
//		switch (pWnd->GetDlgCtrlID())
//		{
//			case ON_TOP_WARNING:
//			{
//				pDC->SetBkMode(TRANSPARENT);
//				pDC->SetBkColor(RGB(0, 0, 255));
//
//				CBrush brush;
//				brush.CreateSolidBrush(COLORREF(RGB(255, 0, 0)));
//				return brush;
//			}
//			break;
//		}
//	}
//
//	return hbr;
//}

//void CQPasteWnd::OnPaint()
//{
//	/*CBrush brush;
//	brush.CreateSolidBrush(COLORREF(RGB(255, 0, 0)));
//
//	CRect clientRect;
//	GetClientRect(clientRect);
//
//	CPaintDC dc(this);
//	dc.FillRect(clientRect, &brush);*/
//
//	
//		CQPasteWnd::OnPaint();
//	
//}

BOOL CQPasteWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	CBrush myBrush(g_Opt.m_Theme.MainWindowBG());    // dialog background color
	CBrush *pOld = pDC->SelectObject(&myBrush);
	BOOL bRes  = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOld);    // restore old brush
	return bRes;                       // CDialog::OnEraseBkgnd(pDC);

	//return TRUE;
	// TODO: Add your message handler code here and/or call default

	//return CWndEx::OnEraseBkgnd(pDC);
}

void CQPasteWnd::OnQuickoptionsShowintaskbar()
{
	DoAction(ActionEnums::SHOW_IN_TASKBAR);
}


void CQPasteWnd::OnMenuViewasqrcode()
{
	DoAction(ActionEnums::EXPORT_TO_QR_CODE);
}

void CQPasteWnd::OnExportExporttotextfile()
{
	DoAction(ActionEnums::EXPORT_TO_TEXT_FILE);
}

void CQPasteWnd::OnCompareCompare()
{
	DoAction(ActionEnums::COMPARE_SELECTED_CLIPS);
}

void CQPasteWnd::OnCompareSelectleftcompare()
{
	DoAction(ActionEnums::SELECT_LEFT_SIDE_COMPARE);
}

void CQPasteWnd::OnCompareCompareagainst()
{
	DoAction(ActionEnums::SELECT_RIGHT_SITE_AND_DO_COMPARE);
}

void CQPasteWnd::OnUpdateCompareCompare(CCmdUI *pCmdUI)
{
	if(!pCmdUI->m_pMenu)
	{
		return ;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::COMPARE_SELECTED_CLIPS);	
}

void CQPasteWnd::UpdateMenuShortCut(CCmdUI *pCmdUI, DWORD action)
{
	if(pCmdUI == NULL ||
		pCmdUI->m_pMenu == NULL)
	{
		return;
	}

	CString cs;
	pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, cs, MF_BYCOMMAND);
	CString shortcutText = m_actions.GetCmdKeyText(action);
	if(shortcutText != _T("") &&
		cs.Find("\t" + shortcutText) < 0)
	{
		cs += "\t";
		cs += shortcutText;
		pCmdUI->SetText(cs);
	}
}

LRESULT CQPasteWnd::OnShowProperties(WPARAM wParam, LPARAM lParam)
{
	return ShowProperties((int) wParam, -1);
}

LRESULT CQPasteWnd::OnNewGroup(WPARAM wParam, LPARAM lParam)
{
	NewGroup(false, (int) wParam);

	return TRUE;
}

LRESULT CQPasteWnd::OnDeleteId(WPARAM wParam, LPARAM lParam)
{
	if (g_Opt.GetPromptWhenDeletingClips())
	{
		bool bStartValue = m_bHideWnd;
		m_bHideWnd = false;

		int nRet = MessageBox(theApp.m_Language.GetString("Delete_Clip_Groups", "Delete Group?"), _T("Ditto"), MB_OKCANCEL| MB_TOPMOST);

		m_bHideWnd = bStartValue;

		if (nRet != IDOK)
		{
			return FALSE;
		}
	}

	CClipIDs IDs;
	ARRAY Indexs;

	IDs.Add((int) wParam);

	bool selectedItem = false;
	int index = 0;
	{
		ATL::CCritSecLock csLock(m_CritSection.m_sect);
		std::vector<CMainTable>::iterator iter = m_listItems.begin();
		while (iter != m_listItems.end())
		{
			if (iter->m_lID == wParam)
			{
				Indexs.Add(index);
				break;
			}
			iter++;
			index++;
		}
	}

	DeleteClips(IDs, Indexs);

	return TRUE;
}

void CQPasteWnd::OnMenuSimpleTextSearch()
{
	CGetSetOptions::SetSimpleTextSearch(!CGetSetOptions::GetSimpleTextSearch());
	CGetSetOptions::SetRegExTextSearch(FALSE);
}

void CQPasteWnd::OnMenuRegularexpressionsearch()
{
	CGetSetOptions::SetSimpleTextSearch(FALSE);
	CGetSetOptions::SetRegExTextSearch(!CGetSetOptions::GetRegExTextSearch());
}


void CQPasteWnd::OnImportExporttogoogletranslate()
{
	DoAction(ActionEnums::EXPORT_TO_GOOGLE_TRANSLATE);
}

void CQPasteWnd::OnUpdateImportExporttogoogletranslate(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::EXPORT_TO_GOOGLE_TRANSLATE);
}


void CQPasteWnd::OnImportExportclipBitmap()
{
	DoAction(ActionEnums::EXPORT_TO_BITMAP_FILE);
}


void CQPasteWnd::OnUpdateImportExportclipBitmap(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::EXPORT_TO_BITMAP_FILE);
}

void CQPasteWnd::OnMenuWildcardsearch()
{
	CGetSetOptions::SetSimpleTextSearch(FALSE);
	CGetSetOptions::SetRegExTextSearch(FALSE);
}

void CQPasteWnd::OnMenuSavecurrentclipboard()
{
	DoAction(ActionEnums::SAVE_CURRENT_CLIPBOARD);
}


void CQPasteWnd::OnUpdateMenuSavecurrentclipboard(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::SAVE_CURRENT_CLIPBOARD);
}


bool CQPasteWnd::SyncClipDataToArrayData(CClip &clip)
{
	int row = 0;
	bool found = false;
	std::vector<CMainTable>::iterator iter = m_listItems.begin();
	while (iter != m_listItems.end())
	{
		if (iter->m_lID == clip.ID())
		{
			iter->m_clipOrder = clip.m_clipOrder;			
			iter->m_clipGroupOrder = clip.m_clipGroupOrder;

			iter->m_stickyClipOrder = clip.m_stickyClipOrder;
			iter->m_stickyClipGroupOrder = clip.m_stickyClipGroupOrder;
			
			found = true;
			break;
		}
		iter++;
		row++;
	}

	return found;
}

bool CQPasteWnd::SelectIds(ARRAY &ids)
{
	int row = 0;
	bool found = false;
	std::vector<CMainTable>::iterator iter = m_listItems.begin();
	while (iter != m_listItems.end())
	{
		if(ids.Find(iter->m_lID))
		{
			if(found == false)
			{
				m_lstHeader.SetListPos(row);
			}
			else
			{
				m_lstHeader.SetSelection(row);
			}

			found = true;
		}
		iter++;
		row++;
	}

	return found;
}

void CQPasteWnd::OnCliporderMoveup()
{
	DoAction(ActionEnums::MOVE_CLIP_UP);
}

void CQPasteWnd::OnUpdateCliporderMoveup(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::MOVE_CLIP_UP);
}

void CQPasteWnd::OnCliporderMovedown()
{
	DoAction(ActionEnums::MOVE_CLIP_DOWN);
}

void CQPasteWnd::OnUpdateCliporderMovedown(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::MOVE_CLIP_DOWN);
}

void CQPasteWnd::OnCliporderMovetotop()
{
	DoAction(ActionEnums::MOVE_CLIP_TOP);
}

void CQPasteWnd::OnUpdateCliporderMovetotop(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::MOVE_CLIP_TOP);
}


void CQPasteWnd::OnMenuFilteron()
{
	DoAction(ActionEnums::FILTER_ON_SELECTED_CLIP);
}


void CQPasteWnd::OnUpdateMenuFilteron(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::FILTER_ON_SELECTED_CLIP);
}

void CQPasteWnd::OnAlwaysOnTopClicked()
{
	DoAction(ActionEnums::TOGGLESHOWPERSISTANT);
}


void CQPasteWnd::OnSpecialpasteUppercase()
{
	DoAction(ActionEnums::PASTE_UPPER_CASE);
}


void CQPasteWnd::OnUpdateSpecialpasteUppercase(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_UPPER_CASE);
}


void CQPasteWnd::OnSpecialpasteLowercase()
{
	DoAction(ActionEnums::PASTE_LOWER_CASE);
}


void CQPasteWnd::OnUpdateSpecialpasteLowercase(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_LOWER_CASE);
}


void CQPasteWnd::OnSpecialpasteCapitalize()
{
	DoAction(ActionEnums::PASTE_CAPITALiZE);
}

void CQPasteWnd::OnUpdateSpecialpasteCapitalize(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_CAPITALiZE);
}


void CQPasteWnd::OnSpecialpasteSentence()
{
	DoAction(ActionEnums::PASTE_SENTENCE_CASE);
}


void CQPasteWnd::OnUpdateSpecialpasteSentence(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_SENTENCE_CASE);
}

void CQPasteWnd::OnSystemButton()
{
	m_lstHeader.HidePopup(true);

	POINT pp;
	CMenu cmPopUp;
	CMenu *cmSubMenu = NULL;

	GetCursorPos(&pp);
	if (cmPopUp.LoadMenu(IDR_QUICK_PASTE_SYSTEM_MENU) != 0)
	{
		cmSubMenu = cmPopUp.GetSubMenu(0);
		if (!cmSubMenu)
		{
			return;
		}				

		if (CGetSetOptions::GetShowStartupMessage())
		{
			cmSubMenu->CheckMenuItem(ID_FIRST_SHOWSTARTUPMESSAGE, MF_CHECKED);
		}
		
		theApp.m_Language.UpdateRightClickMenu(cmSubMenu);

		SetMenuChecks(cmSubMenu);

		cmSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pp.x, pp.y, this, NULL);
	}
}

void CQPasteWnd::OnSpecialpasteRemovelinefeeds()
{
	DoAction(ActionEnums::PASTE_REMOVE_LINE_FEEDS);
}


void CQPasteWnd::OnUpdateSpecialpasteRemovelinefeeds(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_REMOVE_LINE_FEEDS);
}


void CQPasteWnd::OnSpecialpastePaste()
{
	DoAction(ActionEnums::PASTE_ADD_ONE_LINE_FEED);
}


void CQPasteWnd::OnUpdateSpecialpastePaste(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_ADD_ONE_LINE_FEED);
}


void CQPasteWnd::OnSpecialpastePaste32919()
{
	DoAction(ActionEnums::PASTE_ADD_TWO_LINE_FEEDS);
}

void CQPasteWnd::OnUpdateSpecialpastePaste32919(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_ADD_TWO_LINE_FEEDS);
}


void CQPasteWnd::OnSpecialpasteTypoglycemia()
{
	DoAction(ActionEnums::PASTE_TYPOGLYCEMIA);
}

void CQPasteWnd::OnUpdateSpecialpasteTypoglycemia(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_TYPOGLYCEMIA);
}

void CQPasteWnd::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	MSG msg;
	msg.lParam = 0;
	msg.wParam = VK_MOUSE_CLICK;
	msg.message = WM_KEYDOWN;
	if (CheckActions(&msg) == false)
	{
	}
	*pResult = 0;
}


void CQPasteWnd::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	MSG msg;
	msg.lParam = 0;
	msg.wParam = VK_MOUSE_DOUBLE_CLICK;
	msg.message = WM_KEYDOWN;
	if (CheckActions(&msg) == false)
	{
	}
	
	*pResult = 0;
}


void CQPasteWnd::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	MSG msg;
	msg.lParam = 0;
	msg.wParam = VK_MOUSE_RIGHT_CLICK;
	msg.message = WM_KEYDOWN;
	if (CheckActions(&msg) == false)
	{
	}
	*pResult = 0;
}

void CQPasteWnd::OnNMRDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	/*MSG msg;
	msg.lParam = 0;
	msg.wParam = VK_MOUSE_RIGHT_CLICK;
	msg.message = WM_KEYDOWN;
	if (CheckActions(&msg) == false)
	{
	}*/
	*pResult = 0;
}

void CQPasteWnd::OnQuickoptionsShowtextforfirsttencopyhotkeys()
{
	DoAction(ActionEnums::CONFIG_SHOW_FIRST_TEN_TEXT);
}


void CQPasteWnd::OnUpdateQuickoptionsShowtextforfirsttencopyhotkeys(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::CONFIG_SHOW_FIRST_TEN_TEXT);
}


void CQPasteWnd::OnQuickoptionsShowindicatoracliphasbeenpasted()
{
	DoAction(ActionEnums::CONFIG_SHOW_CLIP_WAS_PASTED);
}

void CQPasteWnd::OnUpdateQuickoptionsShowindicatoracliphasbeenpasted(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::CONFIG_SHOW_CLIP_WAS_PASTED);
}


void CQPasteWnd::OnGroupsTogglelastgroup()
{
	DoAction(ActionEnums::TOGGLE_LAST_GROUP_TOGGLE);
}


void CQPasteWnd::OnUpdateGroupsTogglelastgroup(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TOGGLE_LAST_GROUP_TOGGLE);
}


void CQPasteWnd::OnSpecialpastePaste32927()
{
	DoAction(ActionEnums::PASTE_ADD_CURRENT_TIME);
}


void CQPasteWnd::OnUpdateSpecialpastePaste32927(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_ADD_CURRENT_TIME);
}


void CQPasteWnd::OnMenuGlobalhotkeys32933()
{
	DoAction(ActionEnums::GLOBAl_HOTKEYS);
}

void CQPasteWnd::OnUpdateMenuGlobalhotkeys32933(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::GLOBAl_HOTKEYS);
}


void CQPasteWnd::OnMenuDeleteclipdata32934()
{
	DoAction(ActionEnums::DELETE_CLIP_DATA);
}



void CQPasteWnd::OnUpdateMenuDeleteclipdata32934(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::DELETE_CLIP_DATA);
}



void CQPasteWnd::OnMenuImportclip32935()
{
	DoAction(ActionEnums::IMPORT_CLIP);
}

void CQPasteWnd::OnUpdateMenuImportclip32935(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::IMPORT_CLIP);
}

void CQPasteWnd::OnMenuNewclip32937()
{
	DoAction(ActionEnums::NEWCLIP);
}

void CQPasteWnd::OnUpdateMenuNewclip32937(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::NEWCLIP);
}

LRESULT CQPasteWnd::OnSearchFocused(WPARAM wParam, LPARAM lParam)
{
	m_lstHeader.HidePopup(true);

	return TRUE;
}

#include <ShellScalingApi.h>


LRESULT CQPasteWnd::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
	ARRAY Indexs;
	m_lstHeader.GetSelectionIndexes(Indexs);
	if (Indexs.GetCount() <= 0)
	{
		Indexs.Add(0);
	}
	int c = m_lstHeader.GetItemCount();
	m_lstHeader.SetItemCountEx(0);

	int dpi = HIWORD(wParam);	
	m_DittoWindow.OnDpiChanged(this, dpi);
		
	//RECT* const prcNewWindow = (RECT*)lParam;
	CRect r(*(RECT*)lParam);
	if (g_Opt.m_bEnsureEntireWindowCanBeSeen)
	{
		EnsureWindowVisible(&r);
	}
	SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	
	m_systemMenu.Reset();
	m_systemMenu.LoadStdImageDPI(m_DittoWindow.m_dpi.GetDPI(), system_menu_2_24, system_menu_2_30, system_menu_2_36, system_menu_2_42, system_menu_2_48, _T("PNG"), system_menu_54, system_menu_60, system_menu_66, system_menu_72, system_menu_78, system_menu_84);

	m_BackButton.Reset();
	m_BackButton.LoadStdImageDPI(m_DittoWindow.m_dpi.GetDPI(), return_16, return_20, return_24, return_28, return_32, _T("PNG"));

	m_ShowGroupsFolderBottom.Reset();
	m_ShowGroupsFolderBottom.LoadStdImageDPI(m_DittoWindow.m_dpi.GetDPI(), open_folder_24, open_folder_30, open_folder_36, open_folder_42, open_folder_48, _T("PNG"), open_folder_54, open_folder_60, open_folder_66, open_folder_72, open_folder_78, open_folder_84);
	
	m_search.OnDpiChanged();		
	m_lstHeader.OnDpiChanged();	

	UpdateFont();
	this->SetLinesPerRow(CGetSetOptions::GetLinesPerRow(), true, false);
		
	MoveControls();
	
	m_lstHeader.SetItemCountEx(c);
	m_lstHeader.SetListPos(Indexs[0]);
	
	InvalidateNc();
	this->Invalidate();
	m_lstHeader.RefreshVisibleRows();
	this->RedrawWindow();

	return TRUE;
}

void CQPasteWnd::OnCliporderReplacetopstickyclip()
{
	DoAction(ActionEnums::REPLACE_TOP_STICKY_CLIP);
}

void CQPasteWnd::OnUpdateCliporderReplacetopstickyclip(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::REPLACE_TOP_STICKY_CLIP);
}


void CQPasteWnd::OnSendtoPromptforname()
{
	DoAction(ActionEnums::PROMPT_SEND_TO_FRIEND);
}

void CQPasteWnd::OnUpdateSendtoPromptforname(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PROMPT_SEND_TO_FRIEND);
}


void CQPasteWnd::OnImportImportcopiedfile()
{
	DoAction(ActionEnums::SAVE_CF_HDROP_FIlE_DATA);
}

void CQPasteWnd::OnUpdateImportImportcopiedfile(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::SAVE_CF_HDROP_FIlE_DATA);
}

void CQPasteWnd::OnCustomSendToFriend(UINT idIn)
{
	bool showDlg = false;
	CString ip_name = m_customFriendsHelper.GetSendTo(idIn, showDlg);
	if (showDlg)
	{
		DoAction(ActionEnums::PROMPT_SEND_TO_FRIEND);
	}
	else if (ip_name != _T(""))
	{
		SendToFriendbyPos(0, ip_name);
	}
}

void CQPasteWnd::OnChaiScriptPaste(UINT idIn)
{
	CSpecialPasteOptions pasteOptions;
	int index = idIn - ChaiScriptMenuStartId;

	if (index >= 0 &&
		index < g_Opt.m_pasteScripts.m_list.size())
	{
		pasteOptions.m_pasteScriptGuid = g_Opt.m_pasteScripts.m_list[index].m_guid;
		OpenSelection(pasteOptions);
	}
}

void CQPasteWnd::OnCliporderMovetolast()
{
	DoAction(ActionEnums::MOVE_CLIP_LAST);
}


void CQPasteWnd::OnUpdateCliporderMovetolast(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::MOVE_CLIP_LAST);
}


void CQPasteWnd::OnSpecialpastePasteDontUpdateOrder()
{
	DoAction(ActionEnums::PASTE_DONT_MOVE_CLIP);
}


void CQPasteWnd::OnUpdateOnSpecialPasteDontUpdateOrder(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_DONT_MOVE_CLIP);
}


void CQPasteWnd::OnSpecialpasteTrim()
{
	DoAction(ActionEnums::PASTE_TRIM_WHITE_SPACE);
}


void CQPasteWnd::OnUpdateSpecialpasteTrim(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::PASTE_TRIM_WHITE_SPACE);
}

void CQPasteWnd::OnMenuTransparencyNone()
{
	DoAction(ActionEnums::TRANSPARENCY_NONE);
}

void CQPasteWnd::OnUpdateTransparencyNone(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_NONE);
}

void CQPasteWnd::OnMenuTransparency5()
{
	DoAction(ActionEnums::TRANSPARENCY_5);
}

void CQPasteWnd::OnUpdateTransparency5(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_5);
}

void CQPasteWnd::OnMenuTransparency10()
{
	DoAction(ActionEnums::TRANSPARENCY_10);
}

void CQPasteWnd::OnUpdateTransparency10(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_10);
}

void CQPasteWnd::OnMenuTransparency15()
{
	DoAction(ActionEnums::TRANSPARENCY_15);
}

void CQPasteWnd::OnUpdateTransparency15(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_15);
}

void CQPasteWnd::OnMenuTransparency20()
{
	DoAction(ActionEnums::TRANSPARENCY_20);
}

void CQPasteWnd::OnUpdateTransparency20(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_20);
}

void CQPasteWnd::OnMenuTransparency25()
{
	DoAction(ActionEnums::TRANSPARENCY_25);
}

void CQPasteWnd::OnUpdateTransparency25(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_25);
}

void CQPasteWnd::OnMenuTransparency30()
{
	DoAction(ActionEnums::TRANSPARENCY_30);
}

void CQPasteWnd::OnUpdateTransparency30(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_30);
}

void CQPasteWnd::OnTransparency35()
{
	DoAction(ActionEnums::TRANSPARENCY_35);
}

void CQPasteWnd::OnUpdateTransparency35(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_35);
}

void CQPasteWnd::OnMenuTransparency40()
{
	DoAction(ActionEnums::TRANSPARENCY_40);
}

void CQPasteWnd::OnUpdateTransparency40(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_40);
}

bool CQPasteWnd::DoActionToggleTransparency()
{
	CGetSetOptions::SetEnableTransparency(!CGetSetOptions::GetEnableTransparency());
	SetCurrentTransparency();

	return true;
}

bool CQPasteWnd::DoActionIncreaseTransparency()
{
	int current = CGetSetOptions::GetTransparencyPercent();
	current += 5;
	current = min(current, 100);
	SetTransparency(current);

	return true;
}

bool CQPasteWnd::DoActionDecreaseTransparency()
{
	int current = CGetSetOptions::GetTransparencyPercent();
	current -= 5;
	current = max(current, 0);
	SetTransparency(current);

	return true;
}

bool CQPasteWnd::DoActionEmailTo()
{
	CWaitCursor wait;

	CClipIDs IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	CClip clip;

	if (IDs.GetCount() > 1)
	{
		CStringA SepA = CTextConvert::ConvertToChar(g_Opt.GetMultiPasteSeparator());
		CStringW SepW = CTextConvert::ConvertToUnicode(g_Opt.GetMultiPasteSeparator());

		CHTMLFormatAggregator Html(SepA);
		if (IDs.AggregateData(Html, theApp.m_HTML_Format, g_Opt.m_bMultiPasteReverse, false))
		{
			CClipFormat cf(theApp.m_HTML_Format, Html.GetHGlobal());
			clip.m_Formats.Add(cf);
			//clip.m_Formats now owns the global data
			cf.m_autoDeleteData = false;
		}
		
		CCF_UnicodeTextAggregator CFUnicodeText(SepW);
		if (IDs.AggregateData(CFUnicodeText, CF_UNICODETEXT, g_Opt.m_bMultiPasteReverse, false))
		{
			CClipFormat cf(CF_UNICODETEXT, CFUnicodeText.GetHGlobal());
			clip.m_Formats.Add(cf);
			//clip.m_Formats now owns the global data
			cf.m_autoDeleteData = false;
		}
		else
		{
			
			CCF_TextAggregator CFText(SepA);
			if (IDs.AggregateData(CFText, CF_TEXT, g_Opt.m_bMultiPasteReverse, false))
			{
				CClipFormat cf(CF_TEXT, CFText.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}
		}
	}
	else
	{
		clip.LoadFormats(IDs[0], false, false);
	}

	CString path = CGetSetOptions::GetPath(PATH_DRAG_FILES);
	CreateDirectory(path, NULL);
	int dragId = CGetSetOptions::GetDragId();
	int origDragId = dragId;

	CString subject;
	CString body;
	CString attachment;

	CClipFormat *html = clip.m_Formats.FindFormat(theApp.m_HTML_Format);
	if (html != NULL)
	{
		CString file;
		file.Format(_T("%shtml_%d.html"), path, dragId++);

		clip.WriteTextToHtmlFile(file);

		attachment = file;
	}
	else
	{
		CString text = clip.GetUnicodeTextFormat();
		if (text == _T(""))
		{
			text = clip.GetCFTextTextFormat();
		}

		if (text != _T(""))
		{
			body = text;
			subject = text.Left(30);
		}
		else
		{
			CClipFormat *png = NULL;
			CClipFormat *dib = clip.m_Formats.FindFormat(CF_DIB);
			if (dib == NULL)
			{
				png = clip.m_Formats.FindFormat(theApp.m_PNG_Format);
			}

			if (png != NULL ||
				dib != NULL)
			{
				CString file;
				file.Format(_T("%simage_%d.png"), path, dragId++);

				clip.WriteImageToFile(file);

				CString fileWrapper;
				fileWrapper.Format(_T("%shtml_%d.html"), path, dragId++);

				CFile f;
				if (f.Open(fileWrapper, CFile::modeWrite | CFile::modeCreate))
				{
					CString html;
					html.Format(_T("<html><img src=\"%s\"></html>"), file);

					CStringA convToUtf8;
					CTextConvert::ConvertToUTF8(html, convToUtf8);
					f.Write(convToUtf8.GetBuffer(), convToUtf8.GetLength());

					f.Close();
				}

				attachment = fileWrapper;
			}
		}
	}

	if (subject != _T("") ||
		body != _T("") ||
		attachment != _T(""))
	{
		SendMail::Send(subject, body, attachment);
	}

	if (dragId != origDragId)
	{
		CGetSetOptions::SetDragId(dragId);
	}

	return true;
}

bool CQPasteWnd::DoActionGmail()
{
	CWaitCursor wait;

	CClipIDs IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	CClip clip;

	if (IDs.GetCount() > 1)
	{
		CStringW SepW = CTextConvert::ConvertToUnicode(g_Opt.GetMultiPasteSeparator());
		CCF_UnicodeTextAggregator CFUnicodeText(SepW);
		if (IDs.AggregateData(CFUnicodeText, CF_UNICODETEXT, g_Opt.m_bMultiPasteReverse, false))
		{
			CClipFormat cf(CF_UNICODETEXT, CFUnicodeText.GetHGlobal());
			clip.m_Formats.Add(cf);
			//clip.m_Formats now owns the global data
			cf.m_autoDeleteData = false;
		}
		else
		{
			CStringA SepA = CTextConvert::ConvertToChar(g_Opt.GetMultiPasteSeparator());
			CCF_TextAggregator CFText(SepA);
			if (IDs.AggregateData(CFText, CF_TEXT, g_Opt.m_bMultiPasteReverse, false))
			{
				CClipFormat cf(CF_TEXT, CFText.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}
		}
	}
	else
	{
		clip.LoadFormats(IDs[0], true, false);
	}

	CString text = clip.GetUnicodeTextFormat();
	if (text == _T(""))
	{
		text = clip.GetCFTextTextFormat();
	}

	if (text != _T(""))
	{
		CString link;
		text.Replace(_T("\r\n"), _T("%0D%0A"));
		CString en = InternetEncode(text);

		link.Format(_T("https://mail.google.com/mail/u/0/?view=cm&body=%s"), en);

		CHyperLink::GotoURL(link, SW_SHOW);
	}

	return true;
}

bool CQPasteWnd::DoActionEmailToAttachExport()
{
	CWaitCursor wait;

	CClipIDs IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	CString path = CGetSetOptions::GetPath(PATH_DRAG_FILES);
	CreateDirectory(path, NULL);
	int dragId = CGetSetOptions::GetDragId();
	int origDragId = dragId;

	CString file;
	file.Format(_T("%sexport_%d.dto"), path, dragId++);

	IDs.Export(file);
	
	SendMail::Send(_T(""), _T(""), file);

	return true;
}

bool CQPasteWnd::DoActionEmailToAttachContent()
{
	CString path = CGetSetOptions::GetPath(PATH_DRAG_FILES);
	CreateDirectory(path, NULL);

	CClipIDs clipIds;
	m_lstHeader.GetSelectionItemData(clipIds);

	int dragId = CGetSetOptions::GetDragId();
	int origDragId = dragId;

	CStringArray fileList;

	for (int i = 0; i < clipIds.GetCount(); i++)
	{
		CClip fileClip;
		fileClip.LoadFormats(clipIds[i]);

		CClipFormat *unicodeText = fileClip.m_Formats.FindFormat(CF_UNICODETEXT);
		if (unicodeText)
		{
			CString file;
			file.Format(_T("%stext_%d.txt"), path, dragId++);

			fileClip.WriteTextToFile(file, TRUE, FALSE, FALSE);
			fileList.Add(file);
		}
		else
		{
			CClipFormat *asciiText = fileClip.m_Formats.FindFormat(CF_TEXT);
			if (asciiText)
			{
				CString file;
				file.Format(_T("%stext_%d.txt"), path, dragId++);

				fileClip.WriteTextToFile(file, FALSE, TRUE, FALSE);
				fileList.Add(file);
			}
			else
			{
				CClipFormat *png = NULL;
				CClipFormat *bitmap = fileClip.m_Formats.FindFormat(CF_DIB);
				if (bitmap == NULL)
				{
					png = fileClip.m_Formats.FindFormat(theApp.m_PNG_Format);
				}

				if (bitmap != NULL ||
					png != NULL)
				{
					CString file;
					file.Format(_T("%simage_%d.png"), path, dragId++);

					if (fileClip.WriteImageToFile(file))
					{
						fileList.Add(file);
					}
				}
			}
		}

		//couldn't get SendMail::Send to support multiple files
		break;
	}

	if (fileList.GetCount() > 0)
	{
		SendMail::Send(_T(""), _T(""), fileList[0]);
	}

	if (dragId != origDragId)
	{
		CGetSetOptions::SetDragId(dragId);
	}

	return true;
}

bool CQPasteWnd::DoActionSlugify()
{
	if (::GetFocus() == m_lstHeader.GetSafeHwnd())
	{
		CSpecialPasteOptions pasteOptions;
		pasteOptions.m_pasteSlugify = true;
		OpenSelection(pasteOptions);
		return true;
	}
}

bool CQPasteWnd::DoCopySelection()
{
	ARRAY IDs;
	m_lstHeader.GetSelectionItemData(IDs);

	INT_PTR count = IDs.GetSize();

	if (count <= 0)
	{
		return FALSE;
	}

	CProcessPaste paste;

	//Don't send the paste just load it into memory
	paste.m_bSendPaste = false;

	if (count > 1)
		paste.GetClipIDs().Copy(IDs);
	else
		paste.GetClipIDs().Add(IDs[0]);

	//Don't move these to the top
	BOOL itWas = g_Opt.m_bUpdateTimeOnPaste;
	g_Opt.m_bUpdateTimeOnPaste = CGetSetOptions::GetUpdateClipOrderOnCtrlC();

	paste.DoPaste();

	g_Opt.m_bUpdateTimeOnPaste = itWas;
}

void CQPasteWnd::SetTransparency(int percent)
{
	if (percent > 0)
	{
		CGetSetOptions::SetTransparencyPercent(percent);
		CGetSetOptions::SetEnableTransparency(TRUE);

		m_Alpha.SetTransparent(TRUE);

		float fPercent = percent / (float)100.0;

		m_Alpha.SetOpacity(OPACITY_MAX - (int)(fPercent *OPACITY_MAX));
	}
	else
	{
		CGetSetOptions::SetEnableTransparency(FALSE);
		m_Alpha.SetTransparent(FALSE);
	}
}

void CQPasteWnd::SetCurrentTransparency()
{
	//Set the transparency
	if (CGetSetOptions::GetEnableTransparency())
	{
		m_Alpha.SetTransparent(TRUE);

		float fPercent = CGetSetOptions::GetTransparencyPercent() / (float)100.0;

		m_Alpha.SetOpacity(OPACITY_MAX - (int)(fPercent *OPACITY_MAX));
	}
	else
	{
		m_Alpha.SetTransparent(FALSE);
	}
}

void CQPasteWnd::OnTransparencyIncrease()
{
	DoAction(ActionEnums::TRANSPARENCY_INCREASE);
}


void CQPasteWnd::OnUpdateTransparencyIncrease(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_INCREASE);
}


void CQPasteWnd::OnTransparencyDecrease()
{
	DoAction(ActionEnums::TRANSPARENCY_DECREASE);
}


void CQPasteWnd::OnUpdateTransparencyDecrease(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_DECREASE);
}


void CQPasteWnd::OnTransparencyToggle()
{
	DoAction(ActionEnums::TRANSPARENCY_TOGGLE);
}


void CQPasteWnd::OnUpdateTransparencyToggle(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::TRANSPARENCY_TOGGLE);
}

void CQPasteWnd::OnImportEmailto()
{
	DoAction(ActionEnums::EMAILTO_BODY);
}

void CQPasteWnd::OnUpdateImportEmailto(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::EMAILTO_BODY);
}


void CQPasteWnd::OnImportGmail()
{
	DoAction(ActionEnums::GMAIL);
}


void CQPasteWnd::OnUpdateImportGmail(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::GMAIL);
}


void CQPasteWnd::OnImportEmailtoasattachment()
{
	DoAction(ActionEnums::EMAILTO_ATTACH_EXPORT);
}


void CQPasteWnd::OnUpdateImportEmailtoasattachment(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::EMAILTO_ATTACH_EXPORT);
}


void CQPasteWnd::OnSpecialpasteSlugify()
{
	DoAction(ActionEnums::SLUGIFY);
}

void CQPasteWnd::OnUpdateSpecialpasteSlugify(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::SLUGIFY);
}


void CQPasteWnd::OnImportEmailContentAttach()
{
	DoAction(ActionEnums::EMAILTO_ATTACH_CONTENT);
}


void CQPasteWnd::OnUpdateImportEmailContentAttach(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::EMAILTO_ATTACH_CONTENT);
}


void CQPasteWnd::OnSpecialpasteTogglecase()
{
	DoAction(ActionEnums::INVERT_CASE);
}


void CQPasteWnd::OnUpdateSpecialpasteTogglecase(CCmdUI *pCmdUI)
{
	if (!pCmdUI->m_pMenu)
	{
		return;
	}

	UpdateMenuShortCut(pCmdUI, ActionEnums::INVERT_CASE);
}


void CQPasteWnd::OnFirstShowstartupmessage()
{
	BOOL existing = CGetSetOptions::GetShowStartupMessage();
	CGetSetOptions::SetShowStartupMessage(!existing);
}


void CQPasteWnd::OnFirstRestoreDb()
{
	theApp.m_pMainFrame->PostMessage(WM_RESTORE_DB, 0, 0);
}

void CQPasteWnd::OnFirstBackupDb()
{
	theApp.m_pMainFrame->PostMessage(WM_BACKUP_DB, 0, 0);
}