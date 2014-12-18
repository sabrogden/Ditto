#pragma once

#include "CGdiPlusBitmap.h"

class CGdiImageDrawer
{
public:
	CGdiImageDrawer();
	~CGdiImageDrawer();

	BOOL LoadStdImage(UINT id, LPCTSTR pType);
	BOOL LoadStdImageDPI(UINT id96, UINT id120, UINT id144, UINT id192, LPCTSTR pType);
	void Draw(CDC* pScreenDC, CWnd *pWnd, int posX, int posY, bool mouseHover, bool mouseDown, int forceWidth = INT_MAX, int forceHeight = INT_MAX);
	BOOL LoadRaw(unsigned char* bitmapData, int imageSize);

	UINT ImageWidth() { return m_pStdImage->m_pBitmap->GetWidth(); }
	UINT ImageHeight() { return m_pStdImage->m_pBitmap->GetHeight(); }

protected:
	CGdiPlusBitmapResource* m_pStdImage;
	CDC*	m_pCurBtn;		// current pointer to one of the above
	CDC		m_dcStd;		// standard button

	//CDC m_dcBk;
};

