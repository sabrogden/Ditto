// EditWithButton.cpp : implementation file
//

//DISCLAIMER:
//The code in this project is Copyright (C) 2006 by Gautam Jain. You have the right to
//use and distribute the code in any way you see fit as long as this paragraph is included
//with the distribution. No warranties or claims are made as to the validity of the
//information and code contained herein, so use it at your own risk.

#include "stdafx.h"
#include "EditWithButton.h"
#include "cp_main.h"


IMPLEMENT_DYNAMIC(CEditWithButton, CEdit)

CEditWithButton::CEditWithButton()
{
	m_iButtonClickedMessageId = NM_CANCEL_SEARCH;
	m_bButtonExistsAlways = FALSE;

	m_rcEditArea.SetRect(0, 0, 0, 0);
}

CEditWithButton::~CEditWithButton()
{
	m_bmpEmptyEdit.DeleteObject();
	m_bmpFilledEdit.DeleteObject();
}

BEGIN_MESSAGE_MAP(CEditWithButton, CEdit)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CEditWithButton message handlers

void CEditWithButton::PreSubclassWindow()
{
	// We must have a multiline edit
	// to be able to set the edit rect
	ASSERT(GetStyle() & ES_MULTILINE);

	ResizeWindow();
}

BOOL CEditWithButton::PreTranslateMessage(MSG* pMsg)
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

	switch(pMsg->message) 
	{
	case WM_KEYDOWN:
		{
			if(pMsg->wParam == VK_RETURN)
			{
				CWnd *pWnd = GetParent();
				if(pWnd)
				{
					if(g_Opt.m_bFindAsYouType)
					{
						pWnd->SendMessage(NM_DBL_CLICK, 0, 0);
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
					pMsg->wParam == VK_F3)
			{
				CWnd *pWnd = GetParent();
				if(pWnd)
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

BOOL CEditWithButton::SetBitmaps(UINT iEmptyEdit, UINT iFilledEdit)
{
	BITMAP bmpInfo;

	//delete if already loaded.. just in case
	m_bmpEmptyEdit.DeleteObject();
	m_bmpFilledEdit.DeleteObject();

	m_bmpEmptyEdit.LoadBitmap(iEmptyEdit);
	m_bmpFilledEdit.LoadBitmap(iFilledEdit);
	
	m_bmpEmptyEdit.GetBitmap(&bmpInfo);
	m_sizeEmptyBitmap.SetSize(bmpInfo.bmWidth,bmpInfo.bmHeight);
	
	m_bmpFilledEdit.GetBitmap(&bmpInfo);
	m_sizeFilledBitmap.SetSize(bmpInfo.bmWidth,bmpInfo.bmHeight);

	return TRUE;
}

//client area
void CEditWithButton::SetButtonArea(CRect rcButtonArea)
{
	m_rcButtonArea = rcButtonArea;
}

void CEditWithButton::ResizeWindow()
{
	if (!::IsWindow(m_hWnd)) return;

	//proceed only if edit area is set
	if (m_rcBorder == CRect(0,0,0,0)) 
		return;

	CRect r;
	GetWindowRect(r);
	ScreenToClient(r);

	SetWindowPos(&wndTop, 0, 0, r.Width(), r.Height(), SWP_NOMOVE|SWP_NOZORDER);

	m_rcEditArea.left = r.left + m_rcBorder.left;
	m_rcEditArea.top = r.top + m_rcBorder.top;
	m_rcEditArea.right = r.right -= m_rcBorder.right;
	m_rcEditArea.bottom = r.bottom -= m_rcBorder.bottom;
	
	SetRect(&m_rcEditArea);
}

//set edit area may be called before creating the edit control
//especially when using the CEdit::Create method
//or after creating the edit control in CEdit::DoDataExchange
//we call ResizeWindow once in SetEditArea and once in PreSubclassWindow
BOOL CEditWithButton::SetBorder(CRect rcEditArea)
{
	m_rcBorder = rcEditArea;

	ResizeWindow();

	return TRUE;
}

void CEditWithButton::SetButtonExistsAlways(BOOL bButtonExistsAlways)
{
	m_bButtonExistsAlways = bButtonExistsAlways;
}

BOOL CEditWithButton::OnEraseBkgnd(CDC* pDC)
{
	// Get the size of the bitmap
	CDC dcMemory;
    CSize sizeBitmap;
	CBitmap* pOldBitmap = NULL;
	int iTextLength = GetWindowTextLength();

	CRect size;
	GetWindowRect(size);
    
	if (iTextLength == 0)
	{
		sizeBitmap = m_sizeEmptyBitmap;
	}
	else
	{
		sizeBitmap = m_sizeFilledBitmap;
	}

    // Create an in-memory DC compatible with the
    // display DC we're using to paint
    dcMemory.CreateCompatibleDC(pDC);

	if (iTextLength == 0)
	{
		// Select the bitmap into the in-memory DC
		pOldBitmap = dcMemory.SelectObject(&m_bmpEmptyEdit);
	}
	else
	{
		// Select the bitmap into the in-memory DC
		pOldBitmap = dcMemory.SelectObject(&m_bmpFilledEdit);
	}

    // Copy the bits from the in-memory DC into the on-
    // screen DC to actually do the painting. Use the centerpoint
    // we computed for the target offset.
    pDC->BitBlt(0,0, 50, sizeBitmap.cy, &dcMemory, 
        0, 0, SRCCOPY);

	for(int i = 50; i < size.Width()-50; i++)
	{
		pDC->BitBlt(i, 0, 1, sizeBitmap.cy, &dcMemory, 
			50, 0, SRCCOPY);
	}

	pDC->BitBlt(size.Width()-50, 0, 50, sizeBitmap.cy, &dcMemory, 
		sizeBitmap.cx-50, 0, SRCCOPY);

    dcMemory.SelectObject(pOldBitmap);

	return TRUE;
}

void CEditWithButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//this will draw the background again
	//so that the button will be drawn if the text exists

	InvalidateRect(NULL);

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CEditWithButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	//if the button is clicked then send message to the
	//owner.. the owner need not be parent
	//you can set the owner using the CWnd::SetOwner method
	CRect crClose;
	GetWindowRect(crClose);
	ScreenToClient(crClose);


	int left = crClose.right - (m_sizeFilledBitmap.cx - m_rcButtonArea.left);
	int right = crClose.right - (m_sizeFilledBitmap.cx - m_rcButtonArea.right);

	crClose.top = m_rcButtonArea.top;
	crClose.bottom = m_rcButtonArea.bottom;
	crClose.left = left;
	crClose.right = right;

	if (crClose.PtInRect(point))
	{
		//it is assumed that when the text is not typed in the
		//edit control, the button will not be visible
		//but you can override this by setting 
		//the m_bButtonExistsAlways to TRUE
		if ( (GetWindowTextLength() > 0) || m_bButtonExistsAlways)
		{
			CWnd *pOwner = GetOwner();
			if (pOwner)
			{
				pOwner->SendMessage(m_iButtonClickedMessageId, 0, 0);
			}
		}
	}

	CEdit::OnLButtonUp(nFlags, point);
}


//by default, when the mouse moves over the edit control
//the system shows the I-beam cursor. However we want to
//show the arrow cursor when it is over the Non-Edit area
//where the button and icon is displayed
//here is the code to do this
BOOL CEditWithButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint pntCursor;
	GetCursorPos(&pntCursor);
	ScreenToClient(&pntCursor);

	CRect edit;
	GetWindowRect(edit);
	ScreenToClient(edit);
	edit.left += m_rcBorder.left;
	edit.top += m_rcBorder.top;
	edit.right -= m_rcBorder.right;
	edit.bottom -= m_rcBorder.bottom;

	//if mouse is not in the edit area then
	//show arrow cursor
	if (!edit.PtInRect(pntCursor))
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(MAKEINTRESOURCE(IDC_ARROW)));
		return TRUE;
	}

	return CEdit::OnSetCursor(pWnd, nHitTest, message);
}

int CEditWithButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	ResizeWindow();

	return 0;
}


LRESULT CEditWithButton::OnSetFont( WPARAM wParam, LPARAM lParam )
{
	DefWindowProc(WM_SETFONT, wParam, lParam);

	ResizeWindow();

	return 0;
}

void CEditWithButton::OnSize(UINT nType, int cx, int cy) 
{
	CEdit::OnSize(nType, cx, cy);

	ResizeWindow();
}

