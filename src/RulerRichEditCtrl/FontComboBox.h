#if !defined(AFX_FONTCOMBOBOX_H__B88A8EAC_D643_444C_B9B0_87CE8DC81E89__INCLUDED_)
#define AFX_FONTCOMBOBOX_H__B88A8EAC_D643_444C_B9B0_87CE8DC81E89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FontComboBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFontComboBox window

class CFontComboBox : public CComboBox
{
// Construction
public:
	CFontComboBox();

// Attributes
public:

// Operations
public:
	void FillCombo();
	void SetMaxWidth( int maxWidth );
	int GetMaxWidth() const;
	void SelectFontName( const CString& font );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontComboBox)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFontComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFontComboBox)
	afx_msg void OnDropdown();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	int m_maxWidth;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTCOMBOBOX_H__B88A8EAC_D643_444C_B9B0_87CE8DC81E89__INCLUDED_)
