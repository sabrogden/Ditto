#pragma once

#include "Theme.h"

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
#define PATH_UPDATE_FILE	4
#define PATH_DATABASE		5
#define PATH_INI			6
#define PATH_U3_HWND_INI	7
#define PATH_THEMES			8
#define PATH_ADDINS			9
#define PATH_DRAG_FILES		10
#define PATH_CLIP_DIFF		11

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

	static CString GetProfileString(CString csName, CString csDefault, CString csNewPath = _T(""));
	static BOOL	SetProfileString(CString csName, CString csValue);

	static LPVOID GetProfileData(CString csName, DWORD &dwLength);
	static BOOL	SetProfileData(CString csName, LPVOID lpData, DWORD dwLength);

	static BOOL SetQuickPasteSize(CSize size);
	static void GetQuickPasteSize(CSize &size);

	static BOOL SetQuickPastePoint(CPoint point);
	static void GetQuickPastePoint(CPoint &point);

	static BOOL SetEditWndSize(CSize size);
	static void GetEditWndSize(CSize &size);

	static BOOL SetEditWndPoint(CPoint point);
	static void GetEditWndPoint(CPoint &point);

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
	static CString GetDBPath();

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
	static BOOL		GetMainHWND();

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

	static bool		m_bOutputDebugString;

	static CStringArray m_csNetworkPasswordArray;

	static CString  GetPath(long lPathID);

	static void		WriteU3Hwnd(HWND hWnd);
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

	static CString GetCopyString(CString csAppName);
	static CString GetDefaultCopyString();

	static CString GetCutString(CString csAppName);
	static CString GetDefaultCutString();

	static BOOL	GetEditWordWrap();
	static void	SetEditWordWrap(BOOL bSet);

	static bool GetAllowFriends();

	static bool		GetIsPortableDitto();
	static bool		GetIsWindowsApp();

	static long		GetAutoMaxDelay();
	static void		SetAutoMaxDelay(long lDelay);

	static void SetTheme(CString csTheme);
	static CString GetTheme();

	static long		GetKeyStateWaitTimerCount();
	static long		GetKeyStatePasteDelay();

	static DWORD	GetDittoHotKey();
	static DWORD	SendKeysDelay();
	static DWORD	WaitForActiveWndTimeout();
	static DWORD	FocusChangedDelay();
	static DWORD	FocusWndTimerTimeout();

	static BOOL		GetConnectedToClipboard();
	static void		SetConnectedToClipboard(BOOL val);

	static DWORD	GetTextOnlyRestoreDelay();
	static DWORD 	GetTextOnlyPasteDelay();

	static BOOL		GetSetFocusToApp(CString csAppName);

	static DWORD	SelectedIndex();

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

	static int GetActionShortCutA(DWORD action, int pos);
	static void SetActionShortCutA(int action, DWORD shortcut, int pos);

	static int GetActionShortCutB(DWORD action, int pos);
	static void SetActionShortCutB(int action, DWORD shortcut, int pos);

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
};

// global for easy access and for initialization of fast access variables
extern CGetSetOptions g_Opt; 