#include "stdafx.h"
#include "Popup.h"
#include "Misc.h"

void InitToolInfo( TOOLINFO& ti )
{
	// INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_ABSOLUTE | TTF_TRACK;
	ti.hwnd = NULL;
	ti.hinst = NULL;
	ti.uId = 0; // CPopup only uses uid 0
	ti.lpszText = NULL;
	// ToolTip control will cover the whole window
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 0;
	ti.rect.bottom = 0;
}

/*------------------------------------------------------------------*\
CPopup - a tooltip that pops up manually (when Show is called).
- technique learned from codeproject "ToolTipZen" by "Zarembo Maxim"
\*------------------------------------------------------------------*/

CPopup::CPopup()
{
	Init();
}

// HWND_TOP
CPopup::CPopup( int x, int y, HWND hWndPosRelativeTo, HWND hWndInsertAfter )
{
	Init();
	m_hWndPosRelativeTo = hWndPosRelativeTo;
	m_hWndInsertAfter = hWndInsertAfter;
	SetPos( CPoint(x,y) );
}

CPopup::~CPopup()
{
	Hide();
	if( m_bOwnTT && ::IsWindow(m_hTTWnd) )
		::DestroyWindow( m_hTTWnd );
}

void CPopup::Init()
{
	// initialize variables
	m_bOwnTT = false;
	m_hTTWnd = NULL;
	m_bIsShowing = false;
	m_bAllowShow = true; // used by AllowShow()
	
	m_Pos.x = m_Pos.y = 0;
	m_bTop = true;
	m_bLeft = true;
	m_bCenterX = false;
	m_bCenterY = false;
	m_hWndPosRelativeTo = NULL;
	
	RECT rcScreen;
	
	GetMonitorRect(-1, &rcScreen);
	
	m_ScreenMaxX = rcScreen.right;
	m_ScreenMaxY = rcScreen.bottom;
	
	m_hWndInsertAfter = HWND_TOP; //HWND_TOPMOST
	
	SetTTWnd();
}

void CPopup::SetTTWnd( HWND hTTWnd, TOOLINFO* pTI )
{
	if( pTI )
		m_TI = *pTI;
	else
		InitToolInfo( m_TI );
	
	if( m_bOwnTT && ::IsWindow(m_hTTWnd) )
	{
		if( !::IsWindow(hTTWnd) )
			return; // we would have to recreate the one that already exists
		::DestroyWindow( m_hTTWnd );
	}
	
	m_hTTWnd = hTTWnd;
	if( ::IsWindow(m_hTTWnd) )
	{
		m_bOwnTT = false;
		// if our uid tooltip already exists, get the data, else add it.
		if( ! ::SendMessage(m_hTTWnd, TTM_GETTOOLINFO, 0, (LPARAM)(LPTOOLINFO) &m_TI) )
			::SendMessage(m_hTTWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &m_TI);
	}
	else
	{
		m_bOwnTT = true;
		CreateToolTip();
	}
}

void CPopup::CreateToolTip()
{
	if( m_hTTWnd != NULL )
		return;
	
	// CREATE A TOOLTIP WINDOW
	m_hTTWnd = CreateWindowEx(
		WS_EX_TOPMOST,
		TOOLTIPS_CLASS,
		NULL,
		TTS_NOPREFIX | TTS_ALWAYSTIP,		
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL
		);
	m_bOwnTT = true;
	
	// SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW
	::SendMessage(m_hTTWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &m_TI);
}

void CPopup::SetTimeout( int timeout )
{
	if( m_hTTWnd == NULL )
		return;
	::SendMessage(m_hTTWnd, TTM_SETDELAYTIME, TTDT_AUTOMATIC, timeout);
}

void CPopup::SetPos( CPoint& pos )
{
	m_Pos = pos;
}

void CPopup::SetPosInfo( bool bTop, bool bCenterY, bool bLeft, bool bCenterX )
{
	m_bTop = bTop;
	m_bCenterY = bCenterY;
	m_bLeft = bLeft;
	m_bCenterX = bCenterX;
}

void CPopup::AdjustPos( CPoint& pos )
{
	CRect rel(0,0,0,0);
	CRect rect(0,0,0,0);
	
	//	::SendMessage(m_hTTWnd, TTM_ADJUSTRECT, TRUE, (LPARAM)&rect);
	::GetWindowRect(m_hTTWnd,&rect);
	
	if( ::IsWindow(m_hWndPosRelativeTo) )
		::GetWindowRect(m_hWndPosRelativeTo, &rel);
	
	// move the rect to the relative origin
	rect.bottom = rect.Height() + rel.top;
	rect.top = rel.top;
	rect.right = rect.Width() + rel.left;
	rect.left = rel.left;
	
	// adjust the y position
	rect.OffsetRect( 0, pos.y - (m_bCenterY? rect.Height()/2: (m_bTop? 0: rect.Height())) );
	if( rect.bottom > m_ScreenMaxY )
		rect.OffsetRect( 0, m_ScreenMaxY - rect.bottom );
	
	// adjust the x position
	rect.OffsetRect( pos.x - (m_bCenterX? rect.Width()/2: (m_bLeft? 0: rect.Width())), 0 );
	if( rect.right > m_ScreenMaxX )
		rect.OffsetRect( m_ScreenMaxX - rect.right, 0 );
	
	pos.x = rect.left;
	pos.y = rect.top;
}

void CPopup::SendToolTipText( CString text )
{
	m_csToolTipText = text;
	
	//Replace the tabs with spaces, the tooltip didn't like the \t s
	text.Replace(_T("\t"), _T("  "));
	m_TI.lpszText = (LPTSTR) (LPCTSTR) text;
	
	// this allows \n and \r to be interpreted correctly
	::SendMessage(m_hTTWnd, TTM_SETMAXTIPWIDTH, 0, 500);
	// set the text
	::SendMessage(m_hTTWnd, TTM_SETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &m_TI);
}

void CPopup::Show( CString text, CPoint pos, bool bAdjustPos )
{
	if( m_hTTWnd == NULL )
		return;

	m_csToolTipText = text;
	
	if( !m_bIsShowing )
		::SendMessage(m_hTTWnd, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD) MAKELONG(-10000,-10000));
	
	SendToolTipText( text );
	::SendMessage(m_hTTWnd, TTM_TRACKACTIVATE, true, (LPARAM)(LPTOOLINFO) &m_TI);
	if( bAdjustPos )
		AdjustPos(pos);
	// set the position
	::SendMessage(m_hTTWnd, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD) MAKELONG(pos.x,pos.y));
	
	// make sure the tooltip will be on top.
	::SetWindowPos( m_hTTWnd, m_hWndInsertAfter, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
	
	m_bIsShowing = true;
}

void CPopup::Show( CString text )
{ 
	m_csToolTipText = text;
	Show( text, m_Pos ); 
}

void CPopup::AllowShow( CString text )
{
	m_csToolTipText = text;
	
	if( m_bAllowShow )
		Show( text, m_Pos );
}

void CPopup::Hide()
{
	if( m_hTTWnd == NULL )
		return;
	// deactivate if it is currently activated
	::SendMessage(m_hTTWnd, TTM_TRACKACTIVATE, FALSE, (LPARAM)(LPTOOLINFO) &m_TI);
	m_bIsShowing = false;
}