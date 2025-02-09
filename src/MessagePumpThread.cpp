#include "StdAfx.h"
#include "MessagePumpThread.h"


CMessagePumpThread::CMessagePumpThread(void)
{
}


CMessagePumpThread::~CMessagePumpThread(void)
{
}

UINT CMessagePumpThread::MessagePumpThread(void* thisptr) 
{
	CMessagePumpThread *threadClass = (CMessagePumpThread*)thisptr;
	threadClass->RunMessagePump();
	return 0;
}

void CMessagePumpThread::Start() 
{
	m_hEvt = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_thread = _beginthreadex(NULL, 0, MessagePumpThread, this, 0, &m_threadID);
	if (0 == m_thread)
	{
		throw "Could not create thread";
	}

	// now wait until the thread is up and really running
	if (WAIT_OBJECT_0 != WaitForSingleObject(m_hEvt, 10000L))	// 10 seconds
	{
		throw "Timeout waiting for thread to start";
	}
}

void CMessagePumpThread::Stop() 
{
	PostThreadMessage(m_threadID, WM_QUIT, 0, 0L);
	if (WAIT_OBJECT_0 != WaitForSingleObject(m_hEvt, 10000L))
	{
		throw "Timeout waiting for thread to stop";
	}
};

void CMessagePumpThread::PostMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PostThreadMessage(m_threadID, msg, wParam, lParam);
}

void CMessagePumpThread::RunMessagePump()
{
	MSG msg;

	// create the message queue
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	// we're far enough to let the creator know we're running
	SetEvent(m_hEvt);

	while(true)
	{
		BOOL bRet = GetMessage(&msg, NULL, 0, 0);
		if (0 >= bRet) // just read the specs, it's a "MickeySoft BOOL" and can be TRUE, FALSE or -1 (right on)
		{
			SetEvent(m_hEvt);
			break;	
		}

		TakeMsg(msg.message, msg.wParam, msg.lParam);
	}
}
