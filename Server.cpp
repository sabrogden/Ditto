// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "Server.h"

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
	LogSendRecieveInfo("Start of ServerThread");

	WSADATA wsaData;
	sockaddr_in local;
	int wsaret = WSAStartup(0x101,&wsaData);
	if(wsaret!=0)
	{
		LogSendRecieveInfo("ERROR - int wsaret = WSAStartup(0x101,&wsaData);");
		return 0;
	}
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
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

	SOCKET socket;
	
	sockaddr_in from;
	int fromlen = sizeof(from);

	while(true)
	{
		if(theApp.m_bAppExiting)
			break;

		socket = accept(theApp.m_sSocket, (struct sockaddr*)&from,&fromlen);
		
		AfxBeginThread(ClientThread,(LPVOID)socket);
	}	

	LogSendRecieveInfo("End of Server Thread");

	return 0;
}

BOOL RecieveExactSize(SOCKET sock, char *pData, long lSize)
{
	long lReceiveCount = 0;

	long lWanted = lSize;
	long lOffset = 0;

	while(lWanted > 0)
	{
		lReceiveCount = recv(sock, pData + lOffset, lWanted, 0);
		if(lReceiveCount == SOCKET_ERROR)
		{
			LogSendRecieveInfo("********ERROR if(lReceiveCount == SOCKET_ERROR)*******");
			return FALSE;
		}
		else if(lReceiveCount == 0)
		{
			LogSendRecieveInfo("********ERROR lRecieveCount == 0");
			return FALSE;
		}

		LogSendRecieveInfo(StrF("------Bytes Read %d Total Recieved %d", lReceiveCount, lOffset));

		lWanted -= lReceiveCount;
		lOffset += lReceiveCount;
	}

	LogSendRecieveInfo(StrF("------END RecieveExactSize Recieved %d", lOffset));

	return TRUE;
}

BOOL RecieveCSendInfo(SOCKET sock, CSendInfo *pInfo)
{
	BOOL bRet = RecieveExactSize(sock, (char*)pInfo, sizeof(CSendInfo));
	if(bRet)
	{
		bRet = pInfo->m_nSize == sizeof(CSendInfo);
	}

	return bRet;
}

UINT  ClientThread(LPVOID pParam)
{	
	LogSendRecieveInfo("*********************Start of ClientThread*********************");
	
	SOCKET socket = (SOCKET)pParam;

	CClipList *pClipList = NULL;
	CClip *pClip = NULL;
	CClipFormat cf;
	CSendInfo info;
	bool bBreak = false;
	BOOL bSetToClipBoard = FALSE;
	
	while(true)
	{
		if(RecieveCSendInfo(socket, &info) == FALSE)
			break;
		
		switch(info.m_Type)
		{
		case MyEnums::START:
			{				
				if(pClip != NULL)
				{
					delete pClip;
					pClip = NULL;
				}
				
				pClip = new CClip;
				
				CString cs;
				cs.Format("%s\n(%s)(%s)", info.m_cDesc, info.m_cComputerName, info.m_cIP);
				
				if(pClip)
				{
					pClip->m_Desc = cs;
				}
				
				bSetToClipBoard = FALSE;

				if(g_Opt.m_csIPListToPutOnClipboard.Find(info.m_cIP) >= 0)
					bSetToClipBoard = TRUE;

				if(g_Opt.m_csIPListToPutOnClipboard.Find(info.m_cComputerName) >= 0)
					bSetToClipBoard = TRUE;
				
				info.m_cDesc[20] = 0;
				LogSendRecieveInfo(StrF("::START %s %s %s", info.m_cDesc, info.m_cComputerName, info.m_cIP));
			}
			break;
		case MyEnums::DATA_START:
			{
				LogSendRecieveInfo("::DATA_START -- START");
				cf.m_hgData = NewGlobal(info.m_lParameter1);
				cf.m_cfType = GetFormatID(info.m_cDesc);

				LogSendRecieveInfo(StrF("::--------DATA_START Total Size = %d type = %s", info.m_lParameter1, info.m_cDesc));

				if(pClip)
					pClip->m_lTotalCopySize += info.m_lParameter1;

				LogSendRecieveInfo("::--------Before RecieveExactSize");
				LPVOID pvData = GlobalLock(cf.m_hgData);
				
				//Recieve the clip data
				if(RecieveExactSize(socket, (char*)pvData, info.m_lParameter1) == FALSE)
					bBreak = true;

				GlobalUnlock(cf.m_hgData);
				LogSendRecieveInfo("::--------After RecieveExactSize");

				LogSendRecieveInfo("::DATA_START -- END");
			}
			break;
		case MyEnums::DATA_END:
			{
				LogSendRecieveInfo("::DATA_END");
				
				if(pClip)
				{
					pClip->m_Formats.Add(cf);
					cf.m_hgData = 0; // now owned by pClip
				}
			}
			break;
		case MyEnums::END:
			{				
				LogSendRecieveInfo("::END");

				if(pClipList == NULL)
					pClipList = new CClipList;

				if(pClipList)
				{
					pClipList->AddTail(pClip);
					pClip = NULL;
				}
				else
					LogSendRecieveInfo("::ERROR pClipList was NULL");
			}
			break;
		case MyEnums::EXIT:
			{				
				LogSendRecieveInfo("::EXIT");

				if(pClipList && pClipList->GetCount() > 0)
				{
					theApp.m_lClipsRecieved += pClipList->GetCount();

					//Post a message pClipList will be freed by the reciever
					::PostMessage(theApp.m_MainhWnd, WM_ADD_TO_DATABASE_FROM_SOCKET, (WPARAM)pClipList, bSetToClipBoard);
					pClipList = NULL;
				}
				else
					LogSendRecieveInfo("::ERROR pClipList was NULL or Count was 0");

				bBreak = true;
			}
			break;
		}

		if(bBreak || theApp.m_bAppExiting)
			break;
	}		

	if(pClipList)
	{
		delete pClipList;
		pClipList = NULL;

		LogSendRecieveInfo("::ERROR pClipList was not NULL something is wrong");
	}

	if(pClip)
	{
		delete pClip;
		pClip = NULL;

		LogSendRecieveInfo("::ERROR pClip was not NULL something is wrong");
	}

	closesocket(socket);

	LogSendRecieveInfo("*********************End of ClientThread*********************");
	
	return 0;
}