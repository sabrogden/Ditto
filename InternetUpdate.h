// InternetUpdate.h: interface for the CInternetUpdate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERNETUPDATE_H__0ED6CFE4_1E10_4D32_93A1_1C5CC1FDA3D2__INCLUDED_)
#define AFX_INTERNETUPDATE_H__0ED6CFE4_1E10_4D32_93A1_1C5CC1FDA3D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class VersionInfo
{
public:
	int Major;
	int Minor;
	int Revision;
	int Build;
};

class CInternetUpdate  
{
public:
	CInternetUpdate();
	virtual ~CInternetUpdate();

	//BOOL CheckForUpdate(HWND hParent, BOOL bCheckForPrevUpdate, BOOL bShowNoUpdatesDlg);

	VersionInfo GetRunningVersion();
	//long GetUpdateVersion();

	CString GetVersionString(VersionInfo lVersion);
	//CString DownloadUpdate();
	
protected:
	//long m_lRunningVersion;
	//long m_lUpdateVersion;
	//HWND m_hParent;
	//BOOL m_bShowMessages;

	//BOOL RemoveOldUpdateFile();

};

#endif // !defined(AFX_INTERNETUPDATE_H__0ED6CFE4_1E10_4D32_93A1_1C5CC1FDA3D2__INCLUDED_)
