#include "stdafx.h"
#include "GdiImageDrawer.h"
#include "MemDC.h"
#include "CP_Main.h"

CGdiImageDrawer::CGdiImageDrawer()
{
	m_pStdImage = NULL;
}

CGdiImageDrawer::~CGdiImageDrawer()
{
	delete m_pStdImage;
}

void CGdiImageDrawer::Reset()
{
	delete m_pStdImage;
	m_pStdImage = NULL;

}

BOOL CGdiImageDrawer::LoadStdImage(UINT id, LPCTSTR pType)
{
	m_pStdImage = new CGdiPlusBitmapResource;
	return m_pStdImage->Load(id, pType);
}

BOOL CGdiImageDrawer::LoadRaw(unsigned char* bitmapData, int imageSize)
{
	m_pStdImage = new CGdiPlusBitmapResource;
	return m_pStdImage->LoadRaw(bitmapData, imageSize);
}

BOOL CGdiImageDrawer::LoadStdImageDPI(int dpi, UINT id96, UINT id120, UINT id144, UINT id168, UINT id192, LPCTSTR pType, UINT id225, UINT id250, UINT id275, UINT id300, UINT id325, UINT id350)
{
	BOOL ret = FALSE;

	if (dpi >= 336 && id350 != 0)
	{
		ret = LoadStdImage(id350, pType);
	}
	else if (dpi >= 312 && id325 != 0)
	{
		ret = LoadStdImage(id325, pType);
	}
	else if (dpi >= 288 && id300 != 0)
	{
		ret = LoadStdImage(id300, pType);
	}
	else if (dpi >= 264 && id275 != 0)
	{
		ret = LoadStdImage(id275, pType);
	}
	else if (dpi >= 240 && id250 != 0)
	{
		ret = LoadStdImage(id250, pType);
	}
	else if (dpi >= 216 && id225 != 0)
	{
		ret = LoadStdImage(id225, pType);
	}
	else if (dpi >= 192)
	{
		ret = LoadStdImage(id192, pType);
	}
	else if (dpi >= 168)
	{
		ret = LoadStdImage(id168, pType);
	}
	else if (dpi >= 144)
	{
		ret = LoadStdImage(id144, pType);
	}
	else if (dpi >= 120)
	{
		ret = LoadStdImage(id120, pType);
	}
	else
	{
		ret = LoadStdImage(id96, pType);
	}

	return ret;
}

void CGdiImageDrawer::Draw(CDC* pScreenDC, CDPI &dpi, CWnd *pWnd, CRect rc, bool mouseHover, bool mouseDown)
{
	int width = m_pStdImage->m_pBitmap->GetWidth();
	int height = m_pStdImage->m_pBitmap->GetHeight();

	int x = rc.left + (rc.Width() / 2) - (width / 2);
	int y = rc.top + (rc.Height() / 2) - (height / 2);

	Draw(pScreenDC, dpi, pWnd, x, y, mouseHover, mouseDown);
}

void CGdiImageDrawer::Draw(CDC* pScreenDC, CDPI &dpi, CWnd *pWnd, int posX, int posY, bool mouseHover, bool mouseDown, int forceWidth, int forceHeight)
{
	int width = m_pStdImage->m_pBitmap->GetWidth();
	if (forceWidth != INT_MAX)
		width = forceWidth;
	int height = m_pStdImage->m_pBitmap->GetHeight();
	if (forceHeight != INT_MAX)
		height = forceHeight;

	CRect rectWithBorder(posX, posY, posX + width, posY + height);
	
	CDC dcBk;
	CBitmap bmp;
	CClientDC clDC(pWnd);
		
	//Copy the background over the entire area
	dcBk.CreateCompatibleDC(&clDC);
	bmp.CreateCompatibleBitmap(&clDC, 1, 1);
	dcBk.SelectObject(&bmp);
	dcBk.BitBlt(0, 0, 1, 1, &clDC, rectWithBorder.left-1, rectWithBorder.top, SRCCOPY);
	
	bmp.DeleteObject();		

	//Draw the png file
	if (mouseDown)
	{
		int one = dpi.Scale(1);
		posX += one;
		posY += one;
	}

	//ImageAttributes ia;
	//

	//ColorMap blackToRed;
	//blackToRed.oldColor = Color(255, 110, 114, 122);  // black
	//blackToRed.newColor = Color(255, 255, 0, 0);// red
	//ia.SetRemapTable(1, &blackToRed);

	Gdiplus::Graphics graphics(pScreenDC->m_hDC);
	graphics.DrawImage(*m_pStdImage, posX, posY, width, height);	

	//RectF grect; grect.X = posX, grect.Y = posY; grect.Width = width; grect.Height = height;
	//graphics.DrawImage(*m_pStdImage, grect, 0, 0, width, height, UnitPixel, &ia);


	//If we are hoving over then draw the border
	//if(mouseHover && mouseDown == false)
	//{
	//	pScreenDC->Draw3dRect(rectWithBorder, RGB(255, 255, 255), RGB(255, 255, 255));
	//}
}