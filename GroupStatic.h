#if !defined(AFX_GROUPSTATIC_H__C7039DB6_84EE_4622_8054_6DBA48FA21A9__INCLUDED_)
#define AFX_GROUPSTATIC_H__C7039DB6_84EE_4622_8054_6DBA48FA21A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GroupStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGroupStatic window

class CGroupStatic : public CStatic
{
// Construction
public:
	CGroupStatic();

// Attributes
public:

// Operations
public:

	void SetTextColor( COLORREF color )		{ m_dwTextColor = color;	}
	void SetBkColor( COLORREF color )		{ m_dwBkColor = color;		}
	void SetFont( int nPointSize, LPCTSTR lpszFaceName, CDC* pDC = NULL );
	void SetFont(CFont *pFont)				{ CStatic::SetFont(pFont);		}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupStatic)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGroupStatic();

protected:
	COLORREF m_dwTextColor;
	COLORREF m_dwBkColor;
	CFont*   m_pFont;
	CBrush   m_brush;
	CString  m_strBuff;        // Holds the static controls contents before & after the control is displayed


	// Generated message map functions
protected:
	//{{AFX_MSG(CGroupStatic)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPSTATIC_H__C7039DB6_84EE_4622_8054_6DBA48FA21A9__INCLUDED_)
