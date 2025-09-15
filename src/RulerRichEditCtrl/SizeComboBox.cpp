/* ==========================================================================
	File :			SizeComboBox.cpp

	Class :			CSizeComboBox

	Author :		Johan Rosengren, Abstrakt Mekanik AB
					Iain Clarke

	Date :			2005-05-06

	Purpose :		"CSizeComboBox" is derived from "CComboBox" and is a simple 
					combobox for displaying a selection of font sizes.

	Description :	Simpel derived class with members to fill the box and 
					select an entry by contents instead of index.

	Usage :			Create as any combobox, and call "FillCombo" to fill 
					the control with a selection of font sizes. Call 
					"SelectFontSize" to select an entry by content.

   ========================================================================*/
#include "stdafx.h"
#include "SizeComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSizeComboBox

CSizeComboBox::CSizeComboBox()
/* ============================================================
	Function :		CSizeComboBox::CSizeComboBox
	Description :	ctor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

CSizeComboBox::~CSizeComboBox()
/* ============================================================
	Function :		CSizeComboBox::~CSizeComboBox
	Description :	dtor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}


BEGIN_MESSAGE_MAP(CSizeComboBox, CComboBox)
	//{{AFX_MSG_MAP(CSizeComboBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSizeComboBox message handlers

void CSizeComboBox::SelectSize( int size )
/* ============================================================
	Function :		CSizeComboBox::SelectSize
	Description :	Selects the entry corresponding to "size", 
					if it exists in the list.
	Access :		Public
					
	Return :		void
	Parameters :	int size	-	Size to select

	Usage :			Call to select an entry by value in the 
					combo list.

   ============================================================*/
{
	CString sz;
	sz.Format( _T( "%d" ), size );

	int max = GetCount();
	for( int t = 0 ; t < max ; t++ )
	{
		CString data;
		GetLBText( t, data );
		if( data == sz )
		{
			SetCurSel( t );
			return;
		}
	}
}

void CSizeComboBox::FillCombo()
/* ============================================================
	Function :		CSizeComboBox::FillCombo
	Description :	Fills the combo box with a fairly standard 
					selection of font sizes.
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			Call to fill the combobox.

   ============================================================*/
{

	AddString( _T( "8" ) );
	AddString( _T( "9" ) );
	AddString( _T( "10" ) );
	AddString( _T( "11" ) );
	AddString( _T( "12" ) );
	AddString( _T( "14" ) );
	AddString( _T( "16" ) );
	AddString( _T( "18" ) );
	AddString( _T( "20" ) );
	AddString( _T( "22" ) );
	AddString( _T( "24" ) );
	AddString( _T( "26" ) );
	AddString( _T( "28" ) );
	AddString( _T( "36" ) );
	AddString( _T( "48" ) );
	AddString( _T( "72" ) );

}
