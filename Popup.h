#pragma once
/*------------------------------------------------------------------*\
	CPopup - a tooltip that pops up manually (when Show is called).
	- technique learned from codeproject "ToolTipZen" by "Zarembo Maxim"
\*------------------------------------------------------------------*/

void InitToolInfo( TOOLINFO& ti ); // initializes toolinfo with uid 0

class CPopup
{
public:
	bool m_bOwnTT;

	HWND m_hTTWnd; // handle to the ToolTip control
	TOOLINFO m_TI; // struct specifying info about tool in ToolTip control

	bool m_bIsShowing;

	bool m_bTop;  // true if m_Pos.x is the top, false if the bottom
	bool m_bLeft; // true if m_Pos.y is the left, false if the right
	bool m_bCenterY; // true if m_Pos is the y center, false if corner
	bool m_bCenterX; // true if m_Pos is the x center, false if corner
	HWND m_hWndPosRelativeTo;
	CPoint m_Pos;

	int m_ScreenMaxX;
	int m_ScreenMaxY;

	HWND m_hWndInsertAfter;

	bool m_bAllowShow; // used by SafeShow to determine whether to show or not

	CString m_csToolTipText;

	CPopup();
	CPopup( int x, int y, HWND hWndPosRelativeTo = NULL, HWND hWndInsertAfter = HWND_TOP );
	~CPopup();

	void Init();
	void SetTTWnd( HWND hTTWnd = NULL, TOOLINFO* pTI = NULL );
	void CreateToolTip();

	void SetTimeout( int timeout );

	void AdjustPos( CPoint& pos );
	void SetPos( CPoint& pos );
	void SetPosInfo( bool bTop, bool bCenterY, bool bLeft, bool bCenterX );

	void SendToolTipText( CString text );

	void Show( CString text, CPoint pos, bool bAdjustPos = true );
	void Show( CString text );
	void AllowShow( CString text ); // only shows if m_bAllowShow is true

	void Hide();
};

