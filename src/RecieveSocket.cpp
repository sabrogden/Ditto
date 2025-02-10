#include "StdAfx.h"
#include "RecieveSocket.h"
#include "Options.h"
#include "Misc.h"
#include "CP_Main.h"
#include "..\Shared\TextConvert.h"

CRecieveSocket::CRecieveSocket(SOCKET sock)
{
	m_pDataReturnedFromDecrypt = NULL;
	m_Sock = sock;
	m_pEncryptor = new CEncryption; //CreateEncryptionInterface("encryptdecrypt.dll");
	m_pProgress = NULL;
}

CRecieveSocket::~CRecieveSocket()
{
	if(m_pEncryptor)
	{
		m_pEncryptor->FreeBuffer(m_pDataReturnedFromDecrypt);

		delete m_pEncryptor;
		m_pEncryptor = NULL;
	}
}

void CRecieveSocket::FreeDecryptedData()
{ 
	if(CGetSetOptions::m_csPassword == "")
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
		int nOut = 0;

		if(RecieveExactSize(pInput, lInSize))
		{
			CStringA csPassword;
			INT_PTR count = CGetSetOptions::m_csNetworkPasswordArray.GetSize();
			INT_PTR nIndex;
			for(int i = -2; i < count; i++)
			{
				csPassword.Empty();
				nIndex = i;

				//First time through try the last index that was valid
				if(i == -2)
				{
					nIndex = theApp.m_lLastGoodIndexForNextworkPassword;
					if(nIndex == -2)
						continue;
				}

				if(nIndex == -1)
				{
					csPassword = CGetSetOptions::m_csPassword;
				}
				else
				{
					if(nIndex >= 0 && nIndex < count)
					{
						csPassword = CTextConvert::UnicodeToUTF8(CGetSetOptions::m_csNetworkPasswordArray[nIndex]);
					}
					else
					{
						continue;
					}
				}

				if(m_pEncryptor->Decrypt((UCHAR*)pInput, lInSize, csPassword, pOutput, nOut) == FALSE)
				{
					LogSendRecieveInfo(StrF(_T("ReceiveEncryptedData:: Failed to Decrypt data password = %s"), CGetSetOptions::m_csPassword));
				}
				else
				{
					theApp.m_lLastGoodIndexForNextworkPassword = (long)nIndex;
					break;
				}
			}
		}
		else
		{
			LogSendRecieveInfo(StrF(_T("ReceiveEncryptedData:: FAILED"), lInSize));
		}

		lOutSize = nOut;

		delete [] pInput;
		pInput = NULL;
	}	
	else
	{
		ASSERT(FALSE);
		LogSendRecieveInfo(StrF(_T("ReceiveEncryptedData:: Failed to create new data size = %d"), lInSize));
	}

	m_pDataReturnedFromDecrypt = pOutput;

	return pOutput;
}

int recv_to(int fd, char *buffer, int len, int flags, int to) 
{
	fd_set readset;
	int result, iof = -1;
	struct timeval tv;

	// Initialize the set
	FD_ZERO(&readset);
	FD_SET(fd, &readset);

	// Initialize time out struct
	tv.tv_sec = 0;
	tv.tv_usec = to * 1000;
	// select()
	result = select(fd+1, &readset, NULL, NULL, &tv);

	// Check status
	if (result < 0)
		return -1;
	else if (result > 0 && FD_ISSET(fd, &readset)) 
	{
		// Set non-blocking mode
		//if ((iof = fcntl(fd, F_GETFL, 0)) != -1)
		//	fcntl(fd, F_SETFL, iof | O_NONBLOCK);
		// receive
		result = recv(fd, buffer, len, flags);
		// set as before
		//if (iof != -1)
		//	fcntl(fd, F_SETFL, iof);
		return result;
	}
	return -2;
}

BOOL CRecieveSocket::RecieveExactSize(char *pData, long lSize)
{
	LogSendRecieveInfo(StrF(_T("RecieveExactSize:: ------ Start wanted size %d"), lSize));

	long lReceiveCount = 0;

	long lWanted = lSize;
	long lOffset = 0;
	CString originalText = _T("");

	while(lWanted > 0)
	{		
		fd_set readset;
		int res;

		int timeoutMs = CGetSetOptions::GetNetworkReadTimeoutMS();
		int loops100msEach = (timeoutMs/100);

		for(int i = 0; i < loops100msEach; i++)
		{
			lReceiveCount = recv_to(m_Sock, pData + lOffset, lWanted, 0, 100);

			if(lReceiveCount >= 0)
			{
				break;
			}
			else if(lReceiveCount == SOCKET_ERROR)
			{
				ASSERT(FALSE);
				LogSendRecieveInfo(StrF(_T("RecieveExactSize:: Socket Error")));
				return FALSE;
			}

			if(lReceiveCount == -2 && i > 15)
			{
				if(m_pProgress != NULL)
				{
					originalText = m_pProgress->GetMessage();
					m_pProgress->SetMessage(StrF(_T("Requesting data from Server")));					
					m_pProgress->PumpMessages();
					if(m_pProgress->Cancelled())
					{
						return FALSE;
					}
				}
			}
		}

		if(lReceiveCount == -2)
		{
			ASSERT(FALSE);
			LogSendRecieveInfo(StrF(_T("RecieveExactSize:: Timeout waiting for server")));
			return FALSE;
		}

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

		if(m_pProgress != NULL &&
			originalText != _T(""))
		{
			m_pProgress->SetMessage(originalText);					
		}

		lWanted -= lReceiveCount;
		lOffset += lReceiveCount;

		LogSendRecieveInfo(StrF(_T("RecieveExactSize:: ------Bytes Read %d Total Recieved %d"), lReceiveCount, lOffset));
	}

//	LogSendRecieveInfo(StrF(_T("RecieveExactSize:: ------END RecieveExactSize Recieved %d"), lOffset));

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