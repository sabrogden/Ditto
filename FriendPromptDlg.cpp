// FriendPromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "FriendPromptDlg.h"
#include "afxdialogex.h"
#include "Shared\Tokenizer.h"


// CFriendPromptDlg dialog

IMPLEMENT_DYNAMIC(CFriendPromptDlg, CDialogEx)

CFriendPromptDlg::CFriendPromptDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_FREIND_PROMPT, pParent)
{
	m_clearList = _T("Clear List");
}

CFriendPromptDlg::~CFriendPromptDlg()
{
}

void CFriendPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_nameComboBox);
}


BEGIN_MESSAGE_MAP(CFriendPromptDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFriendPromptDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CFriendPromptDlg::OnCbnSelchangeCombo1)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CFriendPromptDlg message handlers


void CFriendPromptDlg::OnBnClickedOk()
{
	m_nameComboBox.GetWindowTextW(m_name);

	CString values = m_name;

	int count = m_nameComboBox.GetCount();
	for (int i = 0; i < count; i++)
	{
		CString lineValue;
		m_nameComboBox.GetLBText(i, lineValue);

		if ((lineValue != m_name) &&
			(lineValue != m_clearList))
		{
			values += _T(",");
			values += lineValue;			
		}
	}

	CGetSetOptions::SetCustomSendToList(values);

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
	

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_COMBO1, DR_SizeWidth);
	m_Resize.AddControl(IDOK, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);

	CString oldValues = CGetSetOptions::GetCustomSendToList();
	CTokenizer token(oldValues, _T(","));
	CString line;

	bool setSelected = false;

	while (token.Next(line))
	{
		if (line != "")
		{
			int row = m_nameComboBox.AddString(line);
			if (setSelected == false)
			{
				m_nameComboBox.SetCurSel(row);
				m_nameComboBox.SetEditSel(0, line.GetLength());
				setSelected = true;
			}
		}
	}

	m_nameComboBox.AddString(m_clearList);

	m_nameComboBox.SetFocus();


	return FALSE;
}

void CFriendPromptDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	m_Resize.MoveControls(CSize(cx, cy));
}


void CFriendPromptDlg::OnCbnSelchangeCombo1()
{
	CString selection;
	int sel = m_nameComboBox.GetCurSel();
	m_nameComboBox.GetLBText(sel, selection);
		
	if (selection == m_clearList)
	{
		m_nameComboBox.ResetContent();
		m_nameComboBox.AddString(m_clearList);
	}
}


HBRUSH CFriendPromptDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;

	//return m_brush;
}
