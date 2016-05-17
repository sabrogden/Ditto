#include "stdafx.h"
#include "Accels.h"
#include "HotKeys.h"

CAccels::CAccels()
{
	m_handleRepeatKeys = false;
	m_firstMapTick = 0;
	m_activeFirstKey = 0;
}


void CAccels::AddAccel(CAccel a)
{
	m_multiMap.insert(pair<DWORD, CAccel>(a.Key, a));
}

void CAccels::AddAccel(DWORD cmd, DWORD key, DWORD key2)
{
	if ((int)key2 <= 0)
	{
		key2 = 0;
	}
	CAccel a(key, cmd, key2);

	m_multiMap.insert(pair<DWORD, CAccel>(key, a));
}

void CAccels::RemoveAll()
{
	m_multiMap.clear();
}

CString CAccels::GetCmdKeyText(DWORD cmd)
{
	CString cmdShortcutText;
	for (multimap<DWORD, CAccel>::iterator it = m_multiMap.begin(); it != m_multiMap.end(); ++it)
	{
		if (it->second.Cmd == cmd)
		{
			if (it->second.Key != 0)
			{
				cmdShortcutText = CHotKey::GetHotKeyDisplayStatic(it->second.Key);
				if (it->second.Key2 != 0)
				{
					CString cmdShortcutText2 = CHotKey::GetHotKeyDisplayStatic(it->second.Key);

					if (cmdShortcutText2.GetLength() > 0)
					{
						cmdShortcutText += _T(" - ");
						cmdShortcutText += cmdShortcutText2;
					}
				}
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

    if(!pMsg)
    {
        return NULL;
    }

    BYTE vkey = LOBYTE(pMsg->wParam);
    BYTE mod = GetKeyStateModifiers();
    DWORD key = ACCEL_MAKEKEY(vkey, mod);

    CString cs;
    cs.Format(_T("Key: %d, Mod: %d, vkey: %d\r\n"), key, mod, vkey);
    OutputDebugString(cs);
		
	if (m_firstMapTick != 0 &&
		(GetTickCount() - m_firstMapTick) < 500)
	{
		pair<multimap<DWORD, CAccel>::iterator, multimap<DWORD, CAccel>::iterator> ppp;
		ppp = m_multiMap.equal_range(m_activeFirstKey);

		for (multimap<DWORD, CAccel>::iterator it2 = ppp.first; it2 != ppp.second; ++it2)
		{
			if (key == it2->second.Key2)
			{
				a = (*it2).second;
				m_firstMapTick = 0;
				m_activeFirstKey = 0;
				return true;
			}
		}
	}
	else
	{
		m_firstMapTick = 0;
		m_activeFirstKey = 0;

		pair<multimap<DWORD, CAccel>::iterator, multimap<DWORD, CAccel>::iterator> ppp;
		ppp = m_multiMap.equal_range(key);

		for (multimap<DWORD, CAccel>::iterator it2 = ppp.first; it2 != ppp.second; ++it2)
		{
			if (it2->second.Key2 == 0)
			{
				a = (*it2).second;
				return true;
			}
			else
			{
				m_activeFirstKey = key;
				m_firstMapTick = GetTickCount();
				break;
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
