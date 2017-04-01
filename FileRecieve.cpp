// FileRecieve.cpp: implementation of the CFileRecieve class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "FileRecieve.h"
#include "RecieveSocket.h"
#include "shared/TextConvert.h"
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
	m_Sock.SetProgressBar(pProgress);

	while(true)
	{
		if(m_Sock.RecieveCSendInfo(&Info) == FALSE)
			break;

		switch(Info.m_Type)
		{
		case MyEnums::START:
			nNumFiles = Info.m_lParameter1;
			if(m_pProgress != NULL)
			{
				m_pProgress->SetNumFiles(nNumFiles);
			}
			LogSendRecieveInfo(StrF(_T("Start recieving files File Count: %d"), nNumFiles));
			break;
		case MyEnums::DATA_START:
		{
			CString csFileName;
			nFilesRecieved++;
			CTextConvert::ConvertFromUTF8(Info.m_cDesc, csFileName);

			if(m_pProgress != NULL)
			{
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
			}

			ULONG lFileSize = (ULONG)Info.m_lParameter1;

			LogSendRecieveInfo(StrF(_T("START of recieving the file %s, size: %d, File %d of %d"), csFileName, lFileSize, nFilesRecieved, nNumFiles));

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
				LogSendRecieveInfo(StrF(_T("END of recieving the file %s, size: %d"), csFileName, lFileSize));
				lRet = TRUE;
			}
		}
		break;

		case MyEnums::DATA_END:
		{
			if (Info.m_lParameter1 != 0 &&
				Info.m_lParameter2 != 0 &&
				m_RecievedFiles.GetCount() > 0)
			{
				FILETIME lastWriteTime;
				lastWriteTime.dwLowDateTime = Info.m_lParameter1;
				lastWriteTime.dwHighDateTime = Info.m_lParameter2;

				HANDLE filename = CreateFile(m_RecievedFiles[m_RecievedFiles.GetCount() - 1], FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);				
				if(filename != NULL)
				{
					SetFileTime(filename, NULL, NULL, &lastWriteTime);
				}
			}
		}

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

HGLOBAL CFileRecieve::CreateCF_HDROPBufferAsString()
{
	CString data;
	int nFileArraySize = (int) m_RecievedFiles.GetSize();
	for (int i = 0; i < nFileArraySize; i++)
	{
		data += m_RecievedFiles[i];
		data += _T("\r\n");
	}

	HGLOBAL hReturn = NewGlobalP(data.GetBuffer(), (data.GetLength() + 1)*sizeof(TCHAR));

	return hReturn;
}

HGLOBAL CFileRecieve::CreateCF_HDROPBuffer()
{
	int nFileArraySize = (int)m_RecievedFiles.GetSize();
	if(nFileArraySize <= 0)
	{
		LogSendRecieveInfo(_T("Recieved files array is empty not creating cf_hdrop structure"));
		return NULL;
	}

	TCHAR *pBuff = NULL;
	int	 nBuffSize = 0;

	for(int i = 0; i < nFileArraySize; i++)
	{
		nBuffSize += m_RecievedFiles[i].GetLength()+1;
	}

	// Add 1 extra for the final null char,
	// and the size of the DROPFILES struct.
	nBuffSize = sizeof(DROPFILES) + (sizeof(TCHAR) * (nBuffSize + 1));

	pBuff = new TCHAR[nBuffSize];

	ZeroMemory(pBuff, nBuffSize);
	((DROPFILES*)pBuff)->pFiles = sizeof(DROPFILES);
	((DROPFILES*)pBuff)->fWide = TRUE;

	TCHAR* pCurrent = (TCHAR*)(LPBYTE(pBuff) + sizeof(DROPFILES));

	for(int n = 0; n < nFileArraySize; n++)
	{
		STRCPY(pCurrent, (LPCTSTR)m_RecievedFiles[n]);

		LogSendRecieveInfo(StrF(_T("CreateCF_HDROPBuffer adding the file '%s' to local cf_hdrop structure"), pCurrent));

		pCurrent += m_RecievedFiles[n].GetLength(); 
		*pCurrent = 0;
		pCurrent++;
	}

	HGLOBAL hReturn = NewGlobalP(pBuff, nBuffSize);

	delete []pBuff;
	pBuff = NULL;

	return hReturn;
}