#pragma once
#include "IClipAggregator.h"

class CRichTextAggregator : public IClipAggregator
{
public:
	CRichTextAggregator(CStringA csSeparator);
	~CRichTextAggregator(void);

	virtual bool AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount);
	virtual HGLOBAL GetHGlobal();

protected:
	CStringA m_csNewText;
	CStringA m_csSeparator;
};
