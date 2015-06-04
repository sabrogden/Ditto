#pragma once

class CTrayWnd : public CWnd
{
	DECLARE_DYNAMIC(CTrayWnd)

public:
	CTrayWnd();
	virtual ~CTrayWnd();

protected:
	DECLARE_MESSAGE_MAP()
	LRESULT OnTaskBarCreated(WPARAM wParam, LPARAM lParam);
};


