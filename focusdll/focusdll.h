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

DLLEXPORT HWND  WINAPI GetCurrentFocus();


#ifdef __cplusplus
}
#endif
