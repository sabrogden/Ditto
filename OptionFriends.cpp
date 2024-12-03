// OptionFriends.cpp : implementation file
//

#include "stdafx.h"
#include "cp_main.h"
#include "OptionFriends.h"
#include "FriendDetails.h"
#include "DimWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EMPTRY_STRING "----"

/////////////////////////////////////////////////////////////////////////////
// COptionFriends property page

IMPLEMENT_DYNCREATE(COptionFriends, CPropertyPage)

COptionFriends::COptionFriends() : CPropertyPage(COptionFriends::IDD)
{
	m_csTitle = theApp.m_Language.GetString("FriendsTitle", "Friends");
	m_psp.pszTitle = m_csTitle;
	m_psp.dwFlags |= PSP_USETITLE; 

	//{{AFX_DATA_INIT(COptionFriends)
	m_PlaceOnClipboard = _T("");
	m_csPassword = _T("");
	m_csAdditionalPasswords = _T("");
	//}}AFX_DATA_INIT
}

COptionFriends::~COptionFriends()
{
}

void COptionFriends::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionFriends)
	DDX_Control(pDX, IDC_CHECK_DISABLE_FRIENDS, m_bDisableRecieve);
	DDX_Control(pDX, IDC_CHECK_LOG_SEND_RECIEVE, m_SendRecieve);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Text(pDX, IDC_EDIT_PLACE_ON_CLIPBOARD, m_PlaceOnClipboard);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_csPassword);
	DDX_Text(pDX, IDC_EDIT_ADDITIONAL, m_csAdditionalPasswords);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionFriends, CPropertyPage)
	//{{AFX_MSG_MAP(COptionFriends)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, OnKeydownList)
	ON_BN_CLICKED(IDC_CHECK_DISABLE_FRIENDS, OnCheckDisableFriends)
	//}}AFX_MSG_MAP 
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionFriends message handlers


BOOL COptionFriends::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_pParent = (COptionsSheet *)GetParent();
	
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	InitListCtrlCols();

	InsertItems();

	m_SendRecieve.SetCheck(CGetSetOptions::GetLogSendReceiveErrors());
	m_bDisableRecieve.SetCheck(CGetSetOptions::GetDisableRecieve());

	m_PlaceOnClipboard = CGetSetOptions::m_csIPListToPutOnClipboard;
	m_csPassword = CGetSetOptions::m_csPassword;
	m_csAdditionalPasswords = CGetSetOptions::GetExtraNetworkPassword(false);

	if(CGetSetOptions::GetRequestFilesUsingIP())
	{
		::CheckDlgButton(m_hWnd, IDC_RADIO_USE_IP, BST_CHECKED);
	}
	else
	{
		::CheckDlgButton(m_hWnd, IDC_RADIO_USE_HOST_NAME, BST_CHECKED);
	}

	UpdateData(FALSE);

	theApp.m_Language.UpdateOptionFriends(this);
		
	return FALSE;
}

BOOL COptionFriends::OnApply() 
{
	CSendClients client;
	for (int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		if(m_List.GetItemText(i, 1) == "X")
			client.bSendAll = TRUE;
		else
			client.bSendAll = FALSE;

		client.csIP = m_List.GetItemText(i, 2);
		if(client.csIP == EMPTRY_STRING)
			client.csIP = "";
		
		client.csDescription = m_List.GetItemText(i, 3);
		if(client.csDescription == EMPTRY_STRING)
			client.csDescription = "";

		CGetSetOptions::SetSendClients(client, i);
	}

	CGetSetOptions::SetNetworkPassword(m_csPassword);
	CGetSetOptions::SetLogSendReceiveErrors(m_SendRecieve.GetCheck());

	CGetSetOptions::SetDisableRecieve(m_bDisableRecieve.GetCheck());
	theApp.StartStopServerThread();

	UpdateData();

	CGetSetOptions::SetListToPutOnClipboard(m_PlaceOnClipboard);
	CGetSetOptions::SetNetworkPassword(m_csPassword);
	CGetSetOptions::GetClientSendCount();

	CGetSetOptions::SetExtraNetworkPassword(m_csAdditionalPasswords);
	//get get to refill the array extra passwords
	CGetSetOptions::GetExtraNetworkPassword(true);

	if(::IsDlgButtonChecked(m_hWnd, IDC_RADIO_USE_IP) == BST_CHECKED)
	{
		CGetSetOptions::SetRequestFilesUsingIP(TRUE);
	}
	else if(::IsDlgButtonChecked(m_hWnd, IDC_RADIO_USE_HOST_NAME) == BST_CHECKED)
	{
		CGetSetOptions::SetRequestFilesUsingIP(FALSE);
	}
	
	return CPropertyPage::OnApply();
}


void COptionFriends::InitListCtrlCols()
{
	// Insert some columns
	m_List.InsertColumn(0, _T(""), LVCFMT_LEFT, 25);

	m_List.InsertColumn(1, theApp.m_Language.GetString("Send_All_Copies", "Send All Copies"), LVCFMT_LEFT, 90);
	m_List.InsertColumn(2, theApp.m_Language.GetString("IP_Name", "IP/Name"), LVCFMT_LEFT, 135);
	m_List.InsertColumn(3, theApp.m_Language.GetString("Descriptions", "Descriptions"), LVCFMT_LEFT, 157);
}

void COptionFriends::InsertItems()
{
	// Delete the current contents
	m_List.DeleteAllItems();

	// Use the LV_ITEM structure to insert the items
	LVITEM lvi;
	CString strItem;
	for (int i = 0; i < MAX_SEND_CLIENTS; i++)
	{
		// Insert the first item
		lvi.mask =  LVIF_TEXT;
	
		lvi.iItem = i;

//-------------------------------------------------------------------

		strItem.Format(_T("%d"), i+1);

		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(strItem);
		m_List.InsertItem(&lvi);

//-------------------------------------------------------------------
		if(CGetSetOptions::m_SendClients[i].bSendAll)
			strItem = "X";
		else
			strItem = EMPTRY_STRING;

		m_List.SetItemText(i, 1, strItem);

//-------------------------------------------------------------------

		strItem = CGetSetOptions::m_SendClients[i].csIP;
		if(CGetSetOptions::m_SendClients[i].csIP.GetLength() <= 0)
		{
			strItem = EMPTRY_STRING;
		}

		m_List.SetItemText(i, 2, strItem);

//-------------------------------------------------------------------

		strItem = CGetSetOptions::m_SendClients[i].csDescription;
		if(CGetSetOptions::m_SendClients[i].csDescription.GetLength() <= 0)
		{
			strItem = EMPTRY_STRING;
		}
		
		m_List.SetItemText(i, 3, strItem);
	}
}

void COptionFriends::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	if(pos)
	{
		int nItem = m_List.GetNextSelectedItem(pos);

		EditItem(nItem);
	}
	
	*pResult = 0;
}

BOOL COptionFriends::EditItem(int nItem)
{
	CDimWnd dim(this->GetParent());

	CFriendDetails dlg(this);

	if(m_List.GetItemText(nItem, 1) == "X")
		dlg.m_checkSendAll = TRUE;
	else
		dlg.m_checkSendAll = FALSE;

	dlg.m_csIP = m_List.GetItemText(nItem, 2);
	if(dlg.m_csIP == EMPTRY_STRING)
		dlg.m_csIP = "";
	dlg.m_csDescription = m_List.GetItemText(nItem, 3);
	if(dlg.m_csDescription == EMPTRY_STRING)
		dlg.m_csDescription = "";

	if(dlg.DoModal() == IDOK)
	{
		if(dlg.m_checkSendAll)
		{
			m_List.SetItemText(nItem, 1, _T("X"));
		}
		else
			m_List.SetItemText(nItem, 1, _T(""));

		m_List.SetItemText(nItem, 2, dlg.m_csIP);
		m_List.SetItemText(nItem, 3, dlg.m_csDescription);

		return TRUE;
	}

	return FALSE;
}

void COptionFriends::OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

	switch (pLVKeyDow->wVKey)
	{
	case VK_DELETE:
		{
			POSITION pos = m_List.GetFirstSelectedItemPosition();
			if(pos)
			{
				int nItem = m_List.GetNextSelectedItem(pos);
				m_List.SetItemText(nItem, 1, _T(EMPTRY_STRING));
				m_List.SetItemText(nItem, 2, _T(EMPTRY_STRING));
				m_List.SetItemText(nItem, 3, _T(EMPTRY_STRING));
			}
		}
		break;
	case VK_RETURN:
		{
			POSITION pos = m_List.GetFirstSelectedItemPosition();
			if(pos)
			{
				int nItem = m_List.GetNextSelectedItem(pos);
				EditItem(nItem);
			}
		}
		break;
	}
	
	
	*pResult = 0;
}

void COptionFriends::OnCheckDisableFriends() 
{
	if(m_bDisableRecieve.GetCheck() == BST_CHECKED)
		GetDlgItem(IDC_EDIT_PLACE_ON_CLIPBOARD)->EnableWindow(FALSE);
	else
		GetDlgItem(IDC_EDIT_PLACE_ON_CLIPBOARD)->EnableWindow(TRUE);
}
