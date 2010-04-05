#include "stdafx.h"
#include "QPasteWndThread.h"
#include "Misc.h"
#include "Options.h"
#include "QPasteWnd.h"
#include "cp_main.h"

CQPasteWndThread::CQPasteWndThread(void)
{
	m_firstLoad = false;
    m_waitTimeout = ONE_HOUR * 12;

    m_SearchingEvent = CreateEvent(NULL, TRUE, FALSE, _T(""));

    for(int eventEnum = 0; eventEnum < ECQPASTEWNDTHREADEVENTS_COUNT; eventEnum++)
    {
        AddEvent(eventEnum);
    }
}

CQPasteWndThread::~CQPasteWndThread(void)
{
    CloseHandle(m_SearchingEvent);
}

void CQPasteWndThread::OnTimeOut(void *param)
{
    CloseDatabase();
}

void CQPasteWndThread::OnEvent(int eventId, void *param)
{
    switch((eCQPasteWndThreadEvents)eventId)
    {
        case DO_QUERY:
            OnDoQuery(param);
            break;
        case LOAD_ACCELERATORS:
            OnLoadAccelerators(param);
            break;
        case UNLOAD_ACCELERATORS:
            OnUnloadAccelerators(param);
            break;
        case LOAD_ITEMS:
            OnLoadItems(param);
            break;
        case LOAD_EXTRA_DATA:
            OnLoadExtraData(param);
            break;
    }
}

void CQPasteWndThread::OnDoQuery(void *param)
{
    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;

	OpenDatabase();

    static CEvent UpdateTimeEvent(TRUE, TRUE, _T("Ditto_Update_Clip_Time"), NULL);
    //If we pasted then wait for the time on the pasted event to be updated before we query the db
    DWORD dRet = WaitForSingleObject(UpdateTimeEvent, 2000);

    ResetEvent(m_SearchingEvent);
    long lTick = GetTickCount();

	CString CountSQL;
	{
		ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

	    pasteWnd->m_bFoundClipToSetFocusTo = false;
	    CountSQL = pasteWnd->m_CountSQL;
	    pasteWnd->m_mapCache.clear();
	    pasteWnd->m_loadItems.clear();
	    pasteWnd->m_bStopQuery = false;
		m_firstLoad = true;
	}

    long lRecordCount = 0;

    try
    {
        lRecordCount = m_db.execScalar(CountSQL);
        ::PostMessage(pasteWnd->m_hWnd, NM_SET_LIST_COUNT, lRecordCount, 0);
    }
    CATCH_SQLITE_EXCEPTION 

    SetEvent(m_SearchingEvent);

    Log(StrF(_T("Set list count = %d, time = %d"), lRecordCount, GetTickCount() - lTick));
}

void CQPasteWndThread::OnLoadItems(void *param)
{
    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;

    ResetEvent(m_SearchingEvent);

	OpenDatabase();

	while(true)
	{
		long startTick = GetTickCount();
	    int loadItemsIndex = 0;
	    int loadItemsCount = 0;
	    int loadCount = 0;
	    CString localSql;
	    bool clearFirstLoadItem = false;

		{
			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

		    if(pasteWnd->m_loadItems.size() > 0)
		    {
		        loadItemsIndex = pasteWnd->m_loadItems.begin()->x;
		        loadItemsCount = pasteWnd->m_loadItems.begin()->y - pasteWnd->m_loadItems.begin()->x;
		        localSql = pasteWnd->m_SQL;
		        pasteWnd->m_bStopQuery = false;
		        clearFirstLoadItem = true;
		    }
		}

	    if(clearFirstLoadItem)
	    {
	        Log(StrF(_T("Load Items start = %d, count = %d"), loadItemsIndex, loadItemsCount));

	        CString limit;
	        limit.Format(_T(" LIMIT %d OFFSET %d"), loadItemsCount, loadItemsIndex);
	        localSql += limit;

	        CMainTable table;

	        CppSQLite3Query q = m_db.execQuery(localSql);
	        while(!q.eof())
	        {
	            pasteWnd->FillMainTable(table, q);
	            table.m_listIndex = loadItemsIndex;

	            {
					ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

	                pasteWnd->m_mapCache[loadItemsIndex] = table;
	            }

	            if(pasteWnd->m_bStopQuery)
	            {
	                Log(StrF(_T("StopQuery called exiting filling cache count = %d"), loadItemsIndex));
	                break;
	            }

	            q.nextRow();

	            loadItemsIndex++;
	            loadCount++;

				if(m_firstLoad == false)
				{
	            	::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, table.m_lID, table.m_listIndex);
				}
	        }

			if(m_firstLoad)
			{
	        	::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, -2, 0);
			}
			else
			{
				::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, -1, 0);
			}

			m_firstLoad = false;

	        if(clearFirstLoadItem)
	        {
	            ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

	            pasteWnd->m_loadItems.erase(pasteWnd->m_loadItems.begin());
	        }

	        Log(StrF(_T("Load items End count = %d, time = %d"), loadCount, GetTickCount() - startTick));
	    }
		else
		{
			break;
		}
	}

    SetEvent(m_SearchingEvent);
}

void CQPasteWndThread::OnLoadExtraData(void *param)
{
    ResetEvent(m_SearchingEvent);

	OpenDatabase();

    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;

    Log(_T("Start of load extra data, Bitmaps/rtf"));

    std::list<CClipFormatQListCtrl> localFormats;
	{
		ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

		for (std::list<CClipFormatQListCtrl>::iterator it = pasteWnd->m_ExtraDataLoadItems.begin(); it != pasteWnd->m_ExtraDataLoadItems.end(); it++)
		{
			localFormats.push_back(*it);
		}
	    pasteWnd->m_ExtraDataLoadItems.clear();
	}

	for (std::list<CClipFormatQListCtrl>::iterator it = localFormats.begin(); it != localFormats.end(); it++)
    {
        if(theApp.GetClipData(it->m_lDBID, *it))
        {
            Log(StrF(_T("Loaded, extra data for clip %d, type: %d"), it->m_lDBID, it->m_cfType));

			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

            if(it->m_cfType == CF_DIB)
            {
                pasteWnd->m_cf_dibCache[it->m_lDBID] = *it;
                //the cache now owns the format data, set it to delete the data in the destructor
                pasteWnd->m_cf_dibCache[it->m_lDBID].m_autoDeleteData = true;
            }
            else if(it->m_cfType == theApp.m_RTFFormat)
            {
                pasteWnd->m_cf_rtfCache[it->m_lDBID] = *it;
                //the cache now owns the format data, set it to delete the data in the destructor
                pasteWnd->m_cf_rtfCache[it->m_lDBID].m_autoDeleteData = true;
            }

            ::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, it->m_lDBID, it->m_clipRow);
        }
        else
        {
			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

            if(it->m_cfType == CF_DIB)
            {
                CClipFormatQListCtrl localFormat;
                pasteWnd->m_cf_dibCache[it->m_lDBID] = *it;
            }
            else if(it->m_cfType == theApp.m_RTFFormat)
            {
                CClipFormatQListCtrl localFormat;
                pasteWnd->m_cf_rtfCache[it->m_lDBID] = *it;
            }
        }
    }

    SetEvent(m_SearchingEvent);
    Log(_T("End of load extra data, Bitmaps/rtf"));
}

void CQPasteWndThread::OnLoadAccelerators(void *param)
{
    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;
    OpenDatabase();
    pasteWnd->m_lstHeader.DestroyAndCreateAccelerator(TRUE, m_db);
}

void CQPasteWndThread::OnUnloadAccelerators(void *param)
{
    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;
    OpenDatabase();
    pasteWnd->m_lstHeader.DestroyAndCreateAccelerator(FALSE, m_db);
}

void CQPasteWndThread::OpenDatabase()
{
    try
    {
        if(m_dbPath.IsEmpty() == FALSE)
        {
            if(m_dbPath != CGetSetOptions::GetDBPath())
            {
                CloseDatabase();
            }
            else
            {
                return ;
            }
        }

        m_dbPath = CGetSetOptions::GetDBPath();

        DWORD dStart = GetTickCount();
        m_db.open(m_dbPath);
        Log(StrF(_T("Thread RunThread is starting time to open the database - %d"), GetTickCount() - dStart));
    }
    CATCH_SQLITE_EXCEPTION
}

void CQPasteWndThread::CloseDatabase()
{
    try
    {
        m_dbPath.Empty();
        m_db.close();
    }
    CATCH_SQLITE_EXCEPTION
}
