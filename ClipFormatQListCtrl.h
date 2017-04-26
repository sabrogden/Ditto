#pragma once

#include "Clip.h"

class CClipFormatQListCtrl : public CClipFormat
{
public:
	CClipFormatQListCtrl(void);
	~CClipFormatQListCtrl(void);

	int m_clipRow;
	bool m_convertedToSmallImage;
	INT64 m_counter;

	HGLOBAL GetDibFittingToHeight(CDC *pDc, int height);
};

