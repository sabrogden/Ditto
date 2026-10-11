#include "stdafx.h"
#include "CP_Main.h"
#include "ClipboardCarouselInput.h"
#include "Options.h"

namespace { const UINT HOOK_UPDATE = WM_APP + 222; }
CClipboardCarouselInput* CClipboardCarouselInput::s_owner = NULL;

CClipboardCarouselInput::CClipboardCarouselInput()
	: m_owner(NULL), m_preview(NULL), m_atom(0), m_registered(false), m_enabled(false),
	m_suspended(false), m_active(false), m_generation(0), m_clickedWindow(NULL), m_clickX(0), m_clickY(0),
	m_swallowedButtons(0), m_swallowedKeyCount(0), m_outsideDown(false), m_downRoot(NULL),
	m_downGeneration(0), m_wheelRemainder(0), m_thread(NULL), m_threadId(0), m_ready(NULL),
	m_keyboardWanted(false), m_hooksReady(false), m_mouseHook(NULL), m_keyboardHook(NULL)
{
	ZeroMemory(m_swallowedKeys, sizeof(m_swallowedKeys));
}
CClipboardCarouselInput::~CClipboardCarouselInput() { Shutdown(); }

bool CClipboardCarouselInput::Initialize(HWND owner, HWND preview)
{
	m_owner = owner;
	m_preview = preview;
	m_atom = GlobalAddAtom(_T("Ditto_SlimCarousel_Activation"));
	CString error;
	return m_atom && Resume(error);
}
void CClipboardCarouselInput::UnregisterActivation()
{
	if(m_registered) UnregisterHotKey(m_owner,m_atom);
	m_registered = false;
}
void CClipboardCarouselInput::Shutdown()
{
	m_enabled = false;
	EndSession();
	UnregisterActivation();
	StopHooks();
	if(m_atom) GlobalDeleteAtom(m_atom);
	m_atom = 0;
	m_owner = m_preview = NULL;
}
void CClipboardCarouselInput::Suspend()
{
	m_suspended = true;
	EndSession();
	UnregisterActivation();
	StopHooks();
}
bool CClipboardCarouselInput::Resume(CString& error)
{
	UnregisterActivation();
	StopHooks();
	m_binding = ClipboardCarouselBinding::Decode(CGetSetOptions::GetCarouselActivation());
	m_enabled = CGetSetOptions::GetCarouselEnabled() != FALSE;
	m_suspended = false;
	if(!m_enabled) return true;
	if(!Validate(true,m_binding,error))
	{
		m_enabled = false;
		return false;
	}
	if(m_binding.IsKeyboard())
	{
		m_registered = RegisterHotKey(m_owner,m_atom,
			CHotKey::GetModifier(m_binding.modifiers) | MOD_NOREPEAT,m_binding.virtualKey) != FALSE;
		if(!m_registered)
		{
			error = theApp.m_Language.GetString("ClipboardCarouselShortcutUnavailable", "The carousel activation shortcut is unavailable.");
			m_enabled = false;
			return false;
		}
		return true;
	}
	if(UpdateHooks(false,error)) return true;
	m_enabled = false;
	return false;
}
bool CClipboardCarouselInput::Validate(bool enabled, const ClipboardCarouselBinding& binding, CString& error)
{
	if(!enabled) return true;
	if(!binding.IsAssigned() || (binding.IsKeyboard() &&
		(binding.virtualKey == VK_CONTROL || binding.virtualKey == VK_SHIFT || binding.virtualKey == VK_MENU ||
		(binding.modifiers == 0 && (binding.virtualKey < VK_F1 || binding.virtualKey > VK_F24)))))
	{
		error = theApp.m_Language.GetString("ClipboardCarouselAssignActivation",
			"Assign a mouse button, a keyboard shortcut with modifiers, or a function key before enabling Clipboard Carousel.");
		return false;
	}
	// Options suspends our own registration, so this also finds conflicts with other Ditto shortcuts.
	if(binding.IsKeyboard())
	{
		ATOM probe = GlobalAddAtom(_T("Ditto_SlimCarousel_Probe"));
		bool available = probe && RegisterHotKey(NULL,probe,
			CHotKey::GetModifier(binding.modifiers) | MOD_NOREPEAT,binding.virtualKey);
		if(available) UnregisterHotKey(NULL,probe);
		if(probe) GlobalDeleteAtom(probe);
		if(!available)
		{
			error = theApp.m_Language.GetString("ClipboardCarouselShortcutUnavailable", "The carousel activation shortcut is unavailable.");
			return false;
		}
	}
	return true;
}
bool CClipboardCarouselInput::HandleHotKey(WPARAM id)
{
	if(id != m_atom) return false;
	if(m_enabled && !m_suspended) PostCommand(ClipboardCarouselCommand::Activate);
	return true;
}
bool CClipboardCarouselInput::BeginSession(CString& error)
{
	m_wheelRemainder = 0;
	m_generation++;
	m_active = true;
	if(UpdateHooks(true,error)) return true;
	EndSession();
	if(m_binding.IsMouse())
	{
		CString idleError;
		if(!UpdateHooks(false,idleError)) m_enabled = false;
	}
	return false;
}
void CClipboardCarouselInput::EndSession()
{
	m_active = false;
	m_generation++;
}
void CClipboardCarouselInput::Tick()
{
	// Keep paired key/button releases suppressed before removing a session hook.
	if(!m_active && m_swallowedButtons == 0 && m_swallowedKeyCount == 0)
	{
		if(!m_enabled || m_suspended || !m_binding.IsMouse())
			StopHooks();
		else if(m_keyboardWanted)
		{
			CString error;
			UpdateHooks(false,error);
		}
	}
}
bool CClipboardCarouselInput::UpdateHooks(bool keyboard, CString& error)
{
	m_keyboardWanted = keyboard;
	if(!m_thread)
	{
		m_ready = CreateEvent(NULL,TRUE,FALSE,NULL);
		if(m_ready)
		{
			s_owner = this;
			m_thread = CreateThread(NULL,0,HookThreadProc,this,0,&m_threadId);
		}
	}
	else
	{
		ResetEvent(m_ready);
		PostThreadMessage(m_threadId,HOOK_UPDATE,0,0);
	}
	if(m_thread && WaitForSingleObject(m_ready,2000) == WAIT_OBJECT_0 && m_hooksReady)
		return true;
	error = theApp.m_Language.GetString("ClipboardCarouselHookInstallError", "Unable to start carousel input handling.");
	StopHooks();
	return false;
}
void CClipboardCarouselInput::StopHooks()
{
	if(m_thread)
	{
		if(!PostThreadMessage(m_threadId,WM_QUIT,0,0))
		{
			HANDLE startup[] = { m_thread,m_ready };
			if(WaitForMultipleObjects(2,startup,FALSE,INFINITE) == WAIT_OBJECT_0 + 1)
				PostThreadMessage(m_threadId,WM_QUIT,0,0);
		}
		WaitForSingleObject(m_thread,INFINITE);
		CloseHandle(m_thread);
		m_thread = NULL;
	}
	if(m_ready) CloseHandle(m_ready);
	m_ready = NULL;
	m_threadId = 0;
	m_hooksReady = false;
	m_keyboardWanted = false;
	m_swallowedButtons = 0;
	m_swallowedKeyCount = 0;
	ZeroMemory(m_swallowedKeys,sizeof(m_swallowedKeys));
	m_outsideDown = false;
	m_wheelRemainder = 0;
	if(s_owner == this) s_owner = NULL;
}
DWORD WINAPI CClipboardCarouselInput::HookThreadProc(LPVOID parameter)
{
	return ((CClipboardCarouselInput*)parameter)->HookThread();
}
DWORD CClipboardCarouselInput::HookThread()
{
	MSG message;
	PeekMessage(&message,NULL,WM_USER,WM_USER,PM_NOREMOVE);
	m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL,MouseHookProc,AfxGetInstanceHandle(),0);
	auto updateKeyboard = [&]()
	{
		if(m_keyboardWanted && !m_keyboardHook)
			m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,KeyboardHookProc,AfxGetInstanceHandle(),0);
		else if(!m_keyboardWanted && m_keyboardHook)
		{
			UnhookWindowsHookEx(m_keyboardHook);
			m_keyboardHook = NULL;
		}
		m_hooksReady = m_mouseHook && (!m_keyboardWanted || m_keyboardHook);
		SetEvent(m_ready);
	};
	updateKeyboard();
	while(m_hooksReady && GetMessage(&message,NULL,0,0) > 0)
	{
		if(message.message == HOOK_UPDATE) updateKeyboard();
		else { TranslateMessage(&message); DispatchMessage(&message); }
	}
	if(m_keyboardHook) UnhookWindowsHookEx(m_keyboardHook);
	if(m_mouseHook) UnhookWindowsHookEx(m_mouseHook);
	m_mouseHook = m_keyboardHook = NULL;
	m_hooksReady = false;
	return 0;
}
LRESULT CALLBACK CClipboardCarouselInput::MouseHookProc(int code, WPARAM message, LPARAM data)
{
	if(code == HC_ACTION && s_owner && s_owner->MouseEvent(message,*(MSLLHOOKSTRUCT*)data))
		return 1;
	return CallNextHookEx(NULL,code,message,data);
}
LRESULT CALLBACK CClipboardCarouselInput::KeyboardHookProc(int code, WPARAM message, LPARAM data)
{
	if(code == HC_ACTION && s_owner && s_owner->KeyboardEvent(message,*(KBDLLHOOKSTRUCT*)data))
		return 1;
	return CallNextHookEx(NULL,code,message,data);
}
BYTE CClipboardCarouselInput::Modifiers() const
{
	BYTE result = 0;
	if(GetAsyncKeyState(VK_CONTROL) & 0x8000) result |= HOTKEYF_CONTROL;
	if(GetAsyncKeyState(VK_SHIFT) & 0x8000) result |= HOTKEYF_SHIFT;
	if(GetAsyncKeyState(VK_MENU) & 0x8000) result |= HOTKEYF_ALT;
	if((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000) result |= HOTKEYF_EXT;
	return result;
}
bool CClipboardCarouselInput::InsidePreview(POINT point) const
{
	HWND window = WindowFromPoint(point);
	return window == m_preview || IsChild(m_preview,window);
}
bool CClipboardCarouselInput::PostCommand(ClipboardCarouselCommand command)
{
	return PostMessage(m_owner,WM_CLIPBOARD_CAROUSEL_INPUT,(WPARAM)command,
		command == ClipboardCarouselCommand::Activate ? 0 : (LPARAM)m_generation.load()) != FALSE;
}
LRESULT CClipboardCarouselInput::MouseEvent(WPARAM message, const MSLLHOOKSTRUCT& event)
{
	if(event.flags & LLMHF_INJECTED) return 0;
	ClipboardCarouselTrigger trigger = ClipboardCarouselTrigger::None;
	bool down = false, up = false;
	if(message == WM_MBUTTONDOWN || message == WM_MBUTTONUP)
	{
		trigger = ClipboardCarouselTrigger::MiddleButton;
		down = message == WM_MBUTTONDOWN; up = !down;
	}
	else if(message == WM_XBUTTONDOWN || message == WM_XBUTTONUP)
	{
		trigger = HIWORD(event.mouseData) == XBUTTON1 ? ClipboardCarouselTrigger::XButton1 : ClipboardCarouselTrigger::XButton2;
		down = message == WM_XBUTTONDOWN; up = !down;
	}
	UINT mask = trigger == ClipboardCarouselTrigger::None ? 0 : 1U << (int)trigger;
	if(up && mask && (m_swallowedButtons.load() & mask))
	{
		m_swallowedButtons.fetch_and(~mask);
		return 1;
	}
	if(!m_enabled || m_suspended) return 0;
	if(down && trigger == m_binding.trigger && Modifiers() == m_binding.modifiers)
	{
		m_swallowedButtons.fetch_or(mask);
		if(!m_active) PostCommand(ClipboardCarouselCommand::Activate);
		return 1;
	}
	if(!m_active) return 0;
	if(message == WM_MOUSEWHEEL)
	{
		m_wheelRemainder += (SHORT)HIWORD(event.mouseData);
		while(abs(m_wheelRemainder) >= WHEEL_DELTA)
		{
			bool upward = m_wheelRemainder > 0;
			PostCommand(upward ? ClipboardCarouselCommand::Newer : ClipboardCarouselCommand::Older);
			m_wheelRemainder += upward ? -WHEEL_DELTA : WHEEL_DELTA;
		}
		return 1;
	}
	if(message == WM_LBUTTONDOWN)
	{
		m_outsideDown = !InsidePreview(event.pt);
		m_downRoot = GetAncestor(WindowFromPoint(event.pt),GA_ROOT);
		m_downGeneration = m_generation;
	}
	else if(message == WM_LBUTTONUP && m_outsideDown)
	{
		m_outsideDown = false;
		HWND clicked = WindowFromPoint(event.pt);
		if(m_downGeneration == m_generation && !InsidePreview(event.pt) && GetAncestor(clicked,GA_ROOT) == m_downRoot)
		{
			m_clickedWindow = clicked;
			m_clickX = event.pt.x; m_clickY = event.pt.y;
			PostCommand(ClipboardCarouselCommand::ClickPaste);
		}
	}
	else if((message == WM_RBUTTONDOWN || down) && !InsidePreview(event.pt))
		PostCommand(ClipboardCarouselCommand::ClickCancel);
	return 0;
}
LRESULT CClipboardCarouselInput::KeyboardEvent(WPARAM message, const KBDLLHOOKSTRUCT& event)
{
	if((event.flags & LLKHF_INJECTED) || event.vkCode >= 256) return 0;
	bool down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
	if(message == WM_KEYUP || message == WM_SYSKEYUP)
	{
		if(m_swallowedKeys[event.vkCode])
		{
			m_swallowedKeys[event.vkCode] = false;
			m_swallowedKeyCount--;
			return 1;
		}
		return 0;
	}
	if(!down) return 0;
	if(m_swallowedKeys[event.vkCode])
	{
		// Drain repeats as well as key-up after closing, without dispatching
		// another command or forwarding a held Enter/Escape to the target.
		if(m_active && !m_suspended && Modifiers() == 0 && (event.vkCode == VK_UP || event.vkCode == VK_DOWN))
			PostCommand(event.vkCode == VK_UP ? ClipboardCarouselCommand::Newer : ClipboardCarouselCommand::Older);
		return 1;
	}
	if(!m_active || m_suspended) return 0;
	ClipboardCarouselCommand command;
	if(event.vkCode == VK_ESCAPE) command = ClipboardCarouselCommand::Cancel;
	else if(event.vkCode == VK_RETURN) command = ClipboardCarouselCommand::Confirm;
	else if(Modifiers() == 0 && event.vkCode == VK_UP)
		command = ClipboardCarouselCommand::Newer;
	else if(Modifiers() == 0 && event.vkCode == VK_DOWN)
		command = ClipboardCarouselCommand::Older;
	else return 0;
	m_swallowedKeys[event.vkCode] = true;
	m_swallowedKeyCount++;
	PostCommand(command);
	return 1;
}
