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
		if(theApp.m_bAppExiting || theApp.m_bExitServerThread)
			break;

		socket = accept(theApp.m_sSocket, (struct sockaddr*)&from,&fromlen);

		if( socket != INVALID_SOCKET )
			AfxBeginThread(ClientThread,(LPVOID)socket);
	}	

	LogSendRecieveInfo("End of Server Thread");

	bRunning = false;
	theApp.m_sSocket = INVALID_SOCKET;

	return 0;
}

CRecieveSocket::CRecieveSocket(SOCKET sk)
{
	m_pDataReturnedFromDecrypt = NULL;
	m_Socket = sk;
	m_pEncryptor = new CEncryption; //CreateEncryptionInterface("encryptdecrypt.dll");
}

CRecieveSocket::~CRecieveSocket()
{
	if(m_pEncryptor)
	{
		m_pEncryptor->FreeBuffer(m_pDataReturnedFromDecrypt);

		delete m_pEncryptor;
		m_pEncryptor = NULL;
		//ReleaseEncryptionInterface(m_pEncryptor);
	}
	closesocket(m_Socket);
}

void CRecieveSocket::FreeDecryptedData()
{ 
	if(g_Opt.m_csPassword == "")
	{
		delete [] m_pDataReturnedFromDecrypt;
	}
	else
	{
		m_pEncryptor->FreeBuffer(m_pDataReturnedFromDecrypt);
	}
	m_pDataReturnedFromDecrypt = NULL;
}

LPVOID CRecieveSocket::ReceiveEncryptedData(long lInSize, long &lOutSize)
{
	if(m_pEncryptor == NULL)
	{
		LogSendRecieveInfo("ReceiveEncryptedData::Encryption not initialized");
		return NULL;
	}

	if(m_pDataReturnedFromDecrypt)
		FreeDecryptedData();

	char *pInput = new char[lInSize];

	UCHAR* pOutput = NULL;
	
	if(pInput)
	{
		RecieveExactSize(pInput, lInSize);

		int nOut = 0;
		CString csPassword;
		int nCount = g_Opt.m_csNetworkPasswordArray.GetSize() + 1;
		for(int i = -1; i < nCount; i++)
		{
			if(i == -1)
				csPassword = g_Opt.m_csPassword;
			else
				csPassword = g_Opt.m_csNetworkPasswordArray[i];

			if(m_pEncryptor->Decrypt((UCHAR*)pInput, lInSize, csPassword, pOutput, nOut) == FALSE)
			{
				LogSendRecieveInfo(StrF("ReceiveEncryptedData:: Failed to Decrypt data password = %s", g_Opt.m_csPassword));
			}
			else
			{
				break;
			}
		}

		lOutSize = nOut;

		delete [] pInput;
		pInput = NULL;
	}	
	else
	{
		ASSERT(FALSE);
		LogSendRecieveInfo(StrF("ReceiveEncryptedData:: Failed to create new data size = %d", lInSize));
	}

	m_pDataReturnedFromDecrypt = pOutput;

	return pOutput;
}

BOOL CRecieveSocket::RecieveExactSize(char *pData, long lSize)
{
	long lReceiveCount = 0;

	long lWanted = lSize;
	long lOffset = 0;

	while(lWanted > 0)
	{
		lReceiveCount = recv(m_Socket, pData + lOffset, lWanted, 0);
		if(lReceiveCount == SOCKET_ERROR)
		{
			LogSendRecieveInfo("RecieveExactSize:: ********ERROR if(lReceiveCount == SOCKET_ERROR)*******");
			return FALSE;
		}
		else if(lReceiveCount == 0)
		{
			LogSendRecieveInfo("RecieveExactSize:: ********ERROR lRecieveCount == 0");
			return FALSE;
		}

		LogSendRecieveInfo(StrF("RecieveExactSize:: ------Bytes Read %d Total Recieved %d", lReceiveCount, lOffset));

		lWanted -= lReceiveCount;
		lOffset += lReceiveCount;
	}

	LogSendRecieveInfo(StrF("RecieveExactSize:: ------END RecieveExactSize Recieved %d", lOffset));

	return TRUE;
}

#define ENCRYPTED_SIZE_CSENDINFO 508

BOOL CRecieveSocket::RecieveCSendInfo(CSendInfo *pInfo)
{
	BOOL bRet = FALSE;
	long lOutSize = 0;

	long lRecieveSize = ENCRYPTED_SIZE_CSENDINFO;

	LPVOID lpData = ReceiveEncryptedData(lRecieveSize, lOutSize);
	if(lpData)
	{
		memcpy(pInfo, lpData, sizeof(CSendInfo));

		bRet = (pInfo->m_nSize == sizeof(CSendInfo));

		FreeDecryptedData();
	}

	return bRet;
}

UINT  ClientThread(LPVOID pParam)
{	
	LogSendRecieveInfo("*********************Start of ClientThread*********************");
	
	CRecieveSocket Sock((SOCKET)pParam);

	CClipList *pClipList = NULL;
	CClip *pClip = NULL;
	CClipFormat cf;
	CSendInfo info;
	bool bBreak = false;
	BOOL bSetToClipBoard = FALSE;
	
	while(true)
	{
		if(Sock.RecieveCSendInfo(&info) == FALSE)
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

				cf.m_cfType = GetFormatID(info.m_cDesc);
				cf.m_hgData = 0;
				
				long lInSize = info.m_lParameter1;
				long lOutSize = 0;

				LPVOID lpData = Sock.ReceiveEncryptedData(lInSize, lOutSize);

				if(lpData && lOutSize > 0)
				{					
					cf.m_hgData = NewGlobal(lOutSize);

					if(cf.m_hgData)
					{
						if(pClip)
							pClip->m_lTotalCopySize += lOutSize;

						LPVOID pvData = GlobalLock(cf.m_hgData);
						if(pvData)
						{
							memcpy(pvData, lpData, lOutSize);

							GlobalUnlock(cf.m_hgData);
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

					Sock.FreeDecryptedData();
				}

				LogSendRecieveInfo("::DATA_START -- END");
			}
			break;
		case MyEnums::DATA_END:
			{
				LogSendRecieveInfo("::DATA_END");
				
				if(pClip && cf.m_hgData)
				{
					pClip->m_Formats.Add(cf);
					cf.m_hgData = 0; // now owned by pClip
				}
				else
				{
					LogSendRecieveInfo("MyEnums::DATA_END Error if(pClip && cf.m_hgData)");
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
		default:
			LogSendRecieveInfo("::ERROR unknown action type exiting");
			bBreak = true;
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

	LogSendRecieveInfo("*********************End of ClientThread*********************");
	
	return 0;
}