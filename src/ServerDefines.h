#pragma once


#define CHUNK_WRITE_SIZE 65536

class MyEnums
{
public:
	enum eSendType{START, DATA, DATA_START, DATA_END, END, EXIT, REQUEST_FILES};
};

class CSendInfo
{
public:
	CSendInfo()
	{
		memset(this, 0, sizeof(*this));
		m_nSize = sizeof(CSendInfo);
		m_nVersion = 1;
		m_lParameter1 = -1;
		m_lParameter2 = -1;
		m_manualSend = 0;
		m_respondPort = 0;
	}
	int					m_nSize;
	MyEnums::eSendType	m_Type;
	int					m_nVersion;
	CHAR				m_cIP[20];
	CHAR				m_cComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	CHAR				m_cDesc[250];
	long				m_lParameter1;
	long				m_lParameter2;
	CHAR				m_md5[32];
	char				m_manualSend;
	short				m_respondPort;
	char				m_cExtra[15];
};

class CDittoCF_HDROP
{
public:
	CDittoCF_HDROP()
	{
		memset(m_cIP, 0, sizeof(m_cIP));
		memset(m_cComputerName, 0, sizeof(m_cComputerName));
		respondPort = 0;
	}
	char m_cIP[25];
	char m_cComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	short respondPort;
};