#include "stdafx.h"
#include "CP_Main.h"
#include "DittoPopupWindow.h"


CDittoPopupWindow::CDittoPopupWindow()
{
}


CDittoPopupWindow::~CDittoPopupWindow()
{
}

BEGIN_MESSAGE_MAP(CDittoPopupWindow, CWndEx)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

int CDittoPopupWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWndEx::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	CWndEx::SetCaptionOn(CAPTION_TOP);

	m_font.CreateFont(-theApp.m_metrics.PointsToPixels(12), 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, 3, 2, 1, 34, _T("MS Sans Serif"));
	m_textLabel.Create(_T("test"), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, CRect(0, 0, 0, 0), this);
	m_textLabel.SetFont(&m_font);

	SetWindowText(_T("Running Ditto Updates"));

	//m_textLabel.SetWindowText(_T("test"));

	return 0;
}

void CDittoPopupWindow::UpdateText(CString text)
{
	m_textLabel.SetWindowText(text);

	CRect rect;
	m_textLabel.GetWindowRect(&rect);
	ScreenToClient(&rect);
	InvalidateRect(&rect);
	UpdateWindow();

	PumpMessages();
}

void CDittoPopupWindow::OnSize(UINT nType, int cx, int cy)
{
	CWndEx::OnSize(nType, cx, cy);

	if(m_textLabel.m_hWnd != NULL)
	{
		m_textLabel.MoveWindow(0, 0, cx, cy);
	}
	
}

HBRUSH CDittoPopupWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == 0xffff)
	{
		pDC->SetBkMode(TRANSPARENT);
		return reinterpret_cast<HBRUSH>(::GetStockObject(NULL_BRUSH));
	}
	else
		return CWndEx::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CDittoPopupWindow::PumpMessages()
{
	int nLoops = 0;
	MSG msg;
	while (::PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		nLoops++;
		if (nLoops > 100)
			break;
	}
}