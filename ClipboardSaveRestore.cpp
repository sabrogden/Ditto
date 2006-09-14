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
	bool bRet = false;
	COleDataObjectEx oleData;
	CClipFormat cf;
	FORMATETC fm;
	
	//Attach to the clipboard
	if(!oleData.AttachClipboard())
	{
		ASSERT(0);
		return bRet;
	}

	oleData.EnsureClipboardObject();

	oleData.BeginEnumFormats();

	while(oleData.GetNextFormat(&fm))
	{
		cf.m_hgData = oleData.GetGlobalData(fm.cfFormat);
		cf.m_cfType = fm.cfFormat;

		if(cf.m_hgData)
		{
			int nSize = GlobalSize(cf.m_hgData);
			if(nSize > 0)
			{
				m_Clipboard.Add(cf);
				bRet = true;
			}
			else
			{
				ASSERT(FALSE); // a valid GlobalMem with 0 size is strange
				cf.Free();
			}
			cf.m_hgData = 0; // m_Formats owns it now
		}
	}

	oleData.Release();

	return bRet;
}

bool CClipboardSaveRestore::Restore()
{
	bool bRet = false;
	if(::OpenClipboard(NULL))
	{
		::EmptyClipboard();

		int nSize = m_Clipboard.GetSize();
		for(int nPos = 0; nPos < nSize; nPos++)
		{
			CClipFormat cf = m_Clipboard[nPos];
			if(cf.m_hgData)
			{
				::SetClipboardData(cf.m_cfType, cf.m_hgData);
				cf.m_hgData = NULL;//clipboard now owns the data

				bRet = TRUE;
			}
		}

		SetClipboardData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));

		::CloseClipboard();
	}

	m_Clipboard.RemoveAll();

	return bRet;
}