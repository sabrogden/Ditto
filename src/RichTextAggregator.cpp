#include "stdafx.h"
#include ".\richtextaggregator.h"
#include "Misc.h"

CRichTextAggregator::CRichTextAggregator(CStringA csSeparator) :
	m_csSeparator(csSeparator)
{
	//Remove the first line feed
//	if(m_csSeparator.GetLength() > 1 && m_csSeparator[0] == '\r' && m_csSeparator[1] == '\n')
//	{
//		m_csSeparator.Delete(0);
//		m_csSeparator.Delete(0);
//	}

	m_csSeparator.Replace("\r\n", "\\par");
}

CRichTextAggregator::~CRichTextAggregator(void)
{
}

bool CRichTextAggregator::AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType)
{
	LPSTR pText = (LPSTR)lpData;
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

	if(nPos != nCount-1)
	{
		//Remove the last } at the end of the rtf
		bool bBreak = false;
		for(int i = nDataSize-1; i >= 0; i--)
		{
			if(pText[i] == '}')
				bBreak = true;

			pText[i] = NULL;

			if(bBreak)
				break;
		}
	}
	else if(nPos >= 1)
	{
		//Remove the {\rtf1 at the start of the rtf
		for(int i = 0; i < 6; i++)
		{
			pText[0] = NULL;
			pText++;
		}
	}

	m_csNewText += pText;

	if(nPos != nCount-1)
	{
		m_csNewText += m_csSeparator;
	}

	return true;
}

HGLOBAL CRichTextAggregator::GetHGlobal()
{
	long lLen = m_csNewText.GetLength();
	HGLOBAL hGlobal = NewGlobalP(m_csNewText.GetBuffer(lLen), lLen+sizeof(char));
	m_csNewText.ReleaseBuffer();

	return hGlobal;
}