#include "stdafx.h"
#include "CP_Main.h"
#include "ProcessPaste.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessPaste::CProcessPaste()
{
}

CProcessPaste::~CProcessPaste()
{
}

BOOL CProcessPaste::LoadDataAndPaste(long lID, HWND hWnd)
{
	COleDataSource *pData = new COleDataSource;
	if(LoadData(lID, pData))
	{
		MarkDataAsPasted(lID);

		//Set this becase we just change the clipboard data so we will get a 
		//clipboard change message
		//If this is set we don't handle that message
		theApp.m_bHandleClipboardDataChange = false;

//		pData->FlushClipboard();
		pData->SetClipboard();
		
		SendPaste(hWnd);
		
		//Set this back to handling messages
		theApp.m_bHandleClipboardDataChange = true;

		return TRUE;
		
	}

	return FALSE;
}

BOOL CProcessPaste::LoadDataAndDrag(long lID)
{
	COleDataSource *pData = new COleDataSource;
	if(LoadData(lID, pData))
	{
		if(pData->DoDragDrop(DROPEFFECT_COPY) != DROPEFFECT_NONE)
		{
			MarkDataAsPasted(lID);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CProcessPaste::MarkDataAsPasted(long lID)
{
	try
	{	
		//Update the time it was copied so that it appears at the top of the 
		//past list items are sorted by this time
		CMainTable ctMain;
		ctMain.Open("SELECT * FROM Main WHERE lID = %d", lID);
		ctMain.Edit();
	
		CTime now = CTime::GetCurrentTime();
		ctMain.m_lDate = (long)now.GetTime();
	
		ctMain.Update();
		ctMain.Close();

		CGetSetOptions::SetTripPasteCount(-1);
		CGetSetOptions::SetTotalPasteCount(-1);

		return TRUE;
	}
	catch(CDaoException *e)
	{
		ASSERT(FALSE);
		e->Delete();
	}

	return FALSE;
}

BOOL CProcessPaste::LoadData(long lID, COleDataSource *pData)
{
	if(pData)
	{
		int nCounter = 0;

		try
		{
			CDataTable recset;

			//Open the data table for all that have the parent id
			CString csSQL, csFormatText;
			csSQL.Format("SELECT * FROM Data WHERE lParentID = %d", lID);
				
			recset.Open(AFX_DAO_USE_DEFAULT_TYPE, csSQL);

			//Go throught each item in the db putting it on the clipboard
			while(!recset.IsEOF())
			{
				csFormatText = recset.m_strClipBoardFormat;
				
				if(recset.LoadData(pData, GetFormatID(csFormatText)))
					nCounter++;

				recset.MoveNext();		
			}

			recset.Close();

			if(nCounter)
				return TRUE;
		}
		catch(CDaoException* e)
		{
			ASSERT(FALSE);
			e->Delete();
		}
	}

	return FALSE;
}

bool CopyToGlobal( HGLOBAL hGlobal, LPVOID pBuf, ULONG ulBufLen )
{
	if( hGlobal == 0 || pBuf == 0 || ulBufLen == 0 )
	{
		ASSERT(FALSE);
		return 0;
	}

	LPVOID pvData = GlobalLock(hGlobal);
	if(!pvData)
	{
		ASSERT(FALSE);
		return false;
	}

	ULONG size = GlobalSize(hGlobal);

	if( size < ulBufLen )
	{
		ASSERT(FALSE);
		GlobalUnlock(hGlobal);
		return false;
	}

	memcpy(pvData, pBuf, ulBufLen);
			
	GlobalUnlock(hGlobal);

	return true;
}

HGLOBAL NewGlobal( LPVOID pBuf, ULONG ulBufLen )
{
	if( pBuf == 0 || ulBufLen == 0 )
	{
		ASSERT(FALSE);
		return 0;
	}
	
	HGLOBAL hGlobal = GlobalAlloc( GMEM_MOVEABLE, ulBufLen );
	if( hGlobal == 0 )
	{
		ASSERT(FALSE);
		return 0;
	}

	CopyToGlobal( hGlobal, pBuf, ulBufLen );

	return hGlobal;
}


BOOL CProcessPaste::MultiPaste( int numIDs, int* pIDs, HWND hWnd )
{
	if( numIDs <= 0 || pIDs == NULL || hWnd == NULL )
		return FALSE;
	if( numIDs == 1 )
		return LoadDataAndPaste( pIDs[0], hWnd );

	CString text = AggregateText(numIDs, pIDs, CF_TEXT, "\r\n");
	HGLOBAL hGlobal = NewGlobal( (void*)(LPCSTR) text, text.GetLength()+1 );
	if( !hGlobal )
		return FALSE;
	
	COleDataSource *pOle = new COleDataSource;
	pOle->CacheGlobalData(CF_TEXT, hGlobal);

	// Should we allow Ditto to add the aggregate to its history list?
	bool bOldHandleChange = theApp.m_bHandleClipboardDataChange;
	theApp.m_bHandleClipboardDataChange = false;

//	pOle->FlushClipboard();
	pOle->SetClipboard();
		
	SendPaste(hWnd);

	//Set this back to original value
	theApp.m_bHandleClipboardDataChange = bOldHandleChange;

	return TRUE;
}

BOOL CProcessPaste::MultiDrag( int numIDs, int* pIDs, HWND hWnd )
{
	if( numIDs <= 0 || pIDs == NULL || hWnd == NULL )
		return FALSE;
	if( numIDs == 1 )
		return LoadDataAndDrag( pIDs[0] );

	CString text = AggregateText(numIDs, pIDs, CF_TEXT, "\r\n");
	HGLOBAL hGlobal = NewGlobal( (void*)(LPCSTR) text, text.GetLength()+1 );
	if( !hGlobal )
		return FALSE;
	
	COleDataSource *pOle = new COleDataSource;
	pOle->CacheGlobalData(CF_TEXT, hGlobal);

	if( pOle->DoDragDrop(DROPEFFECT_COPY) != DROPEFFECT_NONE )
		return TRUE;

	return FALSE;
}


// This assumes that the given uiPastType is a null terminated text type
CString CProcessPaste::AggregateText(int numIDs, int* pIDs, UINT uiPastType, char* pSeparator)
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
	text.GetBuffer(4096);
	text = "";

	csSQL.Format("SELECT * FROM Data WHERE strClipBoardFormat = \'%s\' AND lParentID = %%d", GetFormatName(uiPastType));
	try
	{
		for( int i=0; i < numIDs; i++ )
		{
			recset.Open( csSQL, pIDs[i] );
			if( !recset.IsEOF() )
			{
				maxLen = recset.m_ooData.m_dwDataLength;
				if( maxLen == 0 )
					continue;
				pData = (char*) GlobalLock(recset.m_ooData.m_hData);
				if(!pData)
				{
					ASSERT(FALSE);
					break;
				}

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
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
	}

	return text;
}

void CProcessPaste::SendPaste(HWND hWnd)
{
	if(hWnd)
	{
		//Set a Control key down then a 'V' down then the ups
		//To simulate a paste
		::SetForegroundWindow(hWnd);
		::SetFocus(hWnd);
	
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | 0, 0);

     
		keybd_event('V', 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	else
		ASSERT(FALSE);
}
