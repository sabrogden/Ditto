/* ==========================================================================
	File :			RuleRichEdit.cpp

	Class :			CRulerRichEdit

	Author :		Johan Rosengren, Abstrakt Mekanik AB
					Iain Clarke

	Date :			2004-04-17

	Purpose :		"CRulerRichEdit" is derived from "CWnd". 

	Description :	The class, in addition to the normal "CWnd", 
					handles horizontal scrollbar messages - forcing an 
					update of the parent (to synchronize the ruler). The 
					change notification is called for the same reason. 
					"WM_GETDLGCODE" is handled, we want all keys in a 
					dialog box instantiation.

	Usage :			This class is only useful as a child of the 
					"CRulerRichEditCtrl".

   ========================================================================*/

#include "stdafx.h"
#include "ids.h"
#include "RulerRichEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _UNICODE
	#define RTF_CLASS RICHEDIT_CLASSW
#else
	#define RTF_CLASS RICHEDIT_CLASSA
#endif

/////////////////////////////////////////////////////////////////////////////
// CRulerRichEdit

CRulerRichEdit::CRulerRichEdit()
/* ============================================================
	Function :		CRulerRichEdit::CRulerRichEdit
	Description :	constructor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

CRulerRichEdit::~CRulerRichEdit()
/* ============================================================
	Function :		CRulerRichEdit::~CRulerRichEdit
	Description :	destructor
	Access :		Public
					
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

BEGIN_MESSAGE_MAP(CRulerRichEdit, CRichEditCtrlEx)
	//{{AFX_MSG_MAP(CRulerRichEdit)
	ON_WM_HSCROLL()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CRulerRichEdit::Create( DWORD style, CRect rect, CWnd* parent )
/* ============================================================
	Function :		CRulerRichEdit::~CRulerRichEdit
	Description :	Creates the control
	Access :		Public
					
	Return :		BOOL			-	"TRUE" if created ok
	Parameters :	DWORD			-	Style for the control
					CRect rect		-	Position of the control
					CWnd* parent	-	Parent of the control

	Usage :			Call to create the control

   ============================================================*/
{

	return CWnd::Create (RTF_CLASS, NULL, style, rect, parent, RTF_CONTROL );

};

/////////////////////////////////////////////////////////////////////////////
// CRulerRichEdit message handlers

void CRulerRichEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
/* ============================================================
	Function :		CRulerRichEdit::OnHScroll
	Description :	Handles the "WM_HSCROLL" message.
	Access :		Protected
					
	Return :		void
	Parameters :	UINT nSBCode			-	Type of operation
					UINT nPos				-	New position
					CScrollBar* pScrollBar	-	Pointer to scrollbar
					
	Usage :			Called from MFC. Updates the ruler.

   ============================================================*/
{

	CWnd::OnHScroll( nSBCode, nPos, pScrollBar );

	SCROLLINFO	si;
	ZeroMemory( &si, sizeof( SCROLLINFO ) );
	si.cbSize = sizeof( SCROLLINFO );
	GetScrollInfo( SB_HORZ, &si );

	if ( nSBCode == SB_THUMBTRACK )
	{

		si.nPos = nPos;
		SetScrollInfo( SB_HORZ, &si );

	}

	UpdateRuler();

}

void CRulerRichEdit::OnChange() 
/* ============================================================
	Function :		CRulerRichEdit::OnChange
	Description :	Handles change-notifications.
	Access :		Protected
					
	Return :		void
	Parameters :	none

	Usage :			Called from MFC. Updates the ruler if text 
					is deleted, for example.

   ============================================================*/
{

	UpdateRuler();

}

UINT CRulerRichEdit::OnGetDlgCode() 
/* ============================================================
	Function :		CRulerRichEdit::OnGetDlgCode
	Description :	Handles the "WM_GETDLGCODE" message.
	Access :		Protected
					
	Return :		UINT	-	The keys we are interested in.
	Parameters :	none

	Usage :			Called from MFC. Handled to make sure 
					keypresses stays with us.

   ============================================================*/
{

	return DLGC_WANTALLKEYS;
	
}

/////////////////////////////////////////////////////////////////////////////
// CRulerRichEdit private helpers

void CRulerRichEdit::UpdateRuler()
/* ============================================================
	Function :		CRulerRichEdit::UpdateRuler
	Description :	Updates the ruler.
	Access :		Private
					
	Return :		void
	Parameters :	none

	Usage :			Call to update the parent ruler field.

   ============================================================*/
{

	CRect rect;
	GetClientRect( rect );
	rect.top = TOOLBAR_HEIGHT;
	rect.bottom = rect.top + TOP_HEIGHT;
	GetParent()->RedrawWindow( rect );

}

