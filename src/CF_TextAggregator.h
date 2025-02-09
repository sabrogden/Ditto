#pragma once
#include "IClipAggregator.h"

class CCF_TextAggregator : public IClipAggregator
{
public:
	CCF_TextAggregator(CStringA csSepator);
	~CCF_TextAggregator(void);

	virtual bool AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType);
	virtual HGLOBAL GetHGlobal();

protected:
	CStringA m_csSeparator;
	CStringA m_csNewText;
};
