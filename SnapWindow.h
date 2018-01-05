#pragma once
class SnapWindow
{
public:
	SnapWindow();
	~SnapWindow();

	int snap_Margin;
	int snap_ModifierKey;
	
	LRESULT OnSnapMoving(HWND hWnd, LPRECT snap_prc);
	LRESULT OnSnapEnterSizeMove(HWND hWnd);

private:
	POINT snap_cur_pos;
	RECT snap_rcWindow, snap_wa, *snap_prc;
	int snap_x, snap_y;

	BOOL isSnapClose(int a, int b);
};

