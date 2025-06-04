// QListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QListCtrl.h"
#include "ProcessPaste.h"
#include "BitmapHelper.h"
#include "MainTableFunctions.h"
#include "DittoCopyBuffer.h"
#include <atlbase.h>
#include "DrawHTML.h"
#include "..\Shared\TextConvert.h"
#include <cmath>
#include <vector>
#include <string>
#include <cwchar>   // For swscanf
#include <algorithm> // For std::round

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ROW_BOTTOM_BORDER		4
#define ROW_LEFT_BORDER			3
#define COLOR_SHADOW			RGB(245, 245, 245)
#define DUMMY_COL_WIDTH			2

#define TIMER_SHOW_PROPERTIES	1
#define TIMER_HIDE_SCROL	2
#define TIMER_SHOW_SCROLL	3

#define VALID_TOOLTIP (m_pToolTip && ::IsWindow(m_pToolTip->m_hWnd))


/////////////////////////////////////////////////////////////////////////////
// CQListCtrl

CQListCtrl::CQListCtrl()
{
	m_pchTip = NULL;
	m_pwchTip = NULL;
	m_linesPerRow = 1;
	m_windowDpi = NULL;
	m_SmallFont = NULL;
	m_pToolTip = NULL;
	m_pFormatter = NULL;
	m_allSelected = false;
	m_rowHeight = 50;
	m_mouseOverScrollAreaStart = 0;
	m_showIfClipWasPasted = TRUE;
	m_bShowTextForFirstTenHotKeys = true;
	m_pToolTipActions = NULL;
}

CQListCtrl::~CQListCtrl()
{
	if (m_pchTip != NULL)
		delete m_pchTip;

	if (m_pwchTip != NULL)
		delete m_pwchTip;

	if (m_SmallFont)
		::DeleteObject(m_SmallFont);

	m_Font.DeleteObject();

	m_boldFont.DeleteObject();

	if (m_pFormatter)
	{
		delete m_pFormatter;
		m_pFormatter = NULL;
	}

	DeleteObject(m_SmallFont);
}

// returns the position 1-10 if the index is in the FirstTen block else -1
int CQListCtrl::GetFirstTenNum(int index)
{
	// set firstTenNum to the first ten number (1-10) corresponding to the given index
	int firstTenNum = -1; // -1 means that nItem is not in the FirstTen block.
	int count = GetItemCount();

	if (0 <= index && index <= 9)
	{
		firstTenNum = index + CGetSetOptions::m_firstTenHotKeysStart;
		firstTenNum = firstTenNum % 10;
	}

	return firstTenNum;
}

BEGIN_MESSAGE_MAP(CQListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CQListCtrl)
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomdrawList)
	ON_WM_MOUSEMOVE()
	ON_WM_SYSKEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnSelectionChange)
	ON_WM_VSCROLL()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_WM_KILLFOCUS()
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_MOUSEHWHEEL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQListCtrl message handlers

void CQListCtrl::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;

	*pResult = 0;
}

DROPEFFECT CQListCtrl::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return DROPEFFECT_COPY;
}

void CQListCtrl::GetSelectionIndexes(ARRAY& arr)
{
	arr.RemoveAll();

	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		arr.Add(GetNextSelectedItem(pos));
	}
}

bool CQListCtrl::PutSelectedItemOnDittoCopyBuffer(long lBuffer)
{
	bool bRet = false;
	ARRAY arr;
	GetSelectionItemData(arr);
	INT_PTR nCount = arr.GetSize();
	if (nCount > 0 && arr[0])
	{
		CDittoCopyBuffer::PutClipOnDittoCopyBuffer(arr[0], lBuffer);
		bRet = true;
	}

	return bRet;
}

void CQListCtrl::GetSelectionItemData(ARRAY& arr)
{
	DWORD dwData;
	int i;
	arr.RemoveAll();
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		i = GetNextSelectedItem(pos);
		dwData = GetItemData(i);
		arr.Add(dwData);
	}
}

void CQListCtrl::RemoveAllSelection()
{
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		SetSelection(GetNextSelectedItem(pos), FALSE);
	}
}

BOOL CQListCtrl::SetSelection(int nRow, BOOL bSelect)
{
	if (bSelect)
		return SetItemState(nRow, LVIS_SELECTED, LVIS_SELECTED);
	else
		return SetItemState(nRow, ~LVIS_SELECTED, LVIS_SELECTED);
}

BOOL CQListCtrl::SetText(int nRow, int nCol, CString cs)
{
	return SetItemText(nRow, nCol, cs);
}

BOOL CQListCtrl::SetCaret(int nRow, BOOL bFocus)
{
	if (bFocus)
		return SetItemState(nRow, LVIS_FOCUSED, LVIS_FOCUSED);
	else
		return SetItemState(nRow, ~LVIS_FOCUSED, LVIS_FOCUSED);
}

long CQListCtrl::GetCaret()
{
	return GetNextItem(-1, LVNI_FOCUSED);
}

// moves the caret to the given index, selects it, and ensures it is visible.
BOOL CQListCtrl::SetListPos(int index)
{
	if (index < 0 || index >= GetItemCount())
		return FALSE;

	RemoveAllSelection();
	SetCaret(index);
	SetSelection(index);
	ListView_SetSelectionMark(m_hWnd, index);
	EnsureVisible(index, FALSE);

	return TRUE;
}

BOOL CQListCtrl::SetFormattedText(int nRow, int nCol, LPCTSTR lpszFormat, ...)
{
	CString csText;
	va_list vlist;

	ASSERT(AfxIsValidString(lpszFormat));
	va_start(vlist, lpszFormat);
	csText.FormatV(lpszFormat, vlist);
	va_end(vlist);

	return SetText(nRow, nCol, csText);
}

void CQListCtrl::SetNumberOfLinesPerRow(int nLines, bool force)
{
	if (m_linesPerRow != nLines ||
		force)
	{
		m_linesPerRow = nLines;

		CRect rc;
		GetWindowRect(&rc);
		WINDOWPOS wp;
		wp.hwnd = m_hWnd;
		wp.cx = rc.Width();
		wp.cy = rc.Height();
		wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
		SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);
	}
}

void CQListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	TEXTMETRIC tm;
	HDC hDC = ::GetDC(NULL);
	CFont* pFont = GetFont();
	HFONT hFontOld = (HFONT)SelectObject(hDC, pFont->GetSafeHandle());
	GetTextMetrics(hDC, &tm);
	if (m_windowDpi != NULL)
	{
		lpMeasureItemStruct->itemHeight = ((tm.tmHeight + tm.tmExternalLeading) * m_linesPerRow) + m_windowDpi->Scale(ROW_BOTTOM_BORDER);
		m_rowHeight = lpMeasureItemStruct->itemHeight;
	}
	SelectObject(hDC, hFontOld);
	::ReleaseDC(NULL, hDC);
}

void CQListCtrl::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	*pResult = 0;

	// Request item-specific notifications if this is the
	// beginning of the paint cycle.
	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		LVITEM   rItem;
		int      nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
		CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
		COLORREF crBkgnd;
		BOOL     bListHasFocus;
		CRect    rcItem;

		bListHasFocus = (GetSafeHwnd() == ::GetFocus());

		// Get the image index and selected/focused state of the
		// item being drawn.
		ZeroMemory(&rItem, sizeof(LVITEM));
		rItem.mask = LVIF_STATE;
		rItem.iItem = nItem;
		rItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		GetItem(&rItem);

		// Get the rect that bounds the text label.
		GetItemRect(nItem, rcItem, LVIR_SELECTBOUNDS);

		COLORREF OldColor = -1;
		int nOldBKMode = -1;

		CString csText;
		LPTSTR lpszText = csText.GetBufferSetLength(CGetSetOptions::m_bDescTextSize);
		GetItemText(nItem, 0, lpszText, CGetSetOptions::m_bDescTextSize);
		csText.ReleaseBuffer();

		// extract symbols
		CString strSymbols;
		int nSymEnd = csText.Find('|');
		if (nSymEnd >= 0)
		{
			strSymbols = csText.Left(nSymEnd);
			csText = csText.Mid(nSymEnd + 1);
		}

		// Draw the background of the list item.  Colors are selected
		// according to the item's state.
		if (rItem.state & LVIS_SELECTED)
		{
			if (bListHasFocus)
			{
				crBkgnd = CGetSetOptions::m_Theme.ListBoxSelectedBG();
				OldColor = pDC->SetTextColor(CGetSetOptions::m_Theme.ListBoxSelectedText());
			}
			else
			{
				crBkgnd = CGetSetOptions::m_Theme.ListBoxSelectedNoFocusBG();
				OldColor = pDC->SetTextColor(CGetSetOptions::m_Theme.ListBoxSelectedNoFocusText());
			}
		}
		else
		{
			//Shade alternating Rows
			if ((nItem % 2) == 0)
			{
				crBkgnd = CGetSetOptions::m_Theme.ListBoxOddRowsBG();
				OldColor = pDC->SetTextColor(CGetSetOptions::m_Theme.ListBoxOddRowsText());
			}
			else
			{
				crBkgnd = CGetSetOptions::m_Theme.ListBoxEvenRowsBG();
				OldColor = pDC->SetTextColor(CGetSetOptions::m_Theme.ListBoxEvenRowsText());
			}
		}

		pDC->FillSolidRect(rcItem, crBkgnd);
		nOldBKMode = pDC->SetBkMode(TRANSPARENT);

		CRect rcText = rcItem;
		rcText.left += m_windowDpi->Scale(ROW_LEFT_BORDER);
		rcText.top += m_windowDpi->Scale(1);
		rcText.bottom -= m_windowDpi->Scale(1);

		if (m_showIfClipWasPasted &&
			strSymbols.GetLength() > 0 &&
			strSymbols.Find(_T("<pasted>")) >= 0) //clip was pasted from ditto
		{
			CRect pastedRect(rcItem);
			pastedRect.left++;
			pastedRect.right = pastedRect.left + m_windowDpi->Scale(2);

			pDC->FillSolidRect(pastedRect, CGetSetOptions::m_Theme.ClipPastedColor());
		}

		// set firstTenNum to the first ten number (1-10) corresponding to
		//  the current nItem.
		// -1 means that nItem is not in the FirstTen block.
		int firstTenNum = GetFirstTenNum(nItem);

		if (m_bShowTextForFirstTenHotKeys && firstTenNum >= 0)
		{
			rcText.left += m_windowDpi->Scale(12);
		}
		else
		{
			rcText.left += m_windowDpi->Scale(3);
		}

		bool drawInGroupIcon = true;
		// if we are inside a group, don't display the "in group" flag
		if (theApp.m_GroupID > 0)
		{
			int nFlag = strSymbols.Find(_T("<ingroup>"));
			if (nFlag >= 0)
				drawInGroupIcon = false;
		}

		DrawCopiedColorCode(csText, rcText, pDC);

		DrawBitMap(nItem, rcText, pDC, csText);

		// draw the symbol box
		if (strSymbols.GetLength() > 0)
		{
			if (strSymbols.Find(_T("<group>")) >= 0) //group
			{
				m_groupFolder.Draw(pDC, *m_windowDpi, this, rcText.left, rcText.top, false, false);
				rcText.left += m_groupFolder.ImageWidth() + m_windowDpi->Scale(2);
			}
			if (strSymbols.Find(_T("<noautodelete>")) >= 0) //don't auto delete
			{
				m_dontDeleteImage.Draw(pDC, *m_windowDpi, this, rcText.left, rcText.top, false, false);
				rcText.left += m_dontDeleteImage.ImageWidth() + m_windowDpi->Scale(2);
			}
			if (strSymbols.Find(_T("<shortcut>")) >= 0) // has shortcut
			{
				m_shortCutImage.Draw(pDC, *m_windowDpi, this, rcText.left, rcText.top, false, false);
				rcText.left += m_shortCutImage.ImageWidth() + m_windowDpi->Scale(2);
			}
			if (drawInGroupIcon &&
				strSymbols.Find(_T("<ingroup>")) >= 0) // in group
			{
				m_inFolderImage.Draw(pDC, *m_windowDpi, this, rcText.left, rcText.top, false, false);
				rcText.left += m_inFolderImage.ImageWidth() + m_windowDpi->Scale(2);
			}
			if (strSymbols.Find(_T("<qpastetext>")) >= 0) // has quick paste text
			{
			}
			if (strSymbols.Find(_T("<sticky>")) >= 0) //sticky clip
			{
				m_stickyImage.Draw(pDC, *m_windowDpi, this, rcText.left, rcText.top, false, false);
				rcText.left += m_stickyImage.ImageWidth() + m_windowDpi->Scale(2);
			}
		}

		if (DrawRtfText(nItem, rcText, pDC) == FALSE)
		{
			auto highlightColor = CGetSetOptions::m_Theme.SearchTextHighlight();
			//use unprintable characters so it doesn't find copied html to convert
			if (m_searchText.GetLength() > 0 &&
				FindNoCaseAndInsert(csText, m_searchText, StrF(_T("\x01\x04 color='#%02x%02x%02x'\x02"), GetRValue(highlightColor), GetGValue(highlightColor), GetBValue(highlightColor)), _T("\x01\x03\x04\x02"), m_linesPerRow) > 0)
			{
				DrawHTML(pDC->m_hDC, csText, csText.GetLength(), rcText, DT_VCENTER | DT_EXPANDTABS | DT_NOPREFIX);
			}
			else
			{
				pDC->DrawText(csText, rcText, DT_VCENTER | DT_EXPANDTABS | DT_NOPREFIX);
			}
		}

		// Draw a focus rect around the item if necessary.
		//if(bListHasFocus && (rItem.state & LVIS_FOCUSED))
		//	pDC->DrawFocusRect(rcItem);

		if (m_bShowTextForFirstTenHotKeys && firstTenNum >= 0)
		{
			CString cs;
			if (firstTenNum == 10)
				cs = "0";
			else
				cs.Format(_T("%d"), firstTenNum);

			CRect crClient;

			GetWindowRect(crClient);
			ScreenToClient(crClient);

			CRect crHotKey = rcItem;

			int extraFromClipWasPaste = 0;
			if (m_showIfClipWasPasted)
				extraFromClipWasPaste = 3;

			crHotKey.right = crHotKey.left + m_windowDpi->Scale(11);
			crHotKey.left += m_windowDpi->Scale(1 + extraFromClipWasPaste);
			crHotKey.top += m_windowDpi->Scale(1 + extraFromClipWasPaste);

			HFONT hOldFont = (HFONT)pDC->SelectObject(m_SmallFont);
			COLORREF localOldTextColor = pDC->SetTextColor(CGetSetOptions::m_Theme.ListSmallQuickPasteIndexColor());

			CPen pen(PS_SOLID, 0, CGetSetOptions::m_Theme.ListSmallQuickPasteIndexColor());
			CPen* pOldPen = pDC->SelectObject(&pen);

			pDC->DrawText(cs, crHotKey, DT_BOTTOM);

			pDC->MoveTo(CPoint(rcItem.left + m_windowDpi->Scale(8 + extraFromClipWasPaste), rcItem.top));
			pDC->LineTo(CPoint(rcItem.left + m_windowDpi->Scale(8 + extraFromClipWasPaste), rcItem.bottom));

			pDC->SelectObject(hOldFont);
			pDC->SetTextColor(localOldTextColor);
			pDC->SelectObject(pOldPen);
		}

		// restore the previous values
		if (OldColor > -1)
			pDC->SetTextColor(OldColor);

		if (nOldBKMode > -1)
			pDC->SetBkMode(nOldBKMode);

		*pResult = CDRF_SKIPDEFAULT;    // We've painted everything.
	}
}


// Helper function implementation to check for valid hex characters
bool CQListCtrl::IsHexString(const CString& str)
{
	if (str.IsEmpty()) {
		return false;
	}
	for (int i = 0; i < str.GetLength(); ++i) {
		if (!iswxdigit(str[i])) {
			return false;
		}
	}
	return true;
}

// Helper function implementation for HSL to RGB conversion
COLORREF CQListCtrl::HslToRgb(double h, double s, double l)
{
	// Input h(0-360), s(0-1), l(0-1)
	// Output COLORREF (RGB)

	double r, g, b;

	if (s == 0)
	{
		r = g = b = l; // Achromatic (gray)
	}
	else
	{
		auto hue2rgb = [&](double p, double q, double t)
			{
				if (t < 0) t += 1;
				if (t > 1) t -= 1;
				if (t < 1.0 / 6.0) return p + (q - p) * 6 * t;
				if (t < 1.0 / 2.0) return q;
				if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6;
				return p;
			};

		// Normalize h to 0-1 range
		double h_norm = h / 360.0;

		double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
		double p = 2 * l - q;
		r = hue2rgb(p, q, h_norm + 1.0 / 3.0);
		g = hue2rgb(p, q, h_norm);
		b = hue2rgb(p, q, h_norm - 1.0 / 3.0);
	}

	// Scale RGB values to 0-255 and round
	int R = static_cast<int>(std::round(r * 255));
	int G = static_cast<int>(std::round(g * 255));
	int B = static_cast<int>(std::round(b * 255));

	// Clamp values to 0-255 just in case of floating point inaccuracies
	R = max(0, min(255, R));
	G = max(0, min(255, G));
	B = max(0, min(255, B));

	return RGB(R, G, B);
}


void CQListCtrl::DrawCopiedColorCode(CString& csText, CRect& rcText, CDC* pDC)
{
	if (CGetSetOptions::m_bDrawCopiedColorCode == FALSE || csText.IsEmpty())
		return;

	// 1. Initial Cleaning (Remove specific surrounding chars and leading/trailing whitespace)
	CString cleanedText = csText;
	cleanedText.Trim(_T("»")); // Remove potential ditto mark first
	cleanedText.Trim();        // Trim leading/trailing whitespace
	cleanedText.TrimRight(_T(";")); // Remove trailing semicolon
	cleanedText.Trim();        // Trim again in case semicolon had spaces after it

	// Store original case potentially for display, but use lower for parsing
	CString originalCleanedText = cleanedText;
	CString parseText = cleanedText;
	parseText.MakeLower(); // Use lower case for keyword matching ('rgb', 'hsl', '0x', '#')

	// 2. Helper function to draw the color box
	auto DrawColorBox = [&](COLORREF color)
		{
			CRect pastedRect(rcText);
			int boxSize = rcText.Height();
			// Prevent overly large boxes if text rect somehow becomes huge
			//boxSize = min(boxSize, m_windowDpi->Scale(20));
			pastedRect.right = pastedRect.left + boxSize;
			pastedRect.bottom = pastedRect.top + boxSize; // Make it square

			pDC->FillSolidRect(pastedRect, color);

			// Adjust text rect to be after the color box + padding
			rcText.left += boxSize;
			rcText.left += m_windowDpi->Scale(ROW_LEFT_BORDER);
			// Restore original (potentially mixed-case) text for drawing
			csText = originalCleanedText;
		};

	// 3. --- Hex Color Parsing ---
	CString hexString;
	bool isHex = false;
	bool is0xPrefix = false; // Flag to track prefix type ('0x' vs '#')
	int originalHexLength = 0;

	if (parseText.GetLength() >= 2 && parseText.Left(2) == _T("0x"))
	{
		hexString = parseText.Mid(2); // Get part after 0x
		originalHexLength = hexString.GetLength();
		// Supported lengths for 0x: 3 (RGB -> RRGGBB), 6 (RRGGBB), 8 (AARRGGBB)
		// Exclude length 4 (0xRGBA) as it's ambiguous/non-standard
		if ((originalHexLength == 3 || originalHexLength == 6 || originalHexLength == 8) && IsHexString(hexString))
		{
			isHex = true;
			is0xPrefix = true;
			// Expand 0xRGB -> RRGGBB (will assign Alpha later)
			if (originalHexLength == 3)
			{
				hexString.Format(_T("%c%c%c%c%c%c"), hexString[0], hexString[0], hexString[1], hexString[1], hexString[2], hexString[2]);
				// Length becomes 6
			}
			// No expansion needed for 6 (0xRRGGBB) or 8 (0xAARRGGBB)
		}
	}
	else if (parseText.GetLength() >= 1 && parseText.Left(1) == _T("#"))
	{
		hexString = parseText.Mid(1); // Get part after #
		originalHexLength = hexString.GetLength();
		// Supported lengths for #: 3 (RGB), 4 (RGBA), 6 (RRGGBB), 8 (RRGGBBAA)
		if ((originalHexLength == 3 || originalHexLength == 4 || originalHexLength == 6 || originalHexLength == 8) && IsHexString(hexString))
		{
			isHex = true;
			is0xPrefix = false; // It's a # prefix
			// Expand #RGB -> RRGGBB
			if (originalHexLength == 3)
			{
				hexString.Format(_T("%c%c%c%c%c%c"), hexString[0], hexString[0], hexString[1], hexString[1], hexString[2], hexString[2]);
				// Length becomes 6
			}
			// Expand #RGBA -> RRGGBBAA
			else if (originalHexLength == 4)
			{
				hexString.Format(_T("%c%c%c%c%c%c%c%c"), hexString[0], hexString[0], hexString[1], hexString[1], hexString[2], hexString[2], hexString[3], hexString[3]);
				// Length becomes 8
			}
			// No expansion needed for 6 (#RRGGBB) or 8 (#RRGGBBAA)
		}
	}

	if (isHex)
	{
		int r = 0, g = 0, b = 0, a = 255; // Default alpha to fully opaque
		int scanRet = 0;
		unsigned int hexValue = 0;
		int finalHexLength = hexString.GetLength(); // Length after potential expansion

		// At this point, hexString should contain 6 or 8 hex digits if isHex is true
		if (finalHexLength != 6 && finalHexLength != 8) {
			// This case should not be reached if the logic above is correct, but acts as a safeguard
			isHex = false;
		}
		else {
			// Use swscanf to parse the final 6 or 8 digit hex string
			scanRet = swscanf(hexString, _T("%x"), &hexValue);
		}

		if (isHex && scanRet == 1)
		{
			if (finalHexLength == 8) // 8 digits: AARRGGBB (for 0x) or RRGGBBAA (for #)
			{
				if (is0xPrefix) { // Handle 0xAARRGGBB
					a = (hexValue >> 24) & 0xFF;
					r = (hexValue >> 16) & 0xFF;
					g = (hexValue >> 8) & 0xFF;
					b = hexValue & 0xFF;
				} else { // Handle #RRGGBBAA (or expanded #RGBA)
					r = (hexValue >> 24) & 0xFF;
					g = (hexValue >> 16) & 0xFF;
					b = (hexValue >> 8) & 0xFF;
					a = hexValue & 0xFF;
				}
			}
			else // 6 digits (RRGGBB - from #RGB, 0xRGB, #RRGGBB, 0xRRGGBB)
			{
				r = (hexValue >> 16) & 0xFF;
				g = (hexValue >> 8) & 0xFF;
				b = hexValue & 0xFF;
				a = 255; // Explicitly set full opacity for all 6-digit formats
			}

			// Basic validation on parsed values (should be redundant if IsHexString worked)
			if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255 && a >= 0 && a <= 255) {
				DrawColorBox(RGB(r, g, b)); // Draw using RGB, ignore alpha for the solid box
				return; // Valid hex format found and drawn
			}
		}
	}


	// 4. --- RGB Color Parsing ---
	// Formats: rgb(R, G, B), rgba(R, G, B, A), rgb(R G B), rgb(R G B / A)
	// R, G, B are 0-255. A is 0.0-1.0
	// Percentage formats like rgb(100%, 0%, 0%) are NOT handled by this swscanf logic.
	if (parseText.Left(3) == _T("rgb") && parseText.Right(1) == _T(")"))
	{
		CString content;
		bool isRgba = parseText.Left(4) == _T("rgba");
		int prefixLen = isRgba ? 5 : 4; // Length of "rgba(" or "rgb("
		content = parseText.Mid(prefixLen, parseText.GetLength() - prefixLen - 1); // Extract content within ()
		content.Trim(); // Trim spaces within parentheses

		int r = -1, g = -1, b = -1;
		double a_double = 1.0; // Default alpha
		int scanRet = 0;
		bool parsed = false;

		// Try parsing comma-separated format first (most common)
		// Check if it contains commas before trying swscanf
		if (content.Find(',') != -1) {
			scanRet = swscanf(content, _T("%d , %d ,%d , %lf"), &r, &g, &b, &a_double); // rgba(R, G, B, A)
			if (scanRet >= 3) { // Need at least R, G, B
				parsed = true;
				if (isRgba && scanRet != 4) parsed = false; // rgba() must have 4 values
				if (!isRgba && scanRet == 4) parsed = false; // rgb() must not have 4 values
			}

			if (!parsed) {
				scanRet = swscanf(content, _T("%d %% , %d %% , %d %% , %lf %%"), &r, &g, &b, &a_double);
				if (scanRet >= 3) {
					r = ((double)r / 100) * 255;
					g = ((double)g / 100) * 255;
					b = ((double)b / 100) * 255;
					a_double = (a_double / 100) * 1;
					parsed = true;
					if (isRgba && scanRet != 4) parsed = false; // rgba() must have 4 values
					if (!isRgba && scanRet == 4) parsed = false; // rgb() must not have 4 values
				}
			}
		}

		// Try parsing space-separated format if comma parse failed or wasn't attempted
		// Check for slash for alpha separation
		if (!parsed && content.Find('/') != -1) {
			scanRet = swscanf(content, _T("%d %d %d / %lf"), &r, &g, &b, &a_double); // rgb(R G B / A)
			if (scanRet == 4) { // Must have exactly R G B / A
			    // This format is valid for both rgb() and rgba() in modern CSS, check values
				parsed = true;
			}
		}
		else if (!parsed) { // Try space separated without alpha
			scanRet = swscanf(content, _T("%d %d %d"), &r, &g, &b); // rgb(R G B)
			if (scanRet == 3) { // Must have exactly R G B
				// Check for extraneous characters after the numbers
				CString check_str;
				check_str.Format(_T("%d %d %d"), r, g, b);
				if (content == check_str) // Ensure the whole string was consumed
				{
					parsed = true;
					a_double = 1.0; // Set default alpha
				}
			}
		}

		if (parsed)
		{
			// Validate ranges: R, G, B (0-255), A (0.0-1.0)
			if (r >= 0 && r <= 255 &&
				g >= 0 && g <= 255 &&
				b >= 0 && b <= 255 &&
				a_double >= 0.0 && a_double <= 1.0)
			{
				DrawColorBox(RGB(r, g, b)); // Draw using RGB part
				return; // Valid RGB/RGBA format found and drawn
			}
		}
	}

	//draw hex color copied like #FF0000, FF0000, FF0000 other text with a space before other text
	if (parseText.GetLength() == 6 ||
		parseText.Find(' ') == 6)
	{
		int r, g, b;
		int scanRet = swscanf(parseText, _T("%02x%02x%02x"), &r, &g, &b);
		if (scanRet == 3)
		{
			DrawColorBox(RGB(r, g, b)); // Draw using RGB part
			return; // Valid RGB/RGBA format found and drawn
		}
	}

	// 5. --- HSL Color Parsing ---
	// Formats: hsl(H, S%, L%), hsla(H, S%, L%, A), hsl(H S% L%), hsl(H S% L% / A)
	// Also supports 'deg' suffix for Hue.
	// H is 0-360, S/L are 0-100%, A is 0.0-1.0
	if (parseText.Left(3) == _T("hsl") && parseText.Right(1) == _T(")"))
	{
		CString content;
		bool isHsla = parseText.Left(4) == _T("hsla");
		int prefixLen = isHsla ? 5 : 4; // Length of "hsla(" or "hsl("
		content = parseText.Mid(prefixLen, parseText.GetLength() - prefixLen - 1); // Extract content within ()
		content.Trim(); // Trim spaces within parentheses

		int h = -1;
		double s = -1.0, l = -1.0, a_double = 1.0; // Default alpha
		int scanRet = 0;
		bool parsed = false;
		wchar_t degSuffix[5] = { 0 }; // To capture 'deg' if present
		wchar_t sPercent = 0, lPercent = 0; // To capture '%' signs

		// --- Try parsing comma-separated formats ---
		if (!parsed && content.Find(',') != -1) {
			// hsla(Hdeg, S%, L%, A)
			scanRet = swscanf(content, _T("%ddeg , %lf %% , %lf %% , %lf"), &h, &s, &l, &a_double);
			if (scanRet >= 3) { // H, S, L required
				parsed = true;
				if (isHsla && scanRet != 4) parsed = false; // hsla requires 4
				if (!isHsla && scanRet == 4) parsed = false; // hsl requires 3
			}

			// hsla(H, S%, L%, A) - No 'deg'
			if (!parsed) {
				scanRet = swscanf(content, _T("%d , %lf %% , %lf %% , %lf"), &h, &s, &l, &a_double);
				if (scanRet >= 3) {
					parsed = true;
					if (isHsla && scanRet != 4) parsed = false;
					if (!isHsla && scanRet == 4) parsed = false;
				}
			}
		}

		// --- Try parsing space-separated formats ---
		if (!parsed && content.Find(',') == -1) { // Only if no commas found
			// hsl(Hdeg S% L% / A)
			scanRet = swscanf(content, _T("%ddeg %lf %% %lf %% / %lf"), &h, &s, &l, &a_double);
			if (scanRet == 4) { // Must match exactly 4 components
				parsed = true;
			}

			// hsl(H S% L% / A) - No 'deg'
			if (!parsed) {
				scanRet = swscanf(content, _T("%d %lf %% %lf %% / %lf"), &h, &s, &l, &a_double);
				if (scanRet == 4) {
					parsed = true;
				}
			}

			// hsl(Hdeg S% L%) - No alpha
			if (!parsed) {
				// Need to ensure the *entire* string is consumed to avoid matching prefix of alpha version
				// We store the scanned values and reconstruct to compare
				scanRet = swscanf(content, _T("%ddeg %lf %% %lf %%"), &h, &s, &l);
				if (scanRet == 3) {
					CString check_str;
					// Format considering potential floating point variations slightly
					// However, simple comparison might be sufficient if input is clean
					check_str.Format(_T("%ddeg %.0f%% %.0f%%"), h, s, l);
					// Crude check - might fail if input has different spacing or precision
					// A better check involves checking remaining chars after scan, but swscanf makes this hard.
					// Let's try a simpler length/content check for now.
					CString temp_content = content;
					temp_content.Remove(' '); // Remove spaces for comparison
					CString temp_check = check_str;
					temp_check.Remove(' ');
					if (temp_content == temp_check) { // Basic check if format matches structure
						parsed = true;
						a_double = 1.0; // Set default alpha
					}
				}
			}

			// hsl(H S% L%) - No 'deg', No alpha
			if (!parsed) {
				scanRet = swscanf(content, _T("%d %lf %% %lf %%"), &h, &s, &l);
				if (scanRet == 3) {
					CString check_str;
					check_str.Format(_T("%d %.0f%% %.0f%%"), h, s, l);
					CString temp_content = content;
					temp_content.Remove(' ');
					CString temp_check = check_str;
					temp_check.Remove(' ');
					if (temp_content == temp_check) {
						parsed = true;
						a_double = 1.0;
					}
				}
			}
		}

		if (parsed)
		{
			// Validate ranges: H (any number, will be normalized), S/L (0-100), A (0.0-1.0)
			// Note: Hue can be negative or > 360, it will be normalized later.
			if (s >= 0.0 && s <= 100.0 &&
				l >= 0.0 && l <= 100.0 &&
				a_double >= 0.0 && a_double <= 1.0)
			{
				// Normalize HSL values for conversion
				h = h % 360; // Hue: Normalize to 0-359
				if (h < 0) {
					h += 360;
				}
				double s_norm = s / 100.0; // Saturation: 0.0 - 1.0
				double l_norm = l / 100.0; // Lightness: 0.0 - 1.0

				COLORREF rgbColor = HslToRgb(static_cast<double>(h), s_norm, l_norm);
				DrawColorBox(rgbColor);
				return; // Valid HSL/HSLA format found and drawn
			}
		}
	}

	// draw rgb color copied like 255,0,0
	int firstCommaPos = parseText.Find(',');
	if (firstCommaPos >= 0)
	{
		int secondCommaPos = parseText.Find(',', firstCommaPos + 1);
		if (secondCommaPos)
		{
			int noThirdCommaPos = parseText.Find(',', secondCommaPos + 1);

			if (noThirdCommaPos < 0)
			{
				int r, g, b;
				int scanRet = swscanf(parseText, _T("%d,%d,%d"), &r, &g, &b);
				if (scanRet == 3)
				{
					DrawColorBox(RGB(r, g, b)); // Draw using RGB part
					return; // Valid RGB/RGBA format found and drawn
				}
			}
		}
	}

	// If we reach here, no valid format was detected or parsed correctly.
	// The original csText remains unchanged, and no color box is drawn.
}

BOOL CQListCtrl::DrawRtfText(int nItem, CRect& crRect, CDC* pDC)
{
	if (CGetSetOptions::m_bDrawRTF == FALSE)
		return FALSE;

	BOOL bRet = FALSE;

	CClipFormat* pThumbnail = GetItem_CF_RTF_ClipFormat(nItem);
	if (pThumbnail == NULL)
		return FALSE;

	// if there's no data, then we're done.
	if (pThumbnail->m_hgData == NULL)
		return FALSE;

	if (m_pFormatter == NULL)
	{
		m_pFormatter = new CFormattedTextDraw;
		m_pFormatter->Create();
	}

	if (m_rtfFormater.m_hWnd == NULL)
	{
		m_rtfFormater.Create(_T(""), _T(""), WS_CHILD | WS_VSCROLL |
			WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL |
			ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, -1);
	}

	if (m_pFormatter)
	{
		//somehow ms word places crazy rtf text onto the clipboard and our draw routine doesn't handle that
		//pass the rtf text into a richtext control and get it out and the contorl will clean  the rtf so our routine can draw it
		m_rtfFormater.SetRTF(pThumbnail->GetAsCStringA());
		CString betterRTF = m_rtfFormater.GetRTF();

		CComBSTR bStr(betterRTF);
		m_pFormatter->put_RTFText(bStr);

		m_pFormatter->Draw(pDC->m_hDC, crRect);

		bRet = TRUE;
	}

	return bRet;
}

// DrawBitMap loads a DIB from the DB, draws a crRect thumbnail of the image
//  to pDC and caches that thumbnail as a DIB in m_ThumbNails[ ItemID ].
// ALL items are cached in m_ThumbNails (those without images are cached with NULL m_hgData)
BOOL CQListCtrl::DrawBitMap(int nItem, CRect& crRect, CDC* pDC, const CString& csDescription)
{
	if (CGetSetOptions::m_bDrawThumbnail == FALSE)
		return FALSE;

	CClipFormatQListCtrl* format = GetItem_CF_DIB_ClipFormat(nItem);
	if (format != NULL)
	{
		HGLOBAL smallImage = format->GetDibFittingToHeight(pDC, crRect.Height());
		if (smallImage != NULL)
		{
			//Will return the width of the bitmap in nWidth
			int nWidth = 0;
			if (CBitmapHelper::DrawDIB(pDC, smallImage, crRect.left, crRect.top, nWidth))
			{
				// adjust the rect so other information can be drawn next to the thumbnail
				crRect.left += nWidth + 3;
			}
		}
	}
	else if (csDescription.Find(_T("CF_DIB")) == 0)
	{
		crRect.left += crRect.Height();
	}

	return TRUE;
}

void CQListCtrl::RefreshVisibleRows()
{
	int nTopIndex = GetTopIndex();
	int nLastIndex = nTopIndex + GetCountPerPage();
	RedrawItems(nTopIndex, nLastIndex);
}

void CQListCtrl::RefreshRow(int row)
{
	RedrawItems(row, row);
}

void CQListCtrl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CListCtrl::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CQListCtrl::OnEraseBkgnd(CDC* pDC)
{

	CRect rect;
	GetClientRect(&rect);
	CBrush myBrush(CGetSetOptions::m_Theme.MainWindowBG());    // dialog background color
	CBrush* pOld = pDC->SelectObject(&myBrush);
	BOOL bRes = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOld);    // restore old brush
	return bRes;                       // CDialog::OnEraseBkgnd(pDC);

	// Simply returning TRUE seems OK since we do custom item
	//	painting.  However, there is a pixel buffer around the
	//	border of this control (not within the item rects)
	//	which becomes visually corrupt if it is not erased.

	// In most cases, I do not notice the erasure, so I have kept
	//	the call to CListCtrl::OnEraseBkgnd(pDC);

	// However, for some reason, bulk erasure is very noticeable when
	//	shift-scrolling the page to select a block of items, so
	//	I made a special case for that:
	//if(GetSelectedCount() >= 2)
	//	return TRUE;
	//return CListCtrl::OnEraseBkgnd(pDC);
}

BOOL CQListCtrl::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;

	UINT_PTR nID = pNMHDR->idFrom;

	if (nID == 0)	  	// Notification in NT from automatically
		return FALSE;   	// created tooltip

	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 500);

	if (CGetSetOptions::m_tooltipTimeout > 0)
	{
		::SendMessage(pNMHDR->hwndFrom, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM(CGetSetOptions::m_tooltipTimeout, 0));
	}

	// Use Item's name as the tool tip. Change this for something different.
	// Like use its file size, etc.
	GetToolTipText((int)nID - 1, strTipText);

	//Replace the tabs with spaces, the tooltip didn't like the \t s
	strTipText.Replace(_T("\t"), _T("  "));

	int nLength = strTipText.GetLength() + 2;

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
	{
		if (m_pchTip != NULL)
			delete m_pchTip;

		m_pchTip = new TCHAR[nLength];
		lstrcpyn(m_pchTip, strTipText, nLength - 1);
		m_pchTip[nLength - 1] = 0;
		pTTTW->lpszText = (WCHAR*)m_pchTip;
	}
	else
	{
		if (m_pwchTip != NULL)
			delete m_pwchTip;

		m_pwchTip = new WCHAR[nLength];
		_mbstowcsz(m_pwchTip, strTipText, nLength - 1);
		m_pwchTip[nLength - 1] = 0; // end of text
		pTTTW->lpszText = (WCHAR*)m_pwchTip;
	}
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
	{
		if (m_pchTip != NULL)
			delete m_pchTip;

		m_pchTip = new TCHAR[nLength];
		STRNCPY(m_pchTip, strTipText, nLength - 1);
		m_pchTip[nLength - 1] = 0; // end of text
		pTTTW->lpszText = (LPTSTR)m_pchTip;
	}
	else
	{
		if (m_pwchTip != NULL)
			delete m_pwchTip;

		m_pwchTip = new WCHAR[nLength];
		lstrcpyn(m_pwchTip, strTipText, nLength - 1);
		m_pwchTip[nLength - 1] = 0;
		pTTTW->lpszText = (LPTSTR)m_pwchTip;
	}
#endif
	* pResult = 0;

	return TRUE;    // message was handled
}

INT_PTR CQListCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rect;
	GetClientRect(&rect);
	if (rect.PtInRect(point))
	{
		if (GetItemCount())
		{
			int nTopIndex = GetTopIndex();
			int nBottomIndex = nTopIndex + GetCountPerPage();
			if (nBottomIndex > GetItemCount()) nBottomIndex = GetItemCount();
			for (int nIndex = nTopIndex; nIndex <= nBottomIndex; nIndex++)
			{
				GetItemRect(nIndex, rect, LVIR_BOUNDS);
				if (rect.PtInRect(point))
				{
					pTI->hwnd = m_hWnd;
					pTI->uId = (UINT)(nIndex + 1);
					pTI->lpszText = LPSTR_TEXTCALLBACK;
					pTI->rect = rect;
					pTI->uFlags = TTF_TRANSPARENT;
					return pTI->uId;
				}
			}
		}
	}

	return -1;
}

int CQListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (CGetSetOptions::m_tooltipTimeout > 0 ||
		CGetSetOptions::m_tooltipTimeout == -1)
	{
		EnableToolTips();
	}
	else
	{
		EnableToolTips(FALSE);
	}

	//m_pToolTip = new CToolTipEx;
	//m_pToolTip->Create(this);

	//m_pToolTip->SetNotifyWnd(GetParent());

	return 0;
}

BOOL CQListCtrl::PreTranslateMessage(MSG* pMsg)
{
	CAccel a;
	if (m_Accels.OnMsg(pMsg, a))
	{
		switch (a.Cmd)
		{
		case COPY_BUFFER_HOT_KEY_1_ID:
			PutSelectedItemOnDittoCopyBuffer(0);
			break;
		case COPY_BUFFER_HOT_KEY_2_ID:
			PutSelectedItemOnDittoCopyBuffer(1);
			break;
		case COPY_BUFFER_HOT_KEY_3_ID:
			PutSelectedItemOnDittoCopyBuffer(2);
			break;
		default:
			if (a.RefId == CHotKey::PASTE_OPEN_CLIP)
			{
				GetParent()->SendMessage(NM_SELECT_DB_ID, a.Cmd, 0);
			}
			else if (a.RefId == CHotKey::MOVE_TO_GROUP)
			{
				GetParent()->SendMessage(NM_MOVE_TO_GROUP, a.Cmd, 0);
			}
		}

		return TRUE;
	}

	if (VALID_TOOLTIP)
	{
		if (m_pToolTip->OnMsg(pMsg))
			return TRUE;
	}

	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (HandleKeyDown(pMsg->wParam, pMsg->lParam))
			return TRUE;
		break; // end case WM_KEYDOWN
	case WM_MOUSEWHEEL:
		break;

	case WM_VSCROLL:
		ASSERT(FALSE);
		break;
	} // end switch(pMsg->message)

	return CListCtrl::PreTranslateMessage(pMsg);
}

BOOL CQListCtrl::HandleKeyDown(WPARAM wParam, LPARAM lParam)
{
	if (VALID_TOOLTIP)
	{
		MSG Msg;
		Msg.lParam = lParam;
		Msg.wParam = wParam;
		Msg.message = WM_KEYDOWN;
		if (m_pToolTip->OnMsg(&Msg))
			return TRUE;
	}

	WPARAM vk = wParam;

	switch (vk)
	{
	case 'A': // Ctrl-A = Select All
		if (CONTROL_PRESSED)
		{
			int nCount = GetItemCount();
			for (int i = 0; i < nCount; i++)
			{
				SetSelection(i);
			}
			return TRUE;
		}
		break;

	case VK_HOME:

		if (GetKeyState(VK_SHIFT) & 0x8000)
		{
			int pos = (int)GetFirstSelectedItemPosition();
			if (pos >= 0 && pos < GetItemCount())
			{
				for (int i = 0; i < pos; i++)
				{
					SetSelection(i, (i == 0));
				}
			}

		}
		else
		{
			SetListPos(0);
		}
		break;
	} // end switch(vk)

	return FALSE;
}

bool CQListCtrl::PostEventLoadedCheckDescription(int updatedRow)
{
	bool loadedClip = false;

	if (VALID_TOOLTIP)
	{
		int toolTipClipId = m_pToolTip->GetClipId();
		int toolTipClipRow = m_pToolTip->GetClipRow();

		if (toolTipClipRow >= 0)
		{
			log(StrF(_T("PostEventLoadedCheckDescription refreshRow: %d tt_row: %d tt_id: %d"), updatedRow, toolTipClipRow, toolTipClipId));
		}

		//We tried to show the clip but we didn't have the id yet, it was loaded in a thread, now it's being updated
		//see if we need to show this rows description
		if (toolTipClipId <= 0 &&
			toolTipClipRow == updatedRow &&
			::IsWindow(m_toolTipHwnd))
		{
			ShowFullDescription(false, true);
			loadedClip = true;
		}
	}

	return loadedClip;
}

bool CQListCtrl::ShowFullDescription(bool bFromAuto, bool fromNextPrev)
{
	if (this->GetSelectedCount() == 0)
	{
		return false;
	}

	int clipRow = this->GetCaret();
	int clipId = this->GetItemData(clipRow);

	log(StrF(_T("Show full description row: %d id: %d"), clipRow, clipId));

	if (VALID_TOOLTIP &&
		clipId > 0 &&
		m_pToolTip->GetClipId() == clipId &&
		::IsWindow(m_toolTipHwnd))
	{
		return false;
	}

	int nItem = GetCaret();
	CRect rc, crWindow;
	GetWindowRect(&crWindow);
	GetItemRect(nItem, rc, LVIR_BOUNDS);
	ClientToScreen(rc);

	CPoint pt;

	if (CGetSetOptions::GetRememberDescPos())
	{
		CGetSetOptions::GetDescWndPoint(pt);
	}
	else if (bFromAuto == false)
	{
		pt = CPoint(rc.left, rc.bottom);
	}
	else
	{
		pt = CPoint((crWindow.left + (crWindow.right - crWindow.left) / 2), rc.bottom);
	}

	CString csDescription;
	GetToolTipText(nItem, csDescription);

	if (m_pToolTip == NULL ||
		//fromNextPrev == false ||
		::IsWindow(m_toolTipHwnd) == FALSE)
	{
		if (m_pToolTip != NULL)
		{
			m_pToolTip->DestroyWindow();
		}

		m_pToolTip = new CToolTipEx;
		m_pToolTip->Create(this);
		m_toolTipHwnd = m_pToolTip->GetSafeHwnd();
		m_pToolTip->SetNotifyWnd(GetParent());
	}
	else if (VALID_TOOLTIP)
	{
		if (fromNextPrev)
		{
			CRect r;
			m_pToolTip->GetWindowRectEx(r);
			pt = r.TopLeft();
		}

		m_pToolTip->SetGdiplusBitmap(NULL);
		m_pToolTip->SetRTFText("");
		m_pToolTip->SetToolTipText(_T(""));
		m_pToolTip->SetFolderPath(_T(""));
	}

	if (VALID_TOOLTIP)
	{
		m_pToolTip->SetTooltipActions(m_pToolTipActions);
		m_pToolTip->SetClipId(clipId);
		m_pToolTip->SetClipRow(clipRow);
		m_pToolTip->SetSearchText(m_searchText);
		LOGFONT lf;
		m_Font.GetLogFont(&lf);
		lf.lfHeight = m_windowDpi->UnScale(lf.lfHeight);
		m_pToolTip->SetLogFont(&lf, FALSE);

		m_pToolTip->SetClipData(_T(""));
		m_pToolTip->SetToolTipText(_T(""));
		m_pToolTip->SetRTFText("");
		bool bSetPlainText = false;

		CClipFormat Clip;

		try
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, lDate, lastPasteDate, lDontAutoDelete, QuickPasteText, lShortCut, globalShortCut, stickyClipOrder, stickyClipGroupOrder, lParentID FROM Main WHERE lID = %d"), clipId);
			if (q.eof() == false)
			{
				CString clipData;
				COleDateTime time((time_t)q.getInt64Field(_T("lDate")));
				clipData += "Added: " + time.Format();

				COleDateTime modified((time_t)q.getInt64Field(_T("lastPasteDate")));
				clipData += _T(" | Last Used: ") + modified.Format();

				if (q.getIntField(_T("lDontAutoDelete")) > 0)
				{
					clipData += _T(" | Never Auto Delete");
				}

				CString csQuickPaste = q.getStringField(_T("QuickPasteText"));
				if (csQuickPaste.IsEmpty() == FALSE)
				{
					clipData += _T(" | Quick Paste = ");
					clipData += csQuickPaste;
				}

				int shortCut = q.getIntField(_T("lShortCut"));
				if (shortCut > 0)
				{
					clipData += _T(" | ");
					clipData += CHotKey::GetHotKeyDisplayStatic(shortCut);

					BOOL globalShortCut = q.getIntField(_T("globalShortCut"));
					if (globalShortCut)
					{
						clipData += _T(" - Global Shortcut Key");
					}
				}

				if (theApp.m_GroupID > 0)
				{
					int sticky = q.getIntField(_T("stickyClipGroupOrder"));
					if (sticky != INVALID_STICKY)
					{
						clipData += _T(" | ");
						clipData += _T(" - Sticky In Group");
					}
				}
				else
				{
					int sticky = q.getIntField(_T("stickyClipOrder"));
					if (sticky != INVALID_STICKY)
					{
						clipData += _T(" | ");
						clipData += _T(" - Sticky");
					}
				}

				int parentId = q.getIntField(_T("lParentID"));
				if (parentId > 0)
				{
					CString folder = FolderPath(parentId);

					m_pToolTip->SetFolderPath(folder);
				}

				m_pToolTip->SetClipData(clipData);
			}
		}
		CATCH_SQLITE_EXCEPTION

			Clip.m_cfType = CF_UNICODETEXT;
		if (GetClipData(nItem, Clip) && Clip.m_hgData)
		{
			m_pToolTip->SetToolTipText(Clip.GetAsCString());
			bSetPlainText = true;

			Clip.Free();
			Clip.Clear();
		}

		if (bSetPlainText == false)
		{
			Clip.m_cfType = CF_TEXT;
			if (GetClipData(nItem, Clip) && Clip.m_hgData)
			{
				CString cs(Clip.GetAsCStringA());

				m_pToolTip->SetToolTipText(cs);
				bSetPlainText = true;

				Clip.Free();
				Clip.Clear();
			}
		}

		if (bSetPlainText == false)
		{
			m_pToolTip->SetToolTipText(csDescription);
		}

		Clip.m_cfType = RegisterClipboardFormat(CF_RTF);

		if (GetClipData(nItem, Clip) && Clip.m_hgData)
		{
			m_pToolTip->SetRTFText(Clip.GetAsCStringA());

			Clip.Free();
			Clip.Clear();
		}

		m_pToolTip->SetHtmlText(_T(""));
		Clip.m_cfType = GetFormatID(_T("HTML Format"));
		if (GetClipData(nItem, Clip) && Clip.m_hgData)
		{
			CString html = CTextConvert::Utf8ToUnicode(Clip.GetAsCStringA());
			m_pToolTip->SetHtmlText(html);

			Clip.Free();
			Clip.Clear();
		}

		Clip.m_cfType = CF_DIB;
		if (GetClipData(nItem, Clip) && Clip.m_hgData)
		{
			m_pToolTip->SetGdiplusBitmap(Clip.CreateGdiplusBitmap());
		}
		else
		{
			Clip.m_cfType = theApp.m_PNG_Format;
			if (GetClipData(nItem, Clip) && Clip.m_hgData)
			{
				m_pToolTip->SetGdiplusBitmap(Clip.CreateGdiplusBitmap());
			}
		}

		m_pToolTip->Show(pt);
	}

	return true;
}

void CQListCtrl::GetToolTipText(int nItem, CString& csText)
{
	CWnd* pParent = GetParent();
	if (pParent && (pParent->GetSafeHwnd() != NULL))
	{
		CQListToolTipText info;
		memset(&info, 0, sizeof(info));
		info.hdr.code = NM_GETTOOLTIPTEXT;
		info.hdr.hwndFrom = GetSafeHwnd();
		info.hdr.idFrom = GetDlgCtrlID();
		info.lItem = nItem;
		//plus 100 for extra info - shortcut and such
		int maxCharacters = CGetSetOptions::GetMaxToolTipCharacters();
		info.cchTextMax = min(maxCharacters, CGetSetOptions::m_bDescTextSize) + 200;
		info.pszText = csText.GetBufferSetLength(info.cchTextMax);

		pParent->SendMessage(WM_NOTIFY, (WPARAM)info.hdr.idFrom, (LPARAM)&info);

		csText.ReleaseBuffer();
	}
}

BOOL CQListCtrl::GetClipData(int nItem, CClipFormat& Clip)
{
	return theApp.GetClipData(GetItemData(nItem), Clip);
}

DWORD CQListCtrl::GetItemData(int nItem)
{
	if ((GetStyle() & LVS_OWNERDATA))
	{
		CWnd* pParent = GetParent();
		if (pParent && (pParent->GetSafeHwnd() != NULL))
		{
			LV_DISPINFO info;
			memset(&info, 0, sizeof(info));
			info.hdr.code = LVN_GETDISPINFO;
			info.hdr.hwndFrom = GetSafeHwnd();
			info.hdr.idFrom = GetDlgCtrlID();

			info.item.iItem = nItem;
			info.item.lParam = -1;
			info.item.mask = LVIF_PARAM;

			pParent->SendMessage(WM_NOTIFY, (WPARAM)info.hdr.idFrom, (LPARAM)&info);

			return (DWORD)info.item.lParam;
		}
	}

	return (DWORD)CListCtrl::GetItemData(nItem);
}

CClipFormatQListCtrl* CQListCtrl::GetItem_CF_DIB_ClipFormat(int nItem)
{
	CClipFormatQListCtrl* format = NULL;

	CWnd* pParent = GetParent();
	if (pParent && (pParent->GetSafeHwnd() != NULL))
	{
		LV_DISPINFO info;
		memset(&info, 0, sizeof(info));
		info.hdr.code = LVN_GETDISPINFO;
		info.hdr.hwndFrom = GetSafeHwnd();
		info.hdr.idFrom = GetDlgCtrlID();

		info.item.iItem = nItem;
		info.item.lParam = NULL;
		info.item.mask = LVIF_CF_DIB;

		pParent->SendMessage(WM_NOTIFY, (WPARAM)info.hdr.idFrom, (LPARAM)&info);

		if (info.item.lParam != NULL)
		{
			format = (CClipFormatQListCtrl*)info.item.lParam;
		}
	}

	return format;
}

CClipFormatQListCtrl* CQListCtrl::GetItem_CF_RTF_ClipFormat(int nItem)
{
	CClipFormatQListCtrl* format = NULL;

	CWnd* pParent = GetParent();
	if (pParent && (pParent->GetSafeHwnd() != NULL))
	{
		LV_DISPINFO info;
		memset(&info, 0, sizeof(info));
		info.hdr.code = LVN_GETDISPINFO;
		info.hdr.hwndFrom = GetSafeHwnd();
		info.hdr.idFrom = GetDlgCtrlID();

		info.item.iItem = nItem;
		info.item.lParam = NULL;
		info.item.mask = LVIF_CF_RICHTEXT;

		pParent->SendMessage(WM_NOTIFY, (WPARAM)info.hdr.idFrom, (LPARAM)&info);

		if (info.item.lParam != NULL)
		{
			format = (CClipFormatQListCtrl*)info.item.lParam;
		}
	}

	return format;
}

void CQListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CQListCtrl::DestroyAndCreateAccelerator(BOOL bCreate, CppSQLite3DB& db)
{
	m_Accels.RemoveAll();

	if (bCreate)
	{
		CMainTableFunctions::LoadAcceleratorKeys(m_Accels, db);

		LoadDittoCopyBufferHotkeys();
	}
}

void CQListCtrl::LoadDittoCopyBufferHotkeys()
{
	CCopyBufferItem Item;
	CAccel a;

	CGetSetOptions::GetCopyBufferItem(0, Item);
	if (Item.m_lCopyHotKey > 0)
	{
		a.Cmd = COPY_BUFFER_HOT_KEY_1_ID;
		a.Key = Item.m_lCopyHotKey;
		m_Accels.AddAccel(a);
	}

	CGetSetOptions::GetCopyBufferItem(1, Item);
	if (Item.m_lCopyHotKey > 0)
	{
		a.Cmd = COPY_BUFFER_HOT_KEY_2_ID;
		a.Key = Item.m_lCopyHotKey;
		m_Accels.AddAccel(a);
	}

	CGetSetOptions::GetCopyBufferItem(2, Item);
	if (Item.m_lCopyHotKey > 0)
	{
		a.Cmd = COPY_BUFFER_HOT_KEY_3_ID;
		a.Key = Item.m_lCopyHotKey;
		m_Accels.AddAccel(a);
	}
}

void CQListCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CListCtrl::OnKillFocus(pNewWnd);

	//if(FocusOnToolTip() == FALSE)
		//m_pToolTip->Hide();
}

HWND CQListCtrl::GetToolTipHWnd()
{
	if (VALID_TOOLTIP)
		return m_pToolTip->GetSafeHwnd();

	return NULL;
}

BOOL CQListCtrl::SetItemCountEx(int iCount, DWORD dwFlags /* = 0 */)
{
	return CListCtrl::SetItemCountEx(iCount, dwFlags);
}

void CQListCtrl::OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pnmv = (NMLISTVIEW*)pNMHDR;

	if ((pnmv->uNewState == 3) ||
		(pnmv->uNewState == 1))
	{
		if (VALID_TOOLTIP &&
			::IsWindowVisible(m_pToolTip->m_hWnd))
		{
			this->ShowFullDescription(false, true);
		}
		if (CGetSetOptions::m_bAllwaysShowDescription)
		{
			KillTimer(TIMER_SHOW_PROPERTIES);
			SetTimer(TIMER_SHOW_PROPERTIES, 300, NULL);
		}
		if (GetSelectedCount() > 0)
			theApp.SetStatus(NULL, FALSE);
	}

	if (GetSelectedCount() == this->GetItemCount())
	{
		if (m_allSelected == false)
		{
			Log(StrF(_T("List box Select All")));

			GetParent()->SendMessage(NM_ALL_SELECTED, 0, 0);
			m_allSelected = true;
		}
	}
	else if (m_allSelected == true)
	{
		Log(StrF(_T("List box REMOVED Select All")));
		m_allSelected = false;
	}
}

void CQListCtrl::OnTimer(UINT_PTR nIDEvent)
{
	//http://support.microsoft.com/kb/200054
	//OnTimer() Is Not Called Repeatedly for a List Control
	bool callBase = true;

	switch (nIDEvent)
	{
	case TIMER_SHOW_PROPERTIES:
	{
		if (theApp.m_bShowingQuickPaste)
			ShowFullDescription(true);
		KillTimer(TIMER_SHOW_PROPERTIES);

		callBase = false;
	}
	break;

	case TIMER_HIDE_SCROL:
	{
		CPoint cursorPos;
		GetCursorPos(&cursorPos);

		CRect crWindow;
		this->GetWindowRect(&crWindow);



		//check and see if they moved out of the scroll area
		//If they did tell our parent so
		if (MouseInScrollBarArea(crWindow, cursorPos) == false)
		{
			StopHideScrollBarTimer();
		}

		callBase = false;
	}
	break;

	case TIMER_SHOW_SCROLL:
	{
		CPoint cursorPos;
		GetCursorPos(&cursorPos);

		CRect crWindow;
		this->GetWindowRect(&crWindow);

		//Adjust for the v-scroll bar being off of the screen
		crWindow.right -= m_windowDpi->Scale(GetSystemMetrics(SM_CXVSCROLL));
		crWindow.bottom -= m_windowDpi->Scale(::GetSystemMetrics(SM_CXHSCROLL));

		//Check and see if we are still in the cursor area
		if (MouseInScrollBarArea(crWindow, cursorPos))
		{
			m_timerToHideScrollAreaSet = true;
			GetParent()->SendMessage(NM_SHOW_HIDE_SCROLLBARS, 1, 0);

			//Start looking to hide the scroll bars
			SetTimer(TIMER_HIDE_SCROL, 1000, NULL);
		}

		KillTimer(TIMER_SHOW_SCROLL);

		callBase = false;
	}
	break;
	}

	if (callBase)
	{
		CListCtrl::OnTimer(nIDEvent);
	}
}

void CQListCtrl::SetLogFont(LOGFONT& font)
{
	m_Font.DeleteObject();
	m_boldFont.DeleteObject();

	m_Font.CreateFontIndirect(&font);
	font.lfWeight = 600;
	m_boldFont.CreateFontIndirect(&font);

	SetFont(&m_Font);
}

void CQListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CQListCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	NMLVCACHEHINT* pcachehint = NULL;

	if (message == WM_NOTIFY)
	{
		NMHDR* phdr = (NMHDR*)lParam;

		switch (phdr->code)
		{
		case LVN_ODCACHEHINT:
			pcachehint = (NMLVCACHEHINT*)phdr;

			GetParent()->SendMessage(NM_FILL_REST_OF_LIST, pcachehint->iFrom, pcachehint->iTo);
			return FALSE;
		}
	}

	return CListCtrl::OnChildNotify(message, wParam, lParam, pLResult);
}

BOOL CQListCtrl::OnItemDeleted(long lID)
{
	BOOL bRet2 = m_RTFData.RemoveKey(lID);

	return (bRet2);
}

void CQListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (CGetSetOptions::m_showScrollBar == FALSE)
	{
		CPoint cursorPos;
		GetCursorPos(&cursorPos);

		CRect crWindow;
		this->GetWindowRect(&crWindow);
		ScreenToClient(&crWindow);

		crWindow.right -= m_windowDpi->Scale(::GetSystemMetrics(SM_CXVSCROLL));
		crWindow.bottom -= m_windowDpi->Scale(::GetSystemMetrics(SM_CXHSCROLL));

		if (MouseInScrollBarArea(crWindow, point))
		{
			if ((GetTickCount() - m_mouseOverScrollAreaStart) > 500)
			{
				SetTimer(TIMER_SHOW_SCROLL, 500, NULL);

				m_mouseOverScrollAreaStart = GetTickCount();
			}
		}
		else
		{
			if (m_timerToHideScrollAreaSet)
			{
				StopHideScrollBarTimer();
			}
			KillTimer(TIMER_SHOW_SCROLL);
		}
	}

	CListCtrl::OnMouseMove(nFlags, point);
}

bool CQListCtrl::MouseInScrollBarArea(CRect crWindow, CPoint point)
{
	CRect crRight(crWindow);
	CRect crBottom(crWindow);

	crRight.left = crRight.right - m_windowDpi->Scale(::GetSystemMetrics(SM_CXVSCROLL));
	crBottom.top = crBottom.bottom - m_windowDpi->Scale(::GetSystemMetrics(SM_CYHSCROLL));

	/*CString cs;
	cs.Format(_T("point.x: %d, Width: %d, Height: %d\n"), point.x, crWindow.Width(), crWindow.Height());
	OutputDebugString(cs);*/

	if (crRight.PtInRect(point) || crBottom.PtInRect(point))
	{
		return true;
	}

	return false;
}

void CQListCtrl::StopHideScrollBarTimer()
{
	GetParent()->SendMessage(NM_SHOW_HIDE_SCROLLBARS, 0, 0);

	m_timerToHideScrollAreaSet = false;
	KillTimer(TIMER_HIDE_SCROL);
}

void CQListCtrl::SetSearchText(CString text)
{
	m_searchText = text;
}

void CQListCtrl::HidePopup(bool checkShowPersistant)
{
	if (VALID_TOOLTIP)
	{
		if (checkShowPersistant == false ||
			m_pToolTip->GetShowPersistant() == false)
		{
			m_pToolTip->Hide();
		}
	}
}

BOOL CQListCtrl::IsToolTipWindowVisible()
{
	if (VALID_TOOLTIP)
	{
		return ::IsWindowVisible(m_toolTipHwnd);
	}

	return FALSE;
}

void CQListCtrl::ToggleToolTipShowPersistant()
{
	if (VALID_TOOLTIP)
	{
		m_pToolTip->ToggleShowPersistant();
	}
}

bool CQListCtrl::ToggleToolTipWordWrap()
{
	bool didWordWrap = false;
	if (VALID_TOOLTIP)
	{
		didWordWrap = m_pToolTip->ToggleWordWrap();
	}

	return didWordWrap;
}


BOOL CQListCtrl::IsToolTipWindowFocus()
{
	if (VALID_TOOLTIP)
	{
		return ::GetFocus() == m_toolTipHwnd ||
			::GetParent(::GetFocus()) == m_toolTipHwnd;
	}

	return FALSE;
}

bool CQListCtrl::IsToolTipShowPersistant()
{
	if (VALID_TOOLTIP)
	{
		return m_pToolTip->GetShowPersistant();
	}

	return false;
}

void CQListCtrl::DoToolTipSearch()
{
	if (VALID_TOOLTIP)
	{
		return m_pToolTip->DoSearch();
	}
}

void CQListCtrl::HideToolTip()
{
	if (VALID_TOOLTIP)
	{
		m_pToolTip->Hide();
	}
}

void CQListCtrl::OnDpiChanged()
{
	SetDpiInfo(m_windowDpi);
}

void CQListCtrl::SetDpiInfo(CDPI* dpi)
{
	m_windowDpi = dpi;

	m_groupFolder.Reset();
	m_groupFolder.LoadStdImageDPI(m_windowDpi->GetDPI(), IDB_OPEN_FOLDER_16_16, IDB_OPEN_FOLDER_20_20, IDB_OPEN_FOLDER_24_24, IDB_OPEN_FOLDER_24_24, IDB_OPEN_FOLDER_32_32, _T("PNG"));

	m_dontDeleteImage.Reset();
	m_dontDeleteImage.LoadStdImageDPI(m_windowDpi->GetDPI(), IDB_YELLOW_STAR_16_16, IDB_YELLOW_STAR_20_20, IDB_YELLOW_STAR_24_24, IDB_YELLOW_STAR_24_24, IDB_YELLOW_STAR_32_32, _T("PNG"));

	m_inFolderImage.Reset();
	m_inFolderImage.LoadStdImageDPI(m_windowDpi->GetDPI(), IDB_IN_FOLDER_16_16, IDB_IN_FOLDER_20_20, IDB_IN_FOLDER_24_24, IDB_IN_FOLDER_24_24, IDB_IN_FOLDER_32_32, _T("PNG"));

	m_shortCutImage.Reset();
	m_shortCutImage.LoadStdImageDPI(m_windowDpi->GetDPI(), IDB_KEY_16_16, IDB_KEY_20_20, IDB_KEY_24_24, IDB_KEY_24_24, IDB_KEY_32_32, _T("PNG"));

	m_stickyImage.Reset();
	m_stickyImage.LoadStdImageDPI(m_windowDpi->GetDPI(), IDB_STICKY_16_16, IDB_STICKY_20_20, IDB_STICKY_24_24, IDB_STICKY_24_24, IDB_STICKY_32_32, _T("PNG"));

	DeleteObject(m_SmallFont);

	CreateSmallFont();
}

void CQListCtrl::CreateSmallFont()
{
	LOGFONT lf;

	lf.lfHeight = -MulDiv(CGetSetOptions::GetFirstTenHotKeysFontSize(), m_windowDpi->GetDPI(), 72);
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_LIGHT;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_STRING_PRECIS;
	lf.lfClipPrecision = CLIP_STROKE_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
	lstrcpy(lf.lfFaceName, _T("Small Font"));

	m_SmallFont = ::CreateFontIndirect(&lf);
}

void CQListCtrl::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta < 0)
	{
		this->SendMessage(WM_HSCROLL, SB_LINERIGHT, NULL);
	}
	else
	{
		this->SendMessage(WM_HSCROLL, SB_LINELEFT, NULL);
	}

	//CListCtrl::OnMouseHWheel(nFlags, zDelta, pt);
}