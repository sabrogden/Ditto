// GroupStatic.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "GroupStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupStatic

CGroupStatic::CGroupStatic()
{
	m_dwTextColor = 0;
	m_dwBkColor = RGB(255, 255, 255);
	m_toggleCursorToHand = false;
	m_pFont = NULL;
}

CGroupStatic::~CGroupStatic()
{
}


BEGIN_MESSAGE_MAP(CGroupStatic, CStatic)
	//{{AFX_MSG_MAP(CGroupStatic)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupStatic message handlers

BOOL CGroupStatic::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{  
#ifndef WIN32
      return CStatic::OnChildNotify(message, wParam, lParam, pLResult);
#else
   // If not setting static control color, do default processing
   if( message != WM_CTLCOLORSTATIC )
      return CStatic::OnChildNotify(message, wParam, lParam, pLResult);

   HDC hdcChild = (HDC)wParam;

   // Set the foreground color
   ::SetTextColor( hdcChild, m_dwTextColor );

   // If a background color is pre-determined
   if(m_dwBkColor != -1)
   {  
      ::SetBkMode(hdcChild, TRANSPARENT);
      ::SetBkColor(hdcChild, m_dwBkColor);  
	   m_brush.DeleteObject();
	   m_brush.CreateSolidBrush(m_dwBkColor);
      *pLResult = (LRESULT)(m_brush.GetSafeHandle());
   }
   else
   {
   // Determine the current background color based on my parent window
      static COLORREF clrPrevValid = -1;
      HWND hParent = ::GetParent(m_hWnd);
      HDC  hParentDc = ::GetDC(hParent);

      // Get the color based on the 0, 0 reference
      COLORREF clrParentBkground = ::GetPixel(hParentDc, 0, 0);
      ::ReleaseDC(hParent, hParentDc);

      // If found (not off of the screen or under another window)
      // set my current color to it
      if(clrParentBkground == -1)
      {  clrParentBkground = clrPrevValid;  }
      else
      {  clrPrevValid = clrParentBkground;  }

      // If either the current, or previous color found was not valid
      // allow to perform default processing
      if(clrParentBkground == -1)
      {  return FALSE;  }


      // Set the background mode to transparent
      ::SetBkMode(hdcChild, TRANSPARENT);

      // Set the background color and brush based on my parent's color
      ::SetBkColor(hdcChild, clrParentBkground);  
	   m_brush.DeleteObject();
	   m_brush.CreateSolidBrush(clrParentBkground);
      *pLResult = (LRESULT)(m_brush.GetSafeHandle());
   }

   // Return TRUE to indicate that the message was handled
   return TRUE;
#endif
}


/*************************************************************************
*
*************************************************************************/
void CGroupStatic::SetFont( int nPointSize, LPCTSTR lpszFaceName, CDC* pDC )
{
   // If a font has been allocated, delete it
   if( m_pFont )
      delete m_pFont;

   m_pFont = new CFont;

   // Create a font using the given attributes
   m_pFont->CreatePointFont( nPointSize, lpszFaceName, pDC );

   // Set the window's current font to the specified font
   CStatic::SetFont( m_pFont );
}

BOOL CGroupStatic::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_toggleCursorToHand)
	{
		HCURSOR h = ::LoadCursor(NULL, IDC_HAND);
		::SetCursor(h);
		return TRUE;
	}

	return FALSE;
}