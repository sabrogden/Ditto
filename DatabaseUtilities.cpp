// DatabaseUtilites.cpp: implementation of the CDatabaseUtilites class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "DatabaseUtilities.h"
#include <io.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL DoCleanups()
{
	try
	{		
		//Try and open mText if it's not there then create it
		CDaoRecordset recset(theApp.EnsureOpenDB());
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, "SELECT mText FROM Main", 0);
		recset.Close();		
	}
	catch(CDaoException* e)
	{
		e->Delete();
		try
		{
			theApp.EnsureOpenDB();
			theApp.m_pDatabase->Execute("ALTER TABLE Main ADD COLUMN mText MEMO", dbFailOnError);
			theApp.m_pDatabase->Execute("UPDATE Main SET mText=strText", dbFailOnError);
			theApp.m_pDatabase->Execute("ALTER TABLE Main DROP COLUMN strText", dbFailOnError);
			theApp.m_pDatabase->Execute("ALTER TABLE Main DROP COLUMN strType", dbFailOnError);
		}
		catch(CDaoException *e)
		{
			CString cs;
			cs = e->m_pErrorInfo->m_strDescription;
			cs += "\n\nError updating Database!";
			MessageBox(NULL, cs, "Ditto", MB_OK);

			e->Delete();
			return FALSE;
		}
	}

	return TRUE;
}

CString GetDBName()
{
	return CGetSetOptions::GetDBPath();
}

CString GetDefaultDBName()
{
	CString csDefaultPath;
	LPMALLOC pMalloc;
	
	if(SUCCEEDED(::SHGetMalloc(&pMalloc))) 
	{ 
		LPITEMIDLIST pidlPrograms;

		SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidlPrograms);

		char string[MAX_PATH];
		SHGetPathFromIDList(pidlPrograms, string);

		pMalloc->Free(pidlPrograms);
		pMalloc->Release();

		csDefaultPath = string;		
		csDefaultPath += "\\Ditto\\";
		
		if(_access(csDefaultPath, 0) == -1)
			CreateDirectory(csDefaultPath, NULL);
		
		csDefaultPath += DEFAULT_DB_NAME;
	}

	return csDefaultPath;
}

BOOL CheckDBExists(CString csDBPath)
{
	if(_access(csDBPath, 0) == -1)
	{
		//Database didn't exist
		CGetSetOptions::SetDBPath("");
		
		// -- create a new one
		CreateDB(GetDefaultDBName());
		return TRUE;
	}

	BOOL bRet = FALSE;

	if(ValidDB(csDBPath) == FALSE)
	{
		theApp.CloseDB();
		
		CGetSetOptions::SetDBPath("");

		//Db existed but was bad

		CString csMarkAsBad;
		
		csMarkAsBad = csDBPath;
		csMarkAsBad.Replace(".", "_BAD.");
		
		CString csPath = GetDefaultDBName();

		CString cs;
		cs.Format("Unrecognized Database Format \"%s\",\n"
					"the file will be renamed \"%s\",\n"
					"and a new database will be created,\n"
					"\"%s\"", csDBPath, csMarkAsBad, csPath);

		AfxMessageBox(cs);
		
		CFile::Rename(csDBPath, csMarkAsBad);
		

		bRet = CreateDB(csPath);
	}
	else
		bRet = TRUE;

	return bRet;
}

BOOL ValidDB(CString csPath)
{
	try
	{
		CDaoDatabase db;
		db.Open(csPath);

		CDaoTableDef table(&db);
		CDaoFieldInfo info;

		table.Open("Main");
		table.Close();

		table.Open("Data");
		table.Close();

		table.Open("Types");
		table.Close();
	}
	catch(CDaoException* e)
	{
		e->Delete();
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

BOOL CreateDB(CString csPath)
{
	CDaoDatabase db;

	try
	{
		EnsureDirectory(csPath);

		db.Create(csPath);
		
		CDaoTableDefEx table(&db);

		//Creat the Main table
		table.Create("Main");
		
		table.CreateField("lID", dbLong, 4, dbAutoIncrField);
		table.CreateIndex(TRUE, "lID");

		table.CreateField("lDate", dbLong, 4, 0, "0");
		table.CreateIndex(FALSE, "lDate");

		table.CreateField("mText", dbMemo, 0, dbVariableField);
//		table.CreateField("strText", dbText, 250, dbVariableField);

		table.CreateField("lShortCut", dbLong, 4, 0, "0");
		table.CreateIndex(FALSE, "lShortCut");
		
		table.CreateField("lDontAutoDelete", dbLong, 4, 0, "0");

		table.CreateField("lTotalCopySize", dbLong, 4, 0, "0");

		table.Append();
		table.Close();

		//Create the Data Table
		table.Create("Data");
		
		table.CreateField("lID", dbLong, 4, dbAutoIncrField);
		table.CreateIndex(TRUE, "lID");

		table.CreateField("lParentID", dbLong, 4, 0, "0");
		table.CreateIndex(FALSE, "lParentID");

		table.CreateField("strClipBoardFormat", dbText, 50, dbVariableField);
		table.CreateField("ooData", dbLongBinary, 0);

		table.Append();
		table.Close();

		//Create the Types table
		table.Create("Types");
		table.CreateField("ID", dbLong, 4, dbAutoIncrField);
		table.CreateField("TypeText", dbText, 50, dbVariableField);

		table.Append();
		table.Close();

		db.Close();
		
		return TRUE;
	}
	catch(CDaoException *e)
	{
		ASSERT(FALSE);
		e->Delete();
	}

	return FALSE;
}

BOOL CompactDatabase()
{
	if(!theApp.CloseDB())
		return FALSE;

	CString csDBName = GetDBName();
	CString csTempDBName = csDBName;
	csTempDBName.Replace(".mdb", "TempDBName.mdb");

	//Compact the database			
	try
	{
		CDaoWorkspace::CompactDatabase(csDBName, csTempDBName);//, dbLangGeneral, 0, "andrew");//DATABASE_PASSWORD);
	}
	catch(CDaoException* e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		DeleteFile(csTempDBName);
		e->Delete();
		return FALSE;
	}
	catch(CMemoryException* e) 
	{
		AfxMessageBox("Memory Exception");
		DeleteFile(csTempDBName);
		e->Delete();
		return FALSE;
	}


	//Since compacting the database creates a new db delete the old one and replace it
	//with the compacted db
	if(DeleteFile(csDBName))
	{
		try
		{
			CFile::Rename(csTempDBName, csDBName);
		}
		catch(CFileException *e)
		{
			e->ReportError();
			e->Delete();
			return FALSE;
		}
	}
	else
		AfxMessageBox("Error Compacting Database");

	return TRUE;
}

BOOL RepairDatabase()
{
	if(!theApp.CloseDB())
		return FALSE;

	try
	{
		CDaoWorkspace::RepairDatabase(GetDBName());
	}
	catch(CDaoException *e)
	{
		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
		e->Delete();
		return FALSE;
	}

	return TRUE;
}

BOOL RemoveOldEntries()
{
	if(CGetSetOptions::GetCheckForMaxEntries())
	{
		long lMax = CGetSetOptions::GetMaxEntries();

		CMainTable recset;
		recset.Open("SELECT * FROM Main ORDER BY lDate DESC");
		if(!recset.IsEOF())
		{
			recset.MoveLast();

			long lCount = recset.GetRecordCount();

			ARRAY IDs;

			while((lCount > lMax) && (!recset.IsBOF()))
			{
				//Don't delete entries that have shorcuts or the flag set
				if(recset.m_lDontAutoDelete <= 0)
					IDs.Add(recset.m_lID);

				lCount--;
				recset.MovePrev();
			}

			CClip::Delete(IDs);
		}
	}

	if(CGetSetOptions::GetCheckForExpiredEntries())
	{
		long lExpire = CGetSetOptions::GetExpiredEntries();

		if(lExpire)
		{
			CTime now = CTime::GetCurrentTime();
			now -= CTimeSpan(lExpire, 0, 0, 0);

			CMainTable recset;
			recset.Open("SELECT * FROM Main "
						"WHERE lDate < %d AND "
						"lShortCut <= 0 AND lDontAutoDelete <= 0", now.GetTime());

			ARRAY IDs;

			while(!recset.IsEOF())
			{
				IDs.Add(recset.m_lID);
				recset.MoveNext();
			}

			CClip::Delete(IDs);
		}
	}

	return TRUE;
}

BOOL EnsureDirectory(CString csPath)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(csPath, drive, dir, fname, ext);

	CString csDir(drive);
	csDir += dir;

	if(_access(csDir, 0) == -1)
	{
		if(CreateDirectory(csDir, NULL))
			return TRUE;
	}
	else
		return TRUE;

	return FALSE;
}