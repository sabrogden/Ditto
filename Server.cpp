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

	while(lReceiveCount < lExpectedCount)
	{
		lReceiveCount = recv(sock, (char*)&info, lExpectedCount-lReceiveCount, 0);
		if(lReceiveCount == SOCKET_ERROR)
		{
			LogSendRecieveInfo("********ERROR if(lReceiveCount == SOCKET_ERROR)*******");
			return FALSE;
		}
		else if(lReceiveCount == 0)
			return FALSE;

		LogSendRecieveInfo(StrF("------Bytes Read %d", lReceiveCount));

		memcpy(((char*)pInfo) + lLastRecievedCount, &info, lReceiveCount);
		lReceiveCount = lLastRecievedCount + lReceiveCount;
		lLastRecievedCount += lReceiveCount;
	}

	return TRUE;
}

UINT  ClientThread(LPVOID pParam)
{	
	LogSendRecieveInfo("*********************Start of ClientThread*********************");
	
	SOCKET socket = (SOCKET)pParam;

	SendInfo info;
	int nRecvReturn = 0;
	CClip NewClip;
	CClipFormat cf;
	long lBytesCopied;
	bool bBreak = false;
	bool bAddToDatabase = false;
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
				NewClip.Clear();

				CString cs;
				cs.Format("%s\n(%s)(%s)", info.m_cText, info.m_cComputerName, info.m_cIP);
				
				NewClip.m_Desc = cs;

				bAddToDatabase = false;

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
				NewClip.m_lTotalCopySize += info.m_lParameter1;

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
				cf.m_hgData = NewGlobalP(pByte, lByteSize);

				NewClip.m_Formats.Add(cf);

				delete pByte;
				pByte = NULL;

				bAddToDatabase = true;

				LogSendRecieveInfo("::DATA_END");
			}
			break;
		case MyEnums::END:
			{
				if(bAddToDatabase)
				{
					LogSendRecieveInfo("::END");
					//::SendMessage(theApp.m_MainhWnd, WM_ADD_TO_DATABASE_FROM_SOCKET, (WPARAM)&NewClip, bSetToClipBoard);
				}
			}
			break;
		case MyEnums::EXIT:
			{				
				LogSendRecieveInfo("::EXIT");
				bBreak = true;
			}
			break;
		}

		if(bBreak || theApp.m_bAppExiting)
			break;
	}		

	closesocket(socket);

	LogSendRecieveInfo("*********************End of ClientThread*********************");
	
	return 0;
}