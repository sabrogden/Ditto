// ProcessCopy.cpp: implementation of the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "Clip.h"
#include "DatabaseUtilities.h"

#include <Mmsystem.h>

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
    HGLOBAL hGlobal = COleDataObject::GetGlobalData(cfFormat, lpFormatEtc);
	if(hGlobal)
	{
		if(!::IsValid(hGlobal))
		{
			Log( StrF(
				"COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned.",
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
			"COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned.",
			GetFormatName(cfFormat)));
		::GlobalFree(hGlobal);
		hGlobal = NULL;
	}
	
	return hGlobal;
}

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

CClip::CClip() : 
	m_ID(0), 
	m_DataID(0), 
	m_lTotalCopySize(0)
{
}

CClip::~CClip()
{
	EmptyFormats();
}

void CClip::Clear()
{
	m_ID = 0;
	m_Time = 0;
	m_Desc = "";
	m_lTotalCopySize = 0;
	m_DataID = 0;
	EmptyFormats();
}

const CClip& CClip::operator=(const CClip &clip)
{
	const CClipFormat* pCF;

	m_ID = clip.m_ID;
	m_DataID = clip.m_DataID;
	m_Time = clip.m_Time;
	m_lTotalCopySize = clip.m_lTotalCopySize;

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
	m_lTotalCopySize += nLen;
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
	
	// reset copy stats
	m_lTotalCopySize = 0;
	m_Desc = "[Ditto Error] BAD DESCRIPTION";
	
	// Get Description String
	// NOTE: We make sure that the description always corresponds to the
	//  data saved by using the exact same globalmem instance as the source
	//  for both... i.e. we only fetch the description format type once.
	CClipFormat cfDesc;
	bool bIsDescSet = false;
	cfDesc.m_cfType = CF_TEXT;
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
				if(g_Opt.m_lMaxClipSizeInBytes > 0 && nSize > g_Opt.m_lMaxClipSizeInBytes)
				{
					CString cs;
					cs.Format("Maximum clip size reached max size = %d, clip size = %d", g_Opt.m_lMaxClipSizeInBytes, nSize);
					Log(cs);

					oleData.Release();
					return false;
				}

				ASSERT(::IsValid(cf.m_hgData));
				
				m_Formats.Add(cf);
				m_lTotalCopySize += nSize;
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
	char* text = (char *) GlobalLock(hgData);
	long ulBufLen = GlobalSize(hgData);
	
	ASSERT(text != NULL);
	
	if(ulBufLen > g_Opt.m_bDescTextSize)
	{
		ulBufLen = g_Opt.m_bDescTextSize;
	}
	
	if(ulBufLen > 0)
	{
		char* buf = m_Desc.GetBuffer(ulBufLen);
		memcpy(buf, text, ulBufLen); // in most cases, last char == null
		buf[ulBufLen-1] = '\0'; // just in case not null terminated
		m_Desc.ReleaseBuffer(); // scans for the null
		bRet = m_Desc.GetLength() > 0;
	}
	
	//Unlock the data
	GlobalUnlock(hgData);
	
	return bRet;
}

bool CClip::SetDescFromType()
{
	if(m_Formats.GetSize() <= 0)
	{
		return false;
	}

	m_Desc = GetFormatName(m_Formats[0].m_cfType);

	return m_Desc.GetLength() > 0;
}

bool CClip::AddToDB(bool bCheckForDuplicates)
{
	bool bResult;
	try
	{
		if(bCheckForDuplicates)
		{
			CMainTable recset;
			
			if(FindDuplicate(recset, g_Opt.m_bAllowDuplicates))
			{
				m_ID = recset.m_lID;
				recset.Edit();
				recset.m_lDate = (long) m_Time.GetTime(); // update the copy Time
				recset.Update();
				recset.Close();
				EmptyFormats(); // delete this clip's data from memory.
				return true;
			}
			
			if(recset.IsOpen())
				recset.Close();
		}
	}
	CATCHDAO
		
	// AddToDataTable must go first in order to assign m_DataID
	bResult = AddToDataTable() && AddToMainTable();

	if(bResult)
	{
		if(g_Opt.m_csPlaySoundOnCopy.GetLength() > 0)
			PlaySound(g_Opt.m_csPlaySoundOnCopy, NULL, SND_FILENAME|SND_ASYNC);
	}
	
	// should be emptied by AddToDataTable
	ASSERT(m_Formats.GetSize() == 0);
	
	return bResult;
}

// if a duplicate exists, set recset to the duplicate and return true
bool CClip::FindDuplicate(CMainTable& recset, BOOL bCheckLastOnly)
{
	ASSERT(m_lTotalCopySize > 0);
	try
	{
		recset.m_strSort = "lDate DESC";
		
		if(bCheckLastOnly)
		{
			recset.Open("SELECT * FROM Main");
			if(recset.IsEOF())
			{
				return false;
			}

			recset.MoveFirst();
			// if an entry exists and they are the same size and the format data matches
			if(!recset.IsBOF() && !recset.IsEOF() &&
				m_lTotalCopySize == recset.m_lTotalCopySize &&
				(CompareFormatDataTo(recset.m_lDataID) == 0))
			{	
				return true; 
			}
			return false;
		}
		
		// Look for any other entries that have the same size
		recset.Open("SELECT * FROM Main WHERE lTotalCopySize = %d", m_lTotalCopySize);
		while(!recset.IsEOF())
		{
			//if there is any then look if it is an exact match
			if(CompareFormatDataTo(recset.m_lDataID) == 0)
			{
				return true;
			}
			
			recset.MoveNext();
		}
	}
	CATCHDAO
		
	return false;
}

int CClip::CompareFormatDataTo(long lDataID)
{
	int nRet = 0;
	int nRecs=0, nFormats=0;
	CClipFormat* pFormat = NULL;
	try
	{
		CDataTable recset;
		recset.Open("SELECT * FROM Data WHERE lDataID = %d", lDataID);
		
		if( !recset.IsBOF() && !recset.IsEOF() )
		{
			// Verify the same number of saved types
			recset.MoveLast();
			nRecs = recset.GetRecordCount();
		}

		nFormats = m_Formats.GetSize();
		nRet = nFormats - nRecs;
		if(nRet != 0 || nRecs == 0)
		{	
			recset.Close();	
			return nRet; 
		}
		
		// For each format type in the db
		
		for(recset.MoveFirst(); !recset.IsEOF(); recset.MoveNext())
		{
			pFormat = m_Formats.FindFormat(GetFormatID(recset.m_strClipBoardFormat));
			
			// Verify the format exists
			if(!pFormat)
			{	
				recset.Close(); 
				return -1; 
			}
			
			// Compare the size
			nRet = ::GlobalSize(pFormat->m_hgData) - recset.m_ooData.m_dwDataLength;
			if( nRet != 0 )
			{	
				recset.Close(); 
				return nRet; 
			}
			
			// Binary compare
			nRet = CompareGlobalHH(recset.m_ooData.m_hData,	pFormat->m_hgData, recset.m_ooData.m_dwDataLength);
			if(nRet != 0)
			{	
				recset.Close(); 
				return nRet; 
			}
		}
		recset.Close();
	}
	CATCHDAO
		
	return 0;
}

// assigns m_ID
bool CClip::AddToMainTable()
{
	try
	{
		CMainTable recset;
		
		//		recset.m_strSort = "lDate DESC";
		recset.Open("SELECT * FROM Main");
		
		long lDate = (long) m_Time.GetTime();
		
		recset.AddNew();  // overridden to set m_lID to the new autoincr number
		
		m_ID = recset.m_lID;
		
		recset.m_lDate = lDate;
		recset.m_strText = m_Desc;
		recset.m_lTotalCopySize = m_lTotalCopySize;
		
		recset.m_bIsGroup = FALSE;
		recset.m_lParentID = theApp.m_GroupDefaultID;
		
		VERIFY(m_DataID > 0); // AddToDataTable must be called first to assign this
		recset.m_lDataID = m_DataID;
		
		recset.Update();
		
		//		recset.SetBookmark( recset.GetLastModifiedBookmark() );
		//		m_ID = recset.m_lID;
		
		recset.Close();
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return false;
	}
	
	return true;
}

// Empties m_Formats as it saves them to the Data Table.
bool CClip::AddToDataTable()
{
	VERIFY( m_DataID <= 0 ); // this func will assign m_DataID
	try
	{
		CClipFormat* pCF;
		CDataTable recset;
		recset.Open(dbOpenTable,"Data");
		
		for(int i = m_Formats.GetSize()-1; i >= 0 ; i--)
		{
			pCF = & m_Formats.ElementAt(i);
			
			recset.AddNew(); // overridden to assign new autoincr ID to m_lID
			
			if( m_DataID <= 0 )
			{
				VERIFY( recset.m_lID > 0 );
				m_DataID = recset.m_lID;
			}
			
			recset.m_lDataID = m_DataID;
			recset.m_strClipBoardFormat = GetFormatName(pCF->m_cfType);
			// the recset takes ownership of the HGLOBAL
			recset.ReplaceData(pCF->m_hgData, GlobalSize(pCF->m_hgData));
			
			recset.Update();
			
			m_Formats.RemoveAt(i); // the recset now owns the global
		}
		
		recset.Close();
		return true;
	}
	CATCHDAO
		
	return false;
}

// changes m_Time to be later than the latest clip entry in the db
// ensures that pClip's time is not older than the last clip added
// old times can happen on fast copies (<1 sec).
void CClip::MakeLatestTime()
{
	long lDate;
	try
	{
		CMainTable recset;
		
		recset.m_strSort = "lDate DESC";
		recset.Open("SELECT * FROM Main");
		if(!recset.IsEOF())
		{
			recset.MoveFirst();
			
			lDate = (long) m_Time.GetTime();
			if( lDate <= recset.m_lDate )
			{
				lDate = recset.m_lDate + 1;
				m_Time = lDate;
			}
		}
		recset.Close();
	}
	CATCHDAO
}

// STATICS

// Allocates a Global containing the requested Clip Format Data
HGLOBAL CClip::LoadFormat(long lID, UINT cfType)
{
	HGLOBAL hGlobal = 0;
	try
	{
		CDataTable recset;
		CString csSQL;
		
		csSQL.Format(
			"SELECT Data.* FROM Data "
			"INNER JOIN Main ON Main.lDataID = Data.lDataID "
			"WHERE Main.lID = %d "
			"AND Data.strClipBoardFormat = \'%s\'",
			lID,
			GetFormatName(cfType));
		
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, csSQL);
		
		if( !recset.IsBOF() && !recset.IsEOF() )
		{
			// create a new HGLOBAL duplicate
			hGlobal = NewGlobalH( recset.m_ooData.m_hData, recset.m_ooData.m_dwDataLength );
			// XOR take the recset's HGLOBAL... is this SAFE??
			//			hGlobal = recset.TakeData();
			if( !hGlobal || ::GlobalSize(hGlobal) == 0 )
			{
				TRACE0( GetErrorString(::GetLastError()) );
				//::_RPT0( _CRT_WARN, GetErrorString(::GetLastError()) );
				ASSERT(FALSE);
			}
		}
		
		recset.Close();
	}
	CATCHDAO
		
	return hGlobal;
}

bool CClip::LoadFormats(long lID, CClipFormats& formats, bool bOnlyLoad_CF_TEXT)
{
	CClipFormat cf;
	HGLOBAL hGlobal = 0;
	
	formats.RemoveAll();
	
	try
	{
		CDataTable recset;
		
		//Open the data table for all that have the parent id
		CString csSQL;
		csSQL.Format(
			"SELECT Data.* FROM Data "
			"INNER JOIN Main ON Main.lDataID = Data.lDataID "
			"WHERE Main.lID = %d", lID);
		
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, csSQL);
		
		while( !recset.IsEOF() )
		{
			cf.m_cfType = GetFormatID( recset.m_strClipBoardFormat );
			
			if(bOnlyLoad_CF_TEXT)
			{
				if(cf.m_cfType != CF_TEXT)
				{
					recset.MoveNext();
					continue;
				}
			}
			
			// create a new HGLOBAL duplicate
			hGlobal = NewGlobalH( recset.m_ooData.m_hData, recset.m_ooData.m_dwDataLength );
			// XOR take the recset's HGLOBAL... is this SAFE??
			//			hGlobal = recset.TakeData();
			if( !hGlobal || ::GlobalSize(hGlobal) == 0 )
			{
				TRACE0( GetErrorString(::GetLastError()) );
				//::_RPT0( _CRT_WARN, GetErrorString(::GetLastError()) );
				ASSERT(FALSE);
			}
			
			cf.m_hgData = hGlobal;
			formats.Add( cf );
			recset.MoveNext();
		}
		cf.m_hgData = 0; // formats owns all the data
		
		recset.Close();
	}
	CATCHDAO
		
	return formats.GetSize() > 0;
}

void CClip::LoadTypes(long lID, CClipTypes& types)
{
	types.RemoveAll();
	try
	{
		CDataTable recset;
		CString csSQL;
		// get formats for Clip "lID" (Main.lID) using the corresponding Main.lDataID
		csSQL.Format(
			"SELECT Data.* FROM Data "
			"INNER JOIN Main ON Main.lDataID = Data.lDataID "
			"WHERE Main.lID = %d", lID);
		
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, csSQL);
		
		while(!recset.IsEOF())
		{
			types.Add( GetFormatID( recset.m_strClipBoardFormat ) );
			recset.MoveNext();
		}
		
		recset.Close();
	}
	CATCHDAO
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
		DELETE_PTR( pClip );
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
			theApp.SetStatus(StrF("%d",nRemaining), true);
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

