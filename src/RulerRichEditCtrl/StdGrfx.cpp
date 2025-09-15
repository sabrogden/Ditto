/* ==========================================================================
	File :			StdGrfx.cpp

	Class :			CStdGrfx

	Author :		Johan Rosengren, Abstrakt Mekanik AB

	Date :			2004-03-31

	Purpose :		Static graphics helper class. Will return pens and 
					brushes in the current Windows colors. Will also draw 
					some different kind of boxes, as they seem not to 
					scale well. Neither does these, but better than 
					Draw3dFrame et. al.

	Description :	

	Usage :			

   ========================================================================*/
#include "stdafx.h"
#include "stdgrfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPen	CStdGrfx::s_shadowPen;
CPen	CStdGrfx::s_darkshadowPen;
CPen	CStdGrfx::s_lightPen;
CPen	CStdGrfx::s_highlightPen;
CPen	CStdGrfx::s_dialogPen;
CPen	CStdGrfx::s_windowPen;
CPen	CStdGrfx::s_scrollPen;
CBrush	CStdGrfx::s_dialogBrush;
CBrush	CStdGrfx::s_backgroundBrush;
CBrush	CStdGrfx::s_windowBrush;
CBrush	CStdGrfx::s_scrollBrush;

/////////////////////////////////////////////////////////////////////////////
// CStdGrfx
//

CPen* CStdGrfx::shadowPen()
/* ============================================================
	Function :		CStdGrfx::shadowPen
	Description :	Returns a pen with the current 3d shadow 
					color (dark gray).
	Access :		Public
					
	Return :		CPen*	-	The pen
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_shadowPen.m_hObject == NULL )
		CStdGrfx::s_shadowPen.CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DSHADOW ) );
	return &CStdGrfx::s_shadowPen;

}

CPen* CStdGrfx::darkshadowPen()
/* ============================================================
	Function :		CStdGrfx::darkshadowPen
	Description :	Returns a pen with the current 3d dark 
					shadow color (black).
	Access :		Public
					
	Return :		CPen*	-	The pen
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_darkshadowPen.m_hObject == NULL )
		CStdGrfx::s_darkshadowPen.CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DDKSHADOW ) );
	return &CStdGrfx::s_darkshadowPen;

}

CPen* CStdGrfx::lightPen()
/* ============================================================
	Function :		CStdGrfx::lightPen
	Description :	Returns a pen with the current 3d light 
					color (light gray).
	Access :		Public
					
	Return :		CPen*	-	The pen
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_lightPen.m_hObject == NULL )
		CStdGrfx::s_lightPen.CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DLIGHT ) );
	return &CStdGrfx::s_lightPen;

}

CPen* CStdGrfx::highlightPen()
/* ============================================================
	Function :		CStdGrfx::highlightPen
	Description :	Returns a pen with the current 3d highligh 
					color (white).
	Access :		Public
					
	Return :		CPen*	-	The pen
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_highlightPen.m_hObject == NULL )
		CStdGrfx::s_highlightPen.CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DHILIGHT ) );
	return &CStdGrfx::s_highlightPen;

}

CPen* CStdGrfx::dialogPen()
/* ============================================================
	Function :		CStdGrfx::dialogPen
	Description :	Returns a pen with the current dialog
					background color
	Access :		Public
					
	Return :		CPen*	-	The pen
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_dialogPen.m_hObject == NULL )
		CStdGrfx::s_dialogPen.CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DFACE ) );
	return &CStdGrfx::s_dialogPen;

}

CPen* CStdGrfx::windowPen()
/* ============================================================
	Function :		CStdGrfx::windowPen
	Description :	Returns a pen with the current window 
					background color
	Access :		Public
					
	Return :		CPen*	-	The pen
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_windowPen.m_hObject == NULL )
		CStdGrfx::s_windowPen.CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_WINDOW ) );
	return &CStdGrfx::s_windowPen;

}

CPen* CStdGrfx::scrollPen()
/* ============================================================
	Function :		CStdGrfx::scrollPen
	Description :	Returns a pen with the current scrollbar
					background color
	Access :		Public
					
	Return :		CPen*	-	The pen
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_scrollPen.m_hObject == NULL )
		CStdGrfx::s_scrollPen.CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_SCROLLBAR ) );
	return &CStdGrfx::s_scrollPen;

}

CBrush* CStdGrfx::dialogBrush()
/* ============================================================
	Function :		CStdGrfx::dialogBrush
	Description :	Returns a brush with the current dialog 
					color.
	Access :		Public
					
	Return :		CBrush*	-	The brush
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_dialogBrush.m_hObject == NULL )
		CStdGrfx::s_dialogBrush.CreateSolidBrush( ::GetSysColor( COLOR_3DFACE ) );
	return &CStdGrfx::s_dialogBrush;

}

CBrush* CStdGrfx::windowBrush()
/* ============================================================
	Function :		CStdGrfx::windowBrush
	Description :	Returns a brush with the current window 
					color.
	Access :		Public
					
	Return :		CBrush*	-	The brush
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_windowBrush.m_hObject == NULL )
		CStdGrfx::s_windowBrush.CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
	return &CStdGrfx::s_windowBrush;

}

CBrush* CStdGrfx::scrollBrush()
/* ============================================================
	Function :		CStdGrfx::scrollBrush
	Description :	Returns a brush with the current scrollbar
					color.
	Access :		Public
					
	Return :		CBrush*	-	The brush
	Parameters :	none

	Usage :			Static function.

   ============================================================*/
{

	if( CStdGrfx::s_scrollBrush.m_hObject == NULL )
		CStdGrfx::s_scrollBrush.CreateSolidBrush( ::GetSysColor( COLOR_SCROLLBAR ) );
	return &CStdGrfx::s_scrollBrush;

}

void CStdGrfx::drawframed3dBox( CDC* dc, CRect rect )
/* ============================================================
	Function :		CStdGrfx::drawframed3dBox
	Description :	Draws a 3d rect with a black frame.
	Access :		Public
					
	Return :		void
	Parameters :	CDC* dc		-	The "CDC" to draw to
					CRect rect	-	The rectangle to draw
					
	Usage :			Static function.

   ============================================================*/
{

	dc->SelectObject( CStdGrfx::darkshadowPen() );
	dc->SelectObject( CStdGrfx::dialogBrush() );

	dc->Rectangle( rect );
	rect.InflateRect( -1, -1 );
	CStdGrfx::draw3dFrame( dc, rect );

	dc->SelectStockObject( BLACK_PEN );
	dc->SelectStockObject( WHITE_BRUSH );

}

void CStdGrfx::drawsunkenframed3dWindow( CDC* dc, CRect rect )
/* ============================================================
	Function :		CStdGrfx::drawsunkenframed3dWindow
	Description :	Draws a sunken 3d rect with a black frame.
	Access :		Public
					
	Return :		void
	Parameters :	CDC* dc		-	The "CDC" to draw to
					CRect rect	-	The rectangle to draw
					
	Usage :			Static function.

   ============================================================*/
{

	dc->SelectObject( CStdGrfx::windowPen() );
	dc->SelectObject( CStdGrfx::windowBrush() );

	dc->Rectangle( rect );
	CStdGrfx::drawdoublesunken3dFrame( dc, rect );

	dc->SelectStockObject( BLACK_PEN );
	dc->SelectStockObject( WHITE_BRUSH );

}

void CStdGrfx::draw3dFrame( CDC* dc, CRect rect )
/* ============================================================
	Function :		CStdGrfx::draw3dFrame
	Description :	Draws a 3d rect.
	Access :		Public
					
	Return :		void
	Parameters :	CDC* dc		-	The "CDC" to draw to
					CRect rect	-	The rectangle to draw
					
	Usage :			Static function.

   ============================================================*/
{

	rect.InflateRect( 0, 0, -1, -1 );

	dc->SelectObject( CStdGrfx::highlightPen() );
	dc->MoveTo( rect.left, rect.bottom );
	dc->LineTo( rect.left, rect.top );
	dc->LineTo( rect.right , rect.top );
	dc->SelectObject( CStdGrfx::shadowPen() );
	dc->LineTo( rect.right , rect.bottom );
	dc->LineTo( rect.left, rect.bottom );

	dc->SelectStockObject( BLACK_PEN );

}

void CStdGrfx::drawsunken3dFrame( CDC* dc, CRect rect )
/* ============================================================
	Function :		CStdGrfx::drawsunken3dFrame
	Description :	Draws a sunken 3d rect.
	Access :		Public
					
	Return :		void
	Parameters :	CDC* dc		-	The "CDC" to draw to
					CRect rect	-	The rectangle to draw
					
	Usage :			Static function.

   ============================================================*/
{

	rect.InflateRect( 0, 0, -1, -1 );

	dc->SelectObject( CStdGrfx::shadowPen() );
	dc->MoveTo( rect.left, rect.bottom );
	dc->LineTo( rect.left, rect.top );
	dc->LineTo( rect.right , rect.top );
	dc->SelectObject( CStdGrfx::highlightPen() );
	dc->LineTo( rect.right , rect.bottom );
	dc->LineTo( rect.left, rect.bottom );

	dc->SelectStockObject( BLACK_PEN );

}

void CStdGrfx::drawdoublesunken3dFrame( CDC* dc, CRect rect )
/* ============================================================
	Function :		CStdGrfx::drawdoublesunken3dFrame
	Description :	Draws a double sunken 3d rect.
	Access :		Public
					
	Return :		void
	Parameters :	CDC* dc		-	The "CDC" to draw to
					CRect rect	-	The rectangle to draw
					
	Usage :			Static function.

   ============================================================*/
{

	rect.InflateRect( 0, 0, -1, -1 );

	dc->SelectObject( CStdGrfx::shadowPen() );
	dc->MoveTo( rect.left, rect.bottom );
	dc->LineTo( rect.left, rect.top );
	dc->LineTo( rect.right , rect.top );

	dc->SelectObject( CStdGrfx::highlightPen() );
	dc->LineTo( rect.right , rect.bottom );
	dc->LineTo( rect.left, rect.bottom );

	rect.InflateRect( -1, -1 );

	dc->SelectObject( CStdGrfx::darkshadowPen() );
	dc->MoveTo( rect.left, rect.bottom );
	dc->LineTo( rect.left, rect.top );
	dc->LineTo( rect.right , rect.top );

	dc->SelectObject( CStdGrfx::lightPen() );
	dc->LineTo( rect.right , rect.bottom );
	dc->LineTo( rect.left, rect.bottom );

	dc->SelectStockObject( BLACK_PEN );

}
