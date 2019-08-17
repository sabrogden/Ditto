// DeleteClipData.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "DeleteClipData.h"
#include "afxdialogex.h"
#include "Misc.h"
#include "ProgressWnd.h"
#include <algorithm>


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
	, m_selectedCount(_T(""))
{
	m_applyingDelete = false;
	m_cancelDelete = false;

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
	DDX_Text(pDX, IDC_STATIC_SELECTED_COUNT, m_selectedCount);
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
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_CREATE_DATE, &CDeleteClipData::OnBnClickedCheckCreateDate)
	ON_BN_CLICKED(IDC_CHECK_LAST_USE_DATE, &CDeleteClipData::OnBnClickedCheckLastUseDate)
	ON_BN_CLICKED(IDC_CHECK_DATA_FORMAT, &CDeleteClipData::OnBnClickedCheckDataFormat)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CDeleteClipData::OnLvnColumnclickList2)

END_MESSAGE_MAP()

BOOL CDeleteClipData::OnInitDialog()
{
	CDialog::OnInitDialog();

	theApp.m_Language.UpdateDeleteClipData(this);

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
	m_Resize.AddControl(IDC_BUTTON_SEARCH, DR_MoveLeft);
	m_Resize.AddControl(IDC_STATIC_GROUP_SEARCH, DR_SizeWidth);

	InitListCtrlCols();

	SetTimer(1, 500, 0);

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

	m_List.InsertColumn(0, theApp.m_Language.GetDeleteClipDataString("Title", "Title"), LVCFMT_LEFT, 350);
	m_List.InsertColumn(1, theApp.m_Language.GetDeleteClipDataString("Created", "Created"), LVCFMT_LEFT, 150);
	m_List.InsertColumn(2, theApp.m_Language.GetDeleteClipDataString("LastUsed", "Last Used"), LVCFMT_LEFT, 150);
	m_List.InsertColumn(3, theApp.m_Language.GetDeleteClipDataString("Format", "Format"), LVCFMT_LEFT, 150);
	m_List.InsertColumn(4, theApp.m_Language.GetDeleteClipDataString("DataSize", "Data Size"), LVCFMT_LEFT, 100);
}

void CDeleteClipData::LoadItems()
{
	CWaitCursor wait;
	m_data.clear();	
	m_filteredOut.clear();

	if (m_clipboardFomatCombo.GetCount() == 0)
	{
		CppSQLite3Query qFormats = theApp.m_db.execQueryEx(_T("select DISTINCT(strClipBoardFormat) from Data"));
		while (qFormats.eof() == false)
		{
			CString format = qFormats.getStringField(_T("strClipBoardFormat"));
			m_clipboardFomatCombo.AddString(format);

			qFormats.nextRow();
		}
	}

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
	if (m_applyingDelete)
	{
		return;
	}
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
	for (INT_PTR i = count - 1; i >= 0; i--)
	{
		int row = rowsToDelete[i];
		toSelect = row;

		m_data.erase(m_data.begin() + row);
	}

	//Remove the rows that were filtered out but now match
	count = filteredRowsToDelete.size();
	for (INT_PTR i = count - 1; i >= 0; i--)
	{
		int row = filteredRowsToDelete[i];
		m_filteredOut.erase(m_filteredOut.begin() + row);
	}

	if (toSelect > -1)
	{
		m_List.SetItemState(toSelect, LVIS_SELECTED, LVIS_SELECTED);
	}
	
	m_List.SetItemCountEx((int)m_data.size(), 0);
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

	if (m_filterByCreatedDate)
	{		
		CTime dateStart = CTime(m_createdDateStart.GetYear(), m_createdDateStart.GetMonth(), m_createdDateStart.GetDay(), m_createdTimeStart.GetHour(), m_createdTimeStart.GetMinute(), m_createdTimeStart.GetSecond());
		CTime dateEnd = CTime(m_createdDateEnd.GetYear(), m_createdDateEnd.GetMonth(), m_createdDateEnd.GetDay(), m_createdTimeEnd.GetHour(), m_createdTimeEnd.GetMinute(), m_createdTimeEnd.GetSecond());

		if (pdata->m_createdDateTime >= dateStart && pdata->m_createdDateTime <= dateEnd)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if (m_filterByLastUsedDate)
	{
		CTime dateStart = CTime(m_usedDateStart.GetYear(), m_usedDateStart.GetMonth(), m_usedDateStart.GetDay(), m_usedTimeStart.GetHour(), m_usedTimeStart.GetMinute(), m_usedTimeStart.GetSecond());
		CTime dateEnd = CTime(m_usedDateEnd.GetYear(), m_usedDateEnd.GetMonth(), m_usedDateEnd.GetDay(), m_usedTimeEnd.GetHour(), m_usedTimeEnd.GetMinute(), m_usedTimeEnd.GetSecond());

		if (pdata->m_lastUsedDateTime >= dateStart && pdata->m_lastUsedDateTime <= dateEnd)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if (m_filterByClipboardFormat)
	{
		CString str1;
		int n = m_clipboardFomatCombo.GetLBTextLen(m_clipboardFomatCombo.GetCurSel());
		m_clipboardFomatCombo.GetLBText(m_clipboardFomatCombo.GetCurSel(), str1.GetBuffer(n));
		str1.ReleaseBuffer();

		if (pdata->m_clipboardFormat == str1)
		{
			return true;
		}
		else
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
		this->ApplyDelete();
		break;
	}
	*pResult = 0;
}

void CDeleteClipData::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	__int64 selectedDataSize = 0;
	int selectedCount = 0;

	if (pos != NULL)
	{
		while (pos)
		{
			INT_PTR row = m_List.GetNextSelectedItem(pos);

			if(row >= 0 && row < (INT_PTR)m_data.size())
			{
				selectedDataSize += m_data[row].m_dataSize;
				selectedCount++;
			}
		}
	}

	const int MAX_FILE_SIZE_BUFFER = 255;
	TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
	StrFormatByteSize(selectedDataSize, szFileSize, MAX_FILE_SIZE_BUFFER);

	m_selectedSize = szFileSize;

	CString count;
	count.Format(_T("%d"), selectedCount);

	m_selectedCount = count;

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
		if (pDispInfo->item.iItem >= 0 && pDispInfo->item.iItem < m_data.size())
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
	ApplyDelete();
}

void CDeleteClipData::ApplyDelete()
{
	if (m_applyingDelete)
		return;

	if (MessageBox(_T("Delete selected items?  This cannot be undone!"), _T(""), MB_YESNO | MB_ICONWARNING) == IDYES)
	{
		m_List.EnableWindow(FALSE);
		m_applyingDelete = true;
		m_cancelDelete = false;

		CWaitCursor wait;

		try
		{
			theApp.m_db.execDML(_T("PRAGMA auto_vacuum = 2"));

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

			CProgressWnd progress;
			progress.Create(this, _T("Deleting clip items"), TRUE);
			progress.SetRange(0, (int)rowsToDelete.size() + 4);
			progress.SetText(_T("Deleting selected items"));
			progress.SetStep(1);

			INT_PTR count = rowsToDelete.size();
			for (INT_PTR i = count - 1; i >= 0; i--)
			{
				progress.PeekAndPump();
				if (m_cancelDelete || progress.Cancelled())
				{
					break;
				}
				progress.StepIt();

				int row = rowsToDelete[i];

				CDeleteData data = m_data[row];
				try
				{
					//Sleep(100);
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

					m_data.erase(m_data.begin() + row);
				}
				CATCH_SQLITE_EXCEPTION
			}

			progress.StepIt();
			progress.SetText(_T("Shrinking database"));

			try
			{
				for(int i = 0; i < 100; i++)
				{
					int toDeleteCount = theApp.m_db.execScalar(_T("SELECT COUNT(clipID) FROM MainDeletes"));
					if(toDeleteCount <= 0)
						break;

					RemoveOldEntries(false);
				}
			}
			CATCH_SQLITE_EXCEPTION

			theApp.m_db.execDML(_T("PRAGMA auto_vacuum = 1"));
			theApp.m_db.execQuery(_T("VACUUM"));
			
			progress.StepIt();
			progress.SetText(_T("Refreshing database size"));

			SetDbSize();
			
			progress.StepIt();
			progress.SetText(_T("Reloading list"));

			//LoadItems();
			
			progress.StepIt();
			progress.SetText(_T("Applying filter"));
			
			FilterItems();

			m_List.SetItemCountEx((int)m_data.size(), 0);
		}
		CATCH_SQLITE_EXCEPTION

		m_applyingDelete = false;
		m_List.EnableWindow();
	}
}

void CDeleteClipData::OnBnClickedCancel()
{
	if (m_applyingDelete)
	{
		m_cancelDelete = true;
		return;
	}
	DestroyWindow();
}


void CDeleteClipData::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case 1:
		LoadItems();
		KillTimer(1);
		break;
	}

	CDialog::OnTimer(nIDEvent);
}


void CDeleteClipData::OnBnClickedCheckCreateDate()
{
	UpdateData();
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_DATE_CREATE_START), m_filterByCreatedDate);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_TIME_CREATE_START), m_filterByCreatedDate);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_DATE_CREATE_END), m_filterByCreatedDate);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_TIME_CREATE_END), m_filterByCreatedDate);
	::SetFocus(::GetDlgItem(m_hWnd, IDC_DATE_CREATE_START));	
}


void CDeleteClipData::OnBnClickedCheckLastUseDate()
{
	UpdateData();
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_DATE_USE_START), m_filterByLastUsedDate);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_TIME_USE_START), m_filterByLastUsedDate);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_DATE_USE_END), m_filterByLastUsedDate);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_TIME_USE_END), m_filterByLastUsedDate);
	::SetFocus(::GetDlgItem(m_hWnd, IDC_DATE_USE_START));
}


void CDeleteClipData::OnBnClickedCheckDataFormat()
{
	UpdateData();
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_COMBO_DATA_FORMAT), m_filterByClipboardFormat);
	::SetFocus(::GetDlgItem(m_hWnd, IDC_COMBO_DATA_FORMAT));
}

static bool SortByTitleDesc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_Desc > a2.m_Desc;
}

static bool SortByCreatedDateDesc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_createdDateTime > a2.m_createdDateTime;
}

static bool SortByLastUsedDateDesc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_lastUsedDateTime > a2.m_lastUsedDateTime;
}

static bool SortByFormatDesc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_clipboardFormat > a2.m_clipboardFormat;
}

static bool SortByDataSizeDesc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_dataSize > a2.m_dataSize;
}

static bool SortByTitleAsc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_Desc < a2.m_Desc;
}

static bool SortByCreatedDateAsc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_createdDateTime < a2.m_createdDateTime;
}

static bool SortByLastUsedDateAsc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_lastUsedDateTime < a2.m_lastUsedDateTime;
}

static bool SortByFormatAsc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_clipboardFormat < a2.m_clipboardFormat;
}

static bool SortByDataSizeAsc(const CDeleteData& a1, const CDeleteData& a2)
{
	return a1.m_dataSize < a2.m_dataSize;
}

bool desc = true;
void CDeleteClipData::OnLvnColumnclickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	HD_NOTIFY *phdn = (HD_NOTIFY *)pNMHDR;

	switch (phdn->iItem)
	{
	case 0:
		if(desc)
			std::sort(m_data.begin(), m_data.end(), SortByTitleDesc);
		else
			std::sort(m_data.begin(), m_data.end(), SortByTitleAsc);
		break;
	case 1:
		if(desc)
			std::sort(m_data.begin(), m_data.end(), SortByCreatedDateDesc);
		else
			std::sort(m_data.begin(), m_data.end(), SortByCreatedDateAsc);
		break;
	case 2:
		if(desc)
			std::sort(m_data.begin(), m_data.end(), SortByLastUsedDateDesc);
		else
			std::sort(m_data.begin(), m_data.end(), SortByLastUsedDateAsc);
		break;
	case 3:
		if(desc)
			std::sort(m_data.begin(), m_data.end(), SortByFormatDesc);
		else
			std::sort(m_data.begin(), m_data.end(), SortByFormatAsc);
		break;
	case 4:
		if(desc)
			std::sort(m_data.begin(), m_data.end(), SortByDataSizeDesc);
		else
			std::sort(m_data.begin(), m_data.end(), SortByDataSizeAsc);
		break;
	}
	
	desc = !desc;

	m_List.SetItemCountEx((int)m_data.size(), 0);

	*pResult = 0;
}


BOOL CDeleteClipData::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			FilterItems();
			return TRUE;                // Do not process further
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
