#if !defined(AFX_NUMBEREDIT_H__C68189CB_8445_4009_8F29_34645AEBD9C5__INCLUDED_)
#define AFX_NUMBEREDIT_H__C68189CB_8445_4009_8F29_34645AEBD9C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumberEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumberEdit window

class CNumberEdit : public CEdit
{
// Construction
public:
	CNumberEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumberEdit)
	//}}AFX_VIRTUAL

public:
	double GetNumberD();
	long GetNumber();
	BOOL SetNumber(long lNumber);
	
	void SetMaxNumber(long lMax)		{ m_dMax = lMax;		}
	long GetMaxNumber()					{ return (long)m_dMax;	}
	
protected:
	BOOL ValidateNumber(double dNumber);

	double m_dMax;

// Implementation
public:
	virtual ~CNumberEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNumberEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMBEREDIT_H__C68189CB_8445_4009_8F29_34645AEBD9C5__INCLUDED_)
