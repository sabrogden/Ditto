#pragma once

#include "EventThread.h"
#include "Clip.h"
#include <afxmt.h>


class CAutoSendToClientThread : public CEventThread
{
public:
	CAutoSendToClientThread(void);
	~CAutoSendToClientThread(void);

	enum eCAutoSendToClientThreadEvents
	{ 
		SEND_TO_CLIENTS,

		ECAUTOSENDTOCLIENTTHREADEVENTS_COUNT  //must be last
	};

	void FireSendToClient(CClipList *pClipList);

protected:
	virtual void OnEvent(int eventId, void *param);
	virtual void OnTimeOut(void *param);

	void OnSendToClient();
	bool SendToClient(CClipList *pClipList);

	CCriticalSection m_cs;
	CClipList m_saveClips;
};

