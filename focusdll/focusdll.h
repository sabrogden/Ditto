#ifdef __cplusplus
extern "C" {
#endif


#ifdef FOCUS_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

DLLEXPORT DWORD WINAPI MonitorFocusChanges(HWND hWnd,UINT message);
DLLEXPORT DWORD WINAPI StopMonitoringFocusChanges();

DLLEXPORT DWORD WINAPI MonitorKeyboardChanges(HWND hWnd,UINT message);
DLLEXPORT DWORD WINAPI StopMonitoringKeyboardChanges();

DLLEXPORT HWND  WINAPI GetCurrentFocus();

DLLEXPORT void  WINAPI SetCaptureKeys(bool bCapture);
DLLEXPORT bool  WINAPI GetCaptureKeys();


#ifdef __cplusplus
}
#endif
