// ProcessCopy.cpp: implementation of the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AccessToSqlite.h"
#include "..\UnicodeMacros.h"
#include "Clip.h"
#include "..\Crc32Dynamic.h"

#include <Mmsystem.h>

#define CATCH_SQLITE_EXCEPTION		\
	catch (CppSQLite3Exception& e)	\
{								\
	Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));	\
	ASSERT(!e.errorMessage());	\
}								\

#define CATCH_SQLITE_EXCEPTION_AND_RETURN(bRet)		\
	catch (CppSQLite3Exception& e)	\
{								\
	Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));	\
	ASSERT(!e.errorMessage());	\
	return bRet;				\
}								\

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/*----------------------------------------------------------------------------*\
CClipFormat - holds the data of one clip format.
\*----------------------------------------------------------------------------*/
CClipFormat::CClipFormat(CLIPFORMAT cfType, HGLOBAL hgData)
{
	m_cfType = cfType;
	m_hgData = hgData;
	bDeleteData = true;
}

CClipFormat::~CClipFormat() 
{ 
	Free(); 
}

void CClipFormat::Clear()
{
	m_cfType = 0;
	m_hgData = 0;
}

void CClipFormat::Free()
{
	if(bDeleteData)
	{
		if(m_hgData)
		{
			m_hgData = ::GlobalFree( m_hgData );
			m_hgData = NULL;
		}
	}
}



/*----------------------------------------------------------------------------*\
CClipFormats - holds an array of CClipFormat
\*----------------------------------------------------------------------------*/
// returns a pointer to the CClipFormat in this array which matches the given type
//  or NULL if that type doesn't exist in this array.
CClipFormat* CClipFormats::FindFormat(UINT cfType)
{
	CClipFormat* pCF;
	int count = (int)GetSize();

	for(int i=0; i < count; i++)
	{
		pCF = &ElementAt(i);
		if(pCF->m_cfType == cfType)
			return pCF;
	}
	return NULL;
}


/*----------------------------------------------------------------------------*\
CClip - holds multiple CClipFormats and CopyClipboard() statistics
\*----------------------------------------------------------------------------*/

DWORD CClip::m_LastAddedCRC = 0;
long CClip::m_LastAddedID = -1;

CClip::CClip() : 
	m_ID(0), 
	m_CRC(0),
	m_lParent(-1),
	m_lDontAutoDelete(FALSE),
	m_lShortCut(0),
	m_bIsGroup(FALSE)
{
}

CClip::~CClip()
{
	EmptyFormats();
}

void CClip::Clear()
{
	m_ID = -1;
	m_Time = 0;
	m_Desc = "";
	m_CRC = 0;
	m_lParent = -1;
	m_lDontAutoDelete = FALSE;
	m_lShortCut = 0;
	m_bIsGroup = FALSE;
	m_csQuickPaste = "";
	EmptyFormats();
}

const CClip& CClip::operator=(const CClip &clip)
{
	const CClipFormat* pCF;

	m_ID = clip.m_ID;
	m_Time = clip.m_Time;
	m_CRC = clip.m_CRC;
	m_lParent = clip.m_lParent;
	m_lDontAutoDelete = clip.m_lDontAutoDelete;
	m_lShortCut = clip.m_lShortCut;
	m_bIsGroup = clip.m_bIsGroup;
	m_csQuickPaste = clip.m_csQuickPaste;

	int nCount = (int)clip.m_Formats.GetSize();
	
	for(int i = 0; i < nCount; i++)
	{
		pCF = &clip.m_Formats.GetData()[i];

		LPVOID pvData = GlobalLock(pCF->m_hgData);
		if(pvData)
		{
			AddFormat(pCF->m_cfType, pvData, (UINT)GlobalSize(pCF->m_hgData));
		}
		GlobalUnlock(pCF->m_hgData);
	}

	//Set this after since in could get the wrong description in AddFormat
	m_Desc = clip.m_Desc;

	return *this;
}

void CClip::EmptyFormats()
{
	// free global memory in m_Formats
	for(int i = (int)m_Formats.GetSize()-1; i >= 0; i--)
	{
		m_Formats[i].Free();
		m_Formats.RemoveAt( i );
	}
}

// Adds a new Format to this Clip by copying the given data.
bool CClip::AddFormat(CLIPFORMAT cfType, void* pData, UINT nLen)
{
	ASSERT(pData && nLen);
	HGLOBAL hGlobal = NewGlobalP(pData, nLen);
	ASSERT(hGlobal);

	// update the Clip statistics
	m_Time = m_Time.GetCurrentTime();
	
	CClipFormat format(cfType,hGlobal);
	CClipFormat *pFormat;
	
	pFormat = m_Formats.FindFormat(cfType);
	// if the format type already exists as part of this clip, replace the data
	if(pFormat)
	{
		pFormat->Free();
		pFormat->m_hgData = format.m_hgData;
	}
	else
	{
		m_Formats.Add(format);
	}
	
	format.m_hgData = 0; // now owned by m_Formats
	return true;
}

bool CClip::AddToDB(bool bCheckForDuplicates)
{
	bool bResult;
	
	m_CRC = GenerateCRC();
	
	bResult = false;
	if(AddToMainTable())
	{
		bResult = AddToDataTable();
	}

	// should be emptied by AddToDataTable
	ASSERT(m_Formats.GetSize() == 0);
	
	return bResult;
}

DWORD CClip::GenerateCRC()
{
	CClipFormat* pCF;
	DWORD dwCRC = 0xFFFFFFFF;

	CCrc32Dynamic *pCrc32 = new CCrc32Dynamic;
	if(pCrc32)
	{
		//Generate a CRC value for all copied data

		int nSize = (int)m_Formats.GetSize();
		for(int i = 0; i < nSize ; i++)
		{
			pCF = & m_Formats.ElementAt(i);
			
			const unsigned char *Data = (const unsigned char *)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				pCrc32->GenerateCrc32((const LPBYTE)Data, (DWORD)GlobalSize(pCF->m_hgData), dwCRC);
			}
			GlobalUnlock(pCF->m_hgData);
		}

		dwCRC = ~dwCRC;

		delete pCrc32;
	}

	return dwCRC;
}

// assigns m_ID
bool CClip::AddToMainTable()
{
	try
	{
		m_Desc.Replace(_T("'"), _T("''"));
		m_csQuickPaste.Replace(_T("'"), _T("''"));

		CString cs;
		cs.Format(_T("insert into Main values(NULL, %d, '%s', %d, %d, %d, %d, %d, '%s');"),
							(long)m_Time.GetTime(),
							m_Desc,
							m_lShortCut,
							m_lDontAutoDelete,
							m_CRC,
							m_bIsGroup,
							m_lParent,
							m_csQuickPaste);

		theApp.m_db.execDML(cs);

		m_ID = (long)theApp.m_db.lastRowId();

		m_LastAddedCRC = m_CRC;
		m_LastAddedID = m_ID;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
	
	return true;
}

// Empties m_Formats as it saves them to the Data Table.
bool CClip::AddToDataTable()
{
	CClipFormat* pCF;

	try
	{
		CppSQLite3Statement stmt = theApp.m_db.compileStatement(_T("insert into Data values (NULL, ?, ?, ?);"));
		
		for(int i = (int)m_Formats.GetSize()-1; i >= 0 ; i--)
		{
			pCF = & m_Formats.ElementAt(i);
			
			stmt.bind(1, m_ID);
			stmt.bind(2, GetFormatName(pCF->m_cfType));

			const unsigned char *Data = (const unsigned char *)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				stmt.bind(3, Data, (int)GlobalSize(pCF->m_hgData));
			}
			GlobalUnlock(pCF->m_hgData);
			
			stmt.execDML();
			stmt.reset();

			m_Formats.RemoveAt(i);
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
		
	return true;
}

// changes m_Time to be later than the latest clip entry in the db
// ensures that pClip's time is not older than the last clip added
// old times can happen on fast copies (<1 sec).
void CClip::MakeLatestTime()
{
	try
	{
		CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT lDate FROM Main ORDER BY lDate DESC LIMIT 1"));			
		if(q.eof() == false)
		{
			long lLatestDate = q.getIntField(_T("lDate"));
			if(m_Time.GetTime() <= lLatestDate)
			{
				m_Time = lLatestDate + 1;
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

BOOL CClip::LoadMainTable(long lID)
{
	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT * FROM Main WHERE lID = %d"), lID);

		if(q.eof() == false)
		{
			m_Time = q.getIntField(_T("lDate"));
			m_Desc = q.getStringField(_T("mText"));
			m_CRC = q.getIntField(_T("CRC"));
			m_lParent = q.getIntField(_T("lParentID"));
			m_lDontAutoDelete = q.getIntField(_T("lDontAutoDelete"));
			m_lShortCut = q.getIntField(_T("lShortCut"));
			m_bIsGroup = q.getIntField(_T("bIsGroup"));
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)

	return TRUE;
}

// STATICS

// Allocates a Global containing the requested Clip Format Data
HGLOBAL CClip::LoadFormat(long lID, UINT cfType)
{
	HGLOBAL hGlobal = 0;
	try
	{
		CString csSQL;
		
		csSQL.Format(
			_T("SELECT Data.ooData FROM Data ")
			_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
			_T("WHERE Main.lID = %d ")
			_T("AND Data.strClipBoardFormat = \'%s\'"),
			lID,
			GetFormatName(cfType));

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

		if(q.eof() == false)
		{
			int nDataLen = 0;
			const unsigned char *cData = q.getBlobField(0, nDataLen);
			if(cData == NULL)
			{
				return false;
			}

			hGlobal = NewGlobalP((LPVOID)cData, nDataLen);
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return hGlobal;
}

bool CClip::LoadFormats(long lID, bool bOnlyLoad_CF_TEXT)
{
	CClipFormat cf;
	HGLOBAL hGlobal = 0;
	m_Formats.RemoveAll();

	try
	{	
		//Open the data table for all that have the parent id

		//Order by Data.lID so that when generating CRC it's always in the same order as the first time
		//we generated it
		CString csSQL;
		csSQL.Format(
			_T("SELECT strClipBoardFormat, ooData FROM Data ")
			_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
			_T("WHERE Main.lID = %d ORDER BY Data.lID desc"), lID);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

		while(q.eof() == false)
		{
			cf.m_cfType = GetFormatID(q.getStringField(_T("strClipBoardFormat")));
			
			if(bOnlyLoad_CF_TEXT)
			{
				if(cf.m_cfType != CF_TEXT && cf.m_cfType != CF_UNICODETEXT)
				{
					q.nextRow();
					continue;
				}
			}

			int nDataLen = 0;
			const unsigned char *cData = q.getBlobField(1, nDataLen);
			if(cData != NULL)
			{
				hGlobal = NewGlobalP((LPVOID)cData, nDataLen);
			}
			
			cf.m_hgData = hGlobal;
			m_Formats.Add(cf);

			q.nextRow();
		}

		// formats owns all the data
		cf.m_hgData = 0;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
		
	return m_Formats.GetSize() > 0;
}

void CClip::LoadTypes(long lID, CClipTypes& types)
{
	types.RemoveAll();
	try
	{
		CString csSQL;
		// get formats for Clip "lID" (Main.lID) using the corresponding Main.lDataID
		
		//Order by Data.lID so that when generating CRC it's always in the same order as the first time
		//we generated it
		csSQL.Format(
			_T("SELECT strClipBoardFormat FROM Data ")
			_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
			_T("WHERE Main.lID = %d ORDER BY Data.lID desc"), lID);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);			

		while(q.eof() == false)
		{		
			types.Add(GetFormatID(q.getStringField(0)));
			q.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION
}