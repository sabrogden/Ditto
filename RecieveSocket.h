#pragma once

#include "Winsock2.h"
#include "Encryption.h"
#include "ServerDefines.h"

class CRecieveSocket
{
public:
	CRecieveSocket(SOCKET sk = NULL);

	~CRecieveSocket();
	
	LPVOID ReceiveEncryptedData(long lInSize, long &lOutSize);
	BOOL RecieveExactSize(char *pData, long lSize);
	BOOL RecieveCSendInfo(CSendInfo *pInfo);

	void FreeDecryptedData();

	SOCKET	GetSocket()				{ return m_Sock;	}
	void	SetSocket(SOCKET sock)	{ m_Sock = sock;	}

protected:
	CEncryption *m_pEncryptor;
	SOCKET m_Sock;
	UCHAR *m_pDataReturnedFromDecrypt;	
};