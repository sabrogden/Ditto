#include "stdafx.h"
#include "NoDbFrameWnd.h"
#include "resource.h"
#include "OptionsSheet.h"
#include "DatabaseUtilities.h"
#include "Options.h"
#include "CP_Main.h"
#include "Misc.h"

#define WM_TRAYNOTIFY WM_USER + 100

#define TIMER_OPEN_DB 1
#define TIMER_ERROR_MSG 2

BEGIN_MESSAGE_MAP(CNoDbFrameWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_FIRST_OPTIONS, &CNoDbFrameWnd::OnFirstOptions)
	ON_COMMAND(ID_FIRST_EXIT_NO_DB, &CNoDbFrameWnd::OnFirstExitNoDb)
	ON_MESSAGE(WM_TRAYNOTIFY, &CNoDbFrameWnd::OnTrayNotification)
	ON_MESSAGE(WM_OPTIONS_CLOSED, OnOptionsClosed)
	ON_WM_TIMER()
	ON_WM_HOTKEY()
END_MESSAGE_MAP()

CNoDbFrameWnd::CNoDbFrameWnd()
{
	m_pOptions = NULL;
	m_pDittoHotKey = NULL;
	m_pDittoHotKey2 = NULL;
	m_pDittoHotKey3 = NULL;
}

int CNoDbFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	////Center the main window so message boxes are in the center
	CRect rcScreen = DefaultMonitorRect();
	CPoint cpCenter = rcScreen.CenterPoint();
	MoveWindow(cpCenter.x, cpCenter.x, 1, 1);

	SetWindowText(_T("Ditto"));

	m_trayIcon.Create(this, IDR_MENU_NO_DB, _T("Ditto"), CTrayNotifyIcon::LoadIcon(IDI_MAINFRAME_NO_DB), WM_TRAYNOTIFY, 0, 1);
	m_trayIcon.SetDefaultMenuItem(ID_FIRST_OPTIONS, FALSE);
	m_trayIcon.MinimiseToTray(this);

	SetTimer(TIMER_OPEN_DB, 15000, NULL);
	SetTimer(TIMER_ERROR_MSG, 180000, NULL);

	g_HotKeys.Init(m_hWnd);

	m_pDittoHotKey = new CHotKey(CString("DittoHotKey"), 704); //704 is ctrl-tilda
	m_pDittoHotKey2 = new CHotKey(CString("DittoHotKey2"));
	m_pDittoHotKey3 = new CHotKey(CString("DittoHotKey3"));

	g_HotKeys.RegisterAll();

	return 0;
}

void CNoDbFrameWnd::OnFirstOptions()
{
	if (m_pOptions != NULL)
	{
		::SetForegroundWindow(m_pOptions->m_hWnd);
	}
	else
	{
		m_pOptions = new COptionsSheet(_T(""));

		if (m_pOptions != NULL)
		{
			((COptionsSheet*)m_pOptions)->SetNotifyWnd(m_hWnd);
			m_pOptions->Create();
			m_pOptions->ShowWindow(SW_SHOW);
		}
	}
}

void CNoDbFrameWnd::OnFirstExitNoDb()
{
	this->SendMessage(WM_CLOSE, 0, 0);
}

LRESULT CNoDbFrameWnd::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	m_trayIcon.OnTrayNotification(wParam, lParam);
	return 0L;
}

void CNoDbFrameWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_OPEN_DB:
		TryOpenDatabase();
		break;
	case TIMER_ERROR_MSG:
		KillTimer(TIMER_ERROR_MSG);
		ShowNoDbMessage();
		break;
	}

	CFrameWnd::OnTimer(nIDEvent);
}

void CNoDbFrameWnd::ShowNoDbMessage()
{
	CString msg = theApp.m_Language.GetString(_T("StartupNoDbMsg"), _T("Ditto was unable to open its database, waiting until it can be opened. Update the path in Options if needed. Path: "));
	msg += StrF(_T(" %s"), CGetSetOptions::GetDBPath());
	m_trayIcon.SetBalloonDetails(msg, _T("Ditto"), CTrayNotifyIcon::BalloonStyle::Info, CGetSetOptions::GetBalloonTimeout());
}

void CNoDbFrameWnd::TryOpenDatabase()
{
	if (IsDatabaseOpen() ||
		CheckDBExists(CGetSetOptions::GetDBPath()))
	{
		g_HotKeys.Remove(m_pDittoHotKey);
		delete m_pDittoHotKey;
		m_pDittoHotKey = NULL;

		g_HotKeys.Remove(m_pDittoHotKey2);
		delete m_pDittoHotKey2;
		m_pDittoHotKey2 = NULL;

		g_HotKeys.Remove(m_pDittoHotKey3);
		delete m_pDittoHotKey3;
		m_pDittoHotKey3 = NULL;

		KillTimer(TIMER_OPEN_DB);
		KillTimer(TIMER_ERROR_MSG);
		m_trayIcon.Hide();

		theApp.CreateMainWnd();
	}
}

LRESULT CNoDbFrameWnd::OnOptionsClosed(WPARAM wParam, LPARAM lParam)
{
	delete m_pOptions;
	m_pOptions = NULL;

	TryOpenDatabase();

	return TRUE;
}

void CNoDbFrameWnd::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (m_pDittoHotKey && nHotKeyId == m_pDittoHotKey->m_Atom ||
		m_pDittoHotKey2 && nHotKeyId == m_pDittoHotKey2->m_Atom ||
		m_pDittoHotKey3 && nHotKeyId == m_pDittoHotKey3->m_Atom)
	{
		ShowNoDbMessage();
	}

	CFrameWnd::OnHotKey(nHotKeyId, nKey1, nKey2);
}
