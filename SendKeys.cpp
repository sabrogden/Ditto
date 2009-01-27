#include "stdafx.h"
#include "sendkeys.h"

/* 
* ----------------------------------------------------------------------------- 
* Copyright (c) 2004 lallous <lallousx86@yahoo.com>
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
* ----------------------------------------------------------------------------- 


The Original SendKeys copyright info
---------------------------------------
SendKeys (sndkeys32.pas) routine for 32-bit Delphi.
Written by Ken Henderson
Copyright (c) 1995 Ken Henderson <khen@compuserve.com>

History
----------
04/19/2004
  * Initial version development
04/21/2004
  * Added number of times specifier to special keys
  * Added {BEEP X Y}
  * Added {APPACTIVATE WindowTitle}
  * Added CarryDelay() and now delay works properly with all keys
  * Added SetDelay() method
  * Fixed code in AppActivate that allowed to pass both NULL windowTitle/windowClass

05/21/2004
  * Fixed a bug in StringToVKey() that caused the search for RIGHTPAREN to be matched as RIGHT
  * Adjusted code so it compiles w/ VC6
05/24/2004
  * Added unicode support

Todo
-------
* perhaps add mousecontrol: mousemove+mouse clicks
* allow sending of normal keys multiple times as: {a 10}

*/

const WORD CSendKeys::VKKEYSCANSHIFTON = 0x01;
const WORD CSendKeys::VKKEYSCANCTRLON  = 0x02;
const WORD CSendKeys::VKKEYSCANALTON   = 0x04;
const WORD CSendKeys::INVALIDKEY       = 0xFFFF;

const BYTE CSendKeys::ExtendedVKeys[MaxExtendedVKeys] =
{
    VK_UP, 
    VK_DOWN,
    VK_LEFT,
    VK_RIGHT,
    VK_HOME,
    VK_END,
    VK_PRIOR, // PgUp
    VK_NEXT,  //  PgDn
    VK_INSERT,
    VK_DELETE
};

CSendKeys::CSendKeys()
{
  m_nDelayNow = m_nDelayAlways = 0;
}

// Delphi port regexps:
// ---------------------
// search: .+Name:'([^']+)'.+vkey:([^\)]+)\)
// replace: {"\1", \2}
//
// **If you add to this list, you must be sure to keep it sorted alphabetically
// by Name because a binary search routine is used to scan it.**
//
CSendKeys::key_desc_t CSendKeys::KeyNames[CSendKeys::MaxSendKeysRecs] = 
{
  {_T("ADD"), VK_ADD}, 
  {_T("APPS"), VK_APPS},
  {_T("AT"), '@', true},
  {_T("BACKSPACE"), VK_BACK},
  {_T("BKSP"), VK_BACK},
  {_T("BREAK"), VK_CANCEL},
  {_T("BS"), VK_BACK},
  {_T("CAPSLOCK"), VK_CAPITAL},
  {_T("CARET"), '^', true},
  {_T("CLEAR"), VK_CLEAR},
  {_T("DECIMAL"), VK_DECIMAL}, 
  {_T("DEL"), VK_DELETE},
  {_T("DELETE"), VK_DELETE},
  {_T("DIVIDE"), VK_DIVIDE}, 
  {_T("DOWN"), VK_DOWN},
  {_T("END"), VK_END},
  {_T("ENTER"), VK_RETURN},
  {_T("ESC"), VK_ESCAPE},
  {_T("ESCAPE"), VK_ESCAPE},
  {_T("F1"), VK_F1},
  {_T("F10"), VK_F10},
  {_T("F11"), VK_F11},
  {_T("F12"), VK_F12},
  {_T("F13"), VK_F13},
  {_T("F14"), VK_F14},
  {_T("F15"), VK_F15},
  {_T("F16"), VK_F16},
  {_T("F2"), VK_F2},
  {_T("F3"), VK_F3},
  {_T("F4"), VK_F4},
  {_T("F5"), VK_F5},
  {_T("F6"), VK_F6},
  {_T("F7"), VK_F7},
  {_T("F8"), VK_F8},
  {_T("F9"), VK_F9},
  {_T("HELP"), VK_HELP},
  {_T("HOME"), VK_HOME},
  {_T("INS"), VK_INSERT},
  {_T("LEFT"), VK_LEFT},
  {_T("LEFTBRACE"), '{', true},
  {_T("LEFTPAREN"), '(', true},
  {_T("LWIN"), VK_LWIN},
  {_T("MULTIPLY"), VK_MULTIPLY}, 
  {_T("NUMLOCK"), VK_NUMLOCK},
  {_T("NUMPAD0"), VK_NUMPAD0}, 
  {_T("NUMPAD1"), VK_NUMPAD1}, 
  {_T("NUMPAD2"), VK_NUMPAD2}, 
  {_T("NUMPAD3"), VK_NUMPAD3}, 
  {_T("NUMPAD4"), VK_NUMPAD4}, 
  {_T("NUMPAD5"), VK_NUMPAD5}, 
  {_T("NUMPAD6"), VK_NUMPAD6}, 
  {_T("NUMPAD7"), VK_NUMPAD7}, 
  {_T("NUMPAD8"), VK_NUMPAD8}, 
  {_T("NUMPAD9"), VK_NUMPAD9}, 
  {_T("PERCENT"), '%', true},
  {_T("PGDN"), VK_NEXT},
  {_T("PGUP"), VK_PRIOR},
  {_T("PLUS"), '+', true},
  {_T("PRTSC"), VK_PRINT},
  {_T("RIGHT"), VK_RIGHT},
  {_T("RIGHTBRACE"), '}', true},
  {_T("RIGHTPAREN"), ')', true},
  {_T("RWIN"), VK_RWIN},
  {_T("SCROLL"), VK_SCROLL},
  {_T("SEPARATOR"), VK_SEPARATOR}, 
  {_T("SNAPSHOT"), VK_SNAPSHOT},
  {_T("SUBTRACT"), VK_SUBTRACT}, 
  {_T("TAB"), VK_TAB},
  {_T("TILDE"), '~', true}, 
  {_T("UP"), VK_UP},
  {_T("WIN"), VK_LWIN}
};


// calls keybd_event() and waits, if needed, till the sent input is processed
void CSendKeys::KeyboardEvent(BYTE VKey, BYTE ScanCode, LONG Flags)
{
  MSG KeyboardMsg;

  keybd_event(VKey, ScanCode, Flags, 0);

  if (m_bWait)
  {
    while (::PeekMessage(&KeyboardMsg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
    {
      ::TranslateMessage(&KeyboardMsg);
      ::DispatchMessage(&KeyboardMsg);
    }
  }
}

// Checks whether the specified VKey is an extended key or not
bool CSendKeys::IsVkExtended(BYTE VKey)
{
  for (int i=0;i<MaxExtendedVKeys;i++)
  {
    if (ExtendedVKeys[i] == VKey)
      return true;
  }
  return false;
}

void CSendKeys::AllKeysUp()
{
	for(int key = 0; key < 256; key++)
	{
		//If the key is pressed, send a key up, having other keys down interferes with sending ctrl-v, -c and -x
		if(GetKeyState(key) & 0x8000)
		{
			SendKeyUp(key);
		}
	}
}

// Generates KEYUP
void CSendKeys::SendKeyUp(BYTE VKey)
{
  BYTE ScanCode = LOBYTE(::MapVirtualKey(VKey, 0));

  KeyboardEvent(VKey, 
                ScanCode, 
                KEYEVENTF_KEYUP | (IsVkExtended(VKey) ? KEYEVENTF_EXTENDEDKEY : 0));
}

void CSendKeys::SendKeyDown(BYTE VKey, WORD NumTimes, bool GenUpMsg, bool bDelay)
 {
  WORD Cnt = 0;
  BYTE ScanCode = 0;
  bool NumState = false;

  if (VKey == VK_NUMLOCK)
  {
    DWORD dwVersion = ::GetVersion();

    for (Cnt=1; Cnt<=NumTimes; Cnt++)
    {
      if (bDelay)
        CarryDelay();
      // snippet based on:
      // http://www.codeproject.com/cpp/togglekeys.asp
      if (dwVersion < 0x80000000)
      {
        ::keybd_event(VKey, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
        ::keybd_event(VKey, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
      }
      else
      {
        // Win98 and later
        if ( ((DWORD)(HIBYTE(LOWORD(dwVersion))) >= 10) )
        {
          // Define _WIN32_WINNT > 0x0400
          // to compile
          INPUT input[2] = {0};
          input[0].type = input[1].type = INPUT_KEYBOARD;
          input[0].ki.wVk = input[1].ki.wVk = VKey;
          input[1].ki.dwFlags = KEYEVENTF_KEYUP;
          ::SendInput(sizeof(input) / sizeof(INPUT), input, sizeof(INPUT));
        }
        // Win95
        else
        {
          KEYBOARDSTATE_t KeyboardState;
          NumState = GetKeyState(VK_NUMLOCK) & 1 ? true : false;
          GetKeyboardState(&KeyboardState[0]);
          if (NumState)
            KeyboardState[VK_NUMLOCK] &= ~1;
          else
            KeyboardState[VK_NUMLOCK] |= 1;

          SetKeyboardState(&KeyboardState[0]);
        }
      }
    }
    return;
  }

  // Get scancode
  ScanCode = LOBYTE(::MapVirtualKey(VKey, 0));

  // Send keys
  for (Cnt=1; Cnt<=NumTimes; Cnt++)
  {
    // Carry needed delay ?
    if (bDelay)
      CarryDelay();

    KeyboardEvent(VKey, ScanCode, IsVkExtended(VKey) ? KEYEVENTF_EXTENDEDKEY : 0);
    
	if(m_keyDowUpDelay > 0)
	{
		Sleep(m_keyDowUpDelay);
	}

	if (GenUpMsg)
      KeyboardEvent(VKey, ScanCode, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP);
  }
}

// Checks whether a bit is set
bool CSendKeys::BitSet(BYTE BitTable, UINT BitMask)
{
  return BitTable & BitMask ? true : false;
}

// Sends a single key
void CSendKeys::SendKey(WORD MKey, WORD NumTimes, bool GenDownMsg)
{
  // Send appropriate shift keys associated with the given VKey
  if (BitSet(HIBYTE(MKey), VKKEYSCANSHIFTON))
    SendKeyDown(VK_SHIFT, 1, false);

  if (BitSet(HIBYTE(MKey), VKKEYSCANCTRLON))
    SendKeyDown(VK_CONTROL, 1, false);

  if (BitSet(HIBYTE(MKey), VKKEYSCANALTON))
    SendKeyDown(VK_MENU, 1, false);

  // Send the actual VKey
  SendKeyDown(LOBYTE(MKey), NumTimes, GenDownMsg, true);

  // toggle up shift keys
  if (BitSet(HIBYTE(MKey), VKKEYSCANSHIFTON))
    SendKeyUp(VK_SHIFT);

  if (BitSet(HIBYTE(MKey), VKKEYSCANCTRLON))
    SendKeyUp(VK_CONTROL);

  if (BitSet(HIBYTE(MKey), VKKEYSCANALTON))
    SendKeyUp(VK_MENU);
}

// Implements a simple binary search to locate special key name strings
WORD CSendKeys::StringToVKey(LPCTSTR KeyString, int &idx)
{
  bool Found = false, Collided;
  int  Bottom = 0, 
       Top = MaxSendKeysRecs,
       Middle = (Bottom + Top) / 2;
  WORD retval = INVALIDKEY;

  idx    = -1;

  do
  {
    Collided = (Bottom == Middle) || (Top == Middle);
    int cmp = _tcsnicmp(KeyNames[Middle].keyName, KeyString, _tcslen(KeyString));
    if (cmp == 0)
    {
      Found = true;
      retval = KeyNames[Middle].VKey;
      idx    = Middle;
      break;
    }
    else
    {
      if (cmp < 0)
        Bottom = Middle;
      else
        Top = Middle;
      Middle = (Bottom + Top) / 2;
    }
  } while (!(Found || Collided));

  return retval;
}

// Releases all shift keys (keys that can be depressed while other keys are being pressed
// If we are in a modifier group this function does nothing
void CSendKeys::PopUpShiftKeys()
{
  if (!m_bUsingParens)
  {
    if (m_bShiftDown)
      SendKeyUp(VK_SHIFT);
    if (m_bControlDown)
      SendKeyUp(VK_CONTROL);
    if (m_bAltDown)
      SendKeyUp(VK_MENU);
    if (m_bWinDown)
      SendKeyUp(VK_LWIN);
    m_bWinDown = m_bShiftDown = m_bControlDown = m_bAltDown = false;
  }
}

// Sends a key string
bool CSendKeys::SendKeys(LPCTSTR KeysString, bool Wait)
{
  WORD MKey, NumTimes;
  TCHAR KeyString[300] = {0};
  bool retval  = false;
  int  keyIdx;

  LPTSTR pKey = (LPTSTR) KeysString;
  TCHAR  ch;

  m_bWait = Wait;

  m_bWinDown = m_bShiftDown = m_bControlDown = m_bAltDown = m_bUsingParens = false;

  while (ch = *pKey)
  {
    switch (ch)
    {
    // begin modifier group
    case _TXCHAR('('):
      m_bUsingParens = true;
      break;

    // end modifier group
    case _TXCHAR(')'):
      m_bUsingParens = false;
      PopUpShiftKeys(); // pop all shift keys when we finish a modifier group close
      break;

    // ALT key
    case _TXCHAR('%'):
      m_bAltDown = true;
      SendKeyDown(VK_MENU, 1, false);
      break;

    // SHIFT key
    case _TXCHAR('+'):
      m_bShiftDown = true;
      SendKeyDown(VK_SHIFT, 1, false);
      break;

    // CTRL key
    case _TXCHAR('^'):
      m_bControlDown = true;
      SendKeyDown(VK_CONTROL, 1, false);
      break;

    // WINKEY (Left-WinKey)
    case '@':
      m_bWinDown = true;
      SendKeyDown(VK_LWIN, 1, false);
      break;

    // enter
    case _TXCHAR('~'):
      SendKeyDown(VK_RETURN, 1, true);
      PopUpShiftKeys();
      break;

    // begin special keys
    case _TXCHAR('{'):
      {
        LPTSTR p = pKey+1; // skip past the beginning '{'
        size_t t;

        // find end of close
        while (*p && *p != _TXCHAR('}'))
          p++;

        t = p - pKey;
        // special key definition too big?
        if (t > sizeof(KeyString))
          return false;

        // Take this KeyString into local buffer
        _tcsncpy(KeyString, pKey+1, t);

        KeyString[t-1] = _TXCHAR('\0');
        keyIdx = -1;

        pKey += t; // skip to next keystring

        // Invalidate key
        MKey = INVALIDKEY;

        // sending arbitrary vkeys?
        if (_tcsnicmp(KeyString, _T("VKEY"), 4) == 0)
        {
          p = KeyString + 4;
          MKey = _ttoi(p);
        }
        else if (_tcsnicmp(KeyString, _T("BEEP"), 4) == 0)
        {
          p = KeyString + 4 + 1;
          LPTSTR p1 = p;
          DWORD frequency, delay;

          if ((p1 = _tcsstr(p, _T(" "))) != NULL)
          {
            *p1++ = _TXCHAR('\0');
            frequency = _ttoi(p);
            delay = _ttoi(p1);
            ::Beep(frequency, delay);
          }
        }
        // Should activate a window?
        else if (_tcsnicmp(KeyString, _T("APPACTIVATE"), 11) == 0)
        {
          p = KeyString + 11 + 1;
          AppActivate(p);
        }
        // want to send/set delay?
        else if (_tcsnicmp(KeyString, _T("DELAY"), 5) == 0)
        {
          // Advance to parameters
          p = KeyString + 5;
          // set "sleep factor"
          if (*p == _TXCHAR('='))
            m_nDelayAlways = _ttoi(p + 1); // Take number after the '=' character
          else
            // set "sleep now"
            m_nDelayNow = _ttoi(p);
        }
        // not command special keys, then process as keystring to VKey
        else
        {
          MKey = StringToVKey(KeyString, keyIdx);
          // Key found in table
          if (keyIdx != -1)
          {
            NumTimes = 1;

            // Does the key string have also count specifier?
            t = _tcslen(KeyNames[keyIdx].keyName);
            if (_tcslen(KeyString) > t)
            {
              p = KeyString + t;
              // Take the specified number of times
              NumTimes = _ttoi(p);
            }

            if (KeyNames[keyIdx].normalkey)
              MKey = ::VkKeyScan(KeyNames[keyIdx].VKey);
          }
        }

        // A valid key to send?
        if (MKey != INVALIDKEY)
        {
          SendKey(MKey, NumTimes, true);
          PopUpShiftKeys();
        }
      }
      break;

      // a normal key was pressed
    default:
      // Get the VKey from the key
      MKey = ::VkKeyScan(ch);
      SendKey(MKey, 1, true);
      PopUpShiftKeys();
    }
    pKey++;
  }

  m_bUsingParens = false;
  PopUpShiftKeys();
  return true;
}

bool CSendKeys::AppActivate(HWND wnd)
{
  if (wnd == NULL)
    return false;

  ::SendMessage(wnd, WM_SYSCOMMAND, SC_HOTKEY, (LPARAM) wnd);
  ::SendMessage(wnd, WM_SYSCOMMAND, SC_RESTORE, (LPARAM) wnd);
  
  ::ShowWindow(wnd, SW_SHOW);
  ::SetForegroundWindow(wnd);
  ::SetFocus(wnd);

  return true;
}

BOOL CALLBACK CSendKeys::enumwindowsProc(HWND hwnd, LPARAM lParam)
{
  enumwindow_t *t = (enumwindow_t *) lParam;

  LPTSTR wtitle = 0, wclass = 0, str = t->str;

  if (!*str)
    str++;
  else
  {
    wtitle = str;
    str += _tcslen(str) + 1;
  }

  if (*str)
    wclass = str;

  bool bMatch(false);

  if (wclass)
  {
    TCHAR szClass[300];
    if (::GetClassName(hwnd, szClass, sizeof(szClass)))
      bMatch |= (_tcsstr(szClass, wclass) != 0);
  }

  if (wtitle)
  {
    TCHAR szTitle[300];
    if (::GetWindowText(hwnd, szTitle, sizeof(szTitle)))
      bMatch |= (_tcsstr(szTitle, wtitle) != 0);
  }

  if (bMatch)
  {
    t->hwnd = hwnd;
    return false;
  }
  return true;
}

// Searchs and activates a window given its title or class name
bool CSendKeys::AppActivate(LPCTSTR WindowTitle, LPCTSTR WindowClass)
{
  HWND w;

  w = ::FindWindow(WindowClass, WindowTitle);
  if (w == NULL)
  {
    // Must specify at least a criteria
    if (WindowTitle == NULL && WindowClass == NULL)
      return false;

    // << Code to serialize the windowtitle/windowclass in order to send to EnumWindowProc()
    size_t l1(0), l2(0);
    if (WindowTitle)
      l1 = _tcslen(WindowTitle);
    if (WindowClass)
      l2 = _tcslen(WindowClass);

    LPTSTR titleclass = new TCHAR [l1 + l2 + 5];

    memset(titleclass, '\0', l1+l2+5);

    if (WindowTitle)
      _tcscpy(titleclass, WindowTitle);

    titleclass[l1] = 0;

    if (WindowClass)
      _tcscpy(titleclass+l1+1, WindowClass);

    // >>

    enumwindow_t t;

    t.hwnd = NULL;
    t.str  = titleclass;
    ::EnumWindows(enumwindowsProc, (LPARAM) & t);
    w = t.hwnd;
    delete [] titleclass;
  }

  if (w == NULL)
    return false;

  return AppActivate(w);
}

// Carries the required delay and clears the m_nDelaynow value
void CSendKeys::CarryDelay()
{
  // Should we delay once?
  if (!m_nDelayNow)
    // should we delay always?
    m_nDelayNow = m_nDelayAlways;

  // No delay specified?
  if (m_nDelayNow)
    ::Sleep(m_nDelayNow); //::Beep(100, m_nDelayNow);

  // clear SleepNow
  m_nDelayNow = 0;
}

/*
Test Binary search
void CSendKeys::test()
{
  WORD miss(0);
  for (int i=0;i<MaxSendKeysRecs;i++)
  {
    char *p = (char *)KeyNames[i].keyName;
    WORD v = StringToVKeyB(p);
    if (v == INVALIDKEY)
    {
      miss++;
    }
  }
}
*/

/*
Search in a linear manner
WORD CSendKeys::StringToVKey(const char *KeyString, int &idx)
{
for (int i=0;i<MaxSendKeysRecs;i++)
{
size_t len = strlen(KeyNames[i].keyName);
if (strnicmp(KeyNames[i].keyName, KeyString, len) == 0)
{
idx = i;
return KeyNames[i].VKey;
}
}
idx = -1;
return INVALIDKEY;
}
*/
