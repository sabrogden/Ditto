#pragma once

#include "CGdiPlusBitmap.h"

class CGdiImageDrawer
{
public:
	CGdiImageDrawer();
	~CGdiImageDrawer();

	BOOL LoadStdImage(UINT id, LPCTSTR pType);
	void CGdiImageDrawer::Draw(CDC* pScreenDC, int posX, int posY);

protected:
	CGdiPlusBitmapResource* m_pStdImage;
	CDC*	m_pCurBtn;		// current pointer to one of the above
	CDC		m_dcStd;		// standard button
};

