// Server.h: interface for the CServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVER_H__581A45D4_DCC2_44D7_8B43_60412E769D39__INCLUDED_)
#define AFX_SERVER_H__581A45D4_DCC2_44D7_8B43_60412E769D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Winsock2.h"
#include "Encryption.h"

class MyEnums
{
public:
	enum eSendType{START, DATA, DATA_START, DATA_END, END, EXIT};
};

class CSendInfo
{
public:
	CSendInfo()
	{
		m_nSize = sizeof(CSendInfo);
		m_nVersion = 1;
		m_lParameter1 = -1;
		m_lParameter2 = -1;
	}
	int					m_nSize;
	MyEnums::eSendType	m_Type;
	int					m_nVersion;
	char				m_cIP[20];
	char				m_cComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	char				m_cDesc[250];
	long				m_lParameter1;
	long				m_lParameter2;
	char				m_cExtra[50];
};

class CRecieveSocket
{
public:
	CRecieveSocket(SOCKET sk);

	~CRecieveSocket();
	
	LPVOID ReceiveEncryptedData(long lInSize, long &lOutSize);
	BOOL RecieveExactSize(char *pData, long lSize);
	BOOL RecieveCSendInfo(CSendInfo *pInfo);

	void FreeDecryptedData();

protected:
	CEncryption *m_pEncryptor;
	SOCKET m_Socket;
	UCHAR *m_pDataReturnedFromDecrypt;	
};

UINT  MTServerThread(LPVOID pParam);
UINT  ClientThread(LPVOID pParam);

#endif // !defined(AFX_SERVER_H__581A45D4_DCC2_44D7_8B43_60412E769D39__INCLUDED_)
