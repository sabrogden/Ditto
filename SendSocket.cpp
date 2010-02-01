// SendSocket.cpp: implementation of the CSendSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "SendSocket.h"
#include "shared/TextConvert.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSendSocket::CSendSocket()
{
	m_pEncryptor = new CEncryption;
	m_Connection = NULL;
}

CSendSocket::~CSendSocket()
{
	delete m_pEncryptor;
	m_pEncryptor = NULL;
}


BOOL CSendSocket::SendCSendData(CSendInfo &data, MyEnums::eSendType type)
{
	data.m_Type = type;
	return SendExactSize((char *)&data, sizeof(CSendInfo), true);
}

BOOL CSendSocket::SendExactSize(char *pData, long lLength, bool bEncrypt)
{
	BOOL bRet = FALSE;
	if(!m_pEncryptor && bEncrypt)
	{
		ASSERT(!"Encryption not initialized");
		LogSendRecieveInfo("SendExactSize::Encryption not initialized");
		return bRet;
	}

	LogSendRecieveInfo(StrF(_T("START SendExactSize Total %d"), lLength));

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
				LogSendRecieveInfo(StrF(_T("lSize == SOCKET_ERROR, %d"), WSAGetLastError()));
				bRet = FALSE;
				break;
			}
			lBytesRead += lSize;
		}

		if(lBytesRead == lExpected)
			bRet = TRUE;

		if(pOutput != (UCHAR*)pData)
			m_pEncryptor->FreeBuffer(pOutput);
	}
	else
	{
		LogSendRecieveInfo("SendExactSize::Failed to encrypt data");
	}

//	LogSendRecieveInfo(StrF(_T("END SendExactSize Total %d"), lBytesRead));

	return bRet;
}