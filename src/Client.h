// Client.h: interface for the CClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_)
#define AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Server.h"
#include "..\EncryptDecrypt\Encryption.h"
#include "SendSocket.h"
#include "Popup.h"

class CSendToFriendInfo
{
public:
	CSendToFriendInfo()
	{
		m_pPopup = NULL;
		m_pClipList = NULL;
		m_pPopup = NULL;
		m_manualSend = false;
	}
	~CSendToFriendInfo()
	{
		if(m_pClipList)
		{
			delete m_pClipList;
			m_pClipList = NULL;
		}
	}

	CClipList *m_pClipList;
	CString m_csIP;
	CPopup *m_pPopup;
	CString m_csErrorText;
	bool m_manualSend;
};

class CClient  
{
public:
	CClient();
	virtual ~CClient();

	BOOL SendItem(CClip *pClip, bool manualSend);
	
	BOOL OpenConnection(const TCHAR* servername);
	BOOL CloseConnection();

	HGLOBAL RequestCopiedFiles(CClipFormat &HDropFormat, CString csIP, CString csComputerName);

protected:
	SOCKET m_Connection;
	int m_connectionPort;

	CSendSocket m_SendSocket;

	BOOL SendClipFormat(CClipFormat* pCF);
	
protected:
	
};

BOOL SendToFriend(CSendToFriendInfo &Info);

UINT  SendClientThread(LPVOID pParam);

#endif // !defined(AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_)
