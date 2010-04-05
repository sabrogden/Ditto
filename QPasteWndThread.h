#pragma once
#include "EventThread.h"
#include "sqlite/CppSQLite3.h"

class CQPasteWndThread: public CEventThread
{
public:
    CQPasteWndThread(void);
    ~CQPasteWndThread(void);

    enum eCQPasteWndThreadEvents
    {
        DO_QUERY, LOAD_ACCELERATORS, UNLOAD_ACCELERATORS, LOAD_ITEMS, LOAD_EXTRA_DATA, 

        ECQPASTEWNDTHREADEVENTS_COUNT  //must be last

    };

    void FireDoQuery()
    {
        FireEvent(DO_QUERY);
    }
    void FireLoadItems()
    {
        FireEvent(LOAD_ITEMS);
    }
    void FireLoadExtraData()
    {
        FireEvent(LOAD_EXTRA_DATA);
    }
    void FireLoadAccelerators()
    {
        FireEvent(LOAD_ACCELERATORS);
    }
    void FireUnloadAccelerators()
    {
        FireEvent(UNLOAD_ACCELERATORS);
    }

    HANDLE m_SearchingEvent;

protected:
    virtual void OnEvent(int eventId, void *param);
    virtual void OnTimeOut(void *param);

    void OnDoQuery(void *param);
    void OnLoadItems(void *param);
    void OnLoadExtraData(void *param);
    void OnLoadAccelerators(void *param);
    void OnUnloadAccelerators(void *param);

    void OpenDatabase();
    void CloseDatabase();

    CppSQLite3DB m_db;
    CString m_dbPath;
	bool m_firstLoad;
};
