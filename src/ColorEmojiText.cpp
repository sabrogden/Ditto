#include "stdafx.h"
#include "ColorEmojiText.h"
#include <atlbase.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace
{
	CComPtr<ID2D1Factory> g_d2dFactory;
	CComPtr<IDWriteFactory> g_writeFactory;
	CComPtr<IDWriteGdiInterop> g_gdiInterop;
	CComPtr<ID2D1DCRenderTarget> g_target;
	bool g_factoriesFailed = false;

	// text format for the last used GDI font, the list draws every row with the same one
	CComPtr<IDWriteTextFormat> g_format;
	LOGFONT g_formatLogFont = {};
	TEXTMETRIC g_formatMetrics = {};

	// Rows without these characters keep the plain GDI path, so ordinary text looks exactly as before.
	bool HasEmoji(const CString& text)
	{
		for (int i = 0; i < text.GetLength(); i++)
		{
			wchar_t c = text[i];
			if (IS_HIGH_SURROGATE(c) ||           // U+1F000 and up: most emoji
				c == 0xFE0F ||                     // emoji presentation selector
				c == 0x200D ||                     // zero width joiner in emoji sequences
				(c >= 0x231A && c <= 0x231B) ||    // watch, hourglass
				(c >= 0x23E9 && c <= 0x23FA) ||    // media buttons, alarm clock, pause
				(c >= 0x25FD && c <= 0x25FE) ||    // small squares
				(c >= 0x2600 && c <= 0x27BF) ||    // misc symbols, dingbats
				(c >= 0x2B00 && c <= 0x2BFF))      // arrows and stars like U+2B50
			{
				return true;
			}
		}
		return false;
	}

	bool CreateResources()
	{
		if (g_target)
			return true;
		if (g_factoriesFailed)
			return false;

		HRESULT hr = S_OK;
		if (!g_d2dFactory)
		{
			hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_d2dFactory.p);
			if (SUCCEEDED(hr))
				hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_writeFactory.p));
			if (SUCCEEDED(hr))
				hr = g_writeFactory->GetGdiInterop(&g_gdiInterop);
			if (FAILED(hr))
			{
				g_factoriesFailed = true;
				return false;
			}
		}

		// software target: the rows are small and GDI owns the DC, a GPU round trip per row costs more than it gives.
		// 96 dpi makes one DIP equal one pixel, so GDI rects and font sizes are used as is.
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_SOFTWARE,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			96.0f, 96.0f);

		hr = g_d2dFactory->CreateDCRenderTarget(&props, &g_target);
		if (FAILED(hr))
		{
			g_factoriesFailed = true;
			return false;
		}

		// GDI classic rendering, so letters in an emoji row look like the GDI rows next to it
		CComPtr<IDWriteRenderingParams> defaults;
		CComPtr<IDWriteRenderingParams> gdiClassic;
		if (SUCCEEDED(g_writeFactory->CreateRenderingParams(&defaults)) &&
			SUCCEEDED(g_writeFactory->CreateCustomRenderingParams(defaults->GetGamma(), defaults->GetEnhancedContrast(),
				defaults->GetClearTypeLevel(), defaults->GetPixelGeometry(), DWRITE_RENDERING_MODE_GDI_CLASSIC, &gdiClassic)))
		{
			g_target->SetTextRenderingParams(gdiClassic);
		}
		return true;
	}

	IDWriteTextFormat* GetTextFormat(CDC* pDC)
	{
		LOGFONT lf = {};
		CFont* font = pDC->GetCurrentFont();
		if (font == NULL || font->GetLogFont(&lf) == 0)
			return NULL;

		TEXTMETRIC tm = {};
		if (pDC->GetTextMetrics(&tm) == FALSE)
			return NULL;

		if (g_format &&
			memcmp(&lf, &g_formatLogFont, sizeof(lf)) == 0 &&
			memcmp(&tm, &g_formatMetrics, sizeof(tm)) == 0)
		{
			return g_format;
		}

		g_format.Release();

		// the GDI face name ("Segoe UI Semibold") is not always a DirectWrite family name, ask GDI interop for the real one
		CComPtr<IDWriteFont> writeFont;
		if (FAILED(g_gdiInterop->CreateFontFromLOGFONT(&lf, &writeFont)))
			return NULL;

		CComPtr<IDWriteFontFamily> family;
		CComPtr<IDWriteLocalizedStrings> names;
		if (FAILED(writeFont->GetFontFamily(&family)) ||
			FAILED(family->GetFamilyNames(&names)))
		{
			return NULL;
		}

		UINT32 index = 0;
		BOOL exists = FALSE;
		if (FAILED(names->FindLocaleName(L"en-us", &index, &exists)) || exists == FALSE)
			index = 0;

		UINT32 length = 0;
		if (FAILED(names->GetStringLength(index, &length)))
			return NULL;

		std::wstring familyName(length + 1, L'\0');
		if (FAILED(names->GetString(index, &familyName[0], length + 1)))
			return NULL;

		// em size the way GDI sees it: cell height without internal leading
		float emSize = static_cast<float>(tm.tmHeight - tm.tmInternalLeading);
		if (emSize <= 0)
			return NULL;

		CComPtr<IDWriteTextFormat> format;
		if (FAILED(g_writeFactory->CreateTextFormat(familyName.c_str(), NULL,
			writeFont->GetWeight(), writeFont->GetStyle(), writeFont->GetStretch(),
			emSize, L"", &format)))
		{
			return NULL;
		}

		// same geometry as GDI DrawText: no wrapping, line height = tmHeight, tab = 8 average chars
		format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		format->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, static_cast<float>(tm.tmHeight), static_cast<float>(tm.tmAscent));
		format->SetIncrementalTabStop(static_cast<float>(8 * tm.tmAveCharWidth));

		g_format = format;
		g_formatLogFont = lf;
		g_formatMetrics = tm;
		return g_format;
	}
}

bool ColorEmojiText::DrawColorText(CDC* pDC, const CString& text, const CRect& rc)
{
	if (rc.IsRectEmpty() || HasEmoji(text) == false || CreateResources() == false)
		return false;

	IDWriteTextFormat* format = GetTextFormat(pDC);
	if (format == NULL)
		return false;

	// GDI-compatible layout: whole-pixel advances, same text widths as GDI DrawText
	CComPtr<IDWriteTextLayout> layout;
	if (FAILED(g_writeFactory->CreateGdiCompatibleTextLayout(text, text.GetLength(), format,
		static_cast<float>(rc.Width()), static_cast<float>(rc.Height()), 1.0f, NULL, FALSE, &layout)))
	{
		return false;
	}

	RECT bindRect = rc;
	if (FAILED(g_target->BindDC(pDC->GetSafeHdc(), &bindRect)))
		return false;

	COLORREF color = pDC->GetTextColor();
	CComPtr<ID2D1SolidColorBrush> brush;
	if (FAILED(g_target->CreateSolidColorBrush(D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f), &brush)))
		return false;

	g_target->BeginDraw();
	g_target->SetTransform(D2D1::Matrix3x2F::Identity());
	g_target->DrawTextLayout(D2D1::Point2F(0, 0), layout, brush,
		D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT | D2D1_DRAW_TEXT_OPTIONS_CLIP);
	HRESULT hr = g_target->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET)
		g_target.Release();

	return SUCCEEDED(hr);
}
