#include "stdafx.h"
#include "U3Start.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CU3StartApp theApp;

CString GetFilePath(CString csFileName)
{
	long lSlash = csFileName.ReverseFind('\\');

	if(lSlash > -1)
	{
		csFileName = csFileName.Left(lSlash + 1);
	}

	return csFileName;
}


BOOL CU3StartApp::InitInstance()
{
	CWinApp::InitInstance();

	//Load the ini file in the same dir as this is running
	CString sExeName;
	GetModuleFileName(NULL, sExeName.GetBuffer(_MAX_PATH),_MAX_PATH);
	sExeName.ReleaseBuffer();
	CString csPath = GetFilePath(sExeName);
	csPath += "DittohWnd.ini";
	
	//Load the running Ditto and send a close to it
	HWND hWnd = (HWND)GetPrivateProfileInt("Ditto", "MainhWnd", 0, csPath);
	
	CString cs;
	cs.Format("Ditto Stop sending WM_CLOSE to the hWnd = %d", hWnd);
	OutputDebugString(cs);

	SendMessage(hWnd, WM_CLOSE, 0, 0);

	OutputDebugString("Post Send Message");

	return FALSE;
}
