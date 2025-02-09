#pragma once

#include "SendSocket.h"

class CFileSend  
{
public:
	CFileSend();
	virtual ~CFileSend();

	BOOL SendClientFiles(SOCKET sock, CClipList *pClipList);

protected:
	CClipFormat* GetCF_HDROP_Data(CClipList *pClipList);
	BOOL SendFile(CString csFile);

protected:
	CSendSocket m_Send;
};
