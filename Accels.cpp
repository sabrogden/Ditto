#include "stdafx.h"
#include "Accels.h"
#include "HotKeys.h"

CAccels::CAccels()
{
	m_handleRepeatKeys = false;
	m_firstMapTick = 0;
}

void CAccels::AddAccel(CAccel a)
{
    m_Map.SetAt(a.Key, a);
}

void CAccels::AddAccel(DWORD cmd, DWORD key, DWORD key2)
{
	CAccel a(key, cmd);
	
	if((int)key2 > 0)
	{
		a.SecondKey = true;
		m_Map2.SetAt(key2, a);
	}

	m_Map.SetAt(key, a);
}

void CAccels::RemoveAll()
{
	m_Map.RemoveAll();
	m_Map2.RemoveAll();
}

CString CAccels::GetCmdKeyText(DWORD cmd)
{
	CString cmdShortcutText = _T("");
	POSITION pos = m_Map.GetStartPosition();
	DWORD mapShortcut;
	CAccel a;
	while (pos != NULL)
	{
		m_Map.GetNextAssoc(pos, mapShortcut, a);

		if(a.Cmd == cmd)
		{
			CString cmdShortcutText2;
			CAccel a2;
			DWORD mapShortcut2;
			POSITION pos2 = m_Map2.GetStartPosition();
			while (pos2 != NULL)
			{
				m_Map2.GetNextAssoc(pos2, mapShortcut2, a2);
				if(a2.Cmd == cmd)
				{
					cmdShortcutText2 = CHotKey::GetHotKeyDisplayStatic(mapShortcut2);
				}
			}

			cmdShortcutText = CHotKey::GetHotKeyDisplayStatic(mapShortcut);

			if(cmdShortcutText2.GetLength() > 0)
			{
				cmdShortcutText += _T(" - ");
				cmdShortcutText += cmdShortcutText2;
			}
			break;
		}
	}

	return cmdShortcutText;
}

bool CAccels::OnMsg(MSG *pMsg, CAccel &a)
{    
	if((pMsg->message != WM_KEYDOWN && pMsg->message != WM_SYSKEYDOWN))
	{
		return NULL;
	}

	// bit 30 (0x40000000) is 1 if this is NOT the first msg of the key
	//  i.e. auto-repeat may cause multiple msgs of the same key
	if((pMsg->lParam &0x40000000) && m_handleRepeatKeys == false)
	{
		return NULL;
	}

	m_handleRepeatKeys = false;

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

	if (m_firstMapTick != 0 &&
		(GetTickCount() - m_firstMapTick) < 500)
	{
		if (m_Map2.Lookup(key, a))
		{
			m_firstMapTick = 0;
			return true;
		}
	}
	else
	{
		if (m_Map.Lookup(key, a))
		{
			if (a.SecondKey == false)
			{
				m_firstMapTick = 0;
				return true;
			}
			else
			{
				m_firstMapTick = GetTickCount();
			}
		}
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
