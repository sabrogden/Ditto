// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "Client.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL SendToFriend(CIDArray *pArray, long lPos, CString csIP, CPopup *pPopup)
{
	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - START OF Send To Friend - @@@@@@@@@@@@@@@");

	CSendToFriendInfo Info;
	Info.csIP = csIP;
	Info.lPos = lPos;
	Info.pArray = pArray;
	Info.pPopup = pPopup;

	AfxBeginThread(SendToFriendThread, &Info);

	DWORD dwWaitResult;
	MSG msg;
	while(true)
	{
		dwWaitResult = MsgWaitForMultipleObjects(1, &Info.hThreadRunning, FALSE, 1000, QS_ALLINPUT);

		// Window message to be processed
		if(dwWaitResult == (WAIT_OBJECT_0+1))
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				// get the next message in the queue
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}	
		else if (dwWaitResult == WAIT_OBJECT_0)
		{
			break;
		}
	}

	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - END OF Send To Friend - @@@@@@@@@@@@@@@");

	return Info.bRet;
}

UINT SendToFriendThread(LPVOID pParam)
{
	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - START OF Send To Friend Thread - @@@@@@@@@@@@@@@");

	CSendToFriendInfo *pInfo = (CSendToFriendInfo*)pParam;
	
	if(pInfo->lPos > -1 && pInfo->lPos < MAX_SEND_CLIENTS)
	{
		pInfo->csIP = g_Opt.m_SendClients[pInfo->lPos].csIP;
	}

	LogSendRecieveInfo(StrF("Sending clip to %s", pInfo->csIP));
	CClient client;

	if(client.OpenConnection(pInfo->csIP) == FALSE)
	{
		LogSendRecieveInfo(StrF("ERROR opening connection to %s", pInfo->csIP));
		pInfo->bRet = FALSE;
		SetEvent(pInfo->hThreadRunning);
		return FALSE;
	}

	for(int i = 0; i < pInfo->pArray->lCount; i++)
	{
		if(pInfo->pPopup)
		{
			pInfo->pPopup->SendToolTipText(StrF("Sending %d of %d", i+1, pInfo->pArray->lCount));
		}

		LogSendRecieveInfo(StrF("Sending %d of %d clip to %s", i+1, pInfo->pArray->lCount, pInfo->csIP));

		if(client.SendItem(&pInfo->pArray->pIDs[i]) == FALSE)
		{
			LogSendRecieveInfo("ERROR SendItem Failed");

			pInfo->bRet = FALSE;
			SetEvent(pInfo->hThreadRunning);
			return FALSE;
		}
	}

	pInfo->bRet = TRUE;

	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - END OF Send To Friend Thread - @@@@@@@@@@@@@@@");

	SetEvent(pInfo->hThreadRunning);

	return TRUE;
}

UINT  SendClientThread(LPVOID pParam)
{	
	EnterCriticalSection(&theApp.m_CriticalSection);

	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - START OF SendClientThread - @@@@@@@@@@@@@@@");

	bool bError;
	CIDArray *pArray = (CIDArray*)pParam;
	if(pArray == NULL)
	{
		LogSendRecieveInfo("ERROR if(pArray == NULL)");
		return FALSE;
	}

	LogSendRecieveInfo(StrF("Start of Send ClientThread Count - %d", pArray->lCount));
	
	for(int nClient = 0; nClient < MAX_SEND_CLIENTS; nClient++)
	{
		if(g_Opt.m_SendClients[nClient].bSendAll && 
			g_Opt.m_SendClients[nClient].csIP.GetLength() > 0)
		{
			CClient client;
			if(client.OpenConnection(g_Opt.m_SendClients[nClient].csIP) == FALSE)
			{
				LogSendRecieveInfo(StrF("ERROR opening connection to %s", g_Opt.m_SendClients[nClient].csIP));
				bError = true;
			}

			bError = false;

			for(int i = 0; i < pArray->lCount; i++)
			{
				if(bError == false)
				{
					LogSendRecieveInfo(StrF("Sending clip to %s", g_Opt.m_SendClients[nClient].csIP));
				
					if(client.SendItem(&pArray->pIDs[i]) == FALSE)
						bError = true;
				}

				if(bError)
				{
					CString cs;
					cs.Format("Error sending clip to %s",g_Opt.m_SendClients[nClient].csIP);
					::SendMessage(theApp.m_MainhWnd, WM_SEND_RECIEVE_ERROR, (WPARAM)cs.GetBuffer(cs.GetLength()), 0);
					cs.ReleaseBuffer();
					break;
				}
			}

			client.CloseConnection();
		}
	}

	delete []pArray->pIDs;
	pArray->pIDs = NULL;

	delete pArray;
	pArray = NULL;
	
	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - END OF SendClientThread - @@@@@@@@@@@@@@@");

	LeaveCriticalSection(&theApp.m_CriticalSection);

	return TRUE;
}

CClient::CClient()
{
	m_Connection = NULL;
}

CClient::~CClient()
{			
	CloseConnection();
}

BOOL CClient::CloseConnection()
{
	if(m_Connection != NULL && m_Connection != 0)
	{
		SendInfo Info;
		SendData(&Info, MyEnums::EXIT);

		closesocket(m_Connection);
		WSACleanup();

		m_Connection = NULL;
	}

	return TRUE;
}

BOOL CClient::OpenConnection(const char* servername)
{
	WSADATA wsaData;
	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;
	int wsaret=WSAStartup(0x101,&wsaData);
	if(wsaret)	
	{
		LogSendRecieveInfo("ERROR - WSAStartup(0x101,&wsaData)");
		return FALSE;
	}
	
	m_Connection = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(m_Connection == INVALID_SOCKET)
	{
		LogSendRecieveInfo("ERROR - socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)");
		
		return FALSE;
	}

	if(inet_addr(servername)==INADDR_NONE)
	{
		hp=gethostbyname(servername);
	}
	else
	{
		addr=inet_addr(servername);
		hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	}

	if(hp==NULL)
	{
		LogSendRecieveInfo("ERROR - if(hp==NULL)");

		closesocket(m_Connection);
		return FALSE;
	}

	server.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
	server.sin_family=AF_INET;
	server.sin_port=htons(20248);
	if(connect(m_Connection,(struct sockaddr*)&server,sizeof(server)))
	{
		LogSendRecieveInfo("ERROR if(connect(m_Connection,(struct sockaddr*)&server,sizeof(server)))");
		closesocket(m_Connection);
		return FALSE;	
	}

	return TRUE;
}

BOOL CClient::SendItem(CID *pData)
{
	SendInfo Info;
	
	strncpy(Info.m_cComputerName, GetComputerName(), sizeof(Info.m_cComputerName));
	strncpy(Info.m_cIP, GetIPAddress(), sizeof(Info.m_cIP));
	strncpy(Info.m_cText, pData->m_csDesc, sizeof(Info.m_cText));
	
	Info.m_cText[sizeof(Info.m_cText)-1] = 0;
	Info.m_cComputerName[sizeof(Info.m_cComputerName)-1] = 0;
	Info.m_cIP[sizeof(Info.m_cIP)-1] = 0;

	if(SendData(&Info, MyEnums::START) == FALSE)
		return FALSE;
	
	CClipFormat* pCF;

	//If the data has not been loaded, then send a message to
	//the main thread to load the data from the db
	if(pData->m_Formats.GetSize() <= 0)
	{
		::SendMessage(theApp.m_MainhWnd, WM_LOAD_FORMATS, pData->lID, (LPARAM)&pData->m_Formats);
	}
	
	int nCount = pData->m_Formats.GetSize();
	DWORD dwSize;
	int nDataSent;

	//For each data type
	for( int i=0; i < nCount; i++ )
	{
		pCF = &pData->m_Formats[i];

		Info.m_lParameter1 = dwSize = GlobalSize(pCF->m_hgData);
		strncpy(Info.m_cText, GetFormatName(pCF->m_cfType), sizeof(Info.m_cText));
		Info.m_cText[sizeof(Info.m_cText)-1] = 0;

		if(SendData(&Info, MyEnums::DATA_START) == FALSE)
			return FALSE;

		nDataSent = 0;
		while(nDataSent < dwSize)
		{
			LPVOID pvData = GlobalLock(pCF->m_hgData);

			if(dwSize - nDataSent < MAX_DATA_SIZE)
				Info.m_lParameter1 = dwSize - nDataSent;
			else
				Info.m_lParameter1 = MAX_DATA_SIZE;

			memcpy(Info.m_cText,  ((char*)pvData) + nDataSent, Info.m_lParameter1);

			GlobalUnlock(pCF->m_hgData);

			if(SendData(&Info, MyEnums::DATA) == FALSE)
				return FALSE;

			nDataSent += Info.m_lParameter1;
		}

		if(SendData(&Info, MyEnums::DATA_END) == FALSE)
			return FALSE;
	}
	
	if(SendData(&Info, MyEnums::END) == FALSE)
		return FALSE;
	
	return TRUE;
}

BOOL CClient::SendData(SendInfo *pInfo, MyEnums::eSendType type)
{
	pInfo->m_Type = type;
	long lSize = send(m_Connection, (char *)pInfo, sizeof(SendInfo), 0);
	if(lSize != sizeof(SendInfo))
	{
		ASSERT(FALSE);
	}

	if(lSize == SOCKET_ERROR)
	{
		LogSendRecieveInfo(StrF("lSize == SOCKET_ERROR, %d", WSAGetLastError()));
		return FALSE;
	}

	return TRUE;
}
