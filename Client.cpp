// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "Client.h"
#include "TextConvert.h"
#include "RecieveSocket.h"
#include "FileRecieve.h"
#include "FileTransferProgressDlg.h"


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
		Info.m_csErrorText = StrF(_T("ERROR getting ip position - %d"), Info.m_lPos);
		LogSendRecieveInfo(Info.m_csErrorText);
		return FALSE;
	}

	LogSendRecieveInfo(StrF(_T("Sending clip to %s"), Info.m_csIP));
	CClient client;

	if(client.OpenConnection(Info.m_csIP) == FALSE)
	{
		Info.m_csErrorText = StrF(_T("ERROR opening connection to %s"), Info.m_csIP);
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
			Info.m_pPopup->SendToolTipText(StrF(_T("Sending %d of %d"), i+1, lCount));
		}

		MSG	msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		LogSendRecieveInfo(StrF(_T("Sending %d of %d clip to %s"), i+1, lCount, Info.m_csIP));

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

CCriticalSection SendClientCritSection;
UINT  SendClientThread(LPVOID pParam)
{	
	SendClientCritSection.Lock();

	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - START OF SendClientThread - @@@@@@@@@@@@@@@");

	CClipList *pClipList = (CClipList*)pParam;
	if(pClipList == NULL)
	{
		SendClientCritSection.Unlock();
		LogSendRecieveInfo("ERROR if(pClipList == NULL)");
		return FALSE;
	}

	long lCount = pClipList->GetCount();

	LogSendRecieveInfo(StrF(_T("Start of Send ClientThread Count - %d"), lCount));
	
	for(int nClient = 0; nClient < MAX_SEND_CLIENTS; nClient++)
	{
		if(g_Opt.m_SendClients[nClient].bSendAll && 
			g_Opt.m_SendClients[nClient].csIP.GetLength() > 0)
		{
			CClient client;
			if(client.OpenConnection(g_Opt.m_SendClients[nClient].csIP) == FALSE)
			{
				LogSendRecieveInfo(StrF(_T("ERROR opening connection to %s"), g_Opt.m_SendClients[nClient].csIP));

				if(g_Opt.m_SendClients[nClient].bShownFirstError == FALSE)
				{
					CString cs;
					cs.Format(_T("Error opening connection to %s"), g_Opt.m_SendClients[nClient].csIP);
					::SendMessage(theApp.m_MainhWnd, WM_SEND_RECIEVE_ERROR, (WPARAM)cs.GetBuffer(cs.GetLength()), 0);
					cs.ReleaseBuffer();

					g_Opt.m_SendClients[nClient].bShownFirstError = TRUE;
				}

				continue;
			}

			//We were connected successfully show an error next time we can't connect
			g_Opt.m_SendClients[nClient].bShownFirstError = FALSE;

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

				LogSendRecieveInfo(StrF(_T("Sending clip to %s"), g_Opt.m_SendClients[nClient].csIP));
				
				if(client.SendItem(pClip) == FALSE)
				{
					CString cs;
					cs.Format(_T("Error sending clip to %s"), g_Opt.m_SendClients[nClient].csIP);
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

	SendClientCritSection.Unlock();

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
		CSendInfo Info;
		m_SendSocket.SendCSendData(Info, MyEnums::EXIT);

		closesocket(m_Connection);
		WSACleanup();

		m_Connection = NULL;
	}

	return TRUE;
}

BOOL CClient::OpenConnection(const TCHAR* servername)
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

	CStringA csServerNameA = CTextConvert::ConvertToChar(servername);

	if(inet_addr(csServerNameA)==INADDR_NONE)
	{
		hp = gethostbyname(csServerNameA);
	}
	else
	{
		addr = inet_addr(csServerNameA);
		hp = gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	}

	if(hp == NULL)
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
		LogSendRecieveInfo(StrF(_T("ERROR if(connect(m_Connection,(struct sockaddr*)&server,sizeof(server))) why = %d"), nWhy));
		closesocket(m_Connection);
		m_Connection = NULL;
		return FALSE;	
	}

	return TRUE;
}

BOOL CClient::SendItem(CClip *pClip)
{
	CSendInfo Info;

	//Send all text over as UTF-8
	CStringA dest;
	if(CTextConvert::ConvertToUTF8(GetComputerName(), dest))
	{
		strncpy(Info.m_cComputerName, dest, sizeof(Info.m_cComputerName));
	}

	if(CTextConvert::ConvertToUTF8(GetIPAddress(), dest))
	{
		strncpy(Info.m_cIP, dest, sizeof(Info.m_cIP));	
	}

	if(CTextConvert::ConvertToUTF8(pClip->m_Desc, dest))
	{
		strncpy(Info.m_cDesc, dest, sizeof(Info.m_cDesc));
	}
	
	Info.m_cDesc[sizeof(Info.m_cDesc)-1] = 0;
	Info.m_cComputerName[sizeof(Info.m_cComputerName)-1] = 0;
	Info.m_cIP[sizeof(Info.m_cIP)-1] = 0;

	m_SendSocket.SetSocket(m_Connection);

	if(m_SendSocket.SendCSendData(Info, MyEnums::START) == FALSE)
		return FALSE;
	
	CClipFormat* pCF;
	
	int nCount = pClip->m_Formats.GetSize();

	//For each data type
	for( int i=0; i < nCount; i++ )
	{
		pCF = &pClip->m_Formats.GetData()[i];
		
		SendClipFormat(pCF);
	}
	
	if(m_SendSocket.SendCSendData(Info, MyEnums::END) == FALSE)
		return FALSE;

	theApp.m_lClipsSent++;
	
	return TRUE;
}

BOOL CClient::SendClipFormat(CClipFormat* pCF)
{
	CSendInfo Info;
	LPVOID pvData = GlobalLock(pCF->m_hgData);
	long lLength = GlobalSize(pCF->m_hgData);
	UCHAR* pOutput = NULL;
	int nLenOutput = 0;
	CTextConvert Convert;
	BOOL bRet = FALSE;

	LogSendRecieveInfo(StrF(_T("BEFORE Encrypt clip data %d"), lLength));

	if(m_SendSocket.m_pEncryptor)
	{
		if(m_SendSocket.m_pEncryptor->Encrypt((UCHAR*)pvData, lLength, g_Opt.m_csPassword, pOutput, nLenOutput))
		{
			LogSendRecieveInfo(StrF(_T("AFTER Encrypt clip data %d"), nLenOutput));

			Info.m_lParameter1 = nLenOutput;

			//Send over as UTF-8
			CStringA dest;
			if(CTextConvert::ConvertToUTF8(GetFormatName(pCF->m_cfType), dest))
			{
				strncpy(Info.m_cDesc, dest, sizeof(Info.m_cDesc));
				Info.m_cDesc[sizeof(Info.m_cDesc)-1] = 0;
			}

			if(m_SendSocket.SendCSendData(Info, MyEnums::DATA_START) == FALSE)
				return FALSE;

			m_SendSocket.SendExactSize((char*)pOutput, nLenOutput, false);

			m_SendSocket.m_pEncryptor->FreeBuffer(pOutput);

			bRet = TRUE;
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
	
	if(m_SendSocket.SendCSendData(Info, MyEnums::DATA_END) == FALSE)
		return FALSE;

	return bRet;
}

HGLOBAL CClient::RequestCopiedFiles(CClipFormat &HDropFormat, CString csIP, CString csComputerName)
{
	CSendInfo Info;
	bool bBreak = false;
	HGLOBAL hReturn = NULL;
	CString csErrorString;

	CFileTransferProgressDlg *pProgress = new CFileTransferProgressDlg;
	if(pProgress == NULL)
		return NULL;

	pProgress->Create(IDD_DIALOG_REMOTE_FILE);
	pProgress->ShowWindow(SW_SHOW);
	pProgress->SetMessage(StrF(_T("Opening Connection to %s (%s)"), csComputerName, csIP));
	pProgress->PumpMessages();

	do 
	{
		if(OpenConnection(csIP) == FALSE)
		{
			csErrorString.Format(_T("Error Opening Connection to %s (%s)"), csComputerName, csIP);
			break;
		}

		m_SendSocket.SetSocket(m_Connection);

		if(m_SendSocket.SendCSendData(Info, MyEnums::START) == FALSE)
			break;

		if(SendClipFormat(&HDropFormat) == FALSE)
		{
			csErrorString = _T("Error sending data request.");
			break;
		}

		if(m_SendSocket.SendCSendData(Info, MyEnums::END) == FALSE)
			break;
			
		if(m_SendSocket.SendCSendData(Info, MyEnums::REQUEST_FILES) == FALSE)
			break;

		CFileRecieve Recieve;
		long lRet = Recieve.RecieveFiles(m_Connection, csIP, pProgress);
		if(lRet == TRUE)
		{
			hReturn = Recieve.CreateCF_HDROPBuffer();
		}
		else if(lRet == FALSE)
		{
			csErrorString = _T("Error recieving files.");
		}

	} while(false);

	CloseConnection();

	if(hReturn == NULL && csErrorString.IsEmpty() == FALSE)
	{
		MessageBox(pProgress->m_hWnd, csErrorString, _T("Ditto"), MB_OK|MB_ICONEXCLAMATION);
	}

	pProgress->DestroyWindow();

	return hReturn;
}
