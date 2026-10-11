#pragma once

#include "ClipboardCarouselBinding.h"
#include <atomic>

class CClipboardCarouselInput
{
public:
	CClipboardCarouselInput();
	~CClipboardCarouselInput();
	bool Initialize(HWND owner, HWND preview);
	void Shutdown();
	void Suspend();
	bool Resume(CString& error);
	bool HandleHotKey(WPARAM id);
	bool BeginSession(CString& error);
	void EndSession();
	void Tick();
	bool IsEnabled() const { return m_enabled; }
	UINT_PTR Generation() const { return m_generation; }
	HWND ClickedWindow() const { return m_clickedWindow; }
	POINT ClickedPoint() const { POINT point = { m_clickX.load(), m_clickY.load() }; return point; }
	static bool Validate(bool enabled, const ClipboardCarouselBinding& binding, CString& error);

private:
	bool UpdateHooks(bool keyboard, CString& error);
	void StopHooks();
	void UnregisterActivation();
	bool PostCommand(ClipboardCarouselCommand command);
	bool InsidePreview(POINT point) const;
	BYTE Modifiers() const;
	static DWORD WINAPI HookThreadProc(LPVOID parameter);
	DWORD HookThread();
	static LRESULT CALLBACK MouseHookProc(int code, WPARAM message, LPARAM data);
	static LRESULT CALLBACK KeyboardHookProc(int code, WPARAM message, LPARAM data);
	LRESULT MouseEvent(WPARAM message, const MSLLHOOKSTRUCT& event);
	LRESULT KeyboardEvent(WPARAM message, const KBDLLHOOKSTRUCT& event);

	HWND m_owner;
	HWND m_preview;
	ATOM m_atom;
	bool m_registered;
	ClipboardCarouselBinding m_binding;
	std::atomic<bool> m_enabled;
	std::atomic<bool> m_suspended;
	std::atomic<bool> m_active;
	std::atomic<UINT_PTR> m_generation;
	std::atomic<HWND> m_clickedWindow;
	std::atomic<LONG> m_clickX, m_clickY;
	std::atomic<UINT> m_swallowedButtons;
	std::atomic<int> m_swallowedKeyCount;
	bool m_swallowedKeys[256];
	bool m_outsideDown;
	HWND m_downRoot;
	UINT_PTR m_downGeneration;
	std::atomic<int> m_wheelRemainder;

	HANDLE m_thread;
	DWORD m_threadId;
	HANDLE m_ready;
	std::atomic<bool> m_keyboardWanted;
	std::atomic<bool> m_hooksReady;
	HHOOK m_mouseHook;
	HHOOK m_keyboardHook;
	static CClipboardCarouselInput* s_owner;
};
