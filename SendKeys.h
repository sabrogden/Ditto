#ifndef __SENDKEYS_04192004__INC__
#define __SENDKEYS_04192004__INC__

#include <windows.h>
#include <tchar.h>
// Please see SendKeys.cpp for copyright and usage issues.

class CSendKeys
{
private:
  bool m_bWait, m_bUsingParens, m_bShiftDown, m_bAltDown, m_bControlDown, m_bWinDown;
  DWORD  m_nDelayAlways, m_nDelayNow;

  static BOOL CALLBACK enumwindowsProc(HWND hwnd, LPARAM lParam);
  void   CarryDelay();

  typedef BYTE KEYBOARDSTATE_t[256];
  struct enumwindow_t
  {
    LPTSTR str;
    HWND hwnd;
  };

  struct key_desc_t
  {
    LPCTSTR keyName;
    BYTE VKey;
    bool normalkey; // a normal character or a VKEY ?
  };

  enum
  {
    MaxSendKeysRecs  = 71,
    MaxExtendedVKeys = 10
  };

  /*
  Reference: VkKeyScan() / MSDN
  Bit Meaning 
  --- --------
  1   Either SHIFT key is pressed. 
  2   Either CTRL key is pressed. 
  4   Either ALT key is pressed. 
  8   The Hankaku key is pressed 
  16  Reserved (defined by the keyboard layout driver). 
  32  Reserved (defined by the keyboard layout driver). 
  */
  static const WORD VKKEYSCANSHIFTON;
  static const WORD VKKEYSCANCTRLON;
  static const WORD VKKEYSCANALTON;
  static const WORD INVALIDKEY;

  static key_desc_t KeyNames[MaxSendKeysRecs]; 
  static const BYTE ExtendedVKeys[MaxExtendedVKeys];

  static bool BitSet(BYTE BitTable, UINT BitMask);

  void PopUpShiftKeys();

  static bool IsVkExtended(BYTE VKey);
  void SendKeyUp(BYTE VKey);
  void SendKeyDown(BYTE VKey, WORD NumTimes, bool GenUpMsg, bool bDelay = false);
  void SendKey(WORD MKey, WORD NumTimes, bool GenDownMsg);
  static WORD StringToVKey(LPCTSTR KeyString, int &idx);
  void KeyboardEvent(BYTE VKey, BYTE ScanCode, LONG Flags);

public:

  bool SendKeys(LPCTSTR KeysString, bool Wait = false);
  static bool AppActivate(HWND wnd);
  static bool AppActivate(LPCTSTR WindowTitle, LPCTSTR WindowClass = 0);
  void SetDelay(const DWORD delay) { m_nDelayAlways = delay; }
  CSendKeys();
};

#endif