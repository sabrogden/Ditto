// TypesTable.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "TypesTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTypesTable

IMPLEMENT_DYNAMIC(CTypesTable, CDaoRecordset)

CTypesTable::CTypesTable(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTypesTable)
	m_ID = 0;
	m_TypeText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
}


CString CTypesTable::GetDefaultDBName()
{
	return _T("C:\\Projects\\Ditto\\CP_Shared\\Release_DLL\\97.mdb");
}

CString CTypesTable::GetDefaultSQL()
{
	return _T("[Types]");
}

void CTypesTable::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTypesTable)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Long(pFX, _T("[ID]"), m_ID);
	DFX_Text(pFX, _T("[TypeText]"), m_TypeText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTypesTable diagnostics

#ifdef _DEBUG
void CTypesTable::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CTypesTable::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG


BOOL CTypesTable::DeleteAll()
{
	BOOL bRet = FALSE;
	try
	{
		theApp.EnsureOpenDB();
		theApp.m_pDatabase->Execute("DELETE * FROM Types", dbFailOnError);
		bRet = TRUE;
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
	}

	return bRet;
}

void CTypesTable::Open(int nOpenType, LPCTSTR lpszSql, int nOptions) 
{
	m_pDatabase = theApp.EnsureOpenDB();
	
	CDaoRecordset::Open(nOpenType, lpszSql, nOptions);
}