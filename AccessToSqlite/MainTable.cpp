// MainTable.cpp : implementation file
//

#include "stdafx.h"
#include "AccessToSqlite.h"
#include "MainTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4995)


/////////////////////////////////////////////////////////////////////////////
// CMainTable

IMPLEMENT_DYNAMIC(CMainTable, CDaoRecordset)

CMainTable::CMainTable(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMainTable)
	m_lID = 0;
	m_lDate = 0;
	m_strText = _T("");
	m_lShortCut = 0;
	m_lDontAutoDelete = 0;
	m_lTotalCopySize = 0;

	m_bIsGroup = FALSE;
	m_lParentID = 0;
	m_dOrder = 0;
	m_lDataID = 0;

	m_nFieldCount = m_nFields = 10;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
	m_bBindFields = true;
}

CString CMainTable::GetDefaultSQL()
{
	return _T("[Main]");
}

void CMainTable::DoFieldExchange(CDaoFieldExchange* pFX)
{
	// make sure this isn't called when we aren't using bound fields
	VERIFY( m_bBindFields == true );

	//{{AFX_FIELD_MAP(CMainTable)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Long(pFX, _T("[lID]"), m_lID);
	DFX_Long(pFX, _T("[lDate]"), m_lDate);
	DFX_Text(pFX, _T("[mText]"), m_strText);
	DFX_Long(pFX, _T("[lShortCut]"), m_lShortCut);
	DFX_Long(pFX, _T("[lDontAutoDelete]"), m_lDontAutoDelete);
	DFX_Long(pFX, _T("[lTotalCopySize]"), m_lTotalCopySize);
	// GROUPS
	DFX_Bool(pFX, _T("[bIsGroup]"), m_bIsGroup);
	DFX_Long(pFX, _T("[lParentID]"), m_lParentID);
	DFX_Double(pFX, _T("[dOrder]"), m_dOrder);
	// sharing data
	DFX_Long(pFX, _T("[lDataID]"), m_lDataID);
	//}}AFX_FIELD_MAP
}


