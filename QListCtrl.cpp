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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ROW_BOTTOM_BORDER		2
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
	
	LOGFONT lf;
	
	lf.lfHeight = theApp.m_metrics.PointsToPixels(-7);
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
		
	m_bStartTop = true;
	m_pToolTip = NULL;
	m_pFormatter = NULL;
	m_allSelected = false;
	m_rowHeight = 50;

	m_mouseOverScrollAreaStart = 0;

	m_groupFolder.LoadStdImageDPI(IDB_OPEN_FOLDER_16_16, IDB_OPEN_FOLDER_20_20, IDB_OPEN_FOLDER_24_24, IDB_OPEN_FOLDER_24_24, IDB_OPEN_FOLDER_32_32, _T("PNG"));
	m_dontDeleteImage.LoadStdImageDPI(IDB_YELLOW_STAR_16_16, IDB_YELLOW_STAR_20_20, IDB_YELLOW_STAR_24_24, IDB_YELLOW_STAR_24_24, IDB_YELLOW_STAR_32_32, _T("PNG"));
	m_inFolderImage.LoadStdImageDPI(IDB_IN_FOLDER_16_16, IDB_IN_FOLDER_20_20, IDB_IN_FOLDER_24_24, IDB_IN_FOLDER_24_24, IDB_IN_FOLDER_32_32, _T("PNG"));
	m_shortCutImage.LoadStdImageDPI(IDB_KEY_16_16, IDB_KEY_20_20, IDB_KEY_24_24, IDB_KEY_24_24, IDB_KEY_32_32, _T("PNG"));
	m_stickyImage.LoadStdImageDPI(IDB_STICKY_16_16, IDB_STICKY_20_20, IDB_STICKY_24_24, IDB_STICKY_24_24, IDB_STICKY_32_32, _T("PNG"));

	m_showIfClipWasPasted = TRUE;
	m_bShowTextForFirstTenHotKeys = true;
}

CQListCtrl::~CQListCtrl()
{
	if(m_pchTip != NULL)
		delete m_pchTip;
	
	if(m_pwchTip != NULL)
		delete m_pwchTip;
	
	if( m_SmallFont )
		::DeleteObject( m_SmallFont );
	
	m_Font.DeleteObject();

	m_boldFont.DeleteObject();

	if(m_pFormatter)
	{
		delete m_pFormatter;
		m_pFormatter = NULL;
	}
}

// returns the position 1-10 if the index is in the FirstTen block else -1
int CQListCtrl::GetFirstTenNum( int index )
{
	// set firstTenNum to the first ten number (1-10) corresponding to the given index
	int firstTenNum = -1; // -1 means that nItem is not in the FirstTen block.
	int count = GetItemCount();
	
	if( m_bStartTop )
	{
		if( 0 <= index && index <= 9 )
			firstTenNum = index + 1;
	}
	else // we are starting at the bottom and going up
	{
		int idxStartFirstTen = count-10; // start of the FirstTen block
		// if index is within the FirstTen block
		if( idxStartFirstTen <= index && index < count )
			firstTenNum = count - index;
	}
	return firstTenNum;
}

// returns the list index corresponding to the given FirstTen position number.
// (ret < 0) means that "num" is not in the FirstTen block
int CQListCtrl::GetFirstTenIndex( int num )
{
	if( num <= 0 || num > 10 )
		return -1;
	
	if( m_bStartTop )
		return num-1;
	// else we are starting at the bottom and going up
	int count = GetItemCount();
	return count - num;
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
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_WM_KILLFOCUS()
	ON_WM_MEASUREITEM_REFLECT()
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

void CQListCtrl::GetSelectionIndexes(ARRAY &arr)
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
	if(nCount > 0 && arr[0])
	{
		CDittoCopyBuffer::PutClipOnDittoCopyBuffer(arr[0], lBuffer);
		bRet = true;
	}

	return bRet;
}

void CQListCtrl::GetSelectionItemData(ARRAY &arr)
{
	DWORD dwData;
	int i;
	arr.RemoveAll();
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		i = GetNextSelectedItem(pos);
		dwData = GetItemData(i);
		arr.Add( dwData );
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
	if(bSelect)
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
	if(bFocus)
		return SetItemState(nRow, LVIS_FOCUSED, LVIS_FOCUSED);
	else
		return SetItemState(nRow, ~LVIS_FOCUSED, LVIS_FOCUSED);
}

long CQListCtrl::GetCaret()
{
	return GetNextItem(-1, LVNI_FOCUSED);
}

// moves the caret to the given index, selects it, and ensures it is visible.
BOOL CQListCtrl::SetListPos( int index )
{
	if( index < 0 || index >= GetItemCount() )
		return FALSE;

	RemoveAllSelection();
	SetCaret(index);
	SetSelection(index);
	ListView_SetSelectionMark(m_hWnd, index);
	EnsureVisible(index,FALSE);

	return TRUE;
}

BOOL CQListCtrl::SetFormattedText(int nRow, int nCol, LPCTSTR lpszFormat,...)
{
	CString csText;
	va_list vlist;
	
	ASSERT(AfxIsValidString(lpszFormat));
	va_start(vlist,lpszFormat);
	csText.FormatV(lpszFormat,vlist);
	va_end(vlist);
	
	return SetText(nRow,nCol,csText);
}

void CQListCtrl::SetNumberOfLinesPerRow(int nLines)
{
	if(m_linesPerRow != nLines)
	{
		m_linesPerRow = nLines;

		CRect rc;
		GetWindowRect( &rc );
		WINDOWPOS wp;
		wp.hwnd  = m_hWnd;
		wp.cx    = rc.Width();
		wp.cy    = rc.Height();
		wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
		SendMessage( WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp );
	}
}

void CQListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	TEXTMETRIC tm;
	HDC hDC = ::GetDC(NULL);
	CFont* pFont = GetFont();
	HFONT hFontOld = (HFONT)SelectObject(hDC, pFont->GetSafeHandle());
	GetTextMetrics(hDC, &tm);
	lpMeasureItemStruct->itemHeight = ((tm.tmHeight + tm.tmExternalLeading) * m_linesPerRow) + ROW_BOTTOM_BORDER;
	m_rowHeight = lpMeasureItemStruct->itemHeight;
	SelectObject(hDC, hFontOld);
	::ReleaseDC(NULL, hDC);
}

void CQListCtrl::OnCustomdrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
    
    *pResult = 0;
	
    // Request item-specific notifications if this is the
    // beginning of the paint cycle.
    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
        *pResult = CDRF_NOTIFYITEMDRAW;
	}
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
        LVITEM   rItem;
        int      nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );
        CDC*     pDC   = CDC::FromHandle ( pLVCD->nmcd.hdc );
        COLORREF crBkgnd;
        BOOL     bListHasFocus;
        CRect    rcItem;
		
        bListHasFocus = ( GetSafeHwnd() == ::GetFocus() );
        
        // Get the image index and selected/focused state of the
        // item being drawn.
        ZeroMemory ( &rItem, sizeof(LVITEM) );
        rItem.mask  = LVIF_STATE;
        rItem.iItem = nItem;
        rItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        GetItem(&rItem);
		
        // Get the rect that bounds the text label.
        GetItemRect(nItem, rcItem, LVIR_SELECTBOUNDS);
		
		COLORREF OldColor = -1;
		int nOldBKMode = -1;

		CString csText;
		LPTSTR lpszText = csText.GetBufferSetLength(g_Opt.m_bDescTextSize);
		GetItemText(nItem, 0, lpszText, g_Opt.m_bDescTextSize);
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
		if(rItem.state & LVIS_SELECTED)
		{
            if(bListHasFocus)
			{
                crBkgnd = g_Opt.m_Theme.ListBoxSelectedBG();
                OldColor = pDC->SetTextColor(g_Opt.m_Theme.ListBoxSelectedText());
			}
            else
			{
                crBkgnd = g_Opt.m_Theme.ListBoxSelectedNoFocusBG();
                OldColor = pDC->SetTextColor(g_Opt.m_Theme.ListBoxSelectedNoFocusText());
			}
		}
        else
		{
            //Shade alternating Rows
			if((nItem % 2) == 0)
			{
				crBkgnd = g_Opt.m_Theme.ListBoxOddRowsBG();
				OldColor = pDC->SetTextColor(g_Opt.m_Theme.ListBoxOddRowsText());
			}
			else
			{
				crBkgnd = g_Opt.m_Theme.ListBoxEvenRowsBG();
				OldColor = pDC->SetTextColor(g_Opt.m_Theme.ListBoxEvenRowsText());
			}
		}
		
        pDC->FillSolidRect(rcItem, crBkgnd);
        nOldBKMode = pDC->SetBkMode(TRANSPARENT);
		
        CRect rcText = rcItem;
        rcText.left += ROW_LEFT_BORDER;
		rcText.top++;
		
		if (m_showIfClipWasPasted &&
			strSymbols.GetLength() > 0 &&
			strSymbols.Find(_T("<pasted>")) >= 0) //clip was pasted from ditto 
		{
			CRect pastedRect(rcItem);
			pastedRect.left++;
			pastedRect.right = rcItem.left + theApp.m_metrics.ScaleX(3);
				
			pDC->FillSolidRect(pastedRect, g_Opt.m_Theme.ClipPastedColor());

			rcText.left += theApp.m_metrics.ScaleX(4);
		}
		        		
		// set firstTenNum to the first ten number (1-10) corresponding to
		//  the current nItem.
		// -1 means that nItem is not in the FirstTen block.
		int firstTenNum = GetFirstTenNum(nItem);
		
		if( m_bShowTextForFirstTenHotKeys && firstTenNum > 0 )
		{
			rcText.left += theApp.m_metrics.ScaleX(12);
		}
		
		bool drawInGroupIcon = true;
		// if we are inside a group, don't display the "in group" flag
		if( theApp.m_GroupID > 0 )
		{
			int nFlag = strSymbols.Find(_T("<ingroup>"));
			if (nFlag >= 0)
				drawInGroupIcon = false;
		}
		
		DrawBitMap(nItem, rcText, pDC, csText);			

		// draw the symbol box
		if( strSymbols.GetLength() > 0 )
		{
			if(strSymbols.Find(_T("<group>")) >= 0) //group 
			{
				m_groupFolder.Draw(pDC, this, rcText.left, rcText.top, false, false);
				rcText.left += m_groupFolder.ImageWidth() + theApp.m_metrics.ScaleX(2);
			}
			if (strSymbols.Find(_T("<noautodelete>")) >= 0 &&
				strSymbols.Find(_T("<group>")) < 0 &&
				strSymbols.Find(_T("<sticky>")) < 0) //don't auto delete
			{
				m_dontDeleteImage.Draw(pDC, this, rcText.left, rcText.top, false, false);
				rcText.left += m_dontDeleteImage.ImageWidth() + theApp.m_metrics.ScaleX(2);
			}
			if (strSymbols.Find(_T("<shortcut>")) >= 0) // has shortcut
			{
				m_shortCutImage.Draw(pDC, this, rcText.left, rcText.top, false, false);
				rcText.left += m_shortCutImage.ImageWidth() + theApp.m_metrics.ScaleX(2);
			}
			if (drawInGroupIcon &&
				strSymbols.Find(_T("<ingroup>")) >= 0) // in group
			{
				m_inFolderImage.Draw(pDC, this, rcText.left, rcText.top, false, false);
				rcText.left += m_inFolderImage.ImageWidth() + theApp.m_metrics.ScaleX(2);
			}
			if (strSymbols.Find(_T("<qpastetext>")) >= 0) // has quick paste text
			{
			}
			if (strSymbols.Find(_T("<sticky>")) >= 0) //sticky clip
			{
				m_stickyImage.Draw(pDC, this, rcText.left, rcText.top, false, false);
				rcText.left += m_stickyImage.ImageWidth() + theApp.m_metrics.ScaleX(2);
			}			
		}
		
		if(DrawRtfText(nItem, rcText, pDC) == FALSE)
		{
			//use unprintable characters so it doesn't find copied html to convert
			if (m_searchText.GetLength() > 0 &&
				FindNoCaseAndInsert(csText, m_searchText, _T("\x01\x04 color='#ff0000'\x02"), _T("\x01\x03\x04\x02"), m_linesPerRow) > 0)
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
						
		if( m_bShowTextForFirstTenHotKeys && firstTenNum > 0 )
		{
			CString cs;
			if( firstTenNum == 10 )
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
			
			crHotKey.right = crHotKey.left + theApp.m_metrics.ScaleX(11);
			crHotKey.left += theApp.m_metrics.ScaleX(1 + extraFromClipWasPaste);
			crHotKey.top += theApp.m_metrics.ScaleX(1 + extraFromClipWasPaste);
			
			HFONT hOldFont = (HFONT)pDC->SelectObject(m_SmallFont);
			COLORREF localOldTextColor = pDC->SetTextColor(g_Opt.m_Theme.ListSmallQuickPasteIndexColor());

			CPen pen(PS_SOLID, 0, g_Opt.m_Theme.ListSmallQuickPasteIndexColor());
			CPen* pOldPen = pDC->SelectObject(&pen);
						
			pDC->DrawText(cs, crHotKey, DT_BOTTOM);
			
			pDC->MoveTo(CPoint(rcItem.left + theApp.m_metrics.ScaleX(8 + extraFromClipWasPaste), rcItem.top));
			pDC->LineTo(CPoint(rcItem.left + theApp.m_metrics.ScaleX(8 + extraFromClipWasPaste), rcItem.bottom));
			
			pDC->SelectObject(hOldFont);
			pDC->SetTextColor(localOldTextColor);
			pDC->SelectObject(pOldPen);
		}
		
		// restore the previous values		
		if(OldColor > -1)
			pDC->SetTextColor(OldColor);
		
		if(nOldBKMode > -1)
			pDC->SetBkMode(nOldBKMode);
		
        *pResult = CDRF_SKIPDEFAULT;    // We've painted everything.
	}
}

BOOL CQListCtrl::DrawRtfText(int nItem, CRect &crRect, CDC *pDC)
{
	if(g_Opt.m_bDrawRTF == FALSE)
		return FALSE;
	
	BOOL bRet = FALSE;

	CClipFormat* pThumbnail = GetItem_CF_RTF_ClipFormat(nItem);
	if(pThumbnail == NULL)
		return FALSE;

	// if there's no data, then we're done.
	if(pThumbnail->m_hgData == NULL)
		return FALSE;

	if(m_pFormatter == NULL)
	{
		m_pFormatter = new CFormattedTextDraw;
		m_pFormatter->Create();
	}

	if(m_pFormatter)
	{
	   char *pData = (char*)GlobalLock(pThumbnail->m_hgData);
	   if(pData)
	   {
		   CComBSTR bStr(pData);
		   
		   m_pFormatter->put_RTFText(bStr);
		   
		   m_pFormatter->Draw(pDC->m_hDC, crRect);

		   GlobalUnlock(pThumbnail->m_hgData);

		   bRet = TRUE;
	   }
	}

	return bRet;
}

// DrawBitMap loads a DIB from the DB, draws a crRect thumbnail of the image
//  to pDC and caches that thumbnail as a DIB in m_ThumbNails[ ItemID ].
// ALL items are cached in m_ThumbNails (those without images are cached with NULL m_hgData)
BOOL CQListCtrl::DrawBitMap(int nItem, CRect &crRect, CDC *pDC, const CString &csDescription)
{
	if(g_Opt.m_bDrawThumbnail == FALSE)
		return FALSE;

	CClipFormatQListCtrl *format = GetItem_CF_DIB_ClipFormat(nItem);
	if(format != NULL)
	{
		HGLOBAL smallImage = format->GetDib(pDC, crRect.Height());
		if(smallImage != NULL)
		{
			//Will return the width of the bitmap in nWidth
			int nWidth = 0;
			if(CBitmapHelper::DrawDIB(pDC, smallImage, crRect.left, crRect.top, nWidth))
			{
				// adjust the rect so other information can be drawn next to the thumbnail
				crRect.left += nWidth + 3;
			}
		}
	}
	else if(csDescription.Find(_T("CF_DIB")) == 0)
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
	CBrush myBrush(g_Opt.m_Theme.MainWindowBG());    // dialog background color
	CBrush *pOld = pDC->SelectObject(&myBrush);
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

BOOL CQListCtrl::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;
	
	UINT_PTR nID = pNMHDR->idFrom;
	
	if(nID == 0)	  	// Notification in NT from automatically
		return FALSE;   	// created tooltip
	
	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 500);
	
	// Use Item's name as the tool tip. Change this for something different.
	// Like use its file size, etc.
	GetToolTipText((int)nID-1, strTipText);
	
	//Replace the tabs with spaces, the tooltip didn't like the \t s
	strTipText.Replace(_T("\t"), _T("  "));
	
	int nLength = strTipText.GetLength()+2;

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
	{
		if(m_pchTip != NULL)
			delete m_pchTip;
		
		m_pchTip = new TCHAR[nLength];
		lstrcpyn(m_pchTip, strTipText, nLength-1);
		m_pchTip[nLength-1] = 0;
		pTTTW->lpszText = (WCHAR*)m_pchTip;
	}
	else
	{
		if(m_pwchTip != NULL)
			delete m_pwchTip;
		
		m_pwchTip = new WCHAR[nLength];
		_mbstowcsz(m_pwchTip, strTipText, nLength-1);
		m_pwchTip[nLength-1] = 0; // end of text
		pTTTW->lpszText = (WCHAR*)m_pwchTip;
	}
#else
	if(pNMHDR->code == TTN_NEEDTEXTA)
	{
		if(m_pchTip != NULL)
			delete m_pchTip;
		
		m_pchTip = new TCHAR[nLength];
		STRNCPY(m_pchTip, strTipText, nLength-1);
		m_pchTip[nLength-1] = 0; // end of text
		pTTTW->lpszText = (LPTSTR)m_pchTip;
	}
	else
	{
		if(m_pwchTip != NULL)
			delete m_pwchTip;
		
		m_pwchTip = new WCHAR[nLength];
		lstrcpyn(m_pwchTip, strTipText, nLength-1);
		m_pwchTip[nLength-1] = 0;
		pTTTW->lpszText = (LPTSTR) m_pwchTip;
	}
#endif
	*pResult = 0;
	
	return TRUE;    // message was handled
}

INT_PTR CQListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
	CRect rect;
	GetClientRect(&rect);
	if(rect.PtInRect(point))
	{
		if(GetItemCount())
		{
			int nTopIndex = GetTopIndex();
			int nBottomIndex = nTopIndex + GetCountPerPage();
			if(nBottomIndex > GetItemCount()) nBottomIndex = GetItemCount();
			for(int nIndex = nTopIndex; nIndex <= nBottomIndex; nIndex++)
			{
				GetItemRect(nIndex, rect, LVIR_BOUNDS);
				if(rect.PtInRect(point))
				{
					pTI->hwnd = m_hWnd;
					pTI->uId = (UINT)(nIndex+1);
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
	
	EnableToolTips();

	m_pToolTip = new CToolTipEx;
	m_pToolTip->Create(this);

	m_pToolTip->SetNotifyWnd(GetParent());
	
	return 0;
}

BOOL CQListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	CAccel a;
	if(m_Accels.OnMsg(pMsg, a))
	{
		switch(a.Cmd)
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
			if(a.RefId == CHotKey::PASTE_OPEN_CLIP)
			{
				GetParent()->SendMessage(NM_SELECT_DB_ID, a.Cmd, 0);
			}
			else if(a.RefId == CHotKey::MOVE_TO_GROUP)
			{
				GetParent()->SendMessage(NM_MOVE_TO_GROUP, a.Cmd, 0);
			}
		}

		return TRUE;
	}

	if(VALID_TOOLTIP)
	{
		if(m_pToolTip->OnMsg(pMsg))
			return TRUE;
	}
		
	switch(pMsg->message) 
	{
	case WM_KEYDOWN:
		if(HandleKeyDown(pMsg->wParam, pMsg->lParam))
			return TRUE;
		
		break; // end case WM_KEYDOWN

	case WM_VSCROLL:
		ASSERT(FALSE);
		break;
	} // end switch(pMsg->message)
		
	return CListCtrl::PreTranslateMessage(pMsg);
}

BOOL CQListCtrl::HandleKeyDown(WPARAM wParam, LPARAM lParam)
{
	if(VALID_TOOLTIP)
	{
		MSG Msg;
		Msg.lParam = lParam;
		Msg.wParam = wParam;
		Msg.message = WM_KEYDOWN;
		if(m_pToolTip->OnMsg(&Msg))
			return TRUE;
	}

	WPARAM vk = wParam;
	
	switch( vk )
	{
	case 'X': // Ctrl-X = Cut (prepare for moving the items into a Group)
		if(CONTROL_PRESSED)
		{
			LoadCopyOrCutToClipboard();		
			
			theApp.IC_Cut(); // uses selection
			return TRUE;
		}
		break;
		
	case 'C': // Ctrl-C = Copy (prepare for copying the items into a Group)
		if(CONTROL_PRESSED)
		{
			LoadCopyOrCutToClipboard();
			
			theApp.IC_Copy(); // uses selection
			return TRUE;
		}
		break;
		
	case 'V': // Ctrl-V = Paste (actually performs the copy or move of items into the current Group)
		if(CONTROL_PRESSED)
		{
			theApp.IC_Paste();
			return TRUE;
		}
		break;
		
	case 'A': // Ctrl-A = Select All
		if(CONTROL_PRESSED)
		{
			int nCount = GetItemCount();
			for(int i = 0; i < nCount; i++)
			{
				SetSelection(i);
			}
			return TRUE;
		}
		break;
		
	case VK_HOME:
		SetListPos(0);
		break;
	} // end switch(vk)
	
	return FALSE;
}

void CQListCtrl::LoadCopyOrCutToClipboard()
{
	ARRAY arr;
	GetSelectionItemData(arr);
	INT_PTR count = arr.GetSize();
	if(count <= 0)
		return;
	
	CProcessPaste paste;
	
	//Don't send the paste just load it into memory
	paste.m_bSendPaste = false;
		
	if(count > 1)
		paste.GetClipIDs().Copy(arr);
	else
		paste.GetClipIDs().Add(arr[0]);
	
	//Don't move these to the top
	BOOL bItWas = g_Opt.m_bUpdateTimeOnPaste;
	g_Opt.m_bUpdateTimeOnPaste = FALSE;

	paste.DoPaste();

	g_Opt.m_bUpdateTimeOnPaste = bItWas;
}

bool CQListCtrl::PostEventLoadedCheckDescription()
{
	bool loadedClip = false;

	int clipRow = this->GetCaret();

	if (VALID_TOOLTIP)
	{
		int toolTipClipId = m_pToolTip->GetClipId();
		int toolTipClipRow = m_pToolTip->GetClipRow();

		//We tried to show the clip but we didn't have the id yet, it was loaded in a thread, now it's being updated
		//see if we need to show this rows description
		if (toolTipClipId <= 0 &&
			toolTipClipRow == clipRow &&
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

	if(VALID_TOOLTIP && 
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
	
	if(CGetSetOptions::GetRememberDescPos())
	{
		CGetSetOptions::GetDescWndPoint(pt);
	}
	else if(bFromAuto == false)
	{
		pt = CPoint(rc.left, rc.bottom);
	}
	else
	{
		pt = CPoint((crWindow.left + (crWindow.right - crWindow.left)/2), rc.bottom);
	}

	CString csDescription;
	GetToolTipText(nItem, csDescription);
		
	if (m_pToolTip == NULL ||
		fromNextPrev == false ||
		::IsWindow(m_toolTipHwnd) == FALSE)
	{
		m_pToolTip->DestroyWindow();

		m_pToolTip = new CToolTipEx;
		m_pToolTip->Create(this);
		m_toolTipHwnd = m_pToolTip->GetSafeHwnd();
		m_pToolTip->SetNotifyWnd(GetParent());
	}
	else if(VALID_TOOLTIP)
	{
		CRect r;
		m_pToolTip->GetWindowRect(r);
		pt = r.TopLeft();

		m_pToolTip->SetBitmap(NULL);
		m_pToolTip->SetRTFText("");
		m_pToolTip->SetToolTipText(_T(""));
		
	}
	
	if(VALID_TOOLTIP)
	{
		m_pToolTip->SetClipId(clipId);
		m_pToolTip->SetClipRow(clipRow);
		m_pToolTip->SetSearchText(m_searchText);

		m_pToolTip->SetClipData(_T(""));
		m_pToolTip->SetToolTipText(_T(""));  
		m_pToolTip->SetRTFText("    ");
		bool bSetPlainText = false;
		CClipFormat Clip;


		try
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, lDate, lastPasteDate, lDontAutoDelete, QuickPasteText, lShortCut, globalShortCut FROM Main WHERE lID = %d"), clipId);
			if (q.eof() == false)
			{
				CString clipData;
				COleDateTime time((time_t)q.getIntField(_T("lDate")));
				clipData += "Added: " + time.Format();

				COleDateTime modified((time_t)q.getIntField(_T("lastPasteDate")));
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

				m_pToolTip->SetClipData(clipData);
			}
		}
		CATCH_SQLITE_EXCEPTION




		
		Clip.m_cfType = CF_UNICODETEXT;
		if(GetClipData(nItem, Clip) && Clip.m_hgData)
		{
			LPVOID pvData = GlobalLock(Clip.m_hgData);
			if(pvData)
			{
				CString csText = (WCHAR*)pvData;
				m_pToolTip->SetToolTipText(csText);
				bSetPlainText = true;
			}

			GlobalUnlock(Clip.m_hgData);

			Clip.Free();
			Clip.Clear();
		}

		if(bSetPlainText == false)
		{
			Clip.m_cfType = CF_TEXT;
			if(GetClipData(nItem, Clip) && Clip.m_hgData)
			{
				LPVOID pvData = GlobalLock(Clip.m_hgData);
				if(pvData)
				{
					CString csText = (char*)pvData;
					m_pToolTip->SetToolTipText(csText);

					bSetPlainText = true;
				}

				GlobalUnlock(Clip.m_hgData);

				Clip.Free();
				Clip.Clear();
			}
		}

		if(bSetPlainText == false)
		{
			m_pToolTip->SetToolTipText(csDescription);
		}

		Clip.m_cfType = RegisterClipboardFormat(CF_RTF);

		if(GetClipData(nItem, Clip) && Clip.m_hgData)
		{
			LPVOID pvData = GlobalLock(Clip.m_hgData);
			if(pvData)
			{
				m_pToolTip->SetRTFText((char*)pvData);
			}

			GlobalUnlock(Clip.m_hgData);

			Clip.Free();
			Clip.Clear();
		}	
			
		Clip.m_cfType = CF_DIB;
				
		if(GetClipData(nItem, Clip) && Clip.m_hgData)
		{			
			CBitmap *pBitMap = new CBitmap;
			if(pBitMap)
			{
				CRect rcItem;
				GetWindowRect(rcItem);
				
				CDC *pDC = GetDC();;

				CBitmapHelper::GetCBitmap(&Clip, pDC, pBitMap, MAXINT);

				ReleaseDC(pDC);

				//Tooltip wnd will release
				m_pToolTip->SetBitmap(pBitMap);
			}

			Clip.Free();
			Clip.Clear();
		}			
		
		m_pToolTip->Show(pt);
	}

	return true;
}

void CQListCtrl::GetToolTipText(int nItem, CString &csText)
{
	CWnd* pParent=GetParent();
	if(pParent && (pParent->GetSafeHwnd() != NULL))
	{
		CQListToolTipText info;
		memset(&info, 0, sizeof(info));
		info.hdr.code = NM_GETTOOLTIPTEXT;
		info.hdr.hwndFrom = GetSafeHwnd();
		info.hdr.idFrom = GetDlgCtrlID();
		info.lItem = nItem;
		//plus 100 for extra info - shortcut and such
		info.cchTextMax = g_Opt.m_bDescTextSize + 100;
		info.pszText = csText.GetBufferSetLength(info.cchTextMax);
		
		pParent->SendMessage(WM_NOTIFY,(WPARAM)info.hdr.idFrom,(LPARAM)&info);
		
		csText.ReleaseBuffer();			
	}
}

BOOL CQListCtrl::GetClipData(int nItem, CClipFormat &Clip)
{
	return theApp.GetClipData(GetItemData(nItem), Clip);
}

DWORD CQListCtrl::GetItemData(int nItem)
{
	if((GetStyle() & LVS_OWNERDATA))
	{
		CWnd* pParent=GetParent();
		if(pParent && (pParent->GetSafeHwnd() != NULL))
		{
			LV_DISPINFO info;
			memset(&info, 0, sizeof(info));
			info.hdr.code = LVN_GETDISPINFO;
			info.hdr.hwndFrom = GetSafeHwnd();
			info.hdr.idFrom = GetDlgCtrlID();
			
			info.item.iItem = nItem;
			info.item.lParam = -1;
			info.item.mask = LVIF_PARAM;
			
			pParent->SendMessage(WM_NOTIFY,(WPARAM)info.hdr.idFrom,(LPARAM)&info);
			
			return (DWORD)info.item.lParam;
		}
	}
	
	return (DWORD)CListCtrl::GetItemData(nItem);
}

CClipFormatQListCtrl* CQListCtrl::GetItem_CF_DIB_ClipFormat(int nItem)
{
	CClipFormatQListCtrl *format = NULL;

	CWnd* pParent=GetParent();
	if(pParent && (pParent->GetSafeHwnd() != NULL))
	{
		LV_DISPINFO info;
		memset(&info, 0, sizeof(info));
		info.hdr.code = LVN_GETDISPINFO;
		info.hdr.hwndFrom = GetSafeHwnd();
		info.hdr.idFrom = GetDlgCtrlID();

		info.item.iItem = nItem;
		info.item.lParam = NULL;
		info.item.mask = LVIF_CF_DIB;

		pParent->SendMessage(WM_NOTIFY,(WPARAM)info.hdr.idFrom,(LPARAM)&info);

		if(info.item.lParam != NULL)
		{
			format = (CClipFormatQListCtrl *)info.item.lParam;
		}
	}

	return format;
}

CClipFormatQListCtrl* CQListCtrl::GetItem_CF_RTF_ClipFormat(int nItem)
{
	CClipFormatQListCtrl *format = NULL;

	CWnd* pParent=GetParent();
	if(pParent && (pParent->GetSafeHwnd() != NULL))
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

		if(info.item.lParam != NULL)
		{
			format = (CClipFormatQListCtrl *)info.item.lParam;
		}
	}

	return format;
}

void CQListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{	
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CQListCtrl::DestroyAndCreateAccelerator(BOOL bCreate, CppSQLite3DB &db)
{
	m_Accels.RemoveAll();

	if(bCreate)
	{
		CMainTableFunctions::LoadAcceleratorKeys(m_Accels, db);

		LoadDittoCopyBufferHotkeys();
	}
}

void CQListCtrl::LoadDittoCopyBufferHotkeys()
{
	CCopyBufferItem Item;
	CAccel a;

	g_Opt.GetCopyBufferItem(0, Item);	
	if(Item.m_lCopyHotKey > 0)
	{
		a.Cmd = COPY_BUFFER_HOT_KEY_1_ID; 
		a.Key = Item.m_lCopyHotKey;
		m_Accels.AddAccel(a);
	}

	g_Opt.GetCopyBufferItem(1, Item);
	if(Item.m_lCopyHotKey > 0)
	{
		a.Cmd = COPY_BUFFER_HOT_KEY_2_ID;
		a.Key = Item.m_lCopyHotKey;
		m_Accels.AddAccel(a);
	}

	g_Opt.GetCopyBufferItem(2, Item);
	if(Item.m_lCopyHotKey > 0)
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
	if(VALID_TOOLTIP)
		return m_pToolTip->GetSafeHwnd();

	return NULL;
}

BOOL CQListCtrl::SetItemCountEx(int iCount, DWORD dwFlags /* = 0 */)
{
	return CListCtrl::SetItemCountEx(iCount, dwFlags);
}

void CQListCtrl::OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pnmv = (NMLISTVIEW *) pNMHDR;

	if((pnmv->uNewState == 3) ||
		(pnmv->uNewState == 1))
	{
		if (VALID_TOOLTIP &&
			::IsWindowVisible(m_pToolTip->m_hWnd))
		{
			this->ShowFullDescription(false, true);
		}
		if(g_Opt.m_bAllwaysShowDescription)
		{
			KillTimer(TIMER_SHOW_PROPERTIES);
			SetTimer(TIMER_SHOW_PROPERTIES, 300, NULL);
		}
		if(GetSelectedCount() > 0 )
			theApp.SetStatus(NULL, FALSE);
	}

	if(GetSelectedCount() == this->GetItemCount())
	{
		if(m_allSelected == false)
		{
			Log(StrF(_T("List box Select All")));

			GetParent()->SendMessage(NM_ALL_SELECTED, 0, 0);
			m_allSelected = true;
		}
	}
	else if(m_allSelected == true)
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

	switch(nIDEvent)
	{
		case TIMER_SHOW_PROPERTIES:
			{
				if( theApp.m_bShowingQuickPaste )
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
				if(MouseInScrollBarArea(crWindow, cursorPos) == false)
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
				crWindow.right -= theApp.m_metrics.ScaleX(GetSystemMetrics(SM_CXVSCROLL));
				crWindow.bottom -= theApp.m_metrics.ScaleX(::GetSystemMetrics(SM_CXHSCROLL));

				//Check and see if we are still in the cursor area
				if(MouseInScrollBarArea(crWindow, cursorPos))
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
	
	if(callBase)
	{
		CListCtrl::OnTimer(nIDEvent);
	}
}

void CQListCtrl::SetLogFont(LOGFONT &font)
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

BOOL CQListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	return CListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CQListCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	NMLVCACHEHINT* pcachehint = NULL;

	if(message == WM_NOTIFY)
    {
        NMHDR* phdr = (NMHDR*)lParam;
		
        switch(phdr->code)
        {
        case LVN_ODCACHEHINT:
            pcachehint= (NMLVCACHEHINT*) phdr;

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
	if(g_Opt.m_showScrollBar == FALSE)
	{
		CRect crWindow;
		this->GetWindowRect(&crWindow);
		ScreenToClient(&crWindow);

		crWindow.right -= theApp.m_metrics.ScaleX(::GetSystemMetrics(SM_CXVSCROLL));
		crWindow.bottom -= theApp.m_metrics.ScaleX(::GetSystemMetrics(SM_CXHSCROLL));

		if(MouseInScrollBarArea(crWindow, point))
		{
			if((GetTickCount() - m_mouseOverScrollAreaStart) > 500)
			{
				SetTimer(TIMER_SHOW_SCROLL, 500, NULL);

				m_mouseOverScrollAreaStart = GetTickCount();
			}
		}
		else
		{
			if(m_timerToHideScrollAreaSet)
			{
				StopHideScrollBarTimer();
			}		
			KillTimer(TIMER_SHOW_SCROLL);
		}
	}
}

bool CQListCtrl::MouseInScrollBarArea(CRect crWindow, CPoint point)
{
	CRect crRight(crWindow);
	CRect crBottom(crWindow);	

	crRight.left = crRight.right - theApp.m_metrics.ScaleX(::GetSystemMetrics(SM_CXVSCROLL));
	crBottom.top = crBottom.bottom - theApp.m_metrics.ScaleY(::GetSystemMetrics(SM_CYHSCROLL));

	/*CString cs;
	cs.Format(_T("point.x: %d, Width: %d, Height: %d\n"), point.x, crWindow.Width(), crWindow.Height());
	OutputDebugString(cs);*/

	if(crRight.PtInRect(point) || crBottom.PtInRect(point))
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

void CQListCtrl::HidePopup()
{ 
	if(VALID_TOOLTIP) 
		m_pToolTip->Hide();	
}

BOOL CQListCtrl::IsToolTipWindowVisible() 
{ 
	return ::IsWindowVisible(m_toolTipHwnd); 
}

BOOL CQListCtrl::IsToolTipWindowFocus()
{
	return ::GetFocus() == m_toolTipHwnd ||
		::GetParent(::GetFocus()) == m_toolTipHwnd;
}