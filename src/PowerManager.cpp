#include "stdafx.h"
#include "PowerManager.h"
#include "Misc.h"

static HWND s_notifyHwnd;
static ULONG PowerChanged(PVOID Context, ULONG Type, PVOID Setting);

ULONG PowerChanged(PVOID Context, ULONG Type, PVOID Setting)
{
	//a
	//b
	//c
	CString cs;
	cs.Format(_T("PowerChanged Type %d"), Type);
	Log(cs);

	if(Type == PBT_APMRESUMEAUTOMATIC)
	{
		//had reports of the main window not showing clips after resuming (report was from a vmware vm), catch the resuming callback from windows
		//and close and reopen the database
		Log(_T("windows is RESUMING, sending message to main window to close and reopen the database/qpastewnd"));
		::PostMessage(s_notifyHwnd, WM_REOPEN_DATABASE, 0, 0);
	}

	return 0;
}



CPowerManager::CPowerManager()
{
	m_registrationHandle = 0;
	m_hPowrProf = NULL;
}


CPowerManager::~CPowerManager(void)
{
	Close();
}


void CPowerManager::Start(HWND hWnd)
{
	s_notifyHwnd = hWnd;

	if (m_hPowrProf == NULL)
	{
		m_hPowrProf = LoadLibrary(_T("powrprof.dll"));
	}

	if (m_hPowrProf != NULL && m_registrationHandle == 0)
	{
		DWORD (_stdcall*PowerRegisterSuspendResumeNotification)(_In_ DWORD, _In_ HANDLE, _Out_ PHPOWERNOTIFY);
		PowerRegisterSuspendResumeNotification = (DWORD(_stdcall*)(_In_ DWORD, _In_ HANDLE, _Out_ PHPOWERNOTIFY))GetProcAddress(m_hPowrProf, "PowerRegisterSuspendResumeNotification");
		if (PowerRegisterSuspendResumeNotification)
		{
			static _DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS testCallback = {PowerChanged, nullptr};

			PowerRegisterSuspendResumeNotification(DEVICE_NOTIFY_CALLBACK, &testCallback, &m_registrationHandle); 
		}
	}
}

void CPowerManager::Close()
{	
	if (m_hPowrProf != NULL)
	{
		if (m_registrationHandle != 0)
		{
			DWORD (_stdcall*PowerUnregisterSuspendResumeNotification)(_Inout_ HPOWERNOTIFY);
			PowerUnregisterSuspendResumeNotification = (DWORD(_stdcall*)(_Inout_ HPOWERNOTIFY))GetProcAddress(m_hPowrProf, "PowerUnregisterSuspendResumeNotification");
			if (PowerUnregisterSuspendResumeNotification)
			{
				PowerUnregisterSuspendResumeNotification(m_registrationHandle); 
			}
			m_registrationHandle = 0;
		}

		::FreeLibrary(m_hPowrProf);
		m_hPowrProf = NULL;
	}
}
