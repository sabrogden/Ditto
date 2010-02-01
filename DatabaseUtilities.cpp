// DatabaseUtilites.cpp: implementation of the CDatabaseUtilites class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "DatabaseUtilities.h"
#include "ProcessPaste.h"
#include <io.h>
#include "AccessToSqlite.h"
#include "Path.h"

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
		csOriginal = csPath + StrF(_T(".%03d"), count);
		// in case of some weird infinite loop
		if( count > 50 )
		{
			ASSERT(0);
			return FALSE;
		}
	} while( !::CopyFile(csPath, csOriginal, TRUE));
	
	return TRUE;
}

CString GetDBName()
{
	return CGetSetOptions::GetDBPath();
}

CString GetOLDDefaultDBName()
{
	CString csDefaultPath;
	LPMALLOC pMalloc;

	if(SUCCEEDED(::SHGetMalloc(&pMalloc))) 
	{ 
		LPITEMIDLIST pidlPrograms;

		SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidlPrograms);

		TCHAR string[MAX_PATH];
		SHGetPathFromIDList(pidlPrograms, string);

		pMalloc->Free(pidlPrograms);
		pMalloc->Release();

		csDefaultPath = string;		
		csDefaultPath += "\\Ditto\\";

		csDefaultPath += "DittoDB.mdb";
	}

	return csDefaultPath;
}

CString GetDefaultDBName()
{
	CString csDefaultPath = _T("c:\\program files\\Ditto\\");

	if(g_Opt.m_bU3)
	{
		csDefaultPath = CGetSetOptions::GetPath(PATH_DATABASE);
	}
	else
	{	
		//If portable then default to the running path
		if(CGetSetOptions::GetIsPortableDitto())
		{
			csDefaultPath.Empty();
		}
		else
		{
			LPMALLOC pMalloc;
		
			if(SUCCEEDED(::SHGetMalloc(&pMalloc))) 
			{ 
				LPITEMIDLIST pidlPrograms;
				
				SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidlPrograms);
				
				TCHAR string[MAX_PATH];
				SHGetPathFromIDList(pidlPrograms, string);
				
				pMalloc->Free(pidlPrograms);
				pMalloc->Release();
				
				csDefaultPath = string;		
			}

			FIX_CSTRING_PATH(csDefaultPath);
			csDefaultPath += "Ditto\\";
		}
	}

	CString csTempName = csDefaultPath + "Ditto.db";
	int i = 1;
	while(FileExists(csTempName))
	{
		csTempName.Format(_T("%sDitto_%d.db"), csDefaultPath, i);
		i++;
	}
	csDefaultPath = csTempName;
	
	return csDefaultPath;
}

BOOL CheckDBExists(CString csDBPath)
{
	//If this is the first time running this version then convert the old database to the new db
	if(csDBPath.IsEmpty() && g_Opt.m_bU3 == false)
	{
		csDBPath = GetDefaultDBName();

		if(FileExists(csDBPath) == FALSE && CGetSetOptions::GetIsPortableDitto() == FALSE)
		{
			CString csOldDB = CGetSetOptions::GetDBPathOld();
			if(csOldDB.IsEmpty())
			{
				csOldDB = GetOLDDefaultDBName();
			}

			if(FileExists(csOldDB))
			{
				//create the new sqlite db
				CreateDB(csDBPath);

				CAccessToSqlite Convert;
				Convert.ConvertDatabase(csDBPath, csOldDB);
			}
		}
	}

	BOOL bRet = FALSE;
	if(FileExists(csDBPath) == FALSE)
	{
		csDBPath = GetDefaultDBName();

		nsPath::CPath FullPath(csDBPath);
		CString csPath = FullPath.GetPath().GetStr();
		if(csPath.IsEmpty() == false && FileExists(csDBPath) == FALSE)
		{
			CreateDirectory(csPath, NULL);
		}

		// -- create a new one
		bRet = CreateDB(csDBPath);
	}
	else
	{
		if(ValidDB(csDBPath) == FALSE)
		{		
			//Db existed but was bad
			CString csMarkAsBad;
			
			csMarkAsBad = csDBPath;
			csMarkAsBad.Replace(_T("."), _T("_BAD."));
			
			CString csPath = GetDefaultDBName();
			
			CString cs;
			cs.Format(_T("%s \"%s\",\n")
				_T("%s \"%s\",\n")
				_T("%s,\n")
				_T("\"%s\""),
				theApp.m_Language.GetString("Database_Format", "Unrecognized Database Format"),
				csDBPath, 
				theApp.m_Language.GetString("File_Renamed", "the file will be renamed"),
				csMarkAsBad, 
				theApp.m_Language.GetString("New_Database", "and a new database will be created"),
				csPath);
			
			AfxMessageBox(cs);
			
			CFile::Rename(csDBPath, csMarkAsBad);

			csDBPath = csPath;
			
			bRet = CreateDB(csDBPath);
		}
		else
		{
			bRet = TRUE;
		}
	}

	if(bRet)
	{
		bRet = OpenDatabase(csDBPath);
	}
	
	return bRet;
}

BOOL OpenDatabase(CString csDB)
{
	try
	{
		theApp.m_db.close();
		theApp.m_db.open(csDB);
		CGetSetOptions::SetDBPath(csDB);

		return TRUE;
	}
	CATCH_SQLITE_EXCEPTION

	return FALSE;
}

BOOL ValidDB(CString csPath, BOOL bUpgrade)
{
	try
	{
		CppSQLite3DB db;
		db.open(csPath);

		db.execQuery(_T("SELECT lID, lDate, mText, lShortCut, lDontAutoDelete, ")
								_T("CRC, bIsGroup, lParentID, QuickPasteText ")
								_T("FROM Main"));

		db.execQuery(_T("SELECT lID, lParentID, strClipBoardFormat, ooData FROM Data"));

		db.execQuery(_T("SELECT lID, TypeText FROM Types"));

		try
		{
			db.execDML(_T("DROP TRIGGER delete_data_trigger"));
		}
		catch(CppSQLite3Exception& e)
		{
			e.errorCode();
		}

		try
		{
			db.execDML(_T("DROP TRIGGER delete_copy_buffer_trigger"));
		}
		catch(CppSQLite3Exception& e)
		{
			e.errorCode();
		}

		//This was added later so try to add each time and catch the exception here
 		try
 		{
			db.execDML(_T("CREATE TRIGGER delete_data_trigger BEFORE DELETE ON Main FOR EACH ROW\n")
				_T("BEGIN\n")
					_T("INSERT INTO MainDeletes VALUES(old.lID, datetime('now'));\n")
				_T("END\n"));
 		}
		catch(CppSQLite3Exception& e)
		{
 			e.errorCode();
 		}

		//This was added later so try to add each time and catch the exception here
		try
		{
			db.execQuery(_T("SELECT lID, lClipID, lCopyBuffer FROM CopyBuffers"));
		}
		catch(CppSQLite3Exception& e)
		{
			e.errorCode();

			db.execDML(_T("CREATE TABLE CopyBuffers(")
				_T("lID INTEGER PRIMARY KEY AUTOINCREMENT, ")
				_T("lClipID INTEGER,")
				_T("lCopyBuffer INTEGER)"));
		}

		//This was added later so try to add each time and catch the exception here
		try
		{
			db.execQuery(_T("SELECT clipId FROM MainDeletes"));
		}
		catch(CppSQLite3Exception& e)
		{
			e.errorCode();

			db.execDML(_T("CREATE TABLE MainDeletes(")
				_T("clipID INTEGER,")
				_T("modifiedDate)"));

			db.execDML(_T("CREATE TRIGGER MainDeletes_delete_data_trigger BEFORE DELETE ON MainDeletes FOR EACH ROW\n")
				_T("BEGIN\n")
				_T("DELETE FROM CopyBuffers WHERE lClipID = old.clipID;\n")
				_T("DELETE FROM Data WHERE lParentID = old.clipID;\n")
				_T("END\n"));
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)

	return TRUE;                                                     
}

BOOL CreateDB(CString csFile)
{
	try
	{
		CppSQLite3DB db;
		db.open(csFile);
		
		db.execDML(_T("PRAGMA auto_vacuum = 1"));

		db.execDML(_T("CREATE TABLE Main(")
								_T("lID INTEGER PRIMARY KEY AUTOINCREMENT, ")
								_T("lDate INTEGER, ")
								_T("mText TEXT, ")
								_T("lShortCut INTEGER, ")
								_T("lDontAutoDelete INTEGER, ")
								_T("CRC INTEGER, ")
								_T("bIsGroup INTEGER, ")
								_T("lParentID INTEGER, ")
								_T("QuickPasteText TEXT);"));

		db.execDML(_T("CREATE TABLE Data(")
							_T("lID INTEGER PRIMARY KEY AUTOINCREMENT, ")
							_T("lParentID INTEGER, ")
							_T("strClipBoardFormat TEXT, ")
							_T("ooData BLOB);"));

		db.execDML(_T("CREATE TABLE Types(")
							_T("lID INTEGER PRIMARY KEY AUTOINCREMENT, ")
							_T("TypeText TEXT);"));

		db.execDML(_T("CREATE UNIQUE INDEX Main_ID on Main(lID ASC)"));
		db.execDML(_T("CREATE UNIQUE INDEX Data_ID on Data(lID ASC)"));
		db.execDML(_T("CREATE INDEX Main_Date on Main(lDate DESC)"));

		db.execDML(_T("CREATE TRIGGER delete_data_trigger BEFORE DELETE ON Main FOR EACH ROW\n")
			_T("BEGIN\n")
				_T("INSERT INTO MainDeletes VALUES(old.lID, datetime('now'));\n")
			_T("END\n"));

		db.execDML(_T("CREATE TABLE CopyBuffers(")
			_T("lID INTEGER PRIMARY KEY AUTOINCREMENT, ")
			_T("lClipID INTEGER, ")
			_T("lCopyBuffer INTEGER)"));

		db.execDML(_T("CREATE TABLE MainDeletes(")
			_T("clipID INTEGER,")
			_T("modifiedDate)"));

		db.execDML(_T("CREATE TRIGGER MainDeletes_delete_data_trigger BEFORE DELETE ON MainDeletes FOR EACH ROW\n")
			_T("BEGIN\n")
				_T("DELETE FROM CopyBuffers WHERE lClipID = old.clipID;\n")
				_T("DELETE FROM Data WHERE lParentID = old.clipID;\n")
			_T("END\n"));

		db.close();
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)

	return TRUE;
}

BOOL CompactDatabase()
{
//	if(!theApp.CloseDB())
//		return FALSE;
//
//	CString csDBName = GetDBName();
//	CString csTempDBName = csDBName;
//	csTempDBName.Replace(".mdb", "TempDBName.mdb");
//	
//	//Compact the database			
//	try
//	{
//		CDaoWorkspace::CompactDatabase(csDBName, csTempDBName);//, dbLangGeneral, 0, "andrew");//DATABASE_PASSWORD);
//	}
//	catch(CDaoException* e)
//	{
//		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
//		DeleteFile(csTempDBName);
//		e->Delete();
//		return FALSE;
//	}
//	catch(CMemoryException* e) 
//	{
//		AfxMessageBox("Memory Exception");
//		DeleteFile(csTempDBName);
//		e->Delete();
//		return FALSE;
//	}
//	
//	//Since compacting the database creates a new db delete the old one and replace it
//	//with the compacted db
//	if(DeleteFile(csDBName))
//	{
//		try
//		{
//			CFile::Rename(csTempDBName, csDBName);
//		}
//		catch(CFileException *e)
//		{
//			e->ReportError();
//			e->Delete();
//			return FALSE;
//		}
//	}
//	else
//		AfxMessageBox("Error Compacting Database");
	
	return TRUE;
}

BOOL RepairDatabase()
{
//	if(!theApp.CloseDB())
//		return FALSE;
	
//	try
//	{
//		CDaoWorkspace::RepairDatabase(GetDBName());
//	}
//	catch(CDaoException *e)
//	{
//		AfxMessageBox(e->m_pErrorInfo->m_strDescription);
//		e->Delete();
//		return FALSE;
//	}
	
	return TRUE;
}

BOOL RemoveOldEntries()
{
	Log(StrF(_T("Beginning of RemoveOldEntries MaxEntries: %d - Keep days: %d"), CGetSetOptions::GetMaxEntries(), CGetSetOptions::GetExpiredEntries()));

	try
	{
		CppSQLite3DB db;
		CString csDbPath = CGetSetOptions::GetDBPath();
		db.open(csDbPath);

		if(CGetSetOptions::GetCheckForMaxEntries())
		{
			long lMax = CGetSetOptions::GetMaxEntries();
			if(lMax >= 0)
			{
				CClipIDs IDs;
				
				CppSQLite3Query q = db.execQueryEx(_T("SELECT lID, lShortCut, lDontAutoDelete FROM Main ORDER BY lDate DESC LIMIT -1 OFFSET %d"), lMax);			
				while(q.eof() == false)
				{
					//Only delete entries that have no shortcut and don't have the flag set
					if(q.getIntField(_T("lShortCut")) == 0 && q.getIntField(_T("lDontAutoDelete")) == 0)
						IDs.Add(q.getIntField(_T("lID")));

					Log(StrF(_T("From MaxEntries - Deleting Id: %d"), q.getIntField(_T("lID"))));

					q.nextRow();
				}

				if(IDs.GetCount() > 0)
				{
					IDs.DeleteIDs(false, db);
				}
			}
		}
		
		if(CGetSetOptions::GetCheckForExpiredEntries())
		{
			long lExpire = CGetSetOptions::GetExpiredEntries();
			
			if(lExpire)
			{
				CTime now = CTime::GetCurrentTime();
				now -= CTimeSpan(lExpire, 0, 0, 0);
				
				CClipIDs IDs;
				
				CppSQLite3Query q = db.execQueryEx(_T("SELECT lID FROM Main ")
													_T("WHERE lDate < %d AND ")
													_T("lShortCut = 0 AND lDontAutoDelete = 0"), now.GetTime());

				while(q.eof() == false)
				{
					IDs.Add(q.getIntField(_T("lID")));

					Log(StrF(_T("From Clips Expire - Deleting Id: %d"), q.getIntField(_T("lID"))));

					q.nextRow();
				}
				
				if(IDs.GetCount() > 0)
				{
					IDs.DeleteIDs(false, db);
				}
			}
		}

		Log(_T("Before Deleting emptied out data"));

		//delete any data items sitting out there that the main table data was deleted
		//this was done to speed up deleted from the main table
		int deleteCount = db.execDML(_T("DELETE FROM MainDeletes"));

		Log(StrF(_T("After Deleting emptied out data rows, Count: %d"), deleteCount));
	}
	CATCH_SQLITE_EXCEPTION
	
	Log(_T("End of RemoveOldEntries"));

	return TRUE;
}

BOOL EnsureDirectory(CString csPath)
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	
	SPLITPATH(csPath, drive, dir, fname, ext);
	
	CString csDir(drive);
	csDir += dir;
	
	if(FileExists(csDir) == FALSE)
	{
		if(CreateDirectory(csDir, NULL))
			return TRUE;
	}
	else
		return TRUE;
	
	return FALSE;
}

// BOOL RunZippApp(CString csCommandLine)
// {
// 	CString csLocalPath = GETENV(_T("U3_HOST_EXEC_PATH"));
// 	FIX_CSTRING_PATH(csLocalPath);
// 
// 	CString csZippApp = GETENV(_T("U3_DEVICE_EXEC_PATH"));
// 	FIX_CSTRING_PATH(csZippApp);
// 	csZippApp += "7za.exe";
// 
// 	csZippApp += " ";
// 	csZippApp += csCommandLine;
// 
// 	Log(csZippApp);
// 
// 	STARTUPINFO			StartupInfo;
// 	PROCESS_INFORMATION	ProcessInformation;
// 
// 	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
// 	StartupInfo.cb = sizeof(StartupInfo);
// 	ZeroMemory(&ProcessInformation, sizeof(ProcessInformation));
// 
// 	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
// 	StartupInfo.wShowWindow = SW_HIDE;
// 
// 	BOOL bRet = CreateProcess(NULL, csZippApp.GetBuffer(csZippApp.GetLength()), NULL, NULL, FALSE,
// 			CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS, NULL, csLocalPath,
// 			&StartupInfo, &ProcessInformation);
// 
// 	if(bRet)
// 	{
// 		WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
// 
// 		DWORD dwExitCode;
// 		GetExitCodeProcess(ProcessInformation.hProcess, &dwExitCode);
// 
// 		CString cs;
// 		cs.Format(_T("Exit code from unzip = %d"), dwExitCode);
// 		Log(cs);
// 
// 		if(dwExitCode != 0)
// 		{
// 			bRet = FALSE;
// 		}
// 	}
// 	else
// 	{
// 		bRet = FALSE;
// 		Log(_T("Create Process Failed"));
// 	}
// 
// 	csZippApp.ReleaseBuffer();
// 
// 	return bRet;
// }

// BOOL CopyDownDatabase()
// {
// 	BOOL bRet = FALSE;
// 
// 	CString csZippedPath = GETENV(_T("U3_APP_DATA_PATH"));
// 	FIX_CSTRING_PATH(csZippedPath);
// 	
// 	CString csUnZippedPath = csZippedPath;
// 	csUnZippedPath += "Ditto.db";
// 
// 	csZippedPath += "Ditto.7z";
// 	
// 	CString csLocalPath = GETENV(_T("U3_HOST_EXEC_PATH"));
// 	FIX_CSTRING_PATH(csLocalPath);
// 
// 	if(FileExists(csZippedPath))
// 	{
// 		CString csCommandLine;
// 
// 		//e = extract
// 		//surround command line arguments with quotes
// 		//-aoa = overight files with extracted files
// 
// 		csCommandLine += "e ";
// 		csCommandLine += "\"";
// 		csCommandLine += csZippedPath;
// 		csCommandLine += "\"";
// 		csCommandLine += " -o";
// 		csCommandLine += "\"";
// 		csCommandLine += csLocalPath;
// 		csCommandLine += "\"";
// 		csCommandLine += " -aoa";
// 
// 		bRet = RunZippApp(csCommandLine);
// 
// 		csLocalPath += "Ditto.db";
// 	}
// 	else if(FileExists(csUnZippedPath))
// 	{
// 		csLocalPath += "Ditto.db";
// 		bRet = CopyFile(csUnZippedPath, csLocalPath, FALSE);
// 	}
// 
// 	if(FileExists(csLocalPath) == FALSE)
// 	{
// 		Log(_T("Failed to copy files from device zip file"));
// 	}
// 
// 	g_Opt.nLastDbWriteTime = GetLastWriteTime(csLocalPath);
// 
// 	return bRet;
// }

//BOOL CopyUpDatabase()
//{
//	CStringA csZippedPath = "C:\\";//getenv("U3_APP_DATA_PATH");
//	FIX_CSTRING_PATH(csZippedPath);
//	csZippedPath += "Ditto.zip";

//	CStringA csLocalPath = GetDBName();//getenv("U3_HOST_EXEC_PATH");
//	//FIX_CSTRING_PATH(csLocalPath);
//	//csLocalPath += "Ditto.db";
//
//	CZipper Zip;
//
//	if(Zip.OpenZip(csZippedPath))
//	{
//		Zip.AddFileToZip(csLocalPath);
//	}
//
//	return TRUE;
//}