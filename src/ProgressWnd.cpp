#include "stdafx.h"
#include "ProgressWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_CANCEL   10
#define IDC_TEXT     11
#define IDC_PROGRESS 12

LPCTSTR szSection = _T("Settings");   
LPCTSTR szEntryX  = _T("X");
LPCTSTR szEntryY  = _T("Y");

#define _CRT_NON_CONFORMING_SWPRINTFS 1

/////////////////////////////////////////////////////////////////////////////
// CProgressWnd

CProgressWnd::CProgressWnd()
{
    CommonConstruct();
}

CProgressWnd::CProgressWnd(CWnd* pParent, LPCTSTR pszTitle, BOOL bSmooth /* = FALSE */)
{
    CommonConstruct();
    m_strTitle = pszTitle;

    Create(pParent, pszTitle, bSmooth);
}

void CProgressWnd::CommonConstruct()
{
    m_nNumTextLines  = 4;
    m_nPrevPos       = 0;
    m_nPrevPercent   = 0;
    m_nStep          = 1;
    m_nMinValue      = 0;
    m_nMaxValue      = 100;

    m_strTitle       = _T("Progress");
    m_strCancelLabel = _T(" Cancel ");
    m_bCancelled     = FALSE;
    m_bModal         = FALSE;

    m_bPersistantPosition = TRUE;   // saves and restores position automatically
}

CProgressWnd::~CProgressWnd()
{
    DestroyWindow();
}

BOOL CProgressWnd::Create(CWnd* pParent, LPCTSTR pszTitle, BOOL bSmooth /* = FALSE */)
{
    BOOL bSuccess;

	m_strTitle = pszTitle;
    // Register window class
    CString csClassName = AfxRegisterWndClass(CS_OWNDC|CS_HREDRAW|CS_VREDRAW,
                                              ::LoadCursor(NULL, IDC_APPSTARTING),
                                              CBrush(::GetSysColor(COLOR_BTNFACE)));

    // Get the system window message font for use in the cancel button and text area
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0));
    m_font.CreateFontIndirect(&(ncm.lfMessageFont)); 

    // If no parent supplied then try and get a pointer to it anyway
    if (!pParent)
        pParent = AfxGetMainWnd();

    // Create popup window
    //bSuccess = CreateEx(WS_EX_DLGMODALFRAME|WS_EX_TOPMOST, // Extended style
	bSuccess = CreateEx(WS_EX_DLGMODALFRAME,
                        csClassName,                       // Classname
                        pszTitle,                          // Title
                        WS_POPUP|WS_BORDER|WS_CAPTION,     // style
                        0,0,                               // position - updated soon.
                        390,130,                           // Size - updated soon
                        pParent->GetSafeHwnd(),            // handle to parent
                        0,                                 // No menu
                        NULL);    
    if (!bSuccess) return FALSE;

    // Now create the controls
    CRect TempRect(0,0,10,10);

    bSuccess = m_Text.Create(_T(""), WS_CHILD|WS_VISIBLE|SS_NOPREFIX|SS_LEFTNOWORDWRAP,
                             TempRect, this, IDC_TEXT);
    if (!bSuccess) return FALSE;

    DWORD dwProgressStyle = WS_CHILD|WS_VISIBLE;
#ifdef PBS_SMOOTH    
    if (bSmooth)
       dwProgressStyle |= PBS_SMOOTH;
#endif
    bSuccess = m_wndProgress.Create(dwProgressStyle,TempRect, this, IDC_PROGRESS);
    if (!bSuccess) return FALSE;

    bSuccess = m_CancelButton.Create(m_strCancelLabel, 
                                   WS_CHILD|WS_VISIBLE|WS_TABSTOP| BS_PUSHBUTTON, 
                                     TempRect, this, IDC_CANCEL);
    if (!bSuccess) return FALSE;

    m_CancelButton.SetFont(&m_font, TRUE);
    m_Text.SetFont(&m_font, TRUE);

    // Resize the whole thing according to the number of text lines, desired window
    // width and current font.
    SetWindowSize(m_nNumTextLines, 390);

    // Center and show window
    if (m_bPersistantPosition)
        GetPreviousSettings();
    else
        CenterWindow();

    Show();

    return TRUE;
}

BOOL CProgressWnd::GoModal(LPCTSTR pszTitle /*=_T("Progress")"*/, BOOL bSmooth /*=FALSE*/)
{
    CWnd *pMainWnd = AfxGetMainWnd();

    if (!::IsWindow(m_hWnd) && !Create(pMainWnd, pszTitle, bSmooth))
        return FALSE;

    // Disable main window
    if (pMainWnd)
        pMainWnd->EnableWindow(FALSE);

    // Re-enable this window
    EnableWindow(TRUE);

    m_bModal = TRUE;

    return TRUE;
}
    
void CProgressWnd::SetWindowSize(int nNumTextLines, int nWindowWidth /*=390*/)
{
    int nMargin = 10;
    CSize EdgeSize(::GetSystemMetrics(SM_CXEDGE), ::GetSystemMetrics(SM_CYEDGE));

    CRect TextRect, CancelRect, ProgressRect;
    CSize CancelSize;

    // Set up a default size for the text area in case things go wrong
    TextRect.SetRect(nMargin,nMargin, nWindowWidth-2*nMargin, 100+2*nMargin);

    // Get DrawText to tell us how tall the text area will be (while we're at
    // it, we'll see how big the word "Cancel" is)
    CDC* pDC = GetDC();
    if (pDC) {
        CFont* pOldFont = pDC->SelectObject(&m_font);
        CString str = _T("M");
        for (int i = 0; i < nNumTextLines-1; i++) str += _T("\nM");
        pDC->DrawText(str, TextRect, DT_CALCRECT|DT_NOCLIP|DT_NOPREFIX);
        TextRect.right = TextRect.left + nWindowWidth;
        CancelSize = pDC->GetTextExtent(m_strCancelLabel + _T("  ")) +
                                             CSize(EdgeSize.cx*4, EdgeSize.cy*3);
        pDC->SelectObject(pOldFont);
        ReleaseDC(pDC);
    }
    
    // Work out how big (and where) the cancel button should be
    CancelRect.SetRect(TextRect.right-CancelSize.cx, TextRect.bottom+nMargin, 
                       TextRect.right, TextRect.bottom+nMargin + CancelSize.cy);


    // Work out how big (and where) the progress control should be
    ProgressRect.SetRect(TextRect.left, CancelRect.top + EdgeSize.cy, 
                         CancelRect.left-nMargin, CancelRect.bottom - EdgeSize.cy);


    // Resize the main window to fit the controls
    CSize ClientSize(nMargin + TextRect.Width() + nMargin,
                     nMargin + TextRect.Height() + nMargin + CancelRect.Height() + nMargin);

    CRect WndRect, ClientRect;
    GetWindowRect(WndRect); GetClientRect(ClientRect);
    WndRect.right = WndRect.left + WndRect.Width()-ClientRect.Width()+ClientSize.cx;
    WndRect.bottom = WndRect.top + WndRect.Height()-ClientRect.Height()+ClientSize.cy;
    MoveWindow(WndRect);

    // Now reposition the controls...
    m_wndProgress.MoveWindow(ProgressRect);
    m_CancelButton.MoveWindow(CancelRect);
    m_Text.MoveWindow(TextRect);
}

void CProgressWnd::Clear() 
{ 
    SetText(_T(""));
    SetPos(0);
    m_bCancelled = FALSE; 
    m_nPrevPos = 0;

    if (::IsWindow(GetSafeHwnd()))
        UpdateWindow();
}

void CProgressWnd::Hide()  
{ 
    if (!::IsWindow(GetSafeHwnd())) 
        return;

    if (IsWindowVisible())
    {
        ShowWindow(SW_HIDE);
        ModifyStyle(WS_VISIBLE, 0);
    }
}

void CProgressWnd::Show()  
{ 
    if (!::IsWindow(GetSafeHwnd()))
        return;

    if (!IsWindowVisible())
    {
        ModifyStyle(0, WS_VISIBLE);
        ShowWindow(SW_SHOWNA);
        RedrawWindow(NULL,NULL,RDW_ERASE|RDW_FRAME|RDW_INVALIDATE);
    }
}

void CProgressWnd::SetRange(int nLower, int nUpper, int nStep /* = 1 */)    
{
    if (!::IsWindow(GetSafeHwnd())) 
        return;

 #ifdef PBM_SETRANGE32
    ASSERT(-0x7FFFFFFF <= nLower && nLower <= 0x7FFFFFFF);
    ASSERT(-0x7FFFFFFF <= nUpper && nUpper <= 0x7FFFFFFF);
    m_wndProgress.SendMessage(PBM_SETRANGE32, (WPARAM) nLower, (LPARAM) nUpper);
#else
    ASSERT(0 <= nLower && nLower <= 65535);
    ASSERT(0 <= nUpper && nUpper <= 65535);
    m_wndProgress.SetRange(nLower, nUpper);
#endif

    m_nMaxValue = nUpper;
    m_nMinValue = nLower;
    m_nStep     = nStep;

    m_wndProgress.SetStep(nStep);
}

int CProgressWnd::OffsetPos(int nPos)
{ 
    if (!::IsWindow(GetSafeHwnd())) 
        return m_nPrevPos;

    Show();

    return SetPos(m_nPrevPos + nPos);  
}

int CProgressWnd::StepIt()                
{
    if (!::IsWindow(GetSafeHwnd())) 
        return m_nPrevPos;

    Show();

    return SetPos(m_nPrevPos + m_nStep); 
}

int CProgressWnd::SetStep(int nStep)
{
    int nOldStep = m_nStep;
    m_nStep = nStep;
    if (!::IsWindow(GetSafeHwnd())) 
        return nOldStep;

    return m_wndProgress.SetStep(nStep); 
}

int CProgressWnd::SetPos(int nPos)                    
{
#ifdef PBM_SETRANGE32
    ASSERT(-0x7FFFFFFF <= nPos && nPos <= 0x7FFFFFFF);
#else
    ASSERT(0 <= nPos && nPos <= 65535);
#endif

    if (!::IsWindow(GetSafeHwnd())) 
        return m_nPrevPos;

    Show();

    CString strTitle;
//  int nPercentage;
    
    m_nPrevPos = nPos;

//    if (m_nMaxValue > m_nMinValue)
//        nPercentage = (nPos*100)/(m_nMaxValue - m_nMinValue);
//    else
//        nPercentage = 0;

//    if (nPercentage != m_nPrevPercent) 
//    {
//        m_nPrevPercent = nPercentage;
//        strTitle.Format(_T("%s [%d%%]"),m_strTitle,nPercentage);
		strTitle.Format(_T("%s"),m_strTitle);
        SetWindowText(strTitle);
//    }
    return m_wndProgress.SetPos(nPos);        
}

void CProgressWnd::SetText(LPCTSTR fmt, ...)
{
    if (!::IsWindow(GetSafeHwnd())) 
        return;

    va_list args;
    TCHAR buffer[512];

    va_start(args, fmt);
    _vstprintf(buffer, fmt, args);
    va_end(args);

    m_Text.SetWindowText(buffer);
}

BEGIN_MESSAGE_MAP(CProgressWnd, CWnd)
    //{{AFX_MSG_MAP(CProgressWnd)
    ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_CANCEL, OnCancel)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProgressWnd message handlers

BOOL CProgressWnd::OnEraseBkgnd(CDC* pDC) 
{
    // Fill background with Catchment background colour
    CBrush backBrush(GetSysColor(COLOR_BTNFACE));
    CBrush* pOldBrush = pDC->SelectObject(&backBrush);
    CRect rect;
    pDC->GetClipBox(&rect);     // Erase the area needed
    pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOldBrush);

    return TRUE;
}

void CProgressWnd::OnCancel() 
{
    m_bCancelled = TRUE;
    Hide();

    if (m_bModal)
        SendMessage(WM_CLOSE);

    CWnd *pWnd = AfxGetMainWnd();
    if (pWnd && ::IsWindow(pWnd->m_hWnd))
        pWnd->SetForegroundWindow();
}


BOOL CProgressWnd::DestroyWindow() 
{
    if (m_bPersistantPosition)
        SaveCurrentSettings();

    if (m_bModal)
    {
        m_bModal = FALSE;
        CWnd *pMainWnd = AfxGetMainWnd();

        if (pMainWnd)
            pMainWnd->EnableWindow(TRUE);
    }
	
	return CWnd::DestroyWindow();
}

void CProgressWnd::PeekAndPump(BOOL bCancelOnESCkey /*= TRUE*/)
{
    if (m_bModal && ::GetFocus() != m_hWnd)
        SetFocus();

    MSG msg;
    while (!m_bCancelled && ::PeekMessage(&msg, NULL,0,0,PM_NOREMOVE)) 
    {
        if (bCancelOnESCkey && (msg.message == WM_CHAR) && (msg.wParam == VK_ESCAPE))
            OnCancel();

        // Cancel button disabled if modal, so we fake it.
        if (m_bModal && (msg.message == WM_LBUTTONUP))
        {
            CRect rect;
            m_CancelButton.GetWindowRect(rect);
            if (rect.PtInRect(msg.pt))
                OnCancel();
        }
  
        if (!AfxGetApp()->PumpMessage()) 
        {
            ::PostQuitMessage(0);
            return;
        } 
    }
}

// Retores the previous window size from the registry
void CProgressWnd::GetPreviousSettings()
{
    int x = AfxGetApp()->GetProfileInt(szSection, szEntryX, -1);
    int y = AfxGetApp()->GetProfileInt(szSection, szEntryY, -1);

    if (x >= 0 && x < GetSystemMetrics(SM_CXSCREEN) &&
        y >= 0 && y < GetSystemMetrics(SM_CYSCREEN))
    {
        SetWindowPos(NULL, x,y, 0,0, SWP_NOSIZE|SWP_NOZORDER);
    }
    else
        CenterWindow();
}

// Saves the current window position registry
void CProgressWnd::SaveCurrentSettings()
{   
    if (!IsWindow(m_hWnd))
        return;

    CRect rect;
    GetWindowRect(rect);

    AfxGetApp()->WriteProfileInt(szSection, szEntryX, rect.left);
    AfxGetApp()->WriteProfileInt(szSection, szEntryY, rect.top);
}

void CProgressWnd::HideCancel()
{
	m_CancelButton.ShowWindow(SW_HIDE);
	UpdateData(FALSE);
}

void CProgressWnd::ShowCancel()
{
	m_CancelButton.ShowWindow(SW_SHOW);
	UpdateData(FALSE);
}

void CProgressWnd::SetTitleText(CString csTitle)
{
	m_strTitle = csTitle;
	
	CString title;
	title.Format(_T("%s"),m_strTitle);
	
	SetWindowText(title);
}