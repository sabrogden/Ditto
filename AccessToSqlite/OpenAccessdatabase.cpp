#include "StdAfx.h"
#include "OpenAccessdatabase.h"


BOOL Upgrade_mText(CDaoDatabase& db);
BOOL Upgrade_Groups(CDaoDatabase& db);
BOOL Upgrade_ShareData(CDaoDatabase& db);
int GetFieldPos(CDaoRecordset& recs, LPCTSTR fieldName);
BOOL CreateBackup(CString csPath);

COpenAccessdatabase::COpenAccessdatabase(void)
{
}

COpenAccessdatabase::~COpenAccessdatabase(void)
{
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

BOOL COpenAccessdatabase::ValidDB(CString csPath, BOOL bUpgrade)
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
				if(STRCMP(szErrorMessage, _T("Unable to initialize DAO/Jet db engine.")) == 0)
				{
					e->Delete();
					return ERROR_OPENING_DATABASE;				
				}
			}
			e->ReportError();
			e->Delete();

			return FALSE;
		}


		CDaoTableDef table(&db);
		CDaoFieldInfo info;

		table.Open(_T("Main"));
		table.GetFieldInfo(_T("lID"), info);
		table.GetFieldInfo(_T("lDate"), info);
		ON_FIELD_ABSENT(_T("mText"), Upgrade_mText(db)); // +mText, -strText, -strType
		table.GetFieldInfo(_T("lShortCut"), info);
		table.GetFieldInfo(_T("lDontAutoDelete"), info);
		table.GetFieldInfo(_T("lTotalCopySize"), info);
		ON_FIELD_ABSENT(_T("bIsGroup"), Upgrade_Groups(db));
		table.GetFieldInfo(_T("lParentID"), info); // part of Upgrade_Groups
		table.GetFieldInfo(_T("dOrder"), info);  // part of Upgrade_Groups
		ON_FIELD_ABSENT(_T("lDataID"), Upgrade_ShareData(db)); // +lDataID, -lParentID
		table.Close();

		table.Open(_T("Data"));
		table.GetFieldInfo(_T("lID"), info);
		table.GetFieldInfo(_T("lDataID"), info); // part of Upgrade_ShareData()
		table.GetFieldInfo(_T("strClipBoardFormat"), info);
		table.GetFieldInfo(_T("ooData"), info);
		table.Close();

		table.Open(_T("Types"));
		table.GetFieldInfo(_T("ID"), info);
		table.GetFieldInfo(_T("TypeText"), info);
		table.Close();
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return FALSE;
	}

	// if we upgraded, perform full validation again without upgrading
	if( bUpgraded )
		return ValidDB(csPath, FALSE);

	return bResult;
}

// +mText, -strText, -strType
BOOL Upgrade_mText(CDaoDatabase& db)
{
	try
	{
		db.Execute(_T("ALTER TABLE Main ADD COLUMN mText MEMO"), dbFailOnError);
		db.Execute(_T("UPDATE Main SET mText=strText"), dbFailOnError);
		db.Execute(_T("ALTER TABLE Main DROP COLUMN strText"), dbFailOnError);
		db.Execute(_T("ALTER TABLE Main DROP COLUMN strType"), dbFailOnError);
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return FALSE;
	}
	
	return TRUE;
}

BOOL Upgrade_Groups(CDaoDatabase& db)
{
	try
	{
		CDaoTableDefEx table(&db);
		table.Open(_T("Main"));
		// Groups
		table.CreateField(_T("bIsGroup"), dbBoolean, 1, 0, _T("0")); // for Groups
		table.CreateIndex(FALSE, _T("bIsGroup"));
		table.CreateField(_T("lParentID"), dbLong, 4, 0, _T("0")); // parent Group Main.lID
		table.CreateIndex(FALSE, _T("lParentID"));
		table.CreateField(_T("dOrder"), dbDouble, 8, 0, _T("0")); // for Order within Groups
		table.CreateIndex(FALSE, _T("dOrder"));
		table.Close();
		// set defaults (otherwise might be NULL)
		db.Execute(_T("UPDATE Main SET bIsGroup = 0, lParentID = 0, dOrder = 0"), dbFailOnError);
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return FALSE;
	}
	
	return TRUE;
}

BOOL Upgrade_ShareData(CDaoDatabase& db)
{
	try
	{
		CDaoTableDefEx table(&db);

		table.Open(_T("Main"));
		table.CreateField(_T("lDataID"), dbLong, 4, 0, _T("0")); // corresponds to Data.lDataID
		table.CreateIndex(FALSE, _T("lDataID"));
		table.Close();

		table.Open(_T("Data"));
		table.CreateField(_T("lDataID"), dbLong, 4, 0, _T("0")); // parent Group Main.lID
		table.CreateIndex(FALSE, _T("lDataID"));
		table.Close();

		// set defaults
		db.Execute(_T("UPDATE Main SET lDataID = 0"), dbFailOnError);
		db.Execute(_T("UPDATE Data SET lDataID = 0"), dbFailOnError);

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

		main.Open(dbOpenDynaset, _T("SELECT lID, lDataID FROM Main"));

		main_fldID = GetFieldPos(main, _T("lID"));
		VERIFY(main_fldID == 0);
		main_fldDataID = GetFieldPos(main, _T("lDataID"));
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

			main.GetFieldValue(main_fldID,var);
			main_lID = var.lVal;

			CString cs;
			cs.Format(_T("SELECT lID, lDataID FROM Data WHERE lParentID = %d"), main_lID);

			data.Open(dbOpenDynaset, cs);

			data_fldID = GetFieldPos(data, _T("lID"));
			VERIFY(data_fldID == 0);
			data_fldDataID = GetFieldPos(data, _T("lDataID"));
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
		db.Execute(_T("ALTER TABLE Data DROP CONSTRAINT lParentID"), dbFailOnError);
		db.Execute(_T("ALTER TABLE Data DROP COLUMN lParentID"), dbFailOnError);
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
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

BOOL CreateBackup(CString csPath)
{
	CString csOriginal;
	int count = 0;
	// create a backup of the existing database
	do
	{
		count++;
		csOriginal.Format(_T("%s.%03d"), csPath, count);
		// in case of some weird infinite loop
		if( count > 50 )
		{
			ASSERT(0);
			return FALSE;
		}
	} while( !::CopyFile(csPath, csOriginal, TRUE) );

	return TRUE;
}
