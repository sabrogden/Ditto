#pragma once

#include "CGdiPlusBitmap.h"

class CGdiImageDrawer
{
public:
	CGdiImageDrawer();
	~CGdiImageDrawer();

	BOOL LoadStdImage(UINT id, LPCTSTR pType);
	BOOL LoadStdImageDPI(UINT id96, UINT id120, UINT id144, UINT id192, LPCTSTR pType);
	void CGdiImageDrawer::Draw(CDC* pScreenDC, CWnd *pWnd, int posX, int posY, bool mouseHover, bool mouseDown);

protected:
	CGdiPlusBitmapResource* m_pStdImage;
	CDC*	m_pCurBtn;		// current pointer to one of the above
	CDC		m_dcStd;		// standard button

	//CDC m_dcBk;
};

