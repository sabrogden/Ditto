// Client.h: interface for the CClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_)
#define AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Server.h"

class CID
{
public:
	CID()
	{
		lID = -1;
	}

	~CID()
	{
		m_Formats.RemoveAll();
	}
	
	long lID;
	CString m_csDesc;
	CClipFormats m_Formats;
};

class CIDArray
{
public:
	CIDArray()
	{
		pIDs = NULL;
		lCount = 0;
	}
	CID *pIDs;
	long lCount;
};

class CSendToFriendInfo
{
public:
	CSendToFriendInfo()
	{
		pPopup = NULL;
		pArray = NULL;
		lPos = -1;
		bRet = FALSE;
		hThreadRunning = CreateEvent(NULL, FALSE, FALSE, "SendToFriend");
		ResetEvent(hThreadRunning);
	}
	~CSendToFriendInfo()
	{
		CloseHandle(hThreadRunning);
	}

	CIDArray *pArray;
	long lPos;
	CString csIP;
	CPopup *pPopup;
	BOOL bRet;
	HANDLE hThreadRunning;
};

class CClient  
{
public:
	CClient();
	virtual ~CClient();

	BOOL SendItem(CID *pData);
	
	BOOL OpenConnection(const char* servername);
	BOOL CloseConnection();

protected:
	SOCKET m_Connection;

	BOOL SendData(SendInfo *pInfo, MyEnums::eSendType type);
};

BOOL SendToFriend(CIDArray *pArray, long lPos, CString csIP, CPopup *pPopup);
UINT SendToFriendThread(LPVOID pParam);

UINT  SendClientThread(LPVOID pParam);

#endif // !defined(AFX_CLIENT_H__E9B55197_85B2_4ABF_92FF_E6301F346404__INCLUDED_)
