#pragma once

#define ACCEL_VKEY(key)			LOBYTE(key)
#define ACCEL_MOD(key)			HIBYTE(key)
#define ACCEL_MAKEKEY(vkey,mod) ((mod << 8) | vkey)

class CAccel
{
public:
    DWORD Key;
    DWORD Cmd;
    CAccel(DWORD key = 0, DWORD cmd = 0)
    {
        Key = key;
        Cmd = cmd;
    }
};

/*------------------------------------------------------------------*\
CAccels - Manages a set of CAccel
\*------------------------------------------------------------------*/
class CAccels
{
public:
    CAccels();

    CMap < DWORD, DWORD, DWORD, DWORD > m_Map;

    void AddAccel(CAccel &a);

    // handles a key's first WM_KEYDOWN or WM_SYSKEYDOWN message.
    // it uses GetKeyState to test for modifiers.
    // returns a pointer to the internal CAccel if it matches the given key or NULL
    bool OnMsg(MSG *pMsg, DWORD &dID);

    static BYTE GetKeyStateModifiers();
};
