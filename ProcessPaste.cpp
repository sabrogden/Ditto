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
	m_pastedFromGroup = false;
}

CProcessPaste::~CProcessPaste()
{	
	delete m_pOle;
}

BOOL CProcessPaste::DoPaste()
{
	BOOL ret = FALSE;

	try
	{
		m_pOle->m_pasteOptions = m_pasteOptions;
		if (!m_pOle->DoImmediateRender())
		{
			return ret;
		}

		// MarkAsPasted() must be done first since it makes use of
		//  m_pOle->m_ClipIDs and m_pOle is inaccessible after
		//  SetClipboard is called.
		MarkAsPasted(m_pasteOptions.m_updateClipOrder);

		// Ignore the clipboard change that we will cause IF:
		// 1) we are pasting a single element, since the element is already
		//    in the db and its lDate was updated by MarkAsPasted().
		// OR
		// 2) we are pasting multiple, but g_Opt.m_bSaveMultiPaste is false
		if (GetClipIDs().GetSize() == 1 || !g_Opt.m_bSaveMultiPaste)
		{
			m_pOle->CacheGlobalData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));
		}
		else
		{
			m_pOle->CacheGlobalData(theApp.m_cfDelaySavingData, NewGlobalP("Delay", sizeof("Delay")));
		}

		m_pOle->SetClipboard(); // m_pOle is now managed by the OLE clipboard

		if (m_bSendPaste)
		{
			Log(_T("Sending Paste to active window"));
			theApp.m_activeWnd.SendPaste(m_bActivateTarget);
		}
		else if (m_bActivateTarget)
		{
			Log(_T("Activating active window"));
			theApp.m_activeWnd.ActivateTarget();
		}

		ret = TRUE;
	
	}
	catch (CException *ex)
	{
		TCHAR szCause[255];
		ex->GetErrorMessage(szCause, 255);
		m_lastErrorMessage.Format(_T("Paste exception: %s"), szCause);
		Log(m_lastErrorMessage);
	}
	catch (...) 
	{
		m_lastErrorMessage = _T("Paste generic exception");
		Log(m_lastErrorMessage);
	}

	// The Clipboard now owns the allocated memory
	// and will delete this data object
	// when new data is put on the Clipboard
	m_pOle = NULL; // m_pOle should not be accessed past this point

	return ret;
}

BOOL CProcessPaste::DoDrag()
{
	BOOL ret = FALSE;
	try
	{
		m_pOle->m_pasteOptions = m_pasteOptions;
		m_pOle->DoDelayRender();
		DROPEFFECT de = m_pOle->DoDragDrop(DROPEFFECT_COPY);
		if (de != DROPEFFECT_NONE)
		{
			MarkAsPasted(m_pasteOptions.m_updateClipOrder);
			ret = TRUE;
		}		
	}
	catch (CException *ex)
	{
		TCHAR szCause[255];
		ex->GetErrorMessage(szCause, 255);
		m_lastErrorMessage.Format(_T("Drag drop exception: %s"), szCause);
		Log(m_lastErrorMessage);
	}
	catch (...)
	{
		m_lastErrorMessage = _T("Drag drop generic exception");
		Log(m_lastErrorMessage);
	}

	try
	{
		//from https://www.codeproject.com/Articles/886711/Drag-Drop-Images-and-Drop-Descriptions-for-MFC-App
		//You may have noted the InternalRelease() function call.This is required here to delete the object.While it is possible to use 
		//delete or create the object on the stack with Drag & Drop operations, it is not recommended to do so.
		m_pOle->InternalRelease();
	}
	catch (CException *ex)
	{
		TCHAR szCause[255];
		ex->GetErrorMessage(szCause, 255);
		m_lastErrorMessage.Format(_T("Drag drop exception 2: %s"), szCause);
		Log(m_lastErrorMessage);
	}
	catch (...)
	{
		m_lastErrorMessage = _T("Drag drop generic exception 2");
		Log(m_lastErrorMessage);
	}

	// The Clipboard now owns the allocated memory
	// and will delete this data object
	// when new data is put on the Clipboard
	m_pOle = NULL; // m_pOle should not be accessed past this point

	return ret;
}

void CProcessPaste::MarkAsPasted(bool updateClipOrder)
{
	Log(_T("start of MarkAsPasted"));

	CClipIDs& clips = GetClipIDs();
	
	CGetSetOptions::SetTripPasteCount(-1);
	CGetSetOptions::SetTotalPasteCount(-1);

	MarkAsPastedData* pData = new MarkAsPastedData();
	for (int i = 0; i < clips.GetCount(); i++)
	{
		pData->ids.Add(clips.ElementAt(i));
	}
	pData->pastedFromGroup = m_pastedFromGroup;
	pData->updateClipOrder = updateClipOrder;

	//Moved to a thread because when running from from U3 devices the write is time consuming
	AfxBeginThread(CProcessPaste::MarkAsPastedThread, (LPVOID)pData, THREAD_PRIORITY_LOWEST);

	Log(_T("End of MarkAsPasted"));
}

UINT CProcessPaste::MarkAsPastedThread(LPVOID pParam)
{
	DWORD startTick = GetTickCount();

	static CEvent UpdateTimeEvent(TRUE, TRUE, _T("Ditto_Update_Clip_Time"), NULL);
	UpdateTimeEvent.ResetEvent();

	Log(_T("Start of MarkAsPastedThread"));

	BOOL bRet = FALSE;
	int clipId = 0;

	try
	{
		int refreshFlags = 0;

		MarkAsPastedData* pData = (MarkAsPastedData*)pParam;
		if(pData)
		{
			int clipCount = (int)pData->ids.GetCount();

			if(g_Opt.m_bUpdateTimeOnPaste && 
				pData->updateClipOrder &&
				clipCount == 1)
			{
				if (CGetSetOptions::m_refreshViewAfterPasting)
				{
					refreshFlags |= UPDATE_AFTER_PASTE_SELECT_CLIP;
				}

				for (int i = 0; i < clipCount; i++)
				{
					int id = pData->ids.ElementAt(i);
					try
					{
						if (pData->pastedFromGroup)
						{
							CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT clipGroupOrder FROM Main ORDER BY clipGroupOrder DESC LIMIT 1"));

							if (q.eof() == false)
							{
								double latestDate = q.getFloatField(_T("clipGroupOrder"));
								latestDate += 1;

								Log(StrF(_T("Setting clipId: %d, GroupOrder: %f"), id, latestDate));

								theApp.m_db.execDMLEx(_T("UPDATE Main SET clipGroupOrder = %f where lID = %d;"), latestDate, id);
							}
						}
						else
						{
							CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT clipOrder FROM Main ORDER BY clipOrder DESC LIMIT 1"));

							if (q.eof() == false)
							{
								double latestDate = q.getFloatField(_T("clipOrder"));
								latestDate += 1;

								Log(StrF(_T("Setting clipId: %d, order: %f"), id, latestDate));

								theApp.m_db.execDMLEx(_T("UPDATE Main SET clipOrder = %f where lID = %d;"), latestDate, id);
							}
						}
					}
					CATCH_SQLITE_EXCEPTION
				}
			}

			try
			{
				for (int i = 0; i < clipCount; i++)
				{
					int id = pData->ids.ElementAt(i);
					theApp.m_db.execDMLEx(_T("UPDATE Main SET lastPasteDate = %d where lID = %d;"), (int)CTime::GetCurrentTime().GetTime(), id);
				}
			}
			CATCH_SQLITE_EXCEPTION

			for (int i = 0; i < clipCount; i++)
			{
				int id = pData->ids.ElementAt(i);

				theApp.RefreshClipInUI(id, refreshFlags);
				break;
			}			

			delete pData;
			bRet = TRUE;
		}
	}
	CATCH_SQLITE_EXCEPTION

	Log(_T("End of MarkAsPastedThread"));

	DWORD endTick = GetTickCount();
	if((endTick-startTick) > 350)
		Log(StrF(_T("Paste Timing MarkAsPastedThread: %d, ClipId: %d"), endTick-startTick, clipId));

	UpdateTimeEvent.SetEvent();
	return bRet;
}