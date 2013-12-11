#include "stdafx.h"
#include "cp_main.h"
#include "ToolTipEx.h"
#include "BitmapHelper.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif 

#define DELETE_BITMAP	if(m_pBitmap)					\
{								\
m_pBitmap->DeleteObject();	\
delete m_pBitmap;		\
m_pBitmap = NULL;		\
}


/////////////////////////////////////////////////////////////////////////////
// CToolTipEx

CToolTipEx::CToolTipEx(): m_dwTextStyle(DT_EXPANDTABS | DT_EXTERNALLEADING |
                       DT_NOPREFIX | DT_WORDBREAK), m_rectMargin(2, 2, 3, 3),
                       m_pBitmap(NULL), m_pNotifyWnd(NULL){}

CToolTipEx::~CToolTipEx()
{
    DELETE_BITMAP 

    m_Font.DeleteObject();
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

END_MESSAGE_MAP() 


/////////////////////////////////////////////////////////////////////////////
// CToolTipEx message handlers

BOOL CToolTipEx::Create(CWnd *pParentWnd)
{
    // Get the class name and create the window
    CString szClassName = AfxRegisterWndClass(CS_CLASSDC | CS_SAVEBITS,
        LoadCursor(NULL, IDC_ARROW));

    // Create the window - just don't show it yet.
    if( !CWnd::CreateEx(WS_EX_TOPMOST, szClassName, _T(""), WS_POPUP | WS_BORDER,
       0, 0, 10, 10,  // size & position updated when needed
    pParentWnd->GetSafeHwnd(), 0, NULL))
    {
        return FALSE;
    }

	m_DittoWindow.DoCreate(this);
	m_DittoWindow.SetCaptionColors(g_Opt.m_Theme.CaptionLeft(), g_Opt.m_Theme.CaptionRight());
	m_DittoWindow.SetCaptionOn(this, CGetSetOptions::GetCaptionPos(), true);
	m_DittoWindow.m_bDrawMinimize = false;
	m_DittoWindow.m_bDrawMinimize = false;
	m_DittoWindow.m_bDrawChevron = false;
	m_DittoWindow.m_sendWMClose = false;

    m_RichEdit.Create(_T(""), _T(""), WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                      WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL |
                      ES_AUTOHSCROLL, CRect(10, 10, 100, 200), this, 1);

    m_RichEdit.SetReadOnly();
    m_RichEdit.SetBackgroundColor(FALSE, GetSysColor(COLOR_INFOBK));

    SetLogFont(GetSystemToolTipFont(), FALSE);

    return TRUE;
}

BOOL CToolTipEx::Show(CPoint point)
{
    if(m_pBitmap)
    {
        m_RichEdit.ShowWindow(SW_HIDE);
    }
    else
    {
        m_RichEdit.ShowWindow(SW_SHOW);
    }

    CRect rect = GetBoundsRect();

    //account for the scroll bars
    rect.right += 20;
    rect.bottom += 20;

    //if showing rtf then increase the size because
    //rtf will probably draw bigger
    if(m_csRTF != "")
    {
        long lNewWidth = (long)rect.Width() + (long)(rect.Width() *.3);
        rect.right = rect.left + lNewWidth;

        long lNewHeight = rect.Height() + (rect.Height() *1);
        rect.bottom = rect.top + lNewHeight;
    }

    CRect rcScreen;

    ClientToScreen(rect);

    CRect cr(point, point);

    int nMonitor = GetMonitorFromRect(&cr);
    GetMonitorRect(nMonitor, &rcScreen);

    //ensure that we don't go outside the screen
    if(point.x < 0)
    {
        point.x = 5;
    }
    if(point.y < 0)
    {
        point.y = 5;
    }

    rcScreen.DeflateRect(0, 0, 5, 5);

    long lWidth = rect.Width();
    long lHeight = rect.Height();

    rect.left = point.x;
    rect.top = point.y;
    rect.right = rect.left + lWidth;
    rect.bottom = rect.top + lHeight;

    if(rect.right > rcScreen.right)
    {
        rect.right = rcScreen.right;
    }
    if(rect.bottom > rcScreen.bottom)
    {
        rect.bottom = rcScreen.bottom;
    }

    SetWindowPos(&CWnd::wndTopMost, point.x, point.y, rect.Width(), rect.Height
                 (), SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_NOACTIVATE |
                 SWP_NOZORDER);

    return TRUE;
}

BOOL CToolTipEx::Hide()
{
    DELETE_BITMAP 

    ShowWindow(SW_HIDE);

    m_csRTF = "";
    m_csText = "";

    return TRUE;
}

void CToolTipEx::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect rect;
    GetClientRect(rect);

    //	CBrush  Brush, *pOldBrush;
    //	Brush.CreateSolidBrush(GetSysColor(COLOR_INFOBK));

    //	pOldBrush = dc.SelectObject(&Brush);
    //	CFont *pOldFont = dc.SelectObject(&m_Font);

    //  dc.FillRect(&rect, &Brush);

    // Draw Text
    //    dc.SetBkMode(TRANSPARENT);
    //    rect.DeflateRect(m_rectMargin);

    if(m_pBitmap)
    {
        CDC MemDc;
        MemDc.CreateCompatibleDC(&dc);

        CBitmap *oldBitmap = MemDc.SelectObject(m_pBitmap);

        int nWidth = CBitmapHelper::GetCBitmapWidth(*m_pBitmap);
        int nHeight = CBitmapHelper::GetCBitmapHeight(*m_pBitmap);

        dc.BitBlt(rect.left, rect.top, nWidth, nHeight, &MemDc, 0, 0, SRCCOPY);

		//dc.StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &MemDc, 0, 0, nWidth, nHeight, SRCCOPY);

        MemDc.SelectObject(oldBitmap);

        rect.top += nHeight;
    }

    //dc.DrawText(m_csText, rect, m_dwTextStyle);

    // Cleanup
    //  dc.SelectObject(pOldBrush);
    //	dc.SelectObject(pOldFont);
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
            }
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
                if(!IsCursorInToolTip())
                {
                    Hide();
                }
                break;
            }
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

                Hide();

                break;
            }
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
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
    CWindowDC dc(NULL);

    CFont *pOldFont = (CFont*)dc.SelectObject((CFont*) &m_Font);

    int nLineWidth = 0;

    if(nLineWidth == 0)
    {
        // Count the number of lines of text
        int nStart = 0, nNumLines = 0;
        CString strTextCopy = m_csText;
        do
        {
            nStart = strTextCopy.Find(_T("\n"));

            // skip found character 
            if(nStart >= 0)
            {
                strTextCopy = strTextCopy.Mid(nStart + 1);
            }

            nNumLines++;
        }

        while(nStart >= 0);

        // Find the widest line
        for(int i = 0; i < nNumLines; i++)
        {
            CString strLine = GetFieldFromString(m_csText, i, _T('\n')) + _T(
                "  ");
            nLineWidth = max(nLineWidth, dc.GetTextExtent(strLine).cx);
        }
    }

    CRect rect(0, 0, max(0, nLineWidth), 0);
    dc.DrawText(m_csText, rect, DT_CALCRECT | m_dwTextStyle);

    dc.SelectObject(pOldFont);

    rect.bottom += m_rectMargin.top + m_rectMargin.bottom;
    rect.right += m_rectMargin.left + m_rectMargin.right + 2;

    if(m_pBitmap)
    {
        int nWidth = CBitmapHelper::GetCBitmapWidth(*m_pBitmap);
        int nHeight = CBitmapHelper::GetCBitmapHeight(*m_pBitmap);

        rect.bottom += nHeight;
        if((rect.left + nWidth) > rect.right)
        {
            rect.right = rect.left + nWidth;
        }
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

    m_pBitmap = pBitmap;
}

void CToolTipEx::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    if(::IsWindow(m_RichEdit.GetSafeHwnd()) == FALSE)
    {
        return ;
    }

	m_DittoWindow.DoSetRegion(this);

    CRect cr;
    GetClientRect(cr);
    //	cr.DeflateRect(0, 0, 15, 0);
    m_RichEdit.MoveWindow(cr);

	this->Invalidate();
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
}

void CToolTipEx::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
    CWnd::OnActivate(nState, pWndOther, bMinimized);

    if(nState == WA_INACTIVE)
    {
        Hide();

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
	m_DittoWindow.DoNcLButtonDown(this, nHitTest, point);

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

	CWnd::OnNcLButtonUp(nHitTest, point);
}

void CToolTipEx::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	m_DittoWindow.DoNcMouseMove(this, nHitTest, point);

	CWnd::OnNcMouseMove(nHitTest, point);
}