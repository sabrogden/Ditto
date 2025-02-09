// AlphaBlend.h: interface for the CAlphaBlend class.
//
//////////////////////////////////////////////////////////////////////
// By Mike Ryan (mike@codexia.com)
// Copyright (c) 2000
// 07.03.2000
//
// Free usage granted in all applications including commercial.
// Do NOT distribute without permission from me.  I can be reached
// at mike@codexia.com, http://www.codexia.com
// Please feel free to email me about this class.
//
// NOTE:
//
// You will need the latest Win32 API availble at:
// http://msdn.microsoft.com/downloads/c-frame.htm?007#/downloads/sdks/
// in order to compile this programs.
//
// This will only run under Windows 2000.  It is not compatible with
// Windows 9x or Windows NT 4.0.
//
// Also, you must add the following lines to the StdAfx.h at the top
// above the #define VC_EXTRALEAN.
//
//	#define _WIN32_WINNT 0x0500
//	#define WINVER 0x0500
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALPHABLEND_H__B453CE58_5571_48B4_BD84_3BB70D6A41A3__INCLUDED_)
#define AFX_ALPHABLEND_H__B453CE58_5571_48B4_BD84_3BB70D6A41A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define OPACITY_MAX			255

class CAlphaBlend  
{
public:
	CAlphaBlend();
	CAlphaBlend(HWND hWnd, int nOpacity=220);
	CAlphaBlend(CWnd *pWnd, int nOpacity=220);
	virtual ~CAlphaBlend();

	//// SET DATA MEMBERS ////
	BOOL SetWindowHandle(HWND hWnd);
	BOOL SetWindowHandle(CWnd *pWnd);
	BOOL SetOpacity(int nOpacity=220);

	//// RUN IT ////
	void SetTransparent(BOOL bTransparent=true);

	//// FOR THOSE OF US WHO ARE LAZY ////
	BOOL SetTransparent(HWND hWnd, int nOpactiy=220, BOOL bTransparent=true);

	//// GET TRANSPARENT/OPACITY STATUS ////
	int GetOpacity(void) { return m_nOpacity; }
	HWND GetWindow(void) { return m_hWnd; }
	BOOL IsTransparent(void) { return m_bTransparent; }

protected:
	int m_nOpacity;
	HWND m_hWnd;

	BOOL m_bTransparent;

	BOOL SetLayeredWindowAttributesEx(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
};

#endif // !defined(AFX_ALPHABLEND_H__B453CE58_5571_48B4_BD84_3BB70D6A41A3__INCLUDED_)
