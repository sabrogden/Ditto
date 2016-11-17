// AeroEdit.cpp : implementation file
//

#include "stdafx.h"
#include "SymbolEdit.h"
#include "cp_main.h"

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

	m_editFocusColor = RGB(255, 255, 255);
	m_editNonFocusColor = RGB(240, 240, 240);
	m_editFocusBrush.CreateSolidBrush(m_editFocusColor);
	m_editNonFocusBrush.CreateSolidBrush(m_editNonFocusColor);

	m_searchButton.LoadStdImageDPI(Search_16, Search_20, Search_24, Search_32, _T("PNG"));
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
		dc.FillSolidRect(rect, m_editFocusColor);

		oldFont = dc.SelectObject(GetFont());		
			
		dc.DrawText(text, textRect, DT_SINGLELINE | DT_INTERNAL | DT_EDITCONTROL);
		dc.SelectObject(oldFont);
	}
	else
	{
		dc.FillSolidRect(rect, m_editNonFocusColor);

		if (this != GetFocus() && m_strPromptText.GetLength() > 0)
		{
			oldFont = dc.SelectObject(&m_fontPrompt);
			COLORREF color = dc.GetTextColor();
			dc.SetTextColor(m_colorPromptText);
			
			dc.DrawText(m_strPromptText, textRect, DT_LEFT | DT_SINGLELINE | DT_EDITCONTROL);
			dc.SetTextColor(color);
			dc.SelectObject(oldFont);
		}
	}

	m_searchButton.Draw(&dc, this, rect.right - 22, 4, false, false);

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
		pDC->SetBkColor(m_editFocusColor);
		return m_editFocusBrush;
	}
	else
	{
		pDC->SetBkColor(m_editNonFocusColor);
		return m_editNonFocusBrush;
	}
}

void CSymbolEdit::OnSetFocus(CWnd* pOldWnd)
{
	Invalidate(FALSE);
	CEdit::OnSetFocus(pOldWnd);
}

void CSymbolEdit::OnKillFocus(CWnd* pNewWnd)
{
	Invalidate(FALSE);
	CEdit::OnKillFocus(pNewWnd);
}