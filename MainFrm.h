#pragma once

//#include "SystemTray.h"
#include "QuickPaste.h"
#include "ToolTipEx.h"
#include "EditFrameWnd.h"
#include "MainFrmThread.h"
#include "ClipboardSaveRestore.h"
#include "PowerManager.h"
#include "DittoPopupWindow.h"
#include "NTray.h"

#define CLOSE_WINDOW_TIMER				1	
#define HIDE_ICON_TIMER					2
#define REMOVE_OLD_ENTRIES_TIMER		3
#define REMOVE_OLD_TEMP_FILES			6
#define END_DITTO_BUFFER_CLIPBOARD_TIMER	7
#define KEY_STATE_MODIFIERS				8
#define ACTIVE_WINDOW_TIMER				9
#define TEXT_ONLY_PASTE					11
#define READ_RANDOM_DB_FILE				12
#define GROUP_DOUBLE_CLICK				13
#define CLOSE_POPUP_MSG_WND				14
#define SCREEN_RESOLUTION_CHANGED		15
#define DELAYED_SHOW_DITTO_TIMER		16
#define SET_WINDOWS_THEME_TIMER			17
#define CLOSE_NO_DB_WINDOW_TIMER        18

class CMainFrame: public CFrameWnd
{
public:
    CMainFrame();
protected:
    DECLARE_DYNAMIC(CMainFrame)

    // Attributes
public:

    // Operations
public:

    BOOL ResetKillDBTimer();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMainFrame)
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
    //	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
    //}}AFX_VIRTUAL

    // Implementation
public:
    virtual ~CMainFrame();
    #ifdef _DEBUG
        virtual void AssertValid()const;
        virtual void Dump(CDumpContext &dc)const;
    #endif 

    CQuickPaste m_quickPaste;
    //CSystemTray m_TrayIcon;
	CTrayNotifyIcon m_trayIcon;
    ULONG m_ulCopyGap;
    CString m_csKeyboardPaste;
    CAlphaBlend m_Transparency;
    BYTE m_keyStateModifiers;
    DWORD m_startKeyStateTime;
    bool m_bMovedSelectionMoveKeyState;
    short m_keyModifiersTimerCount;
    HWND m_tempFocusWnd;
    CMainFrmThread m_thread;
	CDialog *m_pGlobalClips;
	CDialog *m_pDeleteClips;
	CPropertySheet *m_pOptions;
	int m_doubleClickGroupId;
	DWORD m_doubleClickGroupStartTime;
	CPowerManager m_PowerManager;
	int m_startupScreenWidth;
	int m_startupScreenHeight;

    void DoDittoCopyBufferPaste(int nCopyBuffer);
    void DoFirstTenPositionsPaste(int nPos);
	void PasteOrShowGroup(int dbId, BOOL updateClipTime, BOOL activeTarget, BOOL sendPaste, bool pastedFromGroup);

	void StartKeyModifierTimer();

	bool PasteQuickPasteEntry(CString csQuickPaste);
    bool SaveQuickPasteEntry(CString csQuickPaste, CClipList *pClipList);
    void ShowErrorMessage(CString csTitle, CString csMessage);
    bool CloseAllOpenDialogs();
	void DoTextOnlyPaste();
	void RefreshShowInTaskBar();

    void ShowEditWnd(CClipIDs &Ids);
    CEditFrameWnd *m_pEditFrameWnd;


    // Generated message map functions
protected:
    //{{AFX_MSG(CMainFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnFirstOption();
    afx_msg void OnFirstExit();
    afx_msg void OnChangeCbChain(HWND hWndRemove, HWND hWndAfter);
    afx_msg void OnDrawClipboard();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnFirstShowquickpaste();
    afx_msg void OnFirstToggleConnectCV();
    afx_msg void OnUpdateFirstToggleConnectCV(CCmdUI *pCmdUI);
    afx_msg void OnFirstHelp();
    //}}AFX_MSG
    afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	void ShowQPasteWithActiveWindowCheck();
    afx_msg LRESULT OnShowTrayIcon(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnClipboardCopied(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAddToDatabaseFromSocket(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnErrorOnSendRecieve(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnErrorMsg(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnEditWndClose(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetConnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenCloseWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadClipOnClipboard(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGlobalClipsClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeleteClipDataClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOptionsClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowOptions(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSaveClipboardMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReAddTaskBarIcon(WPARAM wParam, LPARAM lParam);
DECLARE_MESSAGE_MAP()public:
    virtual BOOL PreTranslateMessage(MSG *pMsg);
    afx_msg void OnClose();
    afx_msg void OnFirstImport();
    afx_msg void OnDestroy();
    afx_msg void OnFirstNewclip();
	afx_msg void OnFirstGlobalhotkeys();
	afx_msg void OnFirstDeleteclipdata();
	afx_msg void OnFirstSavecurrentclipboard();
	afx_msg LRESULT OnReOpenDatabase(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowMsgWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowDittoGroup(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFirstFixupstickycliporder();
	afx_msg LRESULT OnResolutionChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlainTextPaste(WPARAM wParam, LPARAM lParam);
	afx_msg void OnWinIniChange(LPCTSTR lpszSection);
	afx_msg void OnFirstShowstartupmessage();
	afx_msg void OnUpdateFirstShowstartupmessage(CCmdUI *pCmdUI);
	afx_msg void OnFirstBackupdatabase();
	afx_msg void OnFirstRestoredatabase();
	afx_msg LRESULT OnRestoreDb(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBackupDb(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFirstDeleteallnonusedclips();
    afx_msg LRESULT OnPasteClip(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnEditClip(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
};
