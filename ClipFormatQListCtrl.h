#pragma once

#include "Clip.h"

class CClipFormatQListCtrl : public CClipFormat
{
public:
	CClipFormatQListCtrl(void);
	~CClipFormatQListCtrl(void);

	int m_clipRow;
	bool m_convertedToSmallImage;

	HGLOBAL GetDib(CDC *pDc, int height);
};

