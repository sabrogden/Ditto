#include "StdAfx.h"
#include "EventThread.h"
#include "Misc.h"

#define EXIT_EVENT -1

CEventThread::CEventThread(void)
{
	AddEvent(EXIT_EVENT);
	m_hEvt = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_waitTimeout = INFINITE;
	m_threadRunning = false;
	m_exitThread = false;
	m_threadWasStarted = false;
}

CEventThread::~CEventThread(void)
{
	Stop();

	for(EventMapType::iterator it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		CloseHandle(it->first);
	}
}

UINT CEventThread::EventThreadFnc(void* thisptr) 
{
	CEventThread *threadClass = (CEventThread*)thisptr;
	threadClass->RunThread();
	return 0;
}

void CEventThread::AddEvent(int eventId)
{
	HANDLE handle = CreateEvent(NULL, FALSE, FALSE, _T(""));
	m_eventMap[handle] = eventId;
}

void CEventThread::AddEvent(int eventId, CString name)
{
	HANDLE handle = CreateEvent(NULL, FALSE, FALSE, name);
	m_eventMap[handle] = eventId;
}

bool CEventThread::FireEvent(int eventId)
{
	//Log(StrF(_T("Begin FireEvent, eventId: %d"), eventId));

	bool ret = false;

	HANDLE eventHandle = NULL;
	for(EventMapType::iterator it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		if(it->second == eventId)
		{
			eventHandle = it->first;
			break;
		}
	}

	if(eventHandle != NULL)
	{
		SetEvent(eventHandle);
		ret = true;
	}
	
	//Log(StrF(_T("End FireEvent, eventId: %d"), eventId));

	return false;
}

bool CEventThread::UndoFireEvent(int eventId)
{
	HANDLE eventHandle = NULL;
	for(EventMapType::iterator it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		if(it->second == eventId)
		{
			eventHandle = it->first;
			break;
		}
	}

	if(eventHandle != NULL)
	{
		ResetEvent(eventHandle);
		return true;
	}

	return false;
}

void CEventThread::Start(void *param) 
{
	if(m_threadRunning == false)
	{
		ResetEvent(m_hEvt);
		m_exitThread = false;
		m_param = param;
		m_thread = (HANDLE)_beginthreadex(NULL, 0, EventThreadFnc, this, 0, &m_threadID);

		// now wait until the thread is up and really running
		WaitForSingleObject(m_hEvt, 1000);
	}
	else
	{
		UndoFireEvent(EXIT_EVENT);
	}
}

void CEventThread::WaitForThreadToExit(int waitTime)
{
	WaitForSingleObject(m_hEvt, waitTime);
}

void CEventThread::Stop(int waitTime) 
{
	Log(StrF(_T("Start of CEventThread::Stop(int waitTime) %d - Name: %s"), waitTime, m_threadName));

	if(m_threadRunning)
	{
		m_exitThread = true;	
		FireEvent(EXIT_EVENT);

		if(waitTime > 0)
		{
			if (WAIT_OBJECT_0 != WaitForSingleObject(m_hEvt, waitTime))
			{
				Log(_T("Start of TerminateThread CEventThread::Stop(int waitTime) "));
				TerminateThread(m_thread, 0);
				Log(_T("End of TerminateThread CEventThread::Stop(int waitTime) "));
				m_threadRunning = false;
			}
		}
	}

	Log(StrF(_T("End of CEventThread::Stop(int waitTime) %d - Name: %s"), waitTime, m_threadName));
};

void CEventThread::RunThread()
{
	Log(StrF(_T("Start of CEventThread::RunThread() Name: %s"), m_threadName));

	m_threadRunning = true;
	m_threadWasStarted = true;
	HANDLE *pHandleArray = new HANDLE[m_eventMap.size()];

	int indexPos = 0;
	for(EventMapType::iterator it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		pHandleArray[indexPos] = it->first;
		indexPos++;
	}

	SetEvent(m_hEvt);
	ResetEvent(m_hEvt);

	while(m_exitThread == false)
	{
		DWORD event = WaitForMultipleObjects((DWORD)m_eventMap.size(), pHandleArray, FALSE, m_waitTimeout);

		if(event == WAIT_FAILED)
		{
			LPVOID lpMsgBuf = NULL;
			DWORD dwErr = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
							FORMAT_MESSAGE_FROM_SYSTEM |
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							dwErr,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
							(LPTSTR) &lpMsgBuf,
							0,
							NULL);

			ASSERT(!lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
		else if(event == WAIT_TIMEOUT)
		{
			OnTimeOut(m_param);
		}
		else
		{
			HANDLE firedHandle = pHandleArray[event - WAIT_OBJECT_0];
			int eventId = m_eventMap[firedHandle];
			if(eventId == EXIT_EVENT)
			{				
				break;
			}
			else
			{
				Log(StrF(_T("Start of CEventThread::RunThread() - OnEvent %d - Name %s"), eventId, m_threadName));
				OnEvent(eventId, m_param);
				Log(StrF(_T("End of CEventThread::RunThread() - OnEvent %d - Name: %d"), eventId, m_threadName));
			}
		}
	}

	UndoFireEvent(EXIT_EVENT);

	SetEvent(m_hEvt);

	Log(StrF(_T("End of CEventThread::RunThread() Name: %s"), m_threadName));

	m_threadRunning = false;
}
