// ProcessCopy.cpp: implementation of the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "ProcessCopy.h"
#include "DatabaseUtilities.h"
#include ".\processcopy.h"

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
	if( hGlobal )
	{
		if( !::IsValid(hGlobal) )
		{
			LOG( StrF(
			  "COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned.",
			  GetFormatName(cfFormat) ) );
			::GlobalFree( hGlobal );
			hGlobal = NULL;
		}
		return hGlobal;
	}

	// The data isn't in global memory, so try getting an IStream interface to it.
	STGMEDIUM stg;
		
	if( !GetData(cfFormat, &stg) )
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

	if( hGlobal && !::IsValid(hGlobal) )
	{
		LOG( StrF(
			"COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned.",
			GetFormatName(cfFormat) ) );
		::GlobalFree( hGlobal );
		hGlobal = NULL;
	}

	return hGlobal;
}

/*----------------------------------------------------------------------------*\
	CClipFormat - holds the data of one clip format.
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
	CClipFormats - holds an array of CClipFormat
\*----------------------------------------------------------------------------*/
// returns a pointer to the CClipFormat in this array which matches the given type
//  or NULL if that type doesn't exist in this array.
CClipFormat* CClipFormats::FindFormat( UINT cfType )
{
CClipFormat* pCF;
int count = GetSize();
	for( int i=0; i < count; i++ )
	{
		pCF = &ElementAt(i);
		if( pCF->m_cfType == cfType )
			return pCF;
	}
	return NULL;
}


/*----------------------------------------------------------------------------*\
	CClip - holds multiple CClipFormats and CopyClipboard() statistics
\*----------------------------------------------------------------------------*/

CClip::CClip() : m_ID(0), m_DataID(0), m_lTotalCopySize(0)
{}

CClip::~CClip()
{
int count = m_Formats.GetSize();
	// in proper handling, m_Formats should be empty
	ASSERT( count == 0 );
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

void CClip::EmptyFormats()
{
	// free global memory in m_Formats
	for( int i = m_Formats.GetSize()-1; i >= 0; i-- )
	{
		m_Formats[i].Free();
		m_Formats.RemoveAt( i );
	}
}

// Adds a new Format to this Clip by copying the given data.
bool CClip::AddFormat( CLIPFORMAT cfType, void* pData, UINT nLen )
{
	ASSERT( pData && nLen );
HGLOBAL hGlobal = ::NewGlobalP( pData, nLen );
	ASSERT( hGlobal );
	// update the Clip statistics
	m_Time = m_Time.GetCurrentTime();
	m_lTotalCopySize += nLen;
	if( !SetDescFromText( hGlobal ) )
		SetDescFromType();

CClipFormat format(cfType,hGlobal);
CClipFormat *pFormat;

	pFormat = m_Formats.FindFormat(cfType);
	// if the format type already exists as part of this clip, replace the data
	if( pFormat )
	{
		pFormat->Free();
		pFormat->m_hgData = format.m_hgData;
	}
	else
		m_Formats.Add(format);

	format.m_hgData = 0; // now owned by m_Formats
	return true;
}

#define EXIT_LoadFromClipboard(ret)	{ oleData.Release(); g_bCopyingClipboard = false; return(ret); }
bool g_bCopyingClipboard = false; // for debugging reentrance
// Fills this CClip with the contents of the clipboard.
bool CClip::LoadFromClipboard( CClipTypes* pClipTypes )
{
COleDataObjectEx oleData;
CClipTypes defaultTypes;
CClipTypes* pTypes = pClipTypes;

	//ASSERT( !g_bCopyingClipboard );
	// For some reason, this can actually happen with *very* fast copies.
	// This is probably due to the OLE functions processing messages.
	// This *might* be able to be avoided by directly using the win32 Clipboard API
	// If this does happen, we just ignore the request by returning failure.
	if( g_bCopyingClipboard )
		return false;

	g_bCopyingClipboard = true;

	// m_Formats should be empty when this is called.
	ASSERT( m_Formats.GetSize() == 0 );

	// If the data is supposed to be private, then return
	if( ::IsClipboardFormatAvailable( theApp.m_cfIgnoreClipboard ) )
		EXIT_LoadFromClipboard( false );

	//Attach to the clipboard
	if( !oleData.AttachClipboard() )
	{
		ASSERT(0); // does this ever happen?
		EXIT_LoadFromClipboard(false);
	}

	oleData.EnsureClipboardObject();

	// if no types were given, get only the first (most important) type.
	//  (subsequent types could be synthetic due to automatic type conversions)
	if( pTypes == NULL || pTypes->GetSize() == 0 )
	{
		ASSERT(0); // this feature is not currently used... it is an error if it is.

		FORMATETC formatEtc;
		oleData.BeginEnumFormats();
		oleData.GetNextFormat(&formatEtc);
		defaultTypes.Add( formatEtc.cfFormat );
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
	if( oleData.IsDataAvailable( cfDesc.m_cfType ) )
	{
		cfDesc.m_hgData = oleData.GetGlobalData( cfDesc.m_cfType );
		bIsDescSet = SetDescFromText( cfDesc.m_hgData );
	}

	// Get global data for each supported type on the clipboard
UINT nSize;
CClipFormat cf;
int numTypes = pTypes->GetSize();
	for(int i = 0; i < numTypes; i++)
	{
		cf.m_cfType = pTypes->ElementAt(i);

		// is this the description we already fetched?
		if( cf.m_cfType == cfDesc.m_cfType )
		{
			cf = cfDesc;
			cfDesc.m_hgData = 0; // cf owns it now (to go into m_Formats)
		}
		else if( !oleData.IsDataAvailable(cf.m_cfType) )
			continue;
		else
			cf.m_hgData = oleData.GetGlobalData( cf.m_cfType );

		if( cf.m_hgData )
		{
			nSize = GlobalSize( cf.m_hgData );
			if( nSize > 0 )
			{
				ASSERT( ::IsValid(cf.m_hgData) );
				m_Formats.Add( cf );
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

	if( !bIsDescSet )
		SetDescFromType();

	// if the description was in a type that is not supported,
	//	we have to free it since it wasn't added to m_Formats
	if( cfDesc.m_hgData )
		cfDesc.Free();

	if( m_Formats.GetSize() == 0 )
		EXIT_LoadFromClipboard( false );

	EXIT_LoadFromClipboard( true );
}

bool CClip::SetDescFromText( HGLOBAL hgData )
{
	if( hgData == 0 )
		return false;

bool bRet = false;
char* text = (char *) GlobalLock(hgData);
long ulBufLen = GlobalSize(hgData);

	ASSERT( text != NULL );

	if( ulBufLen > g_Opt.m_bDescTextSize )
		ulBufLen = g_Opt.m_bDescTextSize;

	if( ulBufLen > 0 )
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
	if( m_Formats.GetSize() <= 0 )
		return false;
	m_Desc = GetFormatName( m_Formats[0].m_cfType );
	return m_Desc.GetLength() > 0;
}

bool CClip::AddToDB( bool bCheckForDuplicates )
{
bool bResult;
	try
	{
		if( bCheckForDuplicates )
		{
			CMainTable recset;

			if( FindDuplicate( recset, g_Opt.m_bAllowDuplicates ) )
			{
				m_ID = recset.m_lID;
				recset.Edit();
				recset.m_lDate = (long) m_Time.GetTime(); // update the copy Time
				recset.Update();
				recset.Close();
				EmptyFormats(); // delete this clip's data from memory.
				return true;
			}

			if( recset.IsOpen() )
				recset.Close();
		}
	}
	CATCHDAO

	// AddToDataTable must go first in order to assign m_DataID
	bResult = AddToDataTable() && AddToMainTable();

	// should be emptied by AddToDataTable
	ASSERT( m_Formats.GetSize() == 0 );

	return bResult;
}

// if a duplicate exists, set recset to the duplicate and return true
bool CClip::FindDuplicate( CMainTable& recset, BOOL bCheckLastOnly )
{
	ASSERT( m_lTotalCopySize > 0 );
	try
	{
		recset.m_strSort = "lDate DESC";

		if( bCheckLastOnly )
		{
			recset.Open("SELECT * FROM Main");
			recset.MoveFirst();
			// if an entry exists and they are the same size and the format data matches
			if( !recset.IsBOF() && !recset.IsEOF() &&
			     m_lTotalCopySize == recset.m_lTotalCopySize &&
			    (CompareFormatDataTo(recset.m_lDataID) == 0) )
			{	return true; }
			return false;
		}

		// Look for any other entries that have the same size
		recset.Open("SELECT * FROM Main WHERE lTotalCopySize = %d", m_lTotalCopySize);
		while( !recset.IsEOF() )
		{
			//if there is any then look if it is an exact match
			if( CompareFormatDataTo(recset.m_lDataID) == 0 )
				return true;

			recset.MoveNext();
		}
	}
	CATCHDAO

	return false;
}

int CClip::CompareFormatDataTo( long lDataID )
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
		if( nRet != 0 || nRecs == 0 )
		{	recset.Close();	return nRet; }

		// For each format type in the db
		
		for( recset.MoveFirst(); !recset.IsEOF(); recset.MoveNext() )
		{
			pFormat = m_Formats.FindFormat( GetFormatID(recset.m_strClipBoardFormat) );

			// Verify the format exists
			if( !pFormat )
			{	recset.Close(); return -1; }

			// Compare the size
			nRet = ::GlobalSize(pFormat->m_hgData) - recset.m_ooData.m_dwDataLength;
			if( nRet != 0 )
			{	recset.Close(); return nRet; }

			// Binary compare
			nRet = CompareGlobalHH( recset.m_ooData.m_hData,
			                        pFormat->m_hgData,
			                        recset.m_ooData.m_dwDataLength );
			if( nRet != 0 )
			{	recset.Close(); return nRet; }
		}
		recset.Close();
	}
	CATCHDAO

	return 0;
}

// assigns m_ID
bool CClip::AddToMainTable()
{
long lDate;
	try
	{
	CMainTable recset;

//		recset.m_strSort = "lDate DESC";
		recset.Open("SELECT * FROM Main");

		lDate = (long) m_Time.GetTime();

		recset.AddNew();  // overridden to set m_lID to the new autoincr number

		m_ID = recset.m_lID;

		recset.m_lDate = lDate;
		recset.m_strText = m_Desc;
		recset.m_lTotalCopySize = m_lTotalCopySize;

		recset.m_bIsGroup = FALSE;
		recset.m_lParentID = theApp.m_GroupDefaultID;

		VERIFY( m_DataID > 0 ); // AddToDataTable must be called first to assign this
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

		for( int i = m_Formats.GetSize()-1; i >= 0 ; i-- )
		{
			pCF = & m_Formats.ElementAt(i);

			recset.AddNew(); // overridden to assign new autoincr ID to m_lID

			if( m_DataID <= 0 )
			{
				VERIFY( recset.m_lID > 0 );
				m_DataID = recset.m_lID;
			}

			recset.m_lDataID = m_DataID;
			recset.m_strClipBoardFormat = GetFormatName( pCF->m_cfType );
			// the recset takes ownership of the HGLOBAL
			recset.ReplaceData( pCF->m_hgData, GlobalSize(pCF->m_hgData) );

			recset.Update();

			m_Formats.RemoveAt( i ); // the recset now owns the global
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
HGLOBAL CClip::LoadFormat( long lID, UINT cfType )
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

bool CClip::LoadFormats( long lID, CClipFormats& formats )
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
			cf.m_cfType = GetFormatID( recset.m_strClipBoardFormat );
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

void CClip::LoadTypes( long lID, CClipTypes& types )
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

		while( !recset.IsEOF() )
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
	while( GetCount() )
	{
		pClip = RemoveHead();
		DELETE_PTR( pClip );
	}
}

// returns the number of clips actually saved
// while this does empty the Format Data, it does not delete the Clips.
int CClipList::AddToDB( bool bLatestTime, bool bShowStatus )
{
int savedCount = 0;
int nRemaining = 0;
CClip* pClip;
POSITION pos;
bool bResult;

	nRemaining = GetCount();
	pos = GetHeadPosition();
	while( pos )
	{
		if( bShowStatus )
		{
			theApp.SetStatus( StrF("%d",nRemaining), true );
			nRemaining--;
		}

		pClip = GetNext( pos );
		ASSERT( pClip );

		if( bLatestTime )
			pClip->MakeLatestTime();

		bResult = pClip->AddToDB();
		if( bResult )
			savedCount++;
	}

	if( bShowStatus )
		theApp.SetStatus(NULL, true);

	return savedCount;
}


/*----------------------------------------------------------------------------*\
	CClipboardViewer
\*----------------------------------------------------------------------------*/

IMPLEMENT_DYNAMIC(CClipboardViewer, CWnd)

BEGIN_MESSAGE_MAP(CClipboardViewer, CWnd)
	//{{AFX_MSG_MAP(CClipboardViewer)
	ON_WM_CREATE()
	ON_WM_CHANGECBCHAIN()
	ON_WM_DRAWCLIPBOARD()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CV_RECONNECT, OnCVReconnect)
	ON_MESSAGE(WM_CV_IS_CONNECTED, OnCVIsConnected)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClipboardViewer construction/destruction

CClipboardViewer::CClipboardViewer( CCopyThread* pHandler )
{
	m_hNextClipboardViewer = 0;
	m_bCalling_SetClipboardViewer = false;
	m_lReconectCount = 0;
	m_bIsConnected = false;
	m_pHandler = pHandler;
	ASSERT(m_pHandler);
	m_bPinging = false;
	m_bPingSuccess = false;
}

CClipboardViewer::~CClipboardViewer()
{
}

void CClipboardViewer::Create()
{
CString strParentClass = AfxRegisterWndClass(0);
	CWnd::CreateEx(0, strParentClass, "Ditto Clipboard Viewer", 0, -1, -1, 0, 0, 0, 0);
}

// connects as a clipboard viewer
void CClipboardViewer::Connect()
{
	ASSERT( ::IsWindow(m_hWnd) );
	if( m_bIsConnected )
		return;
	//Set up the clip board viewer
	m_bCalling_SetClipboardViewer = true;
	m_hNextClipboardViewer = CWnd::SetClipboardViewer();
	m_bCalling_SetClipboardViewer = false;
	m_bIsConnected = SendPing();
}

// disconnects as a clipboard viewer
void CClipboardViewer::Disconnect()
{
	if( !m_bIsConnected )
		return;
	ASSERT( ::IsWindow(m_hWnd) );
	CWnd::ChangeClipboardChain( m_hNextClipboardViewer );
	m_hNextClipboardViewer = 0;
	m_bIsConnected = false;
}

bool CClipboardViewer::SendPing()
{
HWND hWnd;
bool bResult = false;

	hWnd = ::GetClipboardViewer();
	// if there is a chain
	if( ::IsWindow(hWnd) )
	{
		m_bPingSuccess = false;
		m_bPinging = true;
		::SendMessage( hWnd, WM_DRAWCLIPBOARD, 0, 0 );
		m_bPinging = false;
		bResult = m_bPingSuccess;
	}

	m_bIsConnected = bResult;

	return bResult;
}

bool CClipboardViewer::EnsureConnected()
{
	if( !SendPing() )
		Connect();

	return m_bIsConnected;
}

// puts format "Clipboard Viewer Ignore" on the clipboard
void CClipboardViewer::SetCVIgnore()
{
	if( ::OpenClipboard( m_hWnd ) )
	{
		::SetClipboardData( theApp.m_cfIgnoreClipboard, NULL );
		::CloseClipboard();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CClipboardViewer message handlers

int CClipboardViewer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// verify that we are in the chain every minute
	SetTimer(TIMER_ENSURE_VIEWER_IN_CHAIN, ONE_MINUTE, 0);

	//Set up the clip board viewer
	Connect();

	return 0;
}

void CClipboardViewer::OnDestroy()
{
	Disconnect();
	CWnd::OnDestroy();
}

void CClipboardViewer::OnChangeCbChain(HWND hWndRemove, HWND hWndAfter) 
{
	// If the next window is closing, repair the chain. 
	if(m_hNextClipboardViewer == hWndRemove)
    {
		m_hNextClipboardViewer = hWndAfter;
    }
    // Otherwise, pass the message to the next link.
	else if (m_hNextClipboardViewer != NULL)
    {
		::SendMessage ( m_hNextClipboardViewer, WM_CHANGECBCHAIN, 
						(WPARAM) hWndRemove, (LPARAM) hWndAfter );
    }
}

//Message that the clipboard data has changed
void CClipboardViewer::OnDrawClipboard() 
{
	if( m_bPinging )
	{
		m_bPingSuccess = true;
		return;
	}

	// don't process the event when we first attach
	if( m_pHandler && !m_bCalling_SetClipboardViewer )
	{
		if( !::IsClipboardFormatAvailable( theApp.m_cfIgnoreClipboard ) )
			m_pHandler->OnClipboardChange();
	}

	// pass the event to the next Clipboard viewer in the chain
	if( m_hNextClipboardViewer != NULL )
		::SendMessage(m_hNextClipboardViewer, WM_DRAWCLIPBOARD, 0, 0);	
}

void CClipboardViewer::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case TIMER_ENSURE_VIEWER_IN_CHAIN:
		EnsureConnected();
		break;
	}

	CWnd::OnTimer(nIDEvent);
}

LRESULT CClipboardViewer::OnCVReconnect(WPARAM wParam, LPARAM lParam)
{
	return EnsureConnected();
}

LRESULT CClipboardViewer::OnCVIsConnected(WPARAM wParam, LPARAM lParam)
{
	return SendPing();
}

/*----------------------------------------------------------------------------*\
	CCopyConfig
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
	CCopyThread
\*----------------------------------------------------------------------------*/

IMPLEMENT_DYNCREATE(CCopyThread, CWinThread)

CCopyThread::CCopyThread()
{
	m_bQuit = false;
	m_bAutoDelete = false;

	m_bConfigChanged = false;
	m_pClips = new CClipList;
	m_pClipboardViewer = new CClipboardViewer(this);
	::InitializeCriticalSection(&m_CS);
}

CCopyThread::~CCopyThread()
{
	m_LocalConfig.DeleteTypes();
	m_SharedConfig.DeleteTypes();
	DELETE_PTR( m_pClipboardViewer );
	if( m_pClips )
		ASSERT( m_pClips->GetCount() == 0 );
	DELETE_PTR( m_pClips );
	::DeleteCriticalSection(&m_CS);
}

// perform and per-thread initialization here
BOOL CCopyThread::InitInstance()
{
	ASSERT( ::GetCurrentThreadId() == m_nThreadID );
	SetThreadName(m_nThreadID, "COPY");
	// the window is created within this thread and therefore uses its message queue
	m_pClipboardViewer->Create();
	return TRUE;
}

// perform any per-thread cleanup here
int CCopyThread::ExitInstance()
{
	ASSERT( m_bQuit );  // make sure we intended to quit
	m_pClipboardViewer->Disconnect();
	return CWinThread::ExitInstance();
}

bool CCopyThread::IsClipboardViewerConnected()
{
	ASSERT( m_pClipboardViewer && m_pClipboardViewer->m_hWnd );
	return ::SendMessage( m_pClipboardViewer->m_hWnd, WM_CV_IS_CONNECTED, 0, 0 ) != FALSE;
}

// Called within Copy Thread:
void CCopyThread::OnClipboardChange()
{
	SyncConfig(); // synchronize with the main thread's copy configuration

	// if we are told not to copy on change, then we have nothing to do.
	if( !m_LocalConfig.m_bCopyOnChange )
		return;

	CClip* pClip = new CClip;
	bool bResult = pClip->LoadFromClipboard( m_LocalConfig.m_pSupportedTypes );

	if( !bResult )
	{
		delete pClip;
		return; // error
	}

	AddToClips(	pClip );

	if( m_LocalConfig.m_bAsyncCopy )
		::PostMessage(m_LocalConfig.m_hClipHandler, WM_CLIPBOARD_COPIED, (WPARAM)pClip, 0);
	else
		::SendMessage(m_LocalConfig.m_hClipHandler, WM_CLIPBOARD_COPIED, (WPARAM)pClip, 0);

}

void CCopyThread::SyncConfig()
{
	// atomic read
	if( m_bConfigChanged )
	{
	CClipTypes* pTypes = NULL;
		Hold();

			pTypes = m_LocalConfig.m_pSupportedTypes;

			m_LocalConfig = m_SharedConfig;

			// NULL means that it shouldn't have been sync'ed
			if( m_SharedConfig.m_pSupportedTypes == NULL )
			{	// let m_LocalConfig keep its types
				m_LocalConfig.m_pSupportedTypes = pTypes; // undo sync
				pTypes = NULL; // nothing to delete
			}
			else
				m_SharedConfig.m_pSupportedTypes = NULL; // now owned by LocalConfig

		Release();
		// delete old types
		if( pTypes )
			delete pTypes;
	}
}

void CCopyThread::AddToClips( CClip* pClip )
{
	Hold();
		if( !m_pClips )
			m_pClips = new CClipList;
		m_pClips->AddTail( pClip ); // m_pClips now owns pClip
	Release();
}

// Shared (use thread-safe access functions below)
// Called within Main thread:
CClipList* CCopyThread::GetClips()
{
CClipList* pRet;
CClipList* pClips = new CClipList;
	Hold();
		pRet = m_pClips;
		m_pClips = pClips;
	Release();
	return pRet;
}
void CCopyThread::SetSupportedTypes( CClipTypes* pTypes )
{
CClipTypes* pTemp;
	Hold();
		pTemp = m_SharedConfig.m_pSupportedTypes;
		m_SharedConfig.m_pSupportedTypes = pTypes;
		m_bConfigChanged = true;
	Release();
    if( pTemp )
		delete pTemp;
}
HWND CCopyThread::SetClipHandler( HWND hWnd )
{
HWND hRet;
	Hold();
		hRet = m_SharedConfig.m_hClipHandler;
		m_SharedConfig.m_hClipHandler = hWnd;
		m_bConfigChanged = (hRet != hWnd);
	Release();
	return hRet;
}
HWND CCopyThread::GetClipHandler()
{
HWND hRet;
	Hold();
		hRet = m_SharedConfig.m_hClipHandler;
	Release();
	return hRet;
}
bool CCopyThread::SetCopyOnChange( bool bVal )
{
bool bRet;
	Hold();
		bRet = m_SharedConfig.m_bCopyOnChange;
		m_SharedConfig.m_bCopyOnChange = bVal;
		m_bConfigChanged = (bRet != bVal);
	Release();
	return bRet;
}
bool CCopyThread::GetCopyOnChange()
{
bool bRet;
	Hold();
		bRet = m_SharedConfig.m_bCopyOnChange;
	Release();
	return bRet;
}
bool CCopyThread::SetAsyncCopy( bool bVal )
{
bool bRet;
	Hold();
		bRet = m_SharedConfig.m_bAsyncCopy;
		m_SharedConfig.m_bAsyncCopy = bVal;
		m_bConfigChanged = (bRet != bVal);
	Release();
	return bRet;
}
bool CCopyThread::GetAsyncCopy()
{
bool bRet;
	Hold();
		bRet = m_SharedConfig.m_bAsyncCopy;
	Release();
	return bRet;
}

void CCopyThread::Init( CCopyConfig cfg )
{
	ASSERT( m_LocalConfig.m_pSupportedTypes == NULL );
	m_LocalConfig = m_SharedConfig = cfg;
	// let m_LocalConfig own the m_pSupportedTypes
	m_SharedConfig.m_pSupportedTypes = NULL;
}

bool CCopyThread::Quit()
{
	m_bQuit = true;
	m_pClipboardViewer->PostMessage( WM_QUIT );
	return CWinThread::PostThreadMessage( WM_QUIT, NULL, NULL ) != FALSE;
}


BEGIN_MESSAGE_MAP(CCopyThread, CWinThread)
END_MESSAGE_MAP()

// CCopyThread message handlers



/*----------------------------------------------------------------------------*\
	CProcessCopy
\*----------------------------------------------------------------------------*/

CProcessCopy::CProcessCopy()
{
}

CProcessCopy::~CProcessCopy()
{
}


