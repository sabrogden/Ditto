#include "stdafx.h"
#include "ClipFormatQListCtrl.h"
#include "BitmapHelper.h"

CClipFormatQListCtrl::CClipFormatQListCtrl(void)
{
	m_counter = 0;
	m_clipRow = -1;
	m_convertedToSmallImage = false;
}

CClipFormatQListCtrl::~CClipFormatQListCtrl(void)
{
}


HGLOBAL CClipFormatQListCtrl::GetDibFittingToHeight(CDC *pDc, int height)
{
	if(m_cfType != CF_DIB)
	{
		return NULL;
	}

	if(m_convertedToSmallImage)
	{
		return m_hgData;
	}

	m_convertedToSmallImage = true;

	CBitmap Bitmap;
	if( !CBitmapHelper::GetCBitmap(this, pDc, &Bitmap, height) )
	{
		Bitmap.DeleteObject();
		// the data is useless, so free it.
		this->Free(); 
		return FALSE;
	}

	this->m_autoDeleteData = true;

	// delete the large image data loaded from the db
	this->Free();

	this->m_autoDeleteData = false;

	//Convert the smaller bitmap back to a dib
	HPALETTE hPal = NULL;
	this->m_hgData = CBitmapHelper::hBitmapToDIB((HBITMAP)Bitmap, BI_RGB, hPal);

	return this->m_hgData;
}
