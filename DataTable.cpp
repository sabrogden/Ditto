// DataTable.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "DataTable.h"
#include "DatabaseUtilities.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataTable

IMPLEMENT_DYNAMIC(CDataTable, CDaoRecordset)

CDataTable::CDataTable(CDaoDatabase* pdb)
:CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CDataTable)
	m_lID = 0;
	m_lParentID = 0;
	m_strClipBoardFormat = _T("");
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
}


CString CDataTable::GetDefaultDBName()
{
	return GetDBName();
}

CString CDataTable::GetDefaultSQL()
{
	return _T("[Data]");
}

void CDataTable::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CDataTable)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Long(pFX, _T("[lID]"), m_lID);
	DFX_Long(pFX, _T("[lParentID]"), m_lParentID);
	DFX_Text(pFX, _T("[strClipBoardFormat]"), m_strClipBoardFormat);
	DFX_LongBinary(pFX, _T("[ooData]"), m_ooData);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDataTable diagnostics

#ifdef _DEBUG
void CDataTable::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CDataTable::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG

BOOL CDataTable::DeleteAll()
{
	BOOL bRet = FALSE;
	try
	{
		theApp.EnsureOpenDB();
		theApp.m_pDatabase->Execute("DELETE * FROM Data", dbFailOnError);
		bRet = TRUE;
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
	}

	return bRet;
}

BOOL CDataTable::SetData(HGLOBAL hgData)
{
	//Get the data from the clipboard sent in
	LPVOID pvData = NULL;
	pvData = GlobalLock(hgData);
	if(!pvData)
		return FALSE;

	//Size
	UINT unSize = GlobalSize(hgData);

	//Realocate m_ooData.m_hData
	if(m_ooData.m_hData)
		m_ooData.m_hData = GlobalReAlloc(m_ooData.m_hData, unSize, GMEM_MOVEABLE);
	else
		m_ooData.m_hData = GlobalAlloc(GHND, unSize);

	//Get the data associated 
	LPVOID pvNewData = NULL;
	pvNewData = GlobalLock(m_ooData.m_hData);
	if(!pvNewData)
		return FALSE;

	//Set the size
	m_ooData.m_dwDataLength = unSize;

	//Set the size
	memcpy(pvNewData, pvData, unSize);

	//Set the fields dirty
	SetFieldDirty(&m_ooData);
	SetFieldNull(&m_ooData,FALSE);

	return TRUE;
}

BOOL CDataTable::LoadData(COleDataSource *pData, UINT uiPastType)
{
	BOOL fRetVal = FALSE;
	
	//Retrieve size of array
	ULONG ulBufLen = m_ooData.m_dwDataLength;

	if(ulBufLen > 0)
	{
		HGLOBAL hGlobal;

		hGlobal = GlobalAlloc(GMEM_ZEROINIT, ulBufLen);
		if(hGlobal != NULL)
		{
			LPVOID pvData = NULL;
			pvData = GlobalLock(hGlobal);
			if(!pvData)
				return FALSE;

			LPVOID pvData2 = NULL;
			pvData2 = GlobalLock(m_ooData.m_hData);
			if(!pvData2)
				return FALSE;

			memcpy(pvData, pvData2, ulBufLen);
			
			GlobalUnlock(hGlobal);
			GlobalUnlock(m_ooData.m_hData);

			pData->CacheGlobalData(uiPastType, hGlobal);

			fRetVal = TRUE;
		}
		else
			ASSERT(FALSE);
	}
	else
		ASSERT(FALSE);
	
	return fRetVal;

}

void CDataTable::Open(LPCTSTR lpszFormat,...) 
{
	m_pDatabase = theApp.EnsureOpenDB();

	CString csText;
	va_list vlist;

	ASSERT(AfxIsValidString(lpszFormat));
	va_start(vlist,lpszFormat);
	csText.FormatV(lpszFormat,vlist);
	va_end(vlist);
	
	CDaoRecordset::Open(AFX_DAO_USE_DEFAULT_TYPE, csText, 0);
}

void CDataTable::Open(int nOpenType, LPCTSTR lpszSql, int nOptions) 
{
	m_pDatabase = theApp.EnsureOpenDB();
	
	CDaoRecordset::Open(nOpenType, lpszSql, nOptions);
}

BOOL CDataTable::DataEqual(HGLOBAL hgData)
{
	LPVOID saved = NULL;
	saved = GlobalLock(m_ooData.m_hData);
	if(!saved)
		return FALSE;

	LPVOID data = NULL;
	data = GlobalLock(hgData);
	if(!data)
		return FALSE;

	GlobalUnlock(hgData);
	GlobalUnlock(m_ooData.m_hData);

	int nRet = (memcmp(data, saved, m_ooData.m_dwDataLength) == 0);

	return nRet;
}