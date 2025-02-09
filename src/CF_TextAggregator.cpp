#include "stdafx.h"
#include ".\cf_textaggregator.h"
#include "Misc.h"

CCF_TextAggregator::CCF_TextAggregator(CStringA csSepator) :
	m_csSeparator(csSepator)
{
}

CCF_TextAggregator::~CCF_TextAggregator(void)
{
}

bool CCF_TextAggregator::AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType)
{
	if (cfType == CF_HDROP)
	{
		CStringA hDropFiles = _T("");
		HDROP drop = (HDROP)GlobalLock((HDROP)lpData);
		int nNumFiles = DragQueryFileA(drop, -1, NULL, 0);
		CHAR file[MAX_PATH];

		for (int nFile = 0; nFile < nNumFiles; nFile++)
		{
			if (DragQueryFileA(drop, nFile, file, sizeof(file)) > 0)
			{
				hDropFiles += file;
				hDropFiles += "\r\n";
			}
		}

		if (hDropFiles != _T(""))
		{
			m_csNewText += hDropFiles;

			if (nPos != nCount - 1)
			{
				m_csNewText += m_csSeparator;
			}

			return true;
		}
		return false;
	}

	LPCSTR pText = (LPCSTR)lpData;
	if(pText == NULL)
	{
		return false;
	}

	//Ensure it's null terminated
	if(pText[nDataSize-1] != '\0')
	{
		int len = 0;
		for(len = 0; len < nDataSize && pText[len] != '\0'; len++ )
		{
		}
		// if it is not null terminated, skip this item
		if(len >= nDataSize)
			return false;
	}

	m_csNewText += pText;
	
	if(nPos != nCount-1)
	{
		m_csNewText += m_csSeparator;
	}

	return true;
}

HGLOBAL CCF_TextAggregator::GetHGlobal()
{
	long lLen = m_csNewText.GetLength();
	HGLOBAL hGlobal = NewGlobalP(m_csNewText.GetBuffer(lLen), lLen+sizeof(char));
	m_csNewText.ReleaseBuffer();

	return hGlobal;
}
