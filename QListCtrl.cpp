// QListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "QListCtrl.h"
#include "ProcessPaste.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ROW_BOTTOM_BORDER		2
#define ROW_LEFT_BORDER			3
#define COLOR_SHADOW			RGB(245, 245, 245)
#define DUMMY_COL_WIDTH			1

/////////////////////////////////////////////////////////////////////////////
// CQListCtrl

CQListCtrl::CQListCtrl()
{
	m_pchTip = NULL;
	m_pwchTip = NULL;

	LOGFONT lf;
	
	lf.lfHeight = -9;
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
	lstrcpy(lf.lfFaceName, "Small Font");

	m_SmallFont = CreateFontIndirect(&lf);

	m_bShowTextForFirstTenHotKeys = true;
	m_bStartTop = true;
//	m_Accelerator = NULL; //!!!!!
}

CQListCtrl::~CQListCtrl()
{
	if(m_pchTip != NULL)
		delete m_pchTip;

	if(m_pwchTip != NULL)
		delete m_pwchTip;

	DestroyAndCreateAccelerator(FALSE);
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
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomdrawList)
	ON_WM_SYSKEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQListCtrl message handlers

void CQListCtrl::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	
	switch (pLVKeyDow->wVKey)
	{
	case VK_RETURN:
		{
			ARRAY arr;
			GetSelectionIndexes(arr);
			SendSelection(arr);
		}
		break;

	case VK_ESCAPE:
		GetParent()->SendMessage(NM_END, 0, 0);
		break;

	case VK_RIGHT:
		{
			int nItem = GetNextItem(-1, LVNI_SELECTED);
			if (nItem != -1)
				GetParent()->SendMessage(NM_RIGHT, nItem, 0);
		}
		break;

	case VK_LEFT:
		GetParent()->SendMessage(NM_LEFT, 0, 0);
		break;
	case VK_DELETE:
		GetParent()->SendMessage(NM_DELETE, 0, 0);
		break;
	}

	*pResult = 0;
}

void CQListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMITEMACTIVATE lpnmItem = (LPNMITEMACTIVATE) pNMHDR;
	
	UINT Flags;
	int nItem = -1;
	if ((nItem = HitTest(lpnmItem->ptAction, &Flags)) != -1)
	{
		if (Flags | LVHT_ONITEM)
			SendSelection(nItem);
	}

	*pResult = 0;
}

void CQListCtrl::SendSelection(int nItem)
{
	GetParent()->SendMessage(NM_SELECT, 1, (LPARAM) &nItem);
}

void CQListCtrl::SendSelection(ARRAY &arrItems)
{
	GetParent()->SendMessage(NM_SELECT, arrItems.GetSize(), (LPARAM) arrItems.GetData());
}

void CQListCtrl::GetSelectionIndexes(ARRAY &arr)
{
	arr.RemoveAll();

	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
		arr.Add(GetNextSelectedItem(pos));
	

	/*
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	while (nItem != -1)
	{
		arr.Add(nItem);
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	*/
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
	
	/*
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	while (nItem != -1)
	{
		arr.Add((int)GetItemData(nItem));
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
	*/
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
	CDC *pDC = GetDC();

	CRect crRect(0, 0, 0, 0);

	CFont *pOldFont = pDC->SelectObject(GetFont());

	//Get the height to draw one character
	pDC->DrawText("W", crRect, DT_VCENTER | DT_EXPANDTABS | DT_CALCRECT);

	pDC->SelectObject(pOldFont);

	//Get the total height of each row
	int nHeight = (crRect.Height() * nLines) + ROW_BOTTOM_BORDER;

	//Create a image list of that height and set it to the list box
	CImageList imglist;
	imglist.Create(DUMMY_COL_WIDTH, nHeight, ILC_COLOR16 | ILC_MASK, 1, 1);
	SetImageList(&imglist, LVSIL_SMALL );
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
        GetItemRect(nItem, rcItem, LVIR_LABEL);
		rcItem.left -= DUMMY_COL_WIDTH;

		CPen cpWhite;
		cpWhite.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
		CPen *pOldPen = NULL;
		COLORREF OldColor = -1;
		int nOldBKMode = -1;
		
        // Draw the background of the list item.  Colors are selected 
        // according to the item's state.
        if(rItem.state & LVIS_SELECTED)
		{
            if(bListHasFocus)
			{
                crBkgnd = GetSysColor(COLOR_HIGHLIGHT);
                OldColor = pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
				pOldPen = pDC->SelectObject((CPen*)&cpWhite);
			}
            else
			{
                crBkgnd = GetSysColor(COLOR_BTNFACE);
                OldColor = pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
			}
		}
        else
		{
            //Shade alternating Rows
			if((nItem % 2) == 0)
				crBkgnd = COLOR_SHADOW;
			else
				crBkgnd = GetSysColor(COLOR_WINDOW);
			
            OldColor = pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
		}
		
        pDC->FillSolidRect(rcItem, crBkgnd);
        nOldBKMode = pDC->SetBkMode(TRANSPARENT);
		
        CRect rcText = rcItem;
        rcText.left += ROW_LEFT_BORDER;
		rcText.top++;
		
        // Draw the text.
        CString csText = GetItemText(nItem, 0);

		// set firstTenNum to the first ten number (1-10) corresponding to
		//  the current nItem.
		// -1 means that nItem is not in the FirstTen block.
		int firstTenNum = GetFirstTenNum(nItem);

		if( m_bShowTextForFirstTenHotKeys && firstTenNum > 0 )
		{
			rcText.left += 12;
		}

		pDC->DrawText(csText, rcText, DT_VCENTER | DT_EXPANDTABS);
		
        // Draw a focus rect around the item if necessary.
        if(bListHasFocus && (rItem.state & LVIS_FOCUSED))
			pDC->DrawFocusRect(rcItem);

		if( m_bShowTextForFirstTenHotKeys && firstTenNum > 0 )
		{
			CString cs;
			if( firstTenNum == 10 )
				cs = "0";
			else
				cs.Format("%d", firstTenNum);

			CRect crClient;
			
			GetWindowRect(crClient);
			ScreenToClient(crClient);
			
			CRect crHotKey = rcItem;
			
			crHotKey.right = crHotKey.left + 11;
			crHotKey.left += 2;
			crHotKey.top += 2;

			HFONT hOldFont = (HFONT)pDC->SelectObject(m_SmallFont);

			pDC->DrawText(cs, crHotKey, DT_BOTTOM);

			pDC->MoveTo(CPoint(rcItem.left + 11, rcItem.top));
			pDC->LineTo(CPoint(rcItem.left + 11, rcItem.bottom));
			
			pDC->SelectObject(hOldFont);
		}

		if(pOldPen)
			pDC->SelectObject(pOldPen);

		if(OldColor > -1)
			pDC->SetTextColor(OldColor);

		if(nOldBKMode > -1)
			pDC->SetBkMode(nOldBKMode);

        *pResult = CDRF_SKIPDEFAULT;    // We've painted everything.
	}
}

void CQListCtrl::RefreshVisibleRows()
{
	int nTopIndex = GetTopIndex();
	int nLastIndex = nTopIndex + GetCountPerPage();
	RedrawItems(nTopIndex, nLastIndex);
	::UpdateWindow(m_hWnd);
}

void CQListCtrl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(GetKeyState(VK_RETURN) & 0x800)
		GetParent()->SendMessage(NM_PROPERTIES, 0, 0);
	else
		CListCtrl::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CQListCtrl::OnEraseBkgnd(CDC* pDC) 
{
//	return TRUE;
	return CListCtrl::OnEraseBkgnd(pDC);
}

BOOL CQListCtrl::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;

	UINT nID = pNMHDR->idFrom;

	if(nID == 0)	  	// Notification in NT from automatically
		return FALSE;   	// created tooltip
	
	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 500);

	// Use Item's name as the tool tip. Change this for something different.
	// Like use its file size, etc.
	strTipText = GetToolTipText(nID-1);

	//Replace the tabs with spaces, the tooltip didn't like the \t s
	strTipText.Replace("\t", "  ");

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
	{
		if(m_pchTip != NULL)
			delete m_pchTip;
		
		m_pchTip = new TCHAR[strTipText.GetLength()+1];
		lstrcpyn(m_pchTip, strTipText, strTipText.GetLength());
		m_pchTip[strTipText.GetLength()] = 0;
		pTTTW->lpszText = (WCHAR*)m_pchTip;
	}
	else
	{
		if(m_pwchTip != NULL)
			delete m_pwchTip;

		m_pwchTip = new WCHAR[strTipText.GetLength()+1];
		_mbstowcsz(m_pwchTip, strTipText, strTipText.GetLength());
		m_pwchTip[strTipText.GetLength()] = 0; // end of text
		pTTTW->lpszText = (WCHAR*)m_pwchTip;
	}
#else
	if(pNMHDR->code == TTN_NEEDTEXTA)
	{
		if(m_pchTip != NULL)
			delete m_pchTip;

		m_pchTip = new TCHAR[strTipText.GetLength()+1];
		_wcstombsz(m_pchTip, strTipText, strTipText.GetLength());
		m_pchTip[strTipText.GetLength()] = 0; // end of text
		pTTTA->lpszText = (LPTSTR)m_pchTip;
	}
	else
	{
		if(m_pwchTip != NULL)
			delete m_pwchTip;

		m_pwchTip = new WCHAR[strTipText.GetLength()+1];
		lstrcpyn(m_pwchTip, strTipText, strTipText.GetLength());
		m_pwchTip[strTipText.GetLength()] = 0;
		pTTTA->lpszText = (LPTSTR) m_pwchTip;
	}
#endif
	*pResult = 0;

	return TRUE;    // message was handled
}

int CQListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
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

	m_Popup.Init();
//	m_Popup.Init( GetToolTips()->m_hWnd );
//	m_Popup.m_TI.hwnd = m_hWnd;
    
	return 0;
}

BOOL CQListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	/* !!!!!
	//if(m_Accelerator)
	//{
	//	m_CheckingAccelerator = true;
	//	if(TranslateAccelerator(m_hWnd, m_Accelerator, pMsg) != 0)
	//	{
	//		m_CheckingAccelerator = false;
	//		return TRUE;
	//	}
	//	m_CheckingAccelerator = false;
	//}
	*/

	CAccel* pAccel = m_Accels.OnMsg( pMsg );
	if( pAccel && GetParent()->SendMessage(NM_SELECT_DB_ID, pAccel->Cmd, 0) )
		return TRUE;

	switch(pMsg->message) 
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if(m_Popup.m_bIsShowing)
		{
			m_Popup.Hide();
		}
		break;
	case WM_KEYDOWN:
		WPARAM vk = pMsg->wParam;
		
		if(m_Popup.m_bIsShowing)
		{
			m_Popup.Hide();

			if(vk == VK_ESCAPE)
				return TRUE;
		}

		// if a number key was pressed
		if( '0' <= vk && vk <= '9' )
		{
			// if <Ctrl> is required but is absent, then break
			if( g_Opt.m_bUseCtrlNumAccel && !(GetKeyState(VK_CONTROL) & 0x8000) )
				break;

			int index = vk - '0';
			// '0' is actually 10 in the ditto window
			if( index == 0 )
				index = 10;
			// translate num 1-10 into the actual index (based upon m_bStartTop)
			index = GetFirstTenIndex( index );
			GetParent()->SendMessage(NM_SELECT_INDEX, index, 0);
			return TRUE;
		}

		switch( vk )
		{
		case 'A': // Ctrl-A = Select All
			if(GetKeyState(VK_CONTROL) & 0x8000)
			{
				int nCount = GetItemCount();
				for(int i = 0; i < nCount; i++)
				{
					SetSelection(i);
				}
				return TRUE;
			}
			break;

		case VK_F3:
			{
				int nItem = GetCaret();
				CRect rc;
				GetItemRect(nItem, rc, LVIR_BOUNDS);
				ClientToScreen(rc);
				m_Popup.m_Pos = CPoint(rc.left, rc.bottom);
				m_Popup.Show( GetToolTipText(nItem) );
			
				break;
			}
		} // end switch(vk)

		break; // end case WM_KEYDOWN
	} // end switch(pMsg->message)

	return CListCtrl::PreTranslateMessage(pMsg);
}

CString CQListCtrl::GetToolTipText(int nItem)
{
	CString cs;

	if((GetStyle() & LVS_OWNERDATA))
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
					
			pParent->SendMessage(WM_NOTIFY,(WPARAM)info.hdr.idFrom,(LPARAM)&info);

			cs = info.cText;
		}
	}

	return cs;
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

			return info.item.lParam;
		}
	}

	return CListCtrl::GetItemData(nItem);
}

void CQListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{		
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CQListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{	
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CQListCtrl::DestroyAndCreateAccelerator(BOOL bCreate)
{
// !!!!!!
	//if(m_Accelerator)
	//{
	//	DestroyAcceleratorTable(m_Accelerator);
	//	m_Accelerator = NULL;
	//}
	//
	//if(bCreate)
	//	m_Accelerator = CMainTable::LoadAcceleratorKeys();

	m_Accels.Clear();

	if( bCreate )
		CMainTable::LoadAcceleratorKeys( m_Accels );
}

/* !!!!!
//BOOL CQListCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
//{
//	//return 1 if from accelerator
//	if((HIWORD(wParam) == 1) && (m_CheckingAccelerator))
//	{
//		USHORT usPasteID = LOWORD(wParam);
//
//		GetParent()->SendMessage(NM_SELECT_DB_ID, usPasteID, 0);
//
//		return TRUE;
//	}
//	
//	return CListCtrl::OnCommand(wParam, lParam);
//}
*/

void CQListCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CListCtrl::OnKillFocus(pNewWnd);
	m_Popup.Hide();
}
