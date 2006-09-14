// FileRecieve.cpp: implementation of the CFileRecieve class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "FileRecieve.h"
#include "RecieveSocket.h"
#include "TextConvert.h"
#include "Path.h"
#include "UnicodeMacros.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFileRecieve::CFileRecieve()
{
	m_pProgress = NULL;
}

CFileRecieve::~CFileRecieve()
{
}

long CFileRecieve::RecieveFiles(SOCKET sock, CString csIP, CFileTransferProgressDlg *pProgress)
{
	CSendInfo Info;
	BOOL bBreak = false;
	BOOL lRet = FALSE;
	int nNumFiles = 0;
	int nFilesRecieved = 0;

	m_pProgress = pProgress;
	m_csRecievingFromIP = csIP;
	m_Sock.SetSocket(sock);

	while(true)
	{
		if(m_Sock.RecieveCSendInfo(&Info) == FALSE)
			break;

		switch(Info.m_Type)
		{
		case MyEnums::START:
			nNumFiles = Info.m_lParameter1;
			m_pProgress->SetNumFiles(nNumFiles);
			break;
		case MyEnums::DATA_START:
		{
			CString csFileName;
			nFilesRecieved++;
			CTextConvert::ConvertFromUTF8(Info.m_cDesc, csFileName);

			m_pProgress->StepAllFiles();
			m_pProgress->SetMessage(StrF(_T("Copying File %d of %d"), nFilesRecieved, nNumFiles));
			m_pProgress->SetFileMessage(StrF(_T("Copying %s"), csFileName));
			m_pProgress->PumpMessages();
			if(m_pProgress->Cancelled())
			{
				lRet = USER_CANCELED;
				bBreak = true;
				break;
			}

			ULONG lFileSize = (ULONG)Info.m_lParameter1;

			long lRecieveRet = RecieveFileData(lFileSize, csFileName);
			if(lRecieveRet == USER_CANCELED)
			{
				lRet = USER_CANCELED;
				bBreak = true;
				break;
			}
			else if(lRecieveRet == FALSE)
			{
				LogSendRecieveInfo(StrF(_T("Error recieving the file %s"), csFileName));
			}
			else
			{
				lRet = TRUE;
			}
		}
		break;

		case MyEnums::DATA_END:
			break;

		case MyEnums::END:
			bBreak = true;
			break;

		default:
			LogSendRecieveInfo("::ERROR unknown action type exiting");
			bBreak = true;
		}

		if(bBreak || theApp.m_bAppExiting)
			break;
	}

	return lRet;
}

long CFileRecieve::RecieveFileData(ULONG lFileSize, CString csFileName)
{
	CString csFile = CGetSetOptions::GetPath(PATH_REMOTE_FILES);
	CreateDirectory(csFile, NULL);
	csFile += m_csRecievingFromIP + "\\";
	CreateDirectory(csFile, NULL);
	
	nsPath::CPath path(csFileName);
	csFile += path.GetName();

	CFile File;
	CFileException ex;
	if(File.Open(csFile, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary, &ex) == FALSE)
	{
		TCHAR szError[200];
		ex.GetErrorMessage(szError, 200);
		LogSendRecieveInfo(StrF(_T("Error opening file in RequestCopiedFiles, error: %s"), szError));

		return FALSE;
	}
	
	ULONG lBytesRead = 0;
	long lBytesNeeded = 0;
	int nPercent = 0;
	int nPrevPercent = 0;

	char *pBuffer = new char[CHUNK_WRITE_SIZE];
	if(pBuffer == NULL)
	{
		LogSendRecieveInfo("Error creating buffer in RequestCopiedFiles");
		return FALSE;
	}

	BOOL bRet = FALSE;
	while(true)
	{
		lBytesNeeded = CHUNK_WRITE_SIZE;
		if(lFileSize - lBytesRead < CHUNK_WRITE_SIZE)
			lBytesNeeded = lFileSize - lBytesRead;

		if(m_Sock.RecieveExactSize(pBuffer, lBytesNeeded) == FALSE)
		{
			break;
		}
		
		File.Write(pBuffer, lBytesNeeded);

		lBytesRead += lBytesNeeded;

		if(lBytesRead >= lFileSize)
		{
			m_pProgress->SetSingleFilePos(100);
			bRet = TRUE;
			break;
		}

		if(lBytesNeeded > 0)
		{
			nPercent = (int)((lBytesRead / (double)lFileSize) * 100);
			if((nPercent - nPrevPercent) > 5)
			{
				m_pProgress->SetSingleFilePos(nPercent);
				m_pProgress->PumpMessages();
				if(m_pProgress->Cancelled())
				{
					bRet = USER_CANCELED;
					break;
				}

				nPrevPercent = nPercent;
			}
		}
	}

	File.Close();

	if(bRet)
	{
		m_RecievedFiles.Add(csFile);
	}

	delete []pBuffer;
	pBuffer = NULL;

	return bRet;
}

HGLOBAL CFileRecieve::CreateCF_HDROPBuffer()
{
	int nFileArraySize = m_RecievedFiles.GetSize();
	if(nFileArraySize <= 0)
		return NULL;

	TCHAR *pBuff = NULL;
	int	 nBuffSize = 0;

	for(int i = 0; i < nFileArraySize; i++)
	{
		nBuffSize += m_RecievedFiles[i].GetLength()*sizeof(TCHAR)+1;
	}

	nBuffSize += sizeof(DROPFILES)+2;
	nBuffSize = (nBuffSize/32 + 1)*32;

	pBuff = new TCHAR[nBuffSize];

	ZeroMemory(pBuff, nBuffSize);
	((DROPFILES*)pBuff)->pFiles = sizeof(DROPFILES);

#ifdef _UNICODE
	((DROPFILES*)pBuff)->fWide = TRUE;
#endif

	TCHAR* pCurrent = (TCHAR*)(((char*)pBuff) + sizeof(DROPFILES));

	for(int n = 0; n < nFileArraySize; n++)
	{
		STRCPY(pCurrent, (LPCTSTR)m_RecievedFiles[n]);

		pCurrent += m_RecievedFiles[n].GetLength()*sizeof(TCHAR); 
		*pCurrent = 0;
		pCurrent++;
	}

	HGLOBAL hReturn = NewGlobalP(pBuff, nBuffSize);

	delete []pBuff;
	pBuff = NULL;

	return hReturn;
}