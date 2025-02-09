#pragma once
#include "stdafx.h"
#include "IClipAggregator.h"
#include "Clip.h"

class CImageFormatAggregator : public IClipAggregator
{
public:
	CImageFormatAggregator(BOOL horizontally);
	~CImageFormatAggregator(void);

	virtual bool AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType);
	virtual HGLOBAL GetHGlobal();

protected:
	CClipFormats m_images;
	BOOL m_horizontally;
};

