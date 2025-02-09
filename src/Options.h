#pragma once

#include "Theme.h"
#include "RegExFilterHelper.h"
#include "ChaiScriptXml.h"
#include <set>

#define MAX_SEND_CLIENTS	15
class CSendClients
{
public:
	CSendClients()
	{
		bSendAll = FALSE;
		bShownFirstError = FALSE;
	}
	BOOL bSendAll;
	CString csIP;
	CString csDescription;
	BOOL bShownFirstError;
};

#define MAX_COPY_BUFFER	3
class CCopyBufferItem
{
public:
	CCopyBufferItem()
	{
		m_lCopyHotKey = -1;
		m_lPasteHotKey = -1;
		m_lCutHotKey = -1;
		m_bPlaySoundOnCopy = FALSE;
	}
	long m_lCopyHotKey;
	long m_lPasteHotKey;
	long m_lCutHotKey;
	BOOL m_bPlaySoundOnCopy;
};

#define REG_PATH					"Software\\Ditto"

#define POS_AT_CARET	1
#define POS_AT_CURSOR	2
#define POS_AT_PREVIOUS	3

#define CAPTION_RIGHT	1
#define CAPTION_BOTTOM	2
#define CAPTION_LEFT	3
#define CAPTION_TOP		4

#define TOGGLES_ALLWAYS_ON_TOP				1
#define TOGGLES_ALLWAYS_SHOW_DESCRIPTION	2
#define	ROLLES_UP_WINDOW					3

#define PATH_HELP			0
#define PATH_REMOTE_FILES	1
#define PATH_LOG_FILE		2
#define PATH_LANGUAGE		3
#define PATH_THEMES			8
#define PATH_ADDINS			9
#define PATH_DRAG_FILES		10
#define PATH_CLIP_DIFF		11
#define PATH_RESTORE_TEMP	12
#define PATH_EDIT_CLIPS		13

class CGetSetOptions
{
public:
	CGetSetOptions();
	virtual ~CGetSetOptions();

	static bool m_bFromIni;
	static CString m_csIniFileName;
	static bool m_bInConversion;
	static CTheme m_Theme;
	static bool m_portable;
	static bool m_windowsApp;
	static bool m_chocolateyApp;

	static void LoadSettings();
	static CString GetIniFileName(bool bLocalIniFile);
	static void ConverSettingsToIni();
	static CString GetAppDataPath();
	static CString GetTempFilePath();
	static void CreateIniFile(CString path);

	static CString GetExeFileName();
	static CString GetAppName();

	static BOOL GetShowIconInSysTray();
	static BOOL SetShowIconInSysTray(BOOL bShow);

	static BOOL GetRunOnStartUp();
	static void SetRunOnStartUp(BOOL bRun);

	static BOOL SetProfileFont(CString csSection, LOGFONT &font);
	static BOOL GetProfileFont(CString csSection, LOGFONT &font);

	static long GetResolutionProfileLong(CString csName, long lDefaultValue, CString csNewPath = _T(""));
	static BOOL SetResolutionProfileLong(CString csName, long lValue);

	static BOOL SetProfileLong(CString csName, long lValue);
	static long GetProfileLong(CString csName, long lDefaultValue = -1, CString csNewPath = _T(""));

	static CString GetProfileString(CString csName, CString csDefault, CString csNewPath = _T(""), int maxSize = -1);
	static BOOL	SetProfileString(CString csName, CString csValue);

	static LPVOID GetProfileData(CString csName, DWORD &dwLength);
	static BOOL	SetProfileData(CString csName, LPVOID lpData, DWORD dwLength);

	static BOOL SetQuickPasteSize(CSize size);
	static void GetQuickPasteSize(CSize &size);

	static BOOL SetQuickPastePoint(CPoint point);
	static void GetQuickPastePoint(CPoint &point);

	static BOOL SetEnableTransparency(BOOL bCheck);
	static BOOL GetEnableTransparency();

	static BOOL SetTransparencyPercent(long lPercent);
	static long GetTransparencyPercent();

	static long m_nLinesPerRow;
	static BOOL SetLinesPerRow(long lLines);
	static long GetLinesPerRow();

	static BOOL SetQuickPastePosition(long lPosition);
	static long GetQuickPastePosition();

	static long GetCopyGap();
	static void SetCopyGap(long lGap);

	static BOOL SetDBPath(CString csPath);
	static CString GetDBPath(bool resolvePath = true);
	static CString ResolvePath(CString path);

	static BOOL SetDBPathOld(CString csPath);
	static CString GetDBPathOld();

	static void SetCheckForMaxEntries(BOOL bVal);
	static BOOL GetCheckForMaxEntries();

	static void SetCheckForExpiredEntries(BOOL bVal);
	static BOOL GetCheckForExpiredEntries();

	static void SetMaxEntries(long lVal);
	static long GetMaxEntries();

	static void SetExpiredEntries(long lVal);
	static long GetExpiredEntries();

	static void SetTripCopyCount(long lVal);
	static long GetTripCopyCount();

	static void SetTripPasteCount(long lVal);
	static long GetTripPasteCount();

	static void SetTripDate(long lDate);
	static long GetTripDate();

	static void SetTotalCopyCount(long lVal);
	static long GetTotalCopyCount();

	static void SetTotalPasteCount(long lVal);
	static long GetTotalPasteCount();

	static void SetTotalDate(long lDate);
	static long GetTotalDate();

	static CString	GetUpdateFilePath();
	static BOOL		SetUpdateFilePath(CString cs);

	static CString	GetUpdateInstallPath();
	static BOOL		SetUpdateInstallPath(CString cs);

	static long		GetLastUpdate();
	static long		SetLastUpdate(long lValue);

	static BOOL		GetCheckForUpdates();
	static BOOL		SetCheckForUpdates(BOOL bCheck);

	static BOOL		m_bUseCtrlNumAccel;
	static void		SetUseCtrlNumForFirstTenHotKeys(BOOL bVal);
	static BOOL		GetUseCtrlNumForFirstTenHotKeys();

	static BOOL		m_bAllowDuplicates;
	static void		SetAllowDuplicates(BOOL bVal);
	static BOOL		GetAllowDuplicates();

	static BOOL		m_bUpdateTimeOnPaste;
	static void		SetUpdateTimeOnPaste(BOOL bVal);
	static BOOL		GetUpdateTimeOnPaste();

	static BOOL		m_bSaveMultiPaste;
	static void		SetSaveMultiPaste(BOOL bVal);
	static BOOL		GetSaveMultiPaste();

	static BOOL		m_bShowPersistent;
	static void		SetShowPersistent(BOOL bVal);
	static BOOL		GetShowPersistent();

	static void		SetShowTextForFirstTenHotKeys(BOOL bVal);
	static BOOL		GetShowTextForFirstTenHotKeys();

	static void		SetMainHWND(long lhWnd);
	static long		GetMainHWND();

	static void		SetCaptionPos(long lPos);
	static long		GetCaptionPos();

	static void		SetAutoHide(BOOL bAutoHide);
	static BOOL		GetAutoHide();

	static long		m_bDescTextSize;
	static void		SetDescTextSize(long lSize);
	static long		GetDescTextSize();

	static BOOL		m_bDescShowLeadingWhiteSpace;
	static void		SetDescShowLeadingWhiteSpace(BOOL bVal);
	static BOOL		GetDescShowLeadingWhiteSpace();

	static BOOL		m_bAllwaysShowDescription;
	static void		SetAllwaysShowDescription(long bShow);
	static BOOL		GetAllwaysShowDescription();

	static long		m_bDoubleClickingOnCaptionDoes;
	static void		SetDoubleClickingOnCaptionDoes(long lOption);
	static long		GetDoubleClickingOnCaptionDoes();

	static BOOL		m_bPrompForNewGroupName;
	static void		SetPrompForNewGroupName(BOOL bOption);
	static BOOL		GetPrompForNewGroupName();

	static BOOL		m_bSendPasteOnFirstTenHotKeys;
	static void		SetSendPasteOnFirstTenHotKeys(BOOL bOption);
	static BOOL		GetSendPasteOnFirstTenHotKeys();

	static CSendClients m_SendClients[MAX_SEND_CLIENTS];
	static long		m_lAutoSendClientCount;
	static void		GetClientSendCount();
	static void		SetSendClients(CSendClients Client, int nPos);
	static CSendClients		GetSendClients(int nPos);

	static CString m_csIPListToPutOnClipboard;
	static CString	GetListToPutOnClipboard();
	static BOOL		SetListToPutOnClipboard(CString cs);

	static BOOL		m_bLogSendReceiveErrors;
	static void		SetLogSendReceiveErrors(BOOL bOption);
	static BOOL		GetLogSendReceiveErrors();

	static BOOL		m_HideDittoOnHotKeyIfAlreadyShown;
	static BOOL		GetHideDittoOnHotKeyIfAlreadyShown();
	static void		SetHideDittoOnHotKeyIfAlreadyShown(BOOL bVal);

	static long		m_lPort;
	static void		SetPort(long lPort);
	static long		GetPort();

	static BOOL		GetDisableRecieve();
	static void		SetDisableRecieve(BOOL bVal);

	static BOOL		GetFont(LOGFONT &font);
	static void		SetFont(LOGFONT &font);

	static BOOL		m_bDrawThumbnail;
	static void		SetDrawThumbnail(long bDraw);
	static BOOL		GetDrawThumbnail();

	static BOOL		m_bFastThumbnailMode;
	static void		SetFastThumbnailMode(BOOL bval);
	static BOOL		GetFastThumbnailMode();

	static CStringA	m_csPassword;
	static void		SetNetworkPassword(CString csPassword);
	static CStringA	GetNetworkPassword();

	static BOOL		m_bDrawRTF;
	static void		SetDrawRTF(long bDraw);
	static BOOL		GetDrawRTF();

	static BOOL		m_bMultiPasteReverse;
	static void		SetMultiPasteReverse(BOOL bVal);
	static BOOL		GetMultiPasteReverse();

	static CString	m_csPlaySoundOnCopy;
	static void		SetPlaySoundOnCopy(CString cs);
	static CString	GetPlaySoundOnCopy();

	static BOOL		m_bSendPasteMessageAfterSelection;
	static void		SetSendPasteAfterSelection(BOOL bVal);
	static BOOL		GetSendPasteAfterSelection();

	static BOOL		m_bFindAsYouType;
	static void		SetFindAsYouType(BOOL bVal);
	static BOOL		GetFindAsYouType();

	static BOOL		m_bEnsureEntireWindowCanBeSeen;
	static void		SetEnsureEntireWindowCanBeSeen(BOOL bVal);
	static BOOL		GetEnsureEntireWindowCanBeSeen();

	static BOOL		m_bShowAllClipsInMainList;
	static void		SetShowAllClipsInMainList(BOOL bVal);
	static BOOL		GetShowAllClipsInMainList();

	static void		SetExtraNetworkPassword(CString csPassword);
	static CString	GetExtraNetworkPassword(bool bFillArray);

	static long		m_lMaxClipSizeInBytes;
	static long		GetMaxClipSizeInBytes();
	static void		SetMaxClipSizeInBytes(long lSize);

	static CString	GetLanguageFile();
	static void		SetLanguageFile(CString csLanguage);

	static DWORD	m_dwSaveClipDelay;
	static ULONG	GetSaveClipDelay();
	static void		SetSaveClipDelay(DWORD dwDelay);

	static long		m_lProcessDrawClipboardDelay;
	static long		GetProcessDrawClipboardDelay();
	static void		SetProcessDrawClipboardDelay(long lDelay);

	static BOOL		m_bEnableDebugLogging;
	static BOOL		GetEnableDebugLogging();
	static void		SetEnableDebugLogging(BOOL bEnable);

	static BOOL		m_bEnsureConnectToClipboard;
	static BOOL		GetEnsureConnectToClipboard();
	static void		SetEnsureConnectToClipboard(BOOL bSet);

	static BOOL		GetPromptWhenDeletingClips();
	static void		SetPromptWhenDeletingClips(BOOL bSet);

	static CString	GetLastImportDir();
	static void		SetLastImportDir(CString csDir);

	static CString	GetLastExportDir();
	static void		SetLastExportDir(CString csDir);

	static BOOL		GetUpdateDescWhenSavingClip();
	static void		SetUpdateDescWhenSavingClip(BOOL bSet);

	static BOOL		m_outputDebugStringLogging;
	static BOOL		GetEnableOutputDebugStringLogging();
	static void		SetEnableOutputDebugStringLogging(BOOL bSet);

	static CStringArray m_csNetworkPasswordArray;

	static CString  GetPath(long lPathID);

	static __int64	nLastDbWriteTime;

	static long		GetDittoRestoreClipboardDelay();
	static void		SetDittoRestoreClipboardDelay(long lDelay);

	static void		GetCopyBufferItem(int nPos, CCopyBufferItem &Item);
	static void		SetCopyBufferItem(int nPos, CCopyBufferItem &Item);

	static CString  GetMultiPasteSeparator(bool bConvertToLineFeeds = true);
	static void		SetMultiPasteSeparator(CString csSep);

	static BOOL		GetSetCurrentDirectory();

	static CString GetPasteString(CString csAppName);

	static CString GetDefaultPasteString();
	static void SetDefaultPasteString(CString val);

	static CString GetCopyString(CString csAppName);
	static CString GetDefaultCopyString();
	static void SetDefaultCopyString(CString val);

	static CString GetCutString(CString csAppName);
	static CString GetDefaultCutString();
	static void SetDefaultCutString(CString val);

	static BOOL	GetEditWordWrap();
	static void	SetEditWordWrap(BOOL bSet);

	static void SetAllowFriends(BOOL val);
	static bool GetAllowFriends();

	static bool		GetIsPortableDitto();
	static bool		GetIsWindowsApp();
	static bool		GetIsChocolateyApp();

	static long		GetAutoMaxDelay();
	static void		SetAutoMaxDelay(long lDelay);

	static void SetTheme(CString csTheme);
	static CString GetTheme();

	static long		GetKeyStateWaitTimerCount();
	static long		GetKeyStatePasteDelay();

	static DWORD	GetDittoHotKey();
	
	static DWORD	SendKeysDelay();
	static void		SetSendKeysDelay(DWORD val);

	static DWORD	RealSendKeysDelay();
	static void		SetRealSendKeysDelay(DWORD val);

	static DWORD	WaitForActiveWndTimeout();
	static DWORD	FocusChangedDelay();
	static DWORD	FocusWndTimerTimeout();

	static BOOL		GetConnectedToClipboard();
	static void		SetConnectedToClipboard(BOOL val);

	static DWORD	GetTextOnlyRestoreDelay();
	static DWORD 	GetTextOnlyPasteDelay();

	static BOOL		GetSetFocusToApp(CString csAppName);

	static DWORD	SelectedIndex();
	static void		SetSelectedIndex(int val);

	static void		SetCopyAppInclude(CString csAppName);
	static CString  GetCopyAppInclude();

	static void		SetCopyAppExclude(CString csAppName);
	static CString  GetCopyAppExclude();

	static CString  GetCopyAppSeparator();

	static DWORD	GetNoFormatsRetryDelay();

	static DWORD	GetMainDeletesDeleteCount();

	static DWORD	GetIdleSecondsBeforeDelete();

	static DWORD	GetDbTimeout();

	static DWORD	GetFunnyTickCountAdjustment();

	static DWORD	GetMinIdleTimeBeforeTrackFocus();

	static DWORD	GetTimeBeforeExpandWindow();

	static DWORD	GetUseGuiThreadInfoForFocus();

	static void		SetSearchDescription(BOOL val);
	static BOOL		GetSearchDescription();

	static void		SetSearchFullText(BOOL val);
	static BOOL		GetSearchFullText();

	static void		SetSearchQuickPaste(BOOL val);
	static BOOL		GetSearchQuickPaste();

	static void		SetSimpleTextSearch(BOOL val);
	static BOOL		GetSimpleTextSearch();

	static void		SetMoveClipsOnGlobal10(BOOL val);
	static BOOL		GetMoveClipsOnGlobal10();

	static void		SetShowScrollBar(BOOL val);
	static BOOL		GetShowScrollBar();
	static BOOL		m_showScrollBar;

	static void		SetPasteAsAdmin(BOOL val);
	static BOOL		GetPasteAsAdmin();

	static void		SetRememberDescPos(BOOL val);
	static BOOL		GetRememberDescPos();

	static void		SetSizeDescWindowToContent(BOOL val);
	static BOOL		GetSizeDescWindowToContent();

	static void		SetScaleImagesToDescWindow(BOOL val);
	static BOOL		GetScaleImagesToDescWindow();

	static void		SetDescWndPoint(CPoint point);
	static void		GetDescWndPoint(CPoint &point);

	static void		SetDescWndSize(CSize size);
	static void		GetDescWndSize(CSize &size);

	static void		SetShowInTaskBar(BOOL val);
	static BOOL		GetShowInTaskBar();

	static void		SetDiffApp(CString val);
	static CString	GetDiffApp();

	static void		SetQRCodeBorderPixels(int val);
	static int	GetQRCodeBorderPixels();

	static BOOL GetRegExTextSearch();
	static void SetRegExTextSearch(BOOL val);

	static CString GetTranslateUrl();

	static void SetNetworkReadTimeoutMS(int val);
	static int GetNetworkReadTimeoutMS();

	static void SetRequestFilesUsingIP(int val);
	static int GetRequestFilesUsingIP();

	static int ReadRandomFileInterval();
	static int ReadRandomFileIdleMin();

	static BOOL GetShowGroupsInMainList();
	static void SetShowGroupsInMainList(BOOL val);

	static void SetGroupDoubleClickTimeMS(int val);
	static int GetGroupDoubleClickTimeMS();

	static void SetSaveToGroupTimeoutMS(int val);
	static int GetSaveToGroupTimeoutMS();

	static void SetCopyReasonTimeoutMS(int val);
	static int GetCopyReasonTimeoutMS();

	static void SetWindowsResumeDelayReOpenDbMS(int val);
	static int GetWindowsResumeDelayReOpenDbMS();

	static BOOL GetShowMsgWndOnCopyToGroup();
	static void SetShowMsgWndOnCopyToGroup(BOOL val);

	static int GetActionShortCutA(DWORD action, int pos, CString refData = _T(""));
	static void SetActionShortCutA(int action, DWORD shortcut, int pos, CString refData = _T(""));

	static int GetActionShortCutB(DWORD action, int pos, CString refData = _T(""));
	static void SetActionShortCutB(int action, DWORD shortcut, int pos, CString refData = _T(""));

	static BOOL	m_bShowAlwaysOnTopWarning;
	static BOOL GetShowAlwaysOnTopWarning();
	static void SetShowAlwaysOnTopWarning(BOOL show);
	
	static BOOL GetUseIPFromAccept();
	static void SetUseIPFromAccept(BOOL useAccept);

	static int GetDragId();
	static void SetDragId(int id);

	static BOOL GetShowIfClipWasPasted();
	static void SetShowIfClipWasPasted(BOOL val);

	static int GetLastGroupToggle();
	static void SetLastGroupToggle(int val);

	static BOOL GetMouseClickHidesDescription();
	static void SetMouseClickHidesDescription(int val);

	static BOOL GetWrapDescriptionText();
	static void SetWrapDescriptionText(int val);

	static BOOL GetUseUISelectedGroupForLastTenCopies();
	static void SetUseUISelectedGroupForLastTenCopies(int val);

	static int GetDelayRenderLockout();
	static void SetDelayRenderLockout(int val);

	static BOOL GetAdjustClipsForCRC();
	static void SetAdjustClipsForCRC(int val);

	static BOOL GetCheckMd5OnFileTransfers();
	static void SetCheckMd5OnFileTransfers(int val);

	static int GetBalloonTimeout();
	static void SetBalloonTimeout(int val);

	static void		SetCustomSendToList(CString val);
	static CString	GetCustomSendToList();

	static int GetMaxFileContentsSize();
	static void SetMaxFileContentsSize(int val);

	static int GetErrorMsgPopupTimeout();
	static void SetErrorMsgPopupTimeout(int val);

	static CRegExFilterHelper m_regexHelper;
	static void		SetRegexFilter(CString val, int pos);
	static CString	GetRegexFilter(int pos);

	static void SetRegexFilterByProcessName(CString val, int pos);
	static CString GetRegexFilterByProcessName(int pos);

	static BOOL GetOpenToGroupByActiveExe();
	static void SetOpenToGroupByActiveExe(int val);

	static BOOL GetShowStartupMessage();
	static void SetShowStartupMessage(int val);

	static CChaiScriptXml m_copyScripts;
	static CString GetCopyScriptsXml();
	static void SetCopyScriptsXml(CString val);

	static CChaiScriptXml m_pasteScripts;
	static CString GetPasteScriptsXml();
	static void SetPasteScriptsXml(CString val);

	static long m_tooltipTimeout;
	static long GetToolTipTimeout();
	static void SetToolTipTimeout(int long);

	static CString GetPastSearchXml();
	static void SetPastSearchXml(CString val);

	static BOOL GetShowMsgWhenReceivingManualSentClip();
	static void SetShowMsgWhenReceivingManualSentClip(BOOL val);

	static BOOL m_cleanRTFBeforeDrawing;
	static BOOL GetCleanRTFBeforeDrawing();
	static void SetCleanRTFBeforeDrawing(BOOL val);

	static BOOL GetDisableExpireClipsConfig();
	static void SetDisableExpireClipsConfig(BOOL val);

	static BOOL GetRevertToTopLevelGroup();
	static void SetRevertToTopLevelGroup(BOOL val);

	static BOOL GetUpdateClipOrderOnCtrlC();
	static void SetUpdateClipOrderOnCtrlC(BOOL val);

	static int GetMaxToolTipLines();
	static void SetMaxToolTipLines(int val);

	static int GetMaxToolTipCharacters();
	static void SetMaxToolTipCharacters(int val);

	static int m_doubleKeyStrokeTimeout;
	static int GetDoubleKeyStrokeTimeout();
	static void SetDoubleKeyStrokeTimeout(int val);

	static int m_firstTenHotKeysStart;
	static int GetFirstTenHotKeysStart();
	static void SetFirstTenHotKeysStart(int val);

	static int m_firstTenHotKeysFontSize;
	static int GetFirstTenHotKeysFontSize();
	static void SetFirstTenHotKeysFontSize(int val);

	static BOOL GetAddCFHDROP_OnDrag();
	static void SetAddCFHDROP_OnDrag(BOOL val);

	static int GetCopyAndSveDelay();
	static void SetCopyAndSveDelay(int val);

	static int GetEditorDefaultFontSize();
	static void SetEditorDefaultFontSize(int val);

	static BOOL m_moveSelectionOnOpenHotkey;
	static BOOL GetMoveSelectionOnOpenHotkey();
	static void SetMoveSelectionOnOpenHotkey(BOOL val);

	static BOOL m_allowBackToBackDuplicates;
	static BOOL GetAllowBackToBackDuplicates();
	static void SetAllowBackToBackDuplicates(BOOL val);

	static BOOL m_maintainSearchView;
	static BOOL GetMaintainSearchView();
	static void SetMaintainSearchView(BOOL val);

	static CString GetNetworkBindIPAddress();
	static void SetNetworkBindIPAddress(CString val);

	static CString m_tempDragFileName;
	static CTime m_tempDragFileNameSetTime;
	static CString GetTempDragFileName();
	static void SetTempDragFileName(CString val);

	static BOOL m_refreshViewAfterPasting;
	static BOOL GetRefreshViewAfterPasting();
	static void SetRefreshViewAfterPasting(BOOL val);

	static CString GetSlugifySeparator();
	static void SetSlugifySeparator(CString val);

	static BOOL m_supportAllTypes;
	static BOOL GetSupportAllTypes();
	static void SetSupportAllTypes(BOOL val);

	static CString GetIgnoreAnnoyingCFDIB(BOOL useCache = FALSE);
	static CString m_ignoreAnnoyingCFDIB;
	static void SetIgnoreAnnoyingCFDIB(CString val);
	static set<CString> GetIgnoreAnnoyingCFDIBSet(BOOL useCache = FALSE);

	static BOOL GetRegexCaseInsensitive();
	static void SetRegexCaseInsensitive(BOOL val);

	static BOOL		m_bDrawCopiedColorCode;
	static void		SetDrawCopiedColorCode(long bDraw);
	static BOOL		GetDrawCopiedColorCode();


	static BOOL m_centerWindowBelowCursorOrCaret;
	static void SetCenterWindowBelowCursorOrCaret(BOOL center);
	static BOOL GetCenterWindowBelowCursorOrCaret();

	static BOOL SetTextEditorPath(CString path);
	static CString GetTextEditorPath();

	static BOOL SetRTFEditorPath(CString path);
	static CString GetRTFEditorPath();

	static BOOL SetQRCodeUrl(CString path);
	static CString GetQRCodeUrl();

	static void SetAppendRemoveComputerNameAndIPToDescription(BOOL val);
	static BOOL GetAppendRemoveComputerNameAndIPToDescription();

	static void SetPreferUtf8ForCompare(BOOL val);
	static BOOL GetPreferUtf8ForCompare();
};

// global for easy access and for initialization of fast access variables
extern CGetSetOptions g_Opt; 