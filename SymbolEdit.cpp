// AeroEdit.cpp : implementation file
//

#include "stdafx.h"
#include "SymbolEdit.h"
#include "cp_main.h"
#include "QListCtrl.h"

// CSymbolEdit

IMPLEMENT_DYNAMIC(CSymbolEdit, CEdit)

CSymbolEdit::CSymbolEdit() :
	m_hSymbolIcon(NULL),
	m_bInternalIcon(false),
	m_colorPromptText(RGB(127, 127, 127))
{
	m_fontPrompt.CreateFont(
		16,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		TRUE,                      // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		DEFAULT_CHARSET,           // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Calibri"));


	//m_searchButton.LoadStdImageDPI(Search_16, Search_20, Search_24, Search_32, _T("PNG"));
	m_closeButton.LoadStdImageDPI(search_close_16, Search_20, Search_24, Search_28, Search_32, _T("PNG"));
}

CSymbolEdit::~CSymbolEdit()
{
	DestroyIcon();
}


BEGIN_MESSAGE_MAP(CSymbolEdit, CEdit)
	ON_WM_PAINT()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL CSymbolEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	// Intercept Ctrl + Z (Undo), Ctrl + X (Cut), Ctrl + C (Copy), Ctrl + V (Paste) and Ctrl + A (Select All)
	// before CEdit base class gets a hold of them.
	if (pMsg->message == WM_KEYDOWN &&
		CONTROL_PRESSED)
	{
		switch (pMsg->wParam)
		{
		case 'Z':
			Undo();
			return TRUE;
		case 'X':
			Cut();
			return TRUE;
		case 'C':
			Copy();
			return TRUE;
		case 'V':
			Paste();
			return TRUE;
		case 'A':
			SetSel(0, -1);
			return TRUE;
		}
	}

	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	{
		if (pMsg->wParam == VK_RETURN)
		{
			CWnd *pWnd = GetParent();
			if (pWnd)
			{
				if (g_Opt.m_bFindAsYouType)
				{
					pWnd->SendMessage(NM_SEARCH_ENTER_PRESSED, 0, 0);
				}
				else
				{
					//Send a message to the parent to refill the lb from the search
					pWnd->PostMessage(CB_SEARCH, 0, 0);
				}
			}

			return TRUE;
		}
		else if (pMsg->wParam == VK_DOWN ||
			pMsg->wParam == VK_UP ||
			pMsg->wParam == VK_F3 ||
			pMsg->wParam == VK_PRIOR ||
			pMsg->wParam == VK_NEXT)
		{
			CWnd *pWnd = GetParent();
			if (pWnd)
			{
				pWnd->SendMessage(CB_UPDOWN, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
		}
		else if (pMsg->wParam == VK_DELETE)
		{
			int startChar;
			int endChar;
			this->GetSel(startChar, endChar);
			CString cs;
			this->GetWindowText(cs);
			//if selection is at the end then forward this on to the parent to delete the selected clip
			if(startChar == cs.GetLength() &&
				endChar == cs.GetLength())
			{ 
				CWnd *pWnd = GetParent();
				if (pWnd)
				{
					pWnd->SendMessage(NM_DELETE, pMsg->wParam, pMsg->lParam);
					return TRUE;
				}
			}
		}
		break;
	}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void CSymbolEdit::DestroyIcon()
{
	// if icon was loaded internally, destroy it
	if (m_bInternalIcon || m_hSymbolIcon != NULL)
		::DestroyIcon(m_hSymbolIcon);
}

void CSymbolEdit::PreSubclassWindow()
{
	RecalcLayout();
}

void CSymbolEdit::SetSymbolIcon(HICON hIcon, BOOL redraw)
{
	DestroyIcon();

	m_hSymbolIcon = hIcon;

	// icon was not loaded internally
	m_bInternalIcon = false;

	RecalcLayout();

	if (redraw)
		Invalidate(TRUE);
}

void CSymbolEdit::SetSymbolIcon(UINT id, BOOL redraw)
{
	DestroyIcon();

	m_hSymbolIcon = (HICON)::LoadImage(
		AfxGetResourceHandle(),
		MAKEINTRESOURCE(id),
		IMAGE_ICON,
		16,
		16,
		LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);

	ASSERT(m_hSymbolIcon != NULL);

	// icon was loaded internally
	m_bInternalIcon = true;

	RecalcLayout();

	if (redraw)
		Invalidate(TRUE);
}

void CSymbolEdit::SetPromptText(CString text, BOOL redraw)
{
	m_strPromptText = text;

	if (redraw)
		Invalidate(TRUE);
}

void CSymbolEdit::SetPromptText(LPCTSTR szText, BOOL redraw)
{
	m_strPromptText = szText;

	if (redraw)
		Invalidate(TRUE);
}

void CSymbolEdit::SetPromptTextColor(COLORREF color, BOOL redraw)
{
	m_colorPromptText = color;

	if (redraw)
		Invalidate(TRUE);
}

void CSymbolEdit::SetPromptFont(CFont& font, BOOL redraw)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	font.GetLogFont(&lf);
	SetPromptFont(&lf);

	if (redraw)
		Invalidate(TRUE);
}

void CSymbolEdit::SetPromptFont(const LOGFONT* lpLogFont, BOOL redraw)
{
	m_fontPrompt.DeleteObject();
	m_fontPrompt.CreateFontIndirect(lpLogFont);

	if (redraw)
		Invalidate(TRUE);
}

void CSymbolEdit::RecalcLayout()
{
	int width = GetSystemMetrics(SM_CXSMICON);

	if (m_hSymbolIcon)
	{
		DWORD dwMargins = GetMargins();
		SetMargins(LOWORD(dwMargins), width + 6);
	}
	else
	{
		SetMargins(4, 24);
	}
}

// CSymbolEdit message handlers

void CSymbolEdit::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	DWORD margins = GetMargins();
	
	CRect textRect(rect);
	textRect.left += LOWORD(margins);
	textRect.right -= HIWORD(margins);

	// Clearing the background
	dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

	

	if (m_hSymbolIcon)
	{
		// Drawing the icon
		int width = GetSystemMetrics(SM_CXSMICON);
		int height = GetSystemMetrics(SM_CYSMICON);

		::DrawIconEx(
			dc.m_hDC,
			rect.right - width - 1,
			1,
			m_hSymbolIcon,
			width,
			height,
			0,
			NULL,
			DI_NORMAL);

		rect.left += LOWORD(margins) + 1;
		rect.right -= (width + 7);
	}
	else
	{
		//rect.left += (LOWORD(dwMargins) + 1);
		//rect.right -= (HIWORD(dwMargins) + 1);
	}

	CString text;
	GetWindowText(text);
	CFont* oldFont = NULL;

	//rect.top += 1;


	if(this == GetFocus() || text.GetLength() > 0)
	{
		dc.FillSolidRect(rect, g_Opt.m_Theme.SearchTextBoxFocusBG());

		oldFont = dc.SelectObject(GetFont());		

		COLORREF oldColor = dc.GetTextColor();
		dc.SetTextColor(g_Opt.m_Theme.SearchTextBoxFocusText());
			
		dc.DrawText(text, textRect, DT_SINGLELINE | DT_INTERNAL | DT_EDITCONTROL);

		dc.SelectObject(oldFont);
		dc.SetTextColor(oldColor);
	}
	else
	{
		dc.FillSolidRect(rect, g_Opt.m_Theme.MainWindowBG());
	}


	if (text.GetLength() == 0 && m_strPromptText.GetLength() > 0)
	{
		//if we aren't showing the close icon, then use the full space
		textRect.right += HIWORD(margins);
		textRect.right -= LOWORD(margins);

		oldFont = dc.SelectObject(&m_fontPrompt);
		COLORREF color = dc.GetTextColor();
		dc.SetTextColor(m_colorPromptText);

		dc.DrawText(m_strPromptText, textRect, DT_LEFT | DT_SINGLELINE | DT_EDITCONTROL | DT_VCENTER);
		dc.SetTextColor(color);
		dc.SelectObject(oldFont);
	}

	if (text.GetLength() > 0)
	{
		m_closeButtonRect.SetRect(rect.right - 22, 0, rect.right, rect.bottom);
		m_closeButton.Draw(&dc, this, m_closeButtonRect.left, 4, m_mouseHoveringOverClose, m_mouseDownOnClose);
	}
	else
	{
		m_closeButtonRect.SetRect(0, 0, 0, 0);
		//m_searchButton.Draw(&dc, this, rect.right - 22, 4, false, false);
	}

	//OutputDebugString(_T("OnPaint"));

}

void CSymbolEdit::OnSize(UINT nType, int cx, int cy)
{
	CEdit::OnSize(nType, cx, cy);

	RecalcLayout();
}

LRESULT CSymbolEdit::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	DefWindowProc(WM_SETFONT, wParam, lParam);

	RecalcLayout();

	return 0;
}

HBRUSH CSymbolEdit::CtlColor(CDC* pDC, UINT n)
{
	if (::GetFocus() == m_hWnd)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		return CreateSolidBrush(RGB(255, 255, 255));
	}
	else
	{
		pDC->SetBkColor(g_Opt.m_Theme.MainWindowBG());
		return CreateSolidBrush(g_Opt.m_Theme.MainWindowBG());
	}
}

void CSymbolEdit::OnSetFocus(CWnd* pOldWnd)
{
	Invalidate(FALSE);
	CEdit::OnSetFocus(pOldWnd);

	CWnd *pWnd = GetParent();
	if (pWnd)
	{
		if (g_Opt.m_bFindAsYouType)
		{
			pWnd->SendMessage(NM_FOCUS_ON_SEARCH, 0, 0);
		}
	}
}

void CSymbolEdit::OnKillFocus(CWnd* pNewWnd)
{
	Invalidate(FALSE);
	CEdit::OnKillFocus(pNewWnd);
}

BOOL CSymbolEdit::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint pntCursor;
	GetCursorPos(&pntCursor);
	ScreenToClient(&pntCursor);

	if(m_closeButtonRect.PtInRect(pntCursor))
	{
		HCURSOR h = ::LoadCursor(NULL, IDC_ARROW);
		::SetCursor(h);
		return TRUE;
	}

	return CEdit::OnSetCursor(pWnd, nHitTest, message);
}

void CSymbolEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_mouseDownOnClose)
	{
		ReleaseCapture();
		InvalidateRect(m_closeButtonRect);
	}

	m_mouseDownOnClose = false;

	if (m_closeButtonRect.PtInRect(point))
	{
		if ((GetWindowTextLength() > 0))
		{
			CWnd *pOwner = GetOwner();
			if (pOwner)
			{
				pOwner->SendMessage(NM_CANCEL_SEARCH, 0, 0);
			}
		}		
	}	

	CEdit::OnLButtonUp(nFlags, point);
}

void CSymbolEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_closeButtonRect.PtInRect(point))
	{
		m_mouseDownOnClose = true;
		SetCapture();
		InvalidateRect(m_closeButtonRect);
	}
	else
	{
		m_mouseDownOnClose = false;
	}

	CEdit::OnLButtonDown(nFlags, point);
}

void CSymbolEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_closeButtonRect.PtInRect(point))
	{
		if (m_mouseHoveringOverClose == false)
		{
			m_mouseHoveringOverClose = true;
			InvalidateRect(m_closeButtonRect);
		}
	}
	else if(m_mouseHoveringOverClose)
	{
		m_mouseHoveringOverClose = false;
		InvalidateRect(m_closeButtonRect);
	}

	CEdit::OnMouseMove(nFlags, point);
}
