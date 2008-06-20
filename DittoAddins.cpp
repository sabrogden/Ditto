#include "stdafx.h"
#include ".\dittoaddins.h"
#include "InternetUpdate.h"
#include "CP_Main.h"

CDittoAddins::CDittoAddins(void)
{
}

CDittoAddins::~CDittoAddins(void)
{
	UnloadAll();
}

bool CDittoAddins::UnloadAll()
{
	Log(StrF(_T("Ditto Addin - Unloading all addins Count: %d"), m_Addins.size()));

	int nCount = m_Addins.size();
	for(int i = 0; i < nCount; i++)
	{
		CDittoAddin *pAddin = m_Addins[i];
		if(pAddin)
		{
			delete pAddin;
			pAddin = NULL;
		}
	}

	m_Addins.clear();

	return true;
}

bool CDittoAddins::LoadAll()
{
	CDittoInfo DittoInfo;
	LoadDittoInfo(DittoInfo);

	CString csDir = CGetSetOptions::GetPath(PATH_ADDINS);

	CFileFind find;
	BOOL bCont = find.FindFile(csDir + _T("*.dll"));

	while(bCont)
	{
		bCont = find.FindNextFile();

		Log(StrF(_T("Ditto Addin - Trying to load addin file %s"), find.GetFilePath()));

		CDittoAddin *pAddin = new CDittoAddin;
		if(pAddin->DoLoad(find.GetFilePath(), DittoInfo))
		{
			Log(StrF(_T("Ditto Addin - Success, loaded addin: %s"), find.GetFilePath()));
			m_Addins.push_back(pAddin);
		}
		else
		{
			Log(StrF(_T("Ditto Addin - Failed loading Adding Error: %s"), pAddin->LastError()));

			delete pAddin;
			pAddin = NULL;
		}
	}

	return m_Addins.size() > 0;
}

bool CDittoAddins::AddPrePasteAddinsToMenu(CMenu *pMenu)
{
	bool bRet = false;

	m_FunctionMap.RemoveAll();
	int nMenuId = 3000;

	HMENU AllAddinsMenu = ::CreateMenu();

	int nCount = m_Addins.size();
	for(int i = 0; i < nCount; i++)
	{
		CDittoAddin *pAddin = m_Addins[i];
		if(pAddin)
		{
			int nSubCount = pAddin->m_PrePasteFunctions.size();
			if(nSubCount > 0)
			{
				HMENU AddinMenu = ::CreateMenu();
				for(int x = 0; x < nSubCount; x++)
				{
					::AppendMenu(AddinMenu, MF_ENABLED, nMenuId, pAddin->m_PrePasteFunctions[x].m_csDisplayName);

					CFunctionLookup lookup;
					lookup.m_csFunctionName = pAddin->m_PrePasteFunctions[x].m_csFunction;
					lookup.m_pAddin = pAddin;
					m_FunctionMap.SetAt(nMenuId, lookup);
					nMenuId++;
				}

				::AppendMenu(AllAddinsMenu, MF_ENABLED|MF_POPUP, (UINT)AddinMenu, pAddin->DisplayName());
				bRet = true;
			}
		}
	}

	if(bRet)
	{
		pMenu->InsertMenu(3, MF_BYPOSITION|MF_ENABLED|MF_STRING|MF_POPUP, (UINT)AllAddinsMenu, _T("Add-Ins"));
	}

	return bRet;
}

bool CDittoAddins::CallPrePasteFunction(int Id, IClip *pClip)
{
	bool bRet = false;
	CFunctionLookup func;
	if(m_FunctionMap.Lookup(Id, func))
	{
		CDittoInfo DittoInfo;
		LoadDittoInfo(DittoInfo);

		bRet = func.m_pAddin->PrePasteFunction(DittoInfo, func.m_csFunctionName, pClip);
	}

	return bRet;
}

void CDittoAddins::LoadDittoInfo(CDittoInfo &DittoInfo)
{
	DittoInfo.m_csDatabasePath = CGetSetOptions::GetDBPath();
	DittoInfo.m_csLanguageCode = theApp.m_Language.GetLangCode();
	CInternetUpdate update;
	DittoInfo.m_nVersion = update.GetRunningVersion();
	DittoInfo.m_csSqliteVersion = sqlite3_libversion();
	DittoInfo.m_hWndDitto = theApp.QPastehWnd();
}