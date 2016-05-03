#pragma once

#define ACCEL_VKEY(key)			LOBYTE(key)
#define ACCEL_MOD(key)			HIBYTE(key)
#define ACCEL_MAKEKEY(vkey,mod) ((mod << 8) | vkey)

class CAccel
{
public:
    DWORD Key;
    DWORD Cmd;
	int RefId;
	bool SecondKey;

    CAccel(DWORD key = 0, DWORD cmd = 0)
    {
        Key = key;
        Cmd = cmd;
		RefId = 0;
		SecondKey = false;
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
	CMap < DWORD, DWORD, CAccel, CAccel > m_Map;
	CMap < DWORD, DWORD, CAccel, CAccel > m_Map2;

	DWORD m_firstMapTick;
};
