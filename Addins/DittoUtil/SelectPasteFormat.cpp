// SelectPasteFormat.cpp : implementation file
//

#include "stdafx.h"
#include "DittoUtil.h"
#include "SelectPasteFormat.h"
#include ".\selectpasteformat.h"


// CSelectPasteFormat dialog

IMPLEMENT_DYNAMIC(CSelectPasteFormat, CDialog)
CSelectPasteFormat::CSelectPasteFormat(CWnd* pParent, IClipFormats *clipFormats)
	: CDialog(CSelectPasteFormat::IDD, pParent)
{
	m_pClipFormats = clipFormats;
	m_selectedFormat = 0;
}

CSelectPasteFormat::~CSelectPasteFormat()
{
}

void CSelectPasteFormat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_Formats);
}


BEGIN_MESSAGE_MAP(CSelectPasteFormat, CDialog)
	ON_LBN_DBLCLK(IDC_LIST1, OnLbnDblclkList1)
	ON_WM_SIZE()
END_MESSAGE_MAP()


BOOL CSelectPasteFormat::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if(m_pClipFormats != NULL)
	{
		int count = m_pClipFormats->Size();
		for(int i = 0; i < count; i++)
		{
			CString formatName = DittoAddinHelpers::GetFormatName(m_pClipFormats->GetAt(i)->Type());
			int pos = m_Formats.AddString(formatName);
			m_Formats.SetItemData(pos, m_pClipFormats->GetAt(i)->Type());
		}
	}

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_LIST1, DR_SizeHeight | DR_SizeWidth);
	m_Resize.AddControl(IDOK, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);

	return TRUE;
}

void CSelectPasteFormat::OnOK()
{
	int pos = m_Formats.GetCurSel();
	if(pos >= 0 && pos < m_Formats.GetCount())
	{
		m_selectedFormat = (CLIPFORMAT)m_Formats.GetItemData(pos);
	}

	CDialog::OnOK();
}

void CSelectPasteFormat::OnLbnDblclkList1()
{
	OnOK();
}

void CSelectPasteFormat::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	m_Resize.MoveControls(CSize(cx, cy));
}