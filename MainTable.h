#if !defined(AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_)
#define AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ArrayEx.h"

class CMainTable : public CDaoRecordset
{
public:
	CMainTable(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CMainTable)


// Field/Param Data
	//{{AFX_FIELD(CMainTable, CDaoRecordset)
	long	m_lID;
	long	m_lDate;
	CString	m_strType;
	CString	m_strText;
	long	m_lShortCut;
	long	m_lDontAutoDelete;
	long	m_lTotalCopySize;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainTable)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

public:
	BOOL DeleteRows(ARRAY &IDs);
	BOOL DeleteAll();
	static HACCEL LoadAcceleratorKeys();
	void Open(LPCTSTR lpszFormat,...);


// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_)
