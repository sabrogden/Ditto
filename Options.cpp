#include "stdafx.h"
#include "Options.h"
#include "AlphaBlend.h"
#include "Misc.h"
#include "shared/TextConvert.h"
#include "sqlite\CppSQLite3.h"
#include "Path.h"

using namespace nsPath;

UINT WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nValue, LPCTSTR lpFileName)
{
	// Locals
	TCHAR	szBuff[25];

	// Format
	SPRINTF(szBuff, _T("%d"), nValue);

	// Write
	return WritePrivateProfileString(lpAppName, lpKeyName, szBuff, lpFileName);
}

long CGetSetOptions::m_nLinesPerRow;
BOOL CGetSetOptions::m_bUseCtrlNumAccel;
BOOL CGetSetOptions::m_bAllowDuplicates;
BOOL CGetSetOptions::m_bUpdateTimeOnPaste;
BOOL CGetSetOptions::m_bSaveMultiPaste;
BOOL CGetSetOptions::m_bShowPersistent;
BOOL CGetSetOptions::m_bHistoryStartTop;
long CGetSetOptions::m_bDescTextSize;
BOOL CGetSetOptions::m_bDescShowLeadingWhiteSpace;
BOOL CGetSetOptions::m_bAllwaysShowDescription;
long CGetSetOptions::m_bDoubleClickingOnCaptionDoes;
BOOL CGetSetOptions::m_bPrompForNewGroupName;
BOOL CGetSetOptions::m_bSendPasteOnFirstTenHotKeys;
CSendClients CGetSetOptions::m_SendClients[MAX_SEND_CLIENTS];
long CGetSetOptions::m_lAutoSendClientCount;
CString CGetSetOptions::m_csIPListToPutOnClipboard;
BOOL CGetSetOptions::m_bLogSendReceiveErrors;
BOOL CGetSetOptions::m_bUseHookDllForFocus;
BOOL CGetSetOptions::m_HideDittoOnHotKeyIfAlreadyShown;
long CGetSetOptions::m_lPort;
BOOL CGetSetOptions::m_bDrawThumbnail;
CStringA CGetSetOptions::m_csPassword;
BOOL CGetSetOptions::m_bDrawRTF;
BOOL CGetSetOptions::m_bMultiPasteReverse;
CString CGetSetOptions::m_csPlaySoundOnCopy;
CStringArray CGetSetOptions::m_csNetworkPasswordArray;
BOOL CGetSetOptions::m_bSendPasteMessageAfterSelection;
BOOL CGetSetOptions::m_bFindAsYouType;
BOOL CGetSetOptions::m_bEnsureEntireWindowCanBeSeen;
BOOL CGetSetOptions::m_bShowAllClipsInMainList;
long CGetSetOptions::m_lMaxClipSizeInBytes;
DWORD CGetSetOptions::m_dwSaveClipDelay;
long CGetSetOptions::m_lProcessDrawClipboardDelay;
BOOL CGetSetOptions::m_bEnableDebugLogging;
BOOL CGetSetOptions::m_bEnsureConnectToClipboard;
bool CGetSetOptions::m_bOutputDebugString;
bool CGetSetOptions::m_bU3 = false;
bool CGetSetOptions::m_bInConversion = false;
bool CGetSetOptions::m_bFromIni = false;
CString CGetSetOptions::m_csIniFileName;
__int64 CGetSetOptions::nLastDbWriteTime = 0;
CTheme CGetSetOptions::m_Theme;

CGetSetOptions g_Opt;

CGetSetOptions::CGetSetOptions()
{
}

CGetSetOptions::~CGetSetOptions()
{

}

void CGetSetOptions::LoadSettings()
{
	m_csIniFileName = GetIniFileName(true);

	if(m_bU3)
	{
		m_bFromIni = true;
	}
	else
	{
		//first check if ini file is in app directory
		if(FileExists(m_csIniFileName))
		{
			m_bFromIni = true;
		}
		else
		{
			CString portable = GetFilePath(m_csIniFileName);
			portable += _T("portable");
			if(FileExists(portable))
			{
				m_bFromIni = true;

				//local ini file doesn't exist but portable file does, create the ini file with defaults
				//This is done so they can copy the entire directory for portable zip files and not overright there settings file
				SetProfileLong(_T("SetCurrentDirectory"), 1);
				SetProfileLong(_T("Portable"), 1);
				SetProfileLong(_T("DisableRecieve"), 1);
				SetProfileLong(_T("CheckForMaxEntries"), 1);
				SetProfileLong(_T("MaxEntries"), 100);
				SetProfileLong(_T("UseHookDllForFocus"), 0);
			}
			else
			{
				//next check if it's in app data
				m_csIniFileName = GetIniFileName(false);
				if(FileExists(m_csIniFileName))
				{
					m_bFromIni = true;
				}
			}
		}
	}

	if(m_bFromIni)
	{
		CString csPath = GetFilePath(m_csIniFileName);
		if(FileExists(csPath) == FALSE)
			CreateDirectory(csPath, NULL);
	}

	//first time running set some defaults
	if(GetTotalCopyCount() <= 0)
	{
		SetMaxEntries(500);
		SetCheckForMaxEntries(TRUE);
		SetQuickPastePosition(POS_AT_CARET);
		SetDisableRecieve(TRUE);
	}

	GetSetCurrentDirectory();

	m_nLinesPerRow = GetLinesPerRow();
	m_bUseCtrlNumAccel = GetUseCtrlNumForFirstTenHotKeys();
	m_bAllowDuplicates = GetAllowDuplicates();
	m_bUpdateTimeOnPaste = GetUpdateTimeOnPaste();
	m_bSaveMultiPaste = GetSaveMultiPaste();
	m_bShowPersistent = GetShowPersistent();
	m_bHistoryStartTop = GetHistoryStartTop();
	m_bDescTextSize = GetDescTextSize();
	m_bDescShowLeadingWhiteSpace = GetDescShowLeadingWhiteSpace();
	m_bAllwaysShowDescription = GetAllwaysShowDescription();
	m_bDoubleClickingOnCaptionDoes = GetDoubleClickingOnCaptionDoes();
	m_bPrompForNewGroupName = GetPrompForNewGroupName();
	m_bSendPasteOnFirstTenHotKeys = GetSendPasteOnFirstTenHotKeys();
	m_csIPListToPutOnClipboard = GetListToPutOnClipboard();
	m_bLogSendReceiveErrors = GetLogSendReceiveErrors();
	
	if(IsRunningLimited() == false)
	{
		//If running from U3 then we can't use the hook dll because we are unable
		//to delete the hook dll, it's loaded by other processes
		if(m_bU3)
		{
			m_bUseHookDllForFocus = GetProfileLong("UseHookDllForFocus", FALSE);
		}
		else
		{
			m_bUseHookDllForFocus = GetProfileLong("UseHookDllForFocus", FALSE);
		}
	}
	else
	{
		m_bUseHookDllForFocus = false;
	}

	m_HideDittoOnHotKeyIfAlreadyShown = GetHideDittoOnHotKeyIfAlreadyShown();
	m_lPort = GetPort();
	m_bDrawThumbnail = GetDrawThumbnail();
	m_csPassword = GetNetworkPassword();
	m_bDrawRTF = GetDrawRTF();
	m_bMultiPasteReverse = GetMultiPasteReverse();
	m_csPlaySoundOnCopy = GetPlaySoundOnCopy();
	m_bSendPasteMessageAfterSelection = GetSendPasteAfterSelection();
	m_bFindAsYouType = GetFindAsYouType();
	m_bEnsureEntireWindowCanBeSeen = GetEnsureEntireWindowCanBeSeen();
	m_bShowAllClipsInMainList = GetShowAllClipsInMainList();
	m_lMaxClipSizeInBytes = GetMaxClipSizeInBytes();
	m_dwSaveClipDelay = GetSaveClipDelay();
	m_lProcessDrawClipboardDelay = GetProcessDrawClipboardDelay();
	m_bEnableDebugLogging = GetEnableDebugLogging();
	m_bEnsureConnectToClipboard = GetEnsureConnectToClipboard();
	m_bOutputDebugString = false;

	GetExtraNetworkPassword(true);

	for(int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		GetSendClients(i);
	}

	GetClientSendCount();


	//If running from a U3 device and no language file as been asigned
	//then use the language defined by the U3 launcher
	if(m_bU3)
	{
		CString csLanguage = GetLanguageFile();
		if(csLanguage.IsEmpty())
		{
			CString csLanguage = GETENV(_T("U3_ENV_LANGUAGE"));
			long lLanguage = ATOI(csLanguage);
			csLanguage.Empty();
			switch(lLanguage)
			{
			case 1036: //French
				csLanguage = _T("Français");
				break;

			case 1040: //Italian
				csLanguage = _T("Italiano");
				break;

			case 1031: //German
				csLanguage = _T("German");
				break;

			case 3082: //German
				csLanguage = _T("Español");
				break;
			}

			if(csLanguage.IsEmpty() == FALSE)
			{
				SetLanguageFile(csLanguage);
			}
		}
	}

	m_Theme.Load(GetTheme());
}

void CGetSetOptions::ConverSettingsToIni()
{
	m_bInConversion = true;

	CSize sz;
	CPoint pt;

	GetQuickPasteSize(sz);
	SetQuickPasteSize(sz);
	
	GetQuickPastePoint(pt);
	SetQuickPastePoint(pt);

	GetEditWndSize(sz);
	SetEditWndSize(sz);

	GetEditWndPoint(pt);
	SetEditWndPoint(pt);

	SetShowIconInSysTray(GetShowIconInSysTray());
	SetRunOnStartUp(GetRunOnStartUp());
	SetEnableTransparency(GetEnableTransparency());
	SetTransparencyPercent(GetTransparencyPercent());
	SetLinesPerRow(GetLinesPerRow());
	SetQuickPastePosition(GetQuickPastePosition());
	SetCopyGap(GetCopyGap());
	SetDBPath(GetDBPath());
	SetCheckForMaxEntries(GetCheckForMaxEntries());
	SetCheckForExpiredEntries(GetCheckForExpiredEntries());
	SetMaxEntries(GetMaxEntries());
	SetExpiredEntries(GetExpiredEntries());
	SetTripCopyCount(GetTripCopyCount());
	SetTripPasteCount(GetTripPasteCount());
	SetTripDate(GetTripDate());
	SetTotalCopyCount(GetTotalCopyCount());
	SetTotalPasteCount(GetTotalPasteCount());
	SetTotalDate(GetTotalDate());
	SetUpdateFilePath(GetUpdateFilePath());
	SetUpdateInstallPath(GetUpdateInstallPath());	
	SetLastUpdate(GetLastUpdate());
	SetCheckForUpdates(GetCheckForUpdates());
	SetUseCtrlNumForFirstTenHotKeys(GetUseCtrlNumForFirstTenHotKeys());
	SetAllowDuplicates(GetAllowDuplicates());
	SetUpdateTimeOnPaste(GetUpdateTimeOnPaste());
	SetSaveMultiPaste(GetSaveMultiPaste());
	SetShowPersistent(GetShowPersistent());
	SetHistoryStartTop(GetHistoryStartTop());
	SetShowTextForFirstTenHotKeys(GetShowTextForFirstTenHotKeys());
	SetMainHWND(GetMainHWND());
	SetCaptionPos(GetCaptionPos());
	SetAutoHide(GetAutoHide());
	SetDescTextSize(GetDescTextSize());
	SetDescShowLeadingWhiteSpace(GetDescShowLeadingWhiteSpace());
	SetAllwaysShowDescription(GetAllwaysShowDescription());
	SetDoubleClickingOnCaptionDoes(GetDoubleClickingOnCaptionDoes());
	SetPrompForNewGroupName(GetPrompForNewGroupName());
	SetSendPasteOnFirstTenHotKeys(GetSendPasteOnFirstTenHotKeys());

	for(int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		GetSendClients(i);
		SetSendClients(m_SendClients[i], i);
	}

	SetListToPutOnClipboard(GetListToPutOnClipboard());
	SetLogSendReceiveErrors(GetLogSendReceiveErrors());
	SetHideDittoOnHotKeyIfAlreadyShown(GetHideDittoOnHotKeyIfAlreadyShown());
	SetPort(GetPort());
	SetDisableRecieve(GetDisableRecieve());

	LOGFONT font;
	GetFont(font);
	SetFont(font);

	SetDrawThumbnail(GetDrawThumbnail());	

	CStringA PassA = GetNetworkPassword();
	CString PassW = PassA;
	SetNetworkPassword(PassW);

	SetDrawRTF(GetDrawRTF());
	SetMultiPasteReverse(GetMultiPasteReverse());
	SetPlaySoundOnCopy(GetPlaySoundOnCopy());
	SetSendPasteAfterSelection(GetSendPasteAfterSelection());
	SetFindAsYouType(GetFindAsYouType());
	SetEnsureEntireWindowCanBeSeen(GetEnsureEntireWindowCanBeSeen());
	SetShowAllClipsInMainList(GetShowAllClipsInMainList());
	SetExtraNetworkPassword(GetExtraNetworkPassword(false));
	SetMaxClipSizeInBytes(GetMaxClipSizeInBytes());
	SetLanguageFile(GetLanguageFile());
	SetSaveClipDelay(GetSaveClipDelay());
	SetProcessDrawClipboardDelay(GetProcessDrawClipboardDelay());
	SetEnableDebugLogging(GetEnableDebugLogging());
	SetEnsureConnectToClipboard(GetEnsureConnectToClipboard());
	SetPromptWhenDeletingClips(GetPromptWhenDeletingClips());
	SetLastImportDir(GetLastImportDir());
	SetLastExportDir(GetLastExportDir());
	SetUpdateDescWhenSavingClip(GetUpdateDescWhenSavingClip());

	m_bInConversion = false;
}

CString CGetSetOptions::GetIniFileName(bool bLocalIniFile)
{
	CString csPath = _T("c:\\program files\\Ditto\\");

	if(m_bU3)
	{
		csPath = CGetSetOptions::GetPath(PATH_INI);
	}
	else
	{	
		if(bLocalIniFile)
		{
			csPath = GetFilePath(GetExeFileName());
		}
		else
		{
			csPath = GetAppDataPath();
		}
	}

	csPath += "Ditto.Settings";

	return csPath;
}

CString CGetSetOptions::GetAppDataPath()
{
	CString csPath;
	LPMALLOC pMalloc;

	if(SUCCEEDED(::SHGetMalloc(&pMalloc))) 
	{ 
		LPITEMIDLIST pidlPrograms;

		SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidlPrograms);

		TCHAR string[MAX_PATH];
		SHGetPathFromIDList(pidlPrograms, string);

		pMalloc->Free(pidlPrograms);
		pMalloc->Release();

		csPath = string;		
	}
	FIX_CSTRING_PATH(csPath);
	csPath += "Ditto\\";

	return csPath;
}

long CGetSetOptions::GetProfileLong(CString csName, long lDefaultValue, CString csNewPath)
{
	if(m_bFromIni && !m_bInConversion)
	{
		CString csApp(_T("Ditto"));

		if(csNewPath.IsEmpty() == FALSE)
		{
			csApp = csNewPath;
		}

		return GetPrivateProfileInt(csApp, csName, lDefaultValue, m_csIniFileName);
	}

	CString csPath(_T(REG_PATH));
	if(csNewPath.IsEmpty() == FALSE)
	{
		csPath += "\\" + csNewPath;
	}

	HKEY hkKey;

	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, csPath, NULL, KEY_READ, &hkKey);

	if(lResult != ERROR_SUCCESS)
		return lDefaultValue;

	DWORD buffer;
	DWORD len =  sizeof(buffer);
	DWORD type;

	lResult = ::RegQueryValueEx(hkKey, csName, 0, &type, (LPBYTE)&buffer, &len);

	RegCloseKey(hkKey);

	if(lResult == ERROR_SUCCESS)
		return (long)buffer;

	return lDefaultValue;
}

CString CGetSetOptions::GetProfileString(CString csName, CString csDefault, CString csNewPath)
{
	if(m_bFromIni && !m_bInConversion)
	{
		CString csApp(_T("Ditto"));

		if(csNewPath.IsEmpty() == FALSE)
		{
			csApp = csNewPath;
		}

		TCHAR cString[MAX_PATH];
		GetPrivateProfileString(csApp, csName, csDefault, cString, sizeof(cString), m_csIniFileName);

		return cString;
	}

	CString csPath(_T(REG_PATH));
	if(csNewPath.IsEmpty() == FALSE)
	{
		csPath += "\\" + csNewPath;
	}

	HKEY hkKey;
	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, csPath, NULL, KEY_READ, &hkKey);

	TCHAR szString[256];
	ZeroMemory(szString, sizeof(szString));
	DWORD dwBufLen = 256;

	lResult = ::RegQueryValueEx(hkKey , csName, NULL, NULL, (LPBYTE)szString, &dwBufLen);

	RegCloseKey(hkKey);

	if(lResult != ERROR_SUCCESS)
		return csDefault;

	return szString;
}

BOOL CGetSetOptions::SetProfileLong(CString csName, long lValue)
{
	if(m_bFromIni)
	{
		return WritePrivateProfileInt(_T("Ditto"), csName, lValue, m_csIniFileName);
	}

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
	if(m_bFromIni)
	{
		return WritePrivateProfileString(_T("Ditto"), csName, csValue, m_csIniFileName);
	}

	HKEY hkKey;
	DWORD dWord;
	long lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T(REG_PATH), NULL, 
		NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hkKey, &dWord);

	if(lResult != ERROR_SUCCESS)
		return FALSE;

	::RegSetValueEx(hkKey, csName, NULL, REG_SZ,
		(BYTE*)(LPCTSTR)csValue, csValue.GetLength()*sizeof(TCHAR));

	RegCloseKey(hkKey);

	return lResult == ERROR_SUCCESS;
}

BOOL CGetSetOptions::SetProfileData(CString csName, LPVOID lpData, DWORD dwLength)
{
	if(m_bFromIni)
	{
		ASSERT(!"SetProfileData not supported in .ini settings");
		return FALSE;
	}

	HKEY hkKey;
	DWORD dWord;
	long lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T(REG_PATH), NULL, 
		NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
		NULL, &hkKey, &dWord);

	if(lResult != ERROR_SUCCESS)
		return FALSE;

	::RegSetValueEx(hkKey, csName, NULL, REG_BINARY,
		(BYTE*)lpData, dwLength);

	RegCloseKey(hkKey);

	return lResult == ERROR_SUCCESS;
}

BOOL CGetSetOptions::GetProfileFont(CString csSection, LOGFONT &font)
{
	font.lfHeight = GetPrivateProfileInt(csSection, _T("Height"), 0, m_csIniFileName);
	font.lfWidth = GetPrivateProfileInt(csSection, _T("Width"), 0, m_csIniFileName);
	font.lfEscapement = GetPrivateProfileInt(csSection, _T("Escapement"), 0, m_csIniFileName);
	font.lfOrientation = GetPrivateProfileInt(csSection, _T("Orientation"), 0, m_csIniFileName);
	font.lfWeight = GetPrivateProfileInt(csSection, _T("Weight"), 0, m_csIniFileName);
	font.lfItalic = GetPrivateProfileInt(csSection, _T("Italic"), 0, m_csIniFileName);
	font.lfUnderline = GetPrivateProfileInt(csSection, _T("Underline"), 0, m_csIniFileName);
	font.lfStrikeOut = GetPrivateProfileInt(csSection, _T("StrikeOut"), 0, m_csIniFileName);
	font.lfCharSet = GetPrivateProfileInt(csSection, _T("CharSet"), 0, m_csIniFileName);
	font.lfOutPrecision = GetPrivateProfileInt(csSection, _T("OutPrecision"), 0, m_csIniFileName);
	font.lfClipPrecision = GetPrivateProfileInt(csSection, _T("ClipPrecision"), 0, m_csIniFileName);
	font.lfQuality = GetPrivateProfileInt(csSection, _T("Quality"), 0, m_csIniFileName);
	font.lfPitchAndFamily = GetPrivateProfileInt(csSection, _T("PitchAndFamily"), 0, m_csIniFileName);
	GetPrivateProfileString(csSection, _T("FaceName"), _T(""), font.lfFaceName, sizeof(font.lfFaceName), m_csIniFileName);

	return TRUE;
}

BOOL CGetSetOptions::SetProfileFont(CString csSection, LOGFONT &font)
{
	WritePrivateProfileInt(csSection, _T("Height"), font.lfHeight, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("Width"), font.lfWidth, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("Escapement"), font.lfEscapement, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("Orientation"), font.lfOrientation, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("Weight"), font.lfWeight, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("Italic"), font.lfItalic, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("Underline"), font.lfUnderline, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("StrikeOut"), font.lfStrikeOut, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("CharSet"), font.lfCharSet, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("OutPrecision"), font.lfOutPrecision, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("ClipPrecision"), font.lfClipPrecision, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("Quality"), font.lfQuality, m_csIniFileName);
	WritePrivateProfileInt(csSection, _T("PitchAndFamily"), font.lfPitchAndFamily, m_csIniFileName);
	WritePrivateProfileString(csSection, _T("FaceName"), font.lfFaceName, m_csIniFileName);

	return TRUE;
}

LPVOID CGetSetOptions::GetProfileData(CString csName, DWORD &dwLength)
{
	if(m_bFromIni && !m_bInConversion)
	{
		ASSERT(!"GetProfileData not supported in .ini settings");
		return NULL;
	}

	HKEY hkKey;

	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T(REG_PATH),
		NULL, KEY_READ, &hkKey);

	lResult = ::RegQueryValueEx(hkKey , csName, NULL, NULL, NULL, &dwLength);

	if(lResult != ERROR_SUCCESS)
		return NULL;

	LPVOID lpVoid = new BYTE[dwLength];

	lResult = ::RegQueryValueEx(hkKey , csName, NULL, NULL, (LPBYTE)lpVoid, &dwLength);

	RegCloseKey(hkKey);

	if(lResult != ERROR_SUCCESS)
		return NULL;

	return lpVoid;
}

BOOL CGetSetOptions::GetShowIconInSysTray() 
{
	return GetProfileLong("ShowIconInSystemTray", TRUE);
}

BOOL CGetSetOptions::SetShowIconInSysTray(BOOL bShow) 
{
	return SetProfileLong("ShowIconInSystemTray", bShow);
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
	m_nLinesPerRow = lLines;
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
	//Can't set auto run when running from U3 device
	if(m_bU3)
		return;

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
			(BYTE*)(LPCTSTR)sExeName, sExeName.GetLength()*sizeof(TCHAR));
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
	if(size.cx <= 0 && size.cy <= 0)
	{
		size.cx = 300;
		size.cy = 300;
	}
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

	if(point.x <= 0 && point.y <= 0)
	{
		point.x = 300;
		point.y = 300;
	}
}

BOOL CGetSetOptions::SetEditWndSize(CSize size)
{
	BOOL bRet = SetProfileLong("EditWndCX", size.cx);
	bRet = SetProfileLong("EditWndCY", size.cy);

	return bRet;
}

void CGetSetOptions::GetEditWndSize(CSize &size)
{
	size.cx = GetProfileLong("EditWndCX", 600);
	size.cy = GetProfileLong("EditWndCY", 600);
	if(size.cx <= 0 && size.cy <= 0)
	{
		size.cx = 600;
		size.cy = 600;
	}
}

BOOL CGetSetOptions::SetEditWndPoint(CPoint point)
{
	BOOL bRet = SetProfileLong("EditWndX", point.x);
	bRet = SetProfileLong("EditWndY", point.y);

	return bRet;
}

void CGetSetOptions::GetEditWndPoint(CPoint &point)
{
	point.x = GetProfileLong("EditWndX", 100);
	point.y = GetProfileLong("EditWndY", 100);

	if(point.x <= 0 && point.y <= 0)
	{
		point.x = 100;
		point.y = 100;
	}
}

long CGetSetOptions::GetCopyGap()
{
	return GetProfileLong("CopyGap", 150);
}

void CGetSetOptions::SetCopyGap(long lGap)
{
	SetProfileLong("CopyGap", lGap);
}

BOOL CGetSetOptions::SetDBPathOld(CString csPath)
{
	return SetProfileString("DBPath", csPath);
}

CString CGetSetOptions::GetDBPathOld()
{
	return GetProfileString("DBPath", "");
}

BOOL CGetSetOptions::SetDBPath(CString csPath)
{
	return SetProfileString("DBPath3", csPath);
}

CString CGetSetOptions::GetDBPath()
{
	CString csDBPath;
	if(m_bU3)
	{
		csDBPath = GetProfileString("DBPath3", "");
		if(csDBPath.IsEmpty())
		{
			csDBPath = GetDefaultDBName();
		}

		CPath ExistingPath(csDBPath);
		csDBPath = CGetSetOptions::GetPath(PATH_DATABASE);
		csDBPath += ExistingPath.GetName();
	}
	else
	{
		csDBPath = GetProfileString("DBPath3", "");
	}

	return csDBPath;
}

void CGetSetOptions::SetCheckForMaxEntries(BOOL bVal)
{
	SetProfileLong("CheckForMaxEntries", bVal);
}

BOOL CGetSetOptions::GetCheckForMaxEntries()
{
	BOOL bDefault = FALSE;
	if(m_bU3)
		bDefault = TRUE;

	return GetProfileLong("CheckForMaxEntries", bDefault);
}

void CGetSetOptions::SetCheckForExpiredEntries(BOOL bVal)
{
	SetProfileLong("CheckForExpiredEntries", bVal);
}

BOOL CGetSetOptions::GetCheckForExpiredEntries()
{
	return GetProfileLong("CheckForExpiredEntries", FALSE);
}

void CGetSetOptions::SetMaxEntries(long lVal)
{
	SetProfileLong("MaxEntries", lVal);
}

long CGetSetOptions::GetMaxEntries()
{
	long lMax = 500;
	if(m_bU3)
		lMax = 75;
	return GetProfileLong("MaxEntries", lMax);
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
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTripCopyCount() - lVal; // add the absolute value

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
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTripPasteCount() - lVal; // add the absolute value

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
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTotalCopyCount() - lVal; // add the absolute value

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
	// negative means a relative offset
	if(lVal < 0)
		lVal = GetTotalPasteCount() - lVal; // add the absolute value

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

CString	CGetSetOptions::GetUpdateFilePath()			
{ 
	return GetProfileString("UpdateFilePath", "");	
}
BOOL CGetSetOptions::SetUpdateFilePath(CString cs)	
{ 
	return SetProfileString("UpdateFilePath", cs);	
}

CString	CGetSetOptions::GetUpdateInstallPath()			
{ 
	return GetProfileString("UpdateInstallPath", "");	
}
BOOL CGetSetOptions::SetUpdateInstallPath(CString cs)	
{ 
	return SetProfileString("UpdateInstallPath", cs);	
}

long CGetSetOptions::GetLastUpdate()			
{ 
	return GetProfileLong("LastUpdateDay", 0);		
}
long CGetSetOptions::SetLastUpdate(long lValue)	
{ 
	return SetProfileLong("LastUpdateDay", lValue);	
}

BOOL CGetSetOptions::GetCheckForUpdates()				
{ 
	return GetProfileLong("CheckForUpdates", TRUE);	
}
BOOL CGetSetOptions::SetCheckForUpdates(BOOL bCheck)	
{ 
	return SetProfileLong("CheckForUpdates", bCheck);	
}

void CGetSetOptions::SetUseCtrlNumForFirstTenHotKeys(BOOL bVal)	
{	
	SetProfileLong("UseCtrlNumForFirstTenHotKeys", bVal);	
	m_bUseCtrlNumAccel = bVal;	
}
BOOL CGetSetOptions::GetUseCtrlNumForFirstTenHotKeys()			
{	
	return GetProfileLong("UseCtrlNumForFirstTenHotKeys", 0); 
}

void CGetSetOptions::SetAllowDuplicates(BOOL bVal)	
{	
	SetProfileLong("AllowDuplicates", bVal); 
	m_bAllowDuplicates = bVal; 
}
BOOL CGetSetOptions::GetAllowDuplicates()			
{	
	return GetProfileLong("AllowDuplicates", 0); 
}

void CGetSetOptions::SetUpdateTimeOnPaste(BOOL bVal)	
{	
	SetProfileLong("UpdateTimeOnPaste", bVal); 
	m_bUpdateTimeOnPaste = bVal; 
}
BOOL CGetSetOptions::GetUpdateTimeOnPaste()			
{	
	return GetProfileLong("UpdateTimeOnPaste", TRUE); 
}

void CGetSetOptions::SetSaveMultiPaste(BOOL bVal)	
{	
	SetProfileLong("SaveMultiPaste", bVal); 
	m_bSaveMultiPaste = bVal; 
}
BOOL CGetSetOptions::GetSaveMultiPaste()			
{	
	return GetProfileLong("SaveMultiPaste", 0); 
}

void CGetSetOptions::SetShowPersistent(BOOL bVal)	
{	
	SetProfileLong("ShowPersistent", bVal); 
	m_bShowPersistent = bVal; 
}
BOOL CGetSetOptions::GetShowPersistent()			
{	
	return GetProfileLong("ShowPersistent", 0); 
}

void CGetSetOptions::SetHistoryStartTop(BOOL bVal)	
{	
	SetProfileLong("HistoryStartTop", bVal); 
	m_bHistoryStartTop = bVal; 
}
BOOL CGetSetOptions::GetHistoryStartTop()			
{	
	return GetProfileLong("HistoryStartTop", TRUE); 
}

void CGetSetOptions::SetShowTextForFirstTenHotKeys(BOOL bVal)	
{	
	SetProfileLong("ShowTextForFirstTenHotKeys", bVal);			
}
BOOL CGetSetOptions::GetShowTextForFirstTenHotKeys()			
{	
	return GetProfileLong("ShowTextForFirstTenHotKeys", TRUE);	
}

void CGetSetOptions::SetMainHWND(long lhWnd)	
{	
	SetProfileLong("MainhWnd", lhWnd);		
}
BOOL CGetSetOptions::GetMainHWND()				
{	
	return GetProfileLong("MainhWnd", 0);	
}

void CGetSetOptions::SetCaptionPos(long lPos)	
{	
	SetProfileLong("CaptionPos", lPos);					
}
long CGetSetOptions::GetCaptionPos()			
{	
	return GetProfileLong("CaptionPos", CAPTION_RIGHT);	
}

void CGetSetOptions::SetAutoHide(BOOL bAutoHide)
{	
	SetProfileLong("AutoHide", bAutoHide);					
}
BOOL CGetSetOptions::GetAutoHide()				
{	
	return GetProfileLong("AutoHide", FALSE);				
}

void CGetSetOptions::SetDescTextSize(long lSize)
{	
	SetProfileLong("DescTextSize", lSize); 
	m_bDescTextSize = lSize; 
}
long CGetSetOptions::GetDescTextSize()			
{	
	return GetProfileLong("DescTextSize", 500); 
}

void CGetSetOptions::SetDescShowLeadingWhiteSpace(BOOL bVal)
{ 
	SetProfileLong("DescShowLeadingWhiteSpace", bVal); 
	m_bDescShowLeadingWhiteSpace = bVal; 
}
BOOL CGetSetOptions::GetDescShowLeadingWhiteSpace()         
{ 
	return GetProfileLong("DescShowLeadingWhiteSpace", FALSE); 
}

void CGetSetOptions::SetAllwaysShowDescription(long bShow)	
{	
	SetProfileLong("AllwaysShowDescription", bShow); 
	m_bAllwaysShowDescription = bShow; 
}
BOOL CGetSetOptions::GetAllwaysShowDescription()			
{	
	return GetProfileLong("AllwaysShowDescription", FALSE); 
}

void CGetSetOptions::SetDoubleClickingOnCaptionDoes(long lOption)	
{	
	SetProfileLong("DoubleClickingOnCaptionDoes", lOption); 
	m_bDoubleClickingOnCaptionDoes = lOption; 
}
long CGetSetOptions::GetDoubleClickingOnCaptionDoes()				
{	
	return GetProfileLong("DoubleClickingOnCaptionDoes", TOGGLES_ALLWAYS_ON_TOP); 
}

void CGetSetOptions::SetPrompForNewGroupName(BOOL bOption)	
{	
	SetProfileLong("PrompForNewGroupName", bOption); 
	m_bPrompForNewGroupName = bOption; 
}
BOOL CGetSetOptions::GetPrompForNewGroupName()				
{	
	return GetProfileLong("PrompForNewGroupName", TRUE); 
}

void CGetSetOptions::SetSendPasteOnFirstTenHotKeys(BOOL bOption)	
{	
	SetProfileLong("SendPasteOnFirstTenHotKeys", bOption); 
	m_bSendPasteOnFirstTenHotKeys = bOption; 
}
BOOL CGetSetOptions::GetSendPasteOnFirstTenHotKeys()				
{	
	return GetProfileLong("SendPasteOnFirstTenHotKeys", TRUE); 
}

void CGetSetOptions::SetSendClients(CSendClients Client, int nPos)
{
	CString cs;

	cs.Format(_T("sendclient_ip_%d"), nPos);
	SetProfileString(cs, Client.csIP);

	cs.Format(_T("sendclient_autosend_%d"), nPos);
	SetProfileLong(cs, Client.bSendAll);

	cs.Format(_T("sendclient_description_%d"), nPos);
	SetProfileString(cs, Client.csDescription);

	Client.bShownFirstError = m_SendClients[nPos].bShownFirstError;

	m_SendClients[nPos] = Client;
}

CSendClients CGetSetOptions::GetSendClients(int nPos)
{
	CSendClients Client;

	CString cs;

	cs.Format(_T("sendclient_ip_%d"), nPos);
	Client.csIP = GetProfileString(cs, "");

	cs.Format(_T("sendclient_autosend_%d"), nPos);
	Client.bSendAll = GetProfileLong(cs, FALSE);

	cs.Format(_T("sendclient_description_%d"), nPos);
	Client.csDescription = GetProfileString(cs, "");

	m_SendClients[nPos] = Client;

	return Client;
}

void CGetSetOptions::GetClientSendCount()
{
	m_lAutoSendClientCount = 0;
	for(int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		if(m_SendClients[i].csIP.GetLength() > 0)
		{
			if(m_SendClients[i].bSendAll)
				m_lAutoSendClientCount++;
		}
	}
}

CString	CGetSetOptions::GetListToPutOnClipboard()			
{ 
	CString cs = GetProfileString("ListToPutOnClipboard", "");
	cs.MakeUpper();
	return cs;
}
BOOL CGetSetOptions::SetListToPutOnClipboard(CString cs)	
{ 
	cs.MakeUpper();
	m_csIPListToPutOnClipboard = cs;
	return SetProfileString("ListToPutOnClipboard", cs); 

}

void CGetSetOptions::SetLogSendReceiveErrors(BOOL bOption)
{
	m_bLogSendReceiveErrors = bOption;

	SetProfileLong("LogSendReceiveErrors", bOption);
}

BOOL CGetSetOptions::GetLogSendReceiveErrors()
{
	return GetProfileLong("LogSendReceiveErrors", FALSE);
}

BOOL CGetSetOptions::GetHideDittoOnHotKeyIfAlreadyShown()
{
	return GetProfileLong("HideDittoOnHotKeyIfAlreadyShown", TRUE);
}

void CGetSetOptions::SetHideDittoOnHotKeyIfAlreadyShown(BOOL bVal)
{
	m_HideDittoOnHotKeyIfAlreadyShown = bVal;

	SetProfileLong("HideDittoOnHotKeyIfAlreadyShown", bVal);
}

void CGetSetOptions::SetPort(long lPort)
{
	m_lPort = lPort;
	SetProfileLong("SendRecvPort", lPort);
}

long CGetSetOptions::GetPort()
{
	return GetProfileLong("SendRecvPort", 23443);
}

BOOL CGetSetOptions::GetDisableRecieve()
{
	BOOL bDefault = FALSE;
	if(m_bU3)
		bDefault = TRUE;

	return GetProfileLong("DisableRecieve", bDefault);
}

void CGetSetOptions::SetDisableRecieve(BOOL bVal)
{
	SetProfileLong("DisableRecieve", bVal);
}

BOOL CGetSetOptions::GetFont(LOGFONT &font)
{
	if(m_bFromIni && !m_bInConversion)
	{
		GetProfileFont("DisplayFont", font);

		//Return true if there is a font name
		//other wise load the default font below
		if(font.lfFaceName[0] != 0)
		{
			return TRUE;
		}
	}
	else
	{
		DWORD dwLength = 0;
		#ifdef _UNICODE
			LPVOID lpVoid = GetProfileData("DisplayFont3", dwLength);
		#else
			LPVOID lpVoid = GetProfileData("DisplayFont", dwLength);
		#endif
		if(lpVoid)
		{
			if(sizeof(font) == dwLength)
			{
				memcpy(&font, lpVoid, dwLength);
				delete[] lpVoid;
				lpVoid = NULL;
				return TRUE;
			}
			else
			{
				ASSERT(!"invalid font struct size");
			}

			delete[] lpVoid;
			lpVoid = NULL;
		}
	}

	ZeroMemory(&font, sizeof(font));
	font.lfHeight = -11;
	font.lfWeight = 400;
	font.lfCharSet = 1;
	STRCPY(font.lfFaceName, _T("Arial Unicode MS"));
	return TRUE;
}

void CGetSetOptions::SetFont(LOGFONT &font)
{
	if(m_bFromIni)
	{
		SetProfileFont("DisplayFont", font);
	}
	else
	{
		#ifdef _UNICODE
			CString csParam = "DisplayFont3";
		#else
			CString csParam = "DisplayFont";
		#endif

		SetProfileData(csParam, &font, sizeof(LOGFONT));
	}
}

void CGetSetOptions::SetDrawThumbnail(long bDraw)
{
	SetProfileLong("DrawThumbnail", bDraw); 
	m_bDrawThumbnail = bDraw;
}

BOOL CGetSetOptions::GetDrawThumbnail()
{
	BOOL bDrawThumbnails = TRUE;
	if(g_Opt.m_bU3)
		bDrawThumbnails = FALSE;

	return GetProfileLong("DrawThumbnail", bDrawThumbnails);
}

void CGetSetOptions::SetExtraNetworkPassword(CString csPassword)
{
	SetProfileString("NetworkExtraPassword", csPassword);
}

CString CGetSetOptions::GetExtraNetworkPassword(bool bFillArray)
{
	CString cs = GetProfileString("NetworkExtraPassword", "");

	if(bFillArray)
	{
		m_csNetworkPasswordArray.RemoveAll();

		TCHAR seps[]   = _T(",");
		TCHAR *token;

		TCHAR *pString = cs.GetBuffer(cs.GetLength());

		/* Establish string and get the first token: */
		token = STRTOK(pString, seps);
		while(token != NULL)
		{
			CString cs(token);
			cs.TrimLeft();
			cs.TrimRight();

			m_csNetworkPasswordArray.Add(cs);

			// Get next token
			token = STRTOK(NULL, seps);
		}

		cs.ReleaseBuffer();
	}

	return cs;
}

void CGetSetOptions::SetNetworkPassword(CString csPassword)
{
	CTextConvert Con;
	Con.ConvertToUTF8(csPassword, m_csPassword);

	SetProfileString("NetworkStringPassword", csPassword);
}

CStringA CGetSetOptions::GetNetworkPassword()
{
	CString cs = GetProfileString("NetworkStringPassword", "LetMeIn");

	CTextConvert Con;
	CStringA csReturn;
	Con.ConvertToUTF8(cs, csReturn);

	return csReturn;
}

void CGetSetOptions::SetDrawRTF(long bDraw)
{
//this only works under 32 bit build
#ifdef _M_IX86
	SetProfileLong("DrawRTF", bDraw); 
	m_bDrawRTF = bDraw;
#endif
}

BOOL CGetSetOptions::GetDrawRTF()
{
//this only works under 32 bit build
#ifdef _M_IX86
	return GetProfileLong("DrawRTF", FALSE);
#endif
	return FALSE;
}

void CGetSetOptions::SetMultiPasteReverse(BOOL bVal)
{
	SetProfileLong("MultiPasteReverse", bVal); 
	m_bMultiPasteReverse = bVal;
}

BOOL CGetSetOptions::GetMultiPasteReverse()
{
	return GetProfileLong("MultiPasteReverse", TRUE); 
}

void CGetSetOptions::SetPlaySoundOnCopy(CString cs)
{
	m_csPlaySoundOnCopy = cs;

	SetProfileString("PlaySoundOnCopy", cs);
}

CString CGetSetOptions::GetPlaySoundOnCopy()
{
	return GetProfileString("PlaySoundOnCopy", "");
}

void CGetSetOptions::SetSendPasteAfterSelection(BOOL bVal)
{
	m_bSendPasteMessageAfterSelection = bVal;

	SetProfileLong("SendPasteMessageAfterSelection", bVal);
}

BOOL CGetSetOptions::GetSendPasteAfterSelection()
{
	return GetProfileLong("SendPasteMessageAfterSelection", TRUE);
}

void CGetSetOptions::SetFindAsYouType(BOOL bVal)
{
	m_bFindAsYouType = bVal;
	SetProfileLong("FindAsYouType", bVal);
}

BOOL CGetSetOptions::GetFindAsYouType()
{
	return GetProfileLong("FindAsYouType", TRUE);
}

void CGetSetOptions::SetEnsureEntireWindowCanBeSeen(BOOL bVal)
{
	m_bEnsureEntireWindowCanBeSeen = bVal;
	SetProfileLong("EnsureEntireWindowCanBeSeen", bVal);
}

BOOL CGetSetOptions::GetEnsureEntireWindowCanBeSeen()
{
	return GetProfileLong("EnsureEntireWindowCanBeSeen", TRUE);
}

void CGetSetOptions::SetShowAllClipsInMainList(BOOL bVal)
{
	m_bShowAllClipsInMainList = bVal;
	SetProfileLong("ShowAllClipsInMainList", bVal);
}

BOOL CGetSetOptions::GetShowAllClipsInMainList()
{
	return GetProfileLong("ShowAllClipsInMainList", TRUE);
}

long CGetSetOptions::GetMaxClipSizeInBytes()
{
	return GetProfileLong("MaxClipSizeInBytes", 0);
}

void CGetSetOptions::SetMaxClipSizeInBytes(long lSize)
{
	m_lMaxClipSizeInBytes = lSize;
	SetProfileLong("MaxClipSizeInBytes", lSize);
}

CString CGetSetOptions::GetLanguageFile()
{
	return GetProfileString("LanguageFile", "");
}

void CGetSetOptions::SetLanguageFile(CString csLanguage)
{
	SetProfileString("LanguageFile", csLanguage);
}

ULONG CGetSetOptions::GetSaveClipDelay()
{
	return (ULONG)GetProfileLong("SaveClipDelay", 500);
}

void CGetSetOptions::SetSaveClipDelay(DWORD dwDelay)
{
	m_dwSaveClipDelay = dwDelay;
	SetProfileLong("SaveClipDelay", dwDelay);
}

long CGetSetOptions::GetProcessDrawClipboardDelay()
{
	return GetProfileLong("ProcessDrawClipboardDelay", 100);
}

void CGetSetOptions::SetProcessDrawClipboardDelay(long lDelay)
{
	m_lProcessDrawClipboardDelay = lDelay;
	SetProfileLong("ProcessDrawClipboardDelay", lDelay);
}

BOOL CGetSetOptions::GetEnableDebugLogging()
{
	return GetProfileLong("EnableDebugLogging", FALSE);
}

void CGetSetOptions::SetEnableDebugLogging(BOOL bEnable)
{
	m_bEnableDebugLogging = bEnable;
	SetProfileLong("EnableDebugLogging", bEnable);
}

BOOL CGetSetOptions::GetEnsureConnectToClipboard()
{
	return GetProfileLong("EnsureConnected", TRUE);
}

void CGetSetOptions::SetEnsureConnectToClipboard(BOOL bSet)
{
	m_bEnsureConnectToClipboard = bSet;
	SetProfileLong("EnsureConnected", bSet);
}

BOOL CGetSetOptions::GetPromptWhenDeletingClips()
{
	return GetProfileLong("PromptWhenDeletingClips", TRUE);
}

void CGetSetOptions::SetPromptWhenDeletingClips(BOOL bSet)
{
	SetProfileLong("PromptWhenDeletingClips", bSet);
}

CString CGetSetOptions::GetLastImportDir()
{
	return GetProfileString("LastImportDir", "");
}

void CGetSetOptions::SetLastImportDir(CString csDir)
{
	SetProfileString("LastImportDir", csDir);
}

CString CGetSetOptions::GetLastExportDir()
{
	return GetProfileString("LastExportDir", "");
}

void CGetSetOptions::SetLastExportDir(CString csDir)
{
	SetProfileString("LastExportDir", csDir);
}

BOOL CGetSetOptions::GetUpdateDescWhenSavingClip()
{
	return GetProfileLong("UpdateDescWhenSaving", TRUE);
}

void CGetSetOptions::SetUpdateDescWhenSavingClip(BOOL bSet)
{
	SetProfileLong("UpdateDescWhenSaving", bSet);
}

CString CGetSetOptions::GetPath(long lPathID)
{
	CString csDir = CGetSetOptions::GetExeFileName();
	csDir = GetFilePath(csDir);
	FIX_CSTRING_PATH(csDir);

	//U3_APP_DATA_PATH    -	<U3_DEVICE_PATH>\System\Apps\{app_unique_id}\Data
	//U3_HOST_EXEC_PATH	  - %APPDATA%\U3\{device_serial_number}\{app_unique_id}\Exec
	//U3_DEVICE_EXEC_PATH -	<U3_DEVICE_PATH>\System\Apps\{app_unique_id}\Exec

	switch(lPathID)
	{
	case PATH_HELP:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_DEVICE_EXEC_PATH"));
			FIX_CSTRING_PATH(csDir);
		}

		csDir += "Help\\";
		break;
	
	case PATH_LANGUAGE:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_DEVICE_EXEC_PATH"));
			FIX_CSTRING_PATH(csDir);
		}

		csDir += "language\\";

		break;

	case PATH_THEMES:

		if(m_bU3)
		{
			csDir = GETENV(_T("U3_DEVICE_EXEC_PATH"));
			FIX_CSTRING_PATH(csDir);
		}

		csDir += "Themes\\";

		break;

	case PATH_REMOTE_FILES:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_HOST_EXEC_PATH"));
			FIX_CSTRING_PATH(csDir);
		}
		else if(CGetSetOptions::GetIsPortableDitto() == false)
		{
			csDir = GetAppDataPath();
		}

		csDir += "ReceivedFiles\\";
		break;

	case PATH_LOG_FILE:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_HOST_EXEC_PATH"));
			FIX_CSTRING_PATH(csDir);
		}
		else if(CGetSetOptions::GetIsPortableDitto() == false)
		{
			csDir = GetAppDataPath();
		}

		break;

	case PATH_UPDATE_FILE:
		//same path as the executable
		break;

	case PATH_DATABASE:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_APP_DATA_PATH"));
			FIX_CSTRING_PATH(csDir);
		}
		break;


	case PATH_INI:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_APP_DATA_PATH"));
			FIX_CSTRING_PATH(csDir);
		}
		break;

	case PATH_U3_HWND_INI:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_HOST_EXEC_PATH"));
			FIX_CSTRING_PATH(csDir);
		}
		break;

	case PATH_ADDINS:
		if(m_bU3)
		{
			csDir = GETENV(_T("U3_DEVICE_EXEC_PATH"));
			FIX_CSTRING_PATH(csDir);
		}

		csDir += "Addins\\";
		
		break;
	}

	CreateDirectory(csDir, NULL);

	return csDir;
}

void CGetSetOptions::WriteU3Hwnd(HWND hWnd)
{
	CString csIniFile = GetPath(PATH_U3_HWND_INI);
	csIniFile += _T("DittohWnd.ini");
	WritePrivateProfileInt(_T("Ditto"), _T("MainhWnd"), (int)hWnd, csIniFile);
}

long CGetSetOptions::GetDittoRestoreClipboardDelay()
{
	return GetProfileLong("RestoreClipboardDelay", 750);

}
void CGetSetOptions::SetDittoRestoreClipboardDelay(long lDelay)
{
	SetProfileLong("RestoreClipboardDelay", lDelay);
}

void CGetSetOptions::GetCopyBufferItem(int nPos, CCopyBufferItem &Item)
{
	Item.m_lCopyHotKey = GetProfileLong(StrF(_T("CopyBufferCopyHotKey_%d"), nPos), 0);
	Item.m_lPasteHotKey = GetProfileLong(StrF(_T("CopyBufferPasteHotKey_%d"), nPos), 0);
	Item.m_lCutHotKey = GetProfileLong(StrF(_T("CopyBufferCutHotKey_%d"), nPos), 0);
	Item.m_bPlaySoundOnCopy = GetProfileLong(StrF(_T("CopyBufferPlaySound_%d"), nPos), 0);
}

void CGetSetOptions::SetCopyBufferItem(int nPos, CCopyBufferItem &Item)
{
	SetProfileLong(StrF(_T("CopyBufferCopyHotKey_%d"), nPos), Item.m_lCopyHotKey);
	SetProfileLong(StrF(_T("CopyBufferPasteHotKey_%d"), nPos), Item.m_lPasteHotKey);
	SetProfileLong(StrF(_T("CopyBufferCutHotKey_%d"), nPos), Item.m_lCutHotKey);
	SetProfileLong(StrF(_T("CopyBufferPlaySound_%d"), nPos), Item.m_bPlaySoundOnCopy);
}

CString CGetSetOptions::GetMultiPasteSeparator(bool bConvertToLineFeeds)
{
	CString csSep = GetProfileString(_T("MultiPasteSeparator"), _T("[LF]"));
	if(bConvertToLineFeeds)
	{
		CString csLineFeed(_T("\r\n"));
		csSep.Replace(_T("[LF]"), csLineFeed);
		csSep.Replace(_T("[lf]"), csLineFeed);
	}

	return csSep;
}

void CGetSetOptions::SetMultiPasteSeparator(CString csSep)
{
	SetProfileString(_T("MultiPasteSeparator"), csSep);
}

BOOL CGetSetOptions::GetSetCurrentDirectory()
{
	BOOL bRet = GetProfileLong(_T("SetCurrentDirectory"), FALSE);
	if(bRet)
	{
		CString csExePath = GetFilePath(GetExeFileName());
		FIX_CSTRING_PATH(csExePath);
		::SetCurrentDirectory(csExePath);
	}

	return bRet;
}

bool CGetSetOptions::GetIsPortableDitto()
{
	return GetProfileLong(_T("Portable"), FALSE) == TRUE;
}

CString CGetSetOptions::GetPasteString(CString csAppName)
{
	CString csString = GetProfileString(csAppName, _T(""), _T("PasteStrings"));
	if(csString.IsEmpty())
		return GetDefaultPasteString();

	return csString;
}

CString CGetSetOptions::GetDefaultPasteString()
{
	return GetProfileString(_T("DefaultPasteString"), _T("^v"));
}

CString CGetSetOptions::GetCopyString(CString csAppName)
{
	CString csString = GetProfileString(csAppName, _T(""), _T("CopyStrings"));
	if(csString.IsEmpty())
		return GetDefaultCopyString();

	return csString;
}

CString CGetSetOptions::GetDefaultCopyString()
{
	return GetProfileString(_T("DefaultCopyString"), _T("^c"));
}

CString CGetSetOptions::GetCutString(CString csAppName)
{
	CString csString = GetProfileString(csAppName, _T(""), _T("CutStrings"));
	if(csString.IsEmpty())
		return GetDefaultCutString();

	return csString;
}

CString CGetSetOptions::GetDefaultCutString()
{
	return GetProfileString(_T("DefaultCutString"), _T("^x"));
}

BOOL CGetSetOptions::GetEditWordWrap()
{
	return GetProfileLong(_T("EditWordWrap"), TRUE);
}

void CGetSetOptions::SetEditWordWrap(BOOL bSet)
{
	SetProfileLong(_T("EditWordWrap"), bSet);
}

bool CGetSetOptions::GetAllowFriends()
{
	return (GetProfileLong("AllowFriends", TRUE) == TRUE);
}

long CGetSetOptions::GetAutoMaxDelay()
{
	return GetProfileLong(_T("AutoMaxDelaySeconds"), 2);
}

void CGetSetOptions::SetAutoMaxDelay(long lDelay)
{
	SetProfileLong(_T("AutoMaxDelaySeconds"), lDelay);
}

void CGetSetOptions::SetTheme(CString csTheme)
{
	SetProfileString(_T("Theme"), csTheme);
}

CString CGetSetOptions::GetTheme()
{
	return GetProfileString(_T("Theme"), "");
}

long CGetSetOptions::GetKeyStateWaitTimerCount()
{
	return GetProfileLong(_T("KeyStateWaitTimerCount"), 20);
}

long CGetSetOptions::GetKeyStatePasteDelay()
{
	return GetProfileLong(_T("KeyStatePasteDelay"), 200);
}

DWORD CGetSetOptions::GetDittoHotKey()
{
	return (DWORD)GetProfileLong(_T("DittoHotKey"), 704);
}

DWORD CGetSetOptions::SendKeysDelay()
{
	return (DWORD)GetProfileLong(_T("SendKeysDelay"), 50);
}

DWORD CGetSetOptions::WaitForActiveWndTimeout()
{
	return (DWORD)GetProfileLong(_T("WaitForActiveWndTimeout"), 100);
}

DWORD CGetSetOptions::FocusChangedDelay()
{
	return (DWORD)GetProfileLong(_T("FocusChangedDelay"), 100);
}

DWORD CGetSetOptions::FocusWndTimerTimeout()
{
	return (DWORD)GetProfileLong(_T("FocusWndTimerTimeout"), 2000);
}

BOOL CGetSetOptions::GetConnectedToClipboard()
{
	return GetProfileLong("ConnectedToClipboard", TRUE);
}

void CGetSetOptions::SetConnectedToClipboard(BOOL val)
{
	SetProfileLong("ConnectedToClipboard", val);
}

DWORD CGetSetOptions::GetTextOnlyRestoreDelay()
{
	return (DWORD)GetProfileLong(_T("TextOnlyRestoreDelay"), 2000);
}

DWORD CGetSetOptions::GetTextOnlyPasteDelay()
{
	return (DWORD)GetProfileLong(_T("TextOnlyPasteDelay"), 500);
}

BOOL CGetSetOptions::GetSetFocusToApp(CString csAppName)
{
	CString focusCheck;
	focusCheck = "SetFocus_";
	focusCheck += csAppName;
	return GetProfileLong(focusCheck, FALSE);
}

DWORD CGetSetOptions::SelectedIndex()
{
	return (DWORD)GetProfileLong(_T("SelectedIndex"), 0);
}

void CGetSetOptions::SetCopyAppInclude(CString csAppName)
{
	SetProfileString(_T("CopyAppInclude"), csAppName);
}

CString CGetSetOptions::GetCopyAppInclude()
{
	CString includeApp = GetProfileString(_T("CopyAppInclude"), "*");
	if(includeApp == "")
	{
		includeApp = "*";
	}

	return includeApp;
}

void CGetSetOptions::SetCopyAppExclude(CString csAppName)
{
	SetProfileString(_T("CopyAppExclude"), csAppName);
}

CString CGetSetOptions::GetCopyAppExclude()
{
	return GetProfileString(_T("CopyAppExclude"), "");
}

CString CGetSetOptions::GetCopyAppSeparator()
{
	return GetProfileString(_T("CopyAppSeparator"), ";");
}

DWORD CGetSetOptions::GetNoFormatsRetryDelay()
{
	return GetProfileLong(_T("NoFormatsRetryDelay"), 200);
}

DWORD CGetSetOptions::GetMainDeletesDeleteCount()
{
	return GetProfileLong(_T("MainDeletesDeleteCount"), 5);
}