// DatabaseUtilites.cpp: implementation of the CDatabaseUtilites class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "DatabaseUtilities.h"
#include "ProcessPaste.h"
#include <io.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CreateBackup(CString csPath)
{
	CString csOriginal;
	int count = 0;
	// create a backup of the existing database
	do
	{
		count++;
		csOriginal = csPath + StrF(".%03d",count);
		// in case of some weird infinite loop
		if( count > 50 )
		{
			ASSERT(0);
			return FALSE;
		}
	} while( !::CopyFile(csPath, csOriginal, TRUE) );
	
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
		return CreateDB(GetDefaultDBName());
	}
	
	BOOL bRet = FALSE;

	int nRet = ValidDB(csDBPath);
	
	if(nRet == FALSE)
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
	else if(nRet == ERROR_OPENING_DATABASE)
		bRet = ERROR_OPENING_DATABASE;
	else
		bRet = TRUE;
	
	return bRet;
}

// m_pErrorInfo:
// - m_lErrorCode      0x00000cc1
// - m_strSource       "DAO.Fields"
// - m_strDescription  "Item not found in this collection."
#define ON_FIELD_ABSENT(name,onabsent) \
	try { table.GetFieldInfo(name,info); } \
	catch(CDaoException* e) \
{ \
	if( !bUpgrade || e->m_pErrorInfo->m_lErrorCode != 0x00000cc1 ) \
	throw e; \
	if( bUpgraded == FALSE ) \
	CreateBackup(csPath); \
	bResult &= onabsent; \
	bUpgraded = TRUE; \
	e->Delete(); \
}

BOOL ValidDB(CString csPath, BOOL bUpgrade)
{
	BOOL bResult = TRUE;
	BOOL bUpgraded = FALSE;
	try
	{
		CDaoDatabase db;

		try
		{
			db.Open(csPath);
		}
		catch(CDaoException* e)
		{
			TCHAR   szErrorMessage[512];
			UINT    nHelpContext;

			if(e->GetErrorMessage(szErrorMessage, 512, &nHelpContext))
			{
				if(strcmp(szErrorMessage, "Unable to initialize DAO/Jet db engine.") == 0)
				{
					e->Delete();
					return ERROR_OPENING_DATABASE;				
				}
			}
			e->ReportError();
			e->Delete();
		}
		
		
		CDaoTableDef table(&db);
		CDaoFieldInfo info;
		
		table.Open("Main");
		table.GetFieldInfo("lID", info);
		table.GetFieldInfo("lDate", info);
		ON_FIELD_ABSENT("mText", Upgrade_mText(db)); // +mText, -strText, -strType
		table.GetFieldInfo("lShortCut", info);
		table.GetFieldInfo("lDontAutoDelete", info);
		table.GetFieldInfo("lTotalCopySize", info);
		ON_FIELD_ABSENT("bIsGroup", Upgrade_Groups(db));
		table.GetFieldInfo("lParentID", info); // part of Upgrade_Groups
		table.GetFieldInfo("dOrder", info);  // part of Upgrade_Groups
		ON_FIELD_ABSENT("lDataID", Upgrade_ShareData(db)); // +lDataID, -lParentID
		table.Close();
		
		table.Open("Data");
		table.GetFieldInfo("lID", info);
		table.GetFieldInfo("lDataID", info); // part of Upgrade_ShareData()
		table.GetFieldInfo("strClipBoardFormat", info);
		table.GetFieldInfo("ooData", info);
		table.Close();
		
		table.Open("Types");
		table.GetFieldInfo("ID", info);
		table.GetFieldInfo("TypeText", info);
		table.Close();
	}
	catch(CDaoException* e)
	{
		e->ReportError();
		ASSERT(FALSE);
		e->Delete();
		return FALSE;
	}
	
	// if we upgraded, perform full validation again without upgrading
	if( bUpgraded )
		return ValidDB( csPath, FALSE);
	
	return bResult;
}

BOOL CreateDB(CString csPath)
{
	try
	{
		CDaoDatabase db;
		EnsureDirectory(csPath);

		try
		{
			db.Create(csPath);
		}
		catch(CDaoException* e)
		{
			TCHAR   szErrorMessage[512];
			UINT    nHelpContext;

			if(e->GetErrorMessage(szErrorMessage, 512, &nHelpContext))
			{
				if(strcmp(szErrorMessage, "Unable to initialize DAO/Jet db engine.") == 0)
				{
					e->Delete();
					return ERROR_OPENING_DATABASE;	
				}
			}

			e->ReportError();
			e->Delete();

			return FALSE;
		}
		
		CDaoTableDefEx table(&db);
		//Create the Main table
		table.Create("Main");
		table.CreateField("lID", dbLong, 4, dbAutoIncrField);
		table.CreateIndex(TRUE, "lID");
		table.CreateField("lDate", dbLong, 4, 0, "0");
		table.CreateIndex(FALSE, "lDate");
		table.CreateField("mText", dbMemo, 0, dbVariableField);
		table.CreateField("lShortCut", dbLong, 4, 0, "0");
		table.CreateIndex(FALSE, "lShortCut");
		table.CreateField("lDontAutoDelete", dbLong, 4, 0, "0");
		table.CreateField("lTotalCopySize", dbLong, 4, 0, "0");
		// GROUPS
		table.CreateField("bIsGroup", dbBoolean, 1, 0, "0"); // for Groups
		table.CreateIndex(FALSE, "bIsGroup");
		table.CreateField("lParentID", dbLong, 4, 0, "0"); // parent Group Main.lID
		table.CreateIndex(FALSE, "lParentID");
		table.CreateField("dOrder", dbDouble, 8, 0, "0"); // for Order within Groups
		table.CreateIndex(FALSE, "dOrder");
		// for sharing data amongst multiple clips
		table.CreateField("lDataID", dbLong, 4, 0, "0"); // corresponds to Data.lDataID
		table.CreateIndex(FALSE, "lDataID");
		
		table.Append();
		table.Close();
		
		//Create the Data Table
		table.Create("Data");
		
		table.CreateField("lID", dbLong, 4, dbAutoIncrField);
		table.CreateIndex(TRUE, "lID");
		table.CreateField("lDataID", dbLong, 4, 0, "0");
		table.CreateIndex(FALSE, "lDataID");
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
		e->ReportError();
		ASSERT(FALSE);
		e->Delete();
	}
	
	return FALSE;
}

// +mText, -strText, -strType
BOOL Upgrade_mText(CDaoDatabase& db)
{
	try
	{
		db.Execute("ALTER TABLE Main ADD COLUMN mText MEMO", dbFailOnError);
		db.Execute("UPDATE Main SET mText=strText", dbFailOnError);
		db.Execute("ALTER TABLE Main DROP COLUMN strText", dbFailOnError);
		db.Execute("ALTER TABLE Main DROP COLUMN strType", dbFailOnError);
	}
	CATCHDAO
		return TRUE;
}

BOOL Upgrade_Groups(CDaoDatabase& db)
{
	try
	{
		CDaoTableDefEx table(&db);
		table.Open("Main");
		// Groups
		table.CreateField("bIsGroup", dbBoolean, 1, 0, "0"); // for Groups
		table.CreateIndex(FALSE, "bIsGroup");
		table.CreateField("lParentID", dbLong, 4, 0, "0"); // parent Group Main.lID
		table.CreateIndex(FALSE, "lParentID");
		table.CreateField("dOrder", dbDouble, 8, 0, "0"); // for Order within Groups
		table.CreateIndex(FALSE, "dOrder");
		table.Close();
		// set defaults (otherwise might be NULL)
		db.Execute("UPDATE Main SET bIsGroup = 0, lParentID = 0, dOrder = 0", dbFailOnError);
	}
	CATCHDAO
		return TRUE;
}

BOOL Upgrade_ShareData(CDaoDatabase& db)
{
	CPopup status(10000,10000); // peg at the bottom-right corner of screen
	try
	{
		CDaoTableDefEx table(&db);
		
		table.Open("Main");
		table.CreateField("lDataID", dbLong, 4, 0, "0"); // corresponds to Data.lDataID
		table.CreateIndex(FALSE, "lDataID");
		table.Close();
		
		table.Open("Data");
		table.CreateField("lDataID", dbLong, 4, 0, "0"); // parent Group Main.lID
		table.CreateIndex(FALSE, "lDataID");
		table.Close();
		
		// set defaults
		db.Execute(	"UPDATE Main SET lDataID = 0", dbFailOnError );
		db.Execute( "UPDATE Data SET lDataID = 0", dbFailOnError );
		
		// update Main.lDataID and Data.lParentID for sharing Data
		//
		// - multiple Formats (Data.lID) exist for a single ClipData (Data.lDataID)
		// - The value of lDataID is arbitrary, but must be unique to the ClipData.
		//   - In order to ensure uniqueness, lDataID is assigned the lID of
		//     the first Format in the Clip's set.
		
		COleVariant var((long)0);
		CDaoRecordset main(&db);
		long main_fldID;
		long main_fldDataID;
		long main_lID;
		
		CDaoRecordset data(&db);
		long data_fldID;
		long data_fldDataID;
		long lDataID;
		int count = 0;
		int i = 0;
		int percentPrev = -1;
		int percent = -1;
		
		main.Open(dbOpenDynaset,"SELECT lID, lDataID FROM Main");
		
		main_fldID = GetFieldPos(main,"lID");
		VERIFY(main_fldID == 0);
		main_fldDataID = GetFieldPos(main,"lDataID");
		VERIFY(main_fldDataID == 1);
		
		if( !main.IsEOF() )
		{
			main.MoveLast();
			count = main.GetRecordCount();
			main.MoveFirst();
		}
		
		// for each record in Main and its corresponding records in Data,
		//  assign a new unique lDataID.
		while( !main.IsEOF() )
		{
			i++;
			percentPrev = percent;
			percent = (i*100)/count;
			if( percent != percentPrev )
				status.Show(StrF("Ditto: Upgrading database (%d%%)",percent));
			
			main.GetFieldValue(main_fldID,var);
			main_lID = var.lVal;
			
			data.Open(dbOpenDynaset, StrF(
				"SELECT lID, lDataID "
				"FROM Data WHERE lParentID = %d", main_lID) );
			
			data_fldID = GetFieldPos(data,"lID");
			VERIFY(data_fldID == 0);
			data_fldDataID = GetFieldPos(data,"lDataID");
			VERIFY(data_fldDataID == 1);
			
			// lDataID = the first data record lID
			lDataID = 0;
			if( !data.IsEOF() )
			{
				data.GetFieldValue(0,var); // 0 == lID field
				lDataID = var.lVal;
			}
			// assign all Data records the same lDataID
			while( !data.IsEOF() )
			{
				var.lVal = lDataID;
				data.Edit();
				data.SetFieldValue(1,var); // 1 == lDataID field
				data.Update();
				data.MoveNext();
			}
			
			// assign Main.lDataID
			var.lVal = lDataID;
			main.Edit();
			main.SetFieldValue(1,var); // 1 == lDataID field
			main.Update();
			main.MoveNext();
			
			data.Close();
		}
		
		main.Close();
		
		// delete the old field
		db.Execute("ALTER TABLE Data DROP CONSTRAINT lParentID", dbFailOnError);
		db.Execute("ALTER TABLE Data DROP COLUMN lParentID", dbFailOnError);
	}
	CATCHDAO
		return TRUE;
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
			
			CClipIDs IDs;
			
			while((lCount > lMax) && (!recset.IsBOF()))
			{
				//Only delete entries that have no shortcut and don't have the flag set
				if(recset.m_lShortCut == 0 && recset.m_lDontAutoDelete == 0)
					IDs.Add(recset.m_lID);
				
				lCount--;
				recset.MovePrev();
			}
			
			IDs.DeleteIDs();
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
				"lShortCut = 0 AND lDontAutoDelete = 0", now.GetTime());
			
			CClipIDs IDs;
			
			while(!recset.IsEOF())
			{
				IDs.Add(recset.m_lID);
				recset.MoveNext();
			}
			
			IDs.DeleteIDs();
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

BOOL ExecuteSQL( CString csSQL, BOOL bReportError, CDaoException** ppEx )
{
	try
	{
		theApp.EnsureOpenDB();
		theApp.m_pDatabase->Execute(csSQL, dbFailOnError);
	}
	catch(CDaoException* e)
	{
		if( bReportError )
			e->ReportError();
		
		if( ppEx )
			*ppEx = e;
		else
			e->Delete();
		
		return FALSE;
	}
	
	return TRUE;
}

int GetFieldPos(CDaoRecordset& recs, LPCTSTR fieldName)
{
	CDaoFieldInfo fi;
	int count = recs.GetFieldCount();
	
	for( int i = 0; i < count; i++ )
	{
		recs.GetFieldInfo(i, fi);
		if( fi.m_strName.Compare( fieldName ) == 0 )
			return i; // when found a match, return it
	}
	
	return -1;
}

void VerifyFieldPos(CDaoRecordset& recs, LPCTSTR fieldName, int index)
{
	CDaoFieldInfo fi;
	int count = recs.GetFieldCount();
	VERIFY( index >= 0 && index < count );
	recs.GetFieldInfo(index, fi);
	VERIFY( fi.m_strName.Compare( fieldName ) == 0 );
}

CString GetFieldList(CDaoRecordset& recs)
{
	CString field;
	CString list;
	CDaoFieldInfo fi;
	int count = recs.GetFieldCount();
	
	for( int i = 0; i < count; i++ )
	{
		recs.GetFieldInfo(i, fi);
		field = StrF("\n%d: ",i) + fi.m_strName;
		list += field;
	}
	
	return list;
}