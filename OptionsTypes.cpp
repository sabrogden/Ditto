// OptionsTypes.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsTypes.h"
#include "ArrayEx.h"

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
			CTypesTable recset;
			recset.DeleteAll();

			recset.Open(AFX_DAO_USE_DEFAULT_TYPE, "SELECT * FROM Types" ,NULL);

			int nCount = m_List.GetCount();

			for(int i = 0; i < nCount; i++)
			{
				recset.AddNew();
				m_List.GetText(i, recset.m_TypeText);
				recset.Update();
			}
			recset.Close();
		}
		CATCHDAO

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
		CTypesTable recset;
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, "SELECT * FROM Types" ,NULL);
		if(recset.IsEOF())
		{
			m_List.AddString("CF_TEXT");
			m_List.AddString(GetFormatName(RegisterClipboardFormat(CF_RTF)));
			m_List.AddString("CF_DIB");
		}
		while(!recset.IsEOF())
		{
			m_List.AddString(recset.m_TypeText);
			recset.MoveNext();
		}
	}
	CATCHDAO
	
	m_List.SetFocus();
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
	CAddType add(this);

	if(add.DoModal() == IDOK)
	{
		int nCount = add.m_csSelectedTypes.GetSize();
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
