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
	local.sin_port = htons((u_short)20248);
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

BOOL Recv(SOCKET sock, SendInfo *pInfo)
{
	long lReceiveCount = 0;
	long lLastRecievedCount = 0;
	long lExpectedCount = sizeof(SendInfo);
	SendInfo info;

	while(lLastRecievedCount < lExpectedCount)
	{
		lReceiveCount = recv(sock, (char*)&info, lExpectedCount-lLastRecievedCount, 0);
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

		LogSendRecieveInfo(StrF("------Bytes Read %d LastRecievedCount %d", lReceiveCount, lLastRecievedCount));

		memcpy(((char*)pInfo) + lLastRecievedCount, &info, lReceiveCount);
		//lReceiveCount = lLastRecievedCount + lReceiveCount;
		lLastRecievedCount += lReceiveCount;
	}

	if(pInfo->m_nSize != lExpectedCount)
	{
		LogSendRecieveInfo(StrF("------ERROR Incoming struct size %d Expected Size %d", pInfo->m_nSize, lExpectedCount));
		return FALSE;
	}

	return TRUE;
}

UINT  ClientThread(LPVOID pParam)
{	
	LogSendRecieveInfo("*********************Start of ClientThread*********************");
	
	SOCKET socket = (SOCKET)pParam;

	CClipList *pClipList = NULL;
	CClip *pClip = NULL;
	CClipFormat cf;
	SendInfo info;
	int nRecvReturn = 0;
	long lBytesCopied;
	bool bBreak = false;
	BYTE *pByte = NULL;
	long lByteSize = 0;
	BOOL bSetToClipBoard = FALSE;
	
	while(true)
	{
		if(Recv(socket, &info) == FALSE)
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
				cs.Format("%s\n(%s)(%s)", info.m_cText, info.m_cComputerName, info.m_cIP);
				
				if(pClip)
				{
					pClip->m_Desc = cs;
				}
				
				bSetToClipBoard = FALSE;

				if(g_Opt.m_csIPListToPutOnClipboard.Find(info.m_cIP) >= 0)
					bSetToClipBoard = TRUE;

				if(g_Opt.m_csIPListToPutOnClipboard.Find(info.m_cComputerName) >= 0)
					bSetToClipBoard = TRUE;
				
				info.m_cText[20] = 0;
				LogSendRecieveInfo(StrF("::START %s %s %s", info.m_cText, info.m_cComputerName, info.m_cIP));
			}
			break;
		case MyEnums::DATA_START:
			{
				if(pByte != NULL)
				{
					delete pByte;
					pByte = NULL;
				}
				
				lByteSize = info.m_lParameter1;
				pByte = new BYTE[info.m_lParameter1];

				cf.m_cfType = GetFormatID(info.m_cText);
				lBytesCopied = 0;
				if(pClip)
					pClip->m_lTotalCopySize += info.m_lParameter1;

				LogSendRecieveInfo(StrF("::DATA_START Total Size = %d type = %s", lByteSize, info.m_cText));
			}
			break;
		case MyEnums::DATA:
			{
				memcpy(((char*)pByte) + lBytesCopied, info.m_cText, info.m_lParameter1);
				lBytesCopied += info.m_lParameter1;

				LogSendRecieveInfo(StrF("::DATA Copy Bytes = %d TotalCopy = %d", info.m_lParameter1, lBytesCopied));
			}
			break;
		case MyEnums::DATA_END:
			{
				if(pByte)
				{
					cf.m_hgData = NewGlobalP(pByte, lByteSize);

					if(pClip)
					{
						pClip->m_Formats.Add(cf);
						cf.m_hgData = 0; // now owned by pClip
					}

					delete pByte;
					pByte = NULL;
				}
				else
					LogSendRecieveInfo("::ERROR pByte was null");


				LogSendRecieveInfo("::DATA_END");
			}
			break;
		case MyEnums::END:
			{				
				if(pClipList == NULL)
					pClipList = new CClipList;

				if(pClipList)
				{
					pClipList->AddTail(pClip);
					pClip = NULL;
				}
				else
					LogSendRecieveInfo("::ERROR pClipList was NULL");

				LogSendRecieveInfo("::END");

			}
			break;
		case MyEnums::EXIT:
			{				
				if(pClipList && pClipList->GetCount() > 0)
				{
					//Post a message pClipList will be freed by the reciever
					::PostMessage(theApp.m_MainhWnd, WM_ADD_TO_DATABASE_FROM_SOCKET, (WPARAM)pClipList, bSetToClipBoard);
					pClipList = NULL;
				}
				else
					LogSendRecieveInfo("::ERROR pClipList was NULL or Count was 0");

				LogSendRecieveInfo("::EXIT");
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