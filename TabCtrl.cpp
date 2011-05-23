// SheetCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "TabCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEXT_PAD			7
#define SPIN_PAD			3
#define SHIFT_UNITS			10

#define COLOR_WHITE			RGB(255, 255, 255)
#define COLOR_GRAY			RGB(128, 128, 128)
#define COLOR_MEDGRAY		GetSysColor(COLOR_BTNFACE)
#define COLOR_DARKGRAY		RGB(64, 64, 64)

#define ID_SCROLL_TIMER		0x1010

#define VK_TILDAE			0xC0

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx

CTabCtrlEx::CTabCtrlEx()
{
	m_nStyle = 0;
	m_nActiveTab = -1;
	m_nTabHeight = 19;
	m_btnState[0] = BtnUp;
	m_btnState[1] = BtnUp;
	m_bBtnEnabled[0] = false;
	m_bBtnEnabled[1] = false;
	m_nLeftShifted = 0;
	m_nPrevWidth = 0;
	m_pFntText = NULL;
	m_pFntBoldText = NULL;
	m_bSetFocusToNewlySelectedTab = true;

	m_SelectedColor = (COLORREF)GetSysColor(COLOR_BTNFACE);
	m_NonSelectedColor = RGB(255, 251, 233);
}

CTabCtrlEx::~CTabCtrlEx()
{
	if (m_pFntText)
		delete m_pFntText;
	m_pFntText = NULL;

	if (m_pFntBoldText)
		delete m_pFntBoldText;

	m_pFntBoldText = NULL;
}


BEGIN_MESSAGE_MAP(CTabCtrlEx, CWnd)
	//{{AFX_MSG_MAP(CTabCtrlEx)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx message handlers

BOOL CTabCtrlEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	// Get the sheet styles alone
	m_nStyle = short(dwStyle & 0xFFFF);

	// Remove the sheet styles and create the window
	dwStyle &= 0xFFFF0000;
	
	// If the window border style is set, change it to sheet border style
	if (dwStyle & WS_BORDER)
	{
		m_nStyle |= SCS_BORDER;
		dwStyle &= ~WS_BORDER;
	}

	// Register the class
	WNDCLASS wndClass;
	memset(&wndClass, 0, sizeof(wndClass));
	
	BOOL bRet = false;
	HINSTANCE hInst = AfxGetInstanceHandle();
	if (::GetClassInfo(hInst, SHEET_CLASSNAME, &wndClass))
	{
		bRet = (wndClass.style == SHEET_CLASSTYLE);
	}

	if (bRet == FALSE)
	{
		memset(&wndClass, 0, sizeof(wndClass));
		wndClass.style = SHEET_CLASSTYLE;
		wndClass.lpfnWndProc = ::DefWindowProc;
		wndClass.cbClsExtra = wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInst;
		wndClass.hIcon = NULL;
		wndClass.hCursor = NULL;
		wndClass.hbrBackground = NULL;
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = SHEET_CLASSNAME;
		if ((bRet = AfxRegisterClass(&wndClass)) == FALSE)
			ASSERT(FALSE);
	}

	if (bRet)
		bRet = CWnd::Create(SHEET_CLASSNAME, _T(""), dwStyle, rect, pParentWnd, nID);

	return bRet;
}

int CTabCtrlEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Create the brushes, pens
	m_brSelectedTab.CreateSolidBrush(m_SelectedColor);
	m_brNonSelectedTab.CreateSolidBrush(m_NonSelectedColor);
	m_penGray.CreatePen(PS_SOLID, 1, RGB(172, 168, 153));
	m_penBlack.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

	SetTabHeight(m_nTabHeight);

	return 0;
}

void CTabCtrlEx::SetTabHeight(int nTabHeight)
{
	m_nTabHeight = nTabHeight;

	// Delete the old font and create a new one
	if (m_pFntText)
		delete m_pFntText;
	m_pFntText = new CFont;
	
	if (m_pFntText)
	{
		// Set the regular font
		m_pFntText->CreateFont(-(m_nTabHeight*7/10), 0, 0, 0, FW_LIGHT, FALSE, FALSE, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial Unicode MS"));
	}

	// Delete the old font and create a new one
	if (m_pFntBoldText)
		delete m_pFntBoldText;
	m_pFntBoldText = new CFont;

	if (m_pFntBoldText)
	{
		// Set the bold font
		m_pFntBoldText->CreateFont(-(m_nTabHeight*7/10), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial Unicode MS"));
	}
}

bool CTabCtrlEx::AddItem(const CString& csTabTitle, CWnd* pTabWnd)
{
	return InsertItem((int)m_Tabs.GetSize(), csTabTitle, pTabWnd);
}

bool CTabCtrlEx::InsertItem(int nTab, const CString& csTabTitle, CWnd* pTabWnd)
{
	CTab tab;
	tab.csTitle = csTabTitle;
	tab.pWnd = pTabWnd;
	tab.lWidth = GetTextWidth(csTabTitle);
	tab.clrUnderline = -1;

	m_Tabs.InsertAt(nTab, tab);
	
	if(nTab <= m_nActiveTab)
		m_nActiveTab++;

	// Redraw the window
	RedrawWindow();
	
	return true;
}

bool CTabCtrlEx::ReplaceItem(int nTab, const CString &csTabTitle, CWnd* pTabWnd)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
	{
		if(m_Tabs[nTab].pWnd)
			m_Tabs[nTab].pWnd->ShowWindow(SW_HIDE);

		m_Tabs[nTab].csTitle = csTabTitle;
		m_Tabs[nTab].pWnd = pTabWnd;
		m_Tabs[nTab].lWidth = GetTextWidth(csTabTitle);

		if(pTabWnd)
			pTabWnd->ShowWindow(SW_SHOW);

		SetActiveTab(m_nActiveTab);

		return true;
	}

	return false;
}

bool CTabCtrlEx::DeleteItem(int nTab)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
	{
		// Remove the tab
		m_Tabs.RemoveAt(nTab);

		// Set the new active tab
		if (nTab == GetActiveTab())
		{
			if(nTab > 0)
				nTab--;
			SetActiveTab(nTab, true);
		}
		
		// Redraw the window
		RedrawWindow();

		return true;
	}

	return false;
}

bool CTabCtrlEx::DeleteAllItems()
{
	// Remove all the tab
	m_Tabs.RemoveAll();
		
	// Reset the active tab
	m_nActiveTab = -1;

	// Redraw the window
	RedrawWindow();

	return true;
}

void CTabCtrlEx::UnderlineTabTitle(int nTab, COLORREF clr)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
	{
		m_Tabs[nTab].clrUnderline = clr;
		Invalidate();
	}
}

int CTabCtrlEx::GetTextWidth(const CString& csText)
{
	CDC *pDC = GetDC();
	if (pDC)
	{
		// Select the font
		CFont *pPrevFont = pDC->SelectObject(m_pFntBoldText);
		
		// Get the width
		int nWidth = pDC->GetTextExtent(csText).cx;
		
		// Restore the font
		pDC->SelectObject(pPrevFont);
		ReleaseDC(pDC);

		return (nWidth + 2*TEXT_PAD);
	}
	
	return 0;
}

int CTabCtrlEx::GetDisplayWidth()
{
	CRect rcWnd;
	GetClientRect(rcWnd);
	
	// If all the tabs cannot fit, then the spinner is visiable
	if (GetTabsWidth() > rcWnd.Width())
		return rcWnd.Width()-GetSpinnerWidth();

	return rcWnd.Width();
}

int CTabCtrlEx::GetTabsWidth()
{
	int nWidth = 0;

	// Get the width for all tabs
	for (int iTab=0; iTab<m_Tabs.GetSize(); iTab++)
		nWidth += m_Tabs[iTab].lWidth;

	nWidth += TEXT_PAD;

	return nWidth;
}

int CTabCtrlEx::GetSpinnerWidth()
{
	int nSpinWidth = m_nTabHeight-2*1;
	return (2*nSpinWidth + SPIN_PAD);
}

void CTabCtrlEx::GetFullRect(CRect& rcTab)
{
	// Get the area of the full sheet window
	GetClientRect(rcTab);
	
	// Get the full tab area
	if (m_nStyle & SCS_TOP)
		rcTab.bottom = m_nTabHeight;
	else
		rcTab.top = rcTab.Height()-m_nTabHeight;
}

void CTabCtrlEx::GetTabListRect(CRect& rcTab)
{
	// Get the full tab area
	GetFullRect(rcTab);
	
	// Get the area for the tabs
	if (m_nStyle & SCS_TOP)
		rcTab.bottom -= 1;
	else
		rcTab.top += 1;
}

void CTabCtrlEx::GetTabRect(int nTab, CRect& rcTab)
{
	// Get the area for the tabs
	GetTabListRect(rcTab);

	rcTab.top += 2;

	int nLeft = 0;
	// Get the area for the current tab
	for (int iTab=0; iTab<m_Tabs.GetSize(); iTab++)
	{
		if (iTab == nTab)
		{
			rcTab.left = nLeft;
			rcTab.right = nLeft+m_Tabs[iTab].lWidth;
			break;
		}

		nLeft += m_Tabs[iTab].lWidth;
	}
}

void CTabCtrlEx::GetSpinnerRect(CRect& rcSpin)
{
	// Get the area for the tabs
	GetTabListRect(rcSpin);

	// If all the tabs cannot fit, then show the spinner
	if (GetTabsWidth() > rcSpin.Width())
		rcSpin.left = rcSpin.right-GetSpinnerWidth();
	// Set the spinner width to zero
	else
		rcSpin.SetRectEmpty();
}

void CTabCtrlEx::GetButtonRect(int nBtn, CRect& rcBtn)
{
	GetSpinnerRect(rcBtn);
	rcBtn.DeflateRect(SPIN_PAD, 0, 0, 0);
	
	rcBtn.DeflateRect(1, 1);
	if (nBtn == 0)
		rcBtn.right = rcBtn.left + rcBtn.Width()/2;
	else if (nBtn == 1)
		rcBtn.left = rcBtn.left + rcBtn.Width()/2;
	else
		rcBtn.SetRectEmpty();
}

CString CTabCtrlEx::GetTabTitle(int nTab)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
		return m_Tabs[nTab].csTitle;

	return "";
}

bool CTabCtrlEx::SetTabTitle(int nTab, const CString& csTabTitle)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
	{
		m_Tabs[nTab].csTitle = csTabTitle;
		m_Tabs[nTab].lWidth = GetTextWidth(csTabTitle);

		// Redraw the window
		RedrawWindow();

		return true;
	}

	return false;
}

bool CTabCtrlEx::SetTabItemData(int nTab, long lItemData)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
	{
		m_Tabs[nTab].lItemData = lItemData;

		return true;
	}

	return false;
}

long CTabCtrlEx::GetTabItemData(int nTab)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
	{
		return m_Tabs[nTab].lItemData;
	}

	return -1;
}

int CTabCtrlEx::GetActiveTab()
{
	return m_nActiveTab;
}

int CTabCtrlEx::GetTabCount()
{
	return (int)m_Tabs.GetSize();
}

void CTabCtrlEx::SetActiveTab(int nTab, bool bNotify)
{
	ActivateTab(nTab, bNotify);
}

void CTabCtrlEx::ActivateTab(int nTab, bool bNotify, bool bOnSize)
{
	if ((nTab >= 0) && (nTab < m_Tabs.GetSize()))
	{
		// Set the new active tab
		int nOldTab = GetActiveTab();
		m_nActiveTab = nTab;
		
		// Resize the tab windows
		ResizeTabWindow(nOldTab, GetActiveTab(), bNotify, bOnSize);

		// Make the tab visible
		MakeTabVisible(GetActiveTab());

		// Redraw the tabs
		if (bOnSize)
			InvalidateRect(NULL);
		else
			RedrawWindow();
	}
}

void CTabCtrlEx::ResizeTabWindow(int nOldTab, int nNewTab, bool bNotify, bool bOnSize)
{
	long lOldItemData = -1;
	long lNewItemData = -1;

	if ((nOldTab >= 0) && nOldTab < GetTabCount())
	{
		lOldItemData = m_Tabs[nOldTab].lItemData;
		CWnd* pPrevWnd = m_Tabs[nOldTab].pWnd;
		// Hide the previous tab
		if (pPrevWnd && IsWindow(pPrevWnd->m_hWnd))
			pPrevWnd->ShowWindow(SW_HIDE);
	}

	if ((nNewTab >= 0) && nNewTab < GetTabCount())
	{
		CRect rcWnd;
		//GetClientRect(rcWnd);
		GetWindowRect(rcWnd);

		lNewItemData = m_Tabs[nNewTab].lItemData;

		CWnd *pParentWnd = GetParent();
		if (pParentWnd && IsWindow(pParentWnd->m_hWnd))
			pParentWnd->ScreenToClient(rcWnd);
		
		int nPad = 0;
		CWnd* pNextWnd = m_Tabs[nNewTab].pWnd;;
		// Show the new tab and bring it to the top, set the focus to the new tab
		if (pNextWnd && IsWindow(pNextWnd->m_hWnd))
		{
			// Resize the tab window
			if (m_nStyle & SCS_TOP)
			{
				pNextWnd->SetWindowPos(&wndTop, rcWnd.left+nPad, rcWnd.top+m_nTabHeight+nPad,
					rcWnd.Width()-2*nPad, rcWnd.Height()-m_nTabHeight-2*nPad, SWP_SHOWWINDOW);
			}
			else
			{
				pNextWnd->SetWindowPos(&wndTop, rcWnd.left+nPad, rcWnd.top+nPad,
					rcWnd.Width()-2*nPad, rcWnd.Height()-m_nTabHeight-2*nPad, SWP_SHOWWINDOW);
			}

			if(m_bSetFocusToNewlySelectedTab)
			{
				// Set the focus
				if (bOnSize == FALSE)
					pNextWnd->SetFocus();
			}
		}
	}

	if (bNotify)
	{
		// Send a notification message to the parent window
		CWnd *pParentWnd = GetParent();
		if (pParentWnd && IsWindow(pParentWnd->m_hWnd))
		{
			NMTABCHANGE nmTab;
			ZeroMemory(&nmTab, sizeof(nmTab));
			nmTab.hdr.code = SN_SETACTIVETAB;
			nmTab.hdr.hwndFrom = GetSafeHwnd();
			nmTab.hdr.idFrom = GetDlgCtrlID();
			nmTab.lOldTab = nOldTab;
			nmTab.lNewTab = nNewTab;
			nmTab.lOldItemData = lOldItemData;
			nmTab.lNewItemData = lNewItemData;
			
			pParentWnd->SendMessage(WM_NOTIFY, (WPARAM) nmTab.hdr.idFrom, (LPARAM) &nmTab);
		}
	}
}

void CTabCtrlEx::MakeTabVisible(int nTab)
{
	CRect rcTab;
	GetTabRect(nTab, rcTab);
	rcTab.OffsetRect(-m_nLeftShifted, 0);

	if (rcTab.right > GetDisplayWidth())
		m_nLeftShifted += (rcTab.right-GetDisplayWidth()+TEXT_PAD);
	
	GetTabRect(nTab, rcTab);
	rcTab.OffsetRect(-m_nLeftShifted, 0);

	if (rcTab.left <= 0)
	{
		m_nLeftShifted += rcTab.left;
		if (m_nLeftShifted < 0)
		{
			ASSERT(FALSE);
			m_nLeftShifted = 0;
		}
	}
}

void CTabCtrlEx::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcUpdate(0, 0, 0, 0);
	
	// Enable spinners
	EnableSpinners();

	if (m_nActiveTab < 0)
	{
		// Set the first window to be the active tab
		ActivateTab(0, true, true);
	}

	// Draw the tabs
	DrawTabs(&dc);
}

void CTabCtrlEx::EnableSpinners()
{
	int nDispWidth = GetDisplayWidth();
	
	if (nDispWidth >= GetTabsWidth())
	{
		// We have enough space, disable the spinners
		m_nLeftShifted = 0;
		m_bBtnEnabled[0] = false;
		m_bBtnEnabled[1] = false;
	}
	else
	{
		// Enable/Disable the spinner buttons
		m_bBtnEnabled[0] = (m_nLeftShifted > 0);
		m_bBtnEnabled[1] = ((m_nLeftShifted+nDispWidth) < GetTabsWidth());
	}
}

void CTabCtrlEx::DrawTabs(CDC *pDC)
{
	// Draw the bar
	DrawBar(pDC);

	CRect rcTab;
	// Draw all the tabs but the active tab
	for (int iTab=0; iTab<m_Tabs.GetSize(); iTab++)
	{
		if (iTab == GetActiveTab())
			continue;

		GetTabRect(iTab, rcTab);
		rcTab.OffsetRect(-m_nLeftShifted, 0);
		DrawTab(iTab, pDC, rcTab);
	}

	// Draw the active tab
	GetTabRect(GetActiveTab(), rcTab);
	rcTab.OffsetRect(-m_nLeftShifted, 0);
	if (GetActiveTab() < m_Tabs.GetSize())
		DrawTab(GetActiveTab(), pDC, rcTab);

	// Draw the spinner buttons
	DrawSpinner(pDC);
}

void CTabCtrlEx::DrawBar(CDC* pDC)
{
	CRect rcBar;
	GetFullRect(rcBar);

	// Set the background color for the tabs
	pDC->FillRect(rcBar, &m_brNonSelectedTab);

	if (m_nStyle & SCS_BOLD)
	{
		// Draw the line
		if (m_nStyle & SCS_TOP)
		{
			pDC->MoveTo(CPoint(rcBar.left, rcBar.bottom-1));
			pDC->LineTo(CPoint(rcBar.right, rcBar.bottom-1));
		}
		else
		{
			pDC->MoveTo(rcBar.TopLeft());
			pDC->LineTo(CPoint(rcBar.right, rcBar.top));
		}
	}
}

void CTabCtrlEx::DrawTab(int nTab, CDC *pDC, CRect& rcTab)
{
	if(nTab < 0)
		return;

	DrawTabEx(nTab, pDC, rcTab);
	return;

	// Paint the text centered.
	CPoint ptArr[5];
	if (m_nStyle & SCS_TOP)
	{
		ptArr[0] = CPoint(rcTab.left, rcTab.bottom-1);
		ptArr[1] = CPoint(rcTab.left+TEXT_PAD, rcTab.top);
		ptArr[2] = CPoint(rcTab.right, rcTab.top);
		ptArr[3] = CPoint(rcTab.right+TEXT_PAD, rcTab.bottom);
		ptArr[4] = CPoint(rcTab.left, rcTab.bottom);
	}
	else
	{
		ptArr[0] = rcTab.TopLeft();
		ptArr[1] = CPoint(rcTab.left+TEXT_PAD, rcTab.bottom-1);
		ptArr[2] = CPoint(rcTab.right, rcTab.bottom-1);
		ptArr[3] = CPoint(rcTab.right+TEXT_PAD, rcTab.top-1);
		ptArr[4] = rcTab.TopLeft();
	}
	
	// Draw the Tab
	CRgn rgn;
	if (rgn.CreatePolygonRgn(ptArr, 4, WINDING) == FALSE)
		return;

	CFont *pFont = NULL;
	if (nTab == GetActiveTab())
	{
		pDC->FillRgn(&rgn, &m_brSelectedTab);
		pFont = m_pFntBoldText;
	}
	else
	{
		pDC->FillRgn(&rgn, &m_brNonSelectedTab);
		pFont = m_pFntText;
	}

	CPen *pPen = NULL;
	if ((m_nStyle & SCS_BOLD) == 0)
		pPen = &m_penGray;
	
	CPen *pPrevPen = pDC->SelectObject(pPen);
	pDC->Polyline(ptArr, 4);
	pDC->SelectObject(pPrevPen);
	
	// Draw the text
	CFont *pPrevFont = pDC->SelectObject(pFont);
	rcTab.right += TEXT_PAD;
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(m_Tabs[nTab].csTitle, rcTab, DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
	pDC->SelectObject(pPrevFont);

	if (m_Tabs[nTab].clrUnderline != -1)
	{
		pDC->Draw3dRect(CRect(rcTab.left+TEXT_PAD+2, rcTab.bottom-2, rcTab.right-TEXT_PAD-2, rcTab.bottom),
			m_Tabs[nTab].clrUnderline, m_Tabs[nTab].clrUnderline);
	}
}

void CTabCtrlEx::DrawTabEx(int nTab, CDC *pDC, CRect& rcTab)
{
	if(nTab < 0)
		return;

	CRect rcOrig(rcTab);
	rcTab.OffsetRect(1, 1);
	CFont *pFont = NULL;
	if (nTab == GetActiveTab())
	{
		pDC->FillSolidRect(rcTab, m_SelectedColor);
		pFont = m_pFntBoldText;
	}
	else
	{
		pDC->FillSolidRect(rcTab, m_NonSelectedColor);
		pFont = m_pFntText;
	}

	//Don't draw a line on the item before the selected tab
	if (nTab != GetActiveTab()-1)
	{
		int nBottomOffset = 0;
		int nTopOffset = 0;
		CPen *pPen = NULL;
		if (nTab == GetActiveTab())
		{
			pPen = &m_penBlack;
			nTopOffset = -1;
		}
		else
		{
			pPen = &m_penGray;
			nBottomOffset = 3;
			nTopOffset = 2;
		}

		CPen *pPrevPen = pDC->SelectObject(pPen);
		pDC->MoveTo(CPoint(rcTab.right-1, rcTab.bottom-nBottomOffset));
		pDC->LineTo(CPoint(rcTab.right-1, rcTab.top+nTopOffset));
		pDC->SelectObject(pPrevPen);
	}

	// Draw the text
	CFont *pPrevFont = pDC->SelectObject(pFont);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(m_Tabs[nTab].csTitle, rcOrig, DT_CENTER|DT_VCENTER|DT_END_ELLIPSIS);
	pDC->SelectObject(pPrevFont);
}

void CTabCtrlEx::DrawSpinner(CDC *pDC)
{
	CRect rcSpin;
	GetSpinnerRect(rcSpin);

	if (rcSpin.Width() == 0)
		return;

	// Set the background color
	pDC->FillRect(rcSpin, &m_brNonSelectedTab);

	// Draw the line
	pDC->MoveTo(CPoint(rcSpin.left+1, rcSpin.top));
	pDC->LineTo(CPoint(rcSpin.left+1, rcSpin.bottom));
	rcSpin.DeflateRect(SPIN_PAD, 0, 0, 0);

	CRect rcBtn;
	GetButtonRect(0, rcBtn);
	DrawButton(pDC, rcBtn, m_btnState[0], ArrowLeft, m_bBtnEnabled[0]);
	GetButtonRect(1, rcBtn);
	DrawButton(pDC, rcBtn, m_btnState[1], ArrowRight, m_bBtnEnabled[1]);
}

void CTabCtrlEx::DrawButton(CDC *pDC, CRect& rcBtn, ButtonState btnState,
	ButtonStyle btnStyle, bool bEnable)
{
	// Draw the arrow
	CPoint ptArr[3];
	int x = rcBtn.left + rcBtn.Width()/3;
	int y = rcBtn.top + rcBtn.Height()/2;	
	long lHeight = rcBtn.Height()/4;

	CPen *pPen = NULL;

	// Change pen if button disabled
	if (bEnable == false)
		pPen = &m_penGray;
	
	CPen *pPrevPen = pDC->SelectObject(pPen);

	// Draw the left arrow
	if (btnStyle == ArrowLeft)
	{
		for (int iHt=0; iHt<=lHeight; x++, iHt++)
		{
			pDC->MoveTo(x, y-iHt);
			pDC->LineTo(x, y+iHt+1);
		}
	}
	// Draw the right arrow
	else
	{
		for (int iHt=lHeight; iHt>=0; x++, iHt--)
		{
			pDC->MoveTo(x, y-iHt);
			pDC->LineTo(x, y+iHt+1);
		}
	}
	pDC->SelectObject(pPrevPen);

	// Draw the button frame
	if (btnState == BtnUp)
		pDC->Draw3dRect(rcBtn, COLOR_WHITE, COLOR_DARKGRAY);
	else// if (btnState == BtnDown)
		pDC->Draw3dRect(rcBtn, COLOR_DARKGRAY, COLOR_WHITE);
}

void CTabCtrlEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcBtn;
	bool bOnButton = false;
	for (int iBtn=0; iBtn<2; iBtn++)
	{
		GetButtonRect(iBtn, rcBtn);
		if (rcBtn.PtInRect(point) && m_bBtnEnabled[iBtn])
		{
			m_btnState[iBtn] = BtnDown;
			bOnButton = true;
		}
	}

	if (bOnButton)
	{
		// Get the mouse capture
		SetCapture();

		// Start the timer message
		SetTimer(ID_SCROLL_TIMER, 100, NULL);

		// Redraw the buttons
		RedrawWindow();
	}
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CTabCtrlEx::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == ID_SCROLL_TIMER)
	{
		CPoint point(0, 0);
		GetCursorPos(&point);
		ScreenToClient(&point);
		ScrollTab(point);

		// Redraw the buttons and tabs
		RedrawWindow();
	}
	
	CWnd::OnTimer(nIDEvent);
}

void CTabCtrlEx::OnMouseMove(UINT nFlags, CPoint point) 
{
	/*
	CRect rcBtn;
	bool bRedrawButton = false;
	for (int iBtn=0; iBtn<2; iBtn++)
	{
		GetButtonRect(iBtn, rcBtn);
		if (rcBtn.PtInRect(point) && m_bBtnEnabled[iBtn])
		{
			if (m_btnState[iBtn] != BtnHover)
			{
				m_btnState[iBtn] = BtnHover;
				bRedrawButton = true;
				
				// Get the mouse capture
				SetCapture();
			}
		}
		else
		{
			if (m_btnState[iBtn] != BtnUp)
			{
				m_btnState[iBtn] = BtnUp;
				bRedrawButton = true;

				// Release the mouse capture
				ReleaseCapture();
			}
		}
	}

	if (bRedrawButton)
	{
		// Redraw the buttons
		RedrawWindow();
	}
	*/
	
	CWnd::OnMouseMove(nFlags, point);
}

void CTabCtrlEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	// Release the mouse capture
	ReleaseCapture();

	// End the timer
	KillTimer(ID_SCROLL_TIMER);
	
	// Check and scroll the tabs
	ScrollTab(point);
	
	// Set the button state
	for (int iBtn=0; iBtn<2; iBtn++)
		m_btnState[iBtn] = BtnUp;
	
	// Redraw the buttons
	RedrawWindow();

	CWnd::OnLButtonUp(nFlags, point);
}

void CTabCtrlEx::ScrollTab(CPoint point)
{
	CRect rcBtn;
	for (int iBtn=0; iBtn<2; iBtn++)
	{
		GetButtonRect(iBtn, rcBtn);
		if (rcBtn.PtInRect(point))
		{
			if (m_bBtnEnabled[iBtn])
			{
				m_btnState[iBtn] = BtnDown;

				if (iBtn == 0)
				{
					m_nLeftShifted -= SHIFT_UNITS;
					if (m_nLeftShifted < 0)
						m_nLeftShifted = 0;
				}
				else
				{
					m_nLeftShifted += SHIFT_UNITS;
				}	
			}
			else
			{
				m_btnState[iBtn] = BtnUp;
			}
		}
		else
		{
			m_btnState[iBtn] = BtnUp;
		}
	}

	CRect rcSpin;
	GetSpinnerRect(rcSpin);
	if (rcSpin.PtInRect(point) == FALSE)
	{
		// Adjust for the shift
		point.Offset(m_nLeftShifted, 0);
		
		CRect rcTab;
		for (int iTab=0; iTab<m_Tabs.GetSize(); iTab++)
		{
			GetTabRect(iTab, rcTab);
			if (rcTab.PtInRect(point))
			{
				// Show the appropriate tab window
				ActivateTab(iTab, true);
			}
		}
	}
}

void CTabCtrlEx::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	// Resize the tabs
	if (m_nLeftShifted && (cx > m_nPrevWidth))
	{
		int nDispWidth = cx-GetSpinnerWidth();
		if ((nDispWidth+m_nLeftShifted) >= GetTabsWidth())
		{
			m_nLeftShifted -= (cx-m_nPrevWidth);
			if (m_nLeftShifted <= GetSpinnerWidth())
				m_nLeftShifted = 0;
		}
	}

	ActivateTab(GetActiveTab(), false, true);
	
	m_nPrevWidth = cx;
}

BOOL CTabCtrlEx::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (pMsg->wParam == VK_TILDAE)
			{
				SwitchTabs((GetKeyState(VK_SHIFT) & 0x8000) == 0);
				return TRUE;
			}
			else if (((pMsg->wParam - '1') >= 0) && ((pMsg->wParam - '1') < (UINT) m_Tabs.GetSize()))
			{
				// Set the new active tab
				ActivateTab((int)pMsg->wParam - '1', true);
				return TRUE;
			}
		}
		break;
	}
	
	return FALSE;
}

void CTabCtrlEx::SwitchTabs(bool bNext)
{
	int nNewTab = GetActiveTab();
	if (bNext)
		nNewTab++;
	else
		nNewTab--;

	if (nNewTab < 0)
		nNewTab = (int)m_Tabs.GetSize()-1;
	else if (nNewTab >= m_Tabs.GetSize())
		nNewTab = 0;

	// Set the new active tab
	ActivateTab(nNewTab, true);
}

void CTabCtrlEx::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (bShow)
	{
		// Show the active tab
		SetActiveTab(GetActiveTab(), false);
	}
	else
	{	
		// Hide the active tab
		if (GetActiveTab() >= 0)
		{
			CWnd* pWnd = m_Tabs[GetActiveTab()].pWnd;
			if (pWnd && IsWindow(pWnd->m_hWnd))
				pWnd->ShowWindow(SW_HIDE);
		}
	}
}