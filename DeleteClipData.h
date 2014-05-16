#pragma once

#include "DialogResizer.h"
#include "ShowTaskBarIcon.h"
#include "afxwin.h"
#include "ATLComTime.h"
#include <vector>
#include "afxcmn.h"

// CDeleteClipData dialog

class CDeleteData
{
public:
	CDeleteData() :
		m_lID(-1),
		m_dataSize(0)
	{
	}

	long m_lID;
	long m_DatalID;
	CString m_Desc;
	CTime m_createdDateTime;
	CTime m_lastUsedDateTime;
	CString m_clipboardFormat;
	DWORD m_dataSize;
};
	
	
	
class CDeleteClipData : public CDialog
{
	DECLARE_DYNAMIC(CDeleteClipData)

public:
	CDeleteClipData(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDeleteClipData();

	void SetNotifyWnd(HWND hWnd);

// Dialog Data
	enum { IDD = IDD_DELETE_CLIP_DATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
		

	CDialogResizer m_Resize;
	CListCtrl	m_List;
	HWND m_hWndParent;
	CShowTaskBarIcon m_showTaskbar;
	std::vector<CDeleteData> m_data;
	std::vector<CDeleteData> m_toDelete;
	std::vector<CDeleteData> m_filteredOut;
	bool m_applyingDelete;
	bool m_cancelDelete;


	void InitListCtrlCols();
	virtual BOOL OnInitDialog();

	void SetDbSize();

	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcDestroy();
	void LoadItems();
	void FilterItems();
	bool MatchesFilter(CDeleteData *pdata);
	void ApplyDelete();

public:
	CString m_clipTitle;
	BOOL m_filterByClipTitle;
	BOOL m_filterByCreatedDate;
	BOOL m_filterByLastUsedDate;
	BOOL m_filterByClipboardFormat;
	CComboBox m_clipboardFomatCombo;
	COleDateTime m_createdDateStart;
	COleDateTime m_createdDateEnd;
	COleDateTime m_createdTimeStart;
	COleDateTime m_createdTimeEnd;
	COleDateTime m_usedTimeStart;
	COleDateTime m_usedTimeEnd;
	COleDateTime m_usedDateStart;
	COleDateTime m_usedDateEnd;
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnLvnKeydownList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnGetdispinfoList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetdispinfoList2(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_databaseSize;
	CString m_selectedSize;
	CString m_selectedCount;
	CString m_toDeleteSize;
	afx_msg void OnBnClickedCheckClipTitle();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCheckCreateDate();
	afx_msg void OnBnClickedCheckLastUseDate();
	afx_msg void OnBnClickedCheckDataFormat();
	afx_msg void OnLvnColumnclickList2(NMHDR *pNMHDR, LRESULT *pResult);
};
