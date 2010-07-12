#include "stdafx.h"
#include ".\cf_unicodetextaggregator.h"
#include "Misc.h"

CCF_UnicodeTextAggregator::CCF_UnicodeTextAggregator(CStringW csSeparator) :
	m_csSeparator(csSeparator)
{
}

CCF_UnicodeTextAggregator::~CCF_UnicodeTextAggregator(void)
{
}

bool CCF_UnicodeTextAggregator::AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount)
{
	LPCWSTR pText = (LPCWSTR)lpData;
	if(pText == NULL)
	{
		return false;
	}

	int stringLen = nDataSize/sizeof(wchar_t);

	//Ensure it's null terminated
	if(pText[stringLen-1] != '\0')
	{
		int len = 0;
		for(len = 0; len < stringLen && pText[len] != '\0'; len++ )
		{
		}
		// if it is not null terminated, skip this item
		if(len >= stringLen)
			return false;
	}

	m_csNewText += pText;
	
	if(nPos != nCount-1)
	{
		m_csNewText += m_csSeparator;
	}

	return true;
}

HGLOBAL CCF_UnicodeTextAggregator::GetHGlobal()
{
	long lLen = m_csNewText.GetLength() * sizeof(wchar_t);
	HGLOBAL hGlobal = NewGlobalP(m_csNewText.GetBuffer(lLen), lLen+sizeof(wchar_t));
	m_csNewText.ReleaseBuffer();

	return hGlobal;
}