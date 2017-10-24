#pragma once
#include "afxpropertygridctrl.h"
#include "DialogResizer.h"

class CAdvGeneral : public CDialogEx
{
	DECLARE_DYNAMIC(CAdvGeneral)

public:
	CAdvGeneral(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAdvGeneral();

// Dialog Data
	enum { IDD = IDD_ADV_OPTIONS };

	CDialogResizer m_Resize;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void AddTrueFalse(CMFCPropertyGridProperty * pGroupTest, CString desc, BOOL value, int settingId);

	DECLARE_MESSAGE_MAP()
public:
	CMFCPropertyGridCtrl m_propertyGrid;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtCompactAndRepair();
	afx_msg void OnBnClickedButtonCopyScripts();
	afx_msg void OnBnClickedButtonPasteScripts2();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};
