#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CQuickPasteKeyboard dialog

class CQuickPasteKeyboard : public CPropertyPage
{
	DECLARE_DYNAMIC(CQuickPasteKeyboard)

public:
	CQuickPasteKeyboard();
	virtual ~CQuickPasteKeyboard();

	class KeyboardAB
	{
	public:
		KeyboardAB()
		{
			A = -1;
			B = 0;
			Dirty = false;
		}
		int A;
		int B;
		bool Dirty;
	};

	class KeyboardArray
	{
	public:
		KeyboardArray()
		{
		}

		KeyboardAB Array[10];
	};

// Dialog Data
	enum { IDD = IDD_OPTIONS_QUICK_PASTE_KEYBOARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	virtual BOOL OnInitDialog();

protected:
	void InitListCtrlCols();
	void LoadItems();
	CString GetShortCutText(KeyboardArray ab);
	CString GetShortCutText(KeyboardAB ab);
	std::map<DWORD, KeyboardArray> m_map;
	void LoadHotKey(KeyboardAB ab);
	CString m_csTitle;
	int SelectedCommandId();
	int SelectedCommandShortCutId();
	int SelectedCommandRow();
	void SelectMouseTypeCombo(CComboBox &combo, int value);
	void SelectedRow(int row);

public:
	afx_msg void OnLvnItemActivateList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	CHotKeyCtrl m_hotKey1;
	afx_msg void OnBnClickedAssign();
	CHotKeyCtrl m_hotKey2;
	virtual BOOL OnApply();
	CComboBox m_assignedCombo;
	afx_msg void OnCbnSelchangeComboAllAssigned();
	afx_msg void OnBnClickedButtonRemove();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnCbnKillfocusComboEnterModifyer();
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonEnter();
	afx_msg void OnBnClickedButtonEnter2();

	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	CComboBox m_mouseType1;
	CComboBox m_mouseType2;
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedRadioKeyboard1();
	afx_msg void OnBnClickedRadioMouse1();
	afx_msg void OnBnClickedRadioKeyboard2();
	afx_msg void OnBnClickedRadioMouse2();
};
