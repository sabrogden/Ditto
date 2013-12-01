#include "stdafx.h"
#include "GdiImageDrawer.h"


#include "MemDC.h"


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

void CGdiImageDrawer::Draw(CDC* pScreenDC, int posX, int posY)
{
	//if (m_dcStd.m_hDC == NULL)
	{
		//CBitmap bmp, *pOldBitmap;

		Gdiplus::Graphics graphics(pScreenDC->m_hDC);
		graphics.DrawImage(m_pStdImage->m_pBitmap, posX, posY);


		//m_dcStd.CreateCompatibleDC(pScreenDC);
		//bmp.CreateCompatibleBitmap(pScreenDC, 15, 15);
		//pOldBitmap = m_dcStd.SelectObject(&bmp);
		//m_dcStd.BitBlt(0, 0, 15, 15, pScreenDC, 0, 0, SRCCOPY);
		//bmp.DeleteObject();
	}

	//pScreenDC->BitBlt(0, 0, 15, 15, &m_dcStd, 0, 0, SRCCOPY);
}