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
	LPBITMAPINFO	lpBI ;
	void*           pDIBBits;
	BOOL bRet = FALSE;

	CClipFormat		*pClip = (CClipFormat *)pClip2;

	switch(pClip->m_cfType)
	{
	case CF_DIB:
	{
		lpBI = (LPBITMAPINFO)GlobalLock(pClip->m_hgData);
		if(lpBI)
		{
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


			int nHeight = min(nMaxHeight, lpBI->bmiHeader.biHeight);
			int nWidth = (nHeight * lpBI->bmiHeader.biWidth) / lpBI->bmiHeader.biHeight;
			
			if(pBitMap)
			{
				pBitMap->CreateCompatibleBitmap(pDC, nWidth, nHeight);

				CDC MemDc;
				MemDc.CreateCompatibleDC(pDC);

				CBitmap* oldBitmap = MemDc.SelectObject(pBitMap);	
			
				::StretchDIBits(MemDc.m_hDC,
					0, 0, 
					nWidth, nHeight,
					0, 0, lpBI->bmiHeader.biWidth, 
					lpBI->bmiHeader.biHeight,
					pDIBBits, lpBI, DIB_PAL_COLORS, SRCCOPY);

				MemDc.SelectObject(oldBitmap);

				bRet = TRUE;
			}
		}
	}
	}

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
    hPal = SelectPalette(hDC,hPal,FALSE);
    (void)RealizePalette(hDC);

    // Allocate enough memory to hold bitmapinfoheader and color table
    hDIB = GlobalAlloc(GMEM_FIXED,dwLen);

    if (!hDIB)
	{
        (void)SelectPalette(hDC,hPal,FALSE);
        ReleaseDC(NULL,hDC);
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
    if(handle)
	{
		hDIB = handle;
	}
    else
	{
		GlobalFree(hDIB);

		// Reselect the original palette
		(void)SelectPalette(hDC,hPal,FALSE);
		ReleaseDC(NULL,hDC);
		return NULL;
    }

    // Get the bitmap bits
    lpbi = (LPBITMAPINFOHEADER)hDIB;

    // FINALLY get the DIB
    BOOL bGotBits = GetDIBits( hDC, hBitmap,
                            0L,                             // Start scan line
                            (DWORD)bi.biHeight,             // # of scan lines
                            (LPBYTE)lpbi                    // address for bitmap bits
                            + (bi.biSize + PaletteSize((LPSTR)&bi)),
                            (LPBITMAPINFO)lpbi,             // address of bitmapinfo
                            (DWORD)DIB_RGB_COLORS);         // Use RGB for color table

    if( !bGotBits )
    {
        GlobalFree(hDIB);
        
        (void)SelectPalette(hDC,hPal,FALSE);
        ReleaseDC(NULL,hDC);
        return NULL;
    }

    (void)SelectPalette(hDC,hPal,FALSE);
    ReleaseDC(NULL,hDC);
    return hDIB;
}
