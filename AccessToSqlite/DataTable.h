#if !defined(AFX_DATATABLE_H__B70F6A02_C464_4FFE_91E8_E7F59765178F__INCLUDED_)
#define AFX_DATATABLE_H__B70F6A02_C464_4FFE_91E8_E7F59765178F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataTable.h : header file
//

#pragma warning(disable : 4995)


class CDataTable : public CDaoRecordset
{
public:
	CDataTable(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CDataTable)

// Field/Param Data
	//{{AFX_FIELD(CDataTable, CDaoRecordset)
	long	m_lID;
	long	m_lDataID;
	CString	m_strClipBoardFormat;
	CLongBinary	m_ooData;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataTable)
	public:
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL
public:
	
	HGLOBAL TakeData();
	HGLOBAL LoadData(); // allocates a new copy of the data

	void Open(LPCTSTR lpszFormat,...);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATATABLE_H__B70F6A02_C464_4FFE_91E8_E7F59765178F__INCLUDED_)
