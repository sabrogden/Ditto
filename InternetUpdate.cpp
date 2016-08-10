// InternetUpdate.cpp: implementation of the CInternetUpdate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "InternetUpdate.h"
#include <afxinet.h>
#include "ProgressWnd.h"
#include "io.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HTTPBUFLEN    512 // Size of HTTP Buffer...

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInternetUpdate::CInternetUpdate()
{

}

CInternetUpdate::~CInternetUpdate()
{

}

BOOL CInternetUpdate::CheckForUpdate(HWND hParent, BOOL bCheckForPrevUpdate, BOOL bShowNoUpdatesDlg)
{
	m_bShowMessages = bShowNoUpdatesDlg;
	m_hParent = hParent;
	
	CTime Now = CTime::GetCurrentTime();
	struct tm ptmTemp;
	tm tmNow = *(Now.GetLocalTm(&ptmTemp));
	long lCurrentDayOfYear = tmNow.tm_yday;

	RemoveOldUpdateFile();
	
	if(bCheckForPrevUpdate)
	{
		if(!CGetSetOptions::GetCheckForUpdates())
			return FALSE;

		long lLastUpdateDay = CGetSetOptions::GetLastUpdate();

		if(lCurrentDayOfYear - lLastUpdateDay < 10)
			return FALSE;

		//if the last time we check was today return
		if(lLastUpdateDay == lCurrentDayOfYear)
			return FALSE;
	}

	CGetSetOptions::SetLastUpdate(lCurrentDayOfYear);
	
	BOOL bRet = FALSE;
	
	m_lRunningVersion = GetRunningVersion();
	m_lUpdateVersion = GetUpdateVersion();

	if(m_lUpdateVersion > m_lRunningVersion)
	{
		CString csMessage;
		csMessage.Format(	_T("%s, %s\n")
							_T("%s, %s\n\n")
							_T("%s"),
							theApp.m_Language.GetString("Updates_Available", "Updates available for Ditto.\nVisit ditto-cp.sourceforge.net for details\n\nRunning Version"),
							GetVersionString(m_lRunningVersion), 
							theApp.m_Language.GetString("Update_Version", "Update Version"),
							GetVersionString(m_lUpdateVersion),
							theApp.m_Language.GetString("Download_Update", "Download updated version?"));

		if(MessageBox(hParent, csMessage, _T("Ditto"), MB_YESNO) == IDYES)
		{
			CString csFile = DownloadUpdate();

			if(!csFile.IsEmpty())
			{
				CloseHandle(theApp.m_hMutex);
				Sleep(100);
				ShellExecute(NULL, NULL, csFile, NULL, NULL, SW_SHOWNORMAL);
			}

			bRet = TRUE;
		}
	}
	else if(m_bShowMessages)
	{
		MessageBox(hParent, theApp.m_Language.GetString("No_Updates", "No updates available"), _T("Ditto"), MB_OK);
	}

	return bRet;
}

BOOL CInternetUpdate::RemoveOldUpdateFile()
{
	CString csFile = CGetSetOptions::GetPath(PATH_UPDATE_FILE);
	csFile += "DittoSetup.exe";
	
	BOOL bRet = TRUE;
	if(FileExists(csFile))
	{
		bRet = ::DeleteFile(csFile);
	}

	return bRet;
}

CString CInternetUpdate::GetVersionString(long lVersion)
{
	CString csLine;
	csLine.Format(_T("%02i.%02i.%02i.%02i"), 
		(lVersion >> 24) & 0x03f,
		(lVersion >> 16) & 0x03f,
		((lVersion >> 8) & 0x07f),
		lVersion & 0x07f);

	return csLine;
}

long CInternetUpdate::GetRunningVersion()
{
	CString csFileName = CGetSetOptions::GetExeFileName();

	DWORD dwSize, dwHandle;
	LPBYTE lpData;
	UINT iBuffSize;
	VS_FIXEDFILEINFO *lpFFI;
	long ver;

	dwSize = GetFileVersionInfoSize(csFileName.GetBuffer(csFileName.GetLength()), &dwHandle);

	if(dwSize != 0)
	{
		csFileName.ReleaseBuffer();
		if((lpData=(unsigned char *)malloc(dwSize)) != NULL)
		{
			if(GetFileVersionInfo(csFileName.GetBuffer(csFileName.GetLength()), dwHandle, dwSize, lpData) != 0)
			{
				if(VerQueryValue(lpData, _T("\\"), (LPVOID*)&lpFFI, &iBuffSize) != 0)
				{
					if(iBuffSize > 0)
					{
						ver =        (HIWORD(lpFFI->dwFileVersionMS) & 0x00ff) << 24;
						ver = ver + ((LOWORD(lpFFI->dwFileVersionMS) & 0x00ff) << 16);
						ver = ver + ((HIWORD(lpFFI->dwFileVersionLS) & 0x00ff) << 8);
						ver = ver +   LOWORD(lpFFI->dwFileVersionLS);
						free(lpData);
						return(ver);
					}
				}
			}
			free(lpData);
		}
	}

    return(0);
}

long CInternetUpdate::GetUpdateVersion()
{
	char httpbuff[HTTPBUFLEN];
	
	//Try to get a path from the regestry
	CString csPath = CGetSetOptions::GetUpdateFilePath();

	//if nothing there get the default
	if(csPath.IsEmpty())
	{
		csPath = "ditto-cp.sourceforge.net/Update3/DittoVersion.txt";
	}
	
	CString csUrl = "http://" + csPath;
	
	CString csFile = CGetSetOptions::GetPath(PATH_UPDATE_FILE);
	csFile += "DittoVersion.txt";

	bool bError = false;
	CStdioFile *remotefile = NULL;

	long lReturn = -1;
	try
	{
		CInternetSession mysession;
		remotefile = mysession.OpenURL(csUrl,1,INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_RELOAD);
		if(!remotefile)
			return 0;
		
		CFile myfile(csFile, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		
		UINT unBytesRead = 0;
		UINT unTotalBytes = 0;
		
		while (unBytesRead = remotefile->Read(httpbuff, HTTPBUFLEN))
		{
			unTotalBytes += unBytesRead;
			myfile.Write(httpbuff, unBytesRead);

			if(!remotefile)
			{
				unTotalBytes = 0;
				break;
			}
		}
		
		myfile.Close();
		
		if(unTotalBytes)
		{
			CStdioFile file;
			if(file.Open(csFile, CFile::modeRead|CFile::typeText))
			{
				CString csVersion;
				if(file.ReadString(csVersion))
				{
					file.Close();		
					lReturn = ATOL(csVersion);
				}
			}
		}
	}
	catch(CInternetException *pEX)
	{
		bError = true;	
		pEX->Delete();
	}
	catch(CFileException *e)
	{
		bError = true;
		e->Delete();
	}
	catch(...)
	{
		bError = true;
		csFile.Empty();
	}

	if(bError)
	{
		if(m_bShowMessages)
		{
			MessageBox(m_hParent, _T("Error Connecting."), _T("Ditto"), MB_OK);
			m_bShowMessages = FALSE;
		}
	}
	
	if(remotefile)
	{
		remotefile->Close();

		delete remotefile;
		remotefile = NULL;
	}

	if(FileExists(csFile))
		CFile::Remove(csFile);

	return lReturn;
}

CString CInternetUpdate::DownloadUpdate()
{
	char httpbuff[HTTPBUFLEN];
	
	//Try to get a path from the regestry
	CString csPath = CGetSetOptions::GetUpdateInstallPath();

	//if nothing there get the default
	if(csPath.IsEmpty())
	{
		csPath = "ditto-cp.sourceforge.net/U3/DittoSetup.exe";
	}
		

	CString csUrl = "http://" + csPath;
	
	CString csFile = CGetSetOptions::GetPath(PATH_UPDATE_FILE);
	csFile += "DittoSetup.exe";

	long lReturn = -1;
	CHttpFile	*RemoteFile = NULL;

	try
	{
		CInternetSession mysession;

		RemoteFile = (CHttpFile*)mysession.OpenURL(csUrl,1,INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_RELOAD);
		if(!RemoteFile)
			return "";

		//Get the file size
		DWORD		dFileSize;
		RemoteFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, dFileSize);

		//Set up the progress wnd
		CProgressWnd progress;
		progress.Create(CWnd::FromHandlePermanent(m_hParent), _T("Ditto Update"));
		progress.SetRange(0, dFileSize, HTTPBUFLEN);
		progress.SetText(_T("Downloading Ditto Update ..."));
		
		//Create the file to put the info in
		CFile myfile(csFile, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		
		UINT unBytesRead = 0;
		UINT unTotalBytes = 0;
		
		//Read in the file
		while (unBytesRead = RemoteFile->Read(httpbuff, HTTPBUFLEN))
		{
			progress.StepIt();
			progress.PeekAndPump();
			if(progress.Cancelled())
			{
				csFile.Empty();
				break;
			}
			
			unTotalBytes += unBytesRead;
			myfile.Write(httpbuff, unBytesRead);

			if(!RemoteFile)
			{
				MessageBox(m_hParent, _T("Error Downloading update."), _T("Ditto"), MB_OK);
				csFile = "";
				break;
			}
		}
				
		myfile.Close();
	}
	catch(CInternetException *pEX)
	{
		MessageBox(m_hParent, _T("Error Downloading update."), _T("Ditto"), MB_OK);
		csFile.Empty();
		pEX->Delete();
	}
	catch(CFileException *e)
	{
		MessageBox(m_hParent, _T("Error Downloading update."), _T("Ditto"), MB_OK);
		csFile.Empty();
		e->Delete();
	}
	catch(...)
	{
		MessageBox(m_hParent, _T("Error Downloading update."), _T("Ditto"), MB_OK);
		csFile.Empty();
	}

	if(RemoteFile)
	{
		RemoteFile->Close();
		delete RemoteFile;
		RemoteFile = NULL;
	}

	return csFile;
}