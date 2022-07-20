#include "stdafx.h"
#include "Options.h"
#include "AlphaBlend.h"
#include "Misc.h"
#include "shared/TextConvert.h"
#include "sqlite\CppSQLite3.h"
#include "Path.h"
#include "CP_Main.h"
#include "ActionEnums.h"

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
BOOL CGetSetOptions::m_outputDebugStringLogging;
bool CGetSetOptions::m_bInConversion = false;
bool CGetSetOptions::m_bFromIni = false;
bool CGetSetOptions::m_portable = false;
bool CGetSetOptions::m_windowsApp = false;
bool CGetSetOptions::m_chocolateyApp = false;
CString CGetSetOptions::m_csIniFileName;
__int64 CGetSetOptions::nLastDbWriteTime = 0;
CTheme CGetSetOptions::m_Theme;
BOOL CGetSetOptions::m_showScrollBar = false;
CGetSetOptions g_Opt;
BOOL CGetSetOptions::m_bShowAlwaysOnTopWarning = TRUE;
CRegExFilterHelper CGetSetOptions::m_regexHelper;
BOOL CGetSetOptions::m_excludeCF_DIBInExcel = TRUE;
CChaiScriptXml CGetSetOptions::m_copyScripts;
CChaiScriptXml CGetSetOptions::m_pasteScripts;
long CGetSetOptions::m_tooltipTimeout;
BOOL CGetSetOptions::m_cleanRTFBeforeDrawing = TRUE;
int CGetSetOptions::m_doubleKeyStrokeTimeout = 350;
int CGetSetOptions::m_firstTenHotKeysStart = 1;
int CGetSetOptions::m_firstTenHotKeysFontSize = 5;
BOOL CGetSetOptions::m_moveSelectionOnOpenHotkey = TRUE;
BOOL CGetSetOptions::m_allowBackToBackDuplicates = FALSE;
BOOL CGetSetOptions::m_maintainSearchView = FALSE;
CString CGetSetOptions::m_tempDragFileName = "";
CTime CGetSetOptions::m_tempDragFileNameSetTime;
BOOL CGetSetOptions::m_refreshViewAfterPasting = FALSE;


CGetSetOptions::CGetSetOptions()
{
}

CGetSetOptions::~CGetSetOptions()
{

}

void CGetSetOptions::LoadSettings()
{
	CString exeDir = CGetSetOptions::GetExeFileName();
	exeDir = GetFilePath(exeDir);
	FIX_CSTRING_PATH(exeDir);

	CString windowsAppFile = exeDir + _T("WindowsApp");
	CString chocolateyAppFile = exeDir + _T("chocolatey");
	if (FileExists(windowsAppFile))
	{
		m_windowsApp = true;
		m_bFromIni = true;
		//always use the ini file in the app data folder for windows store
		m_csIniFileName = GetIniFileName(false);
	}
	else if (FileExists(chocolateyAppFile))
	{
		m_chocolateyApp = true;
		m_bFromIni = true;
		//always use the ini file in the app data folder for chocolatey portable
		m_csIniFileName = GetIniFileName(false);
	}
	else
	{
		m_csIniFileName = GetIniFileName(true);

		CString portable = GetFilePath(m_csIniFileName);
		portable += _T("portable");
		if (FileExists(portable))
		{
			m_portable = true;
		}

		//first check if ini file is in app directory
		if (m_portable || FileExists(m_csIniFileName))
		{
			m_bFromIni = true;
		}
		else
		{
			//next check if it's in app data
			m_csIniFileName = GetIniFileName(false);
			if (FileExists(m_csIniFileName))
			{
				m_bFromIni = true;
			}
		}
	}

	if(m_bFromIni)
	{
		CString csPath = GetFilePath(m_csIniFileName);
		if(FileExists(csPath) == FALSE)
			CreateDirectory(csPath, NULL);

		//create the ini file as unicode, this way we can save unicode string to the ini file
		//http://www.codeproject.com/Articles/9071/Using-Unicode-in-INI-files
		CreateIniFile(m_csIniFileName);
	}

	/*CString cs = GetDBPath();
	SetDBPath(_T("some path"));
	CString cs2 = GetDBPath();*/


	GetSetCurrentDirectory();

	//First time we run, set some defaults
	if (GetDBPath() == _T("") &&
		GetTotalCopyCount() == 0)
	{
		SetCheckForMaxEntries(TRUE);
		SetSimpleTextSearch(TRUE);
	}	

	m_nLinesPerRow = GetLinesPerRow();
	m_bUseCtrlNumAccel = GetUseCtrlNumForFirstTenHotKeys();
	m_bAllowDuplicates = GetAllowDuplicates();
	m_bUpdateTimeOnPaste = GetUpdateTimeOnPaste();
	m_bSaveMultiPaste = GetSaveMultiPaste();
	m_bShowPersistent = GetShowPersistent();
	m_bDescTextSize = GetDescTextSize();
	m_bDescShowLeadingWhiteSpace = GetDescShowLeadingWhiteSpace();
	m_bAllwaysShowDescription = GetAllwaysShowDescription();
	m_bDoubleClickingOnCaptionDoes = GetDoubleClickingOnCaptionDoes();
	m_bPrompForNewGroupName = GetPrompForNewGroupName();
	m_bSendPasteOnFirstTenHotKeys = GetSendPasteOnFirstTenHotKeys();
	m_csIPListToPutOnClipboard = GetListToPutOnClipboard();
	m_bLogSendReceiveErrors = GetLogSendReceiveErrors();
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
	m_outputDebugStringLogging = GetEnableOutputDebugStringLogging();
	m_bEnsureConnectToClipboard = GetEnsureConnectToClipboard();
	m_showScrollBar = GetShowScrollBar();
	m_bShowAlwaysOnTopWarning = GetShowAlwaysOnTopWarning();
	m_excludeCF_DIBInExcel = GetExcludeCF_DIBInExcel();
	m_doubleKeyStrokeTimeout = GetDoubleKeyStrokeTimeout();
	m_firstTenHotKeysStart = GetFirstTenHotKeysStart();
	m_firstTenHotKeysFontSize = GetFirstTenHotKeysFontSize();
	m_moveSelectionOnOpenHotkey = GetMoveSelectionOnOpenHotkey();
	m_allowBackToBackDuplicates = GetAllowBackToBackDuplicates();
	m_maintainSearchView = GetMaintainSearchView();

	GetExtraNetworkPassword(true);

	for(int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		GetSendClients(i);
	}

	for (int i = 0; i < MAX_REGEX_FILTERS; i++)
	{
		CRegExFilterData data;
		data.m_regEx = GetRegexFilter(i);
		data.m_processFilters = GetRegexFilterByProcessName(i);
		m_regexHelper.Add(i, data);
	}

	GetClientSendCount();

	m_Theme.Load(GetTheme());

	m_copyScripts.Load(GetCopyScriptsXml());
	if (m_copyScripts.m_assignedGuidOnLoad)
	{
		SetCopyScriptsXml(m_copyScripts.Save());
	}
	m_pasteScripts.Load(GetPasteScriptsXml());
	if (m_pasteScripts.m_assignedGuidOnLoad)
	{
		SetPasteScriptsXml(m_pasteScripts.Save());
	}

	m_tooltipTimeout = GetToolTipTimeout();
}

void CGetSetOptions::CreateIniFile(CString path)
{
	if (!::PathFileExists(path))
	{
		// UTF16-LE BOM(FFFE)
		WORD wBOM = 0xFEFF;
		DWORD NumberOfBytesWritten;

		HANDLE hFile = ::CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		::WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL);
	
		//LPTSTR pszSectionB = _T("[StringTable]"); // section name with bracket 
		//::WriteFile(hFile, pszSectionB, (_tcslen(pszSectionB) + 1)*(sizeof(TCHAR)), &NumberOfBytesWritten, NULL);

		::CloseHandle(hFile);
	}
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
	
	if(bLocalIniFile)
	{
		csPath = GetFilePath(GetExeFileName());
	}
	else
	{
		csPath = GetAppDataPath();
	}

	csPath += "Ditto.Settings";

	return csPath;
}

CString CGetSetOptions::GetAppDataPath()
{
	CString csPath;
	LPMALLOC pMalloc;

	if (GetIsWindowsApp())
	{
		if (SUCCEEDED(::SHGetMalloc(&pMalloc)))
		{
			LPITEMIDLIST pidlPrograms;

			SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidlPrograms);

			TCHAR string[MAX_PATH];
			SHGetPathFromIDList(pidlPrograms, string);

			pMalloc->Free(pidlPrograms);
			pMalloc->Release();

			csPath = string;
		}
		FIX_CSTRING_PATH(csPath);
		csPath += "Ditto_WindowsApp\\";
	}
	else if (GetIsChocolateyApp())
	{
		if (SUCCEEDED(::SHGetMalloc(&pMalloc)))
		{
			LPITEMIDLIST pidlPrograms;

			SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidlPrograms);

			TCHAR string[MAX_PATH];
			SHGetPathFromIDList(pidlPrograms, string);

			pMalloc->Free(pidlPrograms);
			pMalloc->Release();

			csPath = string;
		}
		FIX_CSTRING_PATH(csPath);
		csPath += "Ditto_ChocolateyApp\\";
	}
	else
	{
		if (SUCCEEDED(::SHGetMalloc(&pMalloc)))
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
	}

	return csPath;
}

CString CGetSetOptions::GetTempFilePath()
{
	CString csPath;

	wchar_t wchPath[MAX_PATH];
	if (GetTempPathW(MAX_PATH, wchPath))
	{
		csPath = wchPath;
		csPath += _T("Ditto\\");
		CreateDirectory(csPath, NULL);
	}

	return csPath;
}

long CGetSetOptions::GetResolutionProfileLong(CString csName, long lDefaultValue, CString csNewPath)
{
	CString resName;
	resName.Format(_T("(%dx%d)_%s"), GetScreenWidth(), GetScreenHeight(), csName);

	long value = GetProfileLong(resName, INT_MIN, csNewPath);

	if(value == INT_MIN)
	{
		value = GetProfileLong(csName, lDefaultValue, csNewPath);
	}

	return value;
}

BOOL CGetSetOptions::SetResolutionProfileLong(CString csName, long lValue)
{
	CString resName;
	resName.Format(_T("(%dx%d)_%s"), GetScreenWidth(), GetScreenHeight(), csName);

	return SetProfileLong(resName, lValue);
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

CString CGetSetOptions::GetProfileString(CString csName, CString csDefault, CString csNewPath, int maxSize)
{
	CString returnString;
	DWORD dwBufLen = 0;

	if(m_bFromIni && !m_bInConversion)
	{
		CString csApp(_T("Ditto"));

		if(csNewPath.IsEmpty() == FALSE)
		{
			csApp = csNewPath;
		}

		bool doBreak = false;
		dwBufLen = 10000;
		bool setMaxSize = false;
		while (true)
		{
			if (maxSize > -1 && maxSize < dwBufLen)
			{
				dwBufLen = maxSize;
				setMaxSize = true;
			}

			TCHAR *szString = new TCHAR[dwBufLen];
			ZeroMemory(szString, dwBufLen);

			DWORD readLength = GetPrivateProfileString(csApp, csName, csDefault, szString, dwBufLen, m_csIniFileName);

			if (setMaxSize ||
				readLength < (dwBufLen - 1))
			{
				returnString = szString;
				doBreak = true; //delay break so we can delete the string
			}

			delete[] szString;
			dwBufLen = dwBufLen * 2;

			if (doBreak)
			{
				break;
			}
		}

		return returnString;
	}

	CString csPath(_T(REG_PATH));
	if(csNewPath.IsEmpty() == FALSE)
	{
		csPath += "\\" + csNewPath;
	}

	HKEY hkKey;
	long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, csPath, NULL, KEY_READ, &hkKey);

	if (lResult == ERROR_SUCCESS)
	{
		lResult = ::RegQueryValueEx(hkKey, csName, NULL, NULL, NULL, &dwBufLen);

		if (lResult == ERROR_SUCCESS &&
			dwBufLen > 0)
		{
			if (maxSize > -1 && maxSize < dwBufLen)
			{
				dwBufLen = maxSize;
			}

			dwBufLen++;
			TCHAR *szString = new TCHAR[dwBufLen];
			ZeroMemory(szString, dwBufLen);

			lResult = ::RegQueryValueEx(hkKey, csName, NULL, NULL, (LPBYTE)szString, &dwBufLen);

			returnString = szString;
			delete[] szString;
		}

		RegCloseKey(hkKey);
	}

	if(lResult != ERROR_SUCCESS)
		return csDefault;

	return returnString;
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
	if(lPercent > OPACITY_MAX)
		lPercent = OPACITY_MAX;
	if(lPercent < 0)
		lPercent = 0;

	return SetProfileLong("TransparencyPercent", lPercent);
}

long CGetSetOptions::GetTransparencyPercent()
{
	long lValue = GetProfileLong("TransparencyPercent", 14);

	if(lValue > OPACITY_MAX) lValue = OPACITY_MAX;
	if(lValue < 0) lValue = 0;

	return lValue;
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
	return GetProfileLong("ShowQuickPastePosition", POS_AT_CARET);
}

BOOL CGetSetOptions::SetQuickPasteSize(CSize size)
{
	BOOL bRet = SetResolutionProfileLong("QuickPasteCX", size.cx);
	bRet = SetResolutionProfileLong("QuickPasteCY", size.cy);

	return bRet;
}

void CGetSetOptions::GetQuickPasteSize(CSize &size)
{	
	size.cx = GetResolutionProfileLong("QuickPasteCX", 300);
	size.cy = GetResolutionProfileLong("QuickPasteCY", 300);
	if(size.cx <= 0 && size.cy <= 0)
	{
		size.cx = 300;
		size.cy = 300;
	}
}

BOOL CGetSetOptions::SetQuickPastePoint(CPoint point)
{
	BOOL bRet = SetResolutionProfileLong("QuickPasteX", point.x);
	bRet = SetResolutionProfileLong("QuickPasteY", point.y);

	return bRet;
}

void CGetSetOptions::GetQuickPastePoint(CPoint &point)
{
	point.x = GetResolutionProfileLong("QuickPasteX", 300);
	point.y = GetResolutionProfileLong("QuickPasteY", 300);

	/*if(point.x <= 0 && point.y <= 0)
	{
		point.x = 300;
		point.y = 300;
	}*/
}

BOOL CGetSetOptions::SetEditWndSize(CSize size)
{
	BOOL bRet = SetResolutionProfileLong("EditWndCX", size.cx);
	bRet = SetResolutionProfileLong("EditWndCY", size.cy);

	return bRet;
}

void CGetSetOptions::GetEditWndSize(CSize &size)
{
	size.cx = GetResolutionProfileLong("EditWndCX", 600);
	size.cy = GetResolutionProfileLong("EditWndCY", 600);
	if(size.cx <= 0 && size.cy <= 0)
	{
		size.cx = 600;
		size.cy = 600;
	}
}

BOOL CGetSetOptions::SetEditWndPoint(CPoint point)
{
	BOOL bRet = SetResolutionProfileLong("EditWndX", point.x);
	bRet = SetResolutionProfileLong("EditWndY", point.y);

	return bRet;
}

void CGetSetOptions::GetEditWndPoint(CPoint &point)
{
	point.x = GetResolutionProfileLong("EditWndX", 100);
	point.y = GetResolutionProfileLong("EditWndY", 100);

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

CString CGetSetOptions::ResolvePath(CString path)
{
	CString dest;
	int newSize = max(path.GetLength() * 10, 1000);

	//path = _T("some string %COMPUTERNAME% test");

	ExpandEnvironmentStrings(path, dest.GetBuffer(newSize), newSize);
	dest.ReleaseBuffer();

	return dest;
}

CString CGetSetOptions::GetDBPath(bool resolvePath)
{
	CString csDBPath;
	csDBPath = GetProfileString("DBPath3", "");

	if (resolvePath)
	{
		csDBPath = ResolvePath(csDBPath);
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
	if(GetIsPortableDitto())
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
	if(GetIsPortableDitto())
		lMax = 100;
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
long CGetSetOptions::GetMainHWND()				
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
	if(GetIsPortableDitto())
		bDefault = TRUE;

	if(GetIsWindowsApp())
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
		GetProfileFont("DisplayFont6", font);

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
		LPVOID lpVoid = GetProfileData("DisplayFont6", dwLength);
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
	font.lfHeight = -13;
	font.lfWeight = 400;
	font.lfCharSet = 1;
	STRCPY(font.lfFaceName, _T("Segoe UI"));
	return TRUE;
}

void CGetSetOptions::SetFont(LOGFONT &font)
{
	if(m_bFromIni)
	{
		SetProfileFont("DisplayFont6", font);
	}
	else
	{	
		SetProfileData("DisplayFont6", &font, sizeof(LOGFONT));
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
	m_csPassword = CTextConvert::UnicodeToUTF8(csPassword);
	SetProfileString("NetworkStringPassword", csPassword);
}

CStringA CGetSetOptions::GetNetworkPassword()
{
	CString cs = GetProfileString("NetworkStringPassword", "LetMeIn");
	CString csReturn = CTextConvert::UnicodeToUTF8(cs);
	return csReturn;
}

void CGetSetOptions::SetDrawRTF(long bDraw)
{
	SetProfileLong("DrawRTF", bDraw); 
	m_bDrawRTF = bDraw;
}

BOOL CGetSetOptions::GetDrawRTF()
{
	return GetProfileLong("DrawRTF", FALSE);
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

BOOL CGetSetOptions::GetEnableOutputDebugStringLogging()
{
	return GetProfileLong("EnableOutputDebugStringLogging", FALSE);
}

void CGetSetOptions::SetEnableOutputDebugStringLogging(BOOL bEnable)
{
	m_outputDebugStringLogging = bEnable;
	SetProfileLong("EnableOutputDebugStringLogging", bEnable);
}

BOOL CGetSetOptions::GetEnsureConnectToClipboard()
{
	return GetProfileLong("EnsureConnected2", FALSE);
}

void CGetSetOptions::SetEnsureConnectToClipboard(BOOL bSet)
{
	m_bEnsureConnectToClipboard = bSet;
	SetProfileLong("EnsureConnected2", bSet);
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
		csDir += "Help\\";
		break;
	
	case PATH_LANGUAGE:
		csDir += "language\\";
		break;

	case PATH_THEMES:
		csDir += "Themes\\";
		break;

	case PATH_LOG_FILE:if(CGetSetOptions::GetIsPortableDitto() == false)
		csDir = GetAppDataPath();		

		break;

	case PATH_ADDINS:
		csDir += "Addins\\";		
		break;

	case PATH_REMOTE_FILES:
		if (CGetSetOptions::GetIsPortableDitto() == false)
		{
			csDir = GetTempFilePath();			
		}
		csDir += "ReceivedFiles\\";
		break;

	case PATH_DRAG_FILES:
		if (CGetSetOptions::GetIsPortableDitto() == false)
		{
			csDir = GetTempFilePath();
		}
		csDir += "DragFiles\\";
		break;

	case PATH_CLIP_DIFF:
		if (CGetSetOptions::GetIsPortableDitto() == false)
		{
			csDir = GetTempFilePath();
		}
		csDir += _T("ClipCompare\\");
		break;

	case PATH_RESTORE_TEMP:
		if (CGetSetOptions::GetIsPortableDitto() == false)
		{
			csDir = GetTempFilePath();
		}
		csDir += _T("RestoreDb\\");
		break;

	}

	CreateDirectory(csDir, NULL);

	return csDir;
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
	if(m_portable)
	{
		CString csExePath = GetFilePath(GetExeFileName());
		FIX_CSTRING_PATH(csExePath);
		::SetCurrentDirectory(csExePath);
	}

	return m_portable == true;
}

bool CGetSetOptions::GetIsPortableDitto()
{
	return m_portable;
}

bool CGetSetOptions::GetIsWindowsApp()
{
	return m_windowsApp;
}

bool CGetSetOptions::GetIsChocolateyApp()
{
	return m_chocolateyApp;
}

CString CGetSetOptions::GetPasteString(CString csAppName)
{
	CString csString = GetProfileString(csAppName, _T(""), _T("PasteStrings"));
	if (csString.IsEmpty())
	{
		//edge is really slow to set focus so add a delay before sending the paste
		if (csAppName == L"MicrosoftEdge.exe")
		{
			return _T("{DELAY 500}^{VKEY86}");
		}
		return GetDefaultPasteString();
	}

	return csString;
}

CString CGetSetOptions::GetDefaultPasteString()
{
	//ascii value for v
	//for other language keyboards v gets converted to something else so send the base ascii value
	return GetProfileString(_T("DefaultPasteString"), _T("^{VKEY86}"));
}

void CGetSetOptions::SetDefaultPasteString(CString val)
{
	SetProfileString(_T("DefaultPasteString"), val);
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
	//ascii value for c
	//for other language keyboards c gets converted to something else so send the base ascii value
	return GetProfileString(_T("DefaultCopyString"), _T("^{VKEY67}"));
}

void CGetSetOptions::SetDefaultCopyString(CString val)
{
	SetProfileString(_T("DefaultCopyString"), val);
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
	//ascii value for x
	//for other language keyboards x gets converted to something else so send the base ascii value
	return GetProfileString(_T("DefaultCutString"), _T("^{VKEY88}"));
}

void CGetSetOptions::SetDefaultCutString(CString val)
{
	SetProfileString(_T("DefaultCutString"), val);
}

BOOL CGetSetOptions::GetEditWordWrap()
{
	return GetProfileLong(_T("EditWordWrap"), TRUE);
}

void CGetSetOptions::SetEditWordWrap(BOOL bSet)
{
	SetProfileLong(_T("EditWordWrap"), bSet);
}

void CGetSetOptions::SetAllowFriends(BOOL val)
{
	SetProfileLong("AllowFriends", val);
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
	SetProfileString(_T("Theme2"), csTheme);
}

CString CGetSetOptions::GetTheme()
{
	return GetProfileString(_T("Theme2"), "");
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
	return (DWORD)GetProfileLong(_T("SendKeysDelay"), 100);
}

void CGetSetOptions::SetSendKeysDelay(DWORD val)
{
	SetProfileLong(_T("SendKeysDelay"), val);
}

DWORD CGetSetOptions::RealSendKeysDelay()
{
	return (DWORD)GetProfileLong(_T("RealSendKeysDelay"), 10);
}

void CGetSetOptions::SetRealSendKeysDelay(DWORD val)
{
	SetProfileLong(_T("RealSendKeysDelay"), val);
}

DWORD CGetSetOptions::WaitForActiveWndTimeout()
{
	return (DWORD)GetProfileLong(_T("WaitForActiveWndTimeout"), 500);
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
	return (DWORD)GetProfileLong(_T("TextOnlyPasteDelay"), 0);
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

void CGetSetOptions::SetSelectedIndex(int val)
{
	SetProfileLong(_T("SelectedIndex"), val);
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
	return GetProfileLong(_T("MainDeletesDeleteCount"), 100);
}

DWORD CGetSetOptions::GetIdleSecondsBeforeDelete()
{  
	return GetProfileLong(_T("IdleSecondsBeforeDelete"), 60*10);
}

DWORD CGetSetOptions::GetDbTimeout()
{  
	return GetProfileLong(_T("DbTimeout"), 5000);
}

DWORD CGetSetOptions::GetFunnyTickCountAdjustment()
{  
	return GetProfileLong(_T("FunnyTickCountAdjustment"), 300001);
}

DWORD CGetSetOptions::GetMinIdleTimeBeforeTrackFocus()
{
	return GetProfileLong(_T("MinIdleTimeBeforeTrackFocus"), 100);
}

DWORD CGetSetOptions::GetTimeBeforeExpandWindow()
{
	return GetProfileLong(_T("TimeBeforeExpandWindow"), 250);
}

DWORD CGetSetOptions::GetUseGuiThreadInfoForFocus()
{
	return GetProfileLong(_T("UseGuiThreadInfoForFocus"), 1);
}

void CGetSetOptions::SetSearchDescription(BOOL val)
{
	SetProfileLong(_T("SearchDescription"), val);
}

BOOL CGetSetOptions::GetSearchDescription()
{
	return GetProfileLong(_T("SearchDescription"), 1);
}

void CGetSetOptions::SetSearchFullText(BOOL val)
{
	SetProfileLong(_T("SearchFullText"), val);
}

BOOL CGetSetOptions::GetSearchFullText()
{
	return GetProfileLong(_T("SearchFullText"), 0);
}

void CGetSetOptions::SetSearchQuickPaste(BOOL val)
{
	SetProfileLong(_T("SearchQuickPaste"), val);
}

BOOL CGetSetOptions::GetSearchQuickPaste()
{
	return GetProfileLong(_T("SearchQuickPaste"), 0);
}

void CGetSetOptions::SetSimpleTextSearch(BOOL val)
{
	SetProfileLong(_T("SimpleTextSearch"), val);
}

BOOL CGetSetOptions::GetSimpleTextSearch()
{
	return GetProfileLong(_T("SimpleTextSearch"), 0);
}

void CGetSetOptions::SetRegExTextSearch(BOOL val)
{
	SetProfileLong(_T("RegExTextSearch"), val);
}

BOOL CGetSetOptions::GetRegExTextSearch()
{
	return GetProfileLong(_T("RegExTextSearch"), 0);
}

void CGetSetOptions::SetMoveClipsOnGlobal10(BOOL val)
{
	SetProfileLong(_T("MoveClipsOnGlobal10"), val);
}
BOOL CGetSetOptions::GetMoveClipsOnGlobal10()
{
	return GetProfileLong(_T("MoveClipsOnGlobal10"), 0);
}

void CGetSetOptions::SetShowScrollBar(BOOL val)
{
	m_showScrollBar = val;
	SetProfileLong(_T("ShowScrollBar"), val);
}

BOOL CGetSetOptions::GetShowScrollBar()
{
	return GetProfileLong(_T("ShowScrollBar"), 0);
}

void CGetSetOptions::SetPasteAsAdmin(BOOL val)
{
	SetProfileLong(_T("PasteAsAdmin"), val);
}

BOOL CGetSetOptions::GetPasteAsAdmin()
{
	return GetProfileLong(_T("PasteAsAdmin"), 1);
}

void CGetSetOptions::SetRememberDescPos(BOOL val)
{
	SetProfileLong(_T("RememberDescPos"), val);
}
BOOL CGetSetOptions::GetRememberDescPos()
{
	return GetProfileLong(_T("RememberDescPos"), FALSE);
}

void CGetSetOptions::SetSizeDescWindowToContent(BOOL val)
{
	SetProfileLong(_T("SizeDescWindowToContent"), val);
}
BOOL CGetSetOptions::GetSizeDescWindowToContent()
{
	return GetProfileLong(_T("SizeDescWindowToContent"), TRUE);
}

void CGetSetOptions::SetScaleImagesToDescWindow(BOOL val)
{
	SetProfileLong(_T("ScaleImagesToDescWindow"), val);
}
BOOL CGetSetOptions::GetScaleImagesToDescWindow()
{
	return GetProfileLong(_T("ScaleImagesToDescWindow"), TRUE);
}

void CGetSetOptions::SetDescWndPoint(CPoint point)
{
	SetResolutionProfileLong("DescWndX", point.x);
	SetResolutionProfileLong("DescWndY", point.y);
}

void CGetSetOptions::GetDescWndPoint(CPoint &point)
{
	point.x = GetResolutionProfileLong("DescWndX", 100);
	point.y = GetResolutionProfileLong("DescWndY", 100);
}

void CGetSetOptions::GetDescWndSize(CSize &size)
{
	size.cx = GetResolutionProfileLong("DescWndCX", 300);
	size.cy = GetResolutionProfileLong("DescWndCY", 300);
	if(size.cx <= 0 && size.cy <= 0)
	{
		size.cx = 300;
		size.cy = 300;
	}
}

void CGetSetOptions::SetDescWndSize(CSize size)
{
	SetResolutionProfileLong("DescWndCX", size.cx);
	SetResolutionProfileLong("DescWndCY", size.cy);
}

void CGetSetOptions::SetShowInTaskBar(BOOL val)
{
	SetProfileLong(_T("ShowInTaskBar"), val);
}

BOOL CGetSetOptions::GetShowInTaskBar()
{
	return GetProfileLong(_T("ShowInTaskBar"), FALSE);
}

void CGetSetOptions::SetDiffApp(CString val)
{
	SetProfileString(_T("DiffApp"), val);
}

CString	CGetSetOptions::GetDiffApp()
{
	return GetProfileString(_T("DiffApp"), _T(""));
}

void CGetSetOptions::SetQRCodeBorderPixels(int val)
{
	SetProfileLong(_T("QRCodeBorderPixels"), val);
}

int CGetSetOptions::GetQRCodeBorderPixels()
{
	return GetProfileLong(_T("QRCodeBorderPixels"), 30);
}

CString	CGetSetOptions::GetTranslateUrl()
{
	return GetProfileString(_T("TranslateUrl"), _T("https://translate.google.com/?text=%s"));
}

void CGetSetOptions::SetNetworkReadTimeoutMS(int val)
{
	SetProfileLong(_T("NetworkReadTimeoutMS"), val);
}

int CGetSetOptions::GetNetworkReadTimeoutMS()
{
	return GetProfileLong(_T("NetworkReadTimeoutMS"), 30000);
}

void CGetSetOptions::SetRequestFilesUsingIP(int val)
{
	SetProfileLong(_T("RequestFilesUsingIP"), val);
}

int CGetSetOptions::GetRequestFilesUsingIP()
{
	return GetProfileLong(_T("RequestFilesUsingIP"), 0);
}

int CGetSetOptions::ReadRandomFileInterval()
{
	return GetProfileLong(_T("ReadRandomFileInterval"), 60);
}

int CGetSetOptions::ReadRandomFileIdleMin()
{
	return GetProfileLong(_T("ReadRandomFileIdleMin"), 30);
}

BOOL CGetSetOptions::GetShowGroupsInMainList()
{
	return GetProfileLong(_T("ShowGroupsInMainList"), 0);
}

void CGetSetOptions::SetShowGroupsInMainList(BOOL val)
{
	SetProfileLong(_T("ShowGroupsInMainList"), val);
}

void CGetSetOptions::SetGroupDoubleClickTimeMS(int val)
{
	SetProfileLong(_T("GroupDoubleClickTimeMS"), val);
}

int CGetSetOptions::GetGroupDoubleClickTimeMS()
{
	return GetProfileLong(_T("GroupDoubleClickTimeMS"), 500);
}

void CGetSetOptions::SetSaveToGroupTimeoutMS(int val)
{
	SetProfileLong(_T("SaveToGroupTimeoutMS"), val);
}

int CGetSetOptions::GetSaveToGroupTimeoutMS()
{
	return GetProfileLong(_T("SaveToGroupTimeoutMS"), 1000);
}

void CGetSetOptions::SetCopyReasonTimeoutMS(int val)
{
	SetProfileLong(_T("CopyReasonTimeoutMS"), val);
}

int CGetSetOptions::GetCopyReasonTimeoutMS()
{
	return GetProfileLong(_T("CopyReasonTimeoutMS"), 1000);
}

void CGetSetOptions::SetWindowsResumeDelayReOpenDbMS(int val)
{
	SetProfileLong(_T("WindowsResumeDelayReOpenDbMS"), val);
}

int CGetSetOptions::GetWindowsResumeDelayReOpenDbMS()
{
	return GetProfileLong(_T("WindowsResumeDelayReOpenDbMS"), 2000);
}

BOOL CGetSetOptions::GetShowMsgWndOnCopyToGroup()
{
	return GetProfileLong(_T("ShowMsgWndOnCopyToGroup"), TRUE);
}

void CGetSetOptions::SetShowMsgWndOnCopyToGroup(BOOL val)
{
	SetProfileLong(_T("ShowMsgWndOnCopyToGroup"), val);
}

int CGetSetOptions::GetActionShortCutA(DWORD action, int pos, CString refData)
{
	CString actionText;
	if (refData != _T(""))
	{
		actionText.Format(_T("QP_ShortCut_%d_%s_%d_A"), action, refData, pos);
	}
	else
	{
		actionText.Format(_T("QP_ShortCut_%d_%d_A"), action, pos);
	}
	int ret = GetProfileLong(actionText, -1);
	if (ret == -1)
	{
		ret = ActionEnums::GetDefaultShortCutKeyA((ActionEnums::ActionEnumValues)action, pos);
	}

	return ret;
}

void CGetSetOptions::SetActionShortCutA(int action, DWORD shortcut, int pos, CString refData)
{
	CString actionText;
	if (refData != _T(""))
	{
		actionText.Format(_T("QP_ShortCut_%d_%s_%d_A"), action, refData, pos);
	}
	else
	{
		actionText.Format(_T("QP_ShortCut_%d_%d_A"), action, pos);
	}
	SetProfileLong(actionText, shortcut);
}

int CGetSetOptions::GetActionShortCutB(DWORD action, int pos, CString refData)
{
	CString actionText;
	if (refData != _T(""))
	{
		actionText.Format(_T("QP_ShortCut_%d_%s_%d_B"), action, refData, pos);
	}
	else
	{
		actionText.Format(_T("QP_ShortCut_%d_%d_B"), action, pos);
	}
	int ret = GetProfileLong(actionText, -1);
	if (ret == -1)
	{
		ret = ActionEnums::GetDefaultShortCutKeyB((ActionEnums::ActionEnumValues)action, pos);
	}

	return ret;
}

void CGetSetOptions::SetActionShortCutB(int action, DWORD shortcut, int pos, CString refData)
{
	CString actionText;
	if (refData != _T(""))
	{
		actionText.Format(_T("QP_ShortCut_%d_%s_%d_B"), action, refData, pos);
	}
	else
	{
		actionText.Format(_T("QP_ShortCut_%d_%d_B"), action, pos);
	}
	SetProfileLong(actionText, shortcut);
}

BOOL CGetSetOptions::GetShowAlwaysOnTopWarning()
{
	return GetProfileLong(_T("ShowAlwaysOnTopWarning"), TRUE);
}

void CGetSetOptions::SetShowAlwaysOnTopWarning(BOOL show)
{
	m_bShowAlwaysOnTopWarning = show;
	SetProfileLong(_T("ShowAlwaysOnTopWarning"), show);
}

BOOL CGetSetOptions::GetUseIPFromAccept()
{
	return GetProfileLong(_T("UseIPFromAccept"), TRUE);
}

void CGetSetOptions::SetUseIPFromAccept(BOOL useAccept)
{
	SetProfileLong(_T("UseIPFromAccept"), useAccept);
}

int CGetSetOptions::GetDragId()
{
	return GetProfileLong(_T("DragId"), 1);
}

void CGetSetOptions::SetDragId(int id)
{
	SetProfileLong(_T("DragId"), id);
}

BOOL CGetSetOptions::GetShowIfClipWasPasted()
{
	return GetProfileLong(_T("ShowIfClipWasPasted"), TRUE);
}

void CGetSetOptions::SetShowIfClipWasPasted(BOOL val)
{
	SetProfileLong(_T("ShowIfClipWasPasted"), val);
}

int CGetSetOptions::GetLastGroupToggle()
{
	return GetProfileLong(_T("LastGroupToggle"), -2);
}

void CGetSetOptions::SetLastGroupToggle(int val)
{
	SetProfileLong(_T("LastGroupToggle"), val);
}

BOOL CGetSetOptions::GetMouseClickHidesDescription()
{
	return GetProfileLong(_T("MouseClickHidesDescription"), TRUE);
}

void CGetSetOptions::SetMouseClickHidesDescription(int val)
{
	SetProfileLong(_T("MouseClickHidesDescription"), val);
}

BOOL CGetSetOptions::GetWrapDescriptionText()
{
	return GetProfileLong(_T("WrapDescriptionText"), FALSE);
}

void CGetSetOptions::SetWrapDescriptionText(int val)
{
	SetProfileLong(_T("WrapDescriptionText"), val);
}

BOOL CGetSetOptions::GetUseUISelectedGroupForLastTenCopies()
{
	return GetProfileLong(_T("UseUISelectedGroupForLastTenCopies"), FALSE);
}

void CGetSetOptions::SetUseUISelectedGroupForLastTenCopies(int val)
{
	SetProfileLong(_T("UseUISelectedGroupForLastTenCopies"), val);
}


int CGetSetOptions::GetDelayRenderLockout()
{
	return GetProfileLong(_T("DelayRenderLockout"), 1000);
}

void CGetSetOptions::SetDelayRenderLockout(int val)
{
	SetProfileLong(_T("DelayRenderLockout"), val);
}

BOOL CGetSetOptions::GetAdjustClipsForCRC()
{
	return GetProfileLong(_T("AdjustClipsForCRC"), TRUE);
}

void CGetSetOptions::SetAdjustClipsForCRC(int val)
{
	SetProfileLong(_T("AdjustClipsForCRC"), val);
}

BOOL CGetSetOptions::GetCheckMd5OnFileTransfers()
{
	return GetProfileLong(_T("CheckMd5OnFileTransfers"), TRUE);
}

void CGetSetOptions::SetCheckMd5OnFileTransfers(int val)
{
	SetProfileLong(_T("CheckMd5OnFileTransfers"), val);
}

int CGetSetOptions::GetBalloonTimeout()
{
	return GetProfileLong(_T("BalloonTimeout"), 2500);
}

void CGetSetOptions::SetBalloonTimeout(int val)
{
	SetProfileLong(_T("BalloonTimeout"), val);
}

void CGetSetOptions::SetCustomSendToList(CString val)
{
	SetProfileString(_T("CustomSendToList2"), val);
}

CString	CGetSetOptions::GetCustomSendToList()
{
	return GetProfileString("CustomSendToList2", "");
}

int CGetSetOptions::GetMaxFileContentsSize()
{
	return GetProfileLong(_T("MaxFileContentsSize"), 64000000);
}

void CGetSetOptions::SetMaxFileContentsSize(int val)
{
	SetProfileLong(_T("MaxFileContentsSize"), val);
}

int CGetSetOptions::GetErrorMsgPopupTimeout()
{
	return GetProfileLong(_T("ErrorMsgPopupTimeout"), 3500);
}

void CGetSetOptions::SetErrorMsgPopupTimeout(int val)
{
	SetProfileLong(_T("ErrorMsgPopupTimeout"), val);
}

void CGetSetOptions::SetRegexFilter(CString val, int pos)
{
	CString cs;
	cs.Format(_T("RegexFilter_%d"), pos);

	m_regexHelper.SetRegEx(pos, std::wstring(val));

	SetProfileString(cs, val);
}

CString	CGetSetOptions::GetRegexFilter(int pos)
{
	CString cs;
	cs.Format(_T("RegexFilter_%d"), pos);
	return GetProfileString(cs, "");
}

void CGetSetOptions::SetRegexFilterByProcessName(CString val, int pos)
{
	CString cs;
	cs.Format(_T("RegexFilterByProcessName_%d"), pos);

	m_regexHelper.SetProcessFilter(pos, val);

	SetProfileString(cs, val);
}

CString	CGetSetOptions::GetRegexFilterByProcessName(int pos)
{
	CString cs;
	cs.Format(_T("RegexFilterByProcessName_%d"), pos);
	return GetProfileString(cs, "");
}

BOOL CGetSetOptions::GetOpenToGroupByActiveExe()
{
	return GetProfileLong(_T("OpenToGroupByActiveExe"), FALSE);
}

void CGetSetOptions::SetOpenToGroupByActiveExe(int val)
{
	SetProfileLong(_T("OpenToGroupByActiveExe"), val);
}

BOOL CGetSetOptions::GetExcludeCF_DIBInExcel()
{
	return GetProfileLong(_T("ExcludeCF_DIBInExcel"), TRUE);
}

void CGetSetOptions::SetExcludeCF_DIBInExcel(int val)
{
	m_excludeCF_DIBInExcel = val;
	SetProfileLong(_T("ExcludeCF_DIBInExcel"), val);
}

BOOL CGetSetOptions::GetShowStartupMessage()
{
	return GetProfileLong(_T("ShowStartupMessage"), TRUE);
}

void CGetSetOptions::SetShowStartupMessage(int val)
{
	SetProfileLong(_T("ShowStartupMessage"), val);
}

CString CGetSetOptions::GetCopyScriptsXml()
{
	return GetProfileString("CopyScriptsXml", "");
}

void CGetSetOptions::SetCopyScriptsXml(CString val)
{
	m_copyScripts.Load(val);
	SetProfileString(_T("CopyScriptsXml"), val);
}

CString CGetSetOptions::GetPasteScriptsXml()
{
	return GetProfileString("PasteScriptsXml", "");
}

void CGetSetOptions::SetPasteScriptsXml(CString val)
{
	m_pasteScripts.Load(val);
	SetProfileString(_T("PasteScriptsXml"), val);
}

long CGetSetOptions::GetToolTipTimeout()
{
	return GetProfileLong("ToolTipTimeout", -1);
}

void CGetSetOptions::SetToolTipTimeout(long val)
{
	m_tooltipTimeout = val;
	SetProfileLong("ToolTipTimeout", val);
}

CString CGetSetOptions::GetPastSearchXml()
{
	//max this out at 1000, this should be more than the allowed length in SymbolEdit.cpp
	//had reports of this being really large and causing memory issues, this will prevent us from this
	return GetProfileString("PastSearchXml", "", "", 1000);
}

void CGetSetOptions::SetPastSearchXml(CString val)
{
	SetProfileString(_T("PastSearchXml"), val);
}

BOOL CGetSetOptions::GetShowMsgWhenReceivingManualSentClip()
{
	return GetProfileLong("ShowMsgWhenReceivingManualSentClip", TRUE);
}

void CGetSetOptions::SetShowMsgWhenReceivingManualSentClip(BOOL val)
{
	SetProfileLong("ShowMsgWhenReceivingManualSentClip", val);
}


BOOL CGetSetOptions::GetCleanRTFBeforeDrawing()
{
	return GetProfileLong("CleanRTFBeforeDrawing", TRUE);
}

void CGetSetOptions::SetCleanRTFBeforeDrawing(BOOL val)
{
	m_cleanRTFBeforeDrawing = true;
	SetProfileLong("CleanRTFBeforeDrawing", val);
}

BOOL CGetSetOptions::GetDisableExpireClipsConfig()
{
	return GetProfileLong("DisableExpireClipsConfig", FALSE);
}

void CGetSetOptions::SetDisableExpireClipsConfig(BOOL val)
{
	SetProfileLong("DisableExpireClipsConfig", val);
}

BOOL CGetSetOptions::GetRevertToTopLevelGroup()
{
	return GetProfileLong("RevertToTopLevelGroup", FALSE);
}

void CGetSetOptions::SetRevertToTopLevelGroup(BOOL val)
{
	SetProfileLong("RevertToTopLevelGroup", val);
}

BOOL CGetSetOptions::GetUpdateClipOrderOnCtrlC()
{
	return GetProfileLong("UpdateClipOrderOnCtrlC", FALSE);
}

void CGetSetOptions::SetUpdateClipOrderOnCtrlC(BOOL val)
{
	SetProfileLong("UpdateClipOrderOnCtrlC", val);
}

int CGetSetOptions::GetMaxToolTipLines()
{
	return GetProfileLong("MaxToolTipLines", 30);
}

void CGetSetOptions::SetMaxToolTipLines(int val)
{
	SetProfileLong("MaxToolTipLines", val);
}

int CGetSetOptions::GetMaxToolTipCharacters()
{
	return GetProfileLong("MaxToolTipCharacters", 1000);
}

void CGetSetOptions::SetMaxToolTipCharacters(int val)
{
	SetProfileLong("MaxToolTipCharacters", val);
}

int CGetSetOptions::GetDoubleKeyStrokeTimeout()
{
	return GetProfileLong("DoubleKeyStrokeTimeout", 350);
}

void CGetSetOptions::SetDoubleKeyStrokeTimeout(int val)
{
	m_doubleKeyStrokeTimeout = val;
	SetProfileLong("DoubleKeyStrokeTimeout", val);
}

int CGetSetOptions::GetFirstTenHotKeysStart()
{
	return GetProfileLong("FirstTenHotKeysStart", 1);
}

void CGetSetOptions::SetFirstTenHotKeysStart(int val)
{
	m_firstTenHotKeysStart = val;
	SetProfileLong("FirstTenHotKeysStart", val);
}

int CGetSetOptions::GetFirstTenHotKeysFontSize()
{
	return GetProfileLong("FirstTenHotKeysFontSize", 5);
}

void CGetSetOptions::SetFirstTenHotKeysFontSize(int val)
{
	m_firstTenHotKeysFontSize = val;
	SetProfileLong("FirstTenHotKeysFontSize", val);
}

BOOL CGetSetOptions::GetAddCFHDROP_OnDrag()
{
	return GetProfileLong(_T("AddCFHDROP_OnDrag"), TRUE);
}

void CGetSetOptions::SetAddCFHDROP_OnDrag(BOOL val)
{
	SetProfileLong(_T("AddCFHDROP_OnDrag"), val);
}

int CGetSetOptions::GetCopyAndSveDelay()
{
	return GetProfileLong("CopyAndSveDelay", 1000);
}

void CGetSetOptions::SetCopyAndSveDelay(int val)
{
	SetProfileLong("CopyAndSveDelay", val);
}

int CGetSetOptions::GetEditorDefaultFontSize()
{
	return GetProfileLong("EditorDefaultFontSize", 14);
}

void CGetSetOptions::SetEditorDefaultFontSize(int val)
{
	SetProfileLong("EditorDefaultFontSize", val);
}

BOOL CGetSetOptions::GetMoveSelectionOnOpenHotkey()
{
	return GetProfileLong("MoveSelectionOnOpenHotkey", TRUE);
}

void CGetSetOptions::SetMoveSelectionOnOpenHotkey(BOOL val)
{
	m_moveSelectionOnOpenHotkey = val;
	SetProfileLong("MoveSelectionOnOpenHotkey", val);
}

BOOL CGetSetOptions::GetAllowBackToBackDuplicates()
{
	return GetProfileLong("AllowBackToBackDuplicates", FALSE);
}

void CGetSetOptions::SetAllowBackToBackDuplicates(BOOL val)
{
	m_allowBackToBackDuplicates = val;
	SetProfileLong("AllowBackToBackDuplicates", val);
}

BOOL CGetSetOptions::GetMaintainSearchView()
{
	return GetProfileLong("MaintainSearchView", FALSE);
}

void CGetSetOptions::SetMaintainSearchView(BOOL val)
{
	m_maintainSearchView = val;
	SetProfileLong("MaintainSearchView", val);
}

CString CGetSetOptions::GetNetworkBindIPAddress()
{
	return GetProfileString("NetworkBindIPAddress", "*");
}

void CGetSetOptions::SetNetworkBindIPAddress(CString val)
{
	SetProfileString("NetworkBindIPAddress", val);
}

CString CGetSetOptions::GetTempDragFileName()
{
	auto diff = CTime::GetCurrentTime() - m_tempDragFileNameSetTime;
	if (diff.GetTotalSeconds() < 60 * 2)
	{
		return m_tempDragFileName;
	}

	return _T("");
}

void CGetSetOptions::SetTempDragFileName(CString val)
{
	m_tempDragFileName = val;
	m_tempDragFileNameSetTime = CTime::GetCurrentTime();
}

BOOL CGetSetOptions::GetRefreshViewAfterPasting()
{
	return GetProfileLong("RefreshViewAfterPasting", FALSE);
}

void CGetSetOptions::SetRefreshViewAfterPasting(BOOL val)
{
	m_refreshViewAfterPasting = val;
	SetProfileLong("RefreshViewAfterPasting", val);
}