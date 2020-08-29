#include "stdafx.h"
#include "QPasteWndThread.h"
#include "Misc.h"
#include "Options.h"
#include "QPasteWnd.h"
#include "cp_main.h"
#include <vector>
#include <algorithm>

CQPasteWndThread::CQPasteWndThread(void)
{
	m_rowHeight = 0;
	m_threadName = "CQPasteWndThread";
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
				Log(StrF(_T("Load Items start = %d, count = %d, list size: %d"), loadItemsIndex, loadItemsCount, pasteWnd->m_listItems.size()));

				int pos = loadItemsIndex;
				CString limit;
				limit.Format(_T(" LIMIT %d OFFSET %d"), loadItemsCount, loadItemsIndex);
				localSql += limit;

				CMainTable table;

				CppSQLite3Query q = theApp.m_db.execQuery(localSql);
				while(!q.eof())
				{
					pasteWnd->FillMainTable(table, q);

					int updateIndex = -1;

					{
						ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

						if (pos < pasteWnd->m_listItems.size())
						{
							pasteWnd->m_listItems[pos] = table;

							updateIndex = pos;

							//Log(StrF(_T("updating list pos = %d, id: %d, size: %d"), pos, table.m_lID, pasteWnd->m_listItems.size() - 1));
						}
						else if (pos == pasteWnd->m_listItems.size())
						{
							pasteWnd->m_listItems.push_back(table);
							updateIndex = (int)pasteWnd->m_listItems.size() - 1;
							//Log(StrF(_T("adding (same size) list pos = %d, id: %d, size: %d"), pasteWnd->m_listItems.size()-1, table.m_lID, pasteWnd->m_listItems.size() - 1));
						}
						else if (pos > pasteWnd->m_listItems.size())
						{
							for (int toAdd = (int)pasteWnd->m_listItems.size()-1; toAdd < pos - 1; toAdd++)
							{
								CMainTable empty;
								empty.m_lID = -1;
								pasteWnd->m_listItems.push_back(empty);

								//Log(StrF(_T("adding dummy row size: %d"), pasteWnd->m_listItems.size()-1));
							}

							pasteWnd->m_listItems.push_back(table);

							updateIndex = (int)pasteWnd->m_listItems.size() - 1;

							//Log(StrF(_T("adding list pos = %d, id: %d, size: %d"), pasteWnd->m_listItems.size()-1, table.m_lID, pasteWnd->m_listItems.size() - 1));
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
						/*if (updateIndex != loadItemsIndex)
						{
							Log(StrF(_T("index difference old: %d, new: %d"), loadItemsIndex, updateIndex));
						}*/

	            		::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, table.m_lID, updateIndex);
					}

					loadItemsIndex++;
					loadCount++;
					pos++;
				}

				DWORD loadCount = GetTickCount() - startTick;
				DWORD countCountStart = GetTickCount();
				DWORD countCount = 0;
				DWORD acceleratorCount = 0;

				if(firstLoad)
				{
					::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, -2, 0);
					//allow the next thread message to process, this should be the message to set the list count

					OnSetListCount(param);
					
					countCount = GetTickCount() - countCountStart;
					DWORD acceleratorCountStart = GetTickCount();
					 
					OnLoadAccelerators(param);

					acceleratorCount = GetTickCount() - acceleratorCountStart;
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

				Log(StrF(_T("Load items End count = %d, Total Time = %d, LoadItems: %d, Count: %d, Accel: %d"), loadCount, GetTickCount() - startTick, loadCount, countCount, acceleratorCount));
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

void ReduceMapItems(CF_DibTypeMap &mapItem, CCriticalSection &critSection, CString mapName)
{
	ATL::CCritSecLock csLock(critSection.m_sect);

	int maxSize = 50;
	int reduceToSize = 30;

	if (mapItem.size() > maxSize)
	{
		//create a vector so we can sort and keep the last x number of events
		vector<INT64> counterArray;
		for (CF_DibTypeMap::iterator iterDib = mapItem.begin(); iterDib != mapItem.end(); iterDib++)
		{
			counterArray.push_back(iterDib->second.m_counter);
		}
		std::sort(counterArray.begin(), counterArray.end());
		counterArray.erase(counterArray.begin(), counterArray.end() - reduceToSize);

		//remove the oldest x number if bitmaps
		for (CF_DibTypeMap::iterator iterDib = mapItem.begin(); iterDib != mapItem.end();)
		{
			if (std::binary_search(counterArray.begin(), counterArray.end(), iterDib->second.m_counter) == false)
			{
				Log(StrF(_T("reduced size of %s cache, Id: %d, Row: %d"), mapName, iterDib->second.m_parentId, iterDib->second.m_clipRow));

				mapItem.erase(iterDib++);
			}
			else
			{
				++iterDib;
			}
		}

		Log(StrF(_T("reduced size of %s cache, count: %d"), mapName, mapItem.size()));
	}
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
		bool loadClip = true;

		if (it->m_cfType == CF_DIB)
		{
			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

			CF_DibTypeMap::iterator iterDib = pasteWnd->m_cf_dibCache.find(it->m_parentId);
			if (iterDib != pasteWnd->m_cf_dibCache.end())
			{
				loadClip = false;
			}
			else
			{
				CF_NoDibTypeMap::iterator iterNoDib = pasteWnd->m_cf_NO_dibCache.find(it->m_parentId);
				if (iterNoDib != pasteWnd->m_cf_NO_dibCache.end())
				{
					loadClip = false;
				}
			}
		}
		else if (it->m_cfType == theApp.m_RTFFormat)
		{
			ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

			CF_DibTypeMap::iterator iterDib = pasteWnd->m_cf_rtfCache.find(it->m_parentId);
			if (iterDib != pasteWnd->m_cf_rtfCache.end())
			{
				loadClip = false;
			}
			else
			{
				CF_NoDibTypeMap::iterator iterNoRtf = pasteWnd->m_cf_NO_rtfCache.find(it->m_parentId);
				if (iterNoRtf != pasteWnd->m_cf_NO_rtfCache.end())
				{
					loadClip = false;
				}
			}
		}

		if (loadClip)
		{
			DWORD startLoadClipData = GetTickCount();

			BOOL foundClipData = theApp.GetClipData(it->m_parentId, *it);
			if (foundClipData == false &&
				it->m_cfType == CF_DIB)
			{
				it->Free();
				it->m_cfType = theApp.m_PNG_Format;

				foundClipData = theApp.GetClipData(it->m_parentId, *it);
			}

			if (foundClipData)
			{
				DWORD timeTook = GetTickCount() - startLoadClipData;
				if (timeTook > 20)
				{
					Log(StrF(_T("GetClipData for clip %d, took: %d"), it->m_parentId, timeTook));
				}

				if (it->m_cfType == CF_DIB ||
					it->m_cfType == theApp.m_PNG_Format)
				{
					DWORD startConvertImage = GetTickCount();

					HDC dc = GetDC(NULL);

					it->GetDibFittingToHeight(CDC::FromHandle(dc), m_rowHeight);

					ReleaseDC(NULL, dc);

					DWORD timeTook = GetTickCount() - startConvertImage;
					if (timeTook > 20)
					{
						Log(StrF(_T("GetDibFittingToHeight for clip %d, took: %d"), it->m_parentId, GetTickCount() - startConvertImage));
					}

					ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

					pasteWnd->m_cf_dibCache[it->m_parentId] = *it;
					//the cache now owns the format data, set it to delete the data in the destructor
					pasteWnd->m_cf_dibCache[it->m_parentId].m_autoDeleteData = true;

					Log(StrF(_T("Loaded, extra data for clipId: %d, Row: %d image cache count: %d"), it->m_parentId, it->m_clipRow, pasteWnd->m_cf_dibCache.size()));
				}
				else if (it->m_cfType == theApp.m_RTFFormat)
				{
					ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

					pasteWnd->m_cf_rtfCache[it->m_parentId] = *it;
					it->m_autoDeleteData = false;
					//the cache now owns the format data, set it to delete the data in the destructor
					pasteWnd->m_cf_rtfCache[it->m_parentId].m_autoDeleteData = true;

					Log(StrF(_T("Loaded, extra data for clip %d, rtf cache count: %d"), it->m_parentId, pasteWnd->m_cf_rtfCache.size()));
				}

				::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, it->m_parentId, it->m_clipRow);
			}
			else
			{
				ATL::CCritSecLock csLock(pasteWnd->m_CritSection.m_sect);

				if (it->m_cfType == CF_DIB ||
					it->m_cfType == theApp.m_PNG_Format)
				{
					pasteWnd->m_cf_NO_dibCache[it->m_parentId] = true;
				}
				else if (it->m_cfType == theApp.m_RTFFormat)
				{
					pasteWnd->m_cf_NO_rtfCache[it->m_parentId] = true;
				}
			}
		}

		if (it->m_cfType == CF_DIB)
		{
			ReduceMapItems(pasteWnd->m_cf_dibCache, pasteWnd->m_CritSection, _T("image"));
		}
		else if (it->m_cfType == theApp.m_RTFFormat)
		{
			ReduceMapItems(pasteWnd->m_cf_rtfCache, pasteWnd->m_CritSection, _T("rtf"));
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