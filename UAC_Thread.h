#pragma once

#include "EventThread.h"

class CUAC_Thread : public CEventThread
{
public:
	CUAC_Thread(int processId);
	~CUAC_Thread(void);

	enum eUacThreadEvents
	{
		UAC_PASTE, 
		UAC_EXIT,

		eUacThreadEvents_COUNT  //must be last
	};

	int m_processId;

	void FirePaste()
	{
		FireEvent(UAC_PASTE);
	}
	void FireExit()
	{
		FireEvent(UAC_EXIT);
	}

	void UACPaste();

private:
	virtual void OnEvent(int eventId, void *param);
	virtual void OnTimeOut(void *param);
	CString EnumName(eUacThreadEvents e);
};

