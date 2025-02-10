#include "stdafx.h"
#include "ImageHelper.h"

void DIBImageHelper::prependStream(IStream* pIStream, LPVOID pvData, ULONG size)
{
	BITMAPINFO* lpBI = (BITMAPINFO*)pvData;

	int nPaletteEntries = 1 << lpBI->bmiHeader.biBitCount;
	if (lpBI->bmiHeader.biBitCount > 8)
		nPaletteEntries = 0;
	else if (lpBI->bmiHeader.biClrUsed != 0)
		nPaletteEntries = lpBI->bmiHeader.biClrUsed;

	BITMAPFILEHEADER BFH;
	memset(&BFH, 0, sizeof(BITMAPFILEHEADER));
	BFH.bfType = 'MB';
	BFH.bfSize = sizeof(BITMAPFILEHEADER) + size;
	BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nPaletteEntries * sizeof(RGBQUAD);
	pIStream->Write(&BFH, sizeof(BITMAPFILEHEADER), NULL);
}