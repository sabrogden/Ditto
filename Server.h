// Server.h: interface for the CServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVER_H__581A45D4_DCC2_44D7_8B43_60412E769D39__INCLUDED_)
#define AFX_SERVER_H__581A45D4_DCC2_44D7_8B43_60412E769D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Winsock2.h"

class MyEnums
{
public:
	enum eSendType{START, DATA, DATA_START, DATA_END, END, EXIT};
};

#define MAX_DATA_SIZE	1000
class SendInfo
{
public:
	SendInfo()
	{
		m_nSize = sizeof(this);
		m_nVersion = 1;
	}
	int					m_nSize;
	MyEnums::eSendType	m_Type;
	int					m_nVersion;

	char				m_cIP[20];
	char				m_cComputerName[MAX_COMPUTERNAME_LENGTH + 1];

	long		m_lParameter1;
	long		m_lParameter2;
	char		m_cText[MAX_DATA_SIZE];
};

UINT  MTServerThread(LPVOID pParam);
UINT  ClientThread(LPVOID pParam);

#endif // !defined(AFX_SERVER_H__581A45D4_DCC2_44D7_8B43_60412E769D39__INCLUDED_)
