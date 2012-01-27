#include "stdafx.h"
#include "QPasteWndThread.h"
#include "Misc.h"
#include "Options.h"
#include "QPasteWnd.h"
#include "cp_main.h"

CQPasteWndThread::CQPasteWndThread(void)
{
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
}

void CQPasteWndThread::OnEvent(int eventId, void *param)
{
	DWORD startTick = GetTickCount();
	Log(StrF(_T("Start of OnEvent, eventId: %s"), EnumName((eCQPasteWndThreadEvents)eventId)));

    switch((eCQPasteWndThreadEvents)eventId)
    {
        case DO_SET_LIST_COUNT:
            OnSetListCount(param);
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

	DWORD length = GetTickCount() - startTick;
	Log(StrF(_T("End of OnEvent, eventId: %s, Time: %d(ms)"), EnumName((eCQPasteWndThreadEvents)eventId), length));
}

void CQPasteWndThread::OnSetListCount(void *param)
{
    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;

    static CEvent UpdateTimeEvent(TRUE, TRUE, _T("Ditto_Update_Clip_Time"), NULL);
    //If we pasted then wait for the time on the pasted event to be updated before we query the db
    DWORD dRet = WaitForSingleObject(UpdateTimeEvent, 2000);

    ResetEvent(m_SearchingEvent);
    long lTick = GetTickCount();

	CString CountSQL;
	{
		ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);
	    CountSQL = pasteWnd->m_CountSQL;
	}

    long lRecordCount = 0;

    try
    {
        lRecordCount = theApp.m_db.execScalar(CountSQL);
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

	while(true)
	{
		long startTick = GetTickCount();
	    int loadItemsIndex = 0;
	    int loadItemsCount = 0;
	    int loadCount = 0;
	    CString localSql;
	    bool clearFirstLoadItem = false;
		bool firstLoad = false;

		{
			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

		    if(pasteWnd->m_loadItems.size() > 0)
		    {
				firstLoad = (pasteWnd->m_loadItems.begin()->x == -1);
		        loadItemsIndex = max(pasteWnd->m_loadItems.begin()->x, 0);
		        loadItemsCount = pasteWnd->m_loadItems.begin()->y - pasteWnd->m_loadItems.begin()->x;
		        localSql = pasteWnd->m_SQL;
		        pasteWnd->m_bStopQuery = false;
		        clearFirstLoadItem = true;
		    }
		}

	    if(clearFirstLoadItem)
	    {
			try
			{
				Log(StrF(_T("Load Items start = %d, count = %d"), loadItemsIndex, loadItemsCount));

				int pos = loadItemsIndex;
				CString limit;
				limit.Format(_T(" LIMIT %d OFFSET %d"), loadItemsCount, loadItemsIndex);
				localSql += limit;

				CMainTable table;

				CppSQLite3Query q = theApp.m_db.execQuery(localSql);
				while(!q.eof())
				{
					pasteWnd->FillMainTable(table, q);

					{
						ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

						if(pos < pasteWnd->m_listItems.size())
						{
							pasteWnd->m_listItems[pos] = table;
						}
						else if(pos == pasteWnd->m_listItems.size())
						{
							pasteWnd->m_listItems.push_back(table);
						}
						else if(pos > pasteWnd->m_listItems.size())
						{
							for(int toAdd = pasteWnd->m_listItems.size(); toAdd < pos-1; toAdd++)
							{
								CMainTable empty;
								empty.m_lID = -1;
								pasteWnd->m_listItems.push_back(empty);
							}

							pasteWnd->m_listItems.push_back(table);
						}						
					}

					if(pasteWnd->m_bStopQuery)
					{
						Log(StrF(_T("StopQuery called exiting filling cache count = %d"), loadItemsIndex));
						break;
					}

					q.nextRow();

					if(firstLoad == false)
					{
	            		::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, table.m_lID, loadItemsIndex);
					}

					loadItemsIndex++;
					loadCount++;
					pos++;
				}

				if(firstLoad)
				{
					::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, -2, 0);
					//allow the next thread message to process, this should be the message to set the list count

					OnSetListCount(param);
					OnLoadAccelerators(param);
				}
				else
				{
					::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, -1, 0);
				}

				if(clearFirstLoadItem)
				{
					ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

					pasteWnd->m_loadItems.erase(pasteWnd->m_loadItems.begin());
				}

				Log(StrF(_T("Load items End count = %d, time = %d"), loadCount, GetTickCount() - startTick));
			}
			catch (CppSQLite3Exception& e)	\
			{								\
				Log(StrF(_T("ONLoadItems - SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));	\
				ASSERT(FALSE);				\
				break;
			}	
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
        if(theApp.GetClipData(it->m_dbId, *it))
        {
            Log(StrF(_T("Loaded, extra data for clip %d, type: %d"), it->m_dbId, it->m_cfType));

			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

            if(it->m_cfType == CF_DIB)
            {
                pasteWnd->m_cf_dibCache[it->m_dbId] = *it;
                //the cache now owns the format data, set it to delete the data in the destructor
                pasteWnd->m_cf_dibCache[it->m_dbId].m_autoDeleteData = true;
            }
            else if(it->m_cfType == theApp.m_RTFFormat)
            {
                pasteWnd->m_cf_rtfCache[it->m_dbId] = *it;
                //the cache now owns the format data, set it to delete the data in the destructor
                pasteWnd->m_cf_rtfCache[it->m_dbId].m_autoDeleteData = true;
            }

            ::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, it->m_dbId, it->m_clipRow);
        }
        else
        {
			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

            if(it->m_cfType == CF_DIB)
            {
                CClipFormatQListCtrl localFormat;
                pasteWnd->m_cf_dibCache[it->m_dbId] = *it;
            }
            else if(it->m_cfType == theApp.m_RTFFormat)
            {
                CClipFormatQListCtrl localFormat;
                pasteWnd->m_cf_rtfCache[it->m_dbId] = *it;
            }
        }
    }

    SetEvent(m_SearchingEvent);
    Log(_T("End of load extra data, Bitmaps/rtf"));
}

void CQPasteWndThread::OnLoadAccelerators(void *param)
{
    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;
    pasteWnd->m_lstHeader.DestroyAndCreateAccelerator(TRUE, theApp.m_db);
}

void CQPasteWndThread::OnUnloadAccelerators(void *param)
{
    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;
    pasteWnd->m_lstHeader.DestroyAndCreateAccelerator(FALSE, theApp.m_db);
}

CString CQPasteWndThread::EnumName(eCQPasteWndThreadEvents e)
{
	switch(e)
	{
	case DO_SET_LIST_COUNT:
		return _T("Load List Count");
	case LOAD_ACCELERATORS:
		return _T("Load Accelerators");
	case UNLOAD_ACCELERATORS:
		return _T("Unload Accelerators");
	case LOAD_ITEMS:
		return _T("Load clips");
	case LOAD_EXTRA_DATA:
		return _T("Load Extra Data (rtf/bitmaps)");
	}

	return _T("");
}