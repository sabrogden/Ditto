#include "StdAfx.h"
#include "RecieveSocket.h"
#include "Options.h"
#include "Misc.h"
#include "CP_Main.h"
#include "TextConvert.h"

CRecieveSocket::CRecieveSocket(SOCKET sock)
{
	m_pDataReturnedFromDecrypt = NULL;
	m_Sock = sock;
	m_pEncryptor = new CEncryption; //CreateEncryptionInterface("encryptdecrypt.dll");
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
		CStringA csPassword;
		int nCount = g_Opt.m_csNetworkPasswordArray.GetSize();
		int nIndex;
		for(int i = -2; i < nCount; i++)
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
				csPassword = g_Opt.m_csPassword;
			}
			else
			{
				if(nIndex >= 0 && nIndex < nCount)
				{
					CTextConvert::ConvertToUTF8(g_Opt.m_csNetworkPasswordArray[nIndex], csPassword);
				}
				else
					continue;
			}

			if(m_pEncryptor->Decrypt((UCHAR*)pInput, lInSize, csPassword, pOutput, nOut) == FALSE)
			{
				LogSendRecieveInfo(StrF(_T("ReceiveEncryptedData:: Failed to Decrypt data password = %s"), g_Opt.m_csPassword));
			}
			else
			{
				theApp.m_lLastGoodIndexForNextworkPassword = nIndex;
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
		LogSendRecieveInfo(StrF(_T("ReceiveEncryptedData:: Failed to create new data size = %d"), lInSize));
	}

	m_pDataReturnedFromDecrypt = pOutput;

	return pOutput;
}

BOOL CRecieveSocket::RecieveExactSize(char *pData, long lSize)
{
	LogSendRecieveInfo(StrF(_T("RecieveExactSize:: ------ Start wanted size %d"), lSize));

	long lReceiveCount = 0;

	long lWanted = lSize;
	long lOffset = 0;

	while(lWanted > 0)
	{
		lReceiveCount = recv(m_Sock, pData + lOffset, lWanted, 0);
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