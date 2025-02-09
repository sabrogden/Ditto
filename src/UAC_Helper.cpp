#include "stdafx.h"
#include "UAC_Helper.h"
#include "Misc.h"


CUAC_Helper::CUAC_Helper(void)
{
}


CUAC_Helper::~CUAC_Helper(void)
{
}

bool CUAC_Helper::PasteAsAdmin(HWND hWnd)
{
	bool runningElevated = AmIRunningElevated();
	bool theirRunningElevated = AreTheyRunningElevated(hWnd);

	bool pasteAsAdmin = false;

	if (runningElevated == false &&
		theirRunningElevated)
	{
		pasteAsAdmin = true;
	}

	Log(StrF(_T("I'm running elevated: %d, They are running elevated: %d, PASTE AS ADMIN: %d"), runningElevated, theirRunningElevated, pasteAsAdmin));

	return pasteAsAdmin;
}

bool CUAC_Helper::AmIRunningElevated()
{
	bool ret = RunningElevated(GetCurrentProcess(), true);

	return ret;
}

bool CUAC_Helper::AreTheyRunningElevated(HWND hWnd)
{
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, pid);

	bool ret = RunningElevated(hProcess, true);

	return ret;
}

bool CUAC_Helper::RunningElevated(HANDLE hProcess, bool defaultValue)
{
	bool fIsElevated = defaultValue;
	DWORD dwError = ERROR_SUCCESS;
	HANDLE hToken = NULL;
	TOKEN_ELEVATION elevation;

	if (hProcess == NULL)
	{
		dwError = GetLastError();

		Log(StrF(_T("RunningElevated, initial Handle is NULL, Last Error: %d"), dwError));
	}
	else
	{
		if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
		{
			dwError = GetLastError();

			Log(StrF(_T("RunningElevated, OpenProcessToken failed, Last Error: %d"), dwError));
		}
		else
		{			
			DWORD dwSize;
			if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
			{
				// When the process is run on operating systems prior to Windows 
				// Vista, GetTokenInformation returns FALSE with the 
				// ERROR_INVALID_PARAMETER error code because TokenElevation is 
				// not supported on those operating systems.
				dwError = GetLastError();

				Log(StrF(_T("RunningElevated, GetTokenInformation failed, Last Error: %d"), dwError));
			}
			else
			{
				fIsElevated = elevation.TokenIsElevated == 1;
			}
		}
	}

	if (hProcess != NULL)
		CloseHandle(hProcess);
	if (hToken != NULL)
		CloseHandle(hToken);

	return fIsElevated;
}