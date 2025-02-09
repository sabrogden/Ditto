#pragma once

#include <vector>

class CMagneticWnd : public CWnd
{
public:
	CMagneticWnd(void);
	~CMagneticWnd(void);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnMove(int x, int y);

	std::vector<CMagneticWnd*> m_SnapToWnds;
	std::vector<CMagneticWnd*> m_AttachedWnd;
	bool m_bMovedAttachedWnd;
	CRect m_crLastMove;
	bool m_bHandleWindowPosChanging;

public:
	void AddWindowToSnapTo(CMagneticWnd *pWnd)	{ m_SnapToWnds.push_back(pWnd); }
	void SetWindowAttached(CMagneticWnd *pOther, bool bAttache);
	bool IsWindowAttached(CMagneticWnd *pWnd);
	void MoveMagneticWWnd(LPCRECT lpRect, BOOL bRepaint = TRUE);

	void SetMoveAttachedWnds(bool bMove)	{ m_bMovedAttachedWnd = bMove; }
};
