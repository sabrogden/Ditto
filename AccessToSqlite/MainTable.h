#if !defined(AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_)
#define AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4995)


class CMainTable : public CDaoRecordset
{
public:
	CMainTable(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CMainTable)


// Field/Param Data
	//{{AFX_FIELD(CMainTable, CDaoRecordset)
	long	m_lID;
	long	m_lDate;
	CString	m_strText;
	long	m_lShortCut;
	long	m_lDontAutoDelete;
	long	m_lTotalCopySize;

	BOOL	m_bIsGroup;
	long	m_lParentID;
	double	m_dOrder;
	long	m_lDataID;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainTable)
	public:
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

public:
	CString GetDisplayText( int nMaxLines );

	int		m_nFieldCount;
	bool	m_bBindFields;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_)
