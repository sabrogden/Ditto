#include <windows.h>
#include <assert.h>
#include "focusdll.h"

BYTE GetKeyEventType(LPARAM lParam);

#pragma data_seg (".shared")
HHOOK hHook=NULL;       //
HWND hFocusWnd=NULL;    // window that last gained the focus
HWND hNotifyWnd=NULL;   // window to send message to when focus changes
UINT uMessage=0;        // wm_message to send in the above case
HHOOK g_hKeyboardHook = NULL;       //
bool g_CaptureKeys = FALSE;
UINT g_uKeyboardMessage = 0;
HWND g_hKeyboardNotifyWnd = NULL;
#pragma data_seg ()
#pragma comment(linker, "/SECTION:.shared,RWS")

HINSTANCE hDllInst;

#define KH_KEY_DOWN				0x01 // Key-down event
#define KH_KEY_UP				0x02 // Key-up event
#define KH_KEY_REPEAT			0x04 // Key-repeat event, the key is held down for long enough

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

LRESULT CALLBACK KeyboardProc(INT nCode, WPARAM wParam, LPARAM lParam)
{	
	if (nCode != HC_ACTION)
		return ::CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);

	const BYTE VKCODE = (BYTE)wParam;
	const BYTE KEYEVENT = GetKeyEventType(lParam);	

	if(g_CaptureKeys && KEYEVENT == KH_KEY_DOWN)
	{
		if(g_hKeyboardNotifyWnd)
		{
			PostMessage(g_hKeyboardNotifyWnd, g_uKeyboardMessage, wParam, lParam);

			//Don't send this message on
			return 1;
		}
	}


	return ::CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

BYTE GetKeyEventType(LPARAM lParam)
{
	// Reference: WM_KEYDOWN on MSDN
	if (lParam & 0x80000000) // check bit 31 for up/down
	{
		return KH_KEY_UP;
	}
	else
	{
		if (lParam & 0x40000000) // check bit 30 for previous up/down
			return KH_KEY_REPEAT; // It was pressed down before this key-down event, so it's a key-repeat for sure
		else
			return KH_KEY_DOWN;
	}
}


DLLEXPORT DWORD WINAPI MonitorFocusChanges(HWND hWnd, UINT message)
{    
    if(hHook)
	{
		UnhookWindowsHookEx(hHook);
	}

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

DLLEXPORT DWORD WINAPI MonitorKeyboardChanges(HWND hWnd,UINT message)
{
	if(g_hKeyboardHook)
	{
		UnhookWindowsHookEx(g_hKeyboardHook);
	}

	g_hKeyboardHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, hDllInst, 0);

	g_uKeyboardMessage = message;
	g_hKeyboardNotifyWnd = hWnd;

	return TRUE;
}

DLLEXPORT DWORD WINAPI StopMonitoringKeyboardChanges()
{
	if(g_hKeyboardHook) 
		UnhookWindowsHookEx(g_hKeyboardHook);

    g_hKeyboardHook = NULL;
    hFocusWnd = NULL;

	return TRUE;
}

DLLEXPORT HWND WINAPI GetCurrentFocus()
{
    return hFocusWnd;
}

DLLEXPORT void  WINAPI SetCaptureKeys(bool bCapture)
{
	g_CaptureKeys = bCapture;
}

DLLEXPORT bool  WINAPI GetCaptureKeys()
{
	return g_CaptureKeys;
}


