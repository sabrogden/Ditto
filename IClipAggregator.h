#pragma once

class IClipAggregator
{
public:
	virtual bool AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType) = 0;
	virtual HGLOBAL GetHGlobal() = 0;
};
