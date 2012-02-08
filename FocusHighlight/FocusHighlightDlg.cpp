
// FocusHighlightDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FocusHighlight.h"
#include "FocusHighlightDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CFocusHighlightDlg::CFocusHighlightDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFocusHighlightDlg::IDD, pParent)
{
}

void CFocusHighlightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ACTIVE, m_activeEdit);
	DDX_Control(pDX, IDC_EDIT_FOCUS, m_focusEdit);
	DDX_Control(pDX, IDC_CHECK_TRACK_FOCUS_CHANGES, m_trackFocusChangesCheck);
}

BEGIN_MESSAGE_MAP(CFocusHighlightDlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_HIGHLIGHT_ACTIVE, &CFocusHighlightDlg::OnBnClickedButtonHighlightActive)
	ON_BN_CLICKED(IDC_BUTTON_HIGHLIGHT_FOCUS, &CFocusHighlightDlg::OnBnClickedButtonHighlightFocus)
END_MESSAGE_MAP()

BOOL CFocusHighlightDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	StayOnTop();

	::CheckDlgButton(m_hWnd, IDC_CHECK_TRACK_FOCUS_CHANGES, BST_CHECKED);
	SetTimer(1, 2000, NULL);	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFocusHighlightDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case 1:

		if(::IsDlgButtonChecked(m_hWnd, IDC_CHECK_TRACK_FOCUS_CHANGES) != 0)
		{
			m_tracker.TrackActiveWnd();

			CString cs;
			cs.Format(_T("0x%08x - %s"), m_tracker.ActiveWnd(), m_tracker.ActiveWndName());
			m_activeEdit.SetWindowText(cs);

			cs.Format(_T("0x%08x - %s"), m_tracker.FocusWnd(), m_tracker.FocusWndName());
			m_focusEdit.SetWindowText(cs);
		}

		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CFocusHighlightDlg::StayOnTop()
{	
	CRect rect;

	// get the current window size and position
	GetWindowRect( rect );

	// now change the size, position, and Z order 
	// of the window.
	::SetWindowPos(m_hWnd ,       // handle to window
		HWND_TOPMOST,  // placement-order handle
		rect.left,     // horizontal position
		rect.top,      // vertical position
		rect.Width(),  // width
		rect.Height(), // height
		SWP_SHOWWINDOW); // window-positioning options);
}

void CFocusHighlightDlg::OnBnClickedButtonHighlightActive()
{
	HWND activeWnd = m_tracker.ActiveWnd();

	CRect cr;
	::GetWindowRect(activeWnd, &cr);
	::ScreenToClient(activeWnd, &cr.TopLeft());
	::ScreenToClient(activeWnd, &cr.BottomRight());

	HDC hdc = ::GetWindowDC(activeWnd);

	for(int i = 0; i < 10; i++)
	{
		::DrawFocusRect(hdc, cr);
		
		Sleep(200);
	}

	::ReleaseDC(activeWnd, hdc);
}


void CFocusHighlightDlg::OnBnClickedButtonHighlightFocus()
{
	HWND focusWnd = m_tracker.FocusWnd();

	CRect cr;
	::GetWindowRect(focusWnd, &cr);
	::ScreenToClient(focusWnd, &cr.TopLeft());
	::ScreenToClient(focusWnd, &cr.BottomRight());

	HDC hdc = ::GetWindowDC(focusWnd);

	for(int i = 0; i < 10; i++)
	{
		::DrawFocusRect(hdc, cr);
		
		Sleep(200);
	}

	::ReleaseDC(focusWnd, hdc);

}
