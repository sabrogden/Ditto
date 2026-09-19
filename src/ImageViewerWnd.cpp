#include "stdafx.h"
#include "ImageViewerWnd.h"
#include "CP_Main.h"
#include "Misc.h"
#include "ImageHelper.h"
#include "Sqlite\CppSQLite3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CImageViewerWnd* CImageViewerWnd::s_pInstance = NULL;

BEGIN_MESSAGE_MAP(CImageViewerWnd, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CImageViewerWnd::CImageViewerWnd() :
	m_pImage(NULL),
	m_bClosing(false)
{
	m_hintFont.CreatePointFont(100, _T("Segoe UI"));
}

CImageViewerWnd::~CImageViewerWnd()
{
	if (m_pImage)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
}

bool CImageViewerWnd::ShowForClip(int clipId, CWnd* pRefWnd)
{
	if (clipId <= 0)
	{
		return false;
	}

	// single instance, a new request replaces the old viewer
	if (s_pInstance != NULL)
	{
		s_pInstance->Close();
	}

	CImageViewerWnd* pWnd = new CImageViewerWnd();
	if (pWnd->LoadFromClip(clipId) == false)
	{
		delete pWnd;
		return false;
	}

	// cover the whole monitor that the quick paste window is on
	HMONITOR hMonitor = MonitorFromWindow(
		pRefWnd ? pRefWnd->GetSafeHwnd() : NULL, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);
	CRect rcMonitor = mi.rcMonitor;

	LPCTSTR csClass = AfxRegisterWndClass(
		CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	// the visible popup activates itself during creation, which deactivates the
	// quick paste window - mark the viewer as open BEFORE creating it so the
	// quick paste window knows not to auto-hide (and destroy the preview pane
	// we are currently called from)
	s_pInstance = pWnd;

	// own the viewer by the top level window, not by the preview pane child
	CWnd* pOwner = pRefWnd ? pRefWnd->GetTopLevelParent() : NULL;
	if (pWnd->CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, csClass, _T(""),
		WS_POPUP | WS_VISIBLE, rcMonitor, pOwner, 0) == FALSE)
	{
		s_pInstance = NULL;
		delete pWnd;
		return false;
	}

	pWnd->SetFocus();
	Log(StrF(_T("ImageViewerWnd shown for clip %d, image %d x %d"),
		clipId, (int)pWnd->m_pImage->GetWidth(), (int)pWnd->m_pImage->GetHeight()));
	return true;
}

bool CImageViewerWnd::LoadFromClip(int clipId)
{
	try
	{
		// prefer PNG (lossless, keeps alpha), then the DIB variants
		CString csSQL;
		csSQL.Format(_T("SELECT strClipBoardFormat, ooData FROM Data ")
			_T("WHERE lParentID = %d AND strClipBoardFormat IN ('PNG', 'CF_DIBV5', 'CF_DIB', 'CF_BITMAP') ")
			_T("ORDER BY CASE strClipBoardFormat WHEN 'PNG' THEN 0 WHEN 'CF_DIBV5' THEN 1 WHEN 'CF_DIB' THEN 2 ELSE 3 END, Data.lID desc LIMIT 1"),
			clipId);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);
		if (q.eof())
		{
			return false;
		}

		CString csFormat = q.getStringField(_T("strClipBoardFormat"));
		int nLen = 0;
		const unsigned char* pData = q.getBlobField(_T("ooData"), nLen);
		if (pData == NULL || nLen <= 0)
		{
			return false;
		}

		Gdiplus::Bitmap* pLoaded = NULL;
		HGLOBAL hGlobal = NewGlobalP((LPVOID)pData, nLen);
		if (hGlobal == NULL)
		{
			return false;
		}

		if (csFormat.CompareNoCase(_T("PNG")) == 0)
		{
			pLoaded = PNGImageHelper::GdipImageFromHGLOBAL(hGlobal);
		}
		else
		{
			pLoaded = DIBImageHelper::GdipImageFromHGLOBAL(hGlobal);
		}
		GlobalFree(hGlobal);

		// GDI+ reads lazily from the stream, which is already released by now.
		// Clone so the viewer owns its pixels and can paint at any time.
		if (pLoaded != NULL && pLoaded->GetLastStatus() == Gdiplus::Ok)
		{
			m_pImage = pLoaded->Clone(0, 0, pLoaded->GetWidth(), pLoaded->GetHeight(),
				PixelFormat32bppPARGB);
			delete pLoaded;
		}
		else if (pLoaded != NULL)
		{
			delete pLoaded;
		}

		if (m_pImage == NULL || m_pImage->GetLastStatus() != Gdiplus::Ok)
		{
			if (m_pImage)
			{
				delete m_pImage;
				m_pImage = NULL;
			}
			return false;
		}
		return true;
	}
	CATCH_SQLITE_EXCEPTION

	return false;
}

void CImageViewerWnd::Close()
{
	// DestroyWindow triggers another WM_KILLFOCUS, don't recurse
	if (m_bClosing == false && ::IsWindow(GetSafeHwnd()))
	{
		m_bClosing = true;
		DestroyWindow();
	}
}

BOOL CImageViewerWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	// everything is painted in OnPaint
	return TRUE;
}

void CImageViewerWnd::OnPaint()
{
	CPaintDC dc(this);

	CRect rcClient;
	GetClientRect(rcClient);

	// double buffer, the image + text go to a memory dc first
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

	memDC.FillSolidRect(rcClient, RGB(24, 24, 24));

	if (m_pImage != NULL)
	{
		Gdiplus::Graphics g(memDC.GetSafeHdc());
		g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

		// fit into 85% of the monitor, keep aspect, don't upscale small images
		double dMaxW = rcClient.Width() * 0.85;
		double dMaxH = rcClient.Height() * 0.80;
		double dScale = min(dMaxW / m_pImage->GetWidth(), dMaxH / m_pImage->GetHeight());
		if (dScale > 1.0)
		{
			dScale = 1.0;
		}
		int nW = max(1, (int)(m_pImage->GetWidth() * dScale));
		int nH = max(1, (int)(m_pImage->GetHeight() * dScale));
		int nX = rcClient.left + (rcClient.Width() - nW) / 2;
		int nY = rcClient.top + (rcClient.Height() - nH) / 2;

		g.DrawImage(m_pImage, nX, nY, nW, nH);
	}

	// usage hint at the bottom
	memDC.SetBkMode(TRANSPARENT);
	memDC.SetTextColor(RGB(150, 150, 150));
	CFont* pOldFont = memDC.SelectObject(&m_hintFont);
	CString csHint = theApp.m_Language.GetString("ImageViewerHint", "Esc / Space / Click to close");
	CRect rcHint = rcClient;
	rcHint.top = rcHint.bottom - 40;
	memDC.DrawText(csHint, rcHint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memDC.SelectObject(pOldFont);

	dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void CImageViewerWnd::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	Close();
}

void CImageViewerWnd::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if (nChar == VK_ESCAPE || nChar == VK_SPACE || nChar == VK_RETURN)
	{
		Close();
	}
}

void CImageViewerWnd::OnKillFocus(CWnd* /*pNewWnd*/)
{
	// act like a transient overlay, alt-tab / click elsewhere dismisses it
	Close();
}

void CImageViewerWnd::OnDestroy()
{
	if (s_pInstance == this)
	{
		s_pInstance = NULL;
	}
	CWnd::OnDestroy();
}

void CImageViewerWnd::PostNcDestroy()
{
	CWnd::PostNcDestroy();
	delete this;
}
