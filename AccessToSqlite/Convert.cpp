#include "StdAfx.h"
#include "Convert.h"
#include "AccessToSqlite.h"
#include "DataTable.h"
#include "TypesTable.h"

CConvert::CConvert(void)
{
}

CConvert::~CConvert(void)
{
}

bool CConvert::ConvertNonGroups()
{
	try
	{			
		CMainTable recset;
		recset.m_pDatabase = &theApp.m_AccessDatabase;
		recset.m_strFilter.Format(_T("bIsGroup = FALSE"));
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, _T(""), 0);

		if(recset.IsEOF() == FALSE)
		{		
			while(!recset.IsEOF())
			{
				ConvertEntry(recset);

				recset.MoveNext();

				m_Progress.StepIt();
				m_Progress.PeekAndPump(FALSE);
			}
		}
	}		
	catch(CDaoException* e)
	{
		TCHAR cError[100];
		e->GetErrorMessage(cError, 100);
		Log(StrF(_T("CDaoException - %s"), cError));

		ASSERT(FALSE);
		e->Delete();
		return false;
	}	


	return true;
}

bool CConvert::ConvertGroups(long lParentID)
{	
	try
	{			
		CMainTable recset;
		recset.m_pDatabase = &theApp.m_AccessDatabase;
		recset.m_strFilter.Format(_T("bIsGroup = TRUE AND lParentID = %d"), lParentID);
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, _T(""), 0);

		while(!recset.IsEOF())
		{
			ConvertEntry(recset);
			
			ConvertGroups(recset.m_lID);

			recset.MoveNext();

			m_Progress.StepIt();
			m_Progress.PeekAndPump(FALSE);
		}
	}		
	catch(CDaoException* e)
	{
		TCHAR cError[100];
		e->GetErrorMessage(cError, 100);
		Log(StrF(_T("CDaoException - %s"), cError));

		ASSERT(FALSE);
		e->Delete();
		return false;
	}	

	return true;
}

bool CConvert::ConvertEntry(CMainTable &recset)
{
	bool bRet = false;

	try
	{
		CClip Clip;

		Clip.m_Desc = recset.m_strText;
		Clip.m_Time = recset.m_lDate;
		Clip.m_lShortCut = recset.m_lShortCut;
		Clip.m_lDontAutoDelete = recset.m_lDontAutoDelete;
		Clip.m_bIsGroup = recset.m_bIsGroup;

		long lParent;
		if(m_ParentMap.Lookup(recset.m_lParentID, lParent))
		{
			Clip.m_lParent = lParent;
		}

		if(Clip.m_bIsGroup || LoadFormats(recset.m_lID, Clip.m_Formats))
		{
			Clip.AddToDB(false);

			if(Clip.m_bIsGroup)
			{
				m_ParentMap.SetAt(recset.m_lID, Clip.m_ID);
			}

			bRet = true;
		}
		else
		{
			Log(StrF(_T("Error loading formats id = %d"), recset.m_lID));
		}
	}
	catch (CppSQLite3Exception& e)
	{
		Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));

		ASSERT(FALSE);
		bRet = false;
	}

	return bRet;
}

bool CConvert::LoadFormats(long lID, CClipFormats& formats)
{
	CClipFormat cf;
	HGLOBAL hGlobal = 0;

	formats.RemoveAll();

	try
	{
		CDataTable recset;
		recset.m_pDatabase = &theApp.m_AccessDatabase;

		//Open the data table for all that have the parent id
		CString csSQL;
		csSQL.Format(
			_T("SELECT Data.* FROM Data ")
			_T("INNER JOIN Main ON Main.lDataID = Data.lDataID ")
			_T("WHERE Main.lID = %d"), lID);

		recset.Open(csSQL);

		while( !recset.IsEOF() )
		{
			cf.m_cfType = GetFormatID(recset.m_strClipBoardFormat);

			// create a new HGLOBAL duplicate
			hGlobal = NewGlobalH(recset.m_ooData.m_hData, recset.m_ooData.m_dwDataLength);
			if(hGlobal)
			{
				cf.m_hgData = hGlobal;
				formats.Add( cf );
			}

			recset.MoveNext();
		}
		cf.m_hgData = 0; // formats owns all the data

		recset.Close();
	}
	catch(CDaoException* e)
	{
		TCHAR cError[100];
		e->GetErrorMessage(cError, 100);
		Log(StrF(_T("CDaoException - %s"), cError));

		ASSERT(FALSE);
		e->Delete();
		return false;
	}

	return formats.GetSize() > 0;
}

bool CConvert::ConvertTypes()
{
	try
	{
		CTypesTable recset;
		recset.m_pDatabase = &theApp.m_AccessDatabase;
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, _T("SELECT * FROM Types"), NULL);

		bool bFoundCF_UNICODETEXT = false;
		bool bAddedOne = false;

		while(!recset.IsEOF())
		{
			theApp.m_db.execDMLEx(_T("INSERT INTO Types VALUES(NULL, '%s');"), recset.m_TypeText);

			bAddedOne = true;

			if(recset.m_TypeText == "CF_UNICODETEXT")
				bFoundCF_UNICODETEXT = true;

			recset.MoveNext();
		}

		if(bAddedOne && bFoundCF_UNICODETEXT == false)
		{
			theApp.m_db.execDML(_T("INSERT INTO Types VALUES(NULL, 'CF_UNICODETEXT');"));
		}
	}
	catch(CDaoException* e)
	{
		TCHAR cError[100];
		e->GetErrorMessage(cError, 100);
		Log(StrF(_T("CDaoException - %s"), cError));

		ASSERT(FALSE);
		e->Delete();
		return false;
	}
	catch (CppSQLite3Exception& e)
	{
		Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));

		ASSERT(FALSE);
		return false;
	}	

	return true;
}

void CConvert::SetupProgressWnd()
{
	CMainTable recset;
	recset.m_pDatabase = &theApp.m_AccessDatabase;
	recset.Open(AFX_DAO_USE_DEFAULT_TYPE, _T(""), 0);

	recset.MoveLast();

	m_Progress.Create(NULL, _T("Ditto Conversion"), TRUE);
	m_Progress.SetRange(0, recset.GetRecordCount());
	m_Progress.HideCancel();	
	m_Progress.SetText(_T("Converting version 2 database to version 3 database.\n")
					_T("     Version 3 now uses sqlite (sqlite.org) as it's database\n")
					_T("     Version 3 now support Unicode, display international characters"));

	::SetWindowPos(m_Progress.GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
}