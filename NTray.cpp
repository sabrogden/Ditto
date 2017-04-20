/*
Module : NTray.cpp
Purpose: implementation for a MFC class to encapsulate Shell_NotifyIcon
Created: PJN / 14-05-1997
History: PJN / 25-11-1997 Addition of the following
                          1. HideIcon(), ShowIcon() & MoveToExtremeRight() 
                          2. Support for animated tray icons
         PJN / 23-06-1998 Class now supports the new Taskbar Creation Notification 
                          message which comes with IE 4. This allows the tray icon
                          to be recreated whenever the explorer restarts (Crashes!!)
         PJN / 22-07-1998 1. Code now compiles cleanly at warning level 4
                          2. Code is now UNICODE enabled + build configurations are provided
                          3. The documentation for the class has been updated
         PJN / 27-01-1999 1. Code first tries to load a 16*16 icon before loading the 32*32
                          version. This removes the blurryness which was previously occuring
         PJN / 28-01-1999 1. Fixed a number of level 4 warnings which were occurring.
         PJN / 09-05-1999 1. Fixed a problem as documented in KB article "PRB: Menus for 
                          Notification Icons Do Not Work Correctly", Article ID: Q135788 
         PJN / 15-05-1999 1. Now uses the author's hookwnd class. This prevents the need to 
                          create the two hidden windows namely CTrayRessurectionWnd and
                          CTrayTimerWnd
                          2. Code now compiles cleanly at warning level 4
                          3. General code tidy up and rearrangement
                          4. Added numerous ASSERT's to improve the code's robustness
                          5. Added functions to allow context menu to be customized
         PJN / 01-01-2001 1. Now includes copyright message in the source code and documentation. 
                          2. Fixed problem where the window does not get focus after double clicking 
                          on the tray icon
                          3. Now fully supports the Windows 2000 balloon style tooltips
                          4. Fixed a off by one problem in some of the ASSERT's
                          5. Fixed problems with Unicode build configurations
                          6. Provided Win2k specific build configurations
         PJN / 10-02-2001 1. Now fully supports creation of 2 tray icons at the same time
         PJN / 13-06-2001 1. Now removes windows hook upon call to RemoveIcon
         PJN / 26-08-2001 1. Fixed memory leak in RemoveIcon.
                          2. Fixed GPF in RemoveIcon by removing call to Unhook
         PJN / 28-08-2001 1. Added support for direct access to the System Tray's HDC. This allows
                          you to generate an icon for the tray on the fly using GDI calls. The idea
                          came from the article by Jeff Heaton in the April issue of WDJ. Also added
                          are overriden Create methods to allow you to easily costruct a dynamic
                          tray icon given a BITMAP instead of an ICON.
         PJN / 21-03-2003 1. Fixed icon resource leaks in SetIcon(LPCTSTR lpIconName) and 
                          SetIcon(UINT nIDResource). Thanks to Egor Pervouninski for reporting this.
                          2. Fixed unhooking of the tray icon when the notification window is being
                          closed.
         PJN / 31-03-2003 1. Now uses V1.05 of my Hookwnd class
         PJN / 02-04-2003 1. Now uses v1.06 of my Hookwnd class
                          2. Fixed a bug in the sample app for this class where the hooks should
                          have been created as global instances rather than as member variables of
                          the mainframe window. This ensures that the hooks remain valid even after
                          calling DefWindowProc on the mainframe.
         PJN / 23-07-2004 1. Minor update to remove unnecessary include of "resource.h"
         PJN / 03-03-2006 1. Updated copyright details.
                          2. Updated the documentation to use the same style as the web site.
                          3. Did a spell check of the documentation.        
                          4. Fixed some issues when the code is compiled using /Wp64. Please note that
                          to support this the code now requires a recentish Platform SDK to be installed
                          if the code is compiled with Visual C++ 6.
                          5. Replaced all calls to ZeroMemory with memset.
                          6. Fixed an issue where SetBalloonDetails was not setting the cbSize parameter.
                          Thanks to Enrique Granda for reporting this issue.
                          7. Added support for NIIF_USER and NIIF_NONE flags.
                          8. Now includes support for NIM_NIMSETVERSION via SetVersion. In addition this
                          is now automatically set in the Create() calls if the Win2k boolean parameter
                          is set.
                          9. Removed derivation from CObject as it was not really needed.
                          10. Now includes support for NIM_SETFOCUS
                          11. Added support for NIS_HIDDEN via the ShowIcon and HideIcon methods.
                          12. Added support for NIIF_NOSOUND
         PJN / 27-06-2006 1. Code now uses new C++ style casts rather than old style C casts where necessary.
                          2. The class framework now requires the Platform SDK if compiled using VC 6.  
                          3. Updated the logic of the ASSERTs which validate the various string lengths.
                          4. Fixed a bug in CTrayNotifyIcon::SetFocus() where the cbSize value was not being
                          set correctly.
                          5. CTrayIconHooker class now uses ATL's CWindowImpl class in preference to the author's
                          CHookWnd class. This does mean that for MFC only client projects, you will need to add
                          ATL support to your project.
                          6. Optimized CTrayIconHooker constructor code
                          7. Updated code to compile cleanly using VC 2005. Thanks to "Itamar" for prompting this
                          update.
                          8. Addition of a CTRAYNOTIFYICON_EXT_CLASS and CTRAYNOTIFYICON_EXT_API macros which makes 
                          the class easier to use in an extension dll.
                          9. Made CTrayNotifyIcon destructor virtual
         PJN / 03-07-2005 1. Fixed a bug where the HideIcon functionality did not work on Windows 2000. This was 
                          related to how the cbSize member of the NOTIFYICONDATA structure was initialized. The code
                          now dynamically determines the correct size to set at runtime according to the instructions
                          provided by the MSDN documentation for this structure. As a result of this, all "bWin2k" 
                          parameters which were previously exposed via CTrayNotifyIcon have now been removed as there
                          is no need for them. Thanks to Edwin Geng for reporting this important bug. Client code will
                          still need to intelligently make decisions on what is supported by the OS. For example balloon
                          tray icons are only supported on Shell v5 (nominally Windows 2000 or later). CTrayNotifyIcon
                          will ASSERT if for example calls are made to it to create a balloon tray icon on operating 
                          systems < Windows 2000.
         PJN / 04-07-2006 1. Fixed a bug where the menu may pop up a second time after a menu item is chosen on 
                          Windows 2000. The problem was tracked down to the code in CTrayNotifyIcon::OnTrayNotification. 
                          During testing of this bug, I was unable to get a workable solution using the new shell 
                          messages of WM_CONTEXTMENU, NIN_KEYSELECT & NIN_SELECT on Windows 2000 and Windows XP. 
                          This means that the code in CTrayNotifyIcon::OnTrayNotification uses the old way of handling 
                          notifications (WM_RBUTTDOWN*). This does mean that by default, client apps which use the 
                          CTrayNotifyIcon class will not support the new keyboard and mouse semantics for tray icons
                          (IMHO this is no big loss!). Client code is of course free to handle their own notifications. 
                          If you go down this route then I would advise you to thoroughly test your application on 
                          Windows 2000 and Windows XP as my testing has shown that there is significant differences in 
                          how tray icons handle their messaging on these 2 operating systems. Thanks to Edwin Geng for 
                          reporting this issue.
                          2. Class now displays the menu based on the current message's screen coordinates, rather than
                          the current cursor screen coordinates.
                          3. Fixed bug in sample app where if the about dialog is already up and it is reactivated 
                          from the tray menu, it did not bring itself into the foreground
         PJN / 06-07-2006 1. Reverted the change made for v1.53 where the screen coordinates used to show the context 
                          menu use the current message's screen coordinates. Instead the pre v1.53 mechanism which 
                          uses the current cursor's screen coordinates is now used. Thanks to Itamar Syn-Hershko for 
                          reporting this issue.
         PJN / 19-07-2006 1. The default menu item can now be customized via SetDefaultMenuItem and 
                          GetDefaultMenuItem. Thanks to Mikhail Bykanov for suggesting this nice update.
                          2. Optimized CTrayNotifyIcon constructor code
         PJN / 19-08-2005 1. Updated the code to operate independent of MFC if so desired. This requires WTL which is an
                          open source library extension for ATL to provide UI support along the lines of MFC. Thanks to 
                          zhiguo zhao for providing this very nice addition.
         PJN / 15-09-2006 1. Fixed a bug where WM_DESTROY messages were not been handled correctly for the top level window
                          which the CTrayIconHooker class subclasses in order to handle the tray resurrection message,
                          the animation timers and auto destroying of the icons when the top level window is destroyed. 
                          Thanks to Edward Livingston for reporting this bug.
                          2. Fixed a bug where the tray icons were not being recreated correctly when we receive the 
                          "TaskbarCreated" when Explorer is restarted. Thanks to Nuno Esculcas for reporting this bug.
                          3. Split the functionality of hiding versus deleting and showing versus creating of the tray
                          icon into 4 separate functions, namely Delete(), Create(), Hide() and Show(). Note that Hide 
                          and Show functionality is only available on Shell v5 or later.
                          4. Fixed an issue with recreation of tray icons which use a dynamic icon created from a bitmap
                          (through the use of BitmapToIcon).
                          5. CTrayNotifyIcon::LoadIconResource now loads up an icon as a shared icon resource using 
                          LoadImage. This should avoid resource leaks using this function.
         PJN / 15-06-2007 1. Updated copyright messages.
                          2. If the code detects that MFC is not included in the project, the code uses the standard
                          preprocessor define "_CSTRING_NS" to declare the string class to use rather than explicitly 
                          using WTL::CString. Thanks to Krzysztof Suszka for reporting this issue.
                          3. Updated sample app to compile cleanly on VC 2005.
                          4. Addition of a "BOOL bShow" to all the Create methods. This allows you to create an icon 
                          without actually showing it. This avoids the flicker which previously occurred if you created 
                          the icon and then immediately hid the icon. Thanks to Krzysztof Suszka for providing this 
                          suggestion.
                          5. Demo app now initially creates the first icon as hidden for demonstration purposes.
                          6. Added support for NIIF_LARGE_ICON. This Vista only feature allows you to create a large 
                          balloon icon.
                          7. Added support for NIF_REALTIME. This Vista only flag allows you to specify not to bother 
                          showing the balloon if it is delayed due to the presence of an existing balloon.
                          8. Added support for NOTIFYICONDATA::hBalloonIcon. This Vista only feature allows you to 
                          create a user specified balloon icon which is different to the actual tray icon.
                          9. LoadIconResource method now includes support for loading large icons and has been renamed
                          to simply LoadIcon. Also two overridden versions of this method have been provided which allow
                          the hInstance resource ID to be specified to load the icon from.
                          10. Reworked the internal code to CTrayNotifyIcon which detects the shell version.
                          11. Updated the tray icon text in the demo app to better demonstrate the features of the class.
                          12. Updated the WTL sample to be consistent with the MFC sample code
                          13. Updated comments in documentation about usage of the Platform SDK.
         PJN / 13-10-2007 1. Subclassing of the top level window is now not down internally by the CTrayNotifyIcon class
                          using the CTrayIconHooker class. Instead now a hidden top level window is created for each tray 
                          icon you create and these look after handling the tray resurrection and animated icon timer
                          messages. This refactoring of the internals of the class now also fixes a bug where an application
                          which creates multiples tray icons would only get one icon recreated when the tray resurrection
                          message was received. Thanks to Steven Dwyer for prompting this update. 
                          2. Updated the MFC sample app to correctly initialize ATL for VC 6
         PJN / 12-03-2008 1. Updated copyright details
                          2. Fixed a bug in SetBalloonDetails where the code did not set the flag NIF_ICON if a user defined
                          icon was being set. Thanks to "arni" for reporting this bug. 
                          3. Updated the sample app to clean compile on VC 2008
         PJN / 22-06-2008 1. Code now compiles cleanly using Code Analysis (/analyze)
                          2. Updated code to compile correctly using _ATL_CSTRING_EXPLICIT_CONSTRUCTORS define
                          3. Removed VC 6 style AppWizard comments from the code.
                          4. The code now only supports VC 2005 or later. 
         PJN / 10-04-2010 1. Updated copyright details.
                          2. Updated the project settings to more modern default values.
                          3. Updated the WTL version of LoadIcon to use the more modern ModuleHelper class from WTL to get 
                          the resource instance. Thanks to "Yarp" for reporting this issue.
                          4. The class now has support for the Windows 7 "NIIF_RESPECT_QUIET_TIME" flag. This value can be
                          set via the new "bQuietTime" parameter to the Create method.
                          5. Updated the code which does version detection of the Shell version
         PJN / 10-07-2010 1. Updated the sample app to compile cleanly on VS 2010.
                          2. Fixed a bug in CTrayNotifyIcon::Delete where the code would ASSERT if the tray notify icon was
                          never actually created. Thanks to "trophim" for reporting this bug.
         PJN / 06-11-2010 1. Minor update to code in SetTooltipText to code which handles unreferenced variable compiler 
                          warning
                          2. Implemented a GetTooltipMaxSize method which reports the maximum size which the tooltip can be
                          for a tray icon. Thanks to Geert van Horrik for this nice addition
                          3. All places which copy text to the underlying NOTIFYICONDATA now use the _TRUNCATE parameter in 
                          their call to the Safe CRT runtime. This change in behaviour means that client apps will no longer
                          crash if they supply data larger than this Windows structure can accommadate. Thanks to Geert van 
                          Horrik for prompting this update.
                          4. All calls to sizeof(struct)/sizeof(first element) have been replaced with _countof
                          5. Fixed a linker error when compiling the WTL sample app in release mode.
         PJN / 26-11-2010 1. Minor update to use DBG_UNREFERENCED_LOCAL_VARIABLE macro. Thanks to Jukka Ojanen for prompting this 
                          update.
         PJN / 27-04-2016 1. Updated copyright details.
                          2. Updated the code to clean compile on VC 2012 - VC 2015.
                          3. Removed support for CTRAYNOTIFYICON_NOWIN2K preprocessor macro
                          4. Removed various redefines of ShellApi.h constants from the code
                          5. Added SAL annotations to all the code.
                          6. Reworked the definition of the string class to now use a typedef internal to the CTrayNotifyIcon 
                          class.
                          7. Updated CTrayNotifyIcon::OnTrayNotification to handle NOTIFYICON_VERSION_4 style notifications.
                          8. Reworked the internal storage of the animation icons to use ATL::CHeapPtr

Copyright (c) 1997 - 2016 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////////  Includes  //////////////////////////////////

#include "stdafx.h"
#include "NTray.h"
#ifndef _INC_SHELLAPI
#pragma message("To avoid this message, please put ShellApi.h in your pre compiled header (normally stdafx.h)")
#include <ShellApi.h>
#endif //#ifndef _INC_SHELLAPI


/////////////////////////////////  Macros /////////////////////////////////////

#ifdef _AFX
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG
#endif //#ifdef _AFX

#ifndef NIF_REALTIME
#define NIF_REALTIME 0x00000040
#endif //#ifndef NIF_REALTIME

#ifndef NIIF_LARGE_ICON
#define NIIF_LARGE_ICON 0x00000020
#endif //#ifndef NIIF_LARGE_ICON

#ifndef NIIF_RESPECT_QUIET_TIME
#define NIIF_RESPECT_QUIET_TIME 0x00000080
#endif //#ifndef NIIF_RESPECT_QUIET_TIME


///////////////////////////////// Implementation //////////////////////////////

const UINT wm_TaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

CTrayNotifyIcon::CTrayNotifyIcon() : m_bCreated(FALSE),
                                     m_bHidden(FALSE),
                                     m_pNotificationWnd(NULL),
                                     m_bDefaultMenuItemByPos(TRUE),
                                     m_nDefaultMenuItem(0),
                                     m_hDynamicIcon(NULL),
                                     m_ShellVersion(Version4), //Assume version 4 of the shell
                                     m_nNumIcons(0),
                                     m_nTimerID(0),
                                     m_nCurrentIconIndex(0),
                                     m_nTooltipMaxSize(-1)
{
  typedef HRESULT (CALLBACK DLLGETVERSION)(DLLVERSIONINFO*);
  typedef DLLGETVERSION* LPDLLGETVERSION;

  //Try to get the details with DllGetVersion
  HMODULE hShell32 = GetModuleHandle(_T("SHELL32.DLL"));
  if (hShell32 != NULL)
  {
    LPDLLGETVERSION lpfnDllGetVersion = reinterpret_cast<LPDLLGETVERSION>(GetProcAddress(hShell32, "DllGetVersion"));
    if (lpfnDllGetVersion != NULL)
    {
      DLLVERSIONINFO vinfo;
      vinfo.cbSize = sizeof(DLLVERSIONINFO);
      if (SUCCEEDED(lpfnDllGetVersion(&vinfo)))
      {
        if ((vinfo.dwMajorVersion > 6) || (vinfo.dwMajorVersion == 6 && vinfo.dwMinorVersion > 0))
          m_ShellVersion = Version7;
        else if (vinfo.dwMajorVersion == 6)
        {
          if (vinfo.dwBuildNumber >= 6000)
            m_ShellVersion = VersionVista;
          else
            m_ShellVersion = Version6;
        }
        else if (vinfo.dwMajorVersion >= 5)
          m_ShellVersion = Version5;
      }
    }
  }

  memset(&m_NotifyIconData, 0, sizeof(m_NotifyIconData));
  m_NotifyIconData.cbSize = GetNOTIFYICONDATASizeForOS();
}

CTrayNotifyIcon::~CTrayNotifyIcon()
{
  //Delete the tray icon
  Delete(TRUE);
  
  //Free up any dynamic icon we may have
  if (m_hDynamicIcon != NULL)
  {
    DestroyIcon(m_hDynamicIcon);
    m_hDynamicIcon = NULL;
  }
}

BOOL CTrayNotifyIcon::Delete(_In_ BOOL bCloseHelperWindow)
{
  //What will be the return value from this function (assume the best)
  BOOL bSuccess = TRUE;

  if (m_bCreated)
  {
    m_NotifyIconData.uFlags = 0;
    bSuccess = Shell_NotifyIcon(NIM_DELETE, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
    m_bCreated = FALSE;
  }
  
  //Close the helper window if requested to do so
  if (bCloseHelperWindow && IsWindow())
    SendMessage(WM_CLOSE);
  
  return bSuccess;
}

BOOL CTrayNotifyIcon::Create(_In_ BOOL bShow)
{
  m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  
  if (!bShow)
  {
    ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later
    m_NotifyIconData.uFlags |= NIF_STATE;
    m_NotifyIconData.dwState = NIS_HIDDEN;
    m_NotifyIconData.dwStateMask = NIS_HIDDEN;
  }
  
  BOOL bSuccess = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (bSuccess)
  {
    m_bCreated = TRUE;
    
    if (!bShow)
      m_bHidden = TRUE;
  }
  return bSuccess;
}

BOOL CTrayNotifyIcon::Hide()
{
  //Validate our parameters
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later
//  ATLASSERT(!m_bHidden); //Only makes sense to hide the icon if it is not already hidden

  m_NotifyIconData.uFlags = NIF_STATE;
  m_NotifyIconData.dwState = NIS_HIDDEN;
  m_NotifyIconData.dwStateMask = NIS_HIDDEN; 
  BOOL bSuccess = Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (bSuccess)
    m_bHidden = TRUE;
  return bSuccess;
}

BOOL CTrayNotifyIcon::Show()
{
  //Validate our parameters
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later
//  ATLASSERT(m_bHidden); //Only makes sense to show the icon if it has been previously hidden
  ATLASSERT(m_bCreated);

  m_NotifyIconData.uFlags = NIF_STATE;
  m_NotifyIconData.dwState = 0;
  m_NotifyIconData.dwStateMask = NIS_HIDDEN;
  BOOL bSuccess = Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (bSuccess)
    m_bHidden = FALSE;
  return bSuccess;
}

void CTrayNotifyIcon::SetMenu(_In_ HMENU hMenu, UINT menuId)
{
  m_Menu.DestroyMenu();

  if (menuId != 0)
  {
	  if (!m_Menu.LoadMenu(menuId))
	  {
		  ATLASSERT(FALSE);
		  return;
	  }
  }
  else
  {
	  m_Menu.Attach(hMenu);
  }

#ifdef _AFX
  CMenu* pSubMenu = m_Menu.GetSubMenu(0);
  ATLASSUME(pSubMenu != NULL); //Your menu resource has been designed incorrectly
    
  //Make the specified menu item the default (bold font)
  pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#else
  CMenuHandle subMenu = m_Menu.GetSubMenu(0);
  ATLASSERT(subMenu.IsMenu()); //Your menu resource has been designed incorrectly

  //Make the specified menu item the default (bold font)
  subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#endif //#ifdef _AFX
}

CMenu& CTrayNotifyIcon::GetMenu()
{
  return m_Menu;
}

void CTrayNotifyIcon::SetDefaultMenuItem(_In_ UINT uItem, _In_ BOOL fByPos)
{ 
  m_nDefaultMenuItem = uItem; 
  m_bDefaultMenuItemByPos = fByPos; 

  //Also update in the live menu if it is present
  if (m_Menu.operator HMENU())
  {
  #ifdef _AFX
    CMenu* pSubMenu = m_Menu.GetSubMenu(0);
    ATLASSUME(pSubMenu != NULL); //Your menu resource has been designed incorrectly

    pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
  #else
    CMenuHandle subMenu = m_Menu.GetSubMenu(0);
    ATLASSERT(subMenu.IsMenu()); //Your menu resource has been designed incorrectly
    
    subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
  #endif //#ifdef _AFX
  }
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bShow)
#else
BOOL CTrayNotifyIcon::Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bShow)
#endif //#ifdef _AFX
{
  //Validate our parameters
  ATLASSUME((pNotifyWnd != NULL) && ::IsWindow(pNotifyWnd->operator HWND()));
#ifdef _DEBUG
  if (m_ShellVersion >= Version5) //If on Shell v5 or higher, then use the larger size tooltip
  {
    NOTIFYICONDATA_2 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < _countof(dummy.szTip));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }
  else
  {
    NOTIFYICONDATA_1 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < _countof(dummy.szTip));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }
#endif //#ifdef _DEBUG
  ATLASSERT(hIcon != NULL); 
  ATLASSERT(nNotifyMessage >= WM_USER); //Make sure we avoid conflict with other messages

  //Load up the menu resource which is to be used as the context menu
  if (!m_Menu.LoadMenu(uMenuID == 0 ? uID : uMenuID))
  {
    ATLASSERT(FALSE);
    return FALSE;
  }
#ifdef _AFX
  CMenu* pSubMenu = m_Menu.GetSubMenu(0);
  if (pSubMenu == NULL) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#else
  CMenuHandle subMenu = m_Menu.GetSubMenu(0);
  if (!subMenu.IsMenu())
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#endif //#ifdef _AFX

  //Create the helper window
  if (!CreateHelperWindow())
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  m_NotifyIconData.hWnd = pNotifyWnd->operator HWND();
  m_NotifyIconData.uID = uID;
  m_NotifyIconData.uCallbackMessage = nNotifyMessage;
  m_NotifyIconData.hIcon = hIcon;
  _tcsncpy_s(m_NotifyIconData.szTip, _countof(m_NotifyIconData.szTip), pszTooltipText, _TRUNCATE);

  if (!bShow)
  {
    ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later
    m_NotifyIconData.uFlags |= NIF_STATE;
    m_NotifyIconData.dwState = NIS_HIDDEN;
    m_NotifyIconData.dwStateMask = NIS_HIDDEN;
  }
  m_bCreated = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (m_bCreated)
  {
    if (!bShow)
      m_bHidden = TRUE;
    
    //Turn on Shell v5 style behaviour if supported
    if (m_ShellVersion >= Version5)
      SetVersion(NOTIFYICON_VERSION);
  }
  
  return m_bCreated;
}

BOOL CTrayNotifyIcon::SetVersion(_In_ UINT uVersion)
{
  //Validate our parameters
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uVersion = uVersion;
  return Shell_NotifyIcon(NIM_SETVERSION, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

HICON CTrayNotifyIcon::BitmapToIcon(_In_ CBitmap* pBitmap)
{
  //Validate our parameters
  ATLASSUME(pBitmap != NULL);

  //Get the width and height of a small icon
  int w = GetSystemMetrics(SM_CXSMICON);
  int h = GetSystemMetrics(SM_CYSMICON);

  //Create a 0 mask
  int nMaskSize = h*(w/8);
  ATL::CHeapPtr<BYTE> pMask;
  if (!pMask.Allocate(nMaskSize))
    return NULL;
  memset(pMask.m_pData, 0, nMaskSize);

  //Create a mask bitmap
  CBitmap maskBitmap;
#ifdef _AFX
  BOOL bSuccess = maskBitmap.CreateBitmap(w, h, 1, 1, pMask.m_pData);
#else
  maskBitmap.CreateBitmap(w, h, 1, 1, pMask.m_pData);
  BOOL bSuccess = !maskBitmap.IsNull();
#endif //#ifdef _AFX

  //Handle the error
  if (!bSuccess)
    return NULL;

  //Create an ICON base on the bitmap just created
  ICONINFO iconInfo;
  iconInfo.fIcon = TRUE;
  iconInfo.xHotspot = 0;
  iconInfo.yHotspot = 0;
  iconInfo.hbmMask = maskBitmap;
  iconInfo.hbmColor = *pBitmap; 
  return CreateIconIndirect(&iconInfo); 
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bShow)
#else
BOOL CTrayNotifyIcon::Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bShow)
#endif //#ifdef _AFX
{
  //Convert the bitmap to an Icon
  if (m_hDynamicIcon != NULL)
    DestroyIcon(m_hDynamicIcon);
  m_hDynamicIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  return Create(pNotifyWnd, uID, pszTooltipText, m_hDynamicIcon, nNotifyMessage, uMenuID, bShow);
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bShow)
#else
BOOL CTrayNotifyIcon::Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bShow)
#endif //#ifdef _AFX
{
  //Validate our parameters
  ATLASSUME(phIcons != NULL);
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(dwDelay);

  //let the normal Create function do its stuff
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, phIcons[0], nNotifyMessage, uMenuID, bShow);
  if (bSuccess)
  {
    //Start the animation
    bSuccess = StartAnimation(phIcons, nNumIcons, dwDelay);
  }

  return bSuccess;
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bNoSound, _In_ BOOL bLargeIcon, _In_ BOOL bRealtime, _In_opt_ HICON hBalloonIcon, _In_ BOOL bQuietTime, _In_ BOOL bShow)
#else
BOOL CTrayNotifyIcon::Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bNoSound, _In_ BOOL bLargeIcon, _In_ BOOL bRealtime, _In_opt_ HICON hBalloonIcon, _In_ BOOL bQuietTime, _In_ BOOL bShow)
#endif //#ifdef _AFX
{
  //Validate our parameters
  ATLASSUME((pNotifyWnd != NULL) && ::IsWindow(pNotifyWnd->operator HWND()));
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later
#ifdef _DEBUG
  NOTIFYICONDATA_2 dummy;
  ATLASSERT(_tcslen(pszTooltipText) < _countof(dummy.szTip));
  ATLASSERT(_tcslen(pszBalloonText) < _countof(dummy.szInfo));
  ATLASSERT(_tcslen(pszBalloonCaption) < _countof(dummy.szInfoTitle));
  ATLASSERT(hIcon); 
  ATLASSERT(nNotifyMessage >= WM_USER); //Make sure we avoid conflict with other messages
  DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
#endif //#ifdef _DEBUG

  //Load up the menu resource which is to be used as the context menu
  if (!m_Menu.LoadMenu(uMenuID == 0 ? uID : uMenuID))
  {
    ATLASSERT(FALSE);
    return FALSE;
  }
#ifdef _AFX
  CMenu* pSubMenu = m_Menu.GetSubMenu(0);
  if (pSubMenu == NULL) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  pSubMenu->SetDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#else
  CMenuHandle subMenu = m_Menu.GetSubMenu(0);
  if (!subMenu.IsMenu()) 
  {
    ATLASSERT(FALSE); //Your menu resource has been designed incorrectly
    return FALSE;
  }
  //Make the specified menu item the default (bold font)
  subMenu.SetMenuDefaultItem(m_nDefaultMenuItem, m_bDefaultMenuItemByPos);
#endif //#ifdef _AFX

  //Create the helper window
  if (!CreateHelperWindow())
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.hWnd = pNotifyWnd->operator HWND();
  m_NotifyIconData.uID = uID;
  m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
  m_NotifyIconData.uCallbackMessage = nNotifyMessage;
  m_NotifyIconData.hIcon = hIcon;
  _tcsncpy_s(m_NotifyIconData.szTip, _countof(m_NotifyIconData.szTip), pszTooltipText, _TRUNCATE);
  _tcsncpy_s(m_NotifyIconData.szInfo, _countof(m_NotifyIconData.szInfo), pszBalloonText, _TRUNCATE);
  _tcsncpy_s(m_NotifyIconData.szInfoTitle, _countof(m_NotifyIconData.szInfoTitle), pszBalloonCaption, _TRUNCATE);
  m_NotifyIconData.uTimeout = nTimeout;
  switch (style)
  {
    case Warning:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_WARNING;
      break;
    }
    case Error:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_ERROR;
      break;
    }
    case Info:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_INFO;
      break;
    }
    case None:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_NONE;
      break;
    }
    case User:
    {
      if (hBalloonIcon != NULL)
      {
        ATLASSERT(m_ShellVersion >= VersionVista);
        m_NotifyIconData.hBalloonIcon = hBalloonIcon;
      }
      else
      {
        ATLASSERT(hIcon != NULL); //You forget to provide a user icon
      }
      m_NotifyIconData.dwInfoFlags = NIIF_USER;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }
  if (bNoSound)
    m_NotifyIconData.dwInfoFlags |= NIIF_NOSOUND;
  if (bLargeIcon)
  {
    ATLASSERT(m_ShellVersion >= VersionVista); //Only supported on Vista Shell
    m_NotifyIconData.dwInfoFlags |= NIIF_LARGE_ICON;
  }
  if (bRealtime)
  {
    ATLASSERT(m_ShellVersion >= VersionVista); //Only supported on Vista Shell
    m_NotifyIconData.uFlags |= NIF_REALTIME;
  }
  if (!bShow)
  {
    ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later
    m_NotifyIconData.uFlags |= NIF_STATE;
    m_NotifyIconData.dwState = NIS_HIDDEN;
    m_NotifyIconData.dwStateMask = NIS_HIDDEN;
  }
  if (bQuietTime)
  {
    ATLASSERT(m_ShellVersion >= Version7); //Only supported on Windows 7 Shell
    m_NotifyIconData.dwInfoFlags |= NIIF_RESPECT_QUIET_TIME;
  }
  
  m_bCreated = Shell_NotifyIcon(NIM_ADD, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
  if (m_bCreated)
  {
    if (!bShow)
      m_bHidden = TRUE;
  
    //Turn on Shell v5 tray icon behaviour
    SetVersion(NOTIFYICON_VERSION);
  }

  return m_bCreated;
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bNoSound, _In_ BOOL bLargeIcon, _In_ BOOL bRealtime, _In_opt_ HICON hBalloonIcon, _In_ BOOL bQuietTime, _In_ BOOL bShow)
#else
BOOL CTrayNotifyIcon::Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bNoSound, _In_ BOOL bLargeIcon, _In_ BOOL bRealtime, _In_opt_ HICON hBalloonIcon, _In_ BOOL bQuietTime, _In_ BOOL bShow)
#endif //#ifdef _AFX
{
  //Convert the bitmap to an ICON
  if (m_hDynamicIcon != NULL)
    DestroyIcon(m_hDynamicIcon);
  m_hDynamicIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  return Create(pNotifyWnd, uID, pszTooltipText, pszBalloonText, pszBalloonCaption, nTimeout, style, m_hDynamicIcon, nNotifyMessage, uMenuID, bNoSound, bLargeIcon, bRealtime, hBalloonIcon, bQuietTime, bShow);
}

#ifdef _AFX
BOOL CTrayNotifyIcon::Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bNoSound, _In_ BOOL bLargeIcon, _In_ BOOL bRealtime, _In_opt_ HICON hBalloonIcon, _In_ BOOL bQuietTime, _In_ BOOL bShow)
#else
BOOL CTrayNotifyIcon::Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID, _In_ BOOL bNoSound, _In_ BOOL bLargeIcon, _In_ BOOL bRealtime, _In_opt_ HICON hBalloonIcon, _In_ BOOL bQuietTime, _In_ BOOL bShow)
#endif //#ifdef _AFX
{
  //Validate our parameters
  ATLASSUME(phIcons != NULL);
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSERT(dwDelay);

  //let the normal Create function do its stuff
  BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, pszBalloonText, pszBalloonCaption, nTimeout, style, phIcons[0], nNotifyMessage, uMenuID, bNoSound, bLargeIcon, bRealtime, hBalloonIcon, bQuietTime, bShow);
  if (bSuccess)
  {
    //Start the animation
    bSuccess = StartAnimation(phIcons, nNumIcons, dwDelay);
  }

  return bSuccess;
}

BOOL CTrayNotifyIcon::SetBalloonDetails(_In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ BalloonStyle style, _In_ UINT nTimeout, _In_ HICON hUserIcon, _In_ BOOL bNoSound, _In_ BOOL bLargeIcon, _In_ BOOL bRealtime, _In_ HICON hBalloonIcon)
{
  if (!m_bCreated)
    return FALSE;

  //Validate our parameters
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later
#ifdef _DEBUG
  NOTIFYICONDATA_2 dummy;
  ATLASSERT(_tcslen(pszBalloonText) < _countof(dummy.szInfo));
  ATLASSERT(_tcslen(pszBalloonCaption) < _countof(dummy.szInfoTitle));
  DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
#endif //#ifdef _DEBUG

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_INFO;
  _tcsncpy_s(m_NotifyIconData.szInfo, _countof(m_NotifyIconData.szInfo), pszBalloonText, _TRUNCATE);
  _tcsncpy_s(m_NotifyIconData.szInfoTitle, _countof(m_NotifyIconData.szInfoTitle), pszBalloonCaption, _TRUNCATE);
  m_NotifyIconData.uTimeout = nTimeout;
  switch (style)
  {
    case Warning:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_WARNING;
      break;
    }
    case Error:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_ERROR;
      break;
    }
    case Info:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_INFO;
      break;
    }
    case None:
    {
      m_NotifyIconData.dwInfoFlags = NIIF_NONE;
      break;
    }
    case User:
    {
      if (hBalloonIcon != NULL)
      {
        ATLASSERT(m_ShellVersion >= VersionVista);
        m_NotifyIconData.hBalloonIcon = hBalloonIcon;
      }
      else
      {
        ATLASSERT(hUserIcon != NULL); //You forget to provide a user icon
        m_NotifyIconData.uFlags |= NIF_ICON;
        m_NotifyIconData.hIcon = hUserIcon;
      }
        
      m_NotifyIconData.dwInfoFlags = NIIF_USER;
      break;
    }
    default:
    {
      ATLASSERT(FALSE);
      break;
    }
  }
  if (bNoSound)
    m_NotifyIconData.dwInfoFlags |= NIIF_NOSOUND;
  if (bLargeIcon)
    m_NotifyIconData.dwInfoFlags |= NIIF_LARGE_ICON;
  if (bRealtime)
    m_NotifyIconData.uFlags |= NIF_REALTIME;

  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

CTrayNotifyIcon::String CTrayNotifyIcon::GetBalloonText() const
{
  //Validate our parameters
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later

  String sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szInfo;

  return sText;
}

CTrayNotifyIcon::String CTrayNotifyIcon::GetBalloonCaption() const
{
  //Validate our parameters
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later

  String sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szInfoTitle;

  return sText;
}

UINT CTrayNotifyIcon::GetBalloonTimeout() const
{
  //Validate our parameters
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or later

  UINT nTimeout = 0;
  if (m_bCreated)
    nTimeout = m_NotifyIconData.uTimeout;

  return nTimeout;
}

BOOL CTrayNotifyIcon::SetTooltipText(_In_ LPCTSTR pszTooltipText)
{
  if (!m_bCreated)
    return FALSE;

  if (m_ShellVersion >= Version5) //Allow the larger size tooltip text if on Shell v5 or later
  {
  #ifdef _DEBUG
    NOTIFYICONDATA_2 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < _countof(dummy.szTip));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  #endif //#ifdef _DEBUG
  }
  else 
  {
  #ifdef _DEBUG
    NOTIFYICONDATA_1 dummy;
    ATLASSERT(_tcslen(pszTooltipText) < _countof(dummy.szTip));
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  #endif //#ifdef _DEBUG
  }

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_TIP;
  _tcsncpy_s(m_NotifyIconData.szTip, _countof(m_NotifyIconData.szTip), pszTooltipText, _TRUNCATE);
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

BOOL CTrayNotifyIcon::SetTooltipText(_In_ UINT nID)
{
  String sToolTipText;
  if (!sToolTipText.LoadString(nID))
    return FALSE;

  //Let the other version of the function handle the rest
  return SetTooltipText(sToolTipText);
}

int	CTrayNotifyIcon::GetTooltipMaxSize()
{
  //Return the cached value if we have one
  if (m_nTooltipMaxSize != -1) 
    return m_nTooltipMaxSize;

  //Otherwise calculate the maximum based on the shell version
  if (m_ShellVersion >= Version5)
  {
    NOTIFYICONDATA_2 dummy;
    m_nTooltipMaxSize = _countof(dummy.szTip) - 1; //The -1 is to allow size for the NULL terminator
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }
  else
  {
    NOTIFYICONDATA_1 dummy;
    m_nTooltipMaxSize = _countof(dummy.szTip) - 1; //The -1 is to allow size for the NULL terminator
    DBG_UNREFERENCED_LOCAL_VARIABLE(dummy);
  }

  return m_nTooltipMaxSize;
}

BOOL CTrayNotifyIcon::SetIcon(_In_ CBitmap* pBitmap)
{
  //Convert the bitmap to an ICON
  if (m_hDynamicIcon != NULL)
    DestroyIcon(m_hDynamicIcon);
  m_hDynamicIcon = BitmapToIcon(pBitmap);

  //Pass the buck to the other function to do the work
  return SetIcon(m_hDynamicIcon);
}

BOOL CTrayNotifyIcon::SetIcon(_In_ HICON hIcon)
{
  //Validate our parameters
  ATLASSERT(hIcon != NULL);

  if (!m_bCreated)
    return FALSE;

  //Since we are going to use one icon, stop any animation
  StopAnimation();

  //Call the Shell_NotifyIcon function
  m_NotifyIconData.uFlags = NIF_ICON;
  m_NotifyIconData.hIcon = hIcon;
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

BOOL CTrayNotifyIcon::SetIcon(_In_ LPCTSTR lpIconName)
{
  return SetIcon(LoadIcon(lpIconName));
}

BOOL CTrayNotifyIcon::SetIcon(_In_ UINT nIDResource)
{
  return SetIcon(LoadIcon(nIDResource));
}

BOOL CTrayNotifyIcon::SetStandardIcon(_In_ LPCTSTR lpIconName)
{
  return SetIcon(::LoadIcon(NULL, lpIconName));
}

BOOL CTrayNotifyIcon::SetStandardIcon(_In_ UINT nIDResource)
{
  return SetIcon(::LoadIcon(NULL, MAKEINTRESOURCE(nIDResource)));
}

BOOL CTrayNotifyIcon::SetIcon(_In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay)
{
  //Validate our parameters
  ATLASSERT(nNumIcons >= 2); //must be using at least 2 icons if you are using animation
  ATLASSUME(phIcons != NULL);
  ATLASSERT(dwDelay);

  if (!SetIcon(phIcons[0]))
    return FALSE;

  //Start the animation
  return StartAnimation(phIcons, nNumIcons, dwDelay);
}

HICON CTrayNotifyIcon::LoadIcon(_In_ HINSTANCE hInstance, _In_ LPCTSTR lpIconName, _In_ BOOL bLargeIcon)
{
  return static_cast<HICON>(::LoadImage(hInstance, lpIconName, IMAGE_ICON, bLargeIcon ? GetSystemMetrics(SM_CXICON) : GetSystemMetrics(SM_CXSMICON), bLargeIcon ? GetSystemMetrics(SM_CYICON) : GetSystemMetrics(SM_CYSMICON), LR_SHARED));
}

HICON CTrayNotifyIcon::LoadIcon(_In_ HINSTANCE hInstance, _In_ UINT nIDResource, _In_ BOOL bLargeIcon)
{
  return LoadIcon(hInstance, MAKEINTRESOURCE(nIDResource), bLargeIcon);
}

HICON CTrayNotifyIcon::LoadIcon(_In_ LPCTSTR lpIconName, _In_ BOOL bLargeIcon)
{
#ifdef _AFX
  return LoadIcon(AfxGetResourceHandle(), lpIconName, bLargeIcon);
#else
  return LoadIcon(ModuleHelper::GetResourceInstance(), lpIconName, bLargeIcon);
#endif //#ifdef _AFX
}

HICON CTrayNotifyIcon::LoadIcon(_In_ UINT nIDResource, _In_ BOOL bLargeIcon)
{
  return LoadIcon(MAKEINTRESOURCE(nIDResource), bLargeIcon);
}

#ifdef _AFX
BOOL CTrayNotifyIcon::SetNotificationWnd(_In_ CWnd* pNotifyWnd)
#else
BOOL CTrayNotifyIcon::SetNotificationWnd(_In_ CWindow* pNotifyWnd)
#endif //#ifdef _AFX
{
  //Validate our parameters
  ATLASSUME((pNotifyWnd != NULL) && ::IsWindow(pNotifyWnd->operator HWND()));

  if (!m_bCreated)
    return FALSE;

  //Call the Shell_NotifyIcon function
  m_pNotificationWnd = pNotifyWnd;
  m_NotifyIconData.hWnd = pNotifyWnd->operator HWND();
  m_NotifyIconData.uFlags = 0;
  return Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

CTrayNotifyIcon::String CTrayNotifyIcon::GetTooltipText() const
{
  String sText;
  if (m_bCreated)
    sText = m_NotifyIconData.szTip;

  return sText;
}

HICON CTrayNotifyIcon::GetIcon() const
{
  HICON hIcon = NULL;
  if (m_bCreated)
  {
    if (UsingAnimatedIcon())
      hIcon = GetCurrentAnimationIcon();
    else
      hIcon = m_NotifyIconData.hIcon;
  }

  return hIcon;
}

#ifdef _AFX
CWnd* CTrayNotifyIcon::GetNotificationWnd() const
#else
CWindow* CTrayNotifyIcon::GetNotificationWnd() const
#endif //#ifdef _AFX
{
  return m_pNotificationWnd;
}

BOOL CTrayNotifyIcon::SetFocus()
{
  ATLASSERT(m_ShellVersion >= Version5); //Only supported on Shell v5 or greater

  //Call the Shell_NotifyIcon function
  return Shell_NotifyIcon(NIM_SETFOCUS, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

LRESULT CTrayNotifyIcon::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
  BOOL bShowMenu = FALSE;
  BOOL bDoubleClick = FALSE;
  BOOL bSingleClick = FALSE;
  UINT nIconID = 0;
  if ((m_NotifyIconData.uVersion == 0) || (m_ShellVersion >= Version5))
  {
    nIconID = static_cast<UINT>(wParam);
    bShowMenu = (lParam == WM_RBUTTONUP);
    bDoubleClick = (lParam == WM_LBUTTONDBLCLK);
	bSingleClick = (lParam == WM_LBUTTONUP);
  }
  else
  {
    nIconID = HIWORD(lParam);
    bShowMenu = (LOWORD(lParam) == WM_CONTEXTMENU);
    bDoubleClick = (LOWORD(lParam) == WM_LBUTTONDBLCLK);
	bSingleClick = (LOWORD(lParam) == WM_LBUTTONUP);
  }

  //Return quickly if its not for this tray icon
  if (nIconID != m_NotifyIconData.uID)
    return 0L;

  //Show the context menu or handle the double click
  if (bShowMenu || bDoubleClick || bSingleClick)
  {
  #ifdef _AFX
    CMenu* pSubMenu = m_Menu.GetSubMenu(0);
    ATLASSUME(pSubMenu != NULL); //Your menu resource has been designed incorrectly
  #else
    CMenuHandle subMenu = m_Menu.GetSubMenu(0);
    ATLASSERT(subMenu.IsMenu());
  #endif //#ifdef _AFX
  
    if (bShowMenu)
    {
      CPoint ptCursor;
      GetCursorPos(&ptCursor);
      ::SetForegroundWindow(m_NotifyIconData.hWnd);  
    #ifdef _AFX
      ::TrackPopupMenu(pSubMenu->m_hMenu, TPM_LEFTBUTTON, ptCursor.x, ptCursor.y, 0, m_NotifyIconData.hWnd, NULL);
    #else
      ::TrackPopupMenu(subMenu, TPM_LEFTBUTTON, ptCursor.x, ptCursor.y, 0, m_NotifyIconData.hWnd, NULL);
    #endif //#ifdef _AFX
      ::PostMessage(m_NotifyIconData.hWnd, WM_NULL, 0, 0);
    }
    else if (bDoubleClick || bSingleClick) //double click received, the default action is to execute first menu item 
    {
      ::SetForegroundWindow(m_NotifyIconData.hWnd);
    #ifdef _AFX
      UINT nDefaultItem = pSubMenu->GetDefaultItem(GMDI_GOINTOPOPUPS, FALSE);
    #else
      UINT nDefaultItem = subMenu.GetMenuDefaultItem(FALSE, GMDI_GOINTOPOPUPS);
    #endif //#ifdef _AFX
      if (nDefaultItem != -1)
        ::SendMessage(m_NotifyIconData.hWnd, WM_COMMAND, nDefaultItem, 0);
    }
  }

  return 1; // handled
}

BOOL CTrayNotifyIcon::GetDynamicDCAndBitmap(_In_ CDC* pDC, _In_ CBitmap* pBitmap)
{
  //Validate our parameters
  ATLASSUME(pDC != NULL);
  ATLASSUME(pBitmap != NULL);

  //Get the HWND for the desktop
#ifdef _AFX
  CWnd* pWndScreen = CWnd::GetDesktopWindow();
  if (pWndScreen == NULL)
    return FALSE;
#else
  CWindow WndScreen(::GetDesktopWindow());
  if (!WndScreen.IsWindow())
    return FALSE;
#endif //#ifdef _AFX

  //Get the desktop HDC to create a compatible bitmap from
#ifdef _AFX
  CDC* pDCScreen = pWndScreen->GetDC();
  if (pDCScreen == NULL)
    return FALSE;
#else
  CDC DCScreen(WndScreen.GetDC());
  if (DCScreen.IsNull())
    return FALSE;
#endif //#ifdef _AFX

  //Get the width and height of a small icon
  int w = GetSystemMetrics(SM_CXSMICON);
  int h = GetSystemMetrics(SM_CYSMICON);

  //Create an off-screen bitmap that the dynamic tray icon 
  //can be drawn into (Compatible with the desktop DC)
#ifdef _AFX
  BOOL bSuccess = pBitmap->CreateCompatibleBitmap(pDCScreen, w, h);
#else
  BOOL bSuccess = (pBitmap->CreateCompatibleBitmap(DCScreen.operator HDC(), w, h) != NULL);
#endif //#ifdef _AFX
  if (!bSuccess)
  {
  #ifdef _AFX
    pWndScreen->ReleaseDC(pDCScreen);
  #else
    WndScreen.ReleaseDC(DCScreen);
  #endif //#ifdef _AFX
    return FALSE;
  }

  //Get a HDC to the newly created off-screen bitmap
#ifdef _AFX
  bSuccess = pDC->CreateCompatibleDC(pDCScreen);
#else
  bSuccess = (pDC->CreateCompatibleDC(DCScreen.operator HDC()) != NULL);
#endif //#ifdef _AFX
  if (!bSuccess)
  {
  //Release the Screen DC now that we are finished with it
  #ifdef _AFX
    pWndScreen->ReleaseDC(pDCScreen);
  #else
    WndScreen.ReleaseDC(DCScreen);
  #endif //#ifdef _AFX

    //Free up the bitmap now that we are finished with it
    pBitmap->DeleteObject();

    return FALSE;
  }

  //Select the bitmap into the offscreen DC
#ifdef _AFX
  pDC->SelectObject(pBitmap);
#else
  pDC->SelectBitmap(pBitmap->operator HBITMAP());
#endif //#ifdef _AFX

  //Release the Screen DC now that we are finished with it
#ifdef _AFX
  pWndScreen->ReleaseDC(pDCScreen);
#else
  WndScreen.ReleaseDC(DCScreen);
#endif //#ifdef _AFX

  return TRUE;
}

DWORD CTrayNotifyIcon::GetNOTIFYICONDATASizeForOS()
{
  //What will be the return value from this function
  DWORD dwSize = sizeof(NOTIFYICONDATA_1);

  switch (m_ShellVersion)
  {
    case Version7: //Deliberate fallthrough
    case VersionVista:
    {
      dwSize = sizeof(NOTIFYICONDATA_4);
      break;
    }
    case Version6:
    {
      dwSize = sizeof(NOTIFYICONDATA_3);
      break;
    }
    case Version5:
    {
      dwSize = sizeof(NOTIFYICONDATA_2);
      break;
    }
    default:
    {
      break;
    }
  }
  
  return dwSize;
}

BOOL CTrayNotifyIcon::StartAnimation(_In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay)
{
  //Validate our parameters
  ATLASSERT(nNumIcons >= 2);  //must be using at least 2 icons if you are using animation
  ATLASSUME(phIcons != NULL); //array of icon handles must be valid
  ATLASSERT(dwDelay);         //must be non zero timer interval

  //Stop the animation if already started  
  StopAnimation();

  //Hive away all the values locally
  ATLASSERT(m_Icons.m_pData == NULL);
  if (!m_Icons.Allocate(nNumIcons))
    return FALSE;
  ATLASSUME(m_Icons.m_pData != NULL);
  for (int i=0; i<nNumIcons; i++)
    m_Icons.m_pData[i] = phIcons[i];
  m_nNumIcons = nNumIcons;

  //Start up the timer 
  m_nTimerID = SetTimer(m_NotifyIconData.uID, dwDelay);

  return TRUE;
}

void CTrayNotifyIcon::StopAnimation()
{
  //Kill the timer
  if (m_nTimerID)
  {
    if (::IsWindow(m_hWnd))
      KillTimer(m_nTimerID);
    m_nTimerID = 0;
  }
 
  //Free up the memory
  if (m_Icons.m_pData != NULL)
    m_Icons.Free();

  //Reset the other animation related variables
  m_nCurrentIconIndex = 0;
  m_nNumIcons = 0;
}

BOOL CTrayNotifyIcon::UsingAnimatedIcon() const
{
  return (m_nNumIcons != 0);
}

HICON CTrayNotifyIcon::GetCurrentAnimationIcon() const 
{ 
  //Valiate our parameters
  ATLASSERT(UsingAnimatedIcon());
  ATLASSUME(m_Icons.m_pData != NULL);
  
  return m_Icons.m_pData[m_nCurrentIconIndex];
}

BOOL CTrayNotifyIcon::ProcessWindowMessage(_In_ HWND /*hWnd*/, _In_ UINT nMsg, _In_ WPARAM wParam, _In_ LPARAM lParam, _Inout_ LRESULT& lResult, _In_ DWORD /*dwMsgMapID*/)
{
  lResult = 0;
  BOOL bHandled = FALSE;

  if (nMsg == wm_TaskbarCreated)
  {
    lResult = OnTaskbarCreated(wParam, lParam);
    bHandled = TRUE;
  }
  else if ((nMsg == WM_TIMER) && (wParam == m_NotifyIconData.uID))
  {
    OnTimer(m_NotifyIconData.uID); 
    bHandled = TRUE;
  }
  else if (nMsg == WM_DESTROY)
  {
    OnDestroy();
    bHandled = TRUE;
  }

  return bHandled;
}

void CTrayNotifyIcon::OnDestroy()
{
  StopAnimation();
}

LRESULT CTrayNotifyIcon::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
  //Refresh the tray icon if necessary
  BOOL bShowing = IsShowing();
  Delete(FALSE);
  Create(bShowing);
  
  return 0;
}

#ifdef _DEBUG
void CTrayNotifyIcon::OnTimer(UINT_PTR nIDEvent)
#else
void CTrayNotifyIcon::OnTimer(UINT_PTR /*nIDEvent*/)
#endif //#ifdef _DEBUG
{
  //Validate our parameters
  ATLASSERT(nIDEvent == m_nTimerID);
  ATLASSUME(m_Icons.m_pData != NULL);

  //increment the icon index
  ++m_nCurrentIconIndex;
  m_nCurrentIconIndex = m_nCurrentIconIndex % m_nNumIcons;

  //update the tray icon
  m_NotifyIconData.uFlags = NIF_ICON;
  m_NotifyIconData.hIcon = m_Icons.m_pData[m_nCurrentIconIndex];
  Shell_NotifyIcon(NIM_MODIFY, reinterpret_cast<PNOTIFYICONDATA>(&m_NotifyIconData));
}

BOOL CTrayNotifyIcon::CreateHelperWindow()
{
  //Let the base class do its thing
  return (CWindowImpl<CTrayNotifyIcon>::Create(NULL, CWindow::rcDefault, _T("CTrayNotifyIcon Helper Window"), WS_OVERLAPPEDWINDOW) != NULL);
}

CTrayWnd  CTrayNotifyIcon::m_wndInvisible;

BOOL CTrayNotifyIcon::RemoveTaskbarIcon(CWnd* pWnd)
{
	LPCTSTR pstrOwnerClass = AfxRegisterWndClass(0);

	// Create static invisible window
	if (!::IsWindow(m_wndInvisible.m_hWnd))
	{
		if (!m_wndInvisible.CreateEx(0, pstrOwnerClass, _T(""), WS_POPUP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, 0))
		{
			return FALSE;
		}
	}

	pWnd->SetParent(&m_wndInvisible);

	return TRUE;
}

void CTrayNotifyIcon::MinimiseToTray(CWnd* pWnd)
{
	RemoveTaskbarIcon(pWnd);
	pWnd->ModifyStyle(WS_VISIBLE, 0);
}

void CTrayNotifyIcon::MaximiseFromTray(CWnd* pWnd)
{
	pWnd->SetParent(NULL);

	pWnd->ModifyStyle(0, WS_VISIBLE);
	pWnd->RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME |
		RDW_INVALIDATE | RDW_ERASE);

	// Move focus away and back again to ensure taskbar icon is recreated
	if (::IsWindow(m_wndInvisible.m_hWnd))
		m_wndInvisible.SetActiveWindow();
	pWnd->SetActiveWindow();
	pWnd->SetForegroundWindow();
}