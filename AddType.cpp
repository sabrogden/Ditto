// AddType.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "AddType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddType dialog


CAddType::CAddType(CWnd* pParent /*=NULL*/)
	: CDialog(CAddType::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddType)
	m_eCustomType = _T("");
	//}}AFX_DATA_INIT
}


void CAddType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddType)
	DDX_Control(pDX, IDC_LIST1, m_lbDefaultTypes);
	DDX_Text(pDX, IDC_EDIT1, m_eCustomType);
	DDV_MaxChars(pDX, m_eCustomType, 50);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddType, CDialog)
	//{{AFX_MSG_MAP(CAddType)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO_PRIMARY_TYPES, &CAddType::OnBnClickedRadioPrimaryTypes)
	ON_BN_CLICKED(IDC_RADIO_CURRENT_TYPES, &CAddType::OnBnClickedRadioCurrentTypes)
	ON_BN_CLICKED(IDC_RADIO_CUSTOM_TYPE, &CAddType::OnBnClickedRadioCustomType)
	ON_BN_CLICKED(IDC_ADD_3, &CAddType::OnBnClickedAdd)
	ON_LBN_DBLCLK(IDC_LIST1, &CAddType::OnLbnDblclkList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddType message handlers
BOOL CAddType::OnInitDialog() 
{
	CDialog::OnInitDialog();		
	
	::CheckDlgButton(m_hWnd, IDC_RADIO_CURRENT_TYPES, BST_CHECKED);
	OnBnClickedRadioCurrentTypes();
	
	m_lbDefaultTypes.SetFocus();

	theApp.m_Language.UpdateOptionSupportedTypesAdd(this);
	return FALSE;
}

void CAddType::AddCurrentClipboardTypes()
{
	m_lbDefaultTypes.ResetContent();

	COleDataObject oleData;

	if (!oleData.AttachClipboard())
		return;

	oleData.BeginEnumFormats();

	FORMATETC test;

	while (oleData.GetNextFormat(&test))
	{
		BOOL b = oleData.IsDataAvailable(test.cfFormat);
		m_lbDefaultTypes.AddString(GetFormatName(test.cfFormat));
	}

	oleData.Release();
}

void CAddType::AddCommonTypes()
{
	m_lbDefaultTypes.ResetContent();
	m_lbDefaultTypes.AddString(_T("CF_TEXT"));
	m_lbDefaultTypes.AddString(_T("CF_BITMAP"));
	m_lbDefaultTypes.AddString(_T("CF_METAFILEPICT"));
	m_lbDefaultTypes.AddString(_T("CF_SYLK"));
	m_lbDefaultTypes.AddString(_T("CF_DIF"));
	m_lbDefaultTypes.AddString(_T("CF_TIFF"));
	m_lbDefaultTypes.AddString(_T("CF_OEMTEXT"));
	m_lbDefaultTypes.AddString(_T("CF_DIB"));
	m_lbDefaultTypes.AddString(_T("CF_PALETTE"));
	m_lbDefaultTypes.AddString(_T("CF_PENDATA"));
	m_lbDefaultTypes.AddString(_T("CF_RIFF"));
	m_lbDefaultTypes.AddString(_T("CF_WAVE"));
	m_lbDefaultTypes.AddString(_T("CF_UNICODETEXT"));
	m_lbDefaultTypes.AddString(_T("CF_ENHMETAFILE"));
	m_lbDefaultTypes.AddString(_T("CF_HDROP"));
	m_lbDefaultTypes.AddString(_T("CF_LOCALE"));
	m_lbDefaultTypes.AddString(_T("CF_OWNERDISPLAY"));
	m_lbDefaultTypes.AddString(_T("CF_DSPTEXT"));
	m_lbDefaultTypes.AddString(_T("CF_DSPBITMAP"));
	m_lbDefaultTypes.AddString(_T("CF_DSPMETAFILEPICT"));
	m_lbDefaultTypes.AddString(_T("CF_DSPENHMETAFILE"));
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat(CF_RTF)));
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat(CF_RTFNOOBJS)));
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat(CF_RETEXTOBJ)));
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat(_T("HTML Format"))));
}

void CAddType::OnBnClickedRadioPrimaryTypes()
{
	AddCommonTypes();
	::ShowWindow(::GetDlgItem(m_hWnd, IDC_LIST1), SW_SHOW);
	::ShowWindow(::GetDlgItem(m_hWnd, IDC_EDIT1), SW_HIDE);
	m_lbDefaultTypes.SetFocus();
	if (m_lbDefaultTypes.GetCount() > 0)
	{
		m_lbDefaultTypes.SetCurSel(0);
		m_lbDefaultTypes.SetSel(0);
	}
}

void CAddType::OnBnClickedRadioCurrentTypes()
{
	AddCurrentClipboardTypes();
	::ShowWindow(::GetDlgItem(m_hWnd, IDC_LIST1), SW_SHOW);
	::ShowWindow(::GetDlgItem(m_hWnd, IDC_EDIT1), SW_HIDE);
	m_lbDefaultTypes.SetFocus();
	if (m_lbDefaultTypes.GetCount() > 0)
	{
		m_lbDefaultTypes.SetCurSel(0);
		m_lbDefaultTypes.SetSel(0);

	}
}

void CAddType::OnBnClickedRadioCustomType()
{
	::ShowWindow(::GetDlgItem(m_hWnd, IDC_EDIT1), SW_SHOW);
	::ShowWindow(::GetDlgItem(m_hWnd, IDC_LIST1), SW_HIDE);
	::SetFocus(::GetDlgItem(m_hWnd, IDC_EDIT1));
}

void CAddType::OnBnClickedAdd()
{
	m_csSelectedTypes.RemoveAll();

	if (IsDlgButtonChecked(IDC_RADIO_PRIMARY_TYPES) == BST_CHECKED ||
		IsDlgButtonChecked(IDC_RADIO_CURRENT_TYPES) == BST_CHECKED)
	{
		int nCount = m_lbDefaultTypes.GetSelCount();
		if (nCount)
		{
			CString cs;
			CArray<int, int> items;
			items.SetSize(nCount);
			m_lbDefaultTypes.GetSelItems(nCount, items.GetData());

			for (int i = 0; i < nCount; i++)
			{
				m_lbDefaultTypes.GetText(items[i], cs);
				m_csSelectedTypes.Add(cs);
			}
		}
	}
	else if (IsDlgButtonChecked(IDC_RADIO_CUSTOM_TYPE) == BST_CHECKED)
	{
		UpdateData();
		m_csSelectedTypes.Add(m_eCustomType);
	}

	EndDialog(IDOK);
}


void CAddType::OnLbnDblclkList()
{
	OnBnClickedAdd();
}
