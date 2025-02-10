#pragma once
#include "WndEx.h"

class CDittoPopupWindow : public CWndEx
{
public:
	CDittoPopupWindow();
	~CDittoPopupWindow();

	void UpdateText(CString text);
	void SetProgressBarPercent(int percent);
	void HideProgressBar();

	void SetCopyToGroupId(int groupId) { m_groupId = groupId; }
	
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);


	CStatic m_textLabel;
	CProgressCtrl m_progressWnd;
	CFont m_font;
	int m_groupId;

	void PumpMessages();

	void DoSize(int cx, int cy);
};

