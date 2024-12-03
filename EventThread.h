#pragma once

#include <vector>
#include <map>

typedef std::map<HANDLE, int> EventMapType;

class CEventThread
{
public:
	CEventThread(void);
	~CEventThread(void);

	static unsigned int __stdcall EventThreadFnc(void* thisptr);

protected:
	virtual void OnEvent(int eventId, void *param)	{ return; }
	virtual void OnTimeOut(void *param) { return; }
	void RunThread();
	bool UndoFireEvent(int eventId);
	HANDLE GetHandle(int eventId);
	bool RemoveEvent(int eventId);

	UINT m_threadID;
	HANDLE m_thread;
	HANDLE m_hEvt;
	EventMapType m_eventMap;
	bool m_exitThread;
	bool m_threadRunning;
	bool m_threadWasStarted;
	void *m_param;
	int m_waitTimeout;
	CString m_threadName;
	CCriticalSection m_lock;

public:
	void Start(void *param = NULL);
	void Stop(int waitTime = 5000); 
	void AddEvent(int eventId);
	void AddEvent(int eventId, CString name);
	void AddEvent(int eventId, HANDLE handle);
	bool FireEvent(int eventId);
	bool IsCancelled() { return m_exitThread; }
	void CancelThread() { m_exitThread = true; }
	bool IsRunning() { return m_threadRunning; }
	bool ThreadWasStarted() { return m_threadWasStarted; }
	void WaitForThreadToExit(int waitTime);
	void GetHandleVector(std::vector<HANDLE> &handles);
	void CheckForRebuildHandleVector(std::vector<HANDLE>& handles);
};

