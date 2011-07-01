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
		DO_SET_LIST_COUNT, 
		LOAD_ACCELERATORS, 
		UNLOAD_ACCELERATORS, 
		LOAD_ITEMS, 
		LOAD_EXTRA_DATA, 

        ECQPASTEWNDTHREADEVENTS_COUNT  //must be last

    };

    void FireSetListCount()
    {
        FireEvent(DO_SET_LIST_COUNT);
    }
    void FireLoadItems(bool firstLoad)
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

    void OnSetListCount(void *param);
    void OnLoadItems(void *param);
    void OnLoadExtraData(void *param);
    void OnLoadAccelerators(void *param);
    void OnUnloadAccelerators(void *param);

	CString EnumName(eCQPasteWndThreadEvents e);
};
