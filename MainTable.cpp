// MainTable.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "MainTable.h"
#include "DatabaseUtilities.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainTable

IMPLEMENT_DYNAMIC(CMainTable, CDaoRecordset)

CMainTable::CMainTable(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CMainTable)
	m_lID = 0;
	m_lDate = 0;
	m_strType = _T("");
	m_strText = _T("");
	m_lShortCut = 0;
	m_lDontAutoDelete = 0;
	m_lTotalCopySize = 0;

	m_nFieldCount = m_nFields = 7;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
	m_bBindFields = true;
}

CString CMainTable::GetDefaultDBName()
{
	return GetDBName();
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
	DFX_Text(pFX, _T("[strType]"), m_strType);
	DFX_Text(pFX, _T("[strText]"), m_strText);
	DFX_Long(pFX, _T("[lShortCut]"), m_lShortCut);
	DFX_Long(pFX, _T("[lDontAutoDelete]"), m_lDontAutoDelete);
	DFX_Long(pFX, _T("[lTotalCopySize]"), m_lTotalCopySize);
	//}}AFX_FIELD_MAP
}

void CMainTable::Open(int nOpenType , LPCTSTR lpszSql , int nOptions)
{
	OnQuery();
	CDaoRecordset::Open(nOpenType, lpszSql, nOptions);
}

void CMainTable::Requery()
{
	OnQuery();
	CDaoRecordset::Requery();
}


/////////////////////////////////////////////////////////////////////////////
// CMainTable member functions

void CMainTable::OnQuery()
{}

bool CMainTable::SetBindFields(bool bVal)
{
bool bOld = m_bBindFields;

	m_bBindFields = bVal;

	if(m_bBindFields)
		m_nFields = m_nFieldCount;
	else
		m_nFields = 0;

	return bOld;
}

// only deletes from Main
BOOL CMainTable::DeleteAll()
{
	BOOL bRet = FALSE;
	try
	{
		theApp.EnsureOpenDB();
		theApp.m_pDatabase->Execute("DELETE * FROM Main", dbFailOnError);
		bRet = TRUE;
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
	}

	return bRet;
}

void CMainTable::LoadAcceleratorKeys( CAccels& accels )
{
	CMainTable recset;

	try
	{
		recset.Open("SELECT * FROM Main WHERE lShortCut > 0");
		
		CAccel a;
		while(!recset.IsEOF())
		{
			a.Cmd = recset.m_lID;
			a.Key = recset.m_lShortCut;
			accels.AddAccel( a );

			recset.MoveNext();
		}
	}
	catch(CDaoException* e)
	{
		e->Delete();
	}
}

/*
//HACCEL CMainTable::LoadAcceleratorKeys()
//{
//	CMainTable recset;
//
//	try
//	{
//		recset.Open("SELECT * FROM Main WHERE lShortCut > 0");
//		
//		CArray<ACCEL, ACCEL> keys;
//
//		while(!recset.IsEOF())
//		{
//			ACCEL me;
//			me.cmd = (USHORT)recset.m_lID;
//			me.fVirt = 0;
//			if( HIBYTE(recset.m_lShortCut) & HOTKEYF_SHIFT )   me.fVirt |= FSHIFT;
//			if( HIBYTE(recset.m_lShortCut) & HOTKEYF_CONTROL ) me.fVirt |= FCONTROL;
//			if( HIBYTE(recset.m_lShortCut) & HOTKEYF_ALT )     me.fVirt |= FALT;	
//			me.fVirt |= FVIRTKEY;
//			me.key = LOBYTE(recset.m_lShortCut);
//
//			keys.Add(me);
//
//			recset.MoveNext();
//		}
//
//		if(keys.GetSize() > 0)
//			return CreateAcceleratorTable(keys.GetData(), keys.GetSize());
//	}
//	catch(CDaoException* e)
//	{
//		e->Delete();
//	}
//
//	return NULL;
//}
*/

void CMainTable::Open(LPCTSTR lpszFormat,...) 
{
	m_pDatabase = theApp.EnsureOpenDB();

	CString csText;
	va_list vlist;

	ASSERT(AfxIsValidString(lpszFormat));
	va_start(vlist,lpszFormat);
	csText.FormatV(lpszFormat,vlist);
	va_end(vlist);
	
	Open(AFX_DAO_USE_DEFAULT_TYPE, csText, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CMainTable diagnostics

#ifdef _DEBUG
void CMainTable::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CMainTable::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG
