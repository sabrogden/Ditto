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
