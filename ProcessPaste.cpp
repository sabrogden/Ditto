#include "stdafx.h"
#include "CP_Main.h"
#include "ProcessPaste.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*------------------------------------------------------------------*\
	Globals
\*------------------------------------------------------------------*/

void SendPaste(HWND hWnd)
{
	if(hWnd)
	{
		//Set a Control key down then a 'V' down then the ups
		//To simulate a paste
		::SetForegroundWindow(hWnd);
		// Does SetFocus do anything?... since hWnd is not associated with
		//	our thread's message queue?
//		::SetFocus(hWnd);
	
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

     
		keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	else
		ASSERT(FALSE);
}

BOOL MarkClipAsPasted(long lID)
{
	CGetSetOptions::SetTripPasteCount(-1);
	CGetSetOptions::SetTotalPasteCount(-1);

	if( !g_Opt.m_bUpdateTimeOnPaste )
		return FALSE;

	try
	{	
		//Update the time it was copied so that it appears at the top of the 
		//paste list.  Items are sorted by this time.
		CMainTable ctMain;
		ctMain.Open("SELECT * FROM Main WHERE lID = %d", lID);
		ctMain.Edit();
	
		CTime now = CTime::GetCurrentTime();
		ctMain.m_lDate = (long)now.GetTime();
	
		ctMain.Update();
		ctMain.Close();

		return TRUE;
	}
	catch(CDaoException *e)
	{
		ASSERT(FALSE);
		e->Delete();
	}

	return FALSE;
}


/*------------------------------------------------------------------*\
	CClipIDs
\*------------------------------------------------------------------*/
// allocate an HGLOBAL of the given Format Type representing these Clip IDs.
HGLOBAL CClipIDs::Render( UINT cfType )
{
int count = GetCount();
	if( count <= 0 )
		return 0;
	if( count == 1 )
		return CClip::LoadFormat( GetAt(0), cfType );
	CString text = AggregateText( CF_TEXT, "\r\n" );
	return NewGlobalP( (void*)(LPCSTR) text, text.GetLength()+1 );
}

void CClipIDs::GetTypes( CClipTypes& types )
{
int count = GetCount();
	types.RemoveAll();
	if( count > 1 )
		types.Add( CF_TEXT );
	else if( count == 1 )
		CClip::LoadTypes( GetAt(0), types );
}

// Aggregates the cfType Format Data of the Clip IDs in this array, assuming
//  each Format is NULL terminated and placing pSeparator between them.
// This assumes that the given cfType is a null terminated text type.
CString CClipIDs::AggregateText( UINT cfType, char* pSeparator )
{
	CString csSQL;
	CDataTable recset;
	CString text;
	char* pData = NULL;
	DWORD len;
	DWORD maxLen;

	// maybe we should sum up the "recset.m_ooData.m_dwDataLength" of all IDs first
	//  in order to determine the max space required???  Or would that be wastefull?

	// allocate a large initial buffer to minimize realloc for concatenations
	text.GetBuffer(1000);
	text.ReleaseBuffer(0);

	int numIDs = GetCount();
	int* pIDs = GetData();

	csSQL.Format("SELECT * FROM Data WHERE strClipBoardFormat = \'%s\' AND lParentID = %%d", GetFormatName(cfType));
	try
	{
		for( int i=0; i < numIDs; i++ )
		{
			recset.Open( csSQL, pIDs[i] );
			if( !recset.IsBOF() && !recset.IsEOF() )
			{
				maxLen = recset.m_ooData.m_dwDataLength;
				if( maxLen == 0 )
					continue;
				pData = (char*) GlobalLock(recset.m_ooData.m_hData);
				ASSERT( pData );

				// verify that pData is null terminated 
				// do a quick check to see if the last character is null
				if( pData[maxLen-1] != '\0' )
				{
					for( len=0; len < maxLen && pData[len] != '\0'; len++ ) {}
					// if it is not null terminated, skip this item
					if( len >= maxLen )
						continue;
				}

				text += pData;
				GlobalUnlock(recset.m_ooData.m_hData);

				if( pSeparator )
					text += pSeparator;
			}
			recset.Close();
		}
	}
	CATCHDAO

	return text;
}


/*------------------------------------------------------------------*\
	COleClipSource
\*------------------------------------------------------------------*/
IMPLEMENT_DYNAMIC(COleClipSource, COleDataSource)
COleClipSource::COleClipSource()
{
}

COleClipSource::~COleClipSource()
{
}

BOOL COleClipSource::DoDelayRender()
{
	CClipTypes types;
	m_ClipIDs.GetTypes( types );

	int count = types.GetCount();
	for( int i=0; i < count; i++ )
		DelayRenderData( types[i] );

	return count;
}

BOOL COleClipSource::DoImmediateRender()
{
int count = m_ClipIDs.GetCount();
	if( count <= 0 )
		return 0;
	if( count == 1 )
	{
	CClipFormats formats;
	CClipFormat* pCF;
		CClip::LoadFormats( m_ClipIDs[0], formats );
		count = formats.GetCount(); // reusing "count"
		for( int i=0; i < count; i++ )
		{
			pCF = &formats[i];
			CacheGlobalData( pCF->m_cfType, pCF->m_hgData );
			pCF->m_hgData = 0; // OLE owns it now
		}
		formats.RemoveAll();
		return count;
	}

HGLOBAL hGlobal;
CString text = m_ClipIDs.AggregateText( CF_TEXT, "\r\n" );
	hGlobal = NewGlobalP( (void*)(LPCSTR) text, text.GetLength()+1 );
	CacheGlobalData( CF_TEXT, hGlobal );
	return hGlobal != 0;
}


BEGIN_MESSAGE_MAP(COleClipSource, COleDataSource)
END_MESSAGE_MAP()

// COleClipSource message handlers

BOOL COleClipSource::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	HGLOBAL hData = m_ClipIDs.Render( lpFormatEtc->cfFormat );

	if( !hData )
		return FALSE;

	// if phGlobal is null, we can just give the allocated mem
	// else, our data must fit within the GlobalSize(*phGlobal)
	if( *phGlobal == 0 )
		*phGlobal = hData;
	else
	{
		UINT len = min( ::GlobalSize(*phGlobal), ::GlobalSize(hData) );
		if( len )
			CopyToGlobalHH( *phGlobal, hData, len );
		::GlobalFree( hData );
	}
	return TRUE;
}


/*------------------------------------------------------------------*\
	CProcessPaste
\*------------------------------------------------------------------*/

CProcessPaste::CProcessPaste( HWND hWnd ) :
	m_hWnd(hWnd), m_bDeleteOle(true)
{
	m_pOle = new COleClipSource;
}

CProcessPaste::~CProcessPaste()
{	
	if( m_bDeleteOle )
		delete m_pOle;
}

BOOL CProcessPaste::DoPaste()
{
bool bOldState;
	if( m_pOle->DoImmediateRender() )
	{
		// if we are pasting a single element, do not handle clipboard data change
		// (the element is already in the db and its lDate is updated by MarkAsPasted())
		if( GetClipIDs().GetCount() == 1 )
		{
			bOldState = theApp.EnableCbCopy( false );
			m_pOle->SetClipboard();
			theApp.EnableCbCopy(bOldState);
		}
		else // we are pasting a new aggregate text
		{
			if( g_Opt.m_bSaveMultiPaste )
				m_pOle->SetClipboard();
			else
			{
				bOldState = theApp.EnableCbCopy( false );
				m_pOle->SetClipboard();
				theApp.EnableCbCopy( bOldState );
			}
		}

		m_bDeleteOle = false; // m_pOle is managed by the OLE clipboard now
		SendPaste(m_hWnd);
		MarkAsPasted();
		return TRUE;
	}
	return FALSE;
}

BOOL CProcessPaste::DoDrag()
{
	m_pOle->DoDelayRender();
	DROPEFFECT de = m_pOle->DoDragDrop( DROPEFFECT_COPY );
	if( de != DROPEFFECT_NONE )
	{
		MarkAsPasted();
		return TRUE;
	}
	return FALSE;
}

void CProcessPaste::MarkAsPasted()
{
CClipIDs& clips = GetClipIDs();
	if( clips.GetCount() == 1 )
		MarkClipAsPasted( clips.GetAt(0) );
}

