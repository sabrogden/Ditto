#if !defined(AFX_TYPESTABLE_H__41C712E1_75EC_4385_ABA3_9749852F63FD__INCLUDED_)
#define AFX_TYPESTABLE_H__41C712E1_75EC_4385_ABA3_9749852F63FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TypesTable.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTypesTable DAO recordset

class CTypesTable : public CDaoRecordset
{
public:
	CTypesTable(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTypesTable)

// Field/Param Data
	//{{AFX_FIELD(CTypesTable, CDaoRecordset)
	long	m_ID;
	CString	m_TypeText;
	//}}AFX_FIELD
	
public:
	BOOL DeleteAll();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTypesTable)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	virtual void Open(int nOpenType = AFX_DAO_USE_DEFAULT_TYPE, LPCTSTR lpszSql = NULL, int nOptions = 0);
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TYPESTABLE_H__41C712E1_75EC_4385_ABA3_9749852F63FD__INCLUDED_)
