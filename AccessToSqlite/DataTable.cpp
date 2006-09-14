// DataTable.cpp : implementation file
//

#include "stdafx.h"
#include "AccessToSqlite.h"
#include "DataTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4995)


/////////////////////////////////////////////////////////////////////////////
// CDataTable

IMPLEMENT_DYNAMIC(CDataTable, CDaoRecordset)

CDataTable::CDataTable(CDaoDatabase* pdb)
:CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CDataTable)
	m_lID = 0;
	m_lDataID = 0;
	m_strClipBoardFormat = _T("");
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
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
	DFX_Long(pFX, _T("[lDataID]"), m_lDataID);
	DFX_Text(pFX, _T("[strClipBoardFormat]"), m_strClipBoardFormat);
	DFX_LongBinary(pFX, _T("[ooData]"), m_ooData);
	//}}AFX_FIELD_MAP
}


// caller must free
// takes m_ooData's HGLOBAL (do not update recset after calling this)
// This should be faster than making a copy, but is this SAFE ??
HGLOBAL CDataTable::TakeData()
{
	// if there is nothing to take
	if( m_ooData.m_hData == 0 || m_ooData.m_dwDataLength == 0 )
		return 0;
	
	// Unlock the handle that was locked by DaoLongBinaryAllocCallback()
	//  (through DFX_LongBinary()).
	::GlobalUnlock( m_ooData.m_hData );
	
	// we have to do a realloc in order to make the hGlobal m_dwDataLength
	HGLOBAL hGlobal = ::GlobalReAlloc(m_ooData.m_hData, m_ooData.m_dwDataLength, GMEM_MOVEABLE );
	if( !hGlobal || ::GlobalSize(hGlobal) == 0 )
	{
		//::_RPT0( _CRT_WARN, GetErrorString(::GetLastError()) );
		ASSERT(FALSE);
	}
	// no longer valid
	m_ooData.m_hData = 0;
	m_ooData.m_dwDataLength = 0;
	return hGlobal;
}

// allocates a new copy of the data
HGLOBAL CDataTable::LoadData()
{
	HGLOBAL hGlobal;
	ULONG ulBufLen = m_ooData.m_dwDataLength; //Retrieve size of array
	
	if( ulBufLen == 0 || m_ooData.m_hData == 0 )
		return 0;
	
	hGlobal = NewGlobalH( m_ooData.m_hData, ulBufLen );
	
	return hGlobal;
}

void CDataTable::Open(LPCTSTR lpszFormat,...) 
{
	CString csText;
	va_list vlist;
	
	ASSERT(AfxIsValidString(lpszFormat));
	va_start(vlist,lpszFormat);
	csText.FormatV(lpszFormat,vlist);
	va_end(vlist);
	
	CDaoRecordset::Open(AFX_DAO_USE_DEFAULT_TYPE, csText, 0);
}