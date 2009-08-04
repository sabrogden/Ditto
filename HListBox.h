#if !defined(AFX_HLISTBOX_H__346C3917_14BC_11D5_A025_006067718D04__INCLUDED_)
#define AFX_HLISTBOX_H__346C3917_14BC_11D5_A025_006067718D04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHListBox window

class CHListBox : public CListBox
{
// Construction
public:
	CHListBox();

// Attributes
public:

// Operations
public:
    int AddString(LPCTSTR s);
    int InsertString(int i, LPCTSTR s);
    void ResetContent();
    int DeleteString(int i);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHListBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHListBox();

	// Generated message map functions
protected:
        void updateWidth(LPCTSTR s);
	int width;
	//{{AFX_MSG(CHListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HLISTBOX_H__346C3917_14BC_11D5_A025_006067718D04__INCLUDED_)
