// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "Client.h"
#include "shared/TextConvert.h"
#include "RecieveSocket.h"
#include "FileRecieve.h"
#include "FileTransferProgressDlg.h"
#include "Shared/Tokenizer.h"


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

	if(Info.m_csIP == _T(""))	
	{
		Info.m_csErrorText = StrF(_T("ERROR getting ip/host name position - %s"), Info.m_csIP);
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

	INT_PTR count = Info.m_pClipList->GetCount();
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
			Info.m_pPopup->SendToolTipText(StrF(_T("Sending %d of %d"), i+1, count));
		}

		MSG	msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		LogSendRecieveInfo(StrF(_T("Sending %d of %d clip to %s"), i+1, count, Info.m_csIP));

		if(client.SendItem(pClip, Info.m_manualSend) == FALSE)
		{
			Info.m_csErrorText = "ERROR SendItem Failed";
			LogSendRecieveInfo(Info.m_csErrorText);

			return FALSE;
		}
	}

	LogSendRecieveInfo("@@@@@@@@@@@@@@@ - END OF Send To Friend - @@@@@@@@@@@@@@@");

	return TRUE;
}

CClient::CClient()
{
	m_Connection = NULL;
	m_connectionPort = 0;
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
	unsigned int addr = INADDR_NONE;
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

	CString parsedServerName = servername;
	m_connectionPort = CGetSetOptions::m_lPort;

	CTokenizer tokenizer(servername, ":");
	CString token;
	int pos = 0;
	while (tokenizer.Next(token))
	{
		if (pos == 0)
		{
			parsedServerName = token;
		}
		else if (pos == 1)
		{
			m_connectionPort = ATOI(token);
		}
		pos++;
	}

	CStringA csServerNameA = CTextConvert::UnicodeToAnsi(parsedServerName);

	//11-5-06 Serge Baranov found that if we are passing in an ip then
	//don't look the name up using gethostbyname/gethostbyaddr->
	//on simple networks that don't use DNS these will fail.
	//So now only lookup the host name if they don't provide an IP.	
	addr = inet_addr(csServerNameA);
	if(addr == INADDR_NONE)
	{
		struct hostent *hp = gethostbyname(csServerNameA);
		if(hp != NULL)
		{
			addr = *(unsigned int*)hp->h_addr;
		}
	}

	if(addr == NULL || addr == INADDR_NONE)
	{
		LogSendRecieveInfo("addr == NULL || addr == INADDR_NONE");

		closesocket(m_Connection);
		m_Connection = NULL;
		return FALSE;
	}

	server.sin_addr.s_addr = addr;
	server.sin_family = AF_INET;
	server.sin_port = htons((u_short)m_connectionPort);
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

BOOL CClient::SendItem(CClip *pClip, bool manualSend)
{
	CSendInfo Info;

	Info.m_manualSend = manualSend;

	////only send a response port if it's different than the default
	if (CGetSetOptions::m_lPort != 23443 || m_connectionPort != 23443)
	{
		Info.m_respondPort = CGetSetOptions::m_lPort;
	}

	//Send all text over as UTF-8
	CStringA dest = CTextConvert::UnicodeToUTF8(GetComputerName());
	strncpy(Info.m_cComputerName, dest, sizeof(Info.m_cComputerName));
	
	dest = CTextConvert::UnicodeToUTF8(GetIPAddress());
	strncpy(Info.m_cIP, dest, sizeof(Info.m_cIP));	
	
	dest = CTextConvert::UnicodeToUTF8(pClip->m_Desc);
	strncpy(Info.m_cDesc, dest, sizeof(Info.m_cDesc));
	
	
	Info.m_cDesc[sizeof(Info.m_cDesc)-1] = 0;
	Info.m_cComputerName[sizeof(Info.m_cComputerName)-1] = 0;
	Info.m_cIP[sizeof(Info.m_cIP)-1] = 0;

	m_SendSocket.SetSocket(m_Connection);

	if(m_SendSocket.SendCSendData(Info, MyEnums::START) == FALSE)
		return FALSE;
	
	CClipFormat* pCF;
	
	INT_PTR count = pClip->m_Formats.GetSize();

	//For each data type
	for(int i=0; i < count; i++)
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
	INT_PTR length = GlobalSize(pCF->m_hgData);
	UCHAR* pOutput = NULL;
	int nLenOutput = 0;
	CTextConvert Convert;
	BOOL bRet = FALSE;

	LogSendRecieveInfo(StrF(_T("BEFORE Encrypt clip data %d"), length));

	if(m_SendSocket.m_pEncryptor)
	{
		if(m_SendSocket.m_pEncryptor->Encrypt((UCHAR*)pvData, (int)length, CGetSetOptions::m_csPassword, pOutput, nLenOutput))
		{
			LogSendRecieveInfo(StrF(_T("AFTER Encrypt clip data %d"), nLenOutput));

			Info.m_lParameter1 = nLenOutput;

			//Send over as UTF-8
			CStringA dest = CTextConvert::UnicodeToUTF8(GetFormatName(pCF->m_cfType));
			strncpy(Info.m_cDesc, dest, sizeof(Info.m_cDesc));
			Info.m_cDesc[sizeof(Info.m_cDesc)-1] = 0;
			
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

	LogSendRecieveInfo(StrF(_T("************** START of requesting files from cpu %s, ip: %s **************"), csComputerName, csIP));

	pProgress->Create(IDD_DIALOG_REMOTE_FILE);
	pProgress->ShowWindow(SW_SHOW);
	pProgress->SetMessage(StrF(_T("Opening Connection to %s (%s)"), csComputerName, csIP));
	pProgress->PumpMessages();

	CString requestFrom;
	if(CGetSetOptions::GetRequestFilesUsingIP())
	{
		requestFrom = csIP;
	}
	else
	{
		requestFrom = csComputerName;
	}

	do 
	{
		if(OpenConnection(requestFrom) == FALSE)
		{
			csErrorString.Format(_T("Error Opening Connection to %s (%s)"), csComputerName, csIP);
			break;
		}

		m_SendSocket.SetSocket(m_Connection);
		m_SendSocket.SetProgressBar(pProgress);

		if(m_SendSocket.SendCSendData(Info, MyEnums::START) == FALSE)
			break;

		if(SendClipFormat(&HDropFormat) == FALSE)
		{
			csErrorString = _T("Error sending data request.");
			break;
		}

		if(m_SendSocket.SendCSendData(Info, MyEnums::END) == FALSE)
			break;
			
		pProgress->SetMessage(StrF(_T("Requesting Files from %s (%s)"), csComputerName, csIP));

		if(m_SendSocket.SendCSendData(Info, MyEnums::REQUEST_FILES) == FALSE)
			break;

		CFileRecieve Recieve;
		long lRet = Recieve.RecieveFiles(m_Connection, csIP, pProgress);
		if(lRet == TRUE)
		{
			hReturn = Recieve.CreateCF_HDROPBuffer();
		}
		else if(lRet == FALSE || lRet == MD5_MISMATCH)
		{
			if(pProgress != NULL && pProgress->Cancelled())
			{
				//Don't show an error message the user canceled things
			}
			else	
			{
				csErrorString = _T("Error receiving files.");
				if (lRet == MD5_MISMATCH)
				{
					csErrorString += _T(" MD5 Match Error.");
				}
			}
		}

	} while(false);

	CloseConnection();

	if(hReturn == NULL && csErrorString.IsEmpty() == FALSE)
	{
		::SendMessage(theApp.m_MainhWnd, WM_SEND_RECIEVE_ERROR, (WPARAM)csErrorString.GetBuffer(csErrorString.GetLength()), 0);
	}

	pProgress->DestroyWindow();

	LogSendRecieveInfo(StrF(_T("************** END of requesting files from cpu %s, ip: %s **************************"), csComputerName, csIP));

	return hReturn;
}
