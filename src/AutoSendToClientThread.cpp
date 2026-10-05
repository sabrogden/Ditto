#include "stdafx.h"
#include "AutoSendToClientThread.h"
#include "Misc.h"
#include "Options.h"
#include "CP_Main.h"
#include "client.h"
#include <thread>
#include <vector>

namespace
{
	CCriticalSection g_sendClientStateCs;
}

CAutoSendToClientThread::CAutoSendToClientThread(void)
{
	m_waitTimeout = 30000;
	m_threadName = "CAutoSendToClientThread";
	for(int eventEnum = 0; eventEnum < ECAUTOSENDTOCLIENTTHREADEVENTS_COUNT; eventEnum++)
	{
		AddEvent(eventEnum);
	}
}

CAutoSendToClientThread::~CAutoSendToClientThread(void)
{
}

void CAutoSendToClientThread::FireSendToClient(CClipList *pClipList)
{
	Start();

	ATL::CCritSecLock csLock(m_cs.m_sect);
	if(m_threadRunning)
	{
		Log(_T("Adding clip to send to client in thread"));

		POSITION pos = pClipList->GetHeadPosition();
		while(pos)
		{
			CClip *pClip = pClipList->GetNext(pos);
		
			m_saveClips.AddTail(pClip);
		}

		pClipList->RemoveAll();
		
		FireEvent(SEND_TO_CLIENTS);
	}
	else
	{
		Log(_T("Error creating thread to send to clients"));
	}
}

void CAutoSendToClientThread::OnTimeOut(void *param)
{
	Stop(-1);
}

void CAutoSendToClientThread::OnEvent(int eventId, void *param)
{
	switch((eCAutoSendToClientThreadEvents)eventId)
	{
	case SEND_TO_CLIENTS:
		OnSendToClient();
		break;
	}
}

void CAutoSendToClientThread::OnSendToClient()
{
	CClipList *pLocalClips = new CClipList();

	//Save the clips locally
	{
		ATL::CCritSecLock csLock(m_cs.m_sect);

		POSITION pos;
		CClip* pClip;

		pos = m_saveClips.GetHeadPosition();
		while(pos)
		{
			pClip = m_saveClips.GetNext(pos);
			pLocalClips->AddTail(pClip);
		}

		//pLocalClips now own, the clips
		m_saveClips.RemoveAll();
	}

	// Do not wait for this batch in the event thread. A destination that is
	// offline can otherwise block the next clipboard change until it times out.
	std::thread(&CAutoSendToClientThread::SendBatch, pLocalClips).detach();
}

void CAutoSendToClientThread::SendBatch(CClipList *pClipList)
{
	std::vector<std::thread> sendThreads;
	for(int nClient = 0; nClient < MAX_SEND_CLIENTS; nClient++)
	{
		if(CGetSetOptions::m_SendClients[nClient].bSendAll &&
			CGetSetOptions::m_SendClients[nClient].csIP.GetLength() > 0)
		{
			// Each connection can wait independently, so an offline computer does not
			// delay delivery to the other configured computers.
			CString clientIp = CGetSetOptions::m_SendClients[nClient].csIP;
			sendThreads.emplace_back(&CAutoSendToClientThread::SendToClient, pClipList, nClient, clientIp);
		}
	}

	for(auto &sendThread : sendThreads)
	{
		sendThread.join();
	}

	delete pClipList;
}

bool CAutoSendToClientThread::SendToClient(CClipList *pClipList, int nClient, const CString &clientIp)
{
	LogSendRecieveInfo(StrF(_T("@@@@@@@@@@@@@@@ - START OF SendClientThread to %s - @@@@@@@@@@@@@@@"), clientIp));

	if(pClipList == NULL)
	{
		LogSendRecieveInfo("ERROR if(pClipList == NULL)");
		return FALSE;
	}

	INT_PTR lCount = pClipList->GetCount();

	LogSendRecieveInfo(StrF(_T("Start of Send ClientThread Count - %d"), lCount));

	CClient client;
	if(client.OpenConnection(clientIp) == FALSE)
	{
		LogSendRecieveInfo(StrF(_T("ERROR opening connection to %s"), clientIp));

		bool showError = false;
		{
			ATL::CCritSecLock csLock(g_sendClientStateCs.m_sect);
			if(CGetSetOptions::m_SendClients[nClient].bShownFirstError == FALSE)
			{
				CGetSetOptions::m_SendClients[nClient].bShownFirstError = TRUE;
				showError = true;
			}
		}

		if(showError)
		{
			CString cs;
			cs.Format(_T("Error opening connection to %s"), clientIp);
			::SendMessage(theApp.m_MainhWnd, WM_SEND_RECIEVE_ERROR, (WPARAM)cs.GetBuffer(cs.GetLength()), 0);
			cs.ReleaseBuffer();
		}

		return FALSE;
	}

	//We were connected successfully show an error next time we can't connect.
	{
		ATL::CCritSecLock csLock(g_sendClientStateCs.m_sect);
		CGetSetOptions::m_SendClients[nClient].bShownFirstError = FALSE;
	}

	CClip* pClip;
	POSITION pos;
	pos = pClipList->GetHeadPosition();
	while(pos)
	{
		pClip = pClipList->GetNext(pos);
		if(pClip == NULL)
		{
			ASSERT(FALSE);
			LogSendRecieveInfo("Error in GetNext");
			break;
		}

		LogSendRecieveInfo(StrF(_T("Sending clip to %s"), clientIp));

		if(client.SendItem(pClip, false) == FALSE)
		{
			CString cs;
			cs.Format(_T("Error sending clip to %s"), clientIp);
			::SendMessage(theApp.m_MainhWnd, WM_SEND_RECIEVE_ERROR, (WPARAM)cs.GetBuffer(cs.GetLength()), 0);
			cs.ReleaseBuffer();
			break;
		}
	}

	client.CloseConnection();

	LogSendRecieveInfo(StrF(_T("@@@@@@@@@@@@@@@ - END OF SendClientThread to %s - @@@@@@@@@@@@@@@"), clientIp));

	return TRUE;
}
