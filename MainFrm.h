#pragma once

#include "SystemTray.h"
#include "QuickPaste.h"
#include "ToolTipEx.h"
#include "EditFrameWnd.h"
#include "MainFrmThread.h"
#include "ClipboardSaveRestore.h"


#define CLOSE_WINDOW_TIMER				1	
#define HIDE_ICON_TIMER					2
#define REMOVE_OLD_ENTRIES_TIMER		3
#define REMOVE_OLD_REMOTE_COPIES		6
#define END_DITTO_BUFFER_CLIPBOARD_TIMER	7
#define KEY_STATE_MODIFIERS				8
#define ACTIVE_WINDOW_TIMER				9
#define FOCUS_CHANGED_TIMER				10
#define TEXT_ONLY_PASTE					11

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
    CSystemTray m_TrayIcon;
    ULONG m_ulCopyGap;
    CString m_csKeyboardPaste;
    CAlphaBlend m_Transparency;
    BYTE m_keyStateModifiers;
    DWORD m_startKeyStateTime;
    bool m_bMovedSelectionMoveKeyState;
    short m_keyModifiersTimerCount;
    HWND m_tempFocusWnd;
    CMainFrmThread m_thread;
	CClipboardSaveRestore m_textOnlyPaste;

    void DoDittoCopyBufferPaste(int nCopyBuffer);
    void DoFirstTenPositionsPaste(int nPos);
    bool PasteQuickPasteEntry(CString csQuickPaste);
    bool SaveQuickPasteEntry(CString csQuickPaste, CClipList *pClipList);
    void ShowErrorMessage(CString csTitle, CString csMessage);
    bool CloseAllOpenDialogs();
	void DoTextOnlyPaste();

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
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnFirstShowquickpaste();
    afx_msg void OnFirstToggleConnectCV();
    afx_msg void OnUpdateFirstToggleConnectCV(CCmdUI *pCmdUI);
    afx_msg void OnFirstHelp();
    //}}AFX_MSG
    afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnShowTrayIcon(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnClipboardCopied(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAddToDatabaseFromSocket(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnErrorOnSendRecieve(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFocusChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCustomizeTrayMenu(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnEditWndClose(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetConnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadClipOnClipboard(WPARAM wParam, LPARAM lParam);
DECLARE_MESSAGE_MAP()public:
    virtual BOOL PreTranslateMessage(MSG *pMsg);
    afx_msg void OnClose();
    afx_msg void OnFirstImport();
    afx_msg void OnDestroy();
    afx_msg void OnFirstNewclip();
};

class CShowMainFrame
{
public:
    CShowMainFrame();
    ~CShowMainFrame();
    static bool m_bShowingMainFrame;
    bool m_bHideMainFrameOnExit;
    HWND m_hWnd;
};
