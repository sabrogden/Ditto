#include "stdafx.h"
#include "cp_main.h"
#include "ToolTipEx.h"
#include "BitmapHelper.h"
#include "Options.h"
#include <Richedit.h>

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif 

#define DELETE_BITMAP	if(m_imageViewer.m_pBitmap)					\
{								\
m_imageViewer.m_pBitmap->DeleteObject();	\
delete m_imageViewer.m_pBitmap;		\
m_imageViewer.m_pBitmap = NULL;		\
}

#define HIDE_WINDOW_TIMER 1
#define SAVE_SIZE 2
#define TIMER_BUTTON_UP 3

/////////////////////////////////////////////////////////////////////////////
// CToolTipEx

CToolTipEx::CToolTipEx(): m_dwTextStyle(DT_EXPANDTABS | DT_EXTERNALLEADING |
                       DT_NOPREFIX | DT_WORDBREAK), m_rectMargin(2, 2, 3, 3),
                        m_pNotifyWnd(NULL), m_clipId(0), m_clipRow(-1)
{
}

CToolTipEx::~CToolTipEx()
{
    DELETE_BITMAP 
    m_Font.DeleteObject();
	m_clipDataFont.DeleteObject();
}

BEGIN_MESSAGE_MAP(CToolTipEx, CWnd)
//{{AFX_MSG_MAP(CToolTipEx)
ON_WM_PAINT()
ON_WM_SIZE()
ON_WM_NCHITTEST()
ON_WM_ACTIVATE()
ON_WM_TIMER()

ON_WM_NCPAINT()
ON_WM_NCCALCSIZE()
ON_WM_NCLBUTTONDOWN()
ON_WM_NCMOUSEMOVE()
ON_WM_NCLBUTTONUP()
ON_WM_ERASEBKGND()
ON_COMMAND(ID_FIRST_REMEMBERWINDOWPOSITION, &CToolTipEx::OnRememberwindowposition)
ON_COMMAND(ID_FIRST_SIZEWINDOWTOCONTENT, &CToolTipEx::OnSizewindowtocontent)
ON_COMMAND(ID_FIRST_SCALEIMAGESTOFITWINDOW, &CToolTipEx::OnScaleimagestofitwindow)
ON_COMMAND(2, OnOptions)
ON_WM_RBUTTONDOWN()
ON_WM_SETFOCUS()
ON_COMMAND(ID_FIRST_HIDEDESCRIPTIONWINDOWONM, &CToolTipEx::OnFirstHidedescriptionwindowonm)
ON_COMMAND(ID_FIRST_WRAPTEXT, &CToolTipEx::OnFirstWraptext)
ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CToolTipEx message handlers

BOOL CToolTipEx::Create(CWnd *pParentWnd)
{
	m_saveWindowLockout = true;

    // Get the class name and create the window
    CString szClassName = AfxRegisterWndClass(CS_CLASSDC | CS_SAVEBITS, LoadCursor(NULL, IDC_ARROW));

    // Create the window - just don't show it yet.
    if( !CWnd::CreateEx(0, szClassName, _T(""), WS_POPUP,
       0, 0, 0, 0, pParentWnd->GetSafeHwnd(), 0, NULL))
    {
        return FALSE;
    }	

	//CString szClassName2 = AfxRegisterWndClass(CS_CLASSDC | CS_SAVEBITS, LoadCursor(NULL, IDC_ARROW));
	//BOOL b = m_imageViewer.Create(_T(""), szClassName2, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL, CRect(0, 0, 0, 0), this, 3);
	m_imageViewer.Create(this);
	
	
	m_DittoWindow.DoCreate(this);
	m_DittoWindow.SetCaptionColors(g_Opt.m_Theme.CaptionLeft(), g_Opt.m_Theme.CaptionRight(), g_Opt.m_Theme.Border());
	m_DittoWindow.SetCaptionOn(this, CGetSetOptions::GetCaptionPos(), true, g_Opt.m_Theme.GetCaptionSize(), g_Opt.m_Theme.GetCaptionFontSize());
	m_DittoWindow.m_bDrawMaximize = false;
	m_DittoWindow.m_bDrawMinimize = false;
	m_DittoWindow.m_bDrawChevron = false;
	m_DittoWindow.m_sendWMClose = false;

    m_RichEdit.Create(_T(""), _T(""), WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL |
                      ES_AUTOHSCROLL, CRect(10, 10, 100, 200), this, 1);

    m_RichEdit.SetReadOnly();
    m_RichEdit.SetBackgroundColor(FALSE, g_Opt.m_Theme.DescriptionWindowBG());

	ApplyWordWrap();

    SetLogFont(GetSystemToolTipFont(), FALSE);

	m_optionsButton.Create(NULL, WS_CHILD | BS_OWNERDRAW | WS_TABSTOP, CRect(0, 0, 0, 0), this, 2);
	m_optionsButton.LoadStdImageDPI(IDB_COG_16_16, IDB_COG_20_20, IDB_COG_24_24, cog_28, IDB_COG_32_32, _T("PNG"));
	m_optionsButton.SetToolTipText(theApp.m_Language.GetString(_T("DescriptionOptionsTooltip"), _T("Description Options")));
	m_optionsButton.ShowWindow(SW_SHOW);

	m_clipDataStatic.Create(_T("some text"), WS_CHILD | WS_VISIBLE | SS_SIMPLE, CRect(0, 0, 0, 0), this, 3);

	m_clipDataFont.CreateFont(-theApp.m_metrics.PointsToPixels(8), 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, 3, 2, 1, 34, _T("Segoe UI"));
	m_clipDataStatic.SetFont(&m_clipDataFont);
	m_clipDataStatic.SetBkColor(g_Opt.m_Theme.DescriptionWindowBG());
	m_clipDataStatic.SetTextColor(RGB(127, 127, 127));

	m_saveWindowLockout = false;

    return TRUE;
}

BOOL CToolTipEx::Show(CPoint point)
{
	m_reducedWindowSize = false;
    if(m_imageViewer.m_pBitmap)
    {
        m_RichEdit.ShowWindow(SW_HIDE);
		m_imageViewer.ShowWindow(SW_SHOW);

		m_imageViewer.UpdateBitmapSize();
    }
    else
    {
        m_RichEdit.ShowWindow(SW_SHOW);
		m_imageViewer.ShowWindow(SW_HIDE);
    }

	CRect rect;

	if(CGetSetOptions::GetSizeDescWindowToContent() == FALSE)
	{
		rect.left = point.x;
		rect.top = point.y;
		CSize size;
		CGetSetOptions::GetDescWndSize(size);
		rect.right = rect.left + size.cx;
		rect.bottom = rect.top + size.cy;

		EnsureWindowVisible(&rect);
	}
	else
	{
		rect = GetBoundsRect();

		//account for the scroll bars
		rect.right += 20;
		rect.bottom += 20;

		if (m_imageViewer.m_pBitmap)
		{
			int nWidth = CBitmapHelper::GetCBitmapWidth(*m_imageViewer.m_pBitmap) + ::GetSystemMetrics(SM_CXVSCROLL);
			int nHeight = CBitmapHelper::GetCBitmapHeight(*m_imageViewer.m_pBitmap) + ::GetSystemMetrics(SM_CYHSCROLL);

			rect.right = rect.left + nWidth;
			rect.bottom = rect.top + nHeight;
		}
		else if(m_csRTF != "")
		{
			//if showing rtf then increase the size because
			//rtf will probably draw bigger
			long lNewWidth = (long)rect.Width() + (long)(rect.Width() *1.5);
			rect.right = rect.left + lNewWidth;

			long lNewHeight = (long)rect.Height() + (long)(rect.Height() *1.5);
			rect.bottom = rect.top + lNewHeight;
		}

		rect.right += CAPTION_BORDER * 2;
		rect.bottom += CAPTION_BORDER * 2;

		

		CRect rcScreen;

		ClientToScreen(rect);

		CRect cr(point, point);

		int nMonitor = GetMonitorFromRect(&cr);
		GetMonitorRect(nMonitor, &rcScreen);

		//ensure that we don't go outside the screen
		if(point.x < 0)
		{
			point.x = 5;
			m_reducedWindowSize = true;
		}
		if(point.y < 0)
		{
			point.y = 5;
			m_reducedWindowSize = true;
		}

		rcScreen.DeflateRect(0, 0, 5, 5);

		long lWidth = rect.Width();
		long lHeight = rect.Height();

		rect.left = point.x;
		rect.top = point.y;
		rect.right = rect.left + lWidth;
		rect.bottom = rect.top + lHeight;
		
		if (rect.right > rcScreen.right)
		{
			rect.right = rcScreen.right;
			m_reducedWindowSize = true;
		}
		if (rect.bottom > rcScreen.bottom)
		{
			rect.bottom = rcScreen.bottom;
			m_reducedWindowSize = true;
		}
	}

	m_clipDataStatic.SetWindowText(m_clipData);	

	m_saveWindowLockout = true;
	MoveWindow(rect);
	this->Invalidate();
	ShowWindow(SW_SHOWNA);	
	
	m_saveWindowLockout = false;

    return TRUE;
}

BOOL CToolTipEx::Hide()
{
	DELETE_BITMAP

		SaveWindowSize();

    ShowWindow(SW_HIDE);

    m_csRTF = "";
    m_csText = "";
	m_clipId = 0;
	m_clipRow = -1;
	m_searchText = _T("");	

    return TRUE;
}

void CToolTipEx::SaveWindowSize()
{
	if (::IsWindowVisible(m_hWnd))
	{
		CRect rect;
		this->GetWindowRect(&rect);
		CGetSetOptions::SetDescWndSize(rect.Size());
		CGetSetOptions::SetDescWndPoint(rect.TopLeft());

		OutputDebugString(_T("Saving tooltip size"));
	}
}

void CToolTipEx::PostNcDestroy()
{
    CWnd::PostNcDestroy();

    delete this;
}

BOOL CToolTipEx::PreTranslateMessage(MSG *pMsg)
{
	m_DittoWindow.DoPreTranslateMessage(pMsg);

    switch(pMsg->message)
    {
        case WM_KEYDOWN:

            switch(pMsg->wParam)
            {
            case VK_ESCAPE:
                Hide();
                return TRUE;
            case 'C':
                if(GetKeyState(VK_CONTROL) &0x8000)
                {
                    m_RichEdit.Copy();
                }
                break;
			case VK_F3:
			{
				DoSearch();
			}
			break;
            }
			break;
		case WM_RBUTTONDOWN:
			{
				if (m_RichEdit.m_hWnd == GetFocus()->m_hWnd ||
					m_imageViewer.m_hWnd == GetFocus()->m_hWnd)
				{
					OnOptions();
					return TRUE;
				}
			}
			break;
    }

    return CWnd::PreTranslateMessage(pMsg);
}

BOOL CToolTipEx::OnMsg(MSG *pMsg)
{
    if(FALSE == IsWindowVisible())
    {
        return FALSE;
    }

    switch(pMsg->message)
    {
        case WM_WINDOWPOSCHANGING:
        case WM_LBUTTONDOWN:
            {
				if (CGetSetOptions::GetMouseClickHidesDescription())
				{
					if (!IsCursorInToolTip())
					{
						Hide();
					}
				}
            }
			break;
        case WM_KEYDOWN:
            {
                WPARAM vk = pMsg->wParam;
                if(vk == VK_ESCAPE)
                {
                    Hide();
                    return TRUE;
                }
                else if(vk == VK_TAB)
                {
                    m_RichEdit.SetFocus();
                    return TRUE;
                }
				else if(vk == 'N')
				{
					return FALSE;
				}
				else if (vk == 'P')
				{
					return FALSE;
				}
				else if (vk == VK_UP)
				{
					return FALSE;
				}
				else if (vk == VK_DOWN)
				{
					return FALSE;
				}
				else if(vk == VK_F3)
				{
					DoSearch();

					return TRUE;
				}
				else if(vk == VK_SHIFT)
				{
					return FALSE;
				}
				else if(vk == VK_NEXT)
				{
					return FALSE;
				}
				else if (vk == VK_PRIOR)
				{
					return FALSE;
				}

                Hide();

                break;
            }

        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONDBLCLK:
            {
                Hide();
                break;
            }
    }

    return FALSE;
}

CRect CToolTipEx::GetBoundsRect()
{
	DWORD d = GetTickCount();

    CWindowDC dc(NULL);
	int nLineWidth = 0;

	CRect rect(0, 0, 0, 0);

    if(nLineWidth == 0)
    {
        // Count the number of lines of text
		int nStart = 0;
		INT nNumLines = 0;
		int longestLength = 0;
		CString longestString;
        do
        {
            int newStart = m_csText.Find(_T("\n"), nStart);
			if (newStart < 0)
			{
				int length = m_csText.GetLength() - nStart;
				if (length > longestLength)
				{
					longestString = m_csText.Mid(nStart, length);
					longestLength = length;
				}

				break;
			}

			int length = newStart - nStart;
			if(length > longestLength)
			{
				longestString = m_csText.Mid(nStart, length);
				longestLength = length;
			}           

            nNumLines++;
			nStart = newStart + 1;
        }
        while(nStart >= 0 && nNumLines < 100);

		CFont *pOldFont = (CFont*)dc.SelectObject((CFont*)&m_Font);
		CSize size = dc.GetTextExtent(longestString);  
		dc.SelectObject(pOldFont);

		rect.right = size.cx;
		rect.bottom = size.cy * nNumLines;
    }    

    rect.bottom += m_rectMargin.top + m_rectMargin.bottom;
    rect.right += m_rectMargin.left + m_rectMargin.right + 2;

    if(m_imageViewer.m_pBitmap)
    {
        int nWidth = CBitmapHelper::GetCBitmapWidth(*m_imageViewer.m_pBitmap);
        int nHeight = CBitmapHelper::GetCBitmapHeight(*m_imageViewer.m_pBitmap);

        rect.bottom += nHeight;
        if((rect.left + nWidth) > rect.right)
        {
            rect.right = rect.left + nWidth;
        }
    }

	DWORD diff = GetTickCount() - d;
	if (diff > 10)
	{
		Log(StrF(_T("Size To Content: %d\n"), diff));
	}

    return rect;
}

CString CToolTipEx::GetFieldFromString(CString ref, int nIndex, TCHAR ch)
{
    CString strReturn;
    LPCTSTR pstrStart = ref.LockBuffer();
    LPCTSTR pstrBuffer = pstrStart;
    int nCurrent = 0;
    int nStart = 0;
    int nEnd = 0;
    int nOldStart = 0;

    while(nCurrent <= nIndex &&  *pstrBuffer != _T('\0'))
    {
        if(*pstrBuffer == ch)
        {
            nOldStart = nStart;
            nStart = nEnd + 1;
            nCurrent++;
        }
        nEnd++;
        pstrBuffer++;
    }

    // May have reached the end of the string
    if(*pstrBuffer == _T('\0'))
    {
        nOldStart = nStart;
        nEnd++;
    }

    ref.UnlockBuffer();

    if(nCurrent < nIndex)
    {
        //TRACE1("Warning: GetStringField - Couldn't find field %d.\n", nIndex);
        return strReturn;
    }
    return ref.Mid(nOldStart, nEnd - nOldStart - 1);
}

LPLOGFONT CToolTipEx::GetSystemToolTipFont()
{
    static LOGFONT LogFont;

    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    if(!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
       &ncm, 0))
    {
        return FALSE;
    }

    memcpy(&LogFont, &(ncm.lfStatusFont), sizeof(LOGFONT));

    return  &LogFont;
}

BOOL CToolTipEx::SetLogFont(LPLOGFONT lpLogFont, BOOL bRedraw /*=TRUE*/)
{
    ASSERT(lpLogFont);
    if(!lpLogFont)
    {
        return FALSE;
    }

    LOGFONT LogFont;

    // Store font as the global default
    memcpy(&LogFont, lpLogFont, sizeof(LOGFONT));

    // Create the actual font object
    m_Font.DeleteObject();
    m_Font.CreateFontIndirect(&LogFont);

    if(bRedraw && ::IsWindow(GetSafeHwnd()))
    {
        Invalidate();
    }

    return TRUE;
}

void CToolTipEx::SetBitmap(CBitmap *pBitmap)
{
    DELETE_BITMAP 

	m_imageViewer.m_pBitmap = pBitmap;

	m_imageViewer.UpdateBitmapSize();

	if (m_imageViewer.m_pBitmap != NULL)
	{
		int nWidth = CBitmapHelper::GetCBitmapWidth(*m_imageViewer.m_pBitmap);
		int nHeight = CBitmapHelper::GetCBitmapHeight(*m_imageViewer.m_pBitmap);

		Invalidate();
	}
}

void CToolTipEx::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    if(::IsWindow(m_RichEdit.GetSafeHwnd()) == FALSE)
    {
        return ;
    }

    CRect cr;
    GetClientRect(cr);
    cr.DeflateRect(0, 0, 0, theApp.m_metrics.ScaleY(21));
    m_RichEdit.MoveWindow(cr);
	m_imageViewer.MoveWindow(cr);

	m_optionsButton.MoveWindow(cr.left, cr.bottom + theApp.m_metrics.ScaleY(2), theApp.m_metrics.ScaleX(17), theApp.m_metrics.ScaleY(17));

	m_clipDataStatic.MoveWindow(cr.left + theApp.m_metrics.ScaleX(19), cr.bottom + theApp.m_metrics.ScaleY(2), cr.Width() - cr.left + theApp.m_metrics.ScaleX(19), theApp.m_metrics.ScaleY(17));

	this->Invalidate();
	m_DittoWindow.DoSetRegion(this);

	if (m_saveWindowLockout == false)
	{
		SetTimer(SAVE_SIZE, 250, NULL);
	}
}

BOOL CToolTipEx::IsCursorInToolTip()
{
    CRect cr;
    GetWindowRect(cr);

    CPoint cursorPos;
    GetCursorPos(&cursorPos);

    return cr.PtInRect(cursorPos);
}

void CToolTipEx::SetRTFText(const char *pRTF)
{
    m_RichEdit.SetRTF(pRTF);
    m_csRTF = pRTF;
	m_RichEdit.SetSel(0, 0);
	
	HighlightSearchText();
}

//void CToolTipEx::SetRTFText(const CString &csRTF)
//{
//	m_RichEdit.SetRTF(csRTF);
//	m_csRTF = csRTF;
//}

void CToolTipEx::SetToolTipText(const CString &csText)
{
    m_csText = csText;
    m_RichEdit.SetFont(&m_Font);
    m_RichEdit.SetText(csText);
	m_RichEdit.SetSel(0, 0);

	CHARFORMAT cfNew;
	cfNew.cbSize = sizeof(CHARFORMAT);
	cfNew.dwMask = CFM_COLOR;
	cfNew.dwEffects = CFM_COLOR;
	cfNew.dwEffects &= ~CFE_AUTOCOLOR;
	cfNew.crTextColor = g_Opt.m_Theme.DescriptionWindowText();
	m_RichEdit.SetDefaultCharFormat(cfNew);

	HighlightSearchText();
}

void CToolTipEx::HighlightSearchText()
{
	if (m_searchText.GetLength() <= 0)
		return;

	FINDTEXTEX ft;
	long n = -1;

	ft.lpstrText = m_searchText;
	
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = -1;

	CHARFORMAT cf;

	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = CFE_BOLD | ~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(255, 0, 0);

	do 
	{
		ft.chrg.cpMin = n+1;
		n = m_RichEdit.FindText(FR_DOWN, &ft);
		if (n != -1)
		{
			m_RichEdit.SetSel(ft.chrgText);
			m_RichEdit.SetSelectionCharFormat(cf);
		}

	} while (n != -1);	

	m_RichEdit.SetSel(0, 0);
}

void CToolTipEx::DoSearch()
{
	if (m_searchText.GetLength() <= 0)
		return;

	FINDTEXTEX ft;
	long n = -1;

	ft.lpstrText = m_searchText;

	long start;
	long end;
	m_RichEdit.GetSel(start, end);

	ft.chrg.cpMin = end;
	ft.chrg.cpMax = -1;

	int searchDirection = FR_DOWN;
	if (GetKeyState(VK_SHIFT) & 0x8000)
	{
		searchDirection = 0;
		ft.chrg.cpMin = start;
	}

	n = m_RichEdit.FindText(searchDirection, &ft);
	if (n != -1)
	{
		m_RichEdit.SetSel(ft.chrgText);
	}
	else
	{
		if (searchDirection == 0)
		{
			ft.chrg.cpMin = m_RichEdit.GetTextLength();
		}
		else
		{
			ft.chrg.cpMin = 0;
		}
		ft.chrg.cpMax = -1;

		n = m_RichEdit.FindText(searchDirection, &ft);
		if (n != -1)
		{
			m_RichEdit.SetSel(ft.chrgText);
		}
	}
}

void CToolTipEx::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
    CWnd::OnActivate(nState, pWndOther, bMinimized);

	if (nState == WA_INACTIVE)
	{		
        if(m_pNotifyWnd)
        {
            m_pNotifyWnd->PostMessage(NM_INACTIVE_TOOLTIPWND, 0, 0);
        }
    }
}

void CToolTipEx::OnTimer(UINT_PTR nIDEvent)
{
    switch(nIDEvent)
    {
        case HIDE_WINDOW_TIMER:
            Hide();
            PostMessage(WM_DESTROY, 0, 0);
            break;
		case SAVE_SIZE:
			SaveWindowSize();
			KillTimer(SAVE_SIZE);
			break;
		case TIMER_BUTTON_UP:
		{
			if ((GetKeyState(VK_LBUTTON) & 0x100) == 0)
			{
				m_DittoWindow.DoNcLButtonUp(this, 0, CPoint(0, 0));
				KillTimer(TIMER_BUTTON_UP);
			}
			break;
		}

    }

    CWnd::OnTimer(nIDEvent);
}


void CToolTipEx::OnNcPaint()
{
	m_DittoWindow.DoNcPaint(this);
}

void CToolTipEx::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	m_DittoWindow.DoNcCalcSize(bCalcValidRects, lpncsp);
}

HITTEST_RET CToolTipEx::OnNcHitTest(CPoint point) 
{
	UINT Ret = m_DittoWindow.DoNcHitTest(this, point);
	if(Ret == -1)
		return CWnd::OnNcHitTest(point);

	return Ret;
}

void CToolTipEx::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	int buttonPressed = m_DittoWindow.DoNcLButtonDown(this, nHitTest, point);

	if (buttonPressed != 0)
	{
		SetTimer(TIMER_BUTTON_UP, 100, NULL);
	}

	CWnd::OnNcLButtonDown(nHitTest, point);
}

void CToolTipEx::OnNcLButtonUp(UINT nHitTest, CPoint point) 
{
	long lRet = m_DittoWindow.DoNcLButtonUp(this, nHitTest, point);

	switch(lRet)
	{
	case BUTTON_CLOSE:
		Hide();
		break;
	}

	KillTimer(TIMER_BUTTON_UP);

	CWnd::OnNcLButtonUp(nHitTest, point);
}

void CToolTipEx::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	m_DittoWindow.DoNcMouseMove(this, nHitTest, point);

	CWnd::OnNcMouseMove(nHitTest, point);
}

void CToolTipEx::OnOptions()
{
	POINT pp;
	CMenu cmPopUp;
	CMenu *cmSubMenu = NULL;

	GetCursorPos(&pp);
	if(cmPopUp.LoadMenu(IDR_DESC_OPTIONS_MENU) != 0)
	{
		cmSubMenu = cmPopUp.GetSubMenu(0);
		if(!cmSubMenu)
		{
			return ;
		}

		GetCursorPos(&pp);

		//theApp.m_Language.UpdateRightClickMenu(cmSubMenu);

		if(CGetSetOptions::GetRememberDescPos())
			cmSubMenu->CheckMenuItem(ID_FIRST_REMEMBERWINDOWPOSITION, MF_CHECKED);

		if(CGetSetOptions::GetSizeDescWindowToContent())
			cmSubMenu->CheckMenuItem(ID_FIRST_SIZEWINDOWTOCONTENT, MF_CHECKED);

		if(CGetSetOptions::GetScaleImagesToDescWindow())
			cmSubMenu->CheckMenuItem(ID_FIRST_SCALEIMAGESTOFITWINDOW, MF_CHECKED);

		if (CGetSetOptions::GetMouseClickHidesDescription())
			cmSubMenu->CheckMenuItem(ID_FIRST_HIDEDESCRIPTIONWINDOWONM, MF_CHECKED);

		if (CGetSetOptions::GetWrapDescriptionText())
			cmSubMenu->CheckMenuItem(ID_FIRST_WRAPTEXT, MF_CHECKED);
		
		
		
		cmSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pp.x, pp.y, this, NULL);
	}
}

void CToolTipEx::OnRememberwindowposition()
{
	CGetSetOptions::SetRememberDescPos(!CGetSetOptions::GetRememberDescPos());
}

void CToolTipEx::OnSizewindowtocontent()
{
	CGetSetOptions::SetSizeDescWindowToContent(!CGetSetOptions::GetSizeDescWindowToContent());

	CRect rect;
	this->GetWindowRect(&rect);

	Show(rect.TopLeft());
}

void CToolTipEx::OnScaleimagestofitwindow()
{
	CGetSetOptions::SetScaleImagesToDescWindow(!CGetSetOptions::GetScaleImagesToDescWindow());
	m_imageViewer.UpdateBitmapSize();
	Invalidate();
}

void CToolTipEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	OnOptions();

	CWnd::OnRButtonDown(nFlags, point);
}


void CToolTipEx::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	m_RichEdit.SetFocus();
}

void CToolTipEx::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(rect);
	
	CBrush  Brush, *pOldBrush;
	Brush.CreateSolidBrush(g_Opt.m_Theme.DescriptionWindowBG());

	pOldBrush = dc.SelectObject(&Brush);

	dc.FillRect(&rect, &Brush);

	// Cleanup
	dc.SelectObject(pOldBrush);
}

void CToolTipEx::OnFirstHidedescriptionwindowonm()
{
	CGetSetOptions::SetMouseClickHidesDescription(!CGetSetOptions::GetMouseClickHidesDescription());
}

void CToolTipEx::OnFirstWraptext()
{
	CGetSetOptions::SetWrapDescriptionText(!CGetSetOptions::GetWrapDescriptionText());	
	ApplyWordWrap();
}

void CToolTipEx::ApplyWordWrap()
{
	if (CGetSetOptions::GetWrapDescriptionText())
	{
		m_RichEdit.SetTargetDevice(NULL, 0);
	}
	else
	{
		m_RichEdit.SetTargetDevice(NULL, 1);
	}
}

void CToolTipEx::HideWindowInXMilliSeconds(long lms) 
{ 
	SetTimer(HIDE_WINDOW_TIMER, lms, NULL); 
}

void CToolTipEx::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanging(lpwndpos);

	m_DittoWindow.SnapToEdge(this, lpwndpos);
}