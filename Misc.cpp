#include "stdafx.h"
#include "CP_Main.h"
#include "Misc.h"
#include "OptionsSheet.h"

#ifdef AFTER_98
	#include "AlphaBlend.h"
#endif


long DoOptions(CWnd *pParent)
{
	//Don't let it open up more than once
	if(theApp.m_bShowingOptions)
		return FALSE;

	theApp.m_bShowingOptions = true;
	
	COptionsSheet Sheet("Copy Pro Options", pParent);

	int nRet = Sheet.DoModal();

	theApp.m_bShowingOptions = false;

	return nRet;
}

HWND GetActiveWnd(CPoint *pPointCaret)
{
	HWND hWndFocus = NULL;
	if (pPointCaret)
		*pPointCaret = CPoint(-1, -1);
	
	HWND hWndForground = GetForegroundWindow(); // Get the desktop's foreground window
	if (hWndForground != NULL)
	{
		DWORD ProcID;
		DWORD ThreadID = GetWindowThreadProcessId(hWndForground, &ProcID);
		
		// Attach other thread's message queue to our own to ensure GetFocus() is working properly
		BOOL ARes = AttachThreadInput(ThreadID, GetCurrentThreadId(), TRUE);
		if (ARes)
		{
			// Get the other thread's focussed window
			CWnd *pWnd = CWnd::FromHandle(hWndForground);

			if (pWnd)
			{
				CWnd *pWndFocus = pWnd->GetFocus();
				if (pWndFocus)
				{
					hWndFocus = pWndFocus->m_hWnd;
					if (pPointCaret)
					{
						*pPointCaret = pWndFocus->GetCaretPos();
						pWndFocus->ClientToScreen(pPointCaret);
					}
				}
			}

			// Detach other thread's message queue from our own again
			ARes = AttachThreadInput(ThreadID, GetCurrentThreadId(), FALSE);
		}
	}

	return hWndFocus;
}


//Do not change these these are stored in the database
CLIPFORMAT GetFormatID(LPCSTR cbName)
{
	if(strcmp(cbName, "CF_TEXT") == 0)
		return CF_TEXT;
	else if(strcmp(cbName, "CF_METAFILEPICT") == 0)
		return CF_METAFILEPICT;
	else if(strcmp(cbName, "CF_SYLK") == 0)
		return CF_SYLK;
	else if(strcmp(cbName, "CF_DIF") == 0)
		return CF_DIF;
	else if(strcmp(cbName, "CF_TIFF") == 0)
		return CF_TIFF;
	else if(strcmp(cbName, "CF_OEMTEXT") == 0)
		return CF_OEMTEXT;
	else if(strcmp(cbName, "CF_DIB") == 0)
		return CF_DIB;
	else if(strcmp(cbName, "CF_PALETTE") == 0)
		return CF_PALETTE;
	else if(strcmp(cbName, "CF_PENDATA") == 0)
		return CF_PENDATA;
	else if(strcmp(cbName, "CF_RIFF") == 0)
		return CF_RIFF;
	else if(strcmp(cbName, "CF_WAVE") == 0)
		return CF_WAVE;
	else if(strcmp(cbName, "CF_UNICODETEXT") == 0)
		return CF_UNICODETEXT;
	else if(strcmp(cbName, "CF_ENHMETAFILE") == 0)
		return CF_ENHMETAFILE;
	else if(strcmp(cbName, "CF_HDROP") == 0)
		return CF_HDROP;
	else if(strcmp(cbName, "CF_LOCALE") == 0)
		return CF_LOCALE;
	else if(strcmp(cbName, "CF_OWNERDISPLAY") == 0)
		return CF_OWNERDISPLAY;
	else if(strcmp(cbName, "CF_DSPTEXT") == 0)
		return CF_DSPTEXT;
	else if(strcmp(cbName, "CF_DSPBITMAP") == 0)
		return CF_DSPBITMAP;
	else if(strcmp(cbName, "CF_DSPMETAFILEPICT") == 0)
		return CF_DSPMETAFILEPICT;
	else if(strcmp(cbName, "CF_DSPENHMETAFILE") == 0)
		return CF_DSPENHMETAFILE;
	
	
	return ::RegisterClipboardFormat(cbName);
}

//Do not change these these are stored in the database
CString GetFormatName(CLIPFORMAT cbType)
{
	switch(cbType)
	{
	case CF_TEXT:
		return "CF_TEXT";
	case CF_BITMAP:
		return "CF_BITMAP";
	case CF_METAFILEPICT:
		return "CF_METAFILEPICT";
	case CF_SYLK:
		return "CF_SYLK";
	case CF_DIF:
		return "CF_DIF";
	case CF_TIFF:
		return "CF_TIFF";
	case CF_OEMTEXT:
		return "CF_OEMTEXT";
	case CF_DIB:
		return "CF_DIB";
	case CF_PALETTE:
		return "CF_PALETTE";
	case CF_PENDATA:
		return "CF_PENDATA";
	case CF_RIFF:
		return "CF_RIFF";
	case CF_WAVE:
		return "CF_WAVE";
	case CF_UNICODETEXT:
		return "CF_UNICODETEXT";
	case CF_ENHMETAFILE:
		return "CF_ENHMETAFILE";
	case CF_HDROP:
		return "CF_HDROP";
	case CF_LOCALE:
		return "CF_LOCALE";
	case CF_OWNERDISPLAY:
		return "CF_OWNERDISPLAY";
	case CF_DSPTEXT:
		return "CF_DSPTEXT";
	case CF_DSPBITMAP:
		return "CF_DSPBITMAP";
	case CF_DSPMETAFILEPICT:
		return "CF_DSPMETAFILEPICT";
	case CF_DSPENHMETAFILE:
		return "CF_DSPENHMETAFILE";
	default:
	{
		//Not a default type get the name from the clipboard
		if (cbType != 0)
		{
			TCHAR szFormat[256];
            GetClipboardFormatName(cbType, szFormat, 256);
			return szFormat;
		}
		break;
	}
	}

	return "ERROR";
}

CString GetFilePath(CString csFileName)
{
	long lSlash = csFileName.ReverseFind('\\');
			
	if(lSlash > -1)
	{
		csFileName = csFileName.Left(lSlash + 1);
	}

	return csFileName;
}

CGetSetOptions::CGetSetOptions()
{

}

CGetSetOptions::~CGetSetOptions()
{

}

long CGetSetOptions::GetProfileLong(CString csName, long bDefaultValue)
{
	HKEY hkKey;

	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T(REG_PATH),
								NULL, KEY_READ, &hkKey);
	
	if(lResult != ERROR_SUCCESS)
		return bDefaultValue;

	DWORD buffer;
	DWORD len =  sizeof(buffer);
	DWORD type;

	lResult = ::RegQueryValueEx(hkKey, csName, 0, &type, (LPBYTE)&buffer, &len);
	
	RegCloseKey(hkKey);
	
	if(lResult == ERROR_SUCCESS)
		return (long)buffer;

	return bDefaultValue;
}

CString CGetSetOptions::GetProfileString(CString csName, CString csDefault)
{
	HKEY hkKey;

	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T(REG_PATH),
								NULL, KEY_READ, &hkKey);
	
	char szString[256];
	DWORD dwBufLen = 256;
	
	lResult = ::RegQueryValueEx(hkKey , csName, NULL, NULL, (LPBYTE)szString, &dwBufLen);

	if(lResult != ERROR_SUCCESS)
		return csDefault;

	return CString(szString);
}

BOOL CGetSetOptions::SetProfileLong(CString csName, long lValue)
{
	HKEY hkKey;
	DWORD dWord;
	long lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T(REG_PATH), NULL, 
						NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
						NULL, &hkKey, &dWord);

	if(lResult != ERROR_SUCCESS)
		return FALSE;

	DWORD val = (DWORD)lValue;
	lResult = ::RegSetValueEx(hkKey, csName, 0, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));

	RegCloseKey(hkKey);

	return lResult == ERROR_SUCCESS;
}

BOOL CGetSetOptions::SetProfileString(CString csName, CString csValue)
{
	HKEY hkKey;
	DWORD dWord;
	long lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T(REG_PATH), NULL, 
						NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
						NULL, &hkKey, &dWord);

	if(lResult != ERROR_SUCCESS)
		return FALSE;

	::RegSetValueEx(hkKey, csName, NULL, REG_SZ,
			(BYTE*)(LPCTSTR)csValue, csValue.GetLength()+sizeof(TCHAR));

	RegCloseKey(hkKey);

	return lResult == ERROR_SUCCESS;
}

BOOL CGetSetOptions::GetShowIconInSysTray() 
{
	return GetProfileLong("ShowIconInSystemTray", TRUE);
}

BOOL CGetSetOptions::SetShowIconInSysTray(BOOL bShow) 
{
	return SetProfileLong("ShowIconInSystemTray", bShow);
}

DWORD CGetSetOptions::GetHotKey() 
{
	//704 is ctrl-tilda
	return (DWORD)GetProfileLong("HotKey", 704);
}

BOOL CGetSetOptions::SetHotKey(DWORD dwHotKey) 
{
	return SetProfileLong("HotKey", dwHotKey);
}

DWORD CGetSetOptions::GetNamedCopyHotKey() 
{
	return (DWORD)GetProfileLong("NamedCopyHotKey", 0);
}

BOOL CGetSetOptions::SetNamedCopyHotKey(long lHotKey) 
{
	return SetProfileLong("NamedCopyHotKey", lHotKey);
}

BOOL CGetSetOptions::SetEnableTransparency(BOOL bCheck)
{
	return SetProfileLong("EnableTransparency", bCheck);
}

BOOL CGetSetOptions::GetEnableTransparency()
{
	return GetProfileLong("EnableTransparency", FALSE);
}

BOOL CGetSetOptions::SetTransparencyPercent(long lPercent)
{
	#ifdef AFTER_98
		if(lPercent > OPACITY_MAX)
			lPercent = OPACITY_MAX;
		if(lPercent < 0)
			lPercent = 0;

		return SetProfileLong("TransparencyPercent", lPercent);
	#endif
		return FALSE;
}

long CGetSetOptions::GetTransparencyPercent()
{
	#ifdef AFTER_98
		long lValue = GetProfileLong("TransparencyPercent", 14);

		if(lValue > OPACITY_MAX) lValue = OPACITY_MAX;
		if(lValue < 0) lValue = 0;

		return lValue;
	#endif
		return 0;
}

BOOL CGetSetOptions::SetLinesPerRow(long lLines)
{
	return SetProfileLong("LinesPerRow", lLines);
}

long CGetSetOptions::GetLinesPerRow()
{
	return GetProfileLong("LinesPerRow", 2);
}

BOOL CGetSetOptions::GetRunOnStartUp()
{
	HKEY hkRun;
	
	LONG nResult = RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
		NULL, KEY_READ, &hkRun);
	
	if(nResult != ERROR_SUCCESS)
		return FALSE;

	nResult = RegQueryValueEx(hkRun, GetAppName(), NULL, NULL, NULL, NULL);
	RegCloseKey(hkRun);
	return nResult == ERROR_SUCCESS;
}

void CGetSetOptions::SetRunOnStartUp(BOOL bRun)
{
	if(bRun == GetRunOnStartUp())
		return;

	HKEY hkRun;
	LONG nResult = RegOpenKeyEx(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
		NULL, KEY_ALL_ACCESS, &hkRun);

	if(nResult != ERROR_SUCCESS)
		return;

	if(bRun)
	{
		CString sExeName = GetExeFileName();
		::RegSetValueEx(hkRun, GetAppName(), NULL, REG_SZ,
			(BYTE*)(LPCTSTR)sExeName, sExeName.GetLength()+sizeof(TCHAR));
	} 
	else 
	{
		::RegDeleteValue(hkRun, GetAppName());
	}

	::RegCloseKey(hkRun);
}

CString CGetSetOptions::GetExeFileName()
{
	CString sExeName;
	GetModuleFileName(NULL, sExeName.GetBuffer(_MAX_PATH),_MAX_PATH);
	sExeName.ReleaseBuffer();
	return sExeName;
}

CString CGetSetOptions::GetAppName()
{
	return "Ditto";
}

BOOL CGetSetOptions::SetQuickPastePosition(long lPosition)
{
	return SetProfileLong("ShowQuickPastePosition", lPosition);
}

long CGetSetOptions::GetQuickPastePosition()
{
	return GetProfileLong("ShowQuickPastePosition", POS_AT_PREVIOUS);
}

BOOL CGetSetOptions::RegisterHotKey(HWND hWnd, DWORD wHotKey, ATOM atomID)
{
	if(wHotKey == 0)
		return FALSE;

	return ::RegisterHotKey(hWnd, atomID, GetModifier(wHotKey), LOBYTE(wHotKey));
}

UINT CGetSetOptions::GetModifier(DWORD wHotKey)
{
	UINT uMod = 0;
	if( HIBYTE(wHotKey) & HOTKEYF_SHIFT )   uMod |= MOD_SHIFT;
	if( HIBYTE(wHotKey) & HOTKEYF_CONTROL ) uMod |= MOD_CONTROL;
	if( HIBYTE(wHotKey) & HOTKEYF_ALT )     uMod |= MOD_ALT;
	if( HIBYTE(wHotKey) & HOTKEYF_EXT )     uMod |= MOD_WIN;

	return uMod;
}

BOOL CGetSetOptions::SetQuickPasteSize(CSize size)
{
	BOOL bRet = SetProfileLong("QuickPasteCX", size.cx);
	bRet = SetProfileLong("QuickPasteCY", size.cy);

	return bRet;
}
void CGetSetOptions::GetQuickPasteSize(CSize &size)
{
	size.cx = GetProfileLong("QuickPasteCX", 300);
	size.cy = GetProfileLong("QuickPasteCY", 300);
}

BOOL CGetSetOptions::SetQuickPastePoint(CPoint point)
{
	BOOL bRet = SetProfileLong("QuickPasteX", point.x);
	bRet = SetProfileLong("QuickPasteY", point.y);

	return bRet;
}

void CGetSetOptions::GetQuickPastePoint(CPoint &point)
{
	point.x = GetProfileLong("QuickPasteX", 300);
	point.y = GetProfileLong("QuickPasteY", 300);
}

long CGetSetOptions::GetCopyGap()
{
	return GetProfileLong("CopyGap", 150);
}

BOOL CGetSetOptions::SetDBPath(CString csPath)
{
	return SetProfileString("DBPath", csPath);
}

CString CGetSetOptions::GetDBPath(BOOL bDefault/* = TRUE*/)
{
	//First check the reg string
	CString csDefaultPath = GetProfileString("DBPath", "");

	//If there is nothing in the regesty then get the default
	//In the users application data in my documents
	if(bDefault)
	{
		if(csDefaultPath.IsEmpty())
			csDefaultPath = GetDefaultDBName();
	}
		
	return csDefaultPath;
}

void CGetSetOptions::SetCheckForMaxEntries(BOOL bVal)
{
	SetProfileLong("CheckForMaxEntries", bVal);
}

BOOL CGetSetOptions::GetCheckForMaxEntries()
{
	return GetProfileLong("CheckForMaxEntries", 0);
}

void CGetSetOptions::SetCheckForExpiredEntries(BOOL bVal)
{
	SetProfileLong("CheckForExpiredEntries", bVal);
}

BOOL CGetSetOptions::GetCheckForExpiredEntries()
{
	return GetProfileLong("CheckForExpiredEntries", 0);
}

void CGetSetOptions::SetMaxEntries(long lVal)
{
	SetProfileLong("MaxEntries", lVal);
}

long CGetSetOptions::GetMaxEntries()
{
	return GetProfileLong("MaxEntries", 500);
}

void CGetSetOptions::SetExpiredEntries(long lVal)
{
	SetProfileLong("ExpiredEntries", lVal);
}

long CGetSetOptions::GetExpiredEntries()
{
	return GetProfileLong("ExpiredEntries", 5);
}

void CGetSetOptions::SetTripCopyCount(long lVal)
{
	if(lVal == -1)
		lVal = GetTripCopyCount() + 1;

	if(GetTripDate() == 0)
		SetTripDate(-1);

	SetProfileLong("TripCopies", lVal);
}

long CGetSetOptions::GetTripCopyCount()
{
	return GetProfileLong("TripCopies", 0);
}

void CGetSetOptions::SetTripPasteCount(long lVal)
{
	if(lVal == -1)
		lVal = GetTripPasteCount() + 1;

	if(GetTripDate() == 0)
		SetTripDate(-1);

	SetProfileLong("TripPastes", lVal);
}

long CGetSetOptions::GetTripPasteCount()
{
	return GetProfileLong("TripPastes", 0);
}

void CGetSetOptions::SetTripDate(long lDate)
{
	if(lDate == -1)
		lDate = (long)CTime::GetCurrentTime().GetTime();

	SetProfileLong("TripDate", lDate);
}

long CGetSetOptions::GetTripDate()
{
	return GetProfileLong("TripDate", 0);
}

void CGetSetOptions::SetTotalCopyCount(long lVal)
{
	if(lVal == -1)
		lVal = GetTotalCopyCount() + 1;

	if(GetTotalDate() == 0)
		SetTotalDate(-1);

	SetProfileLong("TotalCopies", lVal);
}

long CGetSetOptions::GetTotalCopyCount()
{
	return GetProfileLong("TotalCopies", 0);
}

void CGetSetOptions::SetTotalPasteCount(long lVal)
{
	if(lVal == -1)
		lVal = GetTotalPasteCount() + 1;

	if(GetTotalDate() == 0)
		SetTotalDate(-1);

	SetProfileLong("TotalPastes", lVal);
}

long CGetSetOptions::GetTotalPasteCount()
{
	return GetProfileLong("TotalPastes", 0);
}

void CGetSetOptions::SetTotalDate(long lDate)
{
	if(lDate == -1)
		lDate = (long)CTime::GetCurrentTime().GetTime();

	SetProfileLong("TotalDate", lDate);
}

long CGetSetOptions::GetTotalDate()
{
	return GetProfileLong("TotalDate", 0);
}

void CGetSetOptions::SetCompactAndRepairOnExit(BOOL bVal)
{
	SetProfileLong("CompactAndRepairOnExit", bVal);
}

BOOL CGetSetOptions::GetCompactAndRepairOnExit()
{
	return GetProfileLong("CompactAndRepairOnExit", 0);
}
