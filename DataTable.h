#if !defined(AFX_DATATABLE_H__B70F6A02_C464_4FFE_91E8_E7F59765178F__INCLUDED_)
#define AFX_DATATABLE_H__B70F6A02_C464_4FFE_91E8_E7F59765178F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataTable.h : header file
//

class CDataTable : public CDaoRecordset
{
public:
	CDataTable(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CDataTable)

// Field/Param Data
	//{{AFX_FIELD(CDataTable, CDaoRecordset)
	long	m_lID;
	long	m_lParentID;
	CString	m_strClipBoardFormat;
	CLongBinary	m_ooData;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataTable)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	virtual void Open(int nOpenType = AFX_DAO_USE_DEFAULT_TYPE, LPCTSTR lpszSql = NULL, int nOptions = 0);
	//}}AFX_VIRTUAL
public:
	BOOL SetData(HGLOBAL hgData);
	BOOL DeleteAll();
	BOOL LoadData(COleDataSource *pSource, UINT uiPastType);
	void Open(LPCTSTR lpszFormat,...);
	BOOL DataEqual(HGLOBAL hgData);


// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATATABLE_H__B70F6A02_C464_4FFE_91E8_E7F59765178F__INCLUDED_)
