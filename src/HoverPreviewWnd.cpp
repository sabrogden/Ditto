// HoverPreviewWnd.cpp : implementation file

#include "stdafx.h"
#include "HoverPreviewWnd.h"
#include "CP_Main.h"
#include "Clip.h"
#include "Misc.h"
#include "FormattedTextDraw.h"
#include "RichEditCtrlEx.h"
#include "ImageHelper.h"
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Distance between the list row and the popup
#define HOVER_PREVIEW_POPUP_GAP		8
// Border and inner padding of the popup
#define HOVER_PREVIEW_MARGIN		4
// Gap between the preview and the information text
#define HOVER_PREVIEW_TEXT_GAP		4
// Smallest width of the popup, so that the information text stays readable
#define HOVER_PREVIEW_MIN_WIDTH		160
// The information text shows at least this many lines, the configured tool
// tip line count wins when it is bigger
#define HOVER_PREVIEW_MIN_TEXT_LINES	6
// Slack added to a measured text width so that it does not wrap
#define HOVER_PREVIEW_WIDTH_SLACK		8
// Width used for the previews that have no size of their own (rtf, colour,
// html) when the menu is on "Original Size"
#define HOVER_PREVIEW_DEFAULT_WIDTH	400
// A colour swatch is half as high as it is wide, within these limits
#define HOVER_PREVIEW_SWATCH_MIN	48
#define HOVER_PREVIEW_SWATCH_MAX	180

CHoverPreviewWnd* CHoverPreviewWnd::s_pInstance = NULL;

CHoverPreviewWnd::CHoverPreviewWnd()
{
	m_kind = KIND_NONE;
	m_pBitmap = NULL;
	m_pRtfDraw = NULL;
	m_pRtfNormalizer = NULL;
	m_crColor = 0;
	m_nColorAlpha = 255;
	m_lClipId = -1;
}

CHoverPreviewWnd::~CHoverPreviewWnd()
{
	if (m_pBitmap != NULL)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}

	if (m_pRtfDraw != NULL)
	{
		delete m_pRtfDraw;
		m_pRtfDraw = NULL;
	}
}

BEGIN_MESSAGE_MAP(CHoverPreviewWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CHoverPreviewWnd* CHoverPreviewWnd::Get()
{
	if (s_pInstance == NULL)
	{
		s_pInstance = new CHoverPreviewWnd();
	}

	return s_pInstance;
}

bool CHoverPreviewWnd::CreateIfNeeded(CWnd* pOwner)
{
	if (::IsWindow(GetSafeHwnd()))
	{
		return true;
	}

	if (pOwner == NULL || ::IsWindow(pOwner->GetSafeHwnd()) == FALSE)
	{
		return false;
	}

	// CS_SAVEBITS makes hiding and showing the popup cheap, a NULL background
	LPCTSTR lpszClass = AfxRegisterWndClass(CS_SAVEBITS, ::LoadCursor(NULL, IDC_ARROW), NULL, NULL);

	// WS_EX_NOACTIVATE is required: the quick paste window hides itself when
	// it loses the activation, so this popup must never be activated.
	// WS_EX_TOOLWINDOW keeps it out of the task bar and out of alt-tab.
	if (CWnd::CreateEx(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		lpszClass,
		_T(""),
		WS_POPUP,
		0, 0, 0, 0,
		pOwner->GetSafeHwnd(),
		NULL,
		NULL) == FALSE)
	{
		return false;
	}

	m_dpi.SetHwnd(GetSafeHwnd());

	return true;
}

bool CHoverPreviewWnd::LoadImage(long lClipId)
{
	CClipFormat Clip;

	Clip.m_cfType = CF_DIB;
	theApp.GetClipData(lClipId, Clip);

	if (Clip.m_hgData == NULL)
	{
		Clip.m_cfType = theApp.m_PNG_Format;
		theApp.GetClipData(lClipId, Clip);
	}

	Gdiplus::Bitmap* pBitmap = NULL;
	if (Clip.m_hgData != NULL)
	{
		pBitmap = Clip.CreateGdiplusBitmap();
	}

	Clip.Free();
	Clip.Clear();

	if (pBitmap == NULL ||
		pBitmap->GetWidth() == 0 ||
		pBitmap->GetHeight() == 0)
	{
		if (pBitmap != NULL)
		{
			delete pBitmap;
		}

		return false;
	}

	if (m_pBitmap != NULL)
	{
		delete m_pBitmap;
	}

	m_pBitmap = pBitmap;

	return true;
}

bool CHoverPreviewWnd::LoadRtf(long lClipId)
{
	CClipFormat Clip;

	Clip.m_cfType = RegisterClipboardFormat(CF_RTF);
	theApp.GetClipData(lClipId, Clip);

	if (Clip.m_hgData == NULL)
	{
		Clip.Clear();
		return false;
	}

	CStringA rtf = Clip.GetAsCStringA();

	Clip.Free();
	Clip.Clear();

	if (rtf.IsEmpty())
	{
		return false;
	}

	// Some apps, ms word for one, put rtf on the clipboard that the drawer
	// can not handle.  The list cleans that up by pushing the rtf through a
	CString csBetterRTF = rtf;

	if (m_pRtfNormalizer == NULL)
	{
		if (::IsWindow(GetSafeHwnd()))
		{
			m_pRtfNormalizer = new CRichEditCtrlEx;

			if (m_pRtfNormalizer->Create(_T(""), _T(""), WS_CHILD | WS_VSCROLL |
				WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL |
				ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, -1) == FALSE)
			{
				delete m_pRtfNormalizer;
				m_pRtfNormalizer = NULL;
			}
		}
	}

	if (m_pRtfNormalizer != NULL && m_pRtfNormalizer->m_hWnd != NULL)
	{
		m_pRtfNormalizer->SetRTF(rtf);

		CString csCleaned = m_pRtfNormalizer->GetRTF();
		if (csCleaned.IsEmpty() == FALSE)
		{
			csBetterRTF = csCleaned;
		}
	}

	if (m_pRtfDraw == NULL)
	{
		m_pRtfDraw = new CFormattedTextDraw;
		m_pRtfDraw->Create();
	}

	if (m_pRtfDraw == NULL)
	{
		return false;
	}

	CComBSTR bStr(csBetterRTF);

	// put_RTFText fails when the renderer could not be created, in that case
	if (m_pRtfDraw->put_RTFText(bStr) != S_OK)
	{
		return false;
	}

	return true;
}

bool CHoverPreviewWnd::HasClipFormat(long lClipId, CLIPFORMAT cfType)
{
	CClipFormat Clip;

	Clip.m_cfType = cfType;
	theApp.GetClipData(lClipId, Clip);

	const bool bHas = (Clip.m_hgData != NULL);

	Clip.Free();
	Clip.Clear();

	return bHas;
}

// The list stores html text with unprintable markers in it: \x01\x05\x02 is
// a line break and \x01\x04 .... \x02 carries a colour.  DrawText would show
void CHoverPreviewWnd::CleanDisplayText(CString& csText) const
{
	CString csCleaned;

	for (int i = 0; i < csText.GetLength(); i++)
	{
		const TCHAR c = csText[i];

		if (c == _T('\x01'))
		{
			CString csMarker;

			i++;
			while (i < csText.GetLength() && csText[i] != _T('\x02'))
			{
				csMarker += csText[i];
				i++;
			}

			if (csMarker.Find(_T('\x05')) >= 0)
			{
				csCleaned += _T("\r\n");
			}

			continue;
		}

		if (c == _T('\x02'))
		{
			continue;
		}

		if (c < _T(' ') && c != _T('\r') && c != _T('\n') && c != _T('\t'))
		{
			continue;
		}

		csCleaned += c;
	}

	while (csCleaned.Replace(_T("\r\n\r\n\r\n"), _T("\r\n\r\n")) > 0)
	{
	}

	csCleaned.TrimRight();
	csText = csCleaned;
}

bool CHoverPreviewWnd::LoadContent(long lClipId, const CString& csInfo, bool bHasColorCode, COLORREF crColorCode, int nColorAlpha)
{
	m_kind = KIND_NONE;
	m_lClipId = lClipId;
	m_csInfoExtra.Empty();

	if (LoadImage(lClipId))
	{
		m_kind = KIND_IMAGE;
		BuildImageInfo(lClipId);
		return true;
	}

	if (LoadRtf(lClipId))
	{
		m_kind = KIND_RTF;
		return true;
	}

	if (bHasColorCode)
	{
		m_kind = KIND_COLOR;
		m_crColor = crColorCode;
		m_nColorAlpha = nColorAlpha;
		return true;
	}

	if (HasClipFormat(lClipId, GetFormatID(_T("HTML Format"))))
	{
		// the preview of html is the clip text with the internal markers
		// turned into line breaks, Layout picks that up from the clip text so
		CString csHtml = csInfo;
		CleanDisplayText(csHtml);

		if (csHtml.IsEmpty() == FALSE)
		{
			m_kind = KIND_HTML;
			return true;
		}
	}

	if (csInfo.IsEmpty() == FALSE)
	{
		m_kind = KIND_TEXT;
		return true;
	}

	return false;
}

void CHoverPreviewWnd::UpdateFont()
{
	LOGFONT lf;
	CGetSetOptions::GetFont(lf);

	lf.lfHeight = m_dpi.Scale(lf.lfHeight);

	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf);
}

int CHoverPreviewWnd::MeasureTextHeight(const CString& csText, int nWidth, int nMaxHeight)
{
	if (csText.IsEmpty() || nWidth <= 0)
	{
		return 0;
	}

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&m_font);

	CRect rcMeasure(0, 0, nWidth, 0);
	dc.DrawText(csText, &rcMeasure, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EXPANDTABS | DT_CALCRECT);

	dc.SelectObject(pOldFont);

	return min(rcMeasure.Height(), nMaxHeight);
}

// Width of the longest line of the text, used to make the popup as narrow as
int CHoverPreviewWnd::MeasureTextWidth(const CString& csText)
{
	if (csText.IsEmpty())
	{
		return 0;
	}

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&m_font);

	int nWidth = 0;
	int nStart = 0;

	while (nStart <= csText.GetLength())
	{
		int nEnd = csText.Find(_T('\n'), nStart);
		if (nEnd < 0)
		{
			nEnd = csText.GetLength();
		}

		CString csLine = csText.Mid(nStart, nEnd - nStart);
		csLine.TrimRight(_T('\r'));
		csLine.Replace(_T("\t"), _T("    "));

		if (csLine.IsEmpty() == FALSE)
		{
			CSize size = dc.GetTextExtent(csLine);
			nWidth = max(nWidth, (int)size.cx);
		}

		nStart = nEnd + 1;
	}

	dc.SelectObject(pOldFont);

	return nWidth;
}

void CHoverPreviewWnd::WrapTextToWidth(CString& csText, int nWidth)
{
	if (csText.IsEmpty() || nWidth <= 0)
	{
		return;
	}

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&m_font);

	const int nLength = csText.GetLength();
	CString csWrapped;
	int nStart = 0;

	while (nStart < nLength)
	{
		int nEnd = nStart;
		int nSpace = -1;

		while (nEnd < nLength)
		{
			const TCHAR c = csText[nEnd];

			if (c == _T('\r') || c == _T('\n'))
			{
				break;
			}

			if (c == _T(' ') || c == _T('\t'))
			{
				nSpace = nEnd;
			}

			if (nEnd > nStart && dc.GetTextExtent(csText.Mid(nStart, nEnd - nStart + 1)).cx > nWidth)
			{
				break;
			}

			nEnd++;
		}

		if (nEnd >= nLength || csText[nEnd] == _T('\r') || csText[nEnd] == _T('\n'))
		{
			csWrapped += csText.Mid(nStart, nEnd - nStart);

			while (nEnd < nLength && (csText[nEnd] == _T('\r') || csText[nEnd] == _T('\n')))
			{
				csWrapped += csText[nEnd];
				nEnd++;
			}

			nStart = nEnd;
			continue;
		}

		// break the line, at the last space when there is one, otherwise in
		const int nBreak = (nSpace > nStart) ? nSpace : nEnd;

		csWrapped += csText.Mid(nStart, nBreak - nStart);
		csWrapped += _T("\r\n");

		nStart = (nSpace > nStart) ? (nBreak + 1) : nBreak;
	}

	dc.SelectObject(pOldFont);

	csText = csWrapped;
}

void CHoverPreviewWnd::ScaleBitmapTo(int nWidth, int nHeight)
{
	if (m_pBitmap == NULL || nWidth <= 0 || nHeight <= 0)
	{
		return;
	}

	if ((int)m_pBitmap->GetWidth() <= nWidth && (int)m_pBitmap->GetHeight() <= nHeight)
	{
		return;
	}

	Gdiplus::Bitmap* pScaled = new Gdiplus::Bitmap(nWidth, nHeight, PixelFormat32bppPARGB);
	if (pScaled == NULL)
	{
		return;
	}

	{
		Gdiplus::Graphics graphics(pScaled);
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
		graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);

		graphics.DrawImage(m_pBitmap,
			Gdiplus::Rect(0, 0, nWidth, nHeight),
			0, 0,
			(INT)m_pBitmap->GetWidth(), (INT)m_pBitmap->GetHeight(),
			Gdiplus::UnitPixel);
	}

	delete m_pBitmap;
	m_pBitmap = pScaled;
}

//---------------------------------------------------------------------------
// exif
static CString GetExifText(Gdiplus::Bitmap* pBitmap, PROPID propId)
{
	CString csValue;

	if (pBitmap == NULL)
	{
		return csValue;
	}

	const UINT nSize = pBitmap->GetPropertyItemSize(propId);
	if (nSize == 0)
	{
		return csValue;
	}

	Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*)malloc(nSize);
	if (pItem == NULL)
	{
		return csValue;
	}

	if (pBitmap->GetPropertyItem(propId, nSize, pItem) == Gdiplus::Ok &&
		pItem->type == PropertyTagTypeASCII &&
		pItem->value != NULL &&
		pItem->length > 1)
	{
		csValue = CString((const char*)pItem->value);
		csValue.Trim();
	}

	free(pItem);

	return csValue;
}

static bool GetExifNumber(Gdiplus::Bitmap* pBitmap, PROPID propId, double& dValue)
{
	bool bFound = false;

	if (pBitmap == NULL)
	{
		return false;
	}

	const UINT nSize = pBitmap->GetPropertyItemSize(propId);
	if (nSize == 0)
	{
		return false;
	}

	Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*)malloc(nSize);
	if (pItem == NULL)
	{
		return false;
	}

	if (pBitmap->GetPropertyItem(propId, nSize, pItem) == Gdiplus::Ok && pItem->value != NULL)
	{
		if (pItem->type == PropertyTagTypeRational && pItem->length >= 8)
		{
			const long* pValues = (const long*)pItem->value;
			if (pValues[1] != 0)
			{
				dValue = (double)pValues[0] / (double)pValues[1];
				bFound = true;
			}
		}
		else if (pItem->type == PropertyTagTypeShort && pItem->length >= 2)
		{
			dValue = (double)*(const unsigned short*)pItem->value;
			bFound = true;
		}
	}

	free(pItem);

	return bFound;
}

bool CHoverPreviewWnd::LoadExif(long lClipId, CString& csText)
{
	CClipFormat Clip;

	Clip.m_cfType = GetFormatID(_T("JFIF"));
	theApp.GetClipData(lClipId, Clip);

	if (Clip.m_hgData == NULL)
	{
		Clip.Clear();
		return false;
	}

	Gdiplus::Bitmap* pBitmap = BitmapImageHelper::GdipImageFromHGLOBAL(Clip.m_hgData);

	Clip.Free();
	Clip.Clear();

	if (pBitmap == NULL)
	{
		return false;
	}

	CString csLines;

	CString csMake = GetExifText(pBitmap, PropertyTagEquipMake);
	CString csModel = GetExifText(pBitmap, PropertyTagEquipModel);

	if (csMake.IsEmpty() == FALSE || csModel.IsEmpty() == FALSE)
	{
		CString csCamera = csMake;

		if (csCamera.IsEmpty() == FALSE && csModel.IsEmpty() == FALSE)
		{
			csCamera += _T(" ");
		}

		csCamera += csModel;
		csLines += csCamera;
	}

	CString csTaken = GetExifText(pBitmap, PropertyTagExifDTOrig);
	if (csTaken.IsEmpty())
	{
		csTaken = GetExifText(pBitmap, PropertyTagDateTime);
	}

	if (csTaken.IsEmpty() == FALSE)
	{
		if (csLines.IsEmpty() == FALSE)
		{
			csLines += _T("\r\n");
		}

		csLines += csTaken;
	}

	CString csValues;
	CString csPart;
	double dValue = 0;

	if (GetExifNumber(pBitmap, PropertyTagExifExposureTime, dValue) && dValue > 0)
	{
		if (dValue < 1.0)
		{
			csPart.Format(_T("1/%.0f s"), 1.0 / dValue);
		}
		else
		{
			csPart.Format(_T("%.1f s"), dValue);
		}

		csValues += csPart;
	}

	if (GetExifNumber(pBitmap, PropertyTagExifFNumber, dValue) && dValue > 0)
	{
		csPart.Format(_T("f/%.1f"), dValue);

		if (csValues.IsEmpty() == FALSE)
		{
			csValues += _T("  ");
		}

		csValues += csPart;
	}

	if (GetExifNumber(pBitmap, PropertyTagExifISOSpeed, dValue) && dValue > 0)
	{
		csPart.Format(_T("ISO %.0f"), dValue);

		if (csValues.IsEmpty() == FALSE)
		{
			csValues += _T("  ");
		}

		csValues += csPart;
	}

	if (GetExifNumber(pBitmap, PropertyTagExifFocalLength, dValue) && dValue > 0)
	{
		csPart.Format(_T("%.0f mm"), dValue);

		if (csValues.IsEmpty() == FALSE)
		{
			csValues += _T("  ");
		}

		csValues += csPart;
	}

	if (csValues.IsEmpty() == FALSE)
	{
		if (csLines.IsEmpty() == FALSE)
		{
			csLines += _T("\r\n");
		}

		csLines += csValues;
	}

	delete pBitmap;

	if (csLines.IsEmpty())
	{
		return false;
	}

	csText = csLines;

	return true;
}

void CHoverPreviewWnd::BuildImageInfo(long lClipId)
{
	m_csInfoExtra.Empty();

	if (m_pBitmap == NULL)
	{
		return;
	}

	CString csInfo;
	csInfo.Format(_T("%u x %u"), m_pBitmap->GetWidth(), m_pBitmap->GetHeight());

	if (HasClipFormat(lClipId, theApp.m_PNG_Format))
	{
		csInfo += _T("  PNG");
	}
	else
	{
		csInfo += _T("  DIB");
	}

	m_csInfoExtra = csInfo;

	CString csExif;
	if (LoadExif(lClipId, csExif))
	{
		m_csInfoExtra += _T("\r\n");
		m_csInfoExtra += csExif;
	}
}

void CHoverPreviewWnd::Layout(const CString& csInfo, const CRect& rcAnchor)
{
	UpdateFont();

	// The information text is the very same text the tool tip bubble shows,
	// the parent already caps it at the configured line count, so keep it
	// whole.  Html text has unprintable markers in it, clean those up for
	m_csClipText = csInfo;
	CleanDisplayText(m_csClipText);
	m_csClipText.TrimRight();

	m_csDetails.Empty();

	int nDetailsStart = m_csClipText.Find(_T("Database ID:"));
	if (nDetailsStart > 0)
	{
		m_csDetails = m_csClipText.Mid(nDetailsStart);
		m_csClipText = m_csClipText.Left(nDetailsStart);
		m_csClipText.TrimRight();
	}

	if (m_csInfoExtra.IsEmpty() == FALSE)
	{
		if (m_csDetails.IsEmpty() == FALSE)
		{
			m_csDetails += _T("\r\n");
		}

		m_csDetails += m_csInfoExtra;
	}

	if (m_kind == KIND_TEXT || m_kind == KIND_HTML)
	{
		m_csHtmlText = m_csClipText;
	}

	// what the line below the preview shows
	if (m_kind == KIND_IMAGE)
	{
		m_csInfo = csInfo;
		CleanDisplayText(m_csInfo);
		m_csInfo.TrimRight();

		if (m_csInfoExtra.IsEmpty() == FALSE)
		{
			if (m_csInfo.IsEmpty() == FALSE)
			{
				m_csInfo += _T("\r\n");
			}

			m_csInfo += m_csInfoExtra;
		}
	}
	else if (m_kind == KIND_COLOR)
	{
		CString csCode = m_csClipText;
		int nLineEnd = csCode.Find(_T("\r\n"));
		if (nLineEnd > 0)
		{
			csCode = csCode.Left(nLineEnd);
		}

		m_csInfo = csCode;

		if (m_csDetails.IsEmpty() == FALSE)
		{
			if (m_csInfo.IsEmpty() == FALSE)
			{
				m_csInfo += _T("\r\n");
			}

			m_csInfo += m_csDetails;
		}
	}
	else
	{
		// text previews only need the details below them
		m_csInfo = m_csDetails;
	}

	const int nMargin = m_dpi.Scale(HOVER_PREVIEW_MARGIN);
	const int nGap = m_dpi.Scale(HOVER_PREVIEW_TEXT_GAP);
	const int nPopupGap = m_dpi.Scale(HOVER_PREVIEW_POPUP_GAP);
	const int nMinWidth = m_dpi.Scale(HOVER_PREVIEW_MIN_WIDTH);

	CRect rcWork = MonitorRectFromRect(CRect(rcAnchor.left, rcAnchor.top, rcAnchor.left + 1, rcAnchor.top + 1));

	const int nWidth = CGetSetOptions::GetToolTipPreviewWidth();
	const int nMaxContentHeight = (rcWork.Height() * 3) / 4;
	const int nMaxContentWidth = rcWork.Width() - (2 * nMargin);

	int nContentWidth = 0;
	int nContentHeight = 0;
	int nImageWidth = 0;
	int nImageHeight = 0;

	if (m_kind == KIND_IMAGE && m_pBitmap != NULL)
	{
		int nMaxImageWidth = 0;
		int nMaxImageHeight = 0;

		if (nWidth <= 0)
		{
			// "Original size": the image is not scaled at all, it is only
			nMaxImageWidth = nMaxContentWidth;
			nMaxImageHeight = rcWork.Height() - (2 * nMargin);
		}
		else
		{
			// the width picked in the menu is the box the image has to fit
			nMaxImageWidth = m_dpi.Scale(nWidth);
			nMaxImageHeight = min(m_dpi.Scale(nWidth), nMaxContentHeight);
		}

		const double dBitmapWidth = (double)m_pBitmap->GetWidth();
		const double dBitmapHeight = (double)m_pBitmap->GetHeight();

		if (dBitmapWidth > 0 && dBitmapHeight > 0)
		{
			double dScale = min(nMaxImageWidth / dBitmapWidth, nMaxImageHeight / dBitmapHeight);

			// The width picked in the menu is the box a large image has to fit
			// in, in both directions.  An image that already fits in the box
			dScale = min(dScale, 1.0);

			nImageWidth = (int)((dBitmapWidth * dScale) + 0.5);
			nImageHeight = (int)((dBitmapHeight * dScale) + 0.5);

			// decode straight to the size that gets drawn: keeping the full
			ScaleBitmapTo(nImageWidth, nImageHeight);
		}

		// The image is drawn at its own size inside the content box, so the box
		// only has to be as wide as the widest thing that gets drawn: the image
		int nInfoWidth = MeasureTextWidth(m_csInfo);

		if (nInfoWidth > 0)
		{
			nInfoWidth = min(nInfoWidth + m_dpi.Scale(HOVER_PREVIEW_WIDTH_SLACK), nMaxImageWidth);
		}

		nContentWidth = max(nImageWidth, nInfoWidth);
		nContentHeight = nImageHeight;
	}
	else
	{
		// rtf, html, colour swatch and plain text have no size of their own,
		int nTierWidth = (nWidth > 0) ? m_dpi.Scale(nWidth) : m_dpi.Scale(HOVER_PREVIEW_DEFAULT_WIDTH);
		nTierWidth = min(nTierWidth, nMaxContentWidth);

		CString csContentText = m_csClipText;

		if (m_kind == KIND_HTML)
		{
			csContentText = m_csHtmlText;
		}
		else if (m_kind == KIND_COLOR)
		{
			int nLineEnd = csContentText.Find(_T("\r\n"));
			if (nLineEnd > 0)
			{
				csContentText = csContentText.Left(nLineEnd);
			}
		}

		// The width is measured from the widest line that gets drawn: the
		// preview itself and, if there is one, the information below it (the
		int nNaturalWidth = 0;

		if (m_kind == KIND_HTML)
		{
			nNaturalWidth = MeasureTextWidth(m_csHtmlText);
		}
		else if (m_kind == KIND_COLOR)
		{
			CString csCode = m_csClipText;
			int nLineEnd = csCode.Find(_T("\r\n"));
			if (nLineEnd > 0)
			{
				csCode = csCode.Left(nLineEnd);
			}

			nNaturalWidth = MeasureTextWidth(csCode);
		}
		else
		{
			nNaturalWidth = MeasureTextWidth(m_csClipText);
		}

		nNaturalWidth = max(nNaturalWidth, MeasureTextWidth(m_csInfo));

		if (m_kind == KIND_RTF)
		{
			// rich text lays itself out with its own fonts, which are usually
			// wider than the plain text measurement, and the renderer knows
			nNaturalWidth += ((nNaturalWidth * 30) / 100);

			long lWantedWidth = 0;

			if (m_pRtfDraw != NULL)
			{
				m_pRtfDraw->get_NaturalWidth(10000, &lWantedWidth);
			}

			if (lWantedWidth > nNaturalWidth && lWantedWidth < nTierWidth)
			{
				nNaturalWidth = (int)lWantedWidth;
			}
		}

		if (nNaturalWidth <= 0)
		{
			nContentWidth = nTierWidth;
		}
		else
		{
			nContentWidth = nNaturalWidth + m_dpi.Scale(HOVER_PREVIEW_WIDTH_SLACK);
			nContentWidth = max(nContentWidth, nMinWidth);
			nContentWidth = min(nContentWidth, nTierWidth);
		}

		// wrap the text before it is measured: a line with a long word turns
		if (m_kind == KIND_TEXT || m_kind == KIND_HTML)
		{
			WrapTextToWidth(m_csHtmlText, nContentWidth);
			csContentText = m_csHtmlText;
		}

		switch (m_kind)
		{
		case KIND_COLOR:
		{
			nContentHeight = nContentWidth / 2;
			nContentHeight = max(nContentHeight, m_dpi.Scale(HOVER_PREVIEW_SWATCH_MIN));
			nContentHeight = min(nContentHeight, m_dpi.Scale(HOVER_PREVIEW_SWATCH_MAX));
		}
		break;

		case KIND_RTF:
		{
			// the plain text lays out about like the rich text does, but with
			int nHeight = MeasureTextHeight(csContentText, nContentWidth, nMaxContentHeight);
			nHeight += ((nHeight * 40) / 100) + m_dpi.Scale(8);

			long lNatural = 0;

			if (m_pRtfDraw != NULL)
			{
				m_pRtfDraw->get_NaturalHeight(nContentWidth, &lNatural);
			}

			// the renderer may ask for more room, but it must never make the
			if (lNatural > nHeight && lNatural <= nMaxContentHeight)
			{
				nHeight = (int)lNatural;
			}

			nContentHeight = max(min(nHeight, nMaxContentHeight), m_dpi.Scale(20));
		}
		break;

		case KIND_HTML:
		case KIND_TEXT:
		{
			nContentHeight = max(MeasureTextHeight(csContentText, nContentWidth, nMaxContentHeight), m_dpi.Scale(20));
		}
		break;

		default:
			nContentHeight = m_dpi.Scale(20);
			break;
		}
	}

	m_rcContent.SetRect(nMargin, nMargin, nMargin + nContentWidth, nMargin + nContentHeight);

	// wrap what is drawn, DrawText alone would cut a long word off at the
	WrapTextToWidth(m_csInfo, nContentWidth);

	int nTextHeight = 0;
	int nDetailsGap = nGap;

	if (m_csInfo.IsEmpty() == FALSE)
	{
		TEXTMETRIC tm;
		{
			CClientDC dc(this);
			CFont* pOldFont = dc.SelectObject(&m_font);
			dc.GetTextMetrics(&tm);
			dc.SelectObject(pOldFont);
		}

		// a text preview and the information below it are both text, so keep
		if (m_kind != KIND_IMAGE && m_kind != KIND_COLOR)
		{
			nDetailsGap = nGap + (2 * tm.tmHeight);
		}

		int nMaxTextHeight = rcWork.Height() - (2 * nMargin) - nContentHeight - nDetailsGap;
		nMaxTextHeight = max(nMaxTextHeight, tm.tmHeight * 3);

		const int nMaxLineHeight = tm.tmHeight * max(HOVER_PREVIEW_MIN_TEXT_LINES, CGetSetOptions::GetMaxToolTipLines());

		nTextHeight = MeasureTextHeight(m_csInfo, nContentWidth, min(nMaxTextHeight, nMaxLineHeight));
	}

	if (nTextHeight > 0)
	{
		m_rcText.SetRect(nMargin,
			nMargin + nContentHeight + nDetailsGap,
			nMargin + nContentWidth,
			nMargin + nContentHeight + nDetailsGap + nTextHeight);
	}
	else
	{
		m_rcText.SetRectEmpty();
	}

	const int nPopupWidth = nContentWidth + (2 * nMargin);
	const int nPopupHeight = nMargin + nContentHeight + (nTextHeight > 0 ? (nDetailsGap + nTextHeight) : 0) + nMargin;

	// Show up next to the mouse pointer, flipped to the other side of the
	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	int x = ptCursor.x + nPopupGap;
	if (x + nPopupWidth > rcWork.right - nMargin)
	{
		x = ptCursor.x - nPopupWidth - nPopupGap;
	}
	if (x < rcWork.left + nMargin)
	{
		x = rcWork.left + nMargin;
	}

	int y = ptCursor.y + nPopupGap;
	if (y + nPopupHeight > rcWork.bottom - nMargin)
	{
		y = ptCursor.y - nPopupHeight - nPopupGap;
	}
	if (y < rcWork.top + nMargin)
	{
		y = rcWork.top + nMargin;
	}

	// The popup must never come up under the mouse pointer: the list control
	// gets a mouse leave when the pointer moves onto the popup, which closes
	// the popup, which puts the pointer back over the list, and so on.
	CRect rcPopup(x, y, x + nPopupWidth, y + nPopupHeight);

	if (rcPopup.PtInRect(ptCursor))
	{
		if (ptCursor.x - nPopupWidth - nPopupGap >= rcWork.left + nMargin)
		{
			x = ptCursor.x - nPopupWidth - nPopupGap;
		}
		else if (ptCursor.y - nPopupHeight - nPopupGap >= rcWork.top + nMargin)
		{
			y = ptCursor.y - nPopupHeight - nPopupGap;
		}
		else if (ptCursor.y + nPopupGap + nPopupHeight <= rcWork.bottom - nMargin)
		{
			x = ptCursor.x + nPopupGap;
			y = ptCursor.y + nPopupGap;
		}
	}

	SetWindowPos(&wndTopMost, x, y, nPopupWidth, nPopupHeight, SWP_NOACTIVATE | SWP_SHOWWINDOW);
	Invalidate(FALSE);
}

bool CHoverPreviewWnd::ShowForClip(CWnd* pOwner, long lClipId, const CString& csInfo, const CRect& rcAnchor,
	bool bHasColorCode, COLORREF crColorCode, int nColorAlpha)
{
	CHoverPreviewWnd* pWnd = Get();

	if (pWnd->CreateIfNeeded(pOwner) == false)
	{
		return false;
	}

	if (pWnd->LoadContent(lClipId, csInfo, bHasColorCode, crColorCode, nColorAlpha) == false)
	{
		HideIfVisible();
		return false;
	}

	pWnd->Layout(csInfo, rcAnchor);

	return true;
}

void CHoverPreviewWnd::ReleaseContent()
{
	if (m_pBitmap != NULL)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}

	m_kind = KIND_NONE;
	m_lClipId = -1;
	m_csHtmlText.Empty();
	m_csClipText.Empty();
	m_csDetails.Empty();
	m_csInfo.Empty();
	m_csInfoExtra.Empty();
	m_rcContent.SetRectEmpty();
	m_rcText.SetRectEmpty();
}

void CHoverPreviewWnd::HideIfVisible()
{
	if (s_pInstance == NULL)
	{
		return;
	}

	if (::IsWindow(s_pInstance->GetSafeHwnd()) == FALSE)
	{
		return;
	}

	if (::IsWindowVisible(s_pInstance->GetSafeHwnd()))
	{
		s_pInstance->ShowWindow(SW_HIDE);
		s_pInstance->ReleaseContent();
	}
}

bool CHoverPreviewWnd::IsShowingClip(long lClipId)
{
	if (s_pInstance == NULL || lClipId <= 0)
	{
		return false;
	}

	if (::IsWindow(s_pInstance->GetSafeHwnd()) == FALSE ||
		::IsWindowVisible(s_pInstance->GetSafeHwnd()) == FALSE)
	{
		return false;
	}

	return s_pInstance->m_lClipId == lClipId;
}

BOOL CHoverPreviewWnd::OnEraseBkgnd(CDC* pDC)
{
	// OnPaint fills the whole client area
	return TRUE;
}

void CHoverPreviewWnd::DrawCheckerboard(CDC* pDC, const CRect& rc)
{
	const int nSquare = 8;
	int nIndex = 0;

	for (int y = rc.top; y < rc.bottom; y += nSquare)
	{
		for (int x = rc.left; x < rc.right; x += nSquare, nIndex++)
		{
			CRect rcSquare(x, y, min(x + nSquare, rc.right), min(y + nSquare, rc.bottom));

			pDC->FillSolidRect(rcSquare, ((nIndex % 2) == 0) ? RGB(255, 255, 255) : RGB(204, 204, 204));
		}

		nIndex++;
	}
}

void CHoverPreviewWnd::OnPaint()
{
	CPaintDC dc(this);

	CRect rcClient;
	GetClientRect(&rcClient);

	dc.FillSolidRect(rcClient, CGetSetOptions::m_Theme.DescriptionWindowBG());

	CFont* pOldFont = dc.SelectObject(&m_font);
	dc.SetBkMode(TRANSPARENT);

	switch (m_kind)
	{
	case KIND_IMAGE:
	{
		if (m_pBitmap != NULL && m_rcContent.IsRectEmpty() == FALSE)
		{
			Gdiplus::Graphics graphics(dc.GetSafeHdc());
			graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
			graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
			graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);

			// Draw it at its own size, centred in the content box.  The box is
			// only wider than the image when the image is narrower than the
			const int nDrawWidth = min((int)m_pBitmap->GetWidth(), m_rcContent.Width());
			const int nDrawHeight = min((int)m_pBitmap->GetHeight(), m_rcContent.Height());
			const int nDrawLeft = m_rcContent.left + ((m_rcContent.Width() - nDrawWidth) / 2);

			Gdiplus::ImageAttributes attrs;
			Gdiplus::Rect dest(nDrawLeft, m_rcContent.top, nDrawWidth, nDrawHeight);

			graphics.DrawImage(m_pBitmap,
				dest,
				0, 0,
				(INT)m_pBitmap->GetWidth(), (INT)m_pBitmap->GetHeight(),
				Gdiplus::UnitPixel,
				&attrs);
		}
	}
	break;

	case KIND_RTF:
	{
		if (m_pRtfDraw != NULL && m_rcContent.IsRectEmpty() == FALSE)
		{
			RECT rc = m_rcContent;
			m_pRtfDraw->Draw(dc.GetSafeHdc(), &rc);
		}
	}
	break;

	case KIND_COLOR:
	{
		if (m_rcContent.IsRectEmpty() == FALSE)
		{
			if (m_nColorAlpha < 255)
			{
				// let the checkerboard show through, the same way the list
				DrawCheckerboard(&dc, m_rcContent);

				Gdiplus::Graphics graphics(dc.GetSafeHdc());
				Gdiplus::Color gdiplusColor(m_nColorAlpha, GetRValue(m_crColor), GetGValue(m_crColor), GetBValue(m_crColor));
				Gdiplus::SolidBrush brush(gdiplusColor);
				graphics.FillRectangle(&brush, Gdiplus::Rect(m_rcContent.left, m_rcContent.top, m_rcContent.Width(), m_rcContent.Height()));
			}
			else
			{
				dc.FillSolidRect(m_rcContent, m_crColor);
			}

			dc.Draw3dRect(m_rcContent, CGetSetOptions::m_Theme.Border(), CGetSetOptions::m_Theme.Border());
		}
	}
	break;

	case KIND_HTML:
	case KIND_TEXT:
	{
		if (m_csHtmlText.IsEmpty() == FALSE && m_rcContent.IsRectEmpty() == FALSE)
		{
			dc.SetTextColor(CGetSetOptions::m_Theme.DescriptionWindowText());
			dc.DrawText(m_csHtmlText, &m_rcContent, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EXPANDTABS);
		}
	}
	break;

	default:
		break;
	}

	if (m_csInfo.IsEmpty() == FALSE && m_rcText.IsRectEmpty() == FALSE)
	{
		// the rtf renderer draws straight into this dc and can leave its own
		// font and colours selected, so put ours back first
		dc.SelectObject(&m_font);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(CGetSetOptions::m_Theme.DescriptionWindowText());
		dc.DrawText(m_csInfo, &m_rcText, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL);
	}

	dc.SelectObject(pOldFont);

	dc.Draw3dRect(rcClient, CGetSetOptions::m_Theme.Border(), CGetSetOptions::m_Theme.Border());
}
