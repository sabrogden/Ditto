#pragma once

#include "QListCtrl.h"
#include "SearchEditBox.h"
#include "WndEx.h"
#include "GroupStatic.h"
#include "GroupTree.h"
#include "AlphaBlend.h"
#include "Sqlite\CppSQLite3.h"
#include <vector>
#include <list>
#include <map>
#include <afxmt.h>
#include "ClipFormatQListCtrl.h"
#include "QPasteWndThread.h"
#include "editwithbutton.h"
#include "GdipButton.h"
#include "SpecialPasteOptions.h"
#include "ClipIds.h"
#include "SymbolEdit.h"
#include "Popup.h"
#include "CustomFriendsHelper.h"

class CMainTable
{
public:
    CMainTable(): 
		m_lID( - 1), 
		m_bDontAutoDelete(false), 
		m_bIsGroup(false), 
		m_bHasShortCut(false), 
		m_bHasParent(false),
		m_dateCopied(0),
		m_datePasted(0)
	{

	}

    ~CMainTable()
	{

    }

    long m_lID;
    CString m_Desc;
    bool m_bDontAutoDelete;
    bool m_bIsGroup;
    bool m_bHasShortCut;
    bool m_bHasParent;
    CString m_QuickPaste;
	double m_clipOrder;
	double m_clipGroupOrder;
	double m_stickyClipOrder;
	double m_stickyClipGroupOrder;
	int m_dateCopied;
	int m_datePasted;

	static bool SortDesc(const CMainTable& d1, const CMainTable& d2)
	{
		double d1StickyOrder = d1.m_stickyClipOrder;
		double d2StickyOrder = d2.m_stickyClipOrder;		

		if (d1StickyOrder != d2StickyOrder)
			return d1StickyOrder > d2StickyOrder;

		if (d1.m_bIsGroup != d2.m_bIsGroup)
			return d1.m_bIsGroup < d2.m_bIsGroup;

		return d1.m_clipOrder > d2.m_clipOrder;
	}

	static bool GroupSortDesc(const CMainTable& d1, const CMainTable& d2)
	{
		double d1StickyOrder = d1.m_stickyClipGroupOrder;

		double d2StickyOrder = d2.m_stickyClipGroupOrder;

		if (d1StickyOrder != d2StickyOrder)
			return d1StickyOrder > d2StickyOrder;

		if (d1.m_bIsGroup != d2.m_bIsGroup)
			return d1.m_bIsGroup < d2.m_bIsGroup;

		return d1.m_clipGroupOrder > d2.m_clipGroupOrder;
	}
};


typedef std::map < int, CMainTable > MainTypeMap;
typedef std::map < int, CClipFormatQListCtrl > CF_DibTypeMap;
typedef std::map < int, char > CF_NoDibTypeMap;



/////////////////////////////////////////////////////////////////////////////
// CQPasteWnd window

class CQPasteWnd: public CWndEx
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
    virtual BOOL Create(CRect rect, CWnd *pParentWnd);
    virtual BOOL PreTranslateMessage(MSG *pMsg);

	bool CheckActions(MSG * pMsg);

	//}}AFX_VIRTUAL

    // Implementation
public:
    bool Add(const CString &csHeader, const CString &csText, int nID);
    virtual ~CQPasteWnd();

    void UpdateFont();

    //protected:
    CQListCtrl m_lstHeader;

    CAlphaBlend m_Alpha;
	//CEditWithButton m_search;
	CSymbolEdit m_search;
    CFont m_SearchFont;
    bool m_bHideWnd;
    CString m_strSQLSearch;
	CString m_strSearch;
    CGroupStatic m_stGroup;
    CFont m_groupFont;
    CString m_Title;
    CGroupTree m_GroupTree;
    CGdipButton m_ShowGroupsFolderBottom;
    CGdipButton m_BackButton;
	CGroupStatic m_alwaysOnToWarningStatic;
	CGdipButton m_systemMenu;
	CGroupStatic m_noSearchResultsStatic;

    CString m_SQL;
    CString m_CountSQL;
    long m_lRecordCount;
    bool m_bStopQuery;
    bool m_bHandleSearchTextChange;
    bool m_bModifersMoveActive;

    CQPasteWndThread m_thread;
	CQPasteWndThread m_extraDataThread;
	std::vector<CMainTable> m_listItems;

	std::list<CPoint> m_loadItems;
    std::list<CClipFormatQListCtrl> m_ExtraDataLoadItems;
    CF_DibTypeMap m_cf_dibCache;
	CF_NoDibTypeMap m_cf_NO_dibCache;
    CF_DibTypeMap m_cf_rtfCache;
	CF_NoDibTypeMap m_cf_NO_rtfCache;
    CCriticalSection m_CritSection;
    CAccels m_actions;
	CAccels m_toolTipActions;
	CAccels m_modifierKeyActions;
	bool m_showScrollBars;
	int m_leftSelectedCompareId;
	INT64 m_extraDataCounter;
	CPopup m_popupMsg;
	CCustomFriendsHelper m_customFriendsHelper;
	bool m_noSearchResults;

    void RefreshNc();
    void UpdateStatus(bool bRepaintImmediately = false); // regenerates the status (caption) text
    BOOL FillList(CString csSQLSearch = "");
    BOOL HideQPasteWindow(bool releaseFocus, bool clearSearchData = true);
    BOOL ShowQPasteWindow(BOOL bFillList = TRUE);
    void MoveControls();

    void DeleteSelectedRows();

	BOOL OpenID(int id, CSpecialPasteOptions pasteOptions);
	BOOL OpenSelection(CSpecialPasteOptions pasteOptions);
    BOOL OpenIndex(int item);
	BOOL NewGroup(bool bGroupSelection = true, int parentId = -1);

    CString LoadDescription(int nItem);
    bool SaveDescription(int nItem, CString text);

    //Menu Items
    void SetLinesPerRow(int lines, bool force, bool resetListCount);
    void SetTransparency(int percent);
    void OnUpdateLinesPerRow(CCmdUI *pCmdUI, int nValue);
    void OnUpdateTransparency(CCmdUI *pCmdUI, int nValue);
    void SetMenuChecks(CMenu *pMenu);
    void SetSendToMenu(CMenu *pMenu, int nMenuID, int nArrayPos);
	void SetFriendChecks(CMenu *pMenu);

    BOOL SendToFriendbyPos(int nPos, CString override_IP_Host);

    bool InsertNextNRecords(int nEnd);

    CString GetDisplayText(int lDontAutoDelete, int lShortCut, bool bIsGroup, int lParentID, CString csText);

    void FillMainTable(CMainTable &table, CppSQLite3Query &q);
    void RunThread();
    void MoveSelection(bool down, bool requireModifersActive);
    void OnKeyStateUp();
    void SetKeyModiferState(bool bActive);
	void SaveWindowSize();
	void SelectFocusID();
	void HideMenuGroup(CMenu* menu, CString text);
	void SetSearchImages();
	void RemoveStickyInternal(int id, bool &sort);

	DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void OnDragLeave();
	COleDropTarget *m_pDropTarget;

	bool DoAction(CAccel a);
	bool DoAction(DWORD cmd);
	bool DoActionShowDescription();
	bool DoActionNextDescription(); 
	bool DoActionPrevDescription();
	bool DoActionShowMenu();
	bool DoActionNewGroup();
	bool DoActionNewGroupSelection();
	bool DoActionToggleFileLogging();
	bool DoActionToggleOutputDebugString();
	bool DoActionCloseWindow();
	bool DoActionNextTabControl();
	bool DoActionPrevTabControl();
	bool DoActionShowGroups();
	bool DoActionNewClip();
	bool DoActionEditClip();
	bool DoActionMoveSelectionDown();
	bool DoActionToggleDescriptionWordWrap();
	bool DoActionApplyLastSearch();
	bool DoActionToggleSearchMethod();
	bool DoActionPasteScript(CString scriptGuid);
	bool DoActionMoveSelectionUp();
	bool DoModifierActiveActionSelectionUp();
	bool DoModifierActiveActionSelectionDown();
	bool DoModifierActiveActionMoveFirst();
	bool DoModifierActiveActionMoveLast();
	bool DoActionCancelFilter();
	bool DoActionHomeList();
	bool DoActionBackGroup();
	bool DoActionToggleShowPersistant();
	bool DoActionDeleteSelected();
	bool DoActionPasteSelected();
	bool DoActionClipProperties();
	bool DoActionPasteSelectedPlainText();
	bool DoActionMoveClipToGroup();
	bool DoActionElevatePrivleges();
	bool DoShowInTaskBar();	
	bool DoClipCompare();
	bool DoSelectLeftSideCompare();
	bool DoSelectRightSideAndDoCompare();
	bool DoExportToQRCode();
	bool DoExportToTextFile();
	bool DoExportToGoogleTranslate();
	bool DoExportToBitMapFile();
	bool DoSaveCurrentClipboard();
	bool DoMoveClipDown();
	bool DoMoveClipUp();
	bool DoMoveClipTOP();
	bool DoMoveClipLast();
	bool DoFilterOnSelectedClip();
	bool DoPasteUpperCase();
	bool DoPasteLowerCase();
	bool DoPasteCapitalize();
	bool DoPasteSentenceCase();
	bool DoPasteRemoveLineFeeds();
	bool DoPastePlusAddLineFeed();
	bool DoPasteAddTwoLineFeeds();
	bool DoPasteTypoglycemia();
	bool DoPasteAddCurrentTime();
	bool OnShowFirstTenText();
	bool OnShowClipWasPasted();
	bool OnToggleLastGroupToggle();
	bool OnMakeTopSticky(bool forceSort);
	bool OnMakeLastSticky();
	bool OnRemoveStickySetting();	
	bool DoActionReplaceTopStickyClip();
	bool DoActionPromptSendToFriend();
	bool DoActionSaveCF_HDROP_FileData();
	bool DoActionToggleClipboardConnection();
	bool DoActionPasteDontMoveClip();
	bool DoActionPasteTrimWhiteSpace();
	bool DoActionToggleTransparency();
	bool DoActionIncreaseTransparency();
	bool DoActionDecreaseTransparency();

	bool OnNewClip();
	bool OnImportClip();
	bool OnDeleteClipData();
	bool OnGlobalHotkyes();
	
	void UpdateMenuShortCut(CCmdUI *pCmdUI, DWORD action);

	bool ShowProperties(int id, int row);
	bool DeleteClips(CClipIDs &IDs, ARRAY &Indexs);
	void RemoveFromImageRtfCache(int id);
	bool SyncClipDataToArrayData(CClip &clip);
	bool SelectIds(ARRAY &ids);

	void LoadShortcuts();

	void ShowRightClickMenu();

	void SetCurrentTransparency();

    // Generated message map functions
protected:
    //{{AFX_MSG(CQPasteWnd)
    DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnKillFocus(CWnd *pOldWnd);
    afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
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
    afx_msg void OnMenuDelete();
    afx_msg void OnMenuPositioningAtcaret();
    afx_msg void OnMenuPositioningAtcursor();
    afx_msg void OnMenuPositioningAtpreviousposition();
    afx_msg void OnMenuOptions();
    afx_msg LRESULT OnCancelFilter(WPARAM wParam, LPARAM lParam);
    afx_msg void OnMenuExitprogram();
    afx_msg void OnMenuToggleConnectCV();
    afx_msg void OnMenuProperties();
    afx_msg void OnClose();
    afx_msg void OnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void GetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnFindItem(NMHDR *pNMHDR, LRESULT *pResult);
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
    afx_msg void OnMenuQuickpropertiesSettoneverautodelete();
    afx_msg void OnMenuQuickpropertiesAutodelete();
    afx_msg void OnMenuQuickpropertiesRemovehotkey();
    afx_msg void OnMenuSenttoFriendEight();
    afx_msg void OnMenuSenttoFriendEleven();
    afx_msg void OnMenuSenttoFriendFifteen();
    afx_msg void OnMenuSenttoFriendFive();
    afx_msg void OnMenuSenttoFriendFore();
    afx_msg void OnMenuSenttoFriendForeteen();
    afx_msg void OnMenuSenttoFriendNine();
    afx_msg void OnMenuSenttoFriendSeven();
    afx_msg void OnMenuSenttoFriendSix();
    afx_msg void OnMenuSenttoFriendTen();
    afx_msg void OnMenuSenttoFriendThirteen();
    afx_msg void OnMenuSenttoFriendThree();
    afx_msg void OnMenuSenttoFriendTwelve();
    afx_msg void OnMenuSenttoFriendTwo();
    afx_msg void OnMenuSenttoFriendone();
    afx_msg void OnMenuSenttoPromptforip();
    afx_msg void OnMenuGroupsMovetogroup();
    afx_msg void OnMenuPasteplaintextonly();
    afx_msg void OnMenuHelp();
    afx_msg void OnMenuQuickoptionsFont();
    afx_msg void OnMenuQuickoptionsShowthumbnails();
    afx_msg void OnMenuQuickoptionsDrawrtftext();
    afx_msg void OnMenuQuickoptionsPasteclipafterselection();
    afx_msg void OnSearchEditChange();
    afx_msg void OnMenuQuickoptionsFindasyoutype();
    afx_msg void OnMenuQuickoptionsEnsureentirewindowisvisible();
    afx_msg void OnMenuQuickoptionsShowclipsthatareingroupsinmainlist();
    afx_msg void OnMenuPastehtmlasplaintext();
    afx_msg void OnPromptToDeleteClip();
    afx_msg void OnUpdateMenuNewgroup(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuNewgroupselection(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuAllwaysontop(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuViewfulldescription(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuViewgroups(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuPasteplaintextonly(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuDelete(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuProperties(CCmdUI *pCmdUI);
    afx_msg void OnDestroy();
    afx_msg LRESULT OnSearchEnterKeyPressed(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnListEnd(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSearch(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDelete(WPARAM wParam, LPARAM lParam);
    afx_msg void OnGetToolTipText(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg LRESULT OnListSelect_DB_ID(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListMoveSelectionToGroup(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRefreshView(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReloadClipAfterPaste(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGroupTreeMessage(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFillRestOfList(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRefeshRow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetListCount(WPARAM wParam, LPARAM lParam);
    afx_msg HBRUSH CtlColor(CDC *pDC, UINT nCtlColor);
    afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
    afx_msg void OnViewcaptionbaronRight();
    afx_msg void OnViewcaptionbaronBottom();
    afx_msg void OnViewcaptionbaronLeft();
    afx_msg void OnViewcaptionbaronTop();
    afx_msg void OnMenuAutohide();
    afx_msg void OnMenuViewfulldescription();
    afx_msg void OnMenuAllwaysontop();
    afx_msg void OnMenuNewGroup();
    afx_msg void OnMenuNewGroupSelection();
    afx_msg void OnBackButton();
	afx_msg void OnSystemButton();
    afx_msg LRESULT OnUpDown(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnItemDeleted(WPARAM wParam, LPARAM lParam);
    LRESULT OnToolTipWndInactive(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnMenuExport();
    afx_msg void OnMenuImport();
    afx_msg void OnQuickpropertiesRemovequickpaste();
    afx_msg void OnMenuEdititem();
    afx_msg void OnMenuNewclip();
    afx_msg void OnUpdateMenuEdititem(CCmdUI *pCmdUI);
    afx_msg void OnUpdateMenuNewclip(CCmdUI *pCmdUI);
    afx_msg void OnAddinSelect(UINT id);
	afx_msg void OnCustomSendToFriend(UINT idIn);
	afx_msg void OnChaiScriptPaste(UINT idIn);
    afx_msg LRESULT OnSelectAll(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowHideScrollBar(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuSearchDescription();
	afx_msg void OnMenuSearchFullText();
	afx_msg void OnMenuSearchQuickPaste();
	afx_msg void OnMenuSimpleTextSearch();
	afx_msg LRESULT OnPostOptions(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMakeTopStickyClip();
	afx_msg void OnMakeLastStickyClip();
	afx_msg void OnRemoveSticky();
	afx_msg void OnElevateAppToPasteIntoElevatedApp();
	
public:
	afx_msg void OnQuickoptionsShowintaskbar();
	afx_msg void OnMenuViewasqrcode();
	afx_msg void OnExportExporttotextfile();
	afx_msg void OnCompareCompare();
	afx_msg void OnCompareSelectleftcompare();
	afx_msg void OnCompareCompareagainst();
	afx_msg void OnUpdateCompareCompare(CCmdUI *pCmdUI);
	afx_msg LRESULT OnShowProperties(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewGroup(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteId(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuRegularexpressionsearch();
	afx_msg void OnImportExporttogoogletranslate();
	afx_msg void OnUpdateImportExporttogoogletranslate(CCmdUI *pCmdUI);
	afx_msg void OnImportExportclipBitmap();
	afx_msg void OnUpdateImportExportclipBitmap(CCmdUI *pCmdUI);
	afx_msg void OnMenuWildcardsearch();
	afx_msg void OnMenuSavecurrentclipboard();
	afx_msg void OnUpdateMenuSavecurrentclipboard(CCmdUI *pCmdUI);
	afx_msg void OnCliporderMoveup();
	afx_msg void OnUpdateCliporderMoveup(CCmdUI *pCmdUI);
	afx_msg void OnCliporderMovedown();
	afx_msg void OnUpdateCliporderMovedown(CCmdUI *pCmdUI);
	afx_msg void OnCliporderMovetotop();
	afx_msg void OnUpdateCliporderMovetotop(CCmdUI *pCmdUI);
	afx_msg void OnMenuFilteron();
	afx_msg void OnUpdateMenuFilteron(CCmdUI *pCmdUI);
	afx_msg void OnAlwaysOnTopClicked();
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSpecialpasteUppercase();
	afx_msg void OnUpdateSpecialpasteUppercase(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpasteLowercase();
	afx_msg void OnUpdateSpecialpasteLowercase(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpasteCapitalize();
	afx_msg void OnUpdateSpecialpasteCapitalize(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpasteSentence();
	afx_msg void OnUpdateSpecialpasteSentence(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpasteRemovelinefeeds();
	afx_msg void OnUpdateSpecialpasteRemovelinefeeds(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpastePaste();
	afx_msg void OnUpdateSpecialpastePaste(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpastePaste32919();
	afx_msg void OnUpdateSpecialpastePaste32919(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpasteTypoglycemia();
	afx_msg void OnUpdateSpecialpasteTypoglycemia(CCmdUI *pCmdUI);
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnQuickoptionsShowtextforfirsttencopyhotkeys();
	afx_msg void OnUpdateQuickoptionsShowtextforfirsttencopyhotkeys(CCmdUI *pCmdUI);
	afx_msg void OnQuickoptionsShowindicatoracliphasbeenpasted();
	afx_msg void OnUpdateQuickoptionsShowindicatoracliphasbeenpasted(CCmdUI *pCmdUI);
	afx_msg void OnGroupsTogglelastgroup();
	afx_msg void OnUpdateGroupsTogglelastgroup(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStickyclipsMaketopstickyclip(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStickyclipsMakelaststickyclip(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStickyclipsRemovestickysetting(CCmdUI *pCmdUI);
	afx_msg void OnSpecialpastePaste32927();
	afx_msg void OnUpdateSpecialpastePaste32927(CCmdUI *pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMenuGlobalhotkeys32933();
	afx_msg void OnMenuDeleteclipdata32934();
	afx_msg void OnMenuImportclip32935();
	afx_msg void OnMenuNewclip32937();
	afx_msg void OnUpdateMenuImportclip32935(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuNewclip32937(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuGlobalhotkeys32933(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuDeleteclipdata32934(CCmdUI *pCmdUI);
	afx_msg LRESULT OnSearchFocused(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCliporderReplacetopstickyclip();
	afx_msg void OnUpdateCliporderReplacetopstickyclip(CCmdUI *pCmdUI);
	afx_msg void OnSendtoPromptforname();
	afx_msg void OnUpdateSendtoPromptforname(CCmdUI *pCmdUI);
	afx_msg void OnImportImportcopiedfile();
	afx_msg void OnUpdateImportImportcopiedfile(CCmdUI *pCmdUI);
	afx_msg void OnUpdate32775(CCmdUI *pCmdUI);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCliporderMovetolast();
	afx_msg void OnUpdateCliporderMovetolast(CCmdUI *pCmdUI);
	afx_msg LRESULT OnCopyClip(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSpecialpastePasteDontUpdateOrder();
	afx_msg void OnUpdateOnSpecialPasteDontUpdateOrder(CCmdUI *pCmdUI);

	afx_msg void OnSpecialpasteTrim();
	afx_msg void OnUpdateSpecialpasteTrim(CCmdUI *pCmdUI);
	afx_msg void OnTransparencyIncrease();
	afx_msg void OnUpdateTransparencyIncrease(CCmdUI *pCmdUI);
	afx_msg void OnTransparencyDecrease();
	afx_msg void OnUpdateTransparencyDecrease(CCmdUI *pCmdUI);
	afx_msg void OnTransparencyToggle();
	afx_msg void OnUpdateTransparencyToggle(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparencyNone(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency5(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency10(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency15(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency20(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency25(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency30(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency35(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTransparency40(CCmdUI *pCmdUI);
	afx_msg void OnTransparency35();
};