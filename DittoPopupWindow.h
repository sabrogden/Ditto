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
	
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);


	CStatic m_textLabel;
	CProgressCtrl m_progressWnd;
	CFont m_font;

	void PumpMessages();

	void DoSize(int cx, int cy);
};

