#include "stdafx.h"
#include "CP_Main.h"
#include "ProcessPaste.h"
#include "ClipIds.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CProcessPaste::CProcessPaste()
{
	m_pOle = new COleClipSource;
	m_bSendPaste = true;
	m_bActivateTarget = true;
	m_bOnlyPaste_CF_TEXT = false;
	m_pastedFromGroup = false;
}

CProcessPaste::~CProcessPaste()
{	
	delete m_pOle;
}

BOOL CProcessPaste::DoPaste()
{
	m_pOle->m_bOnlyPaste_CF_TEXT = m_bOnlyPaste_CF_TEXT;

	if(m_pOle->DoImmediateRender())
	{
		// MarkAsPasted() must be done first since it makes use of
		//  m_pOle->m_ClipIDs and m_pOle is inaccessible after
		//  SetClipboard is called.
		MarkAsPasted();
		
		// Ignore the clipboard change that we will cause IF:
		// 1) we are pasting a single element, since the element is already
		//    in the db and its lDate was updated by MarkAsPasted().
		// OR
		// 2) we are pasting multiple, but g_Opt.m_bSaveMultiPaste is false
		if(GetClipIDs().GetSize() == 1 || !g_Opt.m_bSaveMultiPaste)
		{
			m_pOle->CacheGlobalData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));
		}
		else
		{
			m_pOle->CacheGlobalData(theApp.m_cfDelaySavingData, NewGlobalP("Delay", sizeof("Delay")));
		}
		
		m_pOle->SetClipboard(); // m_pOle is now managed by the OLE clipboard

		// The Clipboard now owns the allocated memory
		// and will delete this data object
		// when new data is put on the Clipboard
		m_pOle = NULL; // m_pOle should not be accessed past this point

		if(m_bSendPaste)
		{
			Log(_T("Sending Paste to active window"));
			theApp.m_activeWnd.SendPaste(m_bActivateTarget);
		}
		else if(m_bActivateTarget)
		{
			Log(_T("Activating active window"));
			theApp.m_activeWnd.ActivateTarget();
		}
		
		return TRUE;
	}
	return FALSE;
}

BOOL CProcessPaste::DoDrag()
{
	m_pOle->DoDelayRender();
	DROPEFFECT de = m_pOle->DoDragDrop(DROPEFFECT_COPY);
	if(de != DROPEFFECT_NONE)
	{
		MarkAsPasted();
		return TRUE;
	}
	return FALSE;
}

void CProcessPaste::MarkAsPasted()
{
	Log(_T("start of MarkAsPasted"));

	CClipIDs& clips = GetClipIDs();
	if(clips.GetSize() == 1)
	{
		CGetSetOptions::SetTripPasteCount(-1);
		CGetSetOptions::SetTotalPasteCount(-1);

		MarkAsPastedData* pData = new MarkAsPastedData();
		pData->clipId = clips.ElementAt(0);
		pData->pastedFromGroup = m_pastedFromGroup;

		//Moved to a thread because when running from from U3 devices the write is time consuming
		AfxBeginThread(CProcessPaste::MarkAsPastedThread, (LPVOID)pData, THREAD_PRIORITY_LOWEST);
	}

	Log(_T("End of MarkAsPasted"));
}

UINT CProcessPaste::MarkAsPastedThread(LPVOID pParam)
{
	static CEvent UpdateTimeEvent(TRUE, TRUE, _T("Ditto_Update_Clip_Time"), NULL);
	UpdateTimeEvent.ResetEvent();

	Log(_T("Start of MarkAsPastedThread"));

	//If running from a U3 device then wait a little before updating the db
	//updating the db can take a second or two and it delays the act of pasting
	if(g_Opt.m_bU3)
	{
		Sleep(350);
	}

	BOOL bRet = FALSE;

	try
	{
		MarkAsPastedData* pData = (MarkAsPastedData*)pParam;
		if(pData)
		{
			if(g_Opt.m_bUpdateTimeOnPaste)
			{
				try
				{
					if(pData->pastedFromGroup)
					{
						CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT clipGroupOrder FROM Main ORDER BY clipGroupOrder DESC LIMIT 1"));

						if(q.eof() == false)
						{
							double latestDate = q.getFloatField(_T("clipGroupOrder"));
							latestDate += 1;

							Log(StrF(_T("Setting clipId: %d, GroupOrder: %f"), pData->clipId, latestDate));

							theApp.m_db.execDMLEx(_T("UPDATE Main SET clipGroupOrder = %f where lID = %d;"), latestDate, pData->clipId);

							theApp.RefreshClipOrder(pData->clipId);
						}
					}
					else
					{
						CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT clipOrder FROM Main ORDER BY clipOrder DESC LIMIT 1"));

						if(q.eof() == false)
						{
							double latestDate = q.getFloatField(_T("clipOrder"));
							latestDate += 1;

							Log(StrF(_T("Setting clipId: %d, order: %f"), pData->clipId, latestDate));

							theApp.m_db.execDMLEx(_T("UPDATE Main SET clipOrder = %f where lID = %d;"), latestDate, pData->clipId);

							theApp.RefreshClipOrder(pData->clipId);
						}
					}
				}
				CATCH_SQLITE_EXCEPTION
			}

			try
			{
				theApp.m_db.execDMLEx(_T("UPDATE Main SET lastPasteDate = %d where lID = %d;"), (int)CTime::GetCurrentTime().GetTime(), pData->clipId);
			}
			CATCH_SQLITE_EXCEPTION

			delete pData;
			bRet = TRUE;
		}
	}
	CATCH_SQLITE_EXCEPTION

	Log(_T("End of MarkAsPastedThread"));

	UpdateTimeEvent.SetEvent();
	return bRet;
}