// FriendPromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "FriendPromptDlg.h"
#include "afxdialogex.h"
#include "..\Shared\Tokenizer.h"


// CFriendPromptDlg dialog

IMPLEMENT_DYNAMIC(CFriendPromptDlg, CDialogEx)

CFriendPromptDlg::CFriendPromptDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_FREIND_PROMPT, pParent)
{
	m_save = false;
	m_clearList = false;
}

CFriendPromptDlg::~CFriendPromptDlg()
{
}

void CFriendPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAME, m_textBox);
	DDX_Control(pDX, IDC_EDIT_DESC, m_description);
}


BEGIN_MESSAGE_MAP(CFriendPromptDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFriendPromptDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDCANCEL, &CFriendPromptDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CFriendPromptDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()

void CFriendPromptDlg::OnBnClickedOk()
{
	m_textBox.GetWindowTextW(m_name);
	m_description.GetWindowTextW(m_desc);

	if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_SAVE) == BST_CHECKED)
	{
		m_save = true;
	}

	CDialogEx::OnOK();
}

BOOL CFriendPromptDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	::SendMessage(this->m_hWnd, WM_SETICON, 0, NULL);

	m_brush.CreateSolidBrush(RGB(255, 255, 255)); // color white brush 

	//remove the default icon in top left of window
	int extendedStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, extendedStyle | WS_EX_DLGMODALFRAME);
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE |	SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	::CheckDlgButton(m_hWnd, IDC_CHECK_SAVE, BST_CHECKED);

	m_textBox.SetFocus();
	
	return FALSE;
}

void CFriendPromptDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}

HBRUSH CFriendPromptDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;

	//return m_brush;
}


void CFriendPromptDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CFriendPromptDlg::OnBnClickedButtonClear()
{
	m_clearList = true;
	CDialogEx::OnOK();
}
