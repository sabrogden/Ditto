// GlobalClips.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "GlobalClips.h"
#include "afxdialogex.h"
#include "CopyProperties.h"


// GlobalClips dialog

IMPLEMENT_DYNAMIC(GlobalClips, CDialogEx)

GlobalClips::GlobalClips(CWnd* pParent /*=NULL*/)
	: CDialogEx(GlobalClips::IDD, pParent)
{

}

GlobalClips::~GlobalClips()
{
}

void GlobalClips::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_List);
}


BEGIN_MESSAGE_MAP(GlobalClips, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &GlobalClips::OnBnClickedButtonRefresh)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST2, &GlobalClips::OnNMDblclkList2)
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()

BOOL GlobalClips::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_LIST2, DR_SizeHeight | DR_SizeWidth);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);

	InitListCtrlCols();

	LoadItems();

	return TRUE;
}

void GlobalClips::LoadItems()
{
	m_List.DeleteAllItems();

	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString strItem;
	int count = (int)g_HotKeys.GetCount();

	int row = 0;
	for (int i = 0; i < count; i++)
	{
		CHotKey *pHotKey = g_HotKeys[i];

		if(pHotKey->m_Key <= 0)
		{
			continue;
		}

		// Insert the first item
		lvi.mask =  LVIF_TEXT;
		lvi.iItem = row;

		strItem = pHotKey->m_Name;

		if(pHotKey->m_clipId > 0)
		{
			strItem.Insert(0, _T("(Clip) "));
		}

		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_List.InsertItem(&lvi);

		strItem = pHotKey->GetHotKeyDisplay();
		m_List.SetItemText(row, 1, strItem);

		strItem = _T("Error");
		if(pHotKey->IsRegistered())
		{
			strItem = _T("Yes");
		}

		m_List.SetItemText(row, 2, strItem);

		m_List.SetItemData(row, pHotKey->m_globalId);

		row++;
	}
}

void GlobalClips::InitListCtrlCols()
{
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	
	m_List.InsertColumn(0, theApp.m_Language.GetString("GlobalKeyDesc", "Description"), LVCFMT_LEFT, 200);
	m_List.InsertColumn(1, theApp.m_Language.GetString("GlobalKeyHotKey", "Hot Key"), LVCFMT_LEFT, 100);
	m_List.InsertColumn(2, theApp.m_Language.GetString("GlobalKeyRegistered", "Registered"), LVCFMT_LEFT, 100);
}

void GlobalClips::SetNotifyWnd(HWND hWnd)
{
	m_hWndParent = hWnd;
}

void GlobalClips::OnClose()
{
	DestroyWindow();
}

void GlobalClips::OnCancel()
{
	DestroyWindow();
}

void GlobalClips::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	m_Resize.MoveControls(CSize(cx, cy));
}

void GlobalClips::OnBnClickedButtonRefresh()
{
	LoadItems();
}

void GlobalClips::OnNMDblclkList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here

	int id = (int)m_List.GetItemData(pNMItemActivate->iItem);

	int count = (int)g_HotKeys.GetCount();

	int row = 0;
	for (int i = 0; i < count; i++)
	{
		CHotKey *pHotKey = g_HotKeys[i];

		if(pHotKey->m_globalId == id)
		{
			if(pHotKey->m_clipId > 0)
			{
				CCopyProperties props(pHotKey->m_clipId, this);
				props.SetToTopMost(false);
				INT_PTR doModalRet = props.DoModal();

				if(doModalRet == IDOK)
				{
				}
			}
			else
			{

			}
			break;
		}
	}
	
	*pResult = 0;
}

void GlobalClips::OnNcDestroy()
{
	CDialogEx::OnNcDestroy();
	::PostMessage(m_hWndParent, WM_GLOBAL_CLIPS_CLOSED, 0, 0);
}
