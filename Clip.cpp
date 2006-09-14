// ProcessCopy.cpp: implementation of the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "Clip.h"
#include "DatabaseUtilities.h"
#include "Crc32Dynamic.h"
#include "sqlite\CppSQLite3.h"
#include "TextConvert.h"
#include "zlib/zlib.h"

#include <Mmsystem.h>

#include "Path.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/*----------------------------------------------------------------------------*\
COleDataObjectEx
\*----------------------------------------------------------------------------*/

HGLOBAL COleDataObjectEx::GetGlobalData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc)
{
	HGLOBAL hGlobal = ::GetClipboardData(cfFormat);
    hGlobal = COleDataObject::GetGlobalData(cfFormat, lpFormatEtc);
	if(hGlobal)
	{
		if(!::IsValid(hGlobal))
		{
			Log( StrF(
				_T("COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned."),
				GetFormatName(cfFormat) ) );
			::GlobalFree( hGlobal );
			hGlobal = NULL;
		}
		return hGlobal;
	}
	
	// The data isn't in global memory, so try getting an IStream interface to it.
	STGMEDIUM stg;
	
	if(!GetData(cfFormat, &stg))
	{
		return 0;
	}
	
	switch(stg.tymed)
	{
	case TYMED_HGLOBAL:
		hGlobal = stg.hGlobal;
		break;
		
	case TYMED_ISTREAM:
		{
			UINT            uDataSize;
			LARGE_INTEGER	li;
			ULARGE_INTEGER	uli;
			
			li.HighPart = li.LowPart = 0;
			
			if ( SUCCEEDED( stg.pstm->Seek ( li, STREAM_SEEK_END, &uli )))
			{
				hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, uli.LowPart );
				
				void* pv = GlobalLock(hGlobal);
				stg.pstm->Seek(li, STREAM_SEEK_SET, NULL);
				HRESULT result = stg.pstm->Read(pv, uli.LowPart, (PULONG)&uDataSize);
				GlobalUnlock(hGlobal);
				
				if( FAILED(result) )
					hGlobal = GlobalFree(hGlobal);
			}
			break;  // case TYMED_ISTREAM
		}
	} // end switch
	
	ReleaseStgMedium(&stg);
	
	if(hGlobal && !::IsValid(hGlobal))
	{
		Log( StrF(
			_T("COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned."),
			GetFormatName(cfFormat)));
		::GlobalFree(hGlobal);
		hGlobal = NULL;
	}
	
	return hGlobal;
}

/*----------------------------------------------------------------------------*\
CClipFormat - holds the data of one clip format.
\*----------------------------------------------------------------------------*/
CClipFormat::CClipFormat(CLIPFORMAT cfType, HGLOBAL hgData, long lDBID)
{
	m_cfType = cfType;
	m_hgData = hgData;
	bDeleteData = true;
	m_lDBID = lDBID;
}

CClipFormat::~CClipFormat() 
{ 
	Free(); 
}

void CClipFormat::Clear()
{
	m_cfType = 0;
	m_hgData = 0;
	m_lDBID = -1;
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
	int count = GetSize();

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

	int nCount = clip.m_Formats.GetSize();
	
	for(int i = 0; i < nCount; i++)
	{
		pCF = &clip.m_Formats.GetData()[i];

		LPVOID pvData = GlobalLock(pCF->m_hgData);
		if(pvData)
		{
			AddFormat(pCF->m_cfType, pvData, GlobalSize(pCF->m_hgData));
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
	for(int i = m_Formats.GetSize()-1; i >= 0; i--)
	{
		m_Formats[i].Free();
		m_Formats.RemoveAt( i );
	}
}

// Adds a new Format to this Clip by copying the given data.
bool CClip::AddFormat(CLIPFORMAT cfType, void* pData, UINT nLen)
{
	ASSERT(pData && nLen);
	HGLOBAL hGlobal = ::NewGlobalP(pData, nLen);
	ASSERT(hGlobal);

	// update the Clip statistics
	m_Time = m_Time.GetCurrentTime();

	if(!SetDescFromText(hGlobal))
		SetDescFromType();
	
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

// Fills this CClip with the contents of the clipboard.
bool CClip::LoadFromClipboard(CClipTypes* pClipTypes)
{
	COleDataObjectEx oleData;
	CClipTypes defaultTypes;
	CClipTypes* pTypes = pClipTypes;

	// m_Formats should be empty when this is called.
	ASSERT(m_Formats.GetSize() == 0);
	
	// If the data is supposed to be private, then return
	if(::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
	{
		return false;
	}

	//If we are saving a multi paste then delay us connecting to the clipboard
	//to allow the ctrl-v to do a paste
	if(::IsClipboardFormatAvailable(theApp.m_cfDelaySavingData))
	{
		Sleep(1500);
	}
		
	//Attach to the clipboard
	if(!oleData.AttachClipboard())
	{
		ASSERT(0); // does this ever happen?
		return false;
	}
	
	oleData.EnsureClipboardObject();
	
	// if no types were given, get only the first (most important) type.
	//  (subsequent types could be synthetic due to automatic type conversions)
	if(pTypes == NULL || pTypes->GetSize() == 0)
	{
		ASSERT(0); // this feature is not currently used... it is an error if it is.
		
		FORMATETC formatEtc;
		oleData.BeginEnumFormats();
		oleData.GetNextFormat(&formatEtc);
		defaultTypes.Add(formatEtc.cfFormat);
		pTypes = &defaultTypes;
	}
	
	m_Desc = "[Ditto Error] BAD DESCRIPTION";
	
	// Get Description String
	// NOTE: We make sure that the description always corresponds to the
	//  data saved by using the exact same globalmem instance as the source
	//  for both... i.e. we only fetch the description format type once.
	CClipFormat cfDesc;
	bool bIsDescSet = false;
#ifdef _UNICODE
	cfDesc.m_cfType = CF_UNICODETEXT;	
#else
	cfDesc.m_cfType = CF_TEXT;	
#endif

	if(oleData.IsDataAvailable(cfDesc.m_cfType))
	{
		cfDesc.m_hgData = oleData.GetGlobalData(cfDesc.m_cfType);
		bIsDescSet = SetDescFromText(cfDesc.m_hgData);
	}
	
	// Get global data for each supported type on the clipboard
	UINT nSize;
	CClipFormat cf;
	int numTypes = pTypes->GetSize();
	for(int i = 0; i < numTypes; i++)
	{
		cf.m_cfType = pTypes->ElementAt(i);
		
		// is this the description we already fetched?
		if(cf.m_cfType == cfDesc.m_cfType)
		{
			cf = cfDesc;
			cfDesc.m_hgData = 0; // cf owns it now (to go into m_Formats)
		}
		else if(!oleData.IsDataAvailable(cf.m_cfType))
		{
			continue;
		}
		else
		{
			cf.m_hgData = oleData.GetGlobalData(cf.m_cfType);
		}
		
		if(cf.m_hgData)
		{
			nSize = GlobalSize(cf.m_hgData);
			if(nSize > 0)
			{
				if(g_Opt.m_lMaxClipSizeInBytes > 0 && (int)nSize > g_Opt.m_lMaxClipSizeInBytes)
				{
					CString cs;
					cs.Format(_T("Maximum clip size reached max size = %d, clip size = %d"), g_Opt.m_lMaxClipSizeInBytes, nSize);
					Log(cs);

					oleData.Release();
					return false;
				}

				ASSERT(::IsValid(cf.m_hgData));
				
				m_Formats.Add(cf);
			}
			else
			{
				ASSERT(FALSE); // a valid GlobalMem with 0 size is strange
				cf.Free();
			}
			cf.m_hgData = 0; // m_Formats owns it now
		}
	}
	
	m_Time = CTime::GetCurrentTime();
	
	if(!bIsDescSet)
	{
		SetDescFromType();
	}
	
	// if the description was in a type that is not supported,
	//we have to free it since it wasn't added to m_Formats
	if(cfDesc.m_hgData)
	{
		cfDesc.Free();
	}
	
	oleData.Release();
	
	if(m_Formats.GetSize() == 0)
	{
		return false;
	}

	return true;
}

bool CClip::SetDescFromText(HGLOBAL hgData)
{
	if(hgData == 0)
		return false;
	
	bool bRet = false;
	TCHAR* text = (TCHAR *) GlobalLock(hgData);
	long ulBufLen = GlobalSize(hgData);
	
	m_Desc = text;
	bRet = true;
		
	if(ulBufLen > g_Opt.m_bDescTextSize)
	{
		m_Desc = m_Desc.Left(g_Opt.m_bDescTextSize);
	}
	
	//Unlock the data
	GlobalUnlock(hgData);
	
	return bRet;
}

bool CClip::SetDescFromType()
{
	int nSize = m_Formats.GetSize();
	if(nSize <= 0)
	{
		return false;
	}

	int nCF_HDROPIndex = -1;
	for(int i = 0; i < nSize; i++)
	{
		if(m_Formats[i].m_cfType == CF_HDROP)
		{
			nCF_HDROPIndex = i;
		}
	}

	if(nCF_HDROPIndex >= 0)
	{
		using namespace nsPath;

		HDROP drop = (HDROP)GlobalLock(m_Formats[nCF_HDROPIndex].m_hgData);
		int nNumFiles = min(5, DragQueryFile(drop, -1, NULL, 0));

		if(nNumFiles > 1)
			m_Desc = "Copied Files - ";
		else
			m_Desc = "Copied File - ";

		TCHAR file[MAX_PATH];
		
		for(int nFile = 0; nFile < nNumFiles; nFile++)
		{
			if(DragQueryFile(drop, nFile, file, sizeof(file)) > 0)
			{
				CPath path(file);
				m_Desc += path.GetName();
				m_Desc += " - ";
				m_Desc += file;
				m_Desc += "\n";
			}
		}
		
		GlobalUnlock(m_Formats[nCF_HDROPIndex].m_hgData);
	}
	else
	{
		m_Desc = GetFormatName(m_Formats[0].m_cfType);
	}

	return m_Desc.GetLength() > 0;
}

bool CClip::AddToDB(bool bCheckForDuplicates)
{
	bool bResult;
	try
	{
		m_CRC = GenerateCRC();

		if(bCheckForDuplicates)
		{	
			int nID = FindDuplicate();
			if(nID >= 0)
			{
				m_csQuickPaste.Replace(_T("'"), _T("''"));

				if(m_csQuickPaste.IsEmpty())
				{
					theApp.m_db.execDMLEx(_T("UPDATE Main SET lDate = %d where lID = %d;"), 
											(long)m_Time.GetTime(), nID);
				}
				else
				{		
					theApp.m_db.execDMLEx(_T("UPDATE Main SET lDate = %d, QuickPasteText = '%s' where lID = %d;"), 
											(long)m_Time.GetTime(), m_csQuickPaste, nID);
				}
				
				EmptyFormats();

				return true;
			}
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
	
	bResult = false;
	if(AddToMainTable())
	{
		bResult = AddToDataTable();
	}

	if(bResult)
	{
		if(g_Opt.m_csPlaySoundOnCopy.IsEmpty() == FALSE)
			PlaySound(g_Opt.m_csPlaySoundOnCopy, NULL, SND_FILENAME|SND_ASYNC);
	}
	
	// should be emptied by AddToDataTable
	ASSERT(m_Formats.GetSize() == 0);
	
	return bResult;
}

// if a duplicate exists, set recset to the duplicate and return true
int CClip::FindDuplicate()
{
	try
	{
		//If they are allowing duplicates still check 
		//the last copied item
		if(g_Opt.m_bAllowDuplicates)
		{
			if(m_CRC == m_LastAddedCRC)
				return m_LastAddedID;
		}
		else
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Main WHERE CRC = %d"), m_CRC);
				
			if(q.eof() == false)
			{
				return q.getIntField(_T("lID"));
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return -1;
}

DWORD CClip::GenerateCRC()
{
	CClipFormat* pCF;
	DWORD dwCRC = 0xFFFFFFFF;

	CCrc32Dynamic *pCrc32 = new CCrc32Dynamic;
	if(pCrc32)
	{
		//Generate a CRC value for all copied data

		int nSize = m_Formats.GetSize();
		for(int i = 0; i < nSize ; i++)
		{
			pCF = & m_Formats.ElementAt(i);
			
			const unsigned char *Data = (const unsigned char *)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				pCrc32->GenerateCrc32((const LPBYTE)Data, GlobalSize(pCF->m_hgData), dwCRC);
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

bool CClip::ModifyMainTable()
{
	bool bRet = false;
	try
	{
		m_Desc.Replace(_T("'"), _T("''"));
		m_csQuickPaste.Replace(_T("'"), _T("''"));

		theApp.m_db.execDMLEx(_T("UPDATE Main SET lShortCut = %d, ")
			_T("mText = '%s', ")
			_T("lParentID = %d, ")
			_T("lDontAutoDelete = %d, ")
			_T("QuickPasteText = '%s' ")
			_T("WHERE lID = %d;"), 
			m_lShortCut, 
			m_Desc, 
			m_lParent, 
			m_lDontAutoDelete, 
			m_csQuickPaste,
			m_ID);

		bRet = true;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)

	return bRet;
}

// Empties m_Formats as it saves them to the Data Table.
bool CClip::AddToDataTable()
{
	CClipFormat* pCF;

	try
	{
		CppSQLite3Statement stmt = theApp.m_db.compileStatement(_T("insert into Data values (NULL, ?, ?, ?);"));
		
		for(int i = m_Formats.GetSize()-1; i >= 0 ; i--)
		{
			pCF = & m_Formats.ElementAt(i);
			
			stmt.bind(1, m_ID);
			stmt.bind(2, GetFormatName(pCF->m_cfType));

			const unsigned char *Data = (const unsigned char *)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				stmt.bind(3, Data, GlobalSize(pCF->m_hgData));
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
			m_csQuickPaste = q.getStringField(_T("QuickPasteText"));
			m_ID = lID;
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
			_T("SELECT Data.lID, strClipBoardFormat, ooData FROM Data ")
			_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
			_T("WHERE Main.lID = %d ORDER BY Data.lID desc"), lID);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

		while(q.eof() == false)
		{
			cf.m_lDBID = q.getIntField(_T("lID"));
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
			const unsigned char *cData = q.getBlobField(_T("ooData"), nDataLen);
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

bool CClip::SaveFromEditWnd(BOOL bUpdateDesc)
{
	bool bRet = false;

	try
	{
		theApp.m_db.execDMLEx(_T("DELETE FROM Data WHERE lParentID = %d;"), m_ID);

		DWORD CRC = GenerateCRC();

		AddToDataTable();

		theApp.m_db.execDMLEx(_T("UPDATE Main SET CRC = %d WHERE lID = %d"), CRC, m_ID);
		
		if(bUpdateDesc)
		{
			m_Desc.Replace(_T("'"), _T("''"));
			theApp.m_db.execDMLEx(_T("UPDATE Main SET mText = '%s' WHERE lID = %d"), m_Desc, m_ID);
		}

		bRet = true;
	}
	CATCH_SQLITE_EXCEPTION

	return bRet;
}

/*----------------------------------------------------------------------------*\
CClipList
\*----------------------------------------------------------------------------*/

CClipList::~CClipList()
{
	CClip* pClip;
	while(GetCount())
	{
		pClip = RemoveHead();
		DELETE_PTR(pClip);
	}
}

// returns the number of clips actually saved
// while this does empty the Format Data, it does not delete the Clips.
int CClipList::AddToDB(bool bLatestTime, bool bShowStatus)
{
	int savedCount = 0;
	int nRemaining = 0;
	CClip* pClip;
	POSITION pos;
	bool bResult;
	
	nRemaining = GetCount();
	pos = GetHeadPosition();
	while(pos)
	{
		if(bShowStatus)
		{
			theApp.SetStatus(StrF(_T("%d"),nRemaining), true);
			nRemaining--;
		}
		
		pClip = GetNext(pos);
		ASSERT(pClip);
		
		if(bLatestTime)
			pClip->MakeLatestTime();
		
		bResult = pClip->AddToDB();
		if( bResult )
			savedCount++;
	}
	
	if(bShowStatus)
		theApp.SetStatus(NULL, true);
	
	return savedCount;
}

const CClipList& CClipList::operator=(const CClipList &cliplist)
{
	POSITION pos;
	CClip* pClip;
	
	pos = cliplist.GetHeadPosition();
	while(pos)
	{
		pClip = cliplist.GetNext(pos);
		ASSERT(pClip);

		CClip *pNewClip = new CClip;
		if(pNewClip)
		{
			*pNewClip = *pClip;
			
			AddTail(pNewClip);
		}
	}
	
	return *this;
}