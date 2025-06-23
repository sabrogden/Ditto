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
#include <gdiplus.h>

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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static map to hold W3C color names and their RGB values
static std::map<CString, COLORREF> g_colorNameMap;
static CMutex g_colorNameMapMutex;

// Initializes the color map on first use
void InitializeColorNameMap()
{
	CSingleLock lock(&g_colorNameMapMutex, TRUE);
	if (g_colorNameMap.empty())
	{
        // Populate the map with W3C named colors
        // A comprehensive list of 148 colors
        g_colorNameMap[_T("black")] = RGB(0, 0, 0);
        g_colorNameMap[_T("silver")] = RGB(192, 192, 192);
        g_colorNameMap[_T("gray")] = RGB(128, 128, 128);
        g_colorNameMap[_T("white")] = RGB(255, 255, 255);
        g_colorNameMap[_T("maroon")] = RGB(128, 0, 0);
        g_colorNameMap[_T("red")] = RGB(255, 0, 0);
        g_colorNameMap[_T("purple")] = RGB(128, 0, 128);
        g_colorNameMap[_T("fuchsia")] = RGB(255, 0, 255);
        g_colorNameMap[_T("green")] = RGB(0, 128, 0);
        g_colorNameMap[_T("lime")] = RGB(0, 255, 0);
        g_colorNameMap[_T("olive")] = RGB(128, 128, 0);
        g_colorNameMap[_T("yellow")] = RGB(255, 255, 0);
        g_colorNameMap[_T("navy")] = RGB(0, 0, 128);
        g_colorNameMap[_T("blue")] = RGB(0, 0, 255);
        g_colorNameMap[_T("teal")] = RGB(0, 128, 128);
        g_colorNameMap[_T("aqua")] = RGB(0, 255, 255);
        g_colorNameMap[_T("aliceblue")] = RGB(240, 248, 255);
        g_colorNameMap[_T("antiquewhite")] = RGB(250, 235, 215);
        g_colorNameMap[_T("aquamarine")] = RGB(127, 255, 212);
        g_colorNameMap[_T("azure")] = RGB(240, 255, 255);
        g_colorNameMap[_T("beige")] = RGB(245, 245, 220);
        g_colorNameMap[_T("bisque")] = RGB(255, 228, 196);
        g_colorNameMap[_T("blanchedalmond")] = RGB(255, 235, 205);
        g_colorNameMap[_T("blueviolet")] = RGB(138, 43, 226);
        g_colorNameMap[_T("brown")] = RGB(165, 42, 42);
        g_colorNameMap[_T("burlywood")] = RGB(222, 184, 135);
        g_colorNameMap[_T("cadetblue")] = RGB(95, 158, 160);
        g_colorNameMap[_T("chartreuse")] = RGB(127, 255, 0);
        g_colorNameMap[_T("chocolate")] = RGB(210, 105, 30);
        g_colorNameMap[_T("coral")] = RGB(255, 127, 80);
        g_colorNameMap[_T("cornflowerblue")] = RGB(100, 149, 237);
        g_colorNameMap[_T("cornsilk")] = RGB(255, 248, 220);
        g_colorNameMap[_T("crimson")] = RGB(220, 20, 60);
        g_colorNameMap[_T("cyan")] = RGB(0, 255, 255);
        g_colorNameMap[_T("darkblue")] = RGB(0, 0, 139);
        g_colorNameMap[_T("darkcyan")] = RGB(0, 139, 139);
        g_colorNameMap[_T("darkgoldenrod")] = RGB(184, 134, 11);
        g_colorNameMap[_T("darkgray")] = RGB(169, 169, 169);
        g_colorNameMap[_T("darkgreen")] = RGB(0, 100, 0);
        g_colorNameMap[_T("darkkhaki")] = RGB(189, 183, 107);
        g_colorNameMap[_T("darkmagenta")] = RGB(139, 0, 139);
        g_colorNameMap[_T("darkolivegreen")] = RGB(85, 107, 47);
        g_colorNameMap[_T("darkorange")] = RGB(255, 140, 0);
        g_colorNameMap[_T("darkorchid")] = RGB(153, 50, 204);
        g_colorNameMap[_T("darkred")] = RGB(139, 0, 0);
        g_colorNameMap[_T("darksalmon")] = RGB(233, 150, 122);
        g_colorNameMap[_T("darkseagreen")] = RGB(143, 188, 143);
        g_colorNameMap[_T("darkslateblue")] = RGB(72, 61, 139);
        g_colorNameMap[_T("darkslategray")] = RGB(47, 79, 79);
        g_colorNameMap[_T("darkturquoise")] = RGB(0, 206, 209);
        g_colorNameMap[_T("darkviolet")] = RGB(148, 0, 211);
        g_colorNameMap[_T("deeppink")] = RGB(255, 20, 147);
        g_colorNameMap[_T("deepskyblue")] = RGB(0, 191, 255);
        g_colorNameMap[_T("dimgray")] = RGB(105, 105, 105);
        g_colorNameMap[_T("dodgerblue")] = RGB(30, 144, 255);
        g_colorNameMap[_T("firebrick")] = RGB(178, 34, 34);
        g_colorNameMap[_T("floralwhite")] = RGB(255, 250, 240);
        g_colorNameMap[_T("forestgreen")] = RGB(34, 139, 34);
        g_colorNameMap[_T("gainsboro")] = RGB(220, 220, 220);
        g_colorNameMap[_T("ghostwhite")] = RGB(248, 248, 255);
        g_colorNameMap[_T("gold")] = RGB(255, 215, 0);
        g_colorNameMap[_T("goldenrod")] = RGB(218, 165, 32);
        g_colorNameMap[_T("greenyellow")] = RGB(173, 255, 47);
        g_colorNameMap[_T("honeydew")] = RGB(240, 255, 240);
        g_colorNameMap[_T("hotpink")] = RGB(255, 105, 180);
        g_colorNameMap[_T("indianred")] = RGB(205, 92, 92);
        g_colorNameMap[_T("indigo")] = RGB(75, 0, 130);
        g_colorNameMap[_T("ivory")] = RGB(255, 255, 240);
        g_colorNameMap[_T("khaki")] = RGB(240, 230, 140);
        g_colorNameMap[_T("lavender")] = RGB(230, 230, 250);
        g_colorNameMap[_T("lavenderblush")] = RGB(255, 240, 245);
        g_colorNameMap[_T("lawngreen")] = RGB(124, 252, 0);
        g_colorNameMap[_T("lemonchiffon")] = RGB(255, 250, 205);
        g_colorNameMap[_T("lightblue")] = RGB(173, 216, 230);
        g_colorNameMap[_T("lightcoral")] = RGB(240, 128, 128);
        g_colorNameMap[_T("lightcyan")] = RGB(224, 255, 255);
        g_colorNameMap[_T("lightgoldenrodyellow")] = RGB(250, 250, 210);
        g_colorNameMap[_T("lightgray")] = RGB(211, 211, 211);
        g_colorNameMap[_T("lightgreen")] = RGB(144, 238, 144);
        g_colorNameMap[_T("lightpink")] = RGB(255, 182, 193);
        g_colorNameMap[_T("lightsalmon")] = RGB(255, 160, 122);
        g_colorNameMap[_T("lightseagreen")] = RGB(32, 178, 170);
        g_colorNameMap[_T("lightskyblue")] = RGB(135, 206, 250);
        g_colorNameMap[_T("lightslategray")] = RGB(119, 136, 153);
        g_colorNameMap[_T("lightsteelblue")] = RGB(176, 196, 222);
        g_colorNameMap[_T("lightyellow")] = RGB(255, 255, 224);
        g_colorNameMap[_T("limegreen")] = RGB(50, 205, 50);
        g_colorNameMap[_T("linen")] = RGB(250, 240, 230);
        g_colorNameMap[_T("magenta")] = RGB(255, 0, 255);
        g_colorNameMap[_T("mediumaquamarine")] = RGB(102, 205, 170);
        g_colorNameMap[_T("mediumblue")] = RGB(0, 0, 205);
        g_colorNameMap[_T("mediumorchid")] = RGB(186, 85, 211);
        g_colorNameMap[_T("mediumpurple")] = RGB(147, 112, 219);
        g_colorNameMap[_T("mediumseagreen")] = RGB(60, 179, 113);
        g_colorNameMap[_T("mediumslateblue")] = RGB(123, 104, 238);
        g_colorNameMap[_T("mediumspringgreen")] = RGB(0, 250, 154);
        g_colorNameMap[_T("mediumturquoise")] = RGB(72, 209, 204);
        g_colorNameMap[_T("mediumvioletred")] = RGB(199, 21, 133);
        g_colorNameMap[_T("midnightblue")] = RGB(25, 25, 112);
        g_colorNameMap[_T("mintcream")] = RGB(245, 255, 250);
        g_colorNameMap[_T("mistyrose")] = RGB(255, 228, 225);
        g_colorNameMap[_T("moccasin")] = RGB(255, 228, 181);
        g_colorNameMap[_T("navajowhite")] = RGB(255, 222, 173);
        g_colorNameMap[_T("oldlace")] = RGB(253, 245, 230);
        g_colorNameMap[_T("olivedrab")] = RGB(107, 142, 35);
        g_colorNameMap[_T("orange")] = RGB(255, 165, 0);
        g_colorNameMap[_T("orangered")] = RGB(255, 69, 0);
        g_colorNameMap[_T("orchid")] = RGB(218, 112, 214);
        g_colorNameMap[_T("palegoldenrod")] = RGB(238, 232, 170);
        g_colorNameMap[_T("palegreen")] = RGB(152, 251, 152);
        g_colorNameMap[_T("paleturquoise")] = RGB(175, 238, 238);
        g_colorNameMap[_T("palevioletred")] = RGB(219, 112, 147);
        g_colorNameMap[_T("papayawhip")] = RGB(255, 239, 213);
        g_colorNameMap[_T("peachpuff")] = RGB(255, 218, 185);
        g_colorNameMap[_T("peru")] = RGB(205, 133, 63);
        g_colorNameMap[_T("pink")] = RGB(255, 192, 203);
        g_colorNameMap[_T("plum")] = RGB(221, 160, 221);
        g_colorNameMap[_T("powderblue")] = RGB(176, 224, 230);
        g_colorNameMap[_T("rebeccapurple")] = RGB(102, 51, 153);
        g_colorNameMap[_T("rosybrown")] = RGB(188, 143, 143);
        g_colorNameMap[_T("royalblue")] = RGB(65, 105, 225);
        g_colorNameMap[_T("saddlebrown")] = RGB(139, 69, 19);
        g_colorNameMap[_T("salmon")] = RGB(250, 128, 114);
        g_colorNameMap[_T("sandybrown")] = RGB(244, 164, 96);
        g_colorNameMap[_T("seagreen")] = RGB(46, 139, 87);
        g_colorNameMap[_T("seashell")] = RGB(255, 245, 238);
        g_colorNameMap[_T("sienna")] = RGB(160, 82, 45);
        g_colorNameMap[_T("skyblue")] = RGB(135, 206, 235);
        g_colorNameMap[_T("slateblue")] = RGB(106, 90, 205);
        g_colorNameMap[_T("slategray")] = RGB(112, 128, 144);
        g_colorNameMap[_T("snow")] = RGB(255, 250, 250);
        g_colorNameMap[_T("springgreen")] = RGB(0, 255, 127);
        g_colorNameMap[_T("steelblue")] = RGB(70, 130, 180);
        g_colorNameMap[_T("tan")] = RGB(210, 180, 140);
        g_colorNameMap[_T("thistle")] = RGB(216, 191, 216);
        g_colorNameMap[_T("tomato")] = RGB(255, 99, 71);
        g_colorNameMap[_T("turquoise")] = RGB(64, 224, 208);
        g_colorNameMap[_T("violet")] = RGB(238, 130, 238);
        g_colorNameMap[_T("wheat")] = RGB(245, 222, 179);
        g_colorNameMap[_T("whitesmoke")] = RGB(245, 245, 245);
        g_colorNameMap[_T("yellowgreen")] = RGB(154, 205, 50);
	}
}

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


// Helper function to parse a CSS numeric value which can be a percentage or a number
static bool ParseCssValue(const CString& token, double& value)
{
	CString localToken = token;
	localToken.Trim();
	if (localToken.IsEmpty()) return false;

	bool isPercent = (localToken.Right(1) == _T('%'));
	if (isPercent)
	{
		localToken.TrimRight(_T('%'));
	}

	if (_stscanf(localToken, _T("%lf"), &value) == 1)
	{
		return true;
	}
	return false;
}


// Converts a color from OKLCH color space to sRGB.
// l: 0-1, c: 0-0.4 (theoretically unbounded, but practically small), h: 0-360
static COLORREF OklchToRgb(double l, double c, double h)
{
	// 1. Convert OKLCH to OKLAB
	double h_rad = h * M_PI / 180.0;
	double a = c * cos(h_rad);
	double b = c * sin(h_rad);

	// 2. Convert OKLAB to XYZ
	// First, convert to an intermediate non-linear LMS-like space
	double l_ = l + 0.3963377774 * a + 0.2158037573 * b;
	double m_ = l - 0.1055613458 * a - 0.0638541728 * b;
	double s_ = l - 0.0894841775 * a - 1.2914855480 * b;

	// Then, convert to linear LMS by undoing the cube-root non-linearity
	double l_linear = l_ * l_ * l_;
	double m_linear = m_ * m_ * m_;
	double s_linear = s_ * s_ * s_;

	// Finally, convert linear LMS to XYZ color space
	double x = +1.2270138511 * l_linear - 0.5577999807 * m_linear + 0.2812561490 * s_linear;
	double y = -0.0405801784 * l_linear + 1.1122568696 * m_linear - 0.0716766787 * s_linear;
	double z = -0.0763812845 * l_linear - 0.4214819784 * m_linear + 1.5861632204 * s_linear;

	// 3. Convert XYZ to linear sRGB
	double r_linear = +3.2404542 * x - 1.5371385 * y - 0.4985314 * z;
	double g_linear = -0.9692660 * x + 1.8760108 * y + 0.0415560 * z;
	double b_linear = +0.0556434 * x - 0.2040259 * y + 1.0572252 * z;

	// 4. Convert linear sRGB to gamma-corrected sRGB
	auto ToSrgb = [](double val) {
		if (val <= 0.0031308) {
			return 12.92 * val;
		}
		return 1.055 * pow(val, 1.0 / 2.4) - 0.055;
	};

	double r_srgb = ToSrgb(r_linear);
	double g_srgb = ToSrgb(g_linear);
	double b_srgb = ToSrgb(b_linear);

	// 5. Scale to 0-255 and clamp (for out-of-gamut colors)
	int R = static_cast<int>(std::round(r_srgb * 255.0));
	int G = static_cast<int>(std::round(g_srgb * 255.0));
	int B = static_cast<int>(std::round(b_srgb * 255.0));

	R = max(0, min(255, R));
	G = max(0, min(255, G));
	B = max(0, min(255, B));

	return RGB(R, G, B);
}


void CQListCtrl::DrawCheckerboard(CDC* pDC, CRect rect)
{
	COLORREF color1 = RGB(255, 255, 255); // White
	COLORREF color2 = RGB(204, 204, 204); // Light grey
	int squareSize = m_windowDpi->Scale(4);
	if (squareSize <= 0)
	{
		squareSize = 4;
	}

	for (int y = rect.top; y < rect.bottom; y += squareSize)
	{
		for (int x = rect.left; x < rect.right; x += squareSize)
		{
			COLORREF color = ((( (x - rect.left) / squareSize) + ((y - rect.top) / squareSize)) % 2 == 0) ? color1 : color2;
			CRect square(x, y, min(x + squareSize, rect.right), min(y + squareSize, rect.bottom));
			pDC->FillSolidRect(square, color);
		}
	}
}


void CQListCtrl::DrawCopiedColorCode(CString& csText, CRect& rcText, CDC* pDC)
{
	if (CGetSetOptions::m_bDrawCopiedColorCode == FALSE || csText.IsEmpty())
		return;

	// 1. Initial Cleaning and Prep
	CString cleanedText = csText;
	cleanedText.Trim(_T("»"));
	cleanedText.Trim();
	cleanedText.TrimRight(_T(";"));
	cleanedText.Trim();

	if (cleanedText.IsEmpty())
		return;

	CString originalCleanedText = cleanedText;
	CString parseText = cleanedText;
	parseText.MakeLower();

	// 2. Helper lambda to draw the color box
	auto DrawColorBox = [&](COLORREF color, int alpha = 255)
	{
		CRect pastedRect(rcText);
		int boxSize = rcText.Height();
		pastedRect.right = pastedRect.left + boxSize;
		pastedRect.bottom = pastedRect.top + boxSize;

		if (alpha < 255)
		{
			DrawCheckerboard(pDC, pastedRect);

			// Use GDI+ for alpha blending
			Gdiplus::Graphics graphics(pDC->GetSafeHdc());
			Gdiplus::Color gdiplusColor(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
			Gdiplus::SolidBrush brush(gdiplusColor);
			graphics.FillRectangle(&brush, Gdiplus::Rect(pastedRect.left, pastedRect.top, pastedRect.Width(), pastedRect.Height()));
		}
		else
		{
			// Opaque color is faster with FillSolidRect and doesn't need a checkerboard.
			pDC->FillSolidRect(pastedRect, color);
		}

		rcText.left += boxSize + m_windowDpi->Scale(ROW_LEFT_BORDER);
		csText = originalCleanedText;
	};

	// 3. Check for formats with unique signatures first

	// Check for W3C Named Colors
	InitializeColorNameMap();
	auto it = g_colorNameMap.find(parseText);
	if (it != g_colorNameMap.end())
	{
		DrawColorBox(it->second);
		return;
	}

	// Check for Hex Colors with # or 0x prefix
	if (parseText.Left(1) == _T('#') || parseText.Left(2) == _T("0x"))
	{
		CString hexString = parseText;
		if (hexString.Left(1) == _T('#')) hexString.Delete(0, 1);
		else if (hexString.Left(2) == _T("0x")) hexString.Delete(0, 2);

		int len = hexString.GetLength();
		if (len == 3 || len == 4) // Expand shorthand
		{
			CString expanded;
			for (int i = 0; i < len; ++i) { expanded += hexString[i]; expanded += hexString[i]; }
			hexString = expanded;
		}

		if (IsHexString(hexString) && (hexString.GetLength() == 6 || hexString.GetLength() == 8))
		{
			unsigned int r = 0, g = 0, b = 0, a = 255;
			if (hexString.GetLength() == 8)
			{
				if (swscanf(hexString, _T("%2x%2x%2x%2x"), &r, &g, &b, &a) == 4)
				{
					DrawColorBox(RGB(r, g, b), a);
					return;
				}
			}
			else // length is 6
			{
				if (swscanf(hexString, _T("%2x%2x%2x"), &r, &g, &b) == 3)
				{
					DrawColorBox(RGB(r, g, b)); // default alpha
					return;
				}
			}
		}
	}

	// Check for W3C notations: rgb(...), hsl(...), and oklch(...)
	if (parseText.Right(1) == _T(")"))
	{
		if (parseText.Left(3) == _T("rgb"))
		{
			CString content;
			int prefixLen = (parseText.Left(4) == _T("rgba")) ? 5 : 4;
			content = parseText.Mid(prefixLen, parseText.GetLength() - prefixLen - 1);
			content.Replace(_T(','), _T(' '));
			content.Replace(_T('/'), _T(' '));

			std::vector<CString> tokens;
			int curPos = 0;
			CString token;
			while (!(token = content.Tokenize(_T(" "), curPos)).IsEmpty())
			{
				tokens.push_back(token);
			}

			if (tokens.size() >= 3)
			{
				double r_val, g_val, b_val;
				if (ParseCssValue(tokens[0], r_val) && ParseCssValue(tokens[1], g_val) && ParseCssValue(tokens[2], b_val))
				{
					if (tokens[0].Find('%') != -1)
					{
						r_val = std::round(r_val * 2.55);
						g_val = std::round(g_val * 2.55);
						b_val = std::round(b_val * 2.55);
					}

					int alpha = 255;
					double a_val = 1.0;
					if (tokens.size() >= 4 && ParseCssValue(tokens[3], a_val))
					{
						if (tokens[3].Find('%') != -1)
						{
							a_val /= 100.0;
						}
						a_val = max(0.0, min(1.0, a_val));
						alpha = static_cast<int>(std::round(a_val * 255.0));
					}

					if (r_val >= 0 && r_val <= 255 && g_val >= 0 && g_val <= 255 && b_val >= 0 && b_val <= 255)
					{
						DrawColorBox(RGB((int)r_val, (int)g_val, (int)b_val), alpha);
						return;
					}
				}
			}
		}
		else if (parseText.Left(3) == _T("hsl"))
		{
			CString content;
			int prefixLen = (parseText.Left(4) == _T("hsla")) ? 5 : 4;
			content = parseText.Mid(prefixLen, parseText.GetLength() - prefixLen - 1);
			content.Replace(_T(','), _T(' '));
			content.Replace(_T('/'), _T(' '));
			content.Replace(_T("deg"), _T(""));

			std::vector<CString> tokens;
			int curPos = 0;
			CString token;
			while (!(token = content.Tokenize(_T(" "), curPos)).IsEmpty())
			{
				tokens.push_back(token);
			}

			if (tokens.size() >= 3)
			{
				double h_val, s_val, l_val;
				if (ParseCssValue(tokens[0], h_val) && ParseCssValue(tokens[1], s_val) && ParseCssValue(tokens[2], l_val))
				{
					int alpha = 255;
					double a_val = 1.0;
					if (tokens.size() >= 4 && ParseCssValue(tokens[3], a_val))
					{
						if (tokens[3].Find('%') != -1)
						{
							a_val /= 100.0;
						}
						a_val = max(0.0, min(1.0, a_val));
						alpha = static_cast<int>(std::round(a_val * 255.0));
					}

					if (s_val >= 0 && s_val <= 100 && l_val >= 0 && l_val <= 100)
					{
						h_val = fmod(h_val, 360.0);
						if (h_val < 0) h_val += 360.0;
						DrawColorBox(HslToRgb(h_val, s_val / 100.0, l_val / 100.0), alpha);
						return;
					}
				}
			}
		}
		else if (parseText.Left(5) == _T("oklch"))
		{
			CString content;
			content = parseText.Mid(6, parseText.GetLength() - 7);
			content.Replace(_T(','), _T(' '));
			content.Replace(_T('/'), _T(' '));

			std::vector<CString> tokens;
			int curPos = 0;
			CString token;
			while (!(token = content.Tokenize(_T(" "), curPos)).IsEmpty())
			{
				tokens.push_back(token);
			}

			if (tokens.size() >= 3)
			{
				double l_val, c_val, h_val;
				if (ParseCssValue(tokens[0], l_val) && ParseCssValue(tokens[1], c_val) && ParseCssValue(tokens[2], h_val))
				{
					bool l_is_percent = tokens[0].Find('%') != -1;

					double l_normalized = l_val;
					if (l_is_percent)
					{
						l_normalized = l_val / 100.0;
					}
					
					int alpha = 255;
					double a_val = 1.0;
					if (tokens.size() >= 4 && ParseCssValue(tokens[3], a_val))
					{
						if (tokens[3].Find('%') != -1)
						{
							a_val /= 100.0;
						}
						a_val = max(0.0, min(1.0, a_val));
						alpha = static_cast<int>(std::round(a_val * 255.0));
					}

					if (l_normalized >= 0 && l_normalized <= 1.0 && c_val >= 0)
					{
						DrawColorBox(OklchToRgb(l_normalized, c_val, h_val), alpha);
						return;
					}
				}
			}
		}
	}

	// 4. --- Non-W3C Format Parsing ---
	int r, g, b, chars_consumed = 0;

	// Check for parenthesized RGB: "(255, 128, 0)" or "(255 128 0)"
	if (parseText.Left(1) == _T("(") && parseText.Right(1) == _T(")"))
	{
		CString content = parseText.Mid(1, parseText.GetLength() - 2);
		content.Trim();
		content.Replace(_T(','), _T(' '));

		if (swscanf(content, _T("%d %d %d %n"), &r, &g, &b, &chars_consumed) == 3)
		{
			CString remainingText = content.Mid(chars_consumed);
			remainingText.Trim();

			if (remainingText.IsEmpty())
			{
				if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
				{
					DrawColorBox(RGB(r, g, b));
					return;
				}
			}
		}
	}

	// Check for comma-separated RGB: "255, 0, 0"
	if (swscanf(parseText, _T("%d , %d , %d %n"), &r, &g, &b, &chars_consumed) == 3 && chars_consumed == parseText.GetLength())
	{
		if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
		{
			DrawColorBox(RGB(r, g, b));
			return;
		}
	}

	// Check for space-separated RGB: "255 128 0"
	if (swscanf(parseText, _T("%d %d %d %n"), &r, &g, &b, &chars_consumed) == 3 && chars_consumed == parseText.GetLength())
	{
		if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
		{
			DrawColorBox(RGB(r, g, b));
			return;
		}
	}

	// Check for 6-digit hex: "FF00CC"
	if (parseText.GetLength() == 6 && IsHexString(parseText))
	{
		// Use %n here as well for consistency, though length check is sufficient.
		if (swscanf(parseText, _T("%2x%2x%2x%n"), &r, &g, &b, &chars_consumed) == 3 && chars_consumed == 6)
		{
			DrawColorBox(RGB(r, g, b));
			return;
		}
	}
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
			int nAnchor = GetSelectionMark();
			if (nAnchor < 0)
			{
				nAnchor = GetCaret();
			}

			if (nAnchor >= 0)
			{
				RemoveAllSelection();

				for (int i = 0; i <= nAnchor; i++)
				{
					SetSelection(i, TRUE);
				}

				ListView_SetSelectionMark(m_hWnd, nAnchor);
				
				SetCaret(0);
				EnsureVisible(0, FALSE);
			}
		}
		else
		{
			SetListPos(0);
		}
		return TRUE;
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