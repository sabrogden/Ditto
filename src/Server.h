#pragma once

#include "Winsock2.h"
#include "..\EncryptDecrypt\Encryption.h"
#include "..\Shared\TextConvert.h"
#include "RecieveSocket.h"
#include "FileSend.h"
#include "ServerDefines.h"


class SocketParams
{
public:
	SOCKET m_socket;
	CString m_ip;
};

class CServer
{
public:
	CServer();
	~CServer();

	void RunThread(SocketParams *pParams);

protected:
	void AddRemoteCF_HDROPFormat();

	void OnStart(CSendInfo &Info);
	void OnDataEnd(CSendInfo &info);
	void OnDataStart(CSendInfo &info);
	void OnEnd(CSendInfo &info);
	void OnExit(CSendInfo &info);
	void OnRequestFiles(CSendInfo &info);

protected:
	CClipList *m_pClipList;
	CClip *m_pClip;
	CTextConvert m_Convert;
	BOOL m_bSetToClipBoard;
	CString m_csIP;
	bool m_manualSend;
	short m_respondPort;
	CString m_csComputerName;
	CString m_csDesc;
	CRecieveSocket m_Sock;
	CClipFormat m_cf;
	CString m_recieveIP;
};

UINT  MTServerThread(LPVOID pParam);
UINT  ClientThread(LPVOID pParam);
