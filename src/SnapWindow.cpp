#include "stdafx.h"
#include "SnapWindow.h"


SnapWindow::SnapWindow()
{
	snap_ModifierKey = VK_SHIFT;
	NONCLIENTMETRICS ncm = { 0 };
	ncm.cbSize = sizeof ncm;
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
	snap_Margin = ncm.iCaptionHeight;

	snap_cur_pos.x = 0;
	snap_cur_pos.y = 0;
	snap_rcWindow.bottom = 0;
	snap_rcWindow.left = 0;
	snap_rcWindow.right = 0;
	snap_rcWindow.top = 0;	
	snap_x = 0;
	snap_y = 0;
}


SnapWindow::~SnapWindow()
{
}

BOOL SnapWindow::isSnapClose(int a, int b) 
{
	return (abs(a - b) < snap_Margin); 
}

LRESULT SnapWindow::OnSnapEnterSizeMove(HWND hWnd)
{
	snap_cur_pos.x = 0;
	snap_cur_pos.y = 0;

	snap_rcWindow.bottom = 0;
	snap_rcWindow.left = 0;
	snap_rcWindow.right = 0;
	snap_rcWindow.top = 0;

	GetWindowRect(hWnd, &snap_rcWindow);

	GetCursorPos(&snap_cur_pos);

	snap_x = snap_cur_pos.x - snap_rcWindow.left;
	snap_y = snap_cur_pos.y - snap_rcWindow.top;

	return 0;
}

LRESULT SnapWindow::OnSnapMoving(HWND hWnd, LPRECT snap_prc)
{
	//no snap if shift key pressed
	if (GetAsyncKeyState(snap_ModifierKey) < 0) 
		return FALSE;
	
	snap_cur_pos.x = 0;
	snap_cur_pos.y = 0;
	snap_rcWindow.bottom = 0;
	snap_rcWindow.left = 0;
	snap_rcWindow.right = 0;
	snap_rcWindow.top = 0;

	GetCursorPos(&snap_cur_pos);
	OffsetRect(snap_prc,
		snap_cur_pos.x - (snap_prc->left + snap_x),
		snap_cur_pos.y - (snap_prc->top + snap_y));

	//it may change during app lifetime
	SystemParametersInfo(SPI_GETWORKAREA, 0, &snap_wa, 0);

	MONITORINFO mi;
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);
	snap_wa = mi.rcWork;

	if (isSnapClose(snap_prc->left, snap_wa.left)) 
	{ 
		OffsetRect(snap_prc, snap_wa.left - snap_prc->left, 0); 
	}
	else
	{
		if (isSnapClose(snap_wa.right, snap_prc->right)) 
		{ 
			OffsetRect(snap_prc, snap_wa.right - snap_prc->right, 0); 
		}
	}

	if (isSnapClose(snap_prc->top, snap_wa.top)) 
	{ 
		OffsetRect(snap_prc, 0, snap_wa.top - snap_prc->top); 
	}
	else
	{
		if (isSnapClose(snap_wa.bottom, snap_prc->bottom)) 
		{ 
			OffsetRect(snap_prc, 0, snap_wa.bottom - snap_prc->bottom); 
		}
	}
	return TRUE;
}