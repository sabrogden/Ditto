#if !defined(AFX_RRECTOOLBAR_H__E0596D7D_418C_49B9_AC57_2F0BF93053C9__INCLUDED_)
#define AFX_RRECTOOLBAR_H__E0596D7D_418C_49B9_AC57_2F0BF93053C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RRECToolbar.h : header file
//

#include "FontComboBox.h"
#include "SizeComboBox.h"
#include "External/ColourPicker.h"

struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;

	WORD* items()
		{ return ( WORD* )( this + 1 ); }

};

/////////////////////////////////////////////////////////////////////////////
// CRRECToolbar window

class CRRECToolbar : public CToolBarCtrl
{
// Construction
public:
	CRRECToolbar();
	BOOL Create( CWnd* parent, CRect& rect );

// Attributes
public:

// Operations
public:

	void SetFontName( const CString& font );
	void SetFontSize( int size );
	void SetFontColor( COLORREF color );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRRECToolbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRRECToolbar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRRECToolbar)
		// NOTE - the ClassWizard will add and remove member functions here.
	afx_msg void OnSelchangeFont();
	afx_msg void OnSelchangeSize();
	afx_msg LRESULT OnColorButton( WPARAM w, LPARAM l);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	CFontComboBox	m_font;
	CSizeComboBox	m_size;
	CColourPicker	m_color;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RRECTOOLBAR_H__E0596D7D_418C_49B9_AC57_2F0BF93053C9__INCLUDED_)
