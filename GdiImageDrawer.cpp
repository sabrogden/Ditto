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

BOOL CGdiImageDrawer::LoadStdImage(UINT id, LPCTSTR pType)
{
	m_pStdImage = new CGdiPlusBitmapResource;
	return m_pStdImage->Load(id, pType);
}

BOOL CGdiImageDrawer::LoadStdImageDPI(UINT id96, UINT id120, UINT id144, UINT id192, LPCTSTR pType)
{
	BOOL ret = FALSE;

	if (theApp.m_metrics.GetDPIX() >= 192)
	{
		ret = LoadStdImage(id192, pType);
	}
	else if (theApp.m_metrics.GetDPIX() >= 144)
	{
		ret = LoadStdImage(id144, pType);
	}
	else if (theApp.m_metrics.GetDPIX() >= 120)
	{
		ret = LoadStdImage(id120, pType);
	}
	else
	{
		ret = LoadStdImage(id96, pType);
	}

	return ret;
}

void CGdiImageDrawer::Draw(CDC* pScreenDC, CWnd *pWnd, int posX, int posY, bool mouseHover, bool mouseDown)
{
	int width = m_pStdImage->m_pBitmap->GetWidth();
	int height = m_pStdImage->m_pBitmap->GetHeight();

	CRect rectWithBorder(posX, posY, posX + width, posY + height);

	int two = theApp.m_metrics.ScaleX(2);
	rectWithBorder.InflateRect(two, two, two, two);

	CDC dcBk;
	CBitmap bmp;
	CClientDC clDC(pWnd);
		
	//Copy the background over the entire area
	dcBk.CreateCompatibleDC(&clDC);
	bmp.CreateCompatibleBitmap(&clDC, 1, 1);
	dcBk.SelectObject(&bmp);
	dcBk.BitBlt(0, 0, 1, 1, &clDC, rectWithBorder.left-1, rectWithBorder.top, SRCCOPY);
	bmp.DeleteObject();
	
	//pScreenDC->StretchBlt(rectWithBorder.left, rectWithBorder.top, rectWithBorder.Width(), rectWithBorder.Height(), &dcBk, 0, 0, 1, 1, SRCCOPY);

	//Draw the png file
	if (mouseDown)
	{
		int one = theApp.m_metrics.ScaleX(1);
		posX += one;
		posY += one;
	}
	Gdiplus::Graphics graphics(pScreenDC->m_hDC);
	graphics.DrawImage(*m_pStdImage, posX, posY, width, height);	

	//If we are hoving over then draw the border
	if(mouseHover && mouseDown == false)
	{
		pScreenDC->Draw3dRect(rectWithBorder, RGB(255, 255, 255), RGB(255, 255, 255));
	}
}