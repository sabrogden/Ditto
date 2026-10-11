#pragma once

#include "HotKeys.h"

enum class ClipboardCarouselTrigger : BYTE { None, Keyboard, MiddleButton, XButton1, XButton2 };
enum class ClipboardCarouselCommand : WPARAM { Activate = 1, Older, Newer, Confirm, Cancel, ClickCancel, ClickPaste };

struct ClipboardCarouselBinding
{
	ClipboardCarouselTrigger trigger = ClipboardCarouselTrigger::None;
	BYTE virtualKey = 0;
	BYTE modifiers = 0;

	bool IsKeyboard() const { return trigger == ClipboardCarouselTrigger::Keyboard; }
	bool IsMouse() const { return trigger >= ClipboardCarouselTrigger::MiddleButton && trigger <= ClipboardCarouselTrigger::XButton2; }
	bool IsAssigned() const { return IsMouse() || (IsKeyboard() && virtualKey != 0); }
	DWORD Encode() const { return (DWORD)trigger | ((DWORD)virtualKey << 8) | ((DWORD)modifiers << 16); }
	static ClipboardCarouselBinding Decode(DWORD value)
	{
		ClipboardCarouselBinding result;
		result.trigger = (ClipboardCarouselTrigger)(value & 0xFF);
		result.virtualKey = (BYTE)(value >> 8);
		result.modifiers = (BYTE)(value >> 16) & (HOTKEYF_SHIFT | HOTKEYF_CONTROL | HOTKEYF_ALT);
		if(result.trigger > ClipboardCarouselTrigger::XButton2)
			return ClipboardCarouselBinding();
		return result;
	}
};

#define WM_CLIPBOARD_CAROUSEL_INPUT (WM_APP + 220)
#define WM_CLIPBOARD_CAROUSEL_PREVIEW_READY (WM_APP + 221)
