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
#include "InternetUpdate.h"

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

		theApp.m_db.setBusyTimeout(CGetSetOptions::GetDbTimeout());

		return TRUE;
	}
	CATCH_SQLITE_EXCEPTION

	return FALSE;
}

void ReOrderStickyClips(int parentID, CppSQLite3DB &db)
{
	try
	{
		Log(StrF(_T("Start of ReOrderStickyClips, ParentId %d"), parentID));

		//groups where created with 0 in these fields, fix them up if they are 0
		if(parentID == -1)
		{
			db.execDMLEx(_T("Update Main Set stickyClipOrder = -(2147483647) where bIsGroup = 1 AND stickyClipOrder = 0"));
			db.execDMLEx(_T("Update Main Set stickyClipGroupOrder = -(2147483647) where bIsGroup = 1 AND stickyClipGroupOrder = 0"));			
		}

		CppSQLite3Query qGroup = db.execQueryEx(_T("SELECT lID, mText FROM Main WHERE bIsGroup = 1 AND lParentID = %d"), parentID);

		if (qGroup.eof() == false)
		{
			while (!qGroup.eof())
			{
				//Get all sticky clips at the top level or group
				CString sql = StrF(_T("SELECT lID FROM Main WHERE stickyClipOrder <> -(2147483647) AND lParentID = %d ORDER BY stickyClipOrder DESC"), parentID);
				if (parentID > -1)
				{
					sql = StrF(_T("SELECT lID FROM Main WHERE stickyClipGroupOrder <> -(2147483647) AND lParentID = %d ORDER BY stickyClipGroupOrder DESC"), parentID);
				}

				CppSQLite3Query qSticky = db.execQueryEx(sql);

				int order = 1;

				if (qSticky.eof() == false)
				{
					while (!qSticky.eof())
					{
						//set the new order
						if (parentID > -1)
						{
							db.execDMLEx(_T("Update Main Set stickyClipGroupOrder = %d where lID = %d"), order, qSticky.getIntField(_T("lID")));
						}
						else
						{
							db.execDMLEx(_T("Update Main Set stickyClipOrder = %d where lID = %d"), order, qSticky.getIntField(_T("lID")));
						}

						qSticky.nextRow();
						order--;
					}
				}				

				ReOrderStickyClips(qGroup.getIntField(_T("lID")), db);

				qGroup.nextRow();
			}
		}

		Log(StrF(_T("End of ReOrderStickyClips, ParentId %d"), parentID));
	}
	CATCH_SQLITE_EXCEPTION
}

BOOL ValidDB(CString csPath, BOOL bUpgrade)
{
	CDittoPopupWindow *popUpMsg = NULL;

	try
	{
		BOOL didBackup = FALSE;
		CString backupFilePrefix = _T("Before_Update_To");

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
			if(didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

 			e.errorCode();
 		}

		//This was added later so try to add each time and catch the exception here
		try
		{
			db.execQuery(_T("SELECT lID, lClipID, lCopyBuffer FROM CopyBuffers"));
		}
		catch(CppSQLite3Exception& e)
		{
			if(didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

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
			if(didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

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

		try
		{
			db.execDML(_T("CREATE INDEX Main_ParentId on Main(lParentID DESC)"));
			db.execDML(_T("CREATE INDEX Main_IsGroup on Main(bIsGroup DESC)"));
			db.execDML(_T("CREATE INDEX Main_ShortCut on Main(lShortCut DESC)"));
		}
		catch(CppSQLite3Exception& e)
		{
			e.errorCode();
		}

		try
		{
			db.execQuery(_T("SELECT clipOrder, clipGroupOrder FROM Main"));
		}
		catch(CppSQLite3Exception& e)
		{
			if(didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

			db.execDML(_T("ALTER TABLE Main ADD clipOrder REAL"));
			db.execDML(_T("ALTER TABLE Main ADD clipGroupOrder REAL"));

			db.execDML(_T("Update Main set clipOrder = lDate, clipGroupOrder = lDate"));

			db.execDML(_T("CREATE INDEX Main_ClipOrder on Main(clipOrder DESC)"));
			db.execDML(_T("CREATE INDEX Main_ClipGroupOrder on Main(clipGroupOrder DESC)"));

			db.execDML(_T("DROP INDEX Main_Date"));

			e.errorCode();
		}

		try
		{
			db.execQuery(_T("SELECT globalShortCut FROM Main"));
		}
		catch(CppSQLite3Exception& e)
		{
			if(didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);


			db.execDML(_T("ALTER TABLE Main ADD globalShortCut INTEGER"));

			e.errorCode();
		}

		try
		{
			db.execQuery(_T("SELECT lastPasteDate FROM Main"));
		}
		catch(CppSQLite3Exception& e)
		{
			if(didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

			db.execDML(_T("ALTER TABLE Main ADD lastPasteDate INTEGER"));
			db.execDML(_T("Update Main set lastPasteDate = lDate"));
			db.execDMLEx(_T("Update Main set lastPasteDate = %d where lastPasteDate <= 0"), (int)CTime::GetCurrentTime().GetTime());

			e.errorCode();
		}

		try
		{
			db.execQuery(_T("SELECT stickyClipOrder FROM Main"));
		}
		catch (CppSQLite3Exception& e)
		{
			if (didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

			db.execDML(_T("ALTER TABLE Main ADD stickyClipOrder REAL"));
			db.execDML(_T("ALTER TABLE Main ADD stickyClipGroupOrder REAL"));

			e.errorCode();
		}

		try
		{			
			CppSQLite3Query q = db.execQuery(_T("PRAGMA index_info(Main_NoGroup);"));
			int count = 0;
			while (q.eof() == false)
			{
				count++;
				q.nextRow();
			}

			if(count == 0)
			{
				if (didBackup == FALSE)
					didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

				db.execDML(_T("Update Main set stickyClipOrder = -(2147483647) where stickyClipOrder IS NULL;"));
				db.execDML(_T("Update Main set stickyClipGroupOrder = -(2147483647) where stickyClipGroupOrder IS NULL;"));
				db.execDML(_T("Update Main set stickyClipOrder = -(2147483647) where stickyClipOrder = 0;"));
				db.execDML(_T("Update Main set stickyClipGroupOrder = -(2147483647) where stickyClipGroupOrder = 0;"));

				db.execDML(_T("CREATE INDEX Main_NoGroup ON Main(bIsGroup ASC, stickyClipOrder DESC, clipOrder DESC);"));
				db.execDML(_T("CREATE INDEX Main_InGroup ON Main(lParentId ASC, bIsGroup ASC, stickyClipGroupOrder DESC, clipGroupOrder DESC);"));
				db.execDML(_T("CREATE INDEX Data_ParentId_Format ON Data(lParentID COLLATE BINARY ASC, strClipBoardFormat COLLATE NOCASE ASC);"));
			}
		}
		catch (CppSQLite3Exception& e)
		{
			if (didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

			e.errorCode();
		}

		try
		{
			db.execQuery(_T("SELECT MoveToGroupShortCut FROM Main"));
			db.execQuery(_T("SELECT GlobalMoveToGroupShortCut FROM Main"));			
		}
		catch(CppSQLite3Exception& e)
		{
			if(didBackup == FALSE)
				didBackup = BackupDB(csPath, backupFilePrefix, &popUpMsg);

			db.execDML(_T("ALTER TABLE Main ADD MoveToGroupShortCut INTEGER"));
			db.execDML(_T("ALTER TABLE Main ADD GlobalMoveToGroupShortCut INTEGER"));			

			e.errorCode();
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)

	

	if(popUpMsg != NULL &&
		IsWindow(popUpMsg->m_hWnd))
	{
		popUpMsg->CloseWindow();
		popUpMsg->DestroyWindow();
		delete popUpMsg;
		popUpMsg = NULL;
	}
	return TRUE;                                                     
}



BOOL BackupDB(CString dbPath, CString prefix, CDittoPopupWindow **popUpMsg)
{
	if ((*popUpMsg) == NULL)
	{
		CRect r;
		GetMonitorRect(0, r);
		*popUpMsg = new CDittoPopupWindow();
		(*popUpMsg)->Create(CRect(r.right - 400, r.bottom - 130, r.right - 10, r.bottom - 10), NULL);		
		::SetWindowPos((*popUpMsg)->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
		(*popUpMsg)->ShowWindow(SW_SHOW);
		(*popUpMsg)->UpdateText(_T("Backing up Ditto's Database"));
	}
	CString backup = GetFilePath(dbPath);

	CInternetUpdate update;

	long runningVersion = update.GetRunningVersion();
	CString versionString = update.GetVersionString(runningVersion);

	backup += GetFileName(dbPath) += _T("_") + prefix + _T("_") + versionString;
	backup.Replace(_T(".db"), _T(""));
	backup.Replace(_T("."), _T("_"));

	CString temp = backup;

	temp += _T(".db");

	int i = 1;
	while(FileExists(temp))
	{
		temp.Format(_T("%s_%d.db"), backup, i);
		i++;
	}

	backup = temp;

	BOOL ret = FALSE;

	try
	{
		CFile file;
		CFileException ex;
		if(file.Open(dbPath, CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone, &ex))
		{
			ULONGLONG fileSize = file.GetLength();
			ULONGLONG totalReadSize = 0;
			int percentageComplete = 0;
			UINT readBytes = 0;
			char *pBuffer = new char[65536];
			if(pBuffer != NULL)
			{
				CFile writeFile;
				if(writeFile.Open(backup, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone, &ex))
				{
					do
					{
						readBytes = file.Read(pBuffer, 65536);
						writeFile.Write(pBuffer, readBytes);
						totalReadSize+= readBytes;

						int percent = ((totalReadSize * 100) / fileSize);
						if(percent != percentageComplete)
						{
							percentageComplete = percent;
							(*popUpMsg)->SetProgressBarPercent(percentageComplete);
						}

					}while(readBytes >= 65536);

					writeFile.Close();

					ret = TRUE;
				}
			}

			file.Close();
		}
	}
	catch(...)
	{

	}
	//BOOL ret = CopyFile(dbPath, backup, TRUE);

	if ((*popUpMsg) != NULL)
	{
		(*popUpMsg)->HideProgressBar();
		(*popUpMsg)->UpdateText(_T("Running Ditto database scripts ..."));
	}

	return ret;
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
								_T("QuickPasteText TEXT, ")
								_T("clipOrder REAL, ")
								_T("clipGroupOrder REAL, ")
								_T("globalShortCut INTEGER, ")
								_T("lastPasteDate INTEGER, ")
								_T("stickyClipOrder REAL, ")
								_T("stickyClipGroupOrder REAL, ")
								_T("MoveToGroupShortCut INTEGER, ")
								_T("GlobalMoveToGroupShortCut INTEGER);"));

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
		db.execDML(_T("CREATE INDEX Main_ClipOrder on Main(clipOrder DESC)"));
		db.execDML(_T("CREATE INDEX Main_ClipGroupOrder on Main(clipGroupOrder DESC)"));
		db.execDML(_T("CREATE INDEX Main_ParentId on Main(lParentID DESC)"));
		db.execDML(_T("CREATE INDEX Main_IsGroup on Main(bIsGroup DESC)"));
        db.execDML(_T("CREATE INDEX Main_ShortCut on Main(lShortCut DESC)"));

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

		db.execDML(_T("CREATE INDEX Main_NoGroup ON Main(bIsGroup ASC, stickyClipOrder DESC, clipOrder DESC);"));
		db.execDML(_T("CREATE INDEX Main_InGroup ON Main(lParentId ASC, bIsGroup ASC, stickyClipGroupOrder DESC, clipGroupOrder DESC);"));
		db.execDML(_T("CREATE INDEX Data_ParentId_Format ON Data(lParentID COLLATE BINARY ASC, strClipBoardFormat COLLATE NOCASE ASC);"));

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

BOOL RemoveOldEntries(bool checkIdleTime)
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
				int clipId;
				
				CppSQLite3Query q = db.execQueryEx(_T("SELECT lID, lShortCut, lParentID, lDontAutoDelete, stickyClipOrder, stickyClipGroupOrder FROM Main WHERE bIsGroup = 0 ORDER BY clipOrder DESC LIMIT -1 OFFSET %d"), lMax);
				while(q.eof() == false)
				{
					int shortcut = q.getIntField(_T("lShortCut"));
					int dontDelete = q.getIntField(_T("lDontAutoDelete"));
					int parentId = q.getIntField(_T("lParentID"));
					double stickyClipOrder = q.getFloatField(_T("stickyClipOrder"));
					double stickyClipGroupOrder = q.getFloatField(_T("stickyClipGroupOrder"));

					//Only delete entries that have no shortcut and don't have the flag set and aren't in groups and 
					if(shortcut == 0 && 
						dontDelete == 0 &&
						parentId <= 0 &&
						stickyClipOrder == -(2147483647) &&
						stickyClipGroupOrder == -(2147483647))
					{
						clipId = q.getIntField(_T("lID"));
						IDs.Add(clipId);
						Log(StrF(_T("From MaxEntries - Deleting Id: %d"), clipId));
					}

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
													_T("WHERE lastPasteDate < %d AND ")
													_T("bIsGroup = 0 AND lShortCut = 0 AND lParentID <= 0 AND lDontAutoDelete = 0 AND stickyClipOrder = -(2147483647) AND stickyClipGroupOrder = -(2147483647)"), (int)now.GetTime());

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

		int toDeleteCount = db.execScalar(_T("SELECT COUNT(clipID) FROM MainDeletes"));

		Log(StrF(_T("Before Deleting emptied out data, count: %d, Idle Seconds: %f"), toDeleteCount, IdleSeconds()));

		//Only delete 1 at a time, was finding that it was taking a long time to delete clips, locking the db and causing other queries
		//to lock up
		CppSQLite3Query q = db.execQueryEx(_T("SELECT * FROM MainDeletes LIMIT %d"), CGetSetOptions::GetMainDeletesDeleteCount());
		int deleteCount = 0;

		while(q.eof() == false)
		{
			double idleSeconds = IdleSeconds();
			if(checkIdleTime == false || idleSeconds > CGetSetOptions::GetIdleSecondsBeforeDelete())
			{
				//delete any data items sitting out there that the main table data was deleted
				//this was done to speed up deleted from the main table
				deleteCount = db.execDMLEx(_T("DELETE FROM MainDeletes WHERE clipID=%d"), q.getIntField(_T("clipID")));
			}
			else
			{
				Log(StrF(_T("Computer has not been idle long enough to delete clips, Min Idle: %d, current Idle: %d"), 
												CGetSetOptions::GetIdleSecondsBeforeDelete(), idleSeconds));

				break;
			}
			q.nextRow();
		}		

		toDeleteCount = db.execScalar(_T("SELECT COUNT(clipID) FROM MainDeletes"));

		Log(StrF(_T("After Deleting emptied out data rows, Count: %d, toDelete: %d"), deleteCount, toDeleteCount));
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