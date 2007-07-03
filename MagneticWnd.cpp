#include "stdafx.h"
#include ".\magneticwnd.h"

bool bDoChange = true;

CMagneticWnd::CMagneticWnd()
{
	m_bMovedAttachedWnd = false;
	m_crLastMove.SetRectEmpty();
	m_bHandleWindowPosChanging = true;
}

CMagneticWnd::~CMagneticWnd()
{
}


BEGIN_MESSAGE_MAP(CMagneticWnd, CWnd)
	ON_WM_MOVE()
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

void CMagneticWnd::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);

	if(m_bMovedAttachedWnd)
	{
		CRect crThis;
		CRect crAttached;
		long lDiff = 0;

		GetWindowRect(crThis);

		for(std::vector<CMagneticWnd*>::iterator Iter = m_AttachedWnd.begin(); Iter != m_AttachedWnd.end(); Iter++)
		{
			CMagneticWnd *pAttachedWnd = *Iter;
			if(pAttachedWnd)
			{
				if(m_crLastMove.IsRectEmpty() == FALSE)
				{
					pAttachedWnd->GetWindowRect(crAttached);

					long lXDiff = crThis.left - m_crLastMove.left;
					long lYDiff = crThis.bottom - m_crLastMove.bottom;

					crAttached.left += lXDiff;
					crAttached.right += lXDiff;
					crAttached.top += lYDiff;
					crAttached.bottom += lYDiff;

					pAttachedWnd->MoveMagneticWWnd(crAttached);
				}
			}
		}

		m_crLastMove = crThis;
	}
}

void CMagneticWnd::MoveMagneticWWnd(LPCRECT lpRect, BOOL bRepaint)
{
	m_bHandleWindowPosChanging = false;
	MoveWindow(lpRect, bRepaint);
	m_bHandleWindowPosChanging = true;
}


void CMagneticWnd::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanging(lpwndpos);

	if(m_bHandleWindowPosChanging == false)
		return;

	for(std::vector<CMagneticWnd*>::iterator Iter = m_SnapToWnds.begin(); Iter != m_SnapToWnds.end(); Iter++)
	{
		CMagneticWnd *pOtherWnd = *Iter;

		if(pOtherWnd != NULL && 
			this->IsWindowVisible() && 
			pOtherWnd->IsWindowVisible() &&
			lpwndpos->x != 0 && lpwndpos->y != 0 && lpwndpos->cx != 0 && lpwndpos->cy != 0)
		{
			if(m_bMovedAttachedWnd && (IsWindowAttached(pOtherWnd)))
				continue;

			CRect rectParent;
			pOtherWnd->GetWindowRect(rectParent);
			bool bAttached = false;

			// Snap left edge
			if(abs(lpwndpos->x - rectParent.right) <= 15)
			{
				lpwndpos->x = rectParent.right;
				bAttached = true;
			}

			// Snap right edge
			if (abs(lpwndpos->x + lpwndpos->cx - rectParent.left) <= 15)
			{
				lpwndpos->x = rectParent.left - lpwndpos->cx;
				bAttached = true;
			}

			// Snap to the bottom
			if (abs(lpwndpos->y + lpwndpos->cy - rectParent.top) <= 15)
			{
				lpwndpos->y = rectParent.top - lpwndpos->cy;
				bAttached = true;
			} 

			// Snap the top
			if (abs(lpwndpos->y - rectParent.bottom) <= 15)
			{
				lpwndpos->y = rectParent.bottom;
				bAttached = true;
			}

			pOtherWnd->SetWindowAttached(this, bAttached);
			SetWindowAttached(pOtherWnd, bAttached);

			if(m_bMovedAttachedWnd)
			{
				m_crLastMove.left = lpwndpos->x;
				m_crLastMove.top = lpwndpos->y;
				m_crLastMove.right = lpwndpos->x + lpwndpos->cx;
				m_crLastMove.bottom = lpwndpos->y + lpwndpos->cy;
			}
		}
	}
}

void CMagneticWnd::SetWindowAttached(CMagneticWnd *pOtherWnd, bool bAttach)
{
	bool bFound = false;
	for(std::vector<CMagneticWnd*>::iterator Iter = m_AttachedWnd.begin(); Iter != m_AttachedWnd.end(); Iter++)
	{
		CMagneticWnd *pAttachedWnd = *Iter;
		if(pAttachedWnd == pOtherWnd)
		{
			bFound = true;
			if(bAttach == false)
			{
				m_AttachedWnd.erase(Iter);
			}
			break;
		}
	}
	
	if(bAttach && bFound == false)
	{
		m_AttachedWnd.push_back(pOtherWnd);
	}
}

bool CMagneticWnd::IsWindowAttached(CMagneticWnd *pWnd)
{
	for(std::vector<CMagneticWnd*>::iterator Iter = m_AttachedWnd.begin(); Iter != m_AttachedWnd.end(); Iter++)
	{
		CMagneticWnd *pAttachedWnd = *Iter;
		if(pAttachedWnd == pWnd)
		{
			return true;
		}
	}

	return false;
}