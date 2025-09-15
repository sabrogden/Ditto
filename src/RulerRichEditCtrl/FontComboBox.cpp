/* ==========================================================================
	File :			FontComboBox.cpp

	Class :			CFontComboBox

	Author :		Johan Rosengren, Abstrakt Mekanik AB
					Iain Clarke

	Date :			2005-05-06

	Purpose :		CFontComboBox is derived from "CComboBox" and is an 
					autosizing no-frills combobox for display of the fonts 
					installed in the system.. 

	Description :	Simpel derived class with members to fill the box, to 
					autosize the dropdown and select an entry by name.

	Usage :			Create as any combobox, and call "FillCombo" to fill 
					the control with the names of the fonts installed in 
					the system. Call "SelectFontName" to select a font by 
					name.

   ========================================================================*/

#include "stdafx.h"
#include "FontComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Enumeration callback function for 
// installed fonts
BOOL CALLBACK EnumFontProc( LPLOGFONT lplf, LPTEXTMETRIC /*lptm*/, DWORD /*dwType*/, LPARAM lpData )	
{	
	CFontComboBox *caller = reinterpret_cast< CFontComboBox* > ( lpData );		
	caller->AddString( lplf->lfFaceName );

	CClientDC dc( caller );
	dc.SelectStockObject( ANSI_VAR_FONT );

	// Add a "0" for the margin.
	CSize sz = dc.GetTextExtent( CString( lplf->lfFaceName ) + CString( "0" ) );

	caller->SetMaxWidth( max( caller->GetMaxWidth(), sz.cx ) );

	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////
// CFontComboBox

CFontComboBox::CFontComboBox()
/* ============================================================
	Function :		CFontComboBox::CFontComboBox
	Description :	ctor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
	m_maxWidth = 0;
}

CFontComboBox::~CFontComboBox()
/* ============================================================
	Function :		CFontComboBox::~CFontComboBox
	Description :	dtor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

BEGIN_MESSAGE_MAP(CFontComboBox, CComboBox)
	//{{AFX_MSG_MAP(CFontComboBox)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontComboBox message handlers

void CFontComboBox::OnDropdown() 
/* ============================================================
	Function :		CFontComboBox::OnDropdown
	Description :	Sets the dropped down width of the combo 
					control to the max width of the string in 
					the list
	Access :		Protected
					
	Return :		void
	Parameters :	none

	Usage :			Called from MFC

   ============================================================*/
{

   int scroll = ::GetSystemMetrics(SM_CXVSCROLL);
   SetDroppedWidth( m_maxWidth + scroll );

}

/////////////////////////////////////////////////////////////////////////////
// CFontComboBox implementation

void CFontComboBox::FillCombo()
/* ============================================================
	Function :		CFontComboBox::FillCombo
	Description :	Fills the combo with the font names from 
					the system
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			Call to fill the combo with the installed 
					fonts.

   ============================================================*/
{

	CClientDC dc( this );		
	::EnumFonts( dc, 0, ( FONTENUMPROC ) EnumFontProc, ( LPARAM ) this );

}

void CFontComboBox::SetMaxWidth( int maxWidth )
/* ============================================================
	Function :		CFontComboBox::SetMaxWidth
	Description :	Sets the "m_maxWidth" member.
	Access :		Public
					
	Return :		void
	Parameters :	int maxWidth	-	New width of the drop
										down list

	Usage :			Call to set a max width for the strings in 
					the list, used to set the width of the drop 
					down list. Called from the font enum 
					callback.

   ============================================================*/
{

	m_maxWidth = maxWidth;

}

int CFontComboBox::GetMaxWidth() const
/* ============================================================
	Function :		CFontComboBox::GetMaxWidth
	Description :	Returns the "m_maxWidth" member of the 
					control.
	Access :		Public
					
	Return :		int		-	Width of the longest string, in 
								pixels.
	Parameters :	none

	Usage :			Call to get the width necessary to display 
					the longest string in the combo drop down.

   ============================================================*/
{

	return m_maxWidth;

}

void CFontComboBox::SelectFontName( const CString& font )
/* ============================================================
	Function :		CFontComboBox::SelectFontName
	Description :	Selects "font" in the list, if it 
					exists.
	Access :		Public
					
	Return :		void
	Parameters :	const CString& font	-	Name of the font to 
											select.

	Usage :			Call to set/change the selection of the 
					combo.

   ============================================================*/
{

	int max = GetCount();
	for( int t = 0 ; t < max ; t++ )
	{
		CString data;
		GetLBText( t, data );
		if( data == font )
		{
			SetCurSel( t );
			return;
		}
	}

}

