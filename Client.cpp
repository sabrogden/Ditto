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

BOOL SendToFriend(CSendToFriendInfo &Info)
{
	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - START OF Send To Friend - @@@@@@@@@@@@@@@");

	if(Info.m_lPos > -1 && Info.m_lPos < MAX_SEND_CLIENTS)
	{
		Info.m_csIP = g_Opt.m_SendClients[Info.m_lPos].csIP;
	}
	else
	{
		Info.m_csErrorText = StrF("ERROR getting ip position - %d", Info.m_lPos);
		LogSendRecieveInfo(Info.m_csErrorText);
		return FALSE;
	}

	LogSendRecieveInfo(StrF("Sending clip to %s", Info.m_csIP));
	CClient client;

	if(client.OpenConnection(Info.m_csIP) == FALSE)
	{
		Info.m_csErrorText = StrF("ERROR opening connection to %s", Info.m_csIP);
		LogSendRecieveInfo(Info.m_csErrorText);
		return FALSE;
	}

	long lCount = Info.m_pClipList->GetCount();
	int i = -1;

	CClip* pClip;
	POSITION pos;
	pos = Info.m_pClipList->GetHeadPosition();
	while(pos)
	{
		pClip = Info.m_pClipList->GetNext(pos);
		if(pClip == NULL)
		{
			ASSERT(FALSE);
			continue;
		}
		i++;

		if(Info.m_pPopup)
		{
			Info.m_pPopup->SendToolTipText(StrF("Sending %d of %d", i+1, lCount));
		}

		MSG	msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		LogSendRecieveInfo(StrF("Sending %d of %d clip to %s", i+1, lCount, Info.m_csIP));

		if(client.SendItem(pClip) == FALSE)
		{
			Info.m_csErrorText = "ERROR SendItem Failed";
			LogSendRecieveInfo(Info.m_csErrorText);

			return FALSE;
		}
	}

	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - END OF Send To Friend - @@@@@@@@@@@@@@@");

	return TRUE;
}

UINT  SendClientThread(LPVOID pParam)
{	
	EnterCriticalSection(&theApp.m_CriticalSection);

	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - START OF SendClientThread - @@@@@@@@@@@@@@@");

	CClipList *pClipList = (CClipList*)pParam;
	if(pClipList == NULL)
	{
		LogSendRecieveInfo("ERROR if(pClipList == NULL)");
		return FALSE;
	}

	long lCount = pClipList->GetCount();

	LogSendRecieveInfo(StrF("Start of Send ClientThread Count - %d", lCount));
	
	for(int nClient = 0; nClient < MAX_SEND_CLIENTS; nClient++)
	{
		if(g_Opt.m_SendClients[nClient].bSendAll && 
			g_Opt.m_SendClients[nClient].csIP.GetLength() > 0)
		{
			CClient client;
			if(client.OpenConnection(g_Opt.m_SendClients[nClient].csIP) == FALSE)
			{
				LogSendRecieveInfo(StrF("ERROR opening connection to %s", g_Opt.m_SendClients[nClient].csIP));

				CString cs;
				cs.Format("Error opening connection to %s",g_Opt.m_SendClients[nClient].csIP);
				::SendMessage(theApp.m_MainhWnd, WM_SEND_RECIEVE_ERROR, (WPARAM)cs.GetBuffer(cs.GetLength()), 0);
				cs.ReleaseBuffer();

				continue;
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

				LogSendRecieveInfo(StrF("Sending clip to %s", g_Opt.m_SendClients[nClient].csIP));
				
				if(client.SendItem(pClip) == FALSE)
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

	delete pClipList;
	pClipList = NULL;
	
	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - END OF SendClientThread - @@@@@@@@@@@@@@@");

	LeaveCriticalSection(&theApp.m_CriticalSection);

	return TRUE;
}

CClient::CClient()
{
	m_Connection = NULL;

	m_pEncryptor = new CEncryption; //CreateEncryptionInterface("encryptdecrypt.dll");
}

CClient::~CClient()
{			
	CloseConnection();

	delete m_pEncryptor;
	m_pEncryptor = NULL;

//	ReleaseEncryptionInterface(m_pEncryptor);
}

BOOL CClient::CloseConnection()
{
	if(m_Connection != NULL && m_Connection != 0)
	{
		CSendInfo Info;
		SendCSendData(Info, MyEnums::EXIT);

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
		
		m_Connection = NULL;
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
		m_Connection = NULL;
		return FALSE;
	}

	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons((u_short) g_Opt.m_lPort);
	if(connect(m_Connection, (struct sockaddr*)&server, sizeof(server)))
	{
		int nWhy = WSAGetLastError();
		LogSendRecieveInfo(StrF("ERROR if(connect(m_Connection,(struct sockaddr*)&server,sizeof(server))) why = %d", nWhy));
		closesocket(m_Connection);
		m_Connection = NULL;
		return FALSE;	
	}

	return TRUE;
}

BOOL CClient::SendItem(CClip *pClip)
{
	CSendInfo Info;
	
	strncpy(Info.m_cComputerName, GetComputerName(), sizeof(Info.m_cComputerName));
	strncpy(Info.m_cIP, GetIPAddress(), sizeof(Info.m_cIP));
	strncpy(Info.m_cDesc, pClip->m_Desc, sizeof(Info.m_cDesc));
	
	Info.m_cDesc[sizeof(Info.m_cDesc)-1] = 0;
	Info.m_cComputerName[sizeof(Info.m_cComputerName)-1] = 0;
	Info.m_cIP[sizeof(Info.m_cIP)-1] = 0;

	if(SendCSendData(Info, MyEnums::START) == FALSE)
		return FALSE;
	
	CClipFormat* pCF;
	
	int nCount = pClip->m_Formats.GetSize();

	//For each data type
	for( int i=0; i < nCount; i++ )
	{
		pCF = &pClip->m_Formats.GetData()[i];
		
		LPVOID pvData = GlobalLock(pCF->m_hgData);
		long lLength = GlobalSize(pCF->m_hgData);

		UCHAR* pOutput = NULL;
		int nLenOutput = 0;

		LogSendRecieveInfo(StrF("BEFORE Encrypt clip data %d", lLength));

		if(m_pEncryptor)
		{
			if(m_pEncryptor->Encrypt((UCHAR*)pvData, lLength, g_Opt.m_csPassword, pOutput, nLenOutput))
			{
				LogSendRecieveInfo(StrF("AFTER Encrypt clip data %d", nLenOutput));

				Info.m_lParameter1 = nLenOutput;
				strncpy(Info.m_cDesc, GetFormatName(pCF->m_cfType), sizeof(Info.m_cDesc));
				Info.m_cDesc[sizeof(Info.m_cDesc)-1] = 0;

				if(SendCSendData(Info, MyEnums::DATA_START) == FALSE)
					return FALSE;

				SendExactSize((char*)pOutput, nLenOutput, false);

				m_pEncryptor->FreeBuffer(pOutput);
			}
			else
			{
				LogSendRecieveInfo("Failed to encrypt data");
				return FALSE;
			}
		}
		else
		{
			ASSERT(!"SendItem::Encryption not initialized");
			LogSendRecieveInfo("SendItem::Encryption not initialized");	
		}

		GlobalUnlock(pCF->m_hgData);
		
		if(SendCSendData(Info, MyEnums::DATA_END) == FALSE)
			return FALSE;
	}
	
	if(SendCSendData(Info, MyEnums::END) == FALSE)
		return FALSE;

	theApp.m_lClipsSent++;
	
	return TRUE;
}

BOOL CClient::SendCSendData(CSendInfo &data, MyEnums::eSendType type)
{
	data.m_Type = type;
	return SendExactSize((char *)&data, sizeof(CSendInfo));
}

BOOL CClient::SendExactSize(char *pData, long lLength, bool bEncrypt)
{
	BOOL bRet = FALSE;
	if(!m_pEncryptor && bEncrypt)
	{
		ASSERT(!"Encryption not initialized");
		LogSendRecieveInfo("SendExactSize::Encryption not initialized");
		return bRet;
	}

	LogSendRecieveInfo(StrF("START SendExactSize Total %d", lLength));

	UCHAR* pOutput = (UCHAR*)pData;
	int nLenOutput = lLength;
	long lBytesRead = 0;

	if(bEncrypt == false || m_pEncryptor->Encrypt((UCHAR*)pData, lLength, g_Opt.m_csPassword, pOutput, nLenOutput))
	{
		long lExpected = nLenOutput;

		while(lBytesRead < lExpected)
		{
			long lSize = send(m_Connection, (char*)pOutput + lBytesRead, lExpected - lBytesRead, 0);
		
			if(lSize == SOCKET_ERROR || lSize == 0)
			{
				LogSendRecieveInfo(StrF("lSize == SOCKET_ERROR, %d", WSAGetLastError()));
				break;
			}
			lBytesRead += lSize;

			LogSendRecieveInfo(StrF("SendExactSize Last Size %d - Total %d", lSize, lBytesRead));
		}

		bRet = TRUE;

		if(pOutput != (UCHAR*)pData)
			m_pEncryptor->FreeBuffer(pOutput);
	}
	else
	{
		LogSendRecieveInfo("SendExactSize::Failed to encrypt data");
	}

	LogSendRecieveInfo(StrF("END SendExactSize Total %d", lBytesRead));

	return bRet;
}
