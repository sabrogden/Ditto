#include "stdafx.h"
#include ".\ImageFormatAggregator.h"
#include "Misc.h"
#include "shared/Tokenizer.h"
#include "BitmapHelper.h"

CImageFormatAggregator::CImageFormatAggregator(BOOL horizontally)
{
	m_horizontally = horizontally;
}

CImageFormatAggregator::~CImageFormatAggregator(void)
{
}

bool CImageFormatAggregator::AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType)
{
	HGLOBAL hGlobal = ::NewGlobalP(lpData, nDataSize);
	
	CClipFormat data(cfType, hGlobal);
	//m_images owns the data now
	data.AutoDeleteData(false);

	m_images.Add(data);

	return true;
}

HGLOBAL CImageFormatAggregator::GetHGlobal()
{
	CBitmap bitmap;
	if (CBitmapHelper::GetCBitmap(m_images, CDC::FromHandle(GetDC(GetActiveWindow())), &bitmap, m_horizontally) == FALSE)
	{
		bitmap.DeleteObject();
		return NULL;
	}

	int count = (int)m_images.GetCount();
	for (int i = 0; i < count; i++)
	{
		CClipFormat clip = m_images[i];
		clip.AutoDeleteData(true);
		clip.Free();
	}

	HPALETTE hPal = NULL;
	auto returnHGlobal = CBitmapHelper::hBitmapToDIB((HBITMAP)bitmap, BI_RGB, hPal);

	bitmap.DeleteObject();

	return returnHGlobal;
}