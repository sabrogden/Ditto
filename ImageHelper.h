#include "stdafx.h"
#include "Misc.h"
#include <memory>

template <class Concrete, class GdipImage>
class ImageHelper abstract
{
public:
	static GdipImage* GdipImageFromHGLOBAL(HGLOBAL hGlobal) {
		IStream* pIStream = StreamFromHGLOBAL(hGlobal);
		if (!pIStream)
			return NULL;

		GdipImage* gdipImage = GdipImage::FromStream(pIStream);
		pIStream->Release();
		return gdipImage;
	};
	static std::shared_ptr<CImage> CImageFromHGLOBAL(HGLOBAL hGlobal) {
		IStream* pIStream = StreamFromHGLOBAL(hGlobal);
		if (!pIStream)
			return NULL;

		std::shared_ptr<CImage> cImage = std::make_shared<CImage>();
		cImage->Load(pIStream);
		pIStream->Release();

		return cImage;
	};
	static IStream* StreamFromHGLOBAL(HGLOBAL hGlobal) {
		if (!hGlobal)
			return NULL;

		IStream* pIStream = NULL;
		if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pIStream) != S_OK)
			return NULL;

		LPVOID pvData = GlobalLock(hGlobal);
		ULONG size = (ULONG)GlobalSize(hGlobal);

		Concrete::prependStream(pIStream, pvData, size);

		pIStream->Write(pvData, size, NULL);
		GlobalUnlock(hGlobal);

		return pIStream;
	};
};

class BitmapImageHelper : public ImageHelper<BitmapImageHelper, Gdiplus::Bitmap>
{
public:
	static void prependStream(IStream* pIStream, LPVOID pvData, ULONG size) {};
};

class PNGImageHelper : public BitmapImageHelper
{
};

class DIBImageHelper : public ImageHelper<DIBImageHelper, Gdiplus::Bitmap>
{
public:
	static void prependStream(IStream* pIStream, LPVOID pvData, ULONG size);
};