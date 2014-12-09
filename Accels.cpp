#include "stdafx.h"
#include "Accels.h"
#include "HotKeys.h"

CAccels::CAccels(){

}

void CAccels::AddAccel(CAccel &a)
{
    m_Map.SetAt(a.Key, a.Cmd);
}

void CAccels::AddAccel(DWORD cmd, DWORD key)
{
	m_Map.SetAt(key, cmd);
}

void CAccels::RemoveAll()
{
	m_Map.RemoveAll();
}

CString CAccels::GetCmdKeyText(DWORD cmd)
{
	CString cmdShortcutText = _T("");
	POSITION pos = m_Map.GetStartPosition();
	DWORD mapShortcut;
	DWORD mapCmd;
	while (pos != NULL)
	{
		m_Map.GetNextAssoc(pos, mapShortcut, mapCmd);

		if(mapCmd == cmd)
		{
			cmdShortcutText = CHotKey::GetHotKeyDisplayStatic(mapShortcut);
			break;
		}
	}

	return cmdShortcutText;
}

bool CAccels::OnMsg(MSG *pMsg, DWORD &dID)
{
    // bit 30 (0x40000000) is 1 if this is NOT the first msg of the key
    //  i.e. auto-repeat may cause multiple msgs of the same key
    if((pMsg->lParam &0x40000000) || (pMsg->message != WM_KEYDOWN && pMsg->message != WM_SYSKEYDOWN))
    {
        return NULL;
    }

    if(!pMsg || m_Map.GetCount() <= 0)
    {
        return NULL;
    }

    BYTE vkey = LOBYTE(pMsg->wParam);
    BYTE mod = GetKeyStateModifiers();
    DWORD key = ACCEL_MAKEKEY(vkey, mod);

    CString cs;
    cs.Format(_T("Key: %d, Mod: %d, vkey: %d"), key, mod, vkey);
    OutputDebugString(cs);

    if(m_Map.Lookup(key, dID))
    {
        return true;
    }

    return false;
}

BYTE CAccels::GetKeyStateModifiers()
{
    BYTE m = 0;
    if(GetKeyState(VK_SHIFT) &0x8000)
    {
        m |= HOTKEYF_SHIFT;
    }
    if(GetKeyState(VK_CONTROL) &0x8000)
    {
        m |= HOTKEYF_CONTROL;
    }
    if(GetKeyState(VK_MENU) &0x8000)
    {
        m |= HOTKEYF_ALT;
    }
    if(GetKeyState(VK_LWIN) &0x8000)
    {
        m |= HOTKEYF_EXT;
    }
    if(GetKeyState(VK_RWIN) &0x8000)
    {
        m |= HOTKEYF_EXT;
    }
    return m;
}
