#pragma once
#include "IClipAggregator.h"

class CCF_UnicodeTextAggregator : public IClipAggregator
{
public:
	CCF_UnicodeTextAggregator(CStringW csSeparator);
	~CCF_UnicodeTextAggregator(void);

	virtual bool AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount);
	virtual HGLOBAL GetHGlobal();

protected:
	CStringW m_csSeparator;
	CStringW m_csNewText;
};
