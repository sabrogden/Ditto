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
	AddEvent(UAC_COPY, StrF(_T("Global\\UAC_COPY_%d"), m_processId));
	AddEvent(UAC_CUT, StrF(_T("Global\\UAC_CUT_%d"), m_processId));

	AddEvent(UAC_EXIT, StrF(_T("Global\\UAC_EXIT_%d"), m_processId));

	m_waitTimeout = 30000;
}

CUAC_Thread::~CUAC_Thread(void)
{
}


void CUAC_Thread::OnTimeOut(void *param)
{
	bool close = false;
	DWORD exitCode = 0;

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, m_processId);
	if(hProcess == NULL)
	{
		close = true;
	}
	else
	{
		if(GetExitCodeProcess(hProcess, &exitCode) == 0)
		{
			close = true;
		}
		else if(exitCode != STILL_ACTIVE)
		{
			close = true;
		}
	}

	if(close)
	{
		Log(StrF(_T("Found parent process id (%d) is not running, Exit Code %d closing uac aware app"), m_processId, exitCode));
		this->CancelThread();
	}

	CloseHandle(hProcess);
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
	case UAC_COPY:
		theApp.m_activeWnd.SendCopy();
		break; 
	case UAC_CUT:
		theApp.m_activeWnd.SendCut();
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
	case UAC_COPY:
		return _T("COPY Elevated");
	case UAC_CUT:
		return _T("Cut Elevated");
	case UAC_EXIT:
		return _T("Save Startup Elevated");
	}

	return _T("");
}

bool CUAC_Thread::UACPaste()
{	
	bool ret = StartProcess();

	FirePaste();

	return ret;
}

bool CUAC_Thread::UACCopy()
{	
	bool ret = StartProcess();

	FireCopy();

	return ret;
}

bool CUAC_Thread::UACCut()
{	
	bool ret = StartProcess();

	FireCut();

	return ret;
}

bool CUAC_Thread::StartProcess()
{
	bool ret = true;
	CString mutexName;
	mutexName.Format(_T("DittoAdminPaste_%d"), GetCurrentProcessId());

	HANDLE mutex = CreateMutex(NULL, FALSE, mutexName);
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
		Log(_T("Paste uac admin exe is already running just signalling paste"));
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
				Log(_T("Failed to startup paste as admin app, we are not pasting using admin app"));
				ret = false;
			}
			else
			{
				Log(_T("Startup up ditto paste as admin app, this will send ctrl-v to the admin app"));
			}
		}
	}

	CloseHandle(mutex);

	return ret;
}