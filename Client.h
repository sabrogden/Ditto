// Client.h: interface for the CClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_)
#define AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Server.h"

class CSendToFriendInfo
{
public:
	CSendToFriendInfo()
	{
		m_pPopup = NULL;
		m_lPos = -1;
		m_pClipList = NULL;
		m_pPopup = NULL;
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
	long m_lPos;
	CString m_csIP;
	CPopup *m_pPopup;
	CString m_csErrorText;
};

class CClient  
{
public:
	CClient();
	virtual ~CClient();

	BOOL SendItem(CClip *pClip);
	
	BOOL OpenConnection(const char* servername);
	BOOL CloseConnection();

protected:
	SOCKET m_Connection;

	BOOL SendData(SendInfo *pInfo, MyEnums::eSendType type);
};

BOOL SendToFriend(CSendToFriendInfo &Info);

UINT  SendClientThread(LPVOID pParam);

#endif // !defined(AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_)
