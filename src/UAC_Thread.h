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
		UAC_COPY,
		UAC_CUT,
		UAC_EXIT,

		eUacThreadEvents_COUNT  //must be last
	};

	int m_processId;

	void FirePaste()
	{
		FireEvent(UAC_PASTE);
	}

	void FireCopy()
	{
		FireEvent(UAC_COPY);
	}

	void FireCut()
	{
		FireEvent(UAC_CUT);
	}

	void FireExit()
	{
		FireEvent(UAC_EXIT);
	}

	bool UACPaste();
	bool UACCopy();
	bool UACCut();

private:
	virtual void OnEvent(int eventId, void *param);
	virtual void OnTimeOut(void *param);
	CString EnumName(eUacThreadEvents e);
	bool StartProcess();
};

