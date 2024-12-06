#include "StdAfx.h"
#include "EventThread.h"
#include "Misc.h"

#define EXIT_EVENT -1
#define REBUILD_EVENTS -2

CEventThread::CEventThread(void)
{
	m_hEvt = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_waitTimeout = INFINITE;
	m_threadRunning = false;
	m_exitThread = false;
	m_threadWasStarted = false;

	AddEvent(EXIT_EVENT);
	AddEvent(REBUILD_EVENTS);
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

	{
		ATL::CCritSecLock csLock(m_lock.m_sect);
		m_eventMap[handle] = eventId;
	}

	if (m_threadRunning)
	{
		FireEvent(REBUILD_EVENTS);
	}
}

void CEventThread::AddEvent(int eventId, HANDLE handle)
{
	{
		ATL::CCritSecLock csLock(m_lock.m_sect);
		m_eventMap[handle] = eventId;
	}

	if (m_threadRunning)
	{
		FireEvent(REBUILD_EVENTS);
	}
}

void CEventThread::AddEvent(int eventId, CString name)
{
	//handle creating events cross users/cross process
	//https://stackoverflow.com/questions/29976596/shared-global-event-between-a-service-user-mode-processes-doesnt-work
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = &sd;

	HANDLE handle = CreateEvent(&sa, FALSE, FALSE, name);

	{
		ATL::CCritSecLock csLock(m_lock.m_sect);
		m_eventMap[handle] = eventId;
	}

	if (m_threadRunning)
	{
		FireEvent(REBUILD_EVENTS);
	}
}

bool CEventThread::FireEvent(int eventId)
{
	HANDLE eventHandle = GetHandle(eventId);
	if(eventHandle != nullptr)
	{
		SetEvent(eventHandle);
		return true;
	}	

	return false;
}

bool CEventThread::UndoFireEvent(int eventId)
{
	HANDLE eventHandle = GetHandle(eventId);
	if (eventHandle != nullptr)
	{
		ResetEvent(eventHandle);
		return true;
	}

	return false;
}

HANDLE CEventThread::GetHandle(int eventId)
{	
	ATL::CCritSecLock csLock(m_lock.m_sect);
	for (auto it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		if (it->second == eventId)
		{
			return it->first;
		}
	}

	return nullptr;
}

bool CEventThread::RemoveEvent(int eventId)
{
	ATL::CCritSecLock csLock(m_lock.m_sect);
	for (auto it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		if (it->second == eventId)
		{
			if (m_threadRunning)
			{
				FireEvent(REBUILD_EVENTS);
			}

			CloseHandle(it->first);
			m_eventMap.erase(it);

			return true;
		}
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

void CEventThread::GetHandleVector(std::vector<HANDLE> &handles)
{
	ATL::CCritSecLock csLock(m_lock.m_sect);
	handles.clear();
	for (auto it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		if (it->first != 0)
		{
			handles.push_back(it->first);
		}
	}
}

void CEventThread::CheckForRebuildHandleVector(std::vector<HANDLE>& handles)
{
	ATL::CCritSecLock csLock(m_lock.m_sect);
	for (auto it = m_eventMap.begin(); it != m_eventMap.end(); it++)
	{
		if (it->second == REBUILD_EVENTS)
		{
			DWORD result = WaitForSingleObject(it->first, 0);
			if (result == WAIT_OBJECT_0)
			{
				GetHandleVector(handles);
			}
			break;
		}
	}
}

void CEventThread::RunThread()
{
	Log(StrF(_T("Start of CEventThread::RunThread() Name: %s"), m_threadName));

	m_threadRunning = true;
	m_threadWasStarted = true;
	std::vector<HANDLE> handles;

	GetHandleVector(handles);

	SetEvent(m_hEvt);
	ResetEvent(m_hEvt);

	while(m_exitThread == false)
	{
		CheckForRebuildHandleVector(handles);

		DWORD event = WaitForMultipleObjects((DWORD)handles.size(), handles.data(), FALSE, m_waitTimeout);

		if(event == WAIT_FAILED)
		{
			const DWORD errorMessageId = GetLastError();
			LPSTR messageBuffer = nullptr;
			size_t size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

			CString message(messageBuffer, (int)size);

			LocalFree(messageBuffer);

			Log(StrF(_T("CEventThread::RunThread() Error, error: %s - Name %s"), message, m_threadName));

			Sleep(1000);
		}
		else if(event == WAIT_TIMEOUT)
		{
			OnTimeOut(m_param);
		}
		else
		{
			const int handleIndex = event - WAIT_OBJECT_0;
			if (handleIndex < 0 || handleIndex >= handles.size())
			{
				Log(StrF(_T("CEventThread::RunThread() Error, Invalid handle index, index: %d, size: %d - Name %s"), handleIndex, handles.size(), m_threadName));
				continue;
			}

			HANDLE firedHandle = handles[handleIndex];
			const int eventId = m_eventMap[firedHandle];
			if(eventId == EXIT_EVENT)
			{				
				break;
			}
			else if (eventId == REBUILD_EVENTS)
			{
				GetHandleVector(handles);				
			}
			else
			{
				Log(StrF(_T("Start of CEventThread::RunThread() - OnEvent %d - Name %s"), eventId, m_threadName));
				OnEvent(eventId, m_param);
				Log(StrF(_T("End of CEventThread::RunThread() - OnEvent %d - Name: %s"), eventId, m_threadName));
			}
		}
	}

	UndoFireEvent(EXIT_EVENT);

	SetEvent(m_hEvt);

	Log(StrF(_T("End of CEventThread::RunThread() Name: %s"), m_threadName));

	m_threadRunning = false;
}
