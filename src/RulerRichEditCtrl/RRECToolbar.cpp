/* ==========================================================================
	File :			RRECToolbar.cpp

	Class :			CRRECToolbar

	Author :		Johan Rosengren, Abstrakt Mekanik AB
					Iain Clarke

	Date :			2004-05-07

	Purpose :		This class encapsulates a toolbar that can be used with 
					"CRulerRichEditCtrl". The class is derived from "CToolBarCtrl", 
					and manages a formatting toolbar

	Description :	A "CToolBarCtrl"-derived class. Reads a toolbar resource 
					with the ID "TOOLBAR_CONTROL" and adds combo controls for 
					font name and -size, as well as a color picker at the 
					positions "FONT_NAME_POS", "FONT_SIZE_POS" and 
					"FONT_COLOR_POS" respectively.

	Usage :			Created by the rich edit mini-editor.

   ========================================================================*/

#include "stdafx.h"
#include "RRECToolbar.h"
#include "ids.h"

#include <tchar.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern UINT urm_SETCURRENTFONTNAME;
extern UINT urm_SETCURRENTFONTSIZE;
extern UINT urm_SETCURRENTFONTCOLOR;

/////////////////////////////////////////////////////////////////////////////
// CRRECToolbar

CRRECToolbar::CRRECToolbar()
/* ============================================================
	Function :		CRRECToolbar::CRRECToolbar
	Description :	ctor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

CRRECToolbar::~CRRECToolbar()
/* ============================================================
	Function :		CRRECToolbar::~CRRECToolbar
	Description :	dtor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

BOOL CRRECToolbar::Create( CWnd* parent, CRect& rc )
/* ============================================================
	Function :		CRRECToolbar::Create
	Description :	Creates the toolbar control
	Access :		Public
					
	Return :		BOOL			-	"TRUE" if success
	Parameters :	CWnd* parent	-	Parent editor
					CRect& rc		-	Rectangle to place 
										toolbar in.

	Usage :			Called from the parent editor

   ============================================================*/
{

	BOOL result = FALSE;

	HINSTANCE hInstance = AfxFindResourceHandle( MAKEINTRESOURCE( TOOLBAR_CONTROL ), RT_TOOLBAR );
	if(!hInstance)
		return FALSE;

	HRSRC hRsrc = ::FindResource( hInstance, MAKEINTRESOURCE( TOOLBAR_CONTROL ), RT_TOOLBAR );
	if( !hRsrc )
		return FALSE;

	HGLOBAL hGlobal = LoadResource( hInstance, hRsrc );
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = ( CToolBarData* ) LockResource( hGlobal );
	if (pData == NULL)
		return FALSE;

	ASSERT( pData->wVersion == 1 );

	TBBUTTON tb, tbSep;
	memset ( &tb, 0, sizeof( tb ) );
	memset ( &tbSep, 0, sizeof( tbSep ) );

	result = CToolBarCtrl::Create(WS_VISIBLE|WS_CHILD, rc, parent, TOOLBAR_CONTROL);

	if( result )
	{
		SetButtonStructSize( sizeof ( tb ) );

		CSize sz ( pData->wWidth, pData->wHeight );
		SetBitmapSize( sz );
		sz.cx += 4;
		sz.cy += 4;
		SetButtonSize( sz );

		// Loop through adding buttons.
		tb.fsState = TBSTATE_ENABLED;
		tb.fsStyle = TBSTYLE_BUTTON;
		tb.iString = -1;
		tb.iBitmap = 0;

		tbSep.iString = -1;
		tbSep.fsStyle = TBSTYLE_SEP;

		for( WORD w = 0; w < pData->wItemCount; w++ )
		{
			if ( pData->items()[ w ] == 0 )
				AddButtons( 1, &tbSep );
			else
			{
				tb.idCommand = pData->items()[ w ];
				AddButtons( 1, &tb );
				tb.iBitmap++;
			}
		}

		HBITMAP	hBitmap = (HBITMAP) ::LoadImage( hInstance, MAKEINTRESOURCE( TOOLBAR_CONTROL ), IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS );
		if( !hBitmap )
			return FALSE;

		BITMAP bm;
		memset( &bm, 0, sizeof ( bm ) );
		::GetObject( hBitmap, sizeof ( bm ), &bm );
		AddBitmap( bm.bmWidth / pData->wWidth, CBitmap::FromHandle ( hBitmap ) );

		UnlockResource( hGlobal );
		FreeResource( hGlobal );

		/////////////////////////////////////
		// Map in combo boxes
		//

		CRect rect;

		TBBUTTONINFO tbi;
		tbi.cbSize = sizeof( TBBUTTONINFO );
		tbi.cx = FONT_COMBO_WIDTH;
		tbi.dwMask = TBIF_SIZE | 0x80000000;  // By index

		SetButtonInfo( FONT_NAME_POS, &tbi );
		GetItemRect( FONT_NAME_POS, &rect );
		rect.bottom += COMBO_HEIGHT;

		// The font name combo
		if( m_font.Create( WS_CHILD | 
							WS_VSCROLL |
							WS_VISIBLE |
							CBS_AUTOHSCROLL | 
							CBS_DROPDOWN | 
							CBS_SORT | 
							CBS_HASSTRINGS, 
							rect, this, DROPDOWN_FONT ) )
		{

			m_font.SetFont( CFont::FromHandle( ( HFONT ) ::GetStockObject( ANSI_VAR_FONT ) ) );
			m_font.FillCombo();

			tbi.cx = COMBO_WIDTH;
			SetButtonInfo( FONT_SIZE_POS, &tbi );
			GetItemRect( FONT_SIZE_POS, &rect );
			rect.bottom += COMBO_HEIGHT;

			// The font size combo
			if( m_size.Create( WS_CHILD | 
								WS_VISIBLE | 
								CBS_AUTOHSCROLL | 
								CBS_DROPDOWNLIST | 
								CBS_HASSTRINGS, 
								rect, this, DROPDOWN_SIZE ) )
			{

				m_size.SetFont( CFont::FromHandle( ( HFONT ) ::GetStockObject( ANSI_VAR_FONT ) ) );
				m_size.FillCombo();
				CString color;
				CString defaultText;
				CString customText;
				color.LoadString( STRING_COLOR );
				defaultText.LoadString( STRING_DEFAULT );
				customText.LoadString( STRING_CUSTOM );

				tbi.cx = COLOR_WIDTH;
				SetButtonInfo( FONT_COLOR_POS, &tbi );
				GetItemRect( FONT_COLOR_POS, &rect );

				// The color picker
				if( m_color.Create( color,
									WS_VISIBLE|
									WS_CHILD,
									rect, this, BUTTON_COLOR ) )
				{

					m_color.SetDefaultText( defaultText );
					m_color.SetCustomText( customText );
					m_color.SetSelectionMode( CP_MODE_TEXT );
					m_color.SetBkColour( RGB( 255, 255, 255 ) );

					m_color.SetFont( CFont::FromHandle( ( HFONT ) ::GetStockObject( ANSI_VAR_FONT ) ) );
					result = TRUE;

				}

			}

		}

	}

	m_dpi.SetHwnd(m_hWnd);

	return result;

}

BEGIN_MESSAGE_MAP(CRRECToolbar, CToolBarCtrl)
	//{{AFX_MSG_MAP(CRRECToolbar)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_CBN_SELCHANGE(DROPDOWN_FONT, OnSelchangeFont)
	ON_CBN_SELCHANGE(DROPDOWN_SIZE, OnSelchangeSize)
	ON_MESSAGE(CPN_SELENDOK, OnColorButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRRECToolbar message handlers

void CRRECToolbar::OnSelchangeFont() 
/* ============================================================
	Function :		CRRECToolbar::OnSelchangeFont
	Description :	Changes the font of the selected text in 
					the editor.
	Access :		Protected
					
	Return :		void
	Parameters :	none

	Usage :			Called from MFC when the selection changes 
					in the font name combo.

   ============================================================*/
{

	CString font;
	int index = m_font.GetCurSel();
	if( index != CB_ERR )
	{
		m_font.GetLBText( index, font );
		GetParent()->SendMessage( urm_SETCURRENTFONTNAME, ( WPARAM ) ( LPCTSTR ) font, 0 );

	}	
}

void CRRECToolbar::OnSelchangeSize() 
/* ============================================================
	Function :		CRRECToolbar::OnSelchangeSize
	Description :	Changes the size of the selected text in 
					the editor.
	Access :		Protected
					
	Return :		void
	Parameters :	none

	Usage :			Called from MFC when the selection changes 
					in the font size combo.

   ============================================================*/
{
	int size = 0;
	int index = m_size.GetCurSel();
	if( index != CB_ERR )
	{

		CString sz;
		m_size.GetLBText( index, sz );
		size = _ttoi( ( LPCTSTR ) sz );

		GetParent()->SendMessage( urm_SETCURRENTFONTSIZE, 0, ( LPARAM ) size );

	}
	
}

LRESULT CRRECToolbar::OnColorButton( WPARAM w, LPARAM l) 
/* ============================================================
	Function :		CRRECToolbar::OnColorButton
	Description :	Mapped to the color picker defined 
					"CPN_SELENDOK" message, sent when the color 
					is changed in the picker.
	Access :		Protected
					
	Return :		LRESULT	-	Not used
	Parameters :	WPARAM	-	Not used
					LPARAM	-	Not used

	Usage :			Called from MFC.

   ============================================================*/
{
	GetParent()->SendMessage( urm_SETCURRENTFONTCOLOR, 0, ( LPARAM ) w );
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CRRECToolbar UI updaters

void CRRECToolbar::SetFontName( const CString& font )
/* ============================================================
	Function :		CRRECToolbar::SetFontName
	Description :	Selects the font name "font" in the font 
					name combo on the toolbar.
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			Call to set the selected font name

   ============================================================*/
{

	if( m_font.m_hWnd )
		m_font.SelectFontName( font );

}

void CRRECToolbar::SetFontSize( int size )
/* ============================================================
	Function :		CRRECToolbar::SetFontSize
	Description :	Selects the font size "size" in the font 
					size combo on the toolbar.
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			Call to set the selected font size

   ============================================================*/
{

	if( m_size.m_hWnd )
		m_size.SelectSize( size );

}

void CRRECToolbar::SetFontColor( COLORREF color )
/* ============================================================
	Function :		CRRECToolbar::SetFontColor
	Description :	Selects the font color "color" in the font 
					color picker on the toolbar.
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			Call to set the color picker color

   ============================================================*/
{

	if( m_color.m_hWnd )
		m_color.SetColour( color );

}

void CRRECToolbar::OnDpiChanged(CWnd* pParent, int dpi)
{
	m_dpi.Update(dpi);
}