/*
Module : NTray.h
Purpose: Interface for a MFC class to encapsulate Shell_NotifyIcon
Created: PJN / 14-05-1997

Copyright (c) 1997 - 2016 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////// Macros / Defines ///////////////////////////

#pragma once

#ifndef _NTRAY_H__
#define _NTRAY_H__

#ifndef CTRAYNOTIFYICON_EXT_CLASS
#define CTRAYNOTIFYICON_EXT_CLASS
#endif //#ifndef CTRAYNOTIFYICON_EXT_CLASS

#ifndef _In_
#define _In_
#endif //#ifndef _In_

#ifndef _In_opt_
#define _In_opt_
#endif //#ifndef _In_opt_

#ifndef _Out_
#define _Out_
#endif //#ifndef _Out_

#ifndef _Inout_
#define _Inout_
#endif //#ifndef _Inout_

#ifndef __ATLWIN_H__
#pragma message("CTrayNotifyIcon as of v1.51 requires ATL support to implement its functionality. If your project is MFC only, then you need to update it to include ATL support")
#endif //#ifndef __ATLWIN_H__

#ifndef _AFX
#ifndef __ATLMISC_H__
#pragma message("To avoid this message, please put atlmisc.h (part of WTL) in your pre compiled header (normally stdafx.h)")
#include <atlmisc.h> //If you do want to use CTrayNotifyIcon independent of MFC, then you need to download and install WTL from http://sourceforge.net/projects/wtl
#endif //#ifndef __ATLMISC_H__
#endif //#ifndef _AFX

#include "TrayWnd.h"


/////////////////////////// Classes ///////////////////////////////////////////

//the actual tray notification class wrapper
class CTRAYNOTIFYICON_EXT_CLASS CTrayNotifyIcon : public CWindowImpl<CTrayNotifyIcon>
{
public:
//Enums / Typedefs
 #ifndef _AFX
  typedef _CSTRING_NS::CString String;
#else
  typedef CString String;
#endif //#ifndef _AFX

  enum BalloonStyle
  {
    Warning,
    Error,
    Info,
    None,
    User
  };

  //We use our own definitions of the NOTIFYICONDATA structs so that
  //we can use all the functionality without requiring client code to 
  //define _WIN32_IE >= 0x500
  typedef struct _NOTIFYICONDATA_1 //The version of the structure supported by Shell v4
  {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[64];
  } NOTIFYICONDATA_1;

  typedef struct _NOTIFYICONDATA_2 //The version of the structure supported by Shell v5
  {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    TCHAR szInfo[256];
    union 
    {
      UINT uTimeout;
      UINT uVersion;
    } DUMMYUNIONNAME;
    TCHAR szInfoTitle[64];
    DWORD dwInfoFlags;
  } NOTIFYICONDATA_2;

  typedef struct _NOTIFYICONDATA_3 //The version of the structure supported by Shell v6
  {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    TCHAR szInfo[256];
    union 
    {
      UINT uTimeout;
      UINT uVersion;
    } DUMMYUNIONNAME;
    TCHAR szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
  } NOTIFYICONDATA_3;

  typedef struct _NOTIFYICONDATA_4 //The version of the structure supported by Shell v7
  {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    TCHAR szInfo[256];
    union 
    {
      UINT uTimeout;
      UINT uVersion;
    } DUMMYUNIONNAME;
    TCHAR szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
    HICON hBalloonIcon;
  } NOTIFYICONDATA_4;

  DECLARE_WND_CLASS(_T("TrayNotifyIconClass"))


//Constructors / Destructors
  CTrayNotifyIcon();
  virtual ~CTrayNotifyIcon();

//Create the tray icon
#ifdef _AFX
  BOOL Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bNoSound = FALSE, _In_ BOOL bLargeIcon = FALSE, _In_ BOOL bRealtime = FALSE, _In_opt_ HICON hBalloonIcon = NULL, _In_ BOOL bQuietTime = FALSE, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bNoSound = FALSE, _In_ BOOL bLargeIcon = FALSE, _In_ BOOL bRealtime = FALSE, _In_opt_ HICON hBalloonIcon = NULL, _In_ BOOL bQuietTime = FALSE, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWnd* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bNoSound = FALSE, _In_ BOOL bLargeIcon = FALSE, _In_ BOOL bRealtime = FALSE, _In_opt_ HICON hBalloonIcon = NULL, _In_ BOOL bQuietTime = FALSE, _In_ BOOL bShow = TRUE);
#else
  BOOL Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON hIcon, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bNoSound = FALSE, _In_ BOOL bLargeIcon = FALSE, _In_ BOOL bRealtime = FALSE, _In_opt_ HICON hBalloonIcon = NULL, _In_ BOOL bQuietTime = FALSE, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ CBitmap* pBitmap, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bNoSound = FALSE, _In_ BOOL bLargeIcon = FALSE, _In_ BOOL bRealtime = FALSE, _In_opt_ HICON hBalloonIcon = NULL, _In_ BOOL bQuietTime = FALSE, _In_ BOOL bShow = TRUE);
  BOOL Create(_In_ CWindow* pNotifyWnd, _In_ UINT uID, _In_ LPCTSTR pszTooltipText, _In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ UINT nTimeout, _In_ BalloonStyle style, _In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay, _In_ UINT nNotifyMessage, _In_ UINT uMenuID = 0, _In_ BOOL bNoSound = FALSE, _In_ BOOL bLargeIcon = FALSE, _In_ BOOL bRealtime = FALSE, _In_opt_ HICON hBalloonIcon = NULL, _In_ BOOL bQuietTime = FALSE, _In_ BOOL bShow = TRUE);
#endif //#ifdef _AFX

  BOOL RemoveTaskbarIcon(CWnd* pWnd);
  void MinimiseToTray(CWnd* pWnd);
  void MaximiseFromTray(CWnd* pWnd);

//Sets or gets the Tooltip text
  BOOL   SetTooltipText(_In_ LPCTSTR pszTooltipText);
  BOOL   SetTooltipText(_In_ UINT nID);
  String GetTooltipText() const;
  int	   GetTooltipMaxSize();

//Sets or gets the icon displayed
  BOOL SetIcon(_In_ HICON hIcon);
  BOOL SetIcon(_In_ CBitmap* pBitmap);
  BOOL SetIcon(_In_ LPCTSTR lpIconName);
  BOOL SetIcon(_In_ UINT nIDResource);
  BOOL SetIcon(_In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay);
  BOOL SetStandardIcon(_In_ LPCTSTR lpIconName);
  BOOL SetStandardIcon(_In_ UINT nIDResource);
  HICON GetIcon() const;
  BOOL UsingAnimatedIcon() const;

//Sets or gets the window to send notification messages to
#ifdef _AFX
  BOOL SetNotificationWnd(_In_ CWnd* pNotifyWnd);
  CWnd* GetNotificationWnd() const;
#else
  BOOL SetNotificationWnd(_In_ CWindow* pNotifyWnd);
  CWindow* GetNotificationWnd() const;
#endif //#ifdef _AFX

//Modification of the tray icons
  BOOL Delete(_In_ BOOL bCloseHelperWindow = TRUE);
  BOOL Create(_In_ BOOL bShow = TRUE);
  BOOL Hide();
  BOOL Show();

//Dynamic tray icon support
  HICON BitmapToIcon(_In_ CBitmap* pBitmap);
  static BOOL GetDynamicDCAndBitmap(_In_ CDC* pDC, _In_ CBitmap* pBitmap);

//Modification of the menu to use as the context menu
  void SetMenu(_In_ HMENU hMenu, UINT menuId);
  CMenu& GetMenu();
  void SetDefaultMenuItem(_In_ UINT uItem, _In_ BOOL fByPos);
  void GetDefaultMenuItem(_Out_ UINT& uItem, _Out_ BOOL& fByPos) { uItem = m_nDefaultMenuItem; fByPos = m_bDefaultMenuItemByPos; };

//Default handler for tray notification message
  virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

//Status information
  BOOL IsShowing() const { return !IsHidden(); };
  BOOL IsHidden() const { return m_bHidden; };

//Sets or gets the Balloon style tooltip settings
  BOOL         SetBalloonDetails(_In_ LPCTSTR pszBalloonText, _In_ LPCTSTR pszBalloonCaption, _In_ BalloonStyle style, _In_ UINT nTimeout, _In_ HICON hUserIcon = NULL, _In_ BOOL bNoSound = TRUE, _In_ BOOL bLargeIcon = FALSE, _In_ BOOL bRealtime = FALSE, _In_ HICON hBalloonIcon = NULL);
  String       GetBalloonText() const;
  String       GetBalloonCaption() const;
  BalloonStyle GetBalloonStyle() const;
  UINT         GetBalloonTimeout() const;

//Other functionality
  BOOL SetVersion(_In_ UINT uVersion);
  BOOL SetFocus();

//Helper functions to load tray icon from resources
  static HICON LoadIcon(_In_ LPCTSTR lpIconName, _In_ BOOL bLargeIcon = FALSE);
  static HICON LoadIcon(_In_ UINT nIDResource, _In_ BOOL bLargeIcon = FALSE);
  static HICON LoadIcon(_In_ HINSTANCE hInstance, _In_ LPCTSTR lpIconName, _In_ BOOL bLargeIcon = FALSE);
  static HICON LoadIcon(_In_ HINSTANCE hInstance, _In_ UINT nIDResource, _In_ BOOL bLargeIcon = FALSE);

protected:
//Methods
  BOOL         CreateHelperWindow();
  BOOL         StartAnimation(_In_ HICON* phIcons, _In_ int nNumIcons, _In_ DWORD dwDelay);
  void         StopAnimation();
  HICON        GetCurrentAnimationIcon() const;
  virtual BOOL ProcessWindowMessage(_In_ HWND hWnd, _In_ UINT nMsg, _In_ WPARAM wParam, _In_ LPARAM lParam, _Inout_ LRESULT& lResult, _In_ DWORD dwMsgMapID);
  LRESULT      OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
  void         OnTimer(UINT_PTR nIDEvent);
  void         OnDestroy();
  DWORD        GetNOTIFYICONDATASizeForOS();

  static CTrayWnd  m_wndInvisible;

//Enums
  enum ShellVersion
  {
    Version4     = 0, //PreWin2k
    Version5     = 1, //Win2k
    Version6     = 2, //XP
    VersionVista = 3, //Vista
    Version7     = 4, //Windows7
  };

//Member variables
  NOTIFYICONDATA_4     m_NotifyIconData;
  BOOL                 m_bCreated;
  BOOL                 m_bHidden;
#ifdef _AFX
  CWnd*                m_pNotificationWnd;
#else
  CWindow*             m_pNotificationWnd;
#endif //#ifdef _AFX
  CMenu                m_Menu;
  UINT                 m_nDefaultMenuItem;
  BOOL                 m_bDefaultMenuItemByPos;
  ShellVersion         m_ShellVersion;
  HICON                m_hDynamicIcon; //Our cached copy of the last icon created with BitmapToIcon
  ATL::CHeapPtr<HICON> m_Icons;
  int                  m_nNumIcons;
  UINT_PTR             m_nTimerID;
  int                  m_nCurrentIconIndex;
  int                  m_nTooltipMaxSize;
};

#endif //#ifndef _NTRAY_H__
