#pragma once

#include <map>
#include "..\Shared\ArrayEx.h"

#define ACCEL_VKEY(key)			LOBYTE(key)
#define ACCEL_MOD(key)			HIBYTE(key)
#define ACCEL_MAKEKEY(vkey,mod) ((mod << 8) | vkey)

using namespace std;

class CAccel
{
public:
    DWORD Key;
	DWORD Key2;
    DWORD Cmd;
	int RefId;
	CString RefData;

    CAccel(DWORD key = 0, DWORD cmd = 0, DWORD key2 = 0, CString refData = _T(""))
    {
        Key = key;
		Key2 = key2;
        Cmd = cmd;
		RefId = 0;
		RefData = refData;
    }
};

/*------------------------------------------------------------------*\
CAccels - Manages a set of CAccel
\*------------------------------------------------------------------*/
class CAccels
{
public:
    CAccels();

    void AddAccel(CAccel a);

	void AddAccel(DWORD cmd, DWORD key, DWORD key2 = 0, CString refData = _T(""));

	void RemoveAll();

	CString GetCmdKeyText(DWORD cmd, CString refData = _T(""));

    // handles a key's first WM_KEYDOWN or WM_SYSKEYDOWN message.
    // it uses GetKeyState to test for modifiers.
    // returns a pointer to the internal CAccel if it matches the given key or NULL
    bool OnMsg(MSG *pMsg, CAccel &a);

	bool ContainsKey(int vKey);

	bool m_handleRepeatKeys;
	bool m_checkModifierKeys;

    static BYTE GetKeyStateModifiers();

protected:

	multimap<DWORD, CAccel> m_multiMap;
	DWORD m_activeFirstKey;

	ULONGLONG m_firstMapTick;
};
