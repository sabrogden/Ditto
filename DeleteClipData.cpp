// DeleteClipData.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "DeleteClipData.h"
#include "afxdialogex.h"
#include "Misc.h"


// CDeleteClipData dialog

IMPLEMENT_DYNAMIC(CDeleteClipData, CDialog)

CDeleteClipData::CDeleteClipData(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteClipData::IDD, pParent)
	, m_clipTitle(_T(""))
	, m_filterByClipTitle(FALSE)
	, m_filterByCreatedDate(FALSE)
	, m_filterByLastUsedDate(FALSE)
	, m_filterByClipboardFormat(FALSE)
	, m_createdDateStart(COleDateTime::GetCurrentTime())
	, m_createdDateEnd(COleDateTime::GetCurrentTime())
	, m_createdTimeStart(COleDateTime::GetCurrentTime())
	, m_createdTimeEnd(COleDateTime::GetCurrentTime())
	, m_usedTimeStart(COleDateTime::GetCurrentTime())
	, m_usedTimeEnd(COleDateTime::GetCurrentTime())
	, m_usedDateStart(COleDateTime::GetCurrentTime())
	, m_usedDateEnd(COleDateTime::GetCurrentTime())
	, m_databaseSize(_T(""))
	, m_selectedSize(_T(""))
	, m_toDeleteSize(_T(""))
{

}

CDeleteClipData::~CDeleteClipData()
{
}

void CDeleteClipData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_List);
	DDX_Text(pDX, IDC_EDIT_CLIP_TITLE, m_clipTitle);
	DDX_Check(pDX, IDC_CHECK_CLIP_TITLE, m_filterByClipTitle);
	DDX_Check(pDX, IDC_CHECK_CREATE_DATE, m_filterByCreatedDate);
	DDX_Check(pDX, IDC_CHECK_LAST_USE_DATE, m_filterByLastUsedDate);
	DDX_Check(pDX, IDC_CHECK_DATA_FORMAT, m_filterByClipboardFormat);
	DDX_Control(pDX, IDC_COMBO_DATA_FORMAT, m_clipboardFomatCombo);
	DDX_DateTimeCtrl(pDX, IDC_DATE_CREATE_START, m_createdDateStart);
	DDX_DateTimeCtrl(pDX, IDC_DATE_CREATE_END, m_createdDateEnd);
	DDX_DateTimeCtrl(pDX, IDC_TIME_CREATE_START, m_createdTimeStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_CREATE_END, m_createdTimeEnd);
	DDX_DateTimeCtrl(pDX, IDC_TIME_USE_START, m_usedTimeStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_USE_END, m_usedTimeEnd);
	DDX_DateTimeCtrl(pDX, IDC_DATE_USE_START, m_usedDateStart);
	DDX_DateTimeCtrl(pDX, IDC_DATE_USE_END, m_usedDateEnd);
	DDX_Text(pDX, IDC_STATIC_DB_SIZE, m_databaseSize);
	DDX_Text(pDX, IDC_STATIC_SELECTED_SIZE, m_selectedSize);
	DDX_Text(pDX, IDC_STATIC_TO_DELETE_SIZE, m_toDeleteSize);
}


BEGIN_MESSAGE_MAP(CDeleteClipData, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_NCDESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CDeleteClipData::OnBnClickedButtonSearch)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, &CDeleteClipData::OnLvnKeydownList2)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CDeleteClipData::OnLvnItemchangedList2)
	ON_NOTIFY(HDN_GETDISPINFO, 0, &CDeleteClipData::OnHdnGetdispinfoList2)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST2, &CDeleteClipData::OnLvnGetdispinfoList2)
	ON_BN_CLICKED(IDC_CHECK_CLIP_TITLE, &CDeleteClipData::OnBnClickedCheckClipTitle)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDeleteClipData::OnBnClickedButtonApply)
	ON_BN_CLICKED(IDCANCEL, &CDeleteClipData::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CDeleteClipData::OnInitDialog()
{
	CDialog::OnInitDialog();

	//theApp.m_Language.UpdateGlobalHotKeys(this);

	m_Resize.SetParent(m_hWnd);
	m_Resize.AddControl(IDC_LIST2, DR_SizeHeight | DR_SizeWidth);
	m_Resize.AddControl(IDCANCEL, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDC_BUTTON_APPLY, DR_MoveTop | DR_MoveLeft);
	m_Resize.AddControl(IDC_STATIC_TO_DELETE_TEXT, DR_MoveTop);
	m_Resize.AddControl(IDC_STATIC_TO_DELETE_SIZE, DR_MoveTop);
	m_Resize.AddControl(IDC_STATIC_SELECTED_SIZE, DR_MoveTop);
	m_Resize.AddControl(IDC_STATIC_SELECTED_SIZE_TEXT, DR_MoveTop);
	m_Resize.AddControl(IDC_STATIC_DB_SIZE, DR_MoveTop);
	m_Resize.AddControl(IDC_STATIC_DB_SIZE_TEXT, DR_MoveTop);

	InitListCtrlCols();

	LoadItems();

	SetDbSize();

	return TRUE;
}

void CDeleteClipData::SetDbSize()
{
	__int64 size = FileSize(GetDBName());

	const int MAX_FILE_SIZE_BUFFER = 255;
	TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
	StrFormatByteSize(size, szFileSize, MAX_FILE_SIZE_BUFFER);

	m_databaseSize = szFileSize;
	UpdateData(0);
}

void CDeleteClipData::InitListCtrlCols()
{
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_List.InsertColumn(0, theApp.m_Language.GetGlobalHotKeyString("Title", "Title"), LVCFMT_LEFT, 250);
	m_List.InsertColumn(1, theApp.m_Language.GetGlobalHotKeyString("Created", "Created"), LVCFMT_LEFT, 150);
	m_List.InsertColumn(2, theApp.m_Language.GetGlobalHotKeyString("Last Used", "Last Used"), LVCFMT_LEFT, 150);
	m_List.InsertColumn(3, theApp.m_Language.GetGlobalHotKeyString("Type", "Type"), LVCFMT_LEFT, 200);
	m_List.InsertColumn(4, theApp.m_Language.GetGlobalHotKeyString("Data Size", "Data Size"), LVCFMT_LEFT, 100);
}

void CDeleteClipData::LoadItems()
{
	CWaitCursor wait;
	m_data.clear();	
	m_filteredOut.clear();
	m_toDelete.clear();

	CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT Main.lID, Main.mText, Main.lDate, Main.lastPasteDate, "
														  _T("Data.lID AS DataID, Data.strClipBoardFormat, length(Data.ooData) AS DataLength ")
													_T("FROM Data ")
													_T("INNER JOIN Main on Main.lID = Data.lParentID ")
													_T("ORDER BY length(ooData) DESC")));

	int row = 0;
	while (q.eof() == false)
	{
		CDeleteData data;
		data.m_lID = q.getIntField(_T("lID"));
		data.m_Desc = q.getStringField(_T("mText"));
		data.m_createdDateTime = q.getIntField(_T("lDate"));
		data.m_lastUsedDateTime = q.getIntField(_T("lastPasteDate"));
		data.m_clipboardFormat = q.getStringField(_T("strClipBoardFormat"));
		data.m_dataSize = q.getIntField(_T("DataLength"));
		data.m_DatalID = q.getIntField(_T("DataID"));

		m_data.push_back(data);

		row++;

		q.nextRow();		
	}

	m_List.SetItemCountEx(row, 0);
}

//void CDeleteClipData::AddRow(CppSQLite3Query& q, int row)
//{
//	LVITEM lvi;
//
//	lvi.mask = LVIF_TEXT;
//	lvi.iItem = row;	
//
//	lvi.iSubItem = 0;
//	lvi.pszText = (LPTSTR) (LPCTSTR) (q.getStringField(_T("mText")));
//	m_List.InsertItem(&lvi);
//
//	CTime created = q.getIntField(_T("lDate"));
//	COleDateTime dtTime(created.GetTime());
//	
//	CTime pasted = q.getIntField(_T("lastPasteDate"));
//	COleDateTime dtPastedTime(pasted.GetTime());
//
//	m_List.SetItemText(row, 1, dtTime.Format());
//	m_List.SetItemText(row, 2, dtPastedTime.Format());
//	m_List.SetItemText(row, 3, q.getStringField(_T("strClipBoardFormat")));
//
//	int dataLength = q.getIntField(_T("DataLength"));
//
//	const int MAX_FILE_SIZE_BUFFER = 255;
//	TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
//	StrFormatByteSize(dataLength, szFileSize, MAX_FILE_SIZE_BUFFER);
//
//	m_List.SetItemText(row, 4, szFileSize);
//
//	m_List.SetItemData(row, ));
//}

void CDeleteClipData::SetNotifyWnd(HWND hWnd)
{
	m_hWndParent = hWnd;
}

void CDeleteClipData::OnClose()
{
	DestroyWindow();
}

void CDeleteClipData::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	m_Resize.MoveControls(CSize(cx, cy));
}

void CDeleteClipData::OnNcDestroy()
{
	CDialog::OnNcDestroy();
	::PostMessage(m_hWndParent, WM_DELETE_CLIPS_CLOSED, 0, 0);
}

// CDeleteClipData message handlers


void CDeleteClipData::OnBnClickedButtonSearch()
{
	FilterItems();
}

void CDeleteClipData::FilterItems()
{
	UpdateData();

	//First search the already filtered text, see if we need to add them back in
	std::vector<int> filteredRowsToDelete;
	std::vector<CDeleteData> addBackIn;
	INT_PTR count = m_filteredOut.size();
	for (int i = 0; i < count; i++)
	{
		CDeleteData data = m_filteredOut[i];

		if (MatchesFilter(&data))
		{
			addBackIn.push_back(data);
			filteredRowsToDelete.push_back(i);
		}
	}

	//next search the main list
	std::vector<int> rowsToDelete;
	count = m_data.size();
	for (int i = 0; i < count; i++)
	{
		CDeleteData data = m_data[i];

		if(MatchesFilter(&data) == false)
		{
			m_filteredOut.push_back(data);
			rowsToDelete.push_back(i);
		}
	}

	//Add back in the filtered out ones that now match
	count = addBackIn.size();
	for (int i = 0; i < count; i++)
	{
		CDeleteData data = addBackIn[i];
		m_data.push_back(data);
	}

	int toSelect = -1;

	//Remove from the main list the ones that don't match
	count = rowsToDelete.size();
	for (int i = count - 1; i >= 0; i--)
	{
		int row = rowsToDelete[i];
		toSelect = row;

		m_data.erase(m_data.begin() + row);
	}

	//Remove the rows that were filtered out but now match
	count = filteredRowsToDelete.size();
	for (int i = count - 1; i >= 0; i--)
	{
		int row = filteredRowsToDelete[i];
		m_filteredOut.erase(m_filteredOut.begin() + row);
	}

	if (toSelect > -1)
	{
		m_List.SetItemState(toSelect, LVIS_SELECTED, LVIS_SELECTED);
	}
	
	m_List.SetItemCountEx(m_data.size(), 0);
}

bool CDeleteClipData::MatchesFilter(CDeleteData *pdata)
{
	if(m_filterByClipTitle &&
		m_clipTitle != _T("") &&
		pdata->m_Desc != _T(""))
	{
		if(pdata->m_Desc.MakeLower().Find(m_clipTitle.MakeLower()) == -1)
		{
			return false;
		}
	}

	return true;
}

void CDeleteClipData::OnLvnKeydownList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here

	switch(pLVKeyDow->wVKey)
	{
	case VK_DELETE:
		DeleteSelectedRows();
		break;
	}
	*pResult = 0;
}

void CDeleteClipData::DeleteSelectedRows()
{
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	std::vector<int> rowsToDelete;

	if (pos != NULL)
	{
		while (pos)
		{
			int row = m_List.GetNextSelectedItem(pos);
			rowsToDelete.push_back(row);
		}
	}

	int toSelect = -1;

	INT_PTR count = rowsToDelete.size();
	for (int i = count-1; i >= 0; i--)
	{		
		int row = rowsToDelete[i];
		toSelect = row;

		m_toDelete.push_back(m_data[row]);
		m_data.erase(m_data.begin() + row);
	}

	if(toSelect > -1)
	{
		m_List.SetItemState(toSelect, LVIS_SELECTED, LVIS_SELECTED);
	}

	UpdateToDeleteSize();

	m_List.SetItemCountEx(m_data.size(), 0);
}

void CDeleteClipData::UpdateToDeleteSize()
{
	__int64 toDeleteDataSize = 0;
	INT_PTR count = m_toDelete.size();
	for (int i = 0; i < count; i++)
	{
		CDeleteData data = m_toDelete[i];
		toDeleteDataSize += data.m_dataSize;
	}

	const int MAX_FILE_SIZE_BUFFER = 255;
	TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
	StrFormatByteSize(toDeleteDataSize, szFileSize, MAX_FILE_SIZE_BUFFER);

	m_toDeleteSize = szFileSize;

	UpdateData(0);
}


void CDeleteClipData::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	__int64 selectedDataSize = 0;

	if (pos != NULL)
	{
		while (pos)
		{
			INT_PTR row = m_List.GetNextSelectedItem(pos);

			if(row >= 0 && row < m_data.size())
			{
				selectedDataSize += m_data[row].m_dataSize;
			}
		}
	}

	const int MAX_FILE_SIZE_BUFFER = 255;
	TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
	StrFormatByteSize(selectedDataSize, szFileSize, MAX_FILE_SIZE_BUFFER);

	m_selectedSize = szFileSize;

	UpdateData(0);

	*pResult = 0;
}


void CDeleteClipData::OnHdnGetdispinfoList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHDDISPINFO pDispInfo = reinterpret_cast<LPNMHDDISPINFO>(pNMHDR);	
	
	if (pDispInfo->mask &LVIF_TEXT)
	{
		switch (pDispInfo->iItem)
		{
		case 0:
			break;
		}
	}

	*pResult = 0;
}


void CDeleteClipData::OnLvnGetdispinfoList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		switch (pDispInfo->item.iSubItem)
		{
		case 0:
			{
				lstrcpyn(pDispInfo->item.pszText, m_data[pDispInfo->item.iItem].m_Desc, pDispInfo->item.cchTextMax);
				pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = '\0';
			}
			break;
		case 1:
			{
				COleDateTime dtTime(m_data[pDispInfo->item.iItem].m_createdDateTime.GetTime());
				lstrcpyn(pDispInfo->item.pszText, dtTime.Format(), pDispInfo->item.cchTextMax);
				pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = '\0';
			}
			break;
		case 2:
			{
				COleDateTime dtTime(m_data[pDispInfo->item.iItem].m_lastUsedDateTime.GetTime());
				lstrcpyn(pDispInfo->item.pszText, dtTime.Format(), pDispInfo->item.cchTextMax);
				pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = '\0';
			}
			break;
		case 3:
			{
				lstrcpyn(pDispInfo->item.pszText, m_data[pDispInfo->item.iItem].m_clipboardFormat, pDispInfo->item.cchTextMax);
				pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = '\0';
			}
			break;
		case 4:
			{
				const int MAX_FILE_SIZE_BUFFER = 255;
				TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
				StrFormatByteSize(m_data[pDispInfo->item.iItem].m_dataSize, szFileSize, MAX_FILE_SIZE_BUFFER);

				lstrcpyn(pDispInfo->item.pszText, szFileSize, pDispInfo->item.cchTextMax);
				pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = '\0';
			}
			break;
		}
	}
	*pResult = 0;
}


void CDeleteClipData::OnBnClickedCheckClipTitle()
{
	UpdateData();
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_CLIP_TITLE), m_filterByClipTitle);	
	::SetFocus(::GetDlgItem(m_hWnd, IDC_EDIT_CLIP_TITLE));
}


void CDeleteClipData::OnBnClickedButtonApply()
{
	if(MessageBox(_T("Delete items?"), _T(""), MB_YESNO) == IDYES)
	{
		ApplyDelete();
	}
}

void CDeleteClipData::ApplyDelete()
{
	CWaitCursor wait;
	INT_PTR count = m_toDelete.size();
	for (int i = 0; i < count; i++)
	{
		CDeleteData data = m_toDelete[i];
		try
		{
			int deleteCount = theApp.m_db.execDMLEx(_T("DELETE FROM Data where lID = %d"), data.m_DatalID);

			//If there are no more children for this clip then delete the parent
			int parentDeleteCount = theApp.m_db.execDMLEx(_T("DELETE FROM Main where lID IN ")
				_T("(")
				_T("SELECT Main.lID ")
				_T("FROM Main ")
				_T("LEFT OUTER JOIN Data on Data.lParentID = Main.lID ")
				_T("WHERE bIsGroup = 0 AND Main.lID = %d ")
				_T("Group by Main.lID ")
				_T("having Count(Data.lID) = 0 ")
				_T(")"), data.m_lID);
		}
		CATCH_SQLITE_EXCEPTION
	}

	

	SetDbSize();
	LoadItems();
	FilterItems();
}

void CDeleteClipData::OnBnClickedCancel()
{
	DestroyWindow();
}
