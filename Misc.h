
#if !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
#define AFX_CP_GUI_H__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DatabaseUtilities.h"

#define HK_ACTIVATE		"HK_ACTIVATE"
#define HK_NAMED_COPY	"NAMED_COPY"

long DoOptions(CWnd *pParent);
HWND GetActiveWnd(CPoint *pPointCaret = NULL);

CLIPFORMAT GetFormatID(LPCSTR cbName);
CString GetFormatName(CLIPFORMAT cbType);
BOOL PreTranslateGuiDll(MSG *pMsg);

CString GetFilePath(CString csFullPath);

#define POS_AT_CARET	1
#define POS_AT_CURSOR	2
#define POS_AT_PREVIOUS	3

//Message to the main window to show icon or not
#define WM_SHOW_TRAY_ICON			WM_USER + 200
#define WM_RECONNECT_TO_COPY_CHAIN	WM_USER + 201
#define WM_IS_TOP_VIEWER			WM_USER + 202
#define WM_COPYPROPERTIES			WM_USER + 203
#define WM_CLOSE_APP				WM_USER + 204

#define REG_PATH					"Software\\Ditto"

class CGetSetOptions  
{
public:
	CGetSetOptions();
	virtual ~CGetSetOptions();

	//System Tray Icon
	static BOOL GetShowIconInSysTray();
	static BOOL SetShowIconInSysTray(BOOL bShow);

	//Hot Keys
	static DWORD GetHotKey();
	static BOOL SetHotKey(DWORD dwHotKey);
	static BOOL RegisterHotKey(HWND hWnd, DWORD wHotKey, ATOM atomID);
	static UINT GetModifier(DWORD wHotKey);
	static DWORD GetNamedCopyHotKey();
	static BOOL SetNamedCopyHotKey(long lHotKey);

	//Run At StartUp
	static BOOL GetRunOnStartUp();
	static void SetRunOnStartUp(BOOL bRun);
	static CString GetExeFileName();
	static CString GetAppName();
	
	static BOOL SetProfileLong(CString csName, long lValue);
	static long GetProfileLong(CString csName, long bDefaultValue = -1);

	static CString GetProfileString(CString csName, CString csDefault);
	static BOOL	SetProfileString(CString csName, CString csValue);

	static BOOL SetQuickPasteSize(CSize size);
	static void GetQuickPasteSize(CSize &size);

	static BOOL SetQuickPastePoint(CPoint point);
	static void GetQuickPastePoint(CPoint &point);

	static BOOL SetEnableTransparency(BOOL bCheck);
	static BOOL GetEnableTransparency();

	static BOOL SetTransparencyPercent(long lPercent);
	static long GetTransparencyPercent();

	static BOOL SetLinesPerRow(long lLines);
	static long GetLinesPerRow();

	static BOOL SetQuickPastePosition(long lPosition);
	static long GetQuickPastePosition();

	static long CGetSetOptions::GetCopyGap();

	static BOOL SetDBPath(CString csPath);
	static CString GetDBPath(BOOL bDefault = TRUE);

	static void SetCheckForMaxEntries(BOOL bVal);
	static BOOL GetCheckForMaxEntries();

	static void SetCheckForExpiredEntries(BOOL bVal);
	static BOOL GetCheckForExpiredEntries();

	static void SetCompactAndRepairOnExit(BOOL bVal);
	static BOOL GetCompactAndRepairOnExit();

	static void SetMaxEntries(long lVal);
	static long GetMaxEntries();

	static void SetExpiredEntries(long lVal);
	static long GetExpiredEntries();

	static void SetTripCopyCount(long lVal);
	static long GetTripCopyCount();
	static void SetTripPasteCount(long lVal);
	static long GetTripPasteCount();
	static void SetTripDate(long lDate);
	static long GetTripDate();

	static void SetTotalCopyCount(long lVal);
	static long GetTotalCopyCount();
	static void SetTotalPasteCount(long lVal);
	static long GetTotalPasteCount();
	static void SetTotalDate(long lDate);
	static long GetTotalDate();

	static CString	GetUpdateFilePath()				{ return GetProfileString("UpdateFilePath", "");	}
	static BOOL		SetUpdateFilePath(CString cs)	{ return SetProfileString("UpdateFilePath", cs);	}

	static CString	GetUpdateInstallPath()				{ return GetProfileString("UpdateInstallPath", "");	}
	static BOOL		SetUpdateInstallPath(CString cs)	{ return SetProfileString("UpdateInstallPath", cs);	}

	static long		GetLastUpdate()				{ return GetProfileLong("LastUpdateDay", 0);		}
	static long		SetLastUpdate(long lValue)	{ return SetProfileLong("LastUpdateDay", lValue);	}

	static BOOL		GetCheckForUpdates()			{ return GetProfileLong("CheckForUpdates", TRUE);	}
	static BOOL		SetCheckForUpdates(BOOL bCheck)	{ return SetProfileLong("CheckForUpdates", bCheck);	}

	static void		SetUseCtrlNumForFirstTenHotKeys(BOOL bVal)	{	SetProfileLong("UseCtrlNumForFirstTenHotKeys", bVal);		}
	static BOOL		GetUseCtrlNumForFirstTenHotKeys()			{	return GetProfileLong("UseCtrlNumForFirstTenHotKeys", 0);	}

	static void		SetShowTextForFirstTenHotKeys(BOOL bVal)	{	SetProfileLong("ShowTextForFirstTenHotKeys", bVal);			}
	static BOOL		GetShowTextForFirstTenHotKeys()				{	return GetProfileLong("ShowTextForFirstTenHotKeys", TRUE);	}

	static void		SetMainHWND(long lhWnd)		{	SetProfileLong("MainhWnd", lhWnd);		}
	static BOOL		GetMainHWND()				{	return GetProfileLong("MainhWnd", 0);	}

	/*
	BOOL IsAutoRun();
	void SetAutoRun(BOOL bRun);
	CString GetExeFileName();
	*/
};


#endif // !defined(AFX_CP_GUI_GLOBALS__FBCDED09_A6F2_47EB_873F_50A746EBC86B__INCLUDED_)
