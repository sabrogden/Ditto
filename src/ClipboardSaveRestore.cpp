#include "stdafx.h"
#include "CP_Main.h"
#include ".\clipboardsaverestore.h"

CClipboardSaveRestore::CClipboardSaveRestore(void)
{
}

CClipboardSaveRestore::~CClipboardSaveRestore(void)
{
}

bool CClipboardSaveRestore::Save(BOOL textOnly)
{
	m_Clipboard.RemoveAll();

	bool bRet = false;
	COleDataObjectEx oleData;
	CClipFormat cf;

	if(::OpenClipboard(theApp.m_MainhWnd))
	{
		int nFormat = EnumClipboardFormats(0);
		while(nFormat != 0)
		{
			if(textOnly == false || (nFormat == CF_TEXT || nFormat == CF_UNICODETEXT || nFormat == CF_HDROP))
			{
				HGLOBAL hGlobal = ::GetClipboardData(nFormat);
				if(hGlobal && ::GlobalSize(hGlobal) > 0) // Ensure clipboard data is valid
				{
					LPVOID pvData = GlobalLock(hGlobal);
					if(pvData)
					{
						INT_PTR size = GlobalSize(hGlobal);
						if(size > 0)
						{
							//Copy the data locally
							cf.m_hgData = NewGlobalP(pvData, size);	
							cf.m_cfType = nFormat;

							m_Clipboard.Add(cf);

							//m_Clipboard owns the data now
							cf.m_hgData = NULL;
						}

						GlobalUnlock(hGlobal);
					}
				}
			}
			nFormat = EnumClipboardFormats(nFormat);
		}

		::CloseClipboard();
		bRet = true;
	}

	return bRet;
}

bool CClipboardSaveRestore::Restore()
{
	bool bRet = false;

	if(::OpenClipboard(theApp.m_MainhWnd))
	{
		::EmptyClipboard();

		SetClipboardData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));

		INT_PTR size = m_Clipboard.GetSize();
		for(int nPos = 0; nPos < size; nPos++)
		{
			CClipFormat *pCF = &m_Clipboard.ElementAt(nPos);
			if(pCF && pCF->m_hgData && ::GlobalSize(pCF->m_hgData) > 0) // Ensure clipboard data is valid
			{
				::SetClipboardData(pCF->m_cfType, pCF->m_hgData);
				pCF->m_hgData = NULL;//clipboard now owns the data
			}
		}

		bRet = TRUE;
		::CloseClipboard();
	}

	m_Clipboard.RemoveAll();

	if(bRet == FALSE)
	{
		Log(_T("CClipboardSaveRestore::Restore failed to restore clipboard"));
	}

	return bRet;
}

bool CClipboardSaveRestore::RestoreTextOnly()
{
	bool bRet = false;

	if(::OpenClipboard(theApp.m_MainhWnd))
	{
		::EmptyClipboard();

		SetClipboardData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));

		bool foundText = false;
		int hDropIndex = -1;

		INT_PTR size = m_Clipboard.GetSize();
		for(int pos = 0; pos < size; pos++)
		{
			CClipFormat *pCF = &m_Clipboard.ElementAt(pos);
			if(pCF && pCF->m_hgData && ::GlobalSize(pCF->m_hgData) > 0) // Ensure clipboard data is valid
			{
				if(pCF->m_cfType == CF_TEXT || pCF->m_cfType == CF_UNICODETEXT)
				{
					//Make a copy of the data we are putting on the clipboard so we can still
					//restore all clips later in Restore()
					LPVOID localData = ::GlobalLock(pCF->m_hgData);

					HGLOBAL newData = NewGlobalP(localData, ::GlobalSize(pCF->m_hgData));	
					::SetClipboardData(pCF->m_cfType, newData);

					::GlobalUnlock(pCF->m_hgData);

					foundText = true;
				}
				else if(pCF->m_cfType == CF_HDROP)
				{
					hDropIndex = pos;
				}
			}
		}

		//if we didn't place text on the clipboard and we have a hdrop then convert the hdrop to text only with contents of hdrop
		if(foundText == false &&
			hDropIndex > -1)
		{
			CString hDropString;
			CClipFormat *pCF = &m_Clipboard.ElementAt(hDropIndex);
			if(pCF && pCF->m_hgData && ::GlobalSize(pCF->m_hgData) > 0) // Ensure clipboard data is valid
			{
				HDROP drop = (HDROP)GlobalLock(pCF->m_hgData);
				int nNumFiles = DragQueryFile(drop, -1, NULL, 0);
				TCHAR file[MAX_PATH];

				for(int nFile = 0; nFile < nNumFiles; nFile++)
				{
					if(DragQueryFile(drop, nFile, file, sizeof(file)) > 0)
					{
						if(PathIsDirectory(file) == FALSE)
						{
							hDropString += file;
							hDropString += _T("\r\n");
						}
					}
				}

				GlobalUnlock(pCF->m_hgData);

				HGLOBAL newData = NewGlobalP(hDropString.GetBuffer(), ((hDropString.GetLength() + 1) * sizeof(TCHAR)));	
				::SetClipboardData(CF_UNICODETEXT, newData);
			}
		}

		bRet = TRUE;
		::CloseClipboard();
	}

	if(bRet == FALSE)
	{
		Log(_T("CClipboardSaveRestore::Restore failed to restore clipboard"));
	}

	return bRet;
}
