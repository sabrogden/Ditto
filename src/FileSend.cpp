// FileSend.cpp: implementation of the CFileSend class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "FileSend.h"
#include "Server.h"
#include "..\Shared\TextConvert.h"
#include "Md5.h"

#include <shlwapi.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//using namespace nsPath;


CFileSend::CFileSend()
{

}

CFileSend::~CFileSend()
{

}

BOOL CFileSend::SendClientFiles(SOCKET sock, CClipList *pClipList)
{
	if(!pClipList || pClipList->GetCount() <= 0)
	{
		LogSendRecieveInfo("::ERROR SendClientFiles called either pClipList was null or empty");
		return FALSE;
	}

	m_Send.SetSocket(sock);

	CSendInfo Info;
	BOOL bRet = FALSE;

	CStringArray CopyFiles;

	CClipFormat *pFormat = GetCF_HDROP_Data(pClipList);
	if(pFormat)
	{
		HDROP drop = (HDROP)GlobalLock(pFormat->m_hgData);
 		int nNumFiles = DragQueryFile(drop, -1, NULL, 0);
 		TCHAR file[MAX_PATH];

 		for(int nFile = 0; nFile < nNumFiles; nFile++)
 		{
 			if(DragQueryFile(drop, nFile, file, sizeof(file)) > 0)
 			{
				if(PathIsDirectory(file) == FALSE)
				{
					CopyFiles.Add(file);
				}
 			}
 		}

		GlobalUnlock(pFormat->m_hgData);
	}

	Info.m_lParameter1 = (long)CopyFiles.GetSize();
	if(Info.m_lParameter1 > 0)
	{
		if(m_Send.SendCSendData(Info, MyEnums::START))
		{
			for(int nFile = 0; nFile < Info.m_lParameter1; nFile++)
			{
				SendFile(CopyFiles[nFile]);
			}
		}
	}
	
	if(m_Send.SendCSendData(Info, MyEnums::END))
			bRet = TRUE;
	
	return bRet;
}

CClipFormat* CFileSend::GetCF_HDROP_Data(CClipList *pClipList)
{
	CClip* pClip;
	CClipFormat* pCF;
	POSITION pos;
	
	pos = pClipList->GetHeadPosition();
	while(pos)
	{
		pClip = pClipList->GetNext(pos);
		if(pClip)
		{
			pCF = pClip->m_Formats.FindFormat(CF_HDROP);
			if(pCF)
				return pCF;
		}
	}

	return NULL;
}
 
BOOL CFileSend::SendFile(CString csFile)
{
	CFile file;
	BOOL bRet = FALSE;
	CSendInfo Info;
	
	char *pBuffer = new char[CHUNK_WRITE_SIZE];
	if(pBuffer == NULL)
	{
		LogSendRecieveInfo("Error creating buffer to send file over in");
		return FALSE;
	}
	
	try
	{
		CFileException ex;
		if(file.Open(csFile, CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone, &ex))
		{
			CStringA dest = CTextConvert::UnicodeToUTF8(csFile);
			strncpy(Info.m_cDesc, dest, sizeof(Info.m_cDesc));
			Info.m_cDesc[sizeof(Info.m_cDesc)-1] = 0;			

			Info.m_lParameter1 = (long)file.GetLength();
			if(m_Send.SendCSendData(Info, MyEnums::DATA_START))
			{
				long lReadBytes = 0;
				BOOL bError = FALSE;
				CMd5 md5;
				md5.MD5Init();

				BOOL calcMd5 = CGetSetOptions::GetCheckMd5OnFileTransfers();

				DWORD d = GetTickCount();
				
				do
				{
					lReadBytes = file.Read(pBuffer, CHUNK_WRITE_SIZE);
					
					if(m_Send.SendExactSize(pBuffer, lReadBytes, false) == FALSE)
					{
						LogSendRecieveInfo("Error sending SendExactSize in SendFile");
						bError = TRUE;
						break;
					}

					if (calcMd5)
					{
						md5.MD5Update((unsigned char *)pBuffer, lReadBytes);
					}
					
				}while(lReadBytes >= CHUNK_WRITE_SIZE);
				
				DWORD end = GetTickCount() - d;

				if(bError == FALSE)
				{
					Info.m_lParameter1 = 0;
					Info.m_lParameter2 = 0;

					FILETIME creationTime;
					FILETIME lastAccessTime;
					FILETIME lastWriteTime;

					if (GetFileTime(file, &creationTime, &lastAccessTime, &lastWriteTime))
					{
						Info.m_lParameter1 = lastWriteTime.dwLowDateTime;
						Info.m_lParameter2 = lastWriteTime.dwHighDateTime;
					}
										
					CStringA csMd5 = md5.MD5FinalToString();
					strncpy(Info.m_md5, csMd5, sizeof(Info.m_md5));

					LogSendRecieveInfo(StrF(_T("Sending data_end for file: %s, md5: %s"), csFile, CTextConvert::AnsiToUnicode(csMd5)));

					if(m_Send.SendCSendData(Info, MyEnums::DATA_END))
						bRet = TRUE;
				}
			}
		}
		else
		{
			TCHAR szError[100];
			ex.GetErrorMessage(szError, 100);
			LogSendRecieveInfo(StrF(_T("Error opening file in Send file, error: %s"), szError));
		}
	}
	catch(CFileException *e)
	{
		TCHAR szError[100];
		e->GetErrorMessage(szError, 100);
		LogSendRecieveInfo(StrF(_T("Exception - Error in Send file, error: %s"), szError));
	}
	
	delete []pBuffer;
	pBuffer = NULL;
	
	return bRet;
}