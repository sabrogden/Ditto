#include <windows.h>
#include <assert.h>
#include "focusdll.h"

#pragma data_seg (".shared")
HHOOK hHook=NULL;       //
HWND hFocusWnd=NULL;    // window that last gained the focus
HWND hNotifyWnd=NULL;   // window to send message to when focus changes
UINT uMessage=0;        // wm_message to send in the above case
#pragma data_seg ()
#pragma comment(linker, "/SECTION:.shared,RWS")

HINSTANCE hDllInst;

BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID lpReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:            
            hDllInst = hInstance;
        case DLL_THREAD_ATTACH:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_DETACH:
            break;
    }
    
    return TRUE;
}

static LRESULT WINAPI HookProc(int code, WPARAM wParam, LPARAM lParam)
{
    
    if(code == HCBT_SETFOCUS)
    {
        //Focus has changed, record new focus window
        hFocusWnd = (HWND)wParam;
        
        //And if notification requested, send a windows message.
        if(hNotifyWnd)
			PostMessage(hNotifyWnd, uMessage, wParam, lParam);
    }
 
    return CallNextHookEx(hHook,code,wParam,lParam);
}

DLLEXPORT DWORD WINAPI MonitorFocusChanges(HWND hWnd, UINT message)
{    
    if(hHook)
		UnhookWindowsHookEx(hHook);

    hHook = SetWindowsHookEx(WH_CBT,HookProc,hDllInst,0);

    hNotifyWnd = hWnd;
    uMessage = message;
    
    return TRUE;
}

DLLEXPORT DWORD WINAPI StopMonitoringFocusChanges()
{
    if(hHook) 
		UnhookWindowsHookEx(hHook);

    hHook = NULL;
    hFocusWnd = NULL;

    return TRUE;
}

DLLEXPORT HWND WINAPI GetCurrentFocus()
{
    return hFocusWnd;
}



