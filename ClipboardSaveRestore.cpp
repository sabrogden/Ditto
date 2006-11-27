#include "stdafx.h"
#include "CP_Main.h"
#include ".\clipboardsaverestore.h"

CClipboardSaveRestore::CClipboardSaveRestore(void)
{
}

CClipboardSaveRestore::~CClipboardSaveRestore(void)
{
}

bool CClipboardSaveRestore::Save()
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
			HGLOBAL hGlobal = ::GetClipboardData(nFormat);
			LPVOID pvData = GlobalLock(hGlobal);
			if(pvData)
			{
				ULONG Size = GlobalSize(hGlobal);
				if(Size > 0)
				{
					//Copy the data locally
					cf.m_hgData = NewGlobalP(pvData, Size);	
					cf.m_cfType = nFormat;

					m_Clipboard.Add(cf);
				}

				GlobalUnlock(hGlobal);
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

		int nSize = m_Clipboard.GetSize();
		for(int nPos = 0; nPos < nSize; nPos++)
		{
			CClipFormat *pCF = &m_Clipboard.ElementAt(nPos);
			if(pCF && pCF->m_hgData)
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