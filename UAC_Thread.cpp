#include "stdafx.h"
#include "UAC_Thread.h"

#include "Misc.h"
#include "Options.h"
#include "QPasteWnd.h"
#include "cp_main.h"

CUAC_Thread::CUAC_Thread(int processId)
{
	m_processId = processId;

	AddEvent(UAC_PASTE, StrF(_T("Global\\UAC_PASTE_%d"), m_processId));
	AddEvent(UAC_EXIT, StrF(_T("Global\\UAC_EXIT_%d"), m_processId));

	m_waitTimeout = 30000;
}

CUAC_Thread::~CUAC_Thread(void)
{
}


void CUAC_Thread::OnTimeOut(void *param)
{
}

void CUAC_Thread::OnEvent(int eventId, void *param)
{
	DWORD startTick = GetTickCount();
	Log(StrF(_T("Start of OnEvent, eventId: %s"), EnumName((eUacThreadEvents)eventId)));

	switch((eUacThreadEvents)eventId)
	{
	case UAC_PASTE:
		theApp.m_activeWnd.SendPaste(false);
		break; 
	case UAC_EXIT:
		this->CancelThread();
		break;
	}

	DWORD length = GetTickCount() - startTick;
	Log(StrF(_T("End of OnEvent, eventId: %s, Time: %d(ms)"), EnumName((eUacThreadEvents)eventId), length));
}

CString CUAC_Thread::EnumName(eUacThreadEvents e)
{
	switch(e)
	{
	case UAC_PASTE:
		return _T("Paste Elevated");
	case UAC_EXIT:
		return _T("Save Startup Elevated");
	}

	return _T("");
}

void CUAC_Thread::UACPaste()
{
	CString mutexName;
	mutexName.Format(_T("DittoAdminPaste_%d"), GetCurrentProcessId());

	HANDLE mutex = CreateMutex(NULL, FALSE, mutexName);
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
	}
	else
	{
		wchar_t szPath[MAX_PATH];
		if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		{
			// Launch itself as administrator.
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = L"runas";
			sei.lpFile = szPath;
			CString csParam;
			csParam.Format(_T("/uacpaste:%d"), GetCurrentProcessId());
			sei.lpParameters = csParam;
			sei.nShow = SW_NORMAL;

			if (!ShellExecuteEx(&sei))
			{
			}
		}
	}

	FirePaste();

	CloseHandle(mutex);
}