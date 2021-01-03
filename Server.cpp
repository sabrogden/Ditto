// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "Server.h"
#include "Shared\Tokenizer.h"
#include "WildCardMatch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UINT  MTServerThread(LPVOID pParam)
{		
	static bool bRunning = false;
	if(bRunning)
		return 0;
	bRunning = true;

	LogSendRecieveInfo("Start of ServerThread");

	theApp.m_bExitServerThread = false;

	WSADATA wsaData;
	sockaddr_in local;
	int wsaret = WSAStartup(0x101,&wsaData);
	if(wsaret!=0)
	{
		LogSendRecieveInfo("ERROR - int wsaret = WSAStartup(0x101,&wsaData);");
		return 0;
	}
	local.sin_family = AF_INET;
	CString bindToIpAddress = CGetSetOptions::GetNetworkBindIPAddress();
	if (bindToIpAddress == _T("*"))
	{
		local.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		local.sin_addr.s_addr = inet_addr(CTextConvert::ConvertToChar(bindToIpAddress));
	}
	local.sin_port = htons((u_short)g_Opt.m_lPort);
	theApp.m_sSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(theApp.m_sSocket == INVALID_SOCKET)
	{
		LogSendRecieveInfo("ERROR - theApp.m_sSocket = socket(AF_INET, SOCK_STREAM, 0);");
		return 0;
	}
	if(bind(theApp.m_sSocket,(sockaddr*)&local,sizeof(local))!=0)
	{
		LogSendRecieveInfo("ERROR - if(bind(theApp.m_sSocket,(sockaddr*)&local,sizeof(local))!=0)");
		return 0;
	}
	if(listen(theApp.m_sSocket,10)!=0)
	{
		LogSendRecieveInfo("ERROR - if(listen(theApp.m_sSocket,10)!=0)");
		return 0;
	}
		
	sockaddr_in from;
	int fromlen = sizeof(from);

	while(true)
	{
		if(theApp.m_bAppExiting || theApp.m_bExitServerThread)
			break;

		SOCKET socket = accept(theApp.m_sSocket, (struct sockaddr*)&from, &fromlen);

		SocketParams *pParams = new SocketParams();
		pParams->m_ip = inet_ntoa(from.sin_addr);
		pParams->m_socket = socket;

		if (socket != INVALID_SOCKET)
		{
			AfxBeginThread(ClientThread, (LPVOID)pParams);
		}
		else
		{
			delete pParams;
		}
	}	

	LogSendRecieveInfo("End of Server Thread");

	bRunning = false;
	theApp.m_sSocket = INVALID_SOCKET;

	return 0;
}

UINT  ClientThread(LPVOID pParam)
{	
	LogSendRecieveInfo("*********************Start of ClientThread*********************");
	
	SocketParams *pParams = (SocketParams*)pParam;

	CServer Server;
	Server.RunThread(pParams);

	delete pParams;

	LogSendRecieveInfo("*********************End of ClientThread*********************");
	
	return 0;
}

CServer::CServer()
{
	m_pClipList = NULL;
	m_pClip = NULL;
	m_bSetToClipBoard = FALSE;
	m_manualSend = false;
	m_respondPort = 0;
}

CServer::~CServer()
{
	closesocket(m_Sock.GetSocket());
}

void CServer::RunThread(SocketParams *pParams)
{
	m_Sock.SetSocket(pParams->m_socket);	
	m_recieveIP = pParams->m_ip;
	CSendInfo info;
	bool bBreak = false;
		
	while(true)
	{
		if(m_Sock.RecieveCSendInfo(&info) == FALSE)
			break;
		
		switch(info.m_Type)
		{
		case MyEnums::START:
			OnStart(info);
			break;

		case MyEnums::DATA_START:
			OnDataStart(info);
			break;

		case MyEnums::DATA_END:
			OnDataEnd(info);
			break;

		case MyEnums::END:
			OnEnd(info);
			break;

		case MyEnums::EXIT:
			OnExit(info);
			bBreak = true;
			break;

		case MyEnums::REQUEST_FILES:
			OnRequestFiles(info);
			break;

		default:
			LogSendRecieveInfo("::ERROR unknown action type exiting");
			bBreak = true;
		}

		if(bBreak || theApp.m_bAppExiting)
			break;
	}		

	if(m_pClipList)
	{
		LogSendRecieveInfo("::ERROR pClipList was not NULL something is wrong");

		delete m_pClipList;
		m_pClipList = NULL;		
	}

	if(m_pClip)
	{
		LogSendRecieveInfo("::ERROR pClip was not NULL something is wrong");

		delete m_pClip;
		m_pClip = NULL;
	}
}

void CServer::OnStart(CSendInfo &info)
{
	if (m_recieveIP != _T("") &&
		g_Opt.GetUseIPFromAccept())
	{
		LogSendRecieveInfo(StrF(_T("Using ip address from the Accept Call - %s"), m_recieveIP));
		m_csIP = m_recieveIP;
	}
	else
	{
		CTextConvert::ConvertFromUTF8(info.m_cIP, m_csIP);
	}
	CTextConvert::ConvertFromUTF8(info.m_cComputerName, m_csComputerName);
	CTextConvert::ConvertFromUTF8(info.m_cDesc, m_csDesc);

	m_manualSend = info.m_manualSend;
	m_respondPort = info.m_respondPort;

	if(m_pClip != NULL)
	{
		delete m_pClip;
		m_pClip = NULL;
	}
	
	m_pClip = new CClip;
	
	CString cs;
	cs.Format(_T("%s\n(%s)(%s)"), m_csDesc, m_csComputerName, m_csIP);
	
	if(m_pClip)
	{
		m_pClip->m_Desc = cs;
	}
	
	m_bSetToClipBoard = FALSE;

	CTokenizer token(g_Opt.m_csIPListToPutOnClipboard, ",");
	CString line;

	while(token.Next(line))
	{
		if(line != "")
		{
			if(CWildCardMatch::WildMatch(line, m_csIP, ""))
			{
				LogSendRecieveInfo(StrF(_T("Found ip match, placing on clipboard Found Match %s - %s"), line, m_csIP));

				m_bSetToClipBoard = TRUE;
				break;
			}

			if(CWildCardMatch::WildMatch(line, m_csComputerName, ""))
			{
				LogSendRecieveInfo(StrF(_T("Found machine match, placing on clipboard Found Match %s - %s"), line, m_csIP));

				m_bSetToClipBoard = TRUE;
				break;
			}
		}
	}
	
	info.m_cDesc[20] = 0;
	LogSendRecieveInfo(StrF(_T("::START %s %s %s"), m_csDesc, m_csComputerName, m_csIP));
}

void CServer::OnDataStart(CSendInfo &info)
{
	LogSendRecieveInfo("::DATA_START -- START");

	CString csFormat;
	CTextConvert::ConvertFromUTF8(info.m_cDesc, csFormat);
	m_cf.m_cfType = GetFormatID(csFormat);
	m_cf.m_hgData = 0;
	
	long lInSize = info.m_lParameter1;
	long lOutSize = 0;

	LPVOID lpData = m_Sock.ReceiveEncryptedData(lInSize, lOutSize);

	if(lpData && lOutSize > 0)
	{					
		m_cf.m_hgData = NewGlobal(lOutSize);

		if(m_cf.m_hgData)
		{
			if(m_pClip)
			{
				m_pClip->m_lTotalCopySize += lOutSize;
			}

			LPVOID pvData = GlobalLock(m_cf.m_hgData);
			if(pvData)
			{
				memcpy(pvData, lpData, lOutSize);

				GlobalUnlock(m_cf.m_hgData);
			}
			else
			{
				LogSendRecieveInfo("::DATA_START -- failed to lock hGlobal");
			}
		}
		else
		{
			LogSendRecieveInfo("::DATA_START -- failed to create new hGlobal");
		}

		m_Sock.FreeDecryptedData();
	}

	LogSendRecieveInfo("::DATA_START -- END");
}

void CServer::OnDataEnd(CSendInfo &info)
{
	LogSendRecieveInfo("::DATA_END");
				
	if(m_pClip && m_cf.m_hgData)
	{
		if(m_cf.m_cfType == CF_HDROP)
			AddRemoteCF_HDROPFormat();

		m_pClip->m_Formats.Add(m_cf);
		m_cf.m_hgData = 0; // now owned by pClip
	}
	else
	{
		LogSendRecieveInfo("MyEnums::DATA_END Error if(pClip && cf.m_hgData)");
	}
}

void CServer::OnEnd(CSendInfo &info)
{				
	LogSendRecieveInfo("::END");

	if(m_pClipList == NULL)
		m_pClipList = new CClipList;

	if(m_pClipList)
	{
		m_pClipList->AddTail(m_pClip);
		m_pClip = NULL;  //clip list now owns the clip
	}
	else
		LogSendRecieveInfo("::ERROR pClipList was NULL");
}

void CServer::OnExit(CSendInfo &info)
{
	LogSendRecieveInfo("::EXIT");

	if(m_pClipList && m_pClipList->GetCount() > 0)
	{
		theApp.m_lClipsRecieved += (long)m_pClipList->GetCount();

		DWORD flags = 0;
		if (m_bSetToClipBoard)
		{
			flags |= REMOTE_CLIP_ADD_TO_CLIPBOARD;
		}
		if (m_manualSend)
		{
			flags |= REMOTE_CLIP_MANUAL_SEND;
		}

		//Post a message pClipList will be freed by the reciever
		::PostMessage(theApp.m_MainhWnd, WM_ADD_TO_DATABASE_FROM_SOCKET, (WPARAM)m_pClipList, flags);
		m_pClipList = NULL;
	}
	else
		LogSendRecieveInfo("::ERROR pClipList was NULL or Count was 0");
}

void CServer::OnRequestFiles(CSendInfo &info)
{
	CFileSend Send;
	Send.SendClientFiles(m_Sock.GetSocket(), m_pClipList);

	delete m_pClipList;
	m_pClipList = NULL;
}

void CServer::AddRemoteCF_HDROPFormat()
{
	CClipFormat cf;

	CDittoCF_HDROP Drop;

	Drop.respondPort = m_respondPort;

	CTextConvert Convert;

	CStringA dest;
	if(CTextConvert::ConvertToUTF8(m_csIP, dest))
	{
		strncpy(Drop.m_cIP, dest, sizeof(Drop.m_cIP)-1);
	}
	if(CTextConvert::ConvertToUTF8(m_csComputerName, dest))
	{
		strncpy(Drop.m_cComputerName, dest, sizeof(Drop.m_cComputerName)-1);
	}

	cf.m_hgData = NewGlobalP(&Drop, sizeof(Drop));
	cf.m_cfType = theApp.m_RemoteCF_HDROP;

	m_pClip->m_Formats.Add(cf);

	//m_pClip->m_Formats now owns the data
	cf.m_hgData = NULL;
}

