// ToolTipEx.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "ToolTipEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DELETE_BITMAP	if(m_pBitmap)					\
						{								\
							m_pBitmap->DeleteObject();	\
							DELETE_PTR(m_pBitmap);		\
						}


/////////////////////////////////////////////////////////////////////////////
// CToolTipEx

CToolTipEx::CToolTipEx() :
	m_dwTextStyle(DT_EXPANDTABS|DT_EXTERNALLEADING|DT_NOPREFIX|DT_WORDBREAK),
	m_rectMargin(2, 2, 3, 3),
	m_pBitmap(NULL)
{
}

CToolTipEx::~CToolTipEx()
{
	DELETE_BITMAP
	
	m_Font.DeleteObject();
}


BEGIN_MESSAGE_MAP(CToolTipEx, CWnd)
	//{{AFX_MSG_MAP(CToolTipEx)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CToolTipEx message handlers

BOOL CToolTipEx::Create(CWnd* pParentWnd)
{    
    // Get the class name and create the window
    CString szClassName = AfxRegisterWndClass(CS_CLASSDC|CS_SAVEBITS, 
                                              LoadCursor(NULL, IDC_ARROW));

    // Create the window - just don't show it yet.
    if (!CWnd::CreateEx(WS_EX_TOPMOST, szClassName, _T(""), 
                        WS_POPUP|WS_BORDER, 
                        0, 0, 10, 10, // size & position updated when needed
                        pParentWnd->GetSafeHwnd(), 0, NULL))
	{
        return FALSE;
	}

	SetLogFont(GetSystemToolTipFont(), FALSE);
   
    return TRUE;
}

BOOL CToolTipEx::Show(CPoint point)
{
	CRect rect = GetBoundsRect();

	CRect rcScreen;
	
	CRect crRectToScreen(point, CPoint(point.x + rect.right, point.y + rect.bottom));

	int nMonitor = GetMonitorFromRect(&crRectToScreen);
	GetMonitorRect(nMonitor, &rcScreen);
		
	if(crRectToScreen.right > rcScreen.right)
	{
		point.x -= (crRectToScreen.right - rcScreen.right);
		///Add a border
		point.x -= 2;
	}

	if(crRectToScreen.bottom > rcScreen.bottom)
	{
		point.y -= (crRectToScreen.bottom - rcScreen.bottom);
		//add a border
		point.y -= 2;
	}


	ShowWindow(SW_HIDE);
    SetWindowPos(NULL,
                 point.x, point.y,
                 rect.Width(), rect.Height(),
                 SWP_SHOWWINDOW|SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_NOZORDER);

	return TRUE;
}

BOOL CToolTipEx::Hide()
{
	DELETE_BITMAP

	ShowWindow(SW_HIDE);

	return TRUE;
}

void CToolTipEx::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
    GetClientRect(rect);

	CBrush  Brush, *pOldBrush;
	Brush.CreateSolidBrush(GetSysColor(COLOR_INFOBK));

	pOldBrush = dc.SelectObject(&Brush);
	CFont *pOldFont = dc.SelectObject(&m_Font);

    dc.FillRect(&rect, &Brush);

	// Draw Text
    dc.SetBkMode(TRANSPARENT);
    rect.DeflateRect(m_rectMargin);

	if(m_pBitmap)
	{
		CDC MemDc;
		MemDc.CreateCompatibleDC(&dc);

		CBitmap* oldBitmap = MemDc.SelectObject(m_pBitmap);

		int nWidth = GetCBitmapWidth(*m_pBitmap);
		int nHeight = GetCBitmapHeight(*m_pBitmap);
		
		dc.BitBlt(rect.left, rect.top, nWidth, nHeight, &MemDc, 0, 0, SRCCOPY);

		MemDc.SelectObject(oldBitmap);

		rect.top += nHeight;
	}

    dc.DrawText(m_csText, rect, m_dwTextStyle);

	// Cleanup
    dc.SelectObject(pOldBrush);
	dc.SelectObject(pOldFont);
}

void CToolTipEx::PostNcDestroy() 
{
	CWnd::PostNcDestroy();

    delete this;
}

BOOL CToolTipEx::OnMsg(MSG* pMsg)
{
	switch(pMsg->message) 
	{
		case WM_WINDOWPOSCHANGING:
		case WM_LBUTTONDOWN:
		{
			//if (!IsCursorInToolTip())
			Hide();
			break;
		}
		case WM_KEYDOWN:
		{
			if(IsWindowVisible())
			{
				Hide();
				WPARAM vk = pMsg->wParam;
				if(vk == VK_ESCAPE)
				{
					return TRUE;
				}
			}
			else
				Hide();
			break;
		}
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN :
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
   
	CFont *pOldFont = (CFont*) dc.SelectObject((CFont*)&m_Font); 
	
	int nLineWidth = 0;

	if (nLineWidth == 0)
    {
        // Count the number of lines of text
        int nStart = 0, nNumLines = 0;
		CString strTextCopy = m_csText;
        do 
		{
            nStart = strTextCopy.Find(_T("\n"));

            // skip found character 
            if (nStart >= 0)
                strTextCopy = strTextCopy.Mid(nStart+1);

            nNumLines++;
        } while (nStart >= 0);

        // Find the widest line
        for (int i = 0; i < nNumLines; i++)
        {
            CString strLine = GetFieldFromString(m_csText, i, _T('\n')) + _T("  ");
            nLineWidth = max(nLineWidth, dc.GetTextExtent(strLine).cx);
        }
    }

    CRect rect(0, 0, max(0,nLineWidth), 0);
    dc.DrawText(m_csText, rect, DT_CALCRECT | m_dwTextStyle);

	dc.SelectObject(pOldFont);
	
    rect.bottom += m_rectMargin.top + m_rectMargin.bottom;
    rect.right += m_rectMargin.left + m_rectMargin.right + 2;

	if(m_pBitmap)
	{
		int nWidth = GetCBitmapWidth(*m_pBitmap);
		int nHeight = GetCBitmapHeight(*m_pBitmap);

		rect.bottom += nHeight;
		if((rect.left + nWidth) > rect.right)
			rect.right = rect.left + nWidth;
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

    while (nCurrent <= nIndex && *pstrBuffer != _T('\0'))
    {
        if (*pstrBuffer == ch)
        {
            nOldStart = nStart;
            nStart = nEnd+1;
            nCurrent++;
        }
        nEnd++;
        pstrBuffer++;
    }

    // May have reached the end of the string
    if (*pstrBuffer == _T('\0'))
    {
        nOldStart = nStart;
        nEnd++;
    }

    ref.UnlockBuffer();

    if (nCurrent < nIndex) 
    {
        //TRACE1("Warning: GetStringField - Couldn't find field %d.\n", nIndex);
        return strReturn;
    }
    return ref.Mid(nOldStart, nEnd-nOldStart-1);
}

LPLOGFONT CToolTipEx::GetSystemToolTipFont()
{
    static LOGFONT LogFont;

    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
        return FALSE;

    memcpy(&LogFont, &(ncm.lfStatusFont), sizeof(LOGFONT));

    return &LogFont; 
}

BOOL CToolTipEx::SetLogFont(LPLOGFONT lpLogFont, BOOL bRedraw /*=TRUE*/)
{
    ASSERT(lpLogFont);
    if (!lpLogFont)
        return FALSE;

	LOGFONT LogFont;

    // Store font as the global default
    memcpy(&LogFont, lpLogFont, sizeof(LOGFONT));

    // Create the actual font object
    m_Font.DeleteObject();
    m_Font.CreateFontIndirect(&LogFont);

    if (bRedraw && ::IsWindow(GetSafeHwnd())) 
        Invalidate();

    return TRUE;
}

void CToolTipEx::SetBitmap(CBitmap *pBitmap)
{
	DELETE_BITMAP

	m_pBitmap = pBitmap;
}