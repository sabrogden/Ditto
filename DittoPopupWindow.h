#pragma once
#include "WndEx.h"

class CDittoPopupWindow : public CWndEx
{
public:
	CDittoPopupWindow();
	~CDittoPopupWindow();

	void UpdateText(CString text);
	
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);


	CStatic m_textLabel;
	CFont m_font;

	void PumpMessages();
};

