// BitmapHelper.h: interface for the CBitmapHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAPHELPER_H__641D941B_5487_4F85_BFC1_012F2083A8B6__INCLUDED_)
#define AFX_BITMAPHELPER_H__641D941B_5487_4F85_BFC1_012F2083A8B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

class CBitmapHelper  
{
public:
	CBitmapHelper();
	virtual ~CBitmapHelper();

	static int		GetCBitmapWidth(const CBitmap & cbm);
	static int		GetCBitmapHeight(const CBitmap & cbm);
	static BOOL		GetCBitmap(void	*pClip2, CDC *pDC, CBitmap *pBitMap, int nMaxHeight);
	static HANDLE	hBitmapToDIB(HBITMAP hBitmap, DWORD dwCompression, HPALETTE hPal);
	static WORD		PaletteSize(LPSTR lpDIB);
	static WORD		DIBNumColors(LPSTR lpDIB);

};

#endif // !defined(AFX_BITMAPHELPER_H__641D941B_5487_4F85_BFC1_012F2083A8B6__INCLUDED_)
