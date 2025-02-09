#pragma once
#include "IClipAggregator.h"
#include "FileRecieve.h"

class CCF_HDropAggregator : public IClipAggregator
{
public:
	CCF_HDropAggregator(void);
	~CCF_HDropAggregator(void);

	virtual bool AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType);
	virtual HGLOBAL GetHGlobal();
	virtual HGLOBAL GetHGlobalAsString();

protected:
	CFileRecieve m_DropFiles;
};
