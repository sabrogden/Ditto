// OptionsTypes.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsTypes.h"
#include "Shared/ArrayEx.h"
#include "DimWnd.h"
#include "Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsTypes property page

IMPLEMENT_DYNCREATE(COptionsTypes, CPropertyPage)

COptionsTypes::COptionsTypes() : CPropertyPage(COptionsTypes::IDD)
{
	m_csTitle = theApp.m_Language.GetString("SupportedTypesTitle", "Supported Types");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE; 

	//{{AFX_DATA_INIT(COptionsTypes)
	//}}AFX_DATA_INIT

	m_bSave = false;
}

COptionsTypes::~COptionsTypes()
{
}

void COptionsTypes::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsTypes)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsTypes, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsTypes)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsTypes message handlers

BOOL COptionsTypes::OnApply() 
{
	if(m_bSave)
	{
		try
		{
			theApp.m_db.execDML(_T("DELETE FROM Types;"));

			CString csText;
			int nCount = m_List.GetCount();
			for(int i = 0; i < nCount; i++)
			{
				m_List.GetText(i, csText);

				theApp.m_db.execDMLEx(_T("INSERT INTO Types VALUES(NULL, '%s');"), csText);
			}
		}
		CATCH_SQLITE_EXCEPTION

		// refresh our local cache
		theApp.ReloadTypes();
	}
	
	return CPropertyPage::OnApply();
}

BOOL COptionsTypes::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	try
	{
		CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT TypeText FROM Types"));
		if(q.eof())
		{
			m_List.AddString(_T("CF_TEXT"));
			m_List.AddString(GetFormatName(RegisterClipboardFormat(CF_RTF)));
			m_List.AddString(_T("CF_UNICODETEXT"));
			m_List.AddString(_T("CF_HDROP"));
			m_List.AddString(_T("CF_DIB"));
			m_List.AddString(GetFormatName(GetFormatID(_T("HTML Format"))));
			m_List.AddString(GetFormatName(GetFormatID(_T("PNG"))));
		}

		while(q.eof() == false)
		{
			m_List.AddString(q.getStringField(0));

			q.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION
	
	m_List.SetFocus();

	theApp.m_Language.UpdateOptionSupportedTypes(this);
	
	return FALSE;
}

void COptionsTypes::OnDelete() 
{
	int nCount = m_List.GetSelCount();
	if(nCount)
	{
		m_bSave = true;
		CArrayEx<int> items;
		items.SetSize(nCount);
		m_List.GetSelItems(nCount, items.GetData()); 
		items.SortDescending();

		for(int i = 0; i < nCount; i++)
			m_List.DeleteString(items[i]);
	}
}

#include "AddType.h"
void COptionsTypes::OnAdd() 
{
	CDimWnd dim(this->GetParent());
	CAddType add(this);

	if(add.DoModal() == IDOK)
	{
		INT_PTR nCount = add.m_csSelectedTypes.GetSize();
		if(nCount)
		{
			m_bSave = true;
			for(int i = 0; i < nCount; i++)
			{
				if(TextAllReadyThere(add.m_csSelectedTypes[i]) == FALSE)
					m_List.AddString(add.m_csSelectedTypes[i]);
			}
		}
	}	
}

BOOL COptionsTypes::TextAllReadyThere(const CString &cs)
{
	CString csThere;
	int nCount = m_List.GetCount();

	for(int i = 0; i < nCount; i++)
	{
		m_List.GetText(i, csThere);
		if(cs == csThere)
			return TRUE;
	}

	return FALSE;
}
