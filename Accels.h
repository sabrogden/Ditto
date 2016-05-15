#pragma once

#include <map>

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

    CAccel(DWORD key = 0, DWORD cmd = 0, DWORD key2 = 0)
    {
        Key = key;
		Key2 = key2;
        Cmd = cmd;
		RefId = 0;
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

	void AddAccel(DWORD cmd, DWORD key, DWORD key2 = 0);

	void RemoveAll();

	CString GetCmdKeyText(DWORD cmd);

    // handles a key's first WM_KEYDOWN or WM_SYSKEYDOWN message.
    // it uses GetKeyState to test for modifiers.
    // returns a pointer to the internal CAccel if it matches the given key or NULL
    bool OnMsg(MSG *pMsg, CAccel &a);

	bool m_handleRepeatKeys;

    static BYTE GetKeyStateModifiers();

protected:

	multimap<DWORD, CAccel> m_multiMap;
	DWORD m_activeFirstKey;

	DWORD m_firstMapTick;
};
