#pragma once

#define DEVICE_NOTIFY_CALLBACK 2

typedef
	ULONG
	DEVICE_NOTIFY_CALLBACK_ROUTINE (
	_In_opt_ PVOID Context,
	_In_ ULONG Type,
	_In_ PVOID Setting
	);
typedef DEVICE_NOTIFY_CALLBACK_ROUTINE* PDEVICE_NOTIFY_CALLBACK_ROUTINE;

typedef struct _DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS {
	PDEVICE_NOTIFY_CALLBACK_ROUTINE Callback;
	PVOID Context;
} DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS, *PDEVICE_NOTIFY_SUBSCRIBE_PARAMETERS;

typedef  PVOID           HPOWERNOTIFY;
typedef  HPOWERNOTIFY   *PHPOWERNOTIFY;

class CPowerManager
{
public:
	CPowerManager();
	~CPowerManager(void);

	void Start(HWND hWnd);
	void CPowerManager::Close();

protected:
	HPOWERNOTIFY m_registrationHandle;
};

