// BitmapHelper.cpp: implementation of the CBitmapHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "BitmapHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBitmapHelper::CBitmapHelper()
{

}

CBitmapHelper::~CBitmapHelper()
{

}

int CBitmapHelper::GetCBitmapWidth(const CBitmap & cbm)
{
	BITMAP bm;
	cbm.GetObject(sizeof(BITMAP),&bm);
	return bm.bmWidth;
}

int CBitmapHelper::GetCBitmapHeight(const CBitmap & cbm)
{
	BITMAP bm;
	cbm.GetObject(sizeof(BITMAP),&bm);
	return bm.bmHeight;
} 

BOOL CBitmapHelper::GetCBitmap(void	*pClip2, CDC *pDC, CBitmap *pBitMap, int nMaxHeight)
{
	CClipFormat		*pClip = (CClipFormat *)pClip2;

	if(pClip->m_cfType != CF_DIB &&
		pClip->m_cfType != theApp.m_PNG_Format)
		return false;
	if(!pBitMap)
		return false;
	if (nMaxHeight < 0)
		return false;

	Gdiplus::Bitmap *gdipBitmap = pClip->CreateGdiplusBitmap();
	if (gdipBitmap == NULL)
	{
		return false;
	}

	const UINT gdipHeight = gdipBitmap->GetHeight();
	const UINT gdipWidth = gdipBitmap->GetWidth();
	if (gdipHeight == 0 || gdipWidth == 0) 
	{
		delete gdipBitmap;
		return false;
	}

	const int nHeight = min(nMaxHeight, (int)gdipHeight);
	const int nWidth = (nHeight * gdipWidth) / gdipHeight;

	//do the resize
	pBitMap->CreateCompatibleBitmap(pDC, nWidth, nHeight);
	ASSERT(pBitMap->m_hObject != NULL);

	CDC MemDc2;
	MemDc2.CreateCompatibleDC(pDC);

	CBitmap* oldBitmap2 = MemDc2.SelectObject(pBitMap);

	Gdiplus::Rect dest(0, 0, nWidth, nHeight);
	Gdiplus::ImageAttributes attrs;
	Gdiplus::Graphics graphics(MemDc2);
	
	Gdiplus::InterpolationMode interpolationMode = Gdiplus::InterpolationModeHighQualityBicubic;
	if (CGetSetOptions::GetFastThumbnailMode())
	{
		interpolationMode = Gdiplus::InterpolationModeBicubic;
	}
	graphics.SetInterpolationMode(interpolationMode);
	graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
	graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
	graphics.DrawImage(gdipBitmap, dest, 0, 0, gdipWidth, gdipHeight, Gdiplus::UnitPixel, &attrs);

	MemDc2.SelectObject(oldBitmap2);
	delete gdipBitmap;

	return true;
}

BOOL CBitmapHelper::GetCBitmap(CClipFormats &clips, CDC* pDC, CBitmap* pBitMap, BOOL horizontal)
{
	BOOL bRet = FALSE;
	if (!pBitMap)
		return bRet;

	int count = clips.GetCount();
	int width = 0;
	int height = 0;

	for (int i = 0; i < count; i++)
	{
		CClipFormat clip = clips[i];

		Gdiplus::Bitmap* gdipBitmap = clip.CreateGdiplusBitmap();

		if (horizontal)
		{
			width += (int)gdipBitmap->GetWidth();
			height = max((int)gdipBitmap->GetHeight(), height);
		}
		else
		{
			width = max((int)gdipBitmap->GetWidth(), width);
			height += (int)gdipBitmap->GetHeight();
		}
	
		delete gdipBitmap;
	}

	pBitMap->CreateCompatibleBitmap(pDC, width, height);
	ASSERT(pBitMap->m_hObject != NULL);

	CDC MemDc2;
	MemDc2.CreateCompatibleDC(pDC);

	CBitmap* oldBitmap2 = MemDc2.SelectObject(pBitMap);

	Gdiplus::Graphics graphics(MemDc2);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

	graphics.Clear(Gdiplus::Color::White);

	int destX = 0;
	int destY = 0;

	for (int i = 0; i < count; i++)
	{
		CClipFormat clip = clips[i];

		if (clip.m_cfType != CF_DIB &&
			clip.m_cfType != theApp.m_PNG_Format)
			continue;
	
		Gdiplus::Bitmap* gdipBitmap = clip.CreateGdiplusBitmap();
		if (gdipBitmap == NULL)
			continue;

		const UINT gdipHeight = gdipBitmap->GetHeight();
		const UINT gdipWidth = gdipBitmap->GetWidth();
		if (gdipHeight == 0 || gdipWidth == 0) 
		{
			delete gdipBitmap;
			continue;
		}

		Gdiplus::Rect dest(destX, destY, gdipBitmap->GetWidth(), gdipBitmap->GetHeight());
		Gdiplus::ImageAttributes attrs;
		graphics.DrawImage(gdipBitmap, dest, 0, 0, gdipBitmap->GetWidth(), gdipBitmap->GetHeight(), Gdiplus::UnitPixel, &attrs);

		if (horizontal)
			destX += gdipBitmap->GetWidth();
		else
			destY += gdipBitmap->GetHeight();

		delete gdipBitmap;
		bRet = TRUE;
	}

	MemDc2.SelectObject(oldBitmap2);

	return bRet;
}

WORD CBitmapHelper::PaletteSize(LPSTR lpDIB)
{
    // calculate the size required by the palette
    if (IS_WIN30_DIB (lpDIB))
        return (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
    else
        return (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
}

WORD CBitmapHelper::DIBNumColors(LPSTR lpDIB)
{
    WORD wBitCount;  // DIB bit count

    // If this is a Windows-style DIB, the number of colors in the
    // color table can be less than the number of bits per pixel
    // allows for (i.e. lpbi->biClrUsed can be set to some value).
    // If this is the case, return the appropriate value.
    

    if (IS_WIN30_DIB(lpDIB))
    {
        DWORD dwClrUsed;

        dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
        if (dwClrUsed)

        return (WORD)dwClrUsed;
    }

    // Calculate the number of colors in the color table based on
    // the number of bits per pixel for the DIB.
    
    if (IS_WIN30_DIB(lpDIB))
        wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
    else
        wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

    // return number of colors based on bits per pixel

    switch (wBitCount)
    {
        case 1:
            return 2;

        case 4:
            return 16;

        case 8:
            return 256;

        default:
            return 0;
    }
}


HANDLE CBitmapHelper::hBitmapToDIB(HBITMAP hBitmap, DWORD dwCompression, HPALETTE hPal) 
{
    BITMAP                  bm;
    BITMAPINFOHEADER        bi;
    LPBITMAPINFOHEADER      lpbi;
    DWORD                   dwLen;
    HANDLE                  hDIB;
    HANDLE                  handle;
    HDC                     hDC;

    // The function has no arg for bitfields
    if( dwCompression == BI_BITFIELDS )
            return NULL;

    // If a palette has not been supplied use defaul palette
    if (hPal==NULL)
            hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

    // Get bitmap information
    (void)GetObject( hBitmap, sizeof(bm), (LPSTR)&bm );

    // Initialize the bitmapinfoheader
    bi.biSize               = sizeof(BITMAPINFOHEADER);
    bi.biWidth              = bm.bmWidth;
    bi.biHeight             = bm.bmHeight;
    bi.biPlanes             = 1;
    bi.biBitCount           = static_cast<USHORT>( bm.bmPlanes * bm.bmBitsPixel );
    bi.biCompression        = dwCompression;
    bi.biSizeImage          = 0;
    bi.biXPelsPerMeter      = 0;
    bi.biYPelsPerMeter      = 0;
    bi.biClrUsed            = 0;
    bi.biClrImportant       = 0;

 	dwLen = bi.biSize + PaletteSize((LPSTR)&bi);

    // We need a device context to get the DIB from
    hDC = GetDC(NULL);
    hPal = SelectPalette(hDC, hPal, FALSE);
    (void)RealizePalette(hDC);

    // Allocate enough memory to hold bitmapinfoheader and color table
    hDIB = GlobalAlloc(GMEM_FIXED, dwLen);

    if (!hDIB)
	{
        (void)SelectPalette(hDC, hPal, FALSE);
        ReleaseDC(NULL, hDC);
        return NULL;
    }

    lpbi = (LPBITMAPINFOHEADER)hDIB;

    *lpbi = bi;

    // Call GetDIBits with a NULL lpBits param, so the device driver 
    // will calculate the biSizeImage field 
    (void)GetDIBits(hDC, hBitmap, 0L, (DWORD)bi.biHeight,
                    (LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

    bi = *lpbi;

    // If the driver did not fill in the biSizeImage field, then compute it
    // Each scan line of the image is aligned on a DWORD (32bit) boundary
    if (bi.biSizeImage == 0)
	{
        bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) 
                                        * bi.biHeight;

        // If a compression scheme is used the result may infact be larger
        // Increase the size to account for this.
        if (dwCompression != BI_RGB)
                bi.biSizeImage = (bi.biSizeImage * 3) / 2;
    }

    // Realloc the buffer so that it can hold all the bits
    dwLen += bi.biSizeImage;
	handle = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE);
    if(!handle)
	{
		GlobalFree(hDIB);

		// Reselect the original palette
		(void)SelectPalette(hDC, hPal, FALSE);
		ReleaseDC(NULL, hDC);
		return NULL;
	}
	
	hDIB = handle;

    // Get the bitmap bits
    lpbi = (LPBITMAPINFOHEADER)hDIB;

    // FINALLY get the DIB
    BOOL bGotBits = GetDIBits(hDC, hBitmap,
                            0L,                             // Start scan line
                            (DWORD)bi.biHeight,             // # of scan lines
                            (LPBYTE)lpbi                    // address for bitmap bits
                            + (bi.biSize + PaletteSize((LPSTR)&bi)),
                            (LPBITMAPINFO)lpbi,             // address of bitmapinfo
                            (DWORD)DIB_RGB_COLORS);         // Use RGB for color table

    if( !bGotBits )
    {
        GlobalFree(hDIB);
        
        (void)SelectPalette(hDC, hPal, FALSE);
        ReleaseDC(NULL,hDC);
        return NULL;
    }

    (void)SelectPalette(hDC, hPal, FALSE);
    ReleaseDC(NULL, hDC);
    return hDIB;
}


bool CBitmapHelper::DrawDIB(CDC *pDC, HANDLE hData, int nLeft, int nRight, int &nWidth)
{
	LPBITMAPINFO	lpBI ;
	void*           pDIBBits;
	
	lpBI = (LPBITMAPINFO)GlobalLock(hData);
	if (!lpBI)
		return false;

	int nColors = lpBI->bmiHeader.biClrUsed ? lpBI->bmiHeader.biClrUsed : 1 << lpBI->bmiHeader.biBitCount;

	if( lpBI->bmiHeader.biBitCount > 8 )
	{
		pDIBBits = (LPVOID)((LPDWORD)(lpBI->bmiColors + lpBI->bmiHeader.biClrUsed) + 
			((lpBI->bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
	}
	else
	{
		pDIBBits = (LPVOID)(lpBI->bmiColors + nColors);
	}

	::StretchDIBits(pDC->m_hDC,
				nLeft, nRight, 
				lpBI->bmiHeader.biWidth, lpBI->bmiHeader.biHeight,
				0, 0, lpBI->bmiHeader.biWidth, 
				lpBI->bmiHeader.biHeight,
				pDIBBits, lpBI, DIB_PAL_COLORS, SRCCOPY);

	nWidth = lpBI->bmiHeader.biWidth;

	GlobalUnlock(hData);

	return true;
}