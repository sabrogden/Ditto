/////////////////////////////////////////////////////////////////////////////
// SystemTray.cpp : implementation file
//
// MFC VERSION
//
// This is a conglomeration of ideas from the MSJ "Webster" application,
// sniffing round the online docs, and from other implementations such
// as PJ Naughter's "CTrayNotifyIcon" (http://indigo.ie/~pjn/ntray.html)
// especially the "CSystemTray::OnTrayNotification" member function.
// Joerg Koenig suggested the icon animation stuff
//
// This class is a light wrapper around the windows system tray stuff. It
// adds an icon to the system tray with the specified ToolTip text and 
// callback notification value, which is sent back to the Parent window.
//
// The tray icon can be instantiated using either the constructor or by
// declaring the object and creating (and displaying) it later on in the
// program. eg.
//
//        CSystemTray m_SystemTray;    // Member variable of some class
//        
//        ... 
//        // in some member function maybe...
//        m_SystemTray.Create(pParentWnd, WM_MY_NOTIFY, "Click here", 
//                          hIcon, nSystemTrayID);
//
// Written by Chris Maunder (cmaunder@mail.com)
// Copyright (c) 1998.
//
// Updated: 25 Jul 1998 - Added icon animation, and derived class
//                        from CWnd in order to handle messages. (CJM)
//                        (icon animation suggested by Joerg Koenig.
//                        Added API to set default menu item. Code provided
//                        by Enrico Lelina.
//
// Updated: 6 June 1999 - SetIcon can now load non-standard sized icons (Chip Calvert)
//                        Added "bHidden" parameter when creating icon
//                        (Thanks to Michael Gombar for these suggestions)
//                        Restricted tooltip text to 64 characters.
//
// Updated: 9 Nov 1999  - Now works in WindowsCE.
//                        Fix for use in NT services (Thomas Mooney, TeleProc, Inc)
//                        Added W2K stuff by Michael Dunn
//
// Updated: 1 Jan 2000  - Added tray minimisation stuff.
// 
// Updated: 21 Sep 2000 - Added GetDoWndAnimation - animation only occurs if the system
//                        settings allow it (Matthew Ellis). Updated the GetTrayWndRect
//                        function to include more fallback logic (Matthew Ellis)
//                        NOTE: Signature of GetTrayWndRect has changed!
//
// Updated: 16 Jun 2002 - Fixed stupid errors so that it compiles clean on VC7
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage caused through use.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 
//
/////////////////////////////////////////////////////////////////////////////
    
#include "stdafx.h"
#include "SystemTray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef _WIN32_WCE  // Use C++ exception handling instead of structured.
#undef TRY
#undef CATCH
#undef END_CATCH
#define TRY try
#define CATCH(ex_class, ex_object) catch(ex_class* ex_object)
#define END_CATCH
#endif  // _WIN32_WCE

#ifndef _countof
#define _countof(x) ( sizeof(x) / sizeof(x[0]) )
#endif

IMPLEMENT_DYNAMIC(CSystemTray, CWnd)

const UINT CSystemTray::m_nTimerID    = 4567;
UINT CSystemTray::m_nMaxTooltipLength  = 64;     // This may change...
const UINT CSystemTray::m_nTaskbarCreatedMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));
CWnd  CSystemTray::m_wndInvisible;
BOOL CSystemTray::m_bShowWndAnimation;

/////////////////////////////////////////////////////////////////////////////
// CSystemTray construction/creation/destruction

CSystemTray::CSystemTray()
{
    Initialise();
}

CSystemTray::CSystemTray(CWnd* pParent,             // The window that will recieve tray notifications
                         UINT uCallbackMessage,     // the callback message to send to parent
                         LPCTSTR szToolTip,         // tray icon tooltip
                         HICON icon,                // Handle to icon
                         UINT uID,                  // Identifier of tray icon
                         BOOL bHidden /*=FALSE*/,   // Hidden on creation?                  
                         LPCTSTR szBalloonTip /*=NULL*/,    // Ballon tip (w2k only)
                         LPCTSTR szBalloonTitle /*=NULL*/,  // Balloon tip title (w2k)
                         DWORD dwBalloonIcon /*=NIIF_NONE*/,// Ballon tip icon (w2k)
                         UINT uBalloonTimeout /*=10*/)      // Balloon timeout (w2k)
{
    Initialise();
    Create(pParent, uCallbackMessage, szToolTip, icon, uID, bHidden,
           szBalloonTip, szBalloonTitle, dwBalloonIcon, uBalloonTimeout);
}

void CSystemTray::Initialise()
{
    memset(&m_tnd, 0, sizeof(m_tnd));

    m_bEnabled = FALSE;
    m_bHidden  = TRUE;
    m_bRemoved = TRUE;

    m_DefaultMenuItemID    = 0;
    m_DefaultMenuItemByPos = TRUE;

    m_bShowIconPending = FALSE;

    m_uIDTimer   = 0;
    m_hSavedIcon = NULL;

	m_pTargetWnd = NULL;
	m_uCreationFlags = 0;

	m_bShowWndAnimation = FALSE;

	m_bSingleClickSelect = FALSE;

#ifdef SYSTEMTRAY_USEW2K
    OSVERSIONINFO os = { sizeof(os) };
    GetVersionEx(&os);
    m_bWin2K = ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5 );
#else
    m_bWin2K = FALSE;
#endif
}

// update by Michael Dunn, November 1999
//
//  New version of Create() that handles new features in Win 2K.
//
// Changes:
//  szTip: Same as old, but can be 128 characters instead of 64.
//  szBalloonTip: Text for a balloon tooltip that is shown when the icon
//                is first added to the tray.  Pass "" if you don't want
//                a balloon.
//  szBalloonTitle: Title text for the balloon tooltip.  This text is shown
//                  in bold above the szBalloonTip text.  Pass "" if you
//                  don't want a title.
//  dwBalloonIcon: Specifies which icon will appear in the balloon.  Legal
//                 values are:
//                     NIIF_NONE: No icon
//                     NIIF_INFO: Information
//                     NIIF_WARNING: Exclamation
//                     NIIF_ERROR: Critical error (red circle with X)
//  uBalloonTimeout: Number of seconds for the balloon to remain visible.
//                   Must be between 10 and 30 inclusive.

BOOL CSystemTray::Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip, 
                         HICON icon, UINT uID, BOOL bHidden /*=FALSE*/,
                         LPCTSTR szBalloonTip /*=NULL*/, 
                         LPCTSTR szBalloonTitle /*=NULL*/,  
                         DWORD dwBalloonIcon /*=NIIF_NONE*/,
                         UINT uBalloonTimeout /*=10*/)
{
#ifdef _WIN32_WCE
    m_bEnabled = TRUE;
#else
    // this is only for Windows 95 (or higher)
    m_bEnabled = (GetVersion() & 0xff) >= 4;
    if (!m_bEnabled) 
    {
        ASSERT(FALSE);
        return FALSE;
    }
#endif

    m_nMaxTooltipLength = _countof(m_tnd.szTip);
    
    // Make sure we avoid conflict with other messages
    ASSERT(uCallbackMessage >= WM_APP);

    // Tray only supports tooltip text up to m_nMaxTooltipLength) characters
    ASSERT(AfxIsValidString(szToolTip));
    ASSERT(_tcslen(szToolTip) <= m_nMaxTooltipLength);

    // Create an invisible window
    CWnd::CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP, 0,0,0,0, NULL, 0);

    // load up the NOTIFYICONDATA structure
    m_tnd.cbSize = sizeof(NOTIFYICONDATA);
    m_tnd.hWnd   = pParent->GetSafeHwnd()? pParent->GetSafeHwnd() : m_hWnd;
    m_tnd.uID    = uID;
    m_tnd.hIcon  = icon;
    m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_tnd.uCallbackMessage = uCallbackMessage;
    _tcsncpy(m_tnd.szTip, szToolTip, m_nMaxTooltipLength-1);

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K && szBalloonTip)
    {
        // The balloon tooltip text can be up to 255 chars long.
        ASSERT(AfxIsValidString(szBalloonTip));
        ASSERT(lstrlen(szBalloonTip) < 256);

        // The balloon title text can be up to 63 chars long.
        if (szBalloonTitle)
        {
            ASSERT(AfxIsValidString(szBalloonTitle));
            ASSERT(lstrlen(szBalloonTitle) < 64);
        }

        // dwBalloonIcon must be valid.
        ASSERT(NIIF_NONE == dwBalloonIcon    || NIIF_INFO == dwBalloonIcon ||
               NIIF_WARNING == dwBalloonIcon || NIIF_ERROR == dwBalloonIcon);

        // The timeout must be between 10 and 30 seconds.
        ASSERT(uBalloonTimeout >= 10 && uBalloonTimeout <= 30);

        m_tnd.uFlags |= NIF_INFO;

        _tcsncpy(m_tnd.szInfo, szBalloonTip, 255);
        if (szBalloonTitle)
            _tcsncpy(m_tnd.szInfoTitle, szBalloonTitle, 63);
        else
            m_tnd.szInfoTitle[0] = _T('\0');
        m_tnd.uTimeout    = uBalloonTimeout * 1000; // convert time to ms
        m_tnd.dwInfoFlags = dwBalloonIcon;
    }
#endif

    m_bHidden = bHidden;

#ifdef SYSTEMTRAY_USEW2K    
    if (m_bWin2K && m_bHidden)
    {
        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = NIS_HIDDEN;
        m_tnd.dwStateMask = NIS_HIDDEN;
    }
#endif

	m_uCreationFlags = m_tnd.uFlags;	// Store in case we need to recreate in OnTaskBarCreate

    BOOL bResult = TRUE;
    if (!m_bHidden || m_bWin2K)
    {
        bResult = Shell_NotifyIcon(NIM_ADD, &m_tnd);
        m_bShowIconPending = m_bHidden = m_bRemoved = !bResult;
    }
    
#ifdef SYSTEMTRAY_USEW2K    
    if (m_bWin2K && szBalloonTip)
    {
        // Zero out the balloon text string so that later operations won't redisplay
        // the balloon.
        m_tnd.szInfo[0] = _T('\0');
    }
#endif

    return bResult;
}

CSystemTray::~CSystemTray()
{
    RemoveIcon();
    m_IconList.RemoveAll();
    DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray icon manipulation

//////////////////////////////////////////////////////////////////////////
//
// Function:    SetFocus()
//
// Description:
//  Sets the focus to the tray icon.  Microsoft's Win 2K UI guidelines
//  say you should do this after the user dismisses the icon's context
//  menu.
//
// Input:
//  Nothing.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////
// Added by Michael Dunn, November, 1999
//////////////////////////////////////////////////////////////////////////

void CSystemTray::SetFocus()
{
#ifdef SYSTEMTRAY_USEW2K
    Shell_NotifyIcon ( NIM_SETFOCUS, &m_tnd );
#endif
}

BOOL CSystemTray::MoveToRight()
{
    RemoveIcon();
    return AddIcon();
}

BOOL CSystemTray::AddIcon()
{
    if (!m_bRemoved)
        RemoveIcon();

    if (m_bEnabled)
    {
        m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        if (!Shell_NotifyIcon(NIM_ADD, &m_tnd))
            m_bShowIconPending = TRUE;
        else
            m_bRemoved = m_bHidden = FALSE;
    }
    return (m_bRemoved == FALSE);
}

BOOL CSystemTray::RemoveIcon()
{
    m_bShowIconPending = FALSE;

    if (!m_bEnabled || m_bRemoved)
        return TRUE;

    m_tnd.uFlags = 0;
    if (Shell_NotifyIcon(NIM_DELETE, &m_tnd))
        m_bRemoved = m_bHidden = TRUE;

    return (m_bRemoved == TRUE);
}

BOOL CSystemTray::HideIcon()
{
    if (!m_bEnabled || m_bRemoved || m_bHidden)
        return TRUE;

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K)
    {
        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = NIS_HIDDEN;
        m_tnd.dwStateMask = NIS_HIDDEN;

        m_bHidden = Shell_NotifyIcon( NIM_MODIFY, &m_tnd);
    }
    else
#endif
        RemoveIcon();

    return (m_bHidden == TRUE);
}

BOOL CSystemTray::ShowIcon()
{
    if (m_bRemoved)
        return AddIcon();

    if (!m_bHidden)
        return TRUE;

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K)
    {
        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = 0;
        m_tnd.dwStateMask = NIS_HIDDEN;
        m_bHidden = !Shell_NotifyIcon ( NIM_MODIFY, &m_tnd );
    }
    else
#endif
        AddIcon();

    return (m_bHidden == FALSE);
}

BOOL CSystemTray::SetIcon(HICON hIcon)
{
    if (!m_bEnabled)
        return FALSE;

    m_tnd.uFlags = NIF_ICON;
    m_tnd.hIcon = hIcon;

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetIcon(LPCTSTR lpszIconName)
{
    HICON hIcon = (HICON) ::LoadImage(AfxGetResourceHandle(), 
                                      lpszIconName,
                                      IMAGE_ICON, 
                                      0, 0,
                                      LR_DEFAULTCOLOR | LR_SHARED);

    return SetIcon(hIcon);
}

BOOL CSystemTray::SetIcon(UINT nIDResource)
{
    return SetIcon(MAKEINTRESOURCE(nIDResource));
}

BOOL CSystemTray::SetStandardIcon(LPCTSTR lpIconName)
{
    HICON hIcon = LoadIcon(NULL, lpIconName);

    return SetIcon(hIcon);
}

BOOL CSystemTray::SetStandardIcon(UINT nIDResource)
{
	return SetStandardIcon(MAKEINTRESOURCE(nIDResource));
}
 
HICON CSystemTray::GetIcon() const
{
    return (m_bEnabled)? m_tnd.hIcon : NULL;
}

BOOL CSystemTray::SetIconList(UINT uFirstIconID, UINT uLastIconID) 
{
	if (uFirstIconID > uLastIconID)
        return FALSE;

	const CWinApp* pApp = AfxGetApp();
    if (!pApp)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_IconList.RemoveAll();
    TRY {
	    for (UINT i = uFirstIconID; i <= uLastIconID; i++)
		    m_IconList.Add(pApp->LoadIcon(i));
    }
    CATCH(CMemoryException, e)
    {
        e->ReportError();
        e->Delete();
        m_IconList.RemoveAll();
        return FALSE;
    }
    END_CATCH

    return TRUE;
}

BOOL CSystemTray::SetIconList(HICON* pHIconList, UINT nNumIcons)
{
    m_IconList.RemoveAll();

    TRY {
	    for (UINT i = 0; i <= nNumIcons; i++)
		    m_IconList.Add(pHIconList[i]);
    }
    CATCH (CMemoryException, e)
    {
        e->ReportError();
        e->Delete();
        m_IconList.RemoveAll();
        return FALSE;
    }
    END_CATCH

    return TRUE;
}

BOOL CSystemTray::Animate(UINT nDelayMilliSeconds, int nNumSeconds /*=-1*/)
{
    StopAnimation();

    m_nCurrentIcon = 0;
    m_StartTime = COleDateTime::GetCurrentTime();
    m_nAnimationPeriod = nNumSeconds;
    m_hSavedIcon = GetIcon();

	// Setup a timer for the animation
	m_uIDTimer = SetTimer(m_nTimerID, nDelayMilliSeconds, NULL);

    return (m_uIDTimer != 0);
}

BOOL CSystemTray::StepAnimation()
{
    if (!m_IconList.GetSize())
        return FALSE;

    m_nCurrentIcon++;
    if (m_nCurrentIcon >= m_IconList.GetSize())
        m_nCurrentIcon = 0;

    return SetIcon(m_IconList[m_nCurrentIcon]);
}

BOOL CSystemTray::StopAnimation()
{
    BOOL bResult = FALSE;

    if (m_uIDTimer)
	    bResult = KillTimer(m_uIDTimer);
    m_uIDTimer = 0;

    if (m_hSavedIcon)
        SetIcon(m_hSavedIcon);
    m_hSavedIcon = NULL;

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray tooltip text manipulation

BOOL CSystemTray::SetTooltipText(LPCTSTR pszTip)
{
    ASSERT(AfxIsValidString(pszTip)); // (md)
    ASSERT(_tcslen(pszTip) < m_nMaxTooltipLength);

    if (!m_bEnabled) 
        return FALSE;

    m_tnd.uFlags = NIF_TIP;
    _tcsncpy(m_tnd.szTip, pszTip, m_nMaxTooltipLength-1);

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetTooltipText(UINT nID)
{
    CString strText;
    VERIFY(strText.LoadString(nID));

    return SetTooltipText(strText);
}

CString CSystemTray::GetTooltipText() const
{
    CString strText;
    if (m_bEnabled)
        strText = m_tnd.szTip;

    return strText;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray support for Win 2K features.

//////////////////////////////////////////////////////////////////////////
//
// Function:    ShowBalloon
//
// Description:
//  Shows a balloon tooltip over the tray icon.
//
// Input:
//  szText: [in] Text for the balloon tooltip.
//  szTitle: [in] Title for the balloon.  This text is shown in bold above
//           the tooltip text (szText).  Pass "" if you don't want a title.
//  dwIcon: [in] Specifies an icon to appear in the balloon.  Legal values are:
//                 NIIF_NONE: No icon
//                 NIIF_INFO: Information
//                 NIIF_WARNING: Exclamation
//                 NIIF_ERROR: Critical error (red circle with X)
//  uTimeout: [in] Number of seconds for the balloon to remain visible.  Can
//            be between 10 and 30 inclusive.
//
// Returns:
//  TRUE if successful, FALSE if not.
//
//////////////////////////////////////////////////////////////////////////
// Added by Michael Dunn, November 1999
//////////////////////////////////////////////////////////////////////////

BOOL CSystemTray::ShowBalloon(LPCTSTR szText,
                              LPCTSTR szTitle  /*=NULL*/,
                              DWORD   dwIcon   /*=NIIF_NONE*/,
                              UINT    uTimeout /*=10*/ )
{
#ifndef SYSTEMTRAY_USEW2K
    return FALSE;
#else
    // Bail out if we're not on Win 2K.
    if (!m_bWin2K)
        return FALSE;

    // Verify input parameters.

    // The balloon tooltip text can be up to 255 chars long.
    ASSERT(AfxIsValidString(szText));
    ASSERT(lstrlen(szText) < 256);

    // The balloon title text can be up to 63 chars long.
    if (szTitle)
    {
        ASSERT(AfxIsValidString( szTitle));
        ASSERT(lstrlen(szTitle) < 64);
    }

    // dwBalloonIcon must be valid.
    ASSERT(NIIF_NONE == dwIcon    || NIIF_INFO == dwIcon ||
           NIIF_WARNING == dwIcon || NIIF_ERROR == dwIcon);

    // The timeout must be between 10 and 30 seconds.
    ASSERT(uTimeout >= 10 && uTimeout <= 30);


    m_tnd.uFlags = NIF_INFO;
    _tcsncpy(m_tnd.szInfo, szText, 256);
    if (szTitle)
        _tcsncpy(m_tnd.szInfoTitle, szTitle, 64);
    else
        m_tnd.szInfoTitle[0] = _T('\0');
    m_tnd.dwInfoFlags = dwIcon;
    m_tnd.uTimeout = uTimeout * 1000;   // convert time to ms

    BOOL bSuccess = Shell_NotifyIcon (NIM_MODIFY, &m_tnd);

    // Zero out the balloon text string so that later operations won't redisplay
    // the balloon.
    m_tnd.szInfo[0] = _T('\0');

    return bSuccess;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification window stuff

BOOL CSystemTray::SetNotificationWnd(CWnd* pWnd)
{
    if (!m_bEnabled) 
        return FALSE;

    // Make sure Notification window is valid
    if (!pWnd || !::IsWindow(pWnd->GetSafeHwnd()))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_tnd.hWnd = pWnd->GetSafeHwnd();
    m_tnd.uFlags = 0;

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

CWnd* CSystemTray::GetNotificationWnd() const
{
    return CWnd::FromHandle(m_tnd.hWnd);
}

// Hatr added

// Hatr added

// Change or retrive the window to send menu commands to
BOOL CSystemTray::SetTargetWnd(CWnd* pTargetWnd)
{
    m_pTargetWnd = pTargetWnd;
    return TRUE;
} // CSystemTray::SetTargetWnd()

CWnd* CSystemTray::GetTargetWnd() const
{
    if (m_pTargetWnd)
        return m_pTargetWnd;
    else
        return AfxGetMainWnd();
} // CSystemTray::GetTargetWnd()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification message stuff

BOOL CSystemTray::SetCallbackMessage(UINT uCallbackMessage)
{
    if (!m_bEnabled)
        return FALSE;

    // Make sure we avoid conflict with other messages
    ASSERT(uCallbackMessage >= WM_APP);

    m_tnd.uCallbackMessage = uCallbackMessage;
    m_tnd.uFlags = NIF_MESSAGE;

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

UINT CSystemTray::GetCallbackMessage() const
{
    return m_tnd.uCallbackMessage;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray menu manipulation

BOOL CSystemTray::SetMenuDefaultItem(UINT uItem, BOOL bByPos)
{
#ifdef _WIN32_WCE
    return FALSE;
#else
    if ((m_DefaultMenuItemID == uItem) && (m_DefaultMenuItemByPos == bByPos)) 
        return TRUE;

    m_DefaultMenuItemID = uItem;
    m_DefaultMenuItemByPos = bByPos;   

    CMenu menu, *pSubMenu;

    if (!menu.LoadMenu(m_tnd.uID))
        return FALSE;

    pSubMenu = menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    ::SetMenuDefaultItem(pSubMenu->m_hMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

    return TRUE;
#endif
}

void CSystemTray::GetMenuDefaultItem(UINT& uItem, BOOL& bByPos)
{
    uItem = m_DefaultMenuItemID;
    bByPos = m_DefaultMenuItemByPos;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray message handlers

BEGIN_MESSAGE_MAP(CSystemTray, CWnd)
	//{{AFX_MSG_MAP(CSystemTray)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
#ifndef _WIN32_WCE
	ON_WM_SETTINGCHANGE()
#endif
    ON_REGISTERED_MESSAGE(CSystemTray::m_nTaskbarCreatedMsg, OnTaskbarCreated)
END_MESSAGE_MAP()

void CSystemTray::OnTimer(UINT nIDEvent) 
{
    if (nIDEvent != m_uIDTimer)
    {
        ASSERT(FALSE);
        return;
    }

    COleDateTime CurrentTime = COleDateTime::GetCurrentTime();
    COleDateTimeSpan period = CurrentTime - m_StartTime;

    if (m_nAnimationPeriod > 0 && m_nAnimationPeriod < period.GetTotalSeconds())
    {
        StopAnimation();
        return;
    }

    StepAnimation();
}

// This is called whenever the taskbar is created (eg after explorer crashes
// and restarts. Please note that the WM_TASKBARCREATED message is only passed
// to TOP LEVEL windows (like WM_QUERYNEWPALETTE)
LRESULT CSystemTray::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/) 
{
    InstallIconPending();
	return 0L;
}

#ifndef _WIN32_WCE
void CSystemTray::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CWnd::OnSettingChange(uFlags, lpszSection);

    if (uFlags == SPI_SETWORKAREA)
        InstallIconPending();	
}
#endif

LRESULT CSystemTray::OnTrayNotification(UINT wParam, LONG lParam) 
{
    //Return quickly if its not for this tray icon
    if (wParam != m_tnd.uID)
        return 0L;

    CMenu menu, *pSubMenu;
    CWnd *pTargetWnd = GetTargetWnd();
    if (!pTargetWnd)
        return 0L;

    // Clicking with right button brings up a context menu
#if defined(_WIN32_WCE) //&& _WIN32_WCE < 211
    BOOL bAltPressed = ((GetKeyState(VK_MENU) & (1 << (sizeof(SHORT)*8-1))) != 0);
    if (LOWORD(lParam) == WM_LBUTTONUP && bAltPressed)
#else
    if (LOWORD(lParam) == WM_RBUTTONUP)
#endif
    {    
        if (!menu.LoadMenu(m_tnd.uID))
            return 0;
        
        pSubMenu = menu.GetSubMenu(0);
        if (!pSubMenu)
            return 0;

#ifndef _WIN32_WCE
        // Make chosen menu item the default (bold font)
        ::SetMenuDefaultItem(pSubMenu->m_hMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);
#endif

        // Display and track the popup menu
        CPoint pos;
#ifdef _WIN32_WCE
        pos = CPoint(GetMessagePos());
#else
        GetCursorPos(&pos);
#endif

        pTargetWnd->SetForegroundWindow(); 
        
#ifndef _WIN32_WCE

		pTargetWnd->SendMessage(WM_CUSTOMIZE_TRAY_MENU, (WPARAM)pSubMenu, 0);

        ::TrackPopupMenu(pSubMenu->m_hMenu, 0, pos.x, pos.y, 0, 
                         pTargetWnd->GetSafeHwnd(), NULL);
#else
        pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pTargetWnd, NULL);
#endif

        // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
        pTargetWnd->PostMessage(WM_NULL, 0, 0);

        menu.DestroyMenu();
    } 
#if defined(_WIN32_WCE) //&& _WIN32_WCE < 211
    if (LOWORD(lParam) == WM_LBUTTONDBLCLK && bAltPressed)
#else
    else if((LOWORD(lParam) == WM_LBUTTONDBLCLK) ||  (LOWORD(lParam) == WM_LBUTTONUP && m_bSingleClickSelect))
#endif
    {
        // double click received, the default action is to execute default menu item
        pTargetWnd->SetForegroundWindow();  

        UINT uItem;
        if (m_DefaultMenuItemByPos)
        {
            if (!menu.LoadMenu(m_tnd.uID))
                return 0;
            
            pSubMenu = menu.GetSubMenu(0);
            if (!pSubMenu)
                return 0;
            
            uItem = pSubMenu->GetMenuItemID(m_DefaultMenuItemID);

			menu.DestroyMenu();
        }
        else
            uItem = m_DefaultMenuItemID;
        
        pTargetWnd->SendMessage(WM_COMMAND, uItem, 0);
    }

    return 1;
}

LRESULT CSystemTray::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if (message == m_tnd.uCallbackMessage)
        return OnTrayNotification(wParam, lParam);
	
	return CWnd::WindowProc(message, wParam, lParam);
}

void CSystemTray::InstallIconPending()
{
    // Is the icon display pending, and it's not been set as "hidden"?
    if (!m_bShowIconPending || m_bHidden)
        return;

	// Reset the flags to what was used at creation
	m_tnd.uFlags = m_uCreationFlags;

    // Try and recreate the icon
    m_bHidden = !Shell_NotifyIcon(NIM_ADD, &m_tnd);

    // If it's STILL hidden, then have another go next time...
    m_bShowIconPending = !m_bHidden;

    ASSERT(m_bHidden == FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// For minimising/maximising from system tray

BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[256];
    GetClassName(hwnd, szClassName, 255);
	
    // Did we find the Main System Tray? If so, then get its size and keep going
    if (_tcscmp(szClassName, _T("TrayNotifyWnd")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        ::GetWindowRect(hwnd, pRect);
        return TRUE;
    }
	
    // Did we find the System Clock? If so, then adjust the size of the rectangle
    // we have and quit (clock will be found after the system tray)
    if (_tcscmp(szClassName, _T("TrayClockWClass")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        CRect rectClock;
        ::GetWindowRect(hwnd, rectClock);
        // if clock is above system tray adjust accordingly
        if (rectClock.bottom < pRect->bottom-5) // 10 = random fudge factor.
            pRect->top = rectClock.bottom;
        else
            pRect->right = rectClock.left;
        return FALSE;
    }
	
    return TRUE;
}
 
#ifndef _WIN32_WCE
// enhanced version by Matthew Ellis <m.t.ellis@bigfoot.com>
void CSystemTray::GetTrayWndRect(LPRECT lprect)
{
#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30
	
    HWND hShellTrayWnd = ::FindWindow(_T("Shell_TrayWnd"), NULL);
    if (hShellTrayWnd)
    {
        ::GetWindowRect(hShellTrayWnd, lprect);
        EnumChildWindows(hShellTrayWnd, FindTrayWnd, (LPARAM)lprect);
        return;
    }
    // OK, we failed to get the rect from the quick hack. Either explorer isn't
    // running or it's a new version of the shell with the window class names
    // changed (how dare Microsoft change these undocumented class names!) So, we
    // try to find out what side of the screen the taskbar is connected to. We
    // know that the system tray is either on the right or the bottom of the
    // taskbar, so we can make a good guess at where to minimize to
    APPBARDATA appBarData;
    appBarData.cbSize=sizeof(appBarData);
    if (SHAppBarMessage(ABM_GETTASKBARPOS,&appBarData))
    {
        // We know the edge the taskbar is connected to, so guess the rect of the
        // system tray. Use various fudge factor to make it look good
        switch(appBarData.uEdge)
        {
        case ABE_LEFT:
        case ABE_RIGHT:
            // We want to minimize to the bottom of the taskbar
            lprect->top    = appBarData.rc.bottom-100;
            lprect->bottom = appBarData.rc.bottom-16;
            lprect->left   = appBarData.rc.left;
            lprect->right  = appBarData.rc.right;
            break;
            
        case ABE_TOP:
        case ABE_BOTTOM:
            // We want to minimize to the right of the taskbar
            lprect->top    = appBarData.rc.top;
            lprect->bottom = appBarData.rc.bottom;
            lprect->left   = appBarData.rc.right-100;
            lprect->right  = appBarData.rc.right-16;
            break;
        }
        return;
    }
    
    // Blimey, we really aren't in luck. It's possible that a third party shell
    // is running instead of explorer. This shell might provide support for the
    // system tray, by providing a Shell_TrayWnd window (which receives the
    // messages for the icons) So, look for a Shell_TrayWnd window and work out
    // the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
    // and stretches either the width or the height of the screen. We can't rely
    // on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
    // rely on it being any size. The best we can do is just blindly use the
    // window rect, perhaps limiting the width and height to, say 150 square.
    // Note that if the 3rd party shell supports the same configuraion as
    // explorer (the icons hosted in NotifyTrayWnd, which is a child window of
    // Shell_TrayWnd), we would already have caught it above
    if (hShellTrayWnd)
    {
        ::GetWindowRect(hShellTrayWnd, lprect);
        if (lprect->right - lprect->left > DEFAULT_RECT_WIDTH)
            lprect->left = lprect->right - DEFAULT_RECT_WIDTH;
        if (lprect->bottom - lprect->top > DEFAULT_RECT_HEIGHT)
            lprect->top = lprect->bottom - DEFAULT_RECT_HEIGHT;
        
        return;
    }
    
    // OK. Haven't found a thing. Provide a default rect based on the current work
    // area
    SystemParametersInfo(SPI_GETWORKAREA,0, lprect, 0);
    lprect->left = lprect->right - DEFAULT_RECT_WIDTH;
    lprect->top  = lprect->bottom - DEFAULT_RECT_HEIGHT;
}

// Check to see if the animation has been disabled (Matthew Ellis <m.t.ellis@bigfoot.com>)
BOOL CSystemTray::GetDoWndAnimation()
{
	if(!m_bShowWndAnimation)
		return FALSE;
	
	ANIMATIONINFO ai;
	
	ai.cbSize=sizeof(ai);
	SystemParametersInfo(SPI_GETANIMATION,sizeof(ai),&ai,0);
	
	return ai.iMinAnimate?TRUE:FALSE;
}
#endif

BOOL CSystemTray::RemoveTaskbarIcon(CWnd* pWnd)
{
    LPCTSTR pstrOwnerClass = AfxRegisterWndClass(0);
	
    // Create static invisible window
    if (!::IsWindow(m_wndInvisible.m_hWnd))
    {
		if (!m_wndInvisible.CreateEx(0, pstrOwnerClass, _T(""), WS_POPUP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, 0))
			return FALSE;
    }
	
    pWnd->SetParent(&m_wndInvisible);
	
    return TRUE;
}

void CSystemTray::MinimiseToTray(CWnd* pWnd)
{
#ifndef _WIN32_WCE
    if (GetDoWndAnimation())
    {
        CRect rectFrom, rectTo;
		
        pWnd->GetWindowRect(rectFrom);
        GetTrayWndRect(rectTo);
		
		::DrawAnimatedRects(pWnd->m_hWnd, IDANI_CAPTION, rectFrom, rectTo);
    }
	
    RemoveTaskbarIcon(pWnd);
    pWnd->ModifyStyle(WS_VISIBLE, 0);
#endif
}

void CSystemTray::MaximiseFromTray(CWnd* pWnd)
{
#ifndef _WIN32_WCE
    if (GetDoWndAnimation())
    {
        CRect rectTo;
        pWnd->GetWindowRect(rectTo);

        CRect rectFrom;
        GetTrayWndRect(rectFrom);

        pWnd->SetParent(NULL);
		::DrawAnimatedRects(pWnd->m_hWnd, IDANI_CAPTION, rectFrom, rectTo);
    }
    else
        pWnd->SetParent(NULL);

    pWnd->ModifyStyle(0, WS_VISIBLE);
    pWnd->RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME |
                                   RDW_INVALIDATE | RDW_ERASE);

    // Move focus away and back again to ensure taskbar icon is recreated
    if (::IsWindow(m_wndInvisible.m_hWnd))
        m_wndInvisible.SetActiveWindow();
    pWnd->SetActiveWindow();
    pWnd->SetForegroundWindow();
#endif
}
