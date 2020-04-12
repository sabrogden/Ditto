// DatabaseUtilites.h: interface for the CDatabaseUtilites class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATABASEUTILITES_H__039F53EB_228F_4640_8009_3D2B1FF435D4__INCLUDED_)
#define AFX_DATABASEUTILITES_H__039F53EB_228F_4640_8009_3D2B1FF435D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DittoPopupWindow.h"
#include "sqlite/CppSQLite3.h"

#define DEFAULT_DB_NAME "Ditto.db"
#define ERROR_OPENING_DATABASE	2

BOOL CreateBackup(CString csPath);
CString GetDBName();
CString GetDefaultDBName();
BOOL OpenDatabase(CString csDB);
BOOL IsDatabaseOpen();

BOOL CheckDBExists(CString csDBPath);
BOOL ValidDB(CString csPath, BOOL bUpgrade=TRUE);
BOOL CreateDB(CString csPath);

BOOL CompactDatabase();
BOOL RepairDatabase();
BOOL RemoveOldEntries(bool checkIdleTime);

BOOL EnsureDirectory(CString csPath);

BOOL BackupDB(CString dbPath, CString backupPath);
BOOL RestoreDB(CString backupPath);

void ReOrderStickyClips(int parentID, CppSQLite3DB &db);

//BOOL CopyDownDatabase();
//BOOL CopyUpDatabase();

#endif // !defined(AFX_DATABASEUTILITES_H__039F53EB_228F_4640_8009_3D2B1FF435D4__INCLUDED_)
