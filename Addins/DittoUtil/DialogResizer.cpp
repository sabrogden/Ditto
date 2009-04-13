// DialogResizer.cpp: implementation of the CDialogResizer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DialogResizer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CDialogResizer::CDialogResizer()
{

}

CDialogResizer::~CDialogResizer()
{

}

void CDialogResizer::SetParent(HWND hWndParent)
{
	m_hWndParent = hWndParent;
	CRect cr;
	GetClientRect(m_hWndParent, cr);

	m_DlgSize.cx = cr.Width();
	m_DlgSize.cy = cr.Height();
}

void CDialogResizer::AddControl(int nControlID, int nFlags)
{
	HWND hWnd = GetDlgItem(m_hWndParent, nControlID);
	if(hWnd)
		AddControl(hWnd, nFlags);
}

void CDialogResizer::AddControl(HWND hWnd, int nFlags)
{
	CDR_Data data;
	data.m_hWnd = hWnd;
	data.m_nFlags = nFlags;


	m_Controls.Add(data);
}

void CDialogResizer::MoveControls(CSize csNewSize)
{
	int nDeltaX = csNewSize.cx - m_DlgSize.cx;
	int nDeltaY = csNewSize.cy - m_DlgSize.cy;

	if(nDeltaX == 0 && nDeltaY == 0)
		return;

	m_DlgSize = csNewSize;

	int nCount = (int)m_Controls.GetSize();
	CRect rc;
	CRect rcParent;

	GetClientRect(m_hWndParent, rcParent);

	for(int i = 0; i < nCount; i++)
	{
		CDR_Data data = m_Controls[i];

		GetWindowRect(data.m_hWnd, rc);
		MapWindowPoints(GetDesktopWindow(),  m_hWndParent, (LPPOINT)&rc, 2 );
		
		//
		//	Adjust the window horizontally
		if( data.m_nFlags & DR_MoveLeft )
		{
			rc.left += nDeltaX;
			rc.right += nDeltaX;
		}

		//
		//	Adjust the window vertically
		if( data.m_nFlags & DR_MoveTop )
		{
			rc.top += nDeltaY;
			rc.bottom += nDeltaY;
		}

		//
		//	Size the window horizontally
		if( data.m_nFlags & DR_SizeWidth )
		{
			rc.right += nDeltaX;
		}

		//	Size the window vertically
		if( data.m_nFlags & DR_SizeHeight )
		{
			rc.bottom += nDeltaY;
		}

		::SetWindowPos(data.m_hWnd, NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
}
