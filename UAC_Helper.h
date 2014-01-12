#pragma once
class CUAC_Helper
{
public:
	CUAC_Helper(void);
	~CUAC_Helper(void);

	static bool PasteAsAdmin(HWND hWnd);

	static bool AmIRunningElevated();
	static bool AreTheyRunningElevated(HWND hWnd);
	static bool RunningElevated(HANDLE h, bool defaultValue);
};

