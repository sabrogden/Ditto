#pragma once
class CShowTaskBarIcon
{
public:
	CShowTaskBarIcon(void);
	~CShowTaskBarIcon(void);

	HWND m_hWnd;
	static volatile long m_refCount;
};

