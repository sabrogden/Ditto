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
	DDX_Control(pDX, IDC_LIST2, m_lbOnClipboard);
	DDX_Control(pDX, IDC_LIST1, m_lbDefaultTypes);
	DDX_Text(pDX, IDC_EDIT1, m_eCustomType);
	DDV_MaxChars(pDX, m_eCustomType, 50);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddType, CDialog)
	//{{AFX_MSG_MAP(CAddType)
	ON_BN_CLICKED(IDC_ADD_1, OnAdd1)
	ON_BN_CLICKED(IDC_ADD_2, OnAdd2)
	ON_BN_CLICKED(IDC_ADD_3, OnAdd3)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	ON_LBN_DBLCLK(IDC_LIST2, OnDblclkList2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddType message handlers
BOOL CAddType::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_lbDefaultTypes.AddString("CF_TEXT");
	m_lbDefaultTypes.AddString("CF_BITMAP");
	m_lbDefaultTypes.AddString("CF_METAFILEPICT");
	m_lbDefaultTypes.AddString("CF_SYLK");
	m_lbDefaultTypes.AddString("CF_DIF");
	m_lbDefaultTypes.AddString("CF_TIFF");
	m_lbDefaultTypes.AddString("CF_OEMTEXT");
	m_lbDefaultTypes.AddString("CF_DIB");
	m_lbDefaultTypes.AddString("CF_PALETTE");
	m_lbDefaultTypes.AddString("CF_PENDATA");
	m_lbDefaultTypes.AddString("CF_RIFF");
	m_lbDefaultTypes.AddString("CF_WAVE");
	m_lbDefaultTypes.AddString("CF_UNICODETEXT");
	m_lbDefaultTypes.AddString("CF_ENHMETAFILE");
	m_lbDefaultTypes.AddString("CF_HDROP");
	m_lbDefaultTypes.AddString("CF_LOCALE");
	m_lbDefaultTypes.AddString("CF_OWNERDISPLAY");
	m_lbDefaultTypes.AddString("CF_DSPTEXT");
	m_lbDefaultTypes.AddString("CF_DSPBITMAP");
	m_lbDefaultTypes.AddString("CF_DSPMETAFILEPICT");
	m_lbDefaultTypes.AddString("CF_DSPENHMETAFILE");
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat(CF_RTF)));
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat(CF_RTFNOOBJS)));
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat(CF_RETEXTOBJ)));
	m_lbDefaultTypes.AddString(GetFormatName(RegisterClipboardFormat("HTML Format")));
		
	COleDataObject oleData;

	if(!oleData.AttachClipboard())
		return FALSE;
	
	oleData.BeginEnumFormats();

	FORMATETC test;
	
	while(oleData.GetNextFormat(&test))
	{
		BOOL b = oleData.IsDataAvailable(test.cfFormat);
		m_lbOnClipboard.AddString(GetFormatName(test.cfFormat));
	}
	
	oleData.Release();

	m_lbDefaultTypes.SetFocus();
	return FALSE;
}

void CAddType::OnAdd1() 
{
	m_csSelectedTypes.RemoveAll();

	int nCount = m_lbDefaultTypes.GetSelCount();
	if(nCount)
	{
		CString cs;
		CArray<int,int> items;
		items.SetSize(nCount);
		m_lbDefaultTypes.GetSelItems(nCount, items.GetData()); 

		for(int i = 0; i < nCount; i++)
		{
			m_lbDefaultTypes.GetText(items[i], cs);
			m_csSelectedTypes.Add(cs);
		}
	}
	
	EndDialog(IDOK);
}

void CAddType::OnAdd2() 
{
	UpdateData();
	m_csSelectedTypes.RemoveAll();
	m_csSelectedTypes.Add(m_eCustomType);
		
	EndDialog(IDOK);
}

void CAddType::OnAdd3() 
{
	m_csSelectedTypes.RemoveAll();

	int nCount = m_lbOnClipboard.GetSelCount();
	if(nCount)
	{
		CString cs;
		CArray<int,int> items;
		items.SetSize(nCount);
		m_lbOnClipboard.GetSelItems(nCount, items.GetData()); 

		for(int i = 0; i < nCount; i++)
		{
			m_lbOnClipboard.GetText(items[i], cs);
			m_csSelectedTypes.Add(cs);
		}
	}

	EndDialog(IDOK);
}


void CAddType::OnDblclkList1() 
{
	OnAdd1();
}

void CAddType::OnDblclkList2() 
{
	OnAdd3();
}
