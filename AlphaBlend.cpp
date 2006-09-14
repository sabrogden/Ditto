// AlphaBlend.cpp: implementation of the CAlphaBlend class.
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

#include "stdafx.h"
#include "AlphaBlend.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAlphaBlend::CAlphaBlend()
{
	m_hWnd = NULL;
	m_nOpacity = 0;
	m_bTransparent = false;
}

CAlphaBlend::~CAlphaBlend()
{
}

CAlphaBlend::CAlphaBlend(HWND hWnd, int nOpactiy)
{
	m_hWnd = NULL;
	m_nOpacity = 0;
	m_bTransparent = false;

	SetWindowHandle(hWnd);
	SetOpacity(nOpactiy);
}

CAlphaBlend::CAlphaBlend(CWnd *pWnd, int nOpacity)
{
	m_hWnd = NULL;
	m_nOpacity = 0;
	m_bTransparent = false;

	SetWindowHandle(pWnd);
	SetOpacity(nOpacity);
}

BOOL CAlphaBlend::SetWindowHandle(HWND hWnd)
{
	if (::IsWindow(hWnd)) m_hWnd = hWnd;
	else return false;

	return true;
}

BOOL CAlphaBlend::SetWindowHandle(CWnd *pWnd)
{
	if (pWnd && ::IsWindow(pWnd->GetSafeHwnd())) 
		m_hWnd = pWnd->GetSafeHwnd();
	else 
		return false;
	return true;
}

BOOL CAlphaBlend::SetOpacity(int nOpacity)
{
	if (nOpacity >= 0 && nOpacity <= OPACITY_MAX)
	{
		m_nOpacity = nOpacity;
		if (m_bTransparent)
		{
			// update the transparency
			ASSERT(::IsWindow(m_hWnd));
			SetLayeredWindowAttributesEx(m_hWnd, 0, m_nOpacity, LWA_ALPHA);		
		}
		return true;
	}
	return false;
}

BOOL CAlphaBlend::SetLayeredWindowAttributesEx(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	BOOL bRet = FALSE;
	typedef BOOL (CALLBACK* fnc)(HWND, COLORREF, BYTE, DWORD);
	HINSTANCE DLL;
	fnc setLayeredWindowAttributes;
	
	DLL = LoadLibrary(_T("user32.dll"));
	if(DLL != NULL)
	{
		setLayeredWindowAttributes = (fnc)GetProcAddress(DLL,"SetLayeredWindowAttributes");  
		
		if(setLayeredWindowAttributes) 
		{
			bRet = setLayeredWindowAttributes(hwnd, crKey, bAlpha, dwFlags);
		}
				
		FreeLibrary(DLL);
	}

	return bRet;
}

void CAlphaBlend::SetTransparent(BOOL bTransparent)
{
	if (bTransparent)
	{
		// make sure they have set it up properly
		ASSERT(m_nOpacity >= 0 && m_nOpacity <= OPACITY_MAX);
		ASSERT(m_hWnd && ::IsWindow(m_hWnd));

		// make it transparent
		long l = GetWindowLong(m_hWnd, GWL_EXSTYLE);
		if(!(l & WS_EX_LAYERED))
		{
			l |= WS_EX_LAYERED;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, l);
		}

		SetLayeredWindowAttributesEx(m_hWnd, 0, m_nOpacity, LWA_ALPHA);

		CRect r;
		::GetWindowRect(m_hWnd, r);
		::InvalidateRect(m_hWnd, r, true);

		m_bTransparent = true;
	}
	else
	{
		long l = GetWindowLong(m_hWnd, GWL_EXSTYLE);
		if(l & WS_EX_LAYERED)
		{
			l ^= WS_EX_LAYERED;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, l);
			CRect r;
			::GetWindowRect(m_hWnd, r);
			::InvalidateRect(m_hWnd, r, true);
		}
		m_bTransparent = false;
	}
}

BOOL CAlphaBlend::SetTransparent(HWND hWnd, int nOpacity, BOOL bTransparent)
{
	// set members
	if (!SetWindowHandle(hWnd)) 
		return false;

	if (!SetOpacity(nOpacity)) 
		return false;

	SetTransparent(bTransparent);

	return true;
}
