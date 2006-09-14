// SendSocket.h: interface for the CSendSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SENDSOCKET_H__4A4A5B96_8123_413C_8F86_A2875C9E5E48__INCLUDED_)
#define AFX_SENDSOCKET_H__4A4A5B96_8123_413C_8F86_A2875C9E5E48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Encryption.h"
#include "ServerDefines.h"

class CSendSocket  
{
public:
	CSendSocket();
	virtual ~CSendSocket();

	void SetSocket(SOCKET sock)		{ m_Connection = sock;	}

	CEncryption *m_pEncryptor;

	BOOL SendCSendData(CSendInfo &data, MyEnums::eSendType type);
	BOOL SendExactSize(char *pData, long lLength, bool bEncrypt);

protected:
	SOCKET m_Connection;
};

#endif // !defined(AFX_SENDSOCKET_H__4A4A5B96_8123_413C_8F86_A2875C9E5E48__INCLUDED_)
