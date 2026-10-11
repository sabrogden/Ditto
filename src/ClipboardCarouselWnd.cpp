#include "stdafx.h"
#include "CP_Main.h"
#include "ClipboardCarouselWnd.h"
#include "Misc.h"

namespace
{
	// Dark palette shared with the other flyouts; logical sizes are DPI-scaled at use.
	const COLORREF BACKGROUND = RGB(28,28,32);
	const COLORREF SURFACE = RGB(37,37,43);
	const COLORREF SURFACE_BORDER = RGB(50,50,58);
	const COLORREF FOREGROUND = RGB(236,236,240);
	const COLORREF MUTED = RGB(150,150,162);
	const COLORREF BORDER = RGB(62,62,72);
	const int OUTER_PADDING = 8;
	const int INNER_PADDING = 12;
	const int SURFACE_RADIUS = 8;
	const int IMAGE_RADIUS = 6;

	HMODULE LoadSystemLibrary(LPCTSTR name)
	{
		return LoadLibraryEx(name,NULL,LOAD_LIBRARY_SEARCH_SYSTEM32);
	}

	// Windows 11 rounds popup corners and draws the border; earlier versions report failure.
	bool SetDwmAttribute(HWND window, DWORD attribute, DWORD value)
	{
		typedef HRESULT(WINAPI *DwmSetWindowAttributeFn)(HWND,DWORD,LPCVOID,DWORD);
		static DwmSetWindowAttributeFn setAttribute = []() -> DwmSetWindowAttributeFn
		{
			HMODULE dwm = LoadSystemLibrary(_T("dwmapi.dll"));
			return dwm ? (DwmSetWindowAttributeFn)GetProcAddress(dwm,"DwmSetWindowAttribute") : NULL;
		}();
		return setAttribute && SUCCEEDED(setAttribute(window,attribute,&value,sizeof(value)));
	}

	void SetControlTheme(HWND window, LPCWSTR theme)
	{
		typedef HRESULT(WINAPI *SetWindowThemeFn)(HWND,LPCWSTR,LPCWSTR);
		static SetWindowThemeFn setTheme = []() -> SetWindowThemeFn
		{
			HMODULE uxtheme = LoadSystemLibrary(_T("uxtheme.dll"));
			return uxtheme ? (SetWindowThemeFn)GetProcAddress(uxtheme,"SetWindowTheme") : NULL;
		}();
		if(setTheme) setTheme(window,theme,NULL);
	}

	Gdiplus::Color ToColor(COLORREF color)
	{
		return Gdiplus::Color(255,GetRValue(color),GetGValue(color),GetBValue(color));
	}

	void AddRoundRect(Gdiplus::GraphicsPath& path, const Gdiplus::RectF& rect, Gdiplus::REAL radius)
	{
		Gdiplus::REAL diameter = min(radius*2,min(rect.Width,rect.Height));
		if(diameter < 1) { path.AddRectangle(rect); return; }
		path.AddArc(rect.X,rect.Y,diameter,diameter,180,90);
		path.AddArc(rect.GetRight()-diameter,rect.Y,diameter,diameter,270,90);
		path.AddArc(rect.GetRight()-diameter,rect.GetBottom()-diameter,diameter,diameter,0,90);
		path.AddArc(rect.X,rect.GetBottom()-diameter,diameter,diameter,90,90);
		path.CloseFigure();
	}
}

BEGIN_MESSAGE_MAP(CClipboardCarouselWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEACTIVATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

CClipboardCarouselWnd::CClipboardCarouselWnd()
	: m_image(NULL), m_imagePreview(false), m_positioning(false), m_anchor{}, m_monitor(NULL),
	m_background(BACKGROUND), m_foreground(FOREGROUND), m_border(BORDER), m_surface(SURFACE),
	m_surfaceBorder(SURFACE_BORDER), m_muted(MUTED), m_roundedByDwm(false)
{
}

CClipboardCarouselWnd::~CClipboardCarouselWnd() { DeleteImage(); }

bool CClipboardCarouselWnd::Create(CWnd* owner)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
		LoadCursor(NULL, IDC_ARROW), NULL, NULL);
	if(!CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, className,
		theApp.m_Language.GetString("ClipboardCarouselTitle", "Clipboard Carousel"),
		WS_POPUP | WS_CLIPCHILDREN, CRect(0,0,1,1), owner, 0))
		return false;
	m_dpi.SetHwnd(m_hWnd);
	if(!m_textEdit.Create(WS_CHILD | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
		CRect(0,0,1,1), this, 1))
		return false;
	RefreshColors();
	m_textEdit.SetLimitText(4096);
	RecreateFont();
	return true;
}

void CClipboardCarouselWnd::RefreshColors()
{
	HIGHCONTRAST contrast = {};
	contrast.cbSize = sizeof(contrast);
	bool highContrast = SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0) &&
		(contrast.dwFlags & HCF_HIGHCONTRASTON);
	m_background = highContrast ? GetSysColor(COLOR_WINDOW) : BACKGROUND;
	m_foreground = highContrast ? GetSysColor(COLOR_WINDOWTEXT) : FOREGROUND;
	m_border = highContrast ? GetSysColor(COLOR_WINDOWTEXT) : BORDER;
	m_surface = highContrast ? m_background : SURFACE;
	m_surfaceBorder = highContrast ? m_foreground : SURFACE_BORDER;
	m_muted = highContrast ? m_foreground : MUTED;
	m_textBrush.DeleteObject();
	m_textBrush.CreateSolidBrush(m_surface);
	ApplyWindowStyle(highContrast);
	if(GetSafeHwnd()) Invalidate();
}

void CClipboardCarouselWnd::ApplyWindowStyle(bool highContrast)
{
	if(!GetSafeHwnd()) return;
	const DWORD DWMWA_WINDOW_CORNER_PREFERENCE_ = 33, DWMWA_BORDER_COLOR_ = 34, DWMWCP_ROUND_ = 2;
	m_roundedByDwm = SetDwmAttribute(m_hWnd,DWMWA_WINDOW_CORNER_PREFERENCE_,DWMWCP_ROUND_);
	if(m_roundedByDwm) SetDwmAttribute(m_hWnd,DWMWA_BORDER_COLOR_,m_border);
	// Dark scrollbar for the text preview; High Contrast keeps the system theme.
	if(m_textEdit.GetSafeHwnd())
		SetControlTheme(m_textEdit.GetSafeHwnd(),highContrast ? NULL : L"DarkMode_Explorer");
}

CRect CClipboardCarouselWnd::SurfaceRect()
{
	CRect surface;
	GetClientRect(surface);
	surface.DeflateRect(m_dpi.Scale(OUTER_PADDING),m_dpi.Scale(OUTER_PADDING));
	return surface;
}

CRect CClipboardCarouselWnd::ContentRect()
{
	CRect content = SurfaceRect();
	content.DeflateRect(m_dpi.Scale(INNER_PADDING),m_dpi.Scale(INNER_PADDING));
	return content;
}

void CClipboardCarouselWnd::RecreateFont()
{
	LOGFONT font = {};
	font.lfHeight = -m_dpi.Scale(15);
	font.lfQuality = CLEARTYPE_QUALITY;
	_tcscpy_s(font.lfFaceName, LF_FACESIZE, _T("Segoe UI"));
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&font);
	if(m_textEdit.GetSafeHwnd())
	{
		m_textEdit.SetFont(&m_font);
		m_textEdit.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN,
			MAKELPARAM(m_dpi.Scale(3), m_dpi.Scale(3)));
	}
}

void CClipboardCarouselWnd::DeleteImage()
{
	if(m_image) DeleteObject(m_image);
	m_image = NULL;
}

void CClipboardCarouselWnd::SetPreview(const CString& text, bool imagePreview)
{
	DeleteImage();
	// An image description can be a raw clipboard format name (such as CF_DIB).
	// Keep its loading frame empty until the thumbnail is ready.
	m_text = imagePreview ? CString() : text.Left(4096);
	if(!m_text.IsEmpty() && m_text[m_text.GetLength()-1] >= 0xD800 && m_text[m_text.GetLength()-1] <= 0xDBFF)
		m_text.Truncate(m_text.GetLength()-1);
	m_imagePreview = imagePreview;
	m_textEdit.SetWindowText(m_text);
	m_textEdit.SetSel(0,0);
	m_textEdit.ShowWindow(imagePreview ? SW_HIDE : SW_SHOWNA);
	if(IsWindowVisible()) LayoutPreview();
	Invalidate();
}

void CClipboardCarouselWnd::SetImage(HBITMAP image)
{
	DeleteImage();
	m_image = image;
	if(!image)
		m_text = theApp.m_Language.GetString("ClipboardCarouselNoPreview","No preview available.");
	if(IsWindowVisible()) Invalidate();
}

void CClipboardCarouselWnd::ShowPreview()
{
	if(!IsWindowVisible())
	{
		GetCursorPos(&m_anchor);
		m_monitor = MonitorFromPoint(m_anchor, MONITOR_DEFAULTTONEAREST);
		// Move a hidden, tiny window first so Windows establishes destination DPI.
		// The final rectangle is then computed from the actual window DPI once.
		m_positioning = true;
		SetWindowPos(NULL, m_anchor.x, m_anchor.y, 1, 1, SWP_NOACTIVATE | SWP_NOZORDER);
		typedef UINT(WINAPI *WindowDpi)(HWND);
		static WindowDpi windowDpi = (WindowDpi)GetProcAddress(GetModuleHandle(_T("user32.dll")),"GetDpiForWindow");
		if(windowDpi) m_dpi.Update(windowDpi(m_hWnd));
		else m_dpi.SetHwnd(m_hWnd);
		RecreateFont();
		m_positioning = false;
	}
	LayoutPreview();
	ShowWindow(SW_SHOWNOACTIVATE);
}

void CClipboardCarouselWnd::LayoutPreview()
{
	if(m_positioning || !GetSafeHwnd()) return;
	MONITORINFO info = {};
	info.cbSize = sizeof(info);
	if(!GetMonitorInfo(m_monitor, &info)) return;
	CRect work(info.rcWork);
	int width = min(m_dpi.Scale(480), work.Width());
	int height = min(m_dpi.Scale(300), work.Height());
	int x = max(work.left, min(m_anchor.x + m_dpi.Scale(16), work.right - width));
	int y = max(work.top, min(m_anchor.y + m_dpi.Scale(20), work.bottom - height));
	m_positioning = true;
	SetWindowPos(&wndTopMost, x, y, width, height, SWP_NOACTIVATE);
	m_positioning = false;
	PositionText();
	Invalidate();
}

void CClipboardCarouselWnd::PositionText()
{
	if(!m_textEdit.GetSafeHwnd()) return;
	CRect content = ContentRect();
	m_textEdit.SetWindowPos(NULL, content.left, content.top, max(1,content.Width()),
		max(1,content.Height()), SWP_NOZORDER | SWP_NOACTIVATE);
	// Measure wrapped lines at the full available width before reserving a scrollbar.
	m_textEdit.ShowScrollBar(SB_VERT,FALSE);
	if(!m_imagePreview)
	{
		CRect formatting;
		m_textEdit.GetRect(formatting);
		CClientDC dc(&m_textEdit);
		CFont* previous = m_font.GetSafeHandle() ? dc.SelectObject(&m_font) : NULL;
		TEXTMETRIC metrics = {};
		dc.GetTextMetrics(&metrics);
		if(previous) dc.SelectObject(previous);
		int visibleLines = max(1,formatting.Height() / max(1L,metrics.tmHeight));
		m_textEdit.ShowScrollBar(SB_VERT,m_textEdit.GetLineCount() > visibleLines);
	}
}

void CClipboardCarouselWnd::HidePreview()
{
	if(GetSafeHwnd()) ShowWindow(SW_HIDE);
	DeleteImage();
}

bool CClipboardCarouselWnd::HasFocus() const
{
	HWND focus = ::GetFocus();
	return focus == GetSafeHwnd() || (GetSafeHwnd() && ::IsChild(GetSafeHwnd(), focus));
}

CString CClipboardCarouselWnd::SelectedText() const
{
	if(m_imagePreview) return CString();
	int start = 0, end = 0;
	m_textEdit.GetSel(start, end);
	CString displayed;
	m_textEdit.GetWindowText(displayed);
	return end > start ? displayed.Mid(start, end - start) : CString();
}

BOOL CClipboardCarouselWnd::PreTranslateMessage(MSG* message)
{
	if(message->message == WM_KEYDOWN && HasFocus() && (::GetKeyState(VK_CONTROL) & 0x8000))
	{
		if(message->wParam == 'A')
		{
			m_textEdit.SetSel(0,-1);
			return TRUE;
		}
		if(message->wParam == 'C')
		{
			CString text = SelectedText();
			if(!text.IsEmpty())
			{
				HGLOBAL data = NewGlobalP((LPVOID)(LPCTSTR)text, (text.GetLength()+1)*sizeof(TCHAR));
				if(data)
				{
					COleDataSource* source = new COleDataSource;
					source->CacheGlobalData(CF_UNICODETEXT, data);
					source->SetClipboard();
				}
			}
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(message);
}

void CClipboardCarouselWnd::OnPaint()
{
	CPaintDC paint(this);
	CRect client;
	GetClientRect(client);
	if(client.Width() <= 0 || client.Height() <= 0) return;

	// Compose off-screen so the rounded surface and image never flicker while cycling.
	CDC dc;
	dc.CreateCompatibleDC(&paint);
	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&paint, client.Width(), client.Height());
	CBitmap* previousBuffer = dc.SelectObject(&buffer);
	dc.FillSolidRect(client, m_background);
	if(!m_roundedByDwm) dc.Draw3dRect(client, m_border, m_border);

	CRect surface = SurfaceRect();
	Gdiplus::REAL line = (Gdiplus::REAL)max(1,m_dpi.Scale(1));
	{
		Gdiplus::Graphics graphics(dc.GetSafeHdc());
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
		Gdiplus::GraphicsPath path;
		AddRoundRect(path, Gdiplus::RectF(surface.left+line/2, surface.top+line/2,
			surface.Width()-line, surface.Height()-line), (Gdiplus::REAL)m_dpi.Scale(SURFACE_RADIUS));
		Gdiplus::SolidBrush fill(ToColor(m_surface));
		graphics.FillPath(&fill, &path);
		Gdiplus::Pen pen(ToColor(m_surfaceBorder), line);
		graphics.DrawPath(&pen, &path);
	}

	CRect content = ContentRect();
	if(m_imagePreview && content.Width() > 0 && content.Height() > 0)
	{
		if(m_image)
		{
			BITMAP bitmap = {};
			GetObject(m_image, sizeof(bitmap), &bitmap);
			CDC imageDC;
			imageDC.CreateCompatibleDC(&dc);
			HGDIOBJ previous = imageDC.SelectObject(m_image);
			double scale = min((double)content.Width()/max(1L,bitmap.bmWidth),
				(double)content.Height()/max(1L,bitmap.bmHeight));
			int width = max(1,(int)(bitmap.bmWidth*scale));
			int height = max(1,(int)(bitmap.bmHeight*scale));
			int x = content.CenterPoint().x-width/2, y = content.CenterPoint().y-height/2;
			dc.SetStretchBltMode(HALFTONE);
			SetBrushOrgEx(dc.GetSafeHdc(), 0, 0, NULL);
			dc.StretchBlt(x, y, width, height, &imageDC, 0,0, bitmap.bmWidth,bitmap.bmHeight, SRCCOPY);
			imageDC.SelectObject(previous);

			// Round the image corners by painting the surface back over them, anti-aliased.
			Gdiplus::Graphics graphics(dc.GetSafeHdc());
			graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
			Gdiplus::GraphicsPath corners(Gdiplus::FillModeAlternate);
			corners.AddRectangle(Gdiplus::RectF((Gdiplus::REAL)x-1, (Gdiplus::REAL)y-1,
				(Gdiplus::REAL)width+2, (Gdiplus::REAL)height+2));
			AddRoundRect(corners, Gdiplus::RectF((Gdiplus::REAL)x, (Gdiplus::REAL)y,
				(Gdiplus::REAL)width, (Gdiplus::REAL)height), (Gdiplus::REAL)m_dpi.Scale(IMAGE_RADIUS));
			Gdiplus::SolidBrush fill(ToColor(m_surface));
			graphics.FillPath(&fill, &corners);
		}
		else if(!m_text.IsEmpty())
		{
			CFont* previous = dc.SelectObject(&m_font);
			dc.SetTextColor(m_muted);
			dc.SetBkMode(TRANSPARENT);
			CRect measured = content;
			dc.DrawText(m_text, measured, DT_WORDBREAK | DT_NOPREFIX | DT_CENTER | DT_CALCRECT);
			CRect text = content;
			text.top = max(content.top, content.CenterPoint().y - measured.Height()/2);
			dc.DrawText(m_text, text, DT_WORDBREAK | DT_NOPREFIX | DT_CENTER);
			dc.SelectObject(previous);
		}
	}

	paint.BitBlt(0, 0, client.Width(), client.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(previousBuffer);
}

BOOL CClipboardCarouselWnd::OnEraseBkgnd(CDC*) { return TRUE; }
int CClipboardCarouselWnd::OnMouseActivate(CWnd*, UINT, UINT) { return MA_ACTIVATE; }
void CClipboardCarouselWnd::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type,cx,cy);
	PositionText();
}
HBRUSH CClipboardCarouselWnd::OnCtlColor(CDC* dc, CWnd* wnd, UINT type)
{
	if(wnd && wnd->GetSafeHwnd() == m_textEdit.GetSafeHwnd())
	{
		LOGBRUSH brush = {};
		m_textBrush.GetLogBrush(&brush);
		dc->SetBkColor(brush.lbColor);
		dc->SetTextColor(m_foreground);
		return (HBRUSH)m_textBrush.GetSafeHandle();
	}
	return CWnd::OnCtlColor(dc,wnd,type);
}
LRESULT CClipboardCarouselWnd::OnDpiChanged(WPARAM dpi, LPARAM)
{
	m_dpi.Update(HIWORD(dpi));
	RecreateFont();
	if(!m_positioning)
	{
		m_monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		LayoutPreview();
	}
	return 0;
}
