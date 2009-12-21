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

    static CEvent UpdateTimeEvent(TRUE, TRUE, _T("Ditto_Update_Clip_Time"), NULL);
    //If we pasted then wait for the time on the pasted event to be updated before we query the db
    DWORD dRet = WaitForSingleObject(UpdateTimeEvent, 2000);

    ResetEvent(m_SearchingEvent);
    long lTick = GetTickCount();

    pasteWnd->m_CritSection.Lock();
    pasteWnd->m_bFoundClipToSetFocusTo = false;
    CString CountSQL(pasteWnd->m_CountSQL);
    pasteWnd->m_mapCache.clear();
    pasteWnd->m_loadItems.clear();
    pasteWnd->m_bStopQuery = false;
    pasteWnd->m_CritSection.Unlock();

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

    long startTick = GetTickCount();
    int loadItemsIndex = 0;
    int loadItemsCount = 0;
    int loadCount = 0;
    CString localSql;
    bool clearFirstLoadItem = false;

    pasteWnd->m_CritSection.Lock();
    if(pasteWnd->m_loadItems.size() > 0)
    {
        loadItemsIndex = pasteWnd->m_loadItems[0].x;
        loadItemsCount = pasteWnd->m_loadItems[0].y - pasteWnd->m_loadItems[0].x;
        localSql = pasteWnd->m_SQL;
        pasteWnd->m_bStopQuery = false;
        clearFirstLoadItem = true;
    }
    pasteWnd->m_CritSection.Unlock();

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

            pasteWnd->m_CritSection.Lock();
            {
                pasteWnd->m_mapCache[loadItemsIndex] = table;
            }
            pasteWnd->m_CritSection.Unlock();

            if(pasteWnd->m_bStopQuery)
            {
                Log(StrF(_T("StopQuery called exiting filling cache count = %d"), loadItemsIndex));
                break;
            }

            q.nextRow();

            loadItemsIndex++;
            loadCount++;

            ::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, table.m_lID, table.m_listIndex);
        }

        ::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW,  - 1, 0);

        if(clearFirstLoadItem)
        {
            pasteWnd->m_CritSection.Lock();
            pasteWnd->m_loadItems.erase(pasteWnd->m_loadItems.begin());
            pasteWnd->m_CritSection.Unlock();
        }

        Log(StrF(_T("Load items End count = %d, time = %d"), loadCount, GetTickCount() - startTick));
    }

    SetEvent(m_SearchingEvent);
}

void CQPasteWndThread::OnLoadExtraData(void *param)
{
    ResetEvent(m_SearchingEvent);

    CQPasteWnd *pasteWnd = (CQPasteWnd*)param;

    Log(_T("Start of load extra data, Bitmaps/rtf"));

    std::vector < CClipFormatQListCtrl > localFormats;
    pasteWnd->m_CritSection.Lock();
    int count = pasteWnd->m_ExtraDataLoadItems.size();
    for(int i = 0; i < count; i++)
    {
        localFormats.push_back(pasteWnd->m_ExtraDataLoadItems[i]);
    }
    pasteWnd->m_ExtraDataLoadItems.clear();
    pasteWnd->m_CritSection.Unlock();

    count = localFormats.size();
    for(int i = 0; i < count; i++)
    {
        if(theApp.GetClipData(localFormats[i].m_lDBID, localFormats[i]))
        {
            Log(StrF(_T("Loaded, extra data for clip %d, type: %d"), localFormats[i].m_lDBID, localFormats[i].m_cfType));

            pasteWnd->m_CritSection.Lock();

            if(localFormats[i].m_cfType == CF_DIB)
            {
                pasteWnd->m_cf_dibCache[localFormats[i].m_lDBID] = localFormats[i];
                //the cache now owns the format data, set it to delete the data in the destructor
                pasteWnd->m_cf_dibCache[localFormats[i].m_lDBID].m_autoDeleteData = true;
            }
            else if(localFormats[i].m_cfType == theApp.m_RTFFormat)
            {
                pasteWnd->m_cf_rtfCache[localFormats[i].m_lDBID] = localFormats[i];
                //the cache now owns the format data, set it to delete the data in the destructor
                pasteWnd->m_cf_rtfCache[localFormats[i].m_lDBID].m_autoDeleteData = true;
            }

            pasteWnd->m_CritSection.Unlock();

            ::PostMessage(pasteWnd->m_hWnd, NM_REFRESH_ROW, localFormats[i].m_lDBID, localFormats[i].m_clipRow);
        }
        else
        {
            pasteWnd->m_CritSection.Lock();

            if(localFormats[i].m_cfType == CF_DIB)
            {
                CClipFormatQListCtrl localFormat;
                pasteWnd->m_cf_dibCache[localFormats[i].m_lDBID] = localFormat;
            }
            else if(localFormats[i].m_cfType == theApp.m_RTFFormat)
            {
                CClipFormatQListCtrl localFormat;
                pasteWnd->m_cf_rtfCache[localFormats[i].m_lDBID] = localFormat;
            }

            pasteWnd->m_CritSection.Unlock();
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
