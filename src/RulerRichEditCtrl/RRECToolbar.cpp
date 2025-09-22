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
#include "..\..\resource.h"

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

BOOL CRRECToolbar::Create( CWnd* parent, CRect& rc, int resourceId)
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

	HINSTANCE hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(resourceId), RT_TOOLBAR);
	if(!hInstance)
		return FALSE;

	HRSRC hRsrc = ::FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_TOOLBAR);
	if(!hRsrc)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInstance, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*) LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;

	ASSERT(pData->wVersion == 1);

	TBBUTTON tb, tbSep;
	memset(&tb, 0, sizeof(tb));
	memset(&tbSep, 0, sizeof(tbSep));

	result = CToolBarCtrl::Create(WS_VISIBLE|WS_CHILD, rc, parent, resourceId);

	if(result)
	{
		m_dpi.Invalidate();
		m_dpi.SetHwnd(m_hWnd);

		m_font.DeleteObject();
		m_font.CreateFont(-m_dpi.Scale(12), 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, 3, 2, 1, 34, _T("Segoe UI"));

		SetButtonStructSize(sizeof(tb));

		CSize sz(pData->wWidth, pData->wHeight);
		SetBitmapSize(sz);
		sz.cx += 4;
		sz.cy += 4;
		SetButtonSize(sz);

		// Loop through adding buttons.
		tb.fsState = TBSTATE_ENABLED;
		tb.fsStyle = TBSTYLE_BUTTON;
		tb.iString = -1;
		tb.iBitmap = 0;

		tbSep.iString = -1;
		tbSep.fsStyle = TBSTYLE_SEP;

		for(WORD w = 0; w < pData->wItemCount; w++)
		{
			if (pData->items()[w] == 0)
			{
				AddButtons(1, &tbSep);
			}
			else
			{
				tb.idCommand = pData->items()[w];
				AddButtons(1, &tb);
				tb.iBitmap++;
			}
		}

		HBITMAP	hBitmap = (HBITMAP)::LoadImage( hInstance, MAKEINTRESOURCE(resourceId), IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS );
		if( !hBitmap )
			return FALSE;

		BITMAP bm;
		memset(&bm, 0, sizeof (bm));
		::GetObject(hBitmap, sizeof (bm), &bm);
		AddBitmap(bm.bmWidth / pData->wWidth, CBitmap::FromHandle (hBitmap));

		UnlockResource(hGlobal);
		FreeResource(hGlobal);

		CRect rect;

		TBBUTTONINFO tbi;
		tbi.cbSize = sizeof( TBBUTTONINFO );
		tbi.cx = m_dpi.Scale(100);
		tbi.dwMask = TBIF_SIZE | 0x80000000;  // By index

		SetButtonInfo(2, &tbi);
		GetItemRect(2, &rect);

		// The font name combo
		if(m_fontCombo.Create(WS_CHILD | WS_VSCROLL | WS_VISIBLE | CBS_AUTOHSCROLL | CBS_DROPDOWN | CBS_SORT | CBS_HASSTRINGS, rect, this, 31))
		{
			m_fontCombo.SetFont(&m_font);
			m_fontCombo.FillCombo();
			
			tbi.cx = m_dpi.Scale(48);
			SetButtonInfo(4, &tbi);
			GetItemRect(4, &rect);

			//The font size combo
			if( m_size.Create( WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS, rect, this, 32))
			{
				m_size.SetFont(&m_font);
				m_size.FillCombo();

				CString color = _T("Color");
				CString defaultText = _T("Automatic");
				CString customText = _T("More Colours...");

				tbi.cx = m_dpi.Scale(64);
				SetButtonInfo(6, &tbi);
				GetItemRect(6, &rect);

				// The color picker
				if(m_color.Create(color, WS_VISIBLE | WS_CHILD, rect, this, BUTTON_COLOR))
				{
					m_color.SetDefaultText(defaultText);
					m_color.SetCustomText(customText);
					m_color.SetSelectionMode(CP_MODE_TEXT);
					m_color.SetBkColour(RGB(255, 255, 255 ));
					m_color.SetFont(&m_font);

					result = TRUE;
				}
			}
		}
	}

	return result;

}

BEGIN_MESSAGE_MAP(CRRECToolbar, CToolBarCtrl)
	//{{AFX_MSG_MAP(CRRECToolbar)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_CBN_SELCHANGE(31, OnSelchangeFont)
	ON_CBN_SELCHANGE(32, OnSelchangeSize)
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
	int index = m_fontCombo.GetCurSel();
	if( index != CB_ERR )
	{
		m_fontCombo.GetLBText( index, font );
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

	if(m_fontCombo.m_hWnd )
		m_fontCombo.SelectFontName( font );

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