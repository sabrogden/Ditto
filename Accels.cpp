#include "stdafx.h"
#include "Accels.h"
#include "HotKeys.h"
#include "Options.h"

CAccels::CAccels()
{
	m_handleRepeatKeys = false;
	m_firstMapTick = 0;
	m_activeFirstKey = 0;
	m_checkModifierKeys = true;
}


void CAccels::AddAccel(CAccel a)
{
	m_multiMap.insert(pair<DWORD, CAccel>(a.Key, a));
}

void CAccels::AddAccel(DWORD cmd, DWORD key, DWORD key2, CString refData)
{
	if ((int)key2 <= 0)
	{
		key2 = 0;
	}
	CAccel a(key, cmd, key2, refData);

	m_multiMap.insert(pair<DWORD, CAccel>(key, a));
}

void CAccels::RemoveAll()
{
	m_multiMap.clear();
}

CString CAccels::GetCmdKeyText(DWORD cmd, CString refData)
{
	CString cmdShortcutText;
	for (multimap<DWORD, CAccel>::iterator it = m_multiMap.begin(); it != m_multiMap.end(); ++it)
	{
		if (it->second.Cmd == cmd &&
			(refData == _T("") || it->second.RefData == refData))
		{
			if (it->second.Key != 0)
			{
				cmdShortcutText = CHotKey::GetHotKeyDisplayStatic(it->second.Key);
				if (it->second.Key2 != 0)
				{
					CString cmdShortcutText2 = CHotKey::GetHotKeyDisplayStatic(it->second.Key2);

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

    const BYTE vkey = LOBYTE(pMsg->wParam);
	BYTE mod = 0;
	if (m_checkModifierKeys)
	{
		mod = GetKeyStateModifiers();
	}
    const DWORD key = ACCEL_MAKEKEY(vkey, mod);

    //CString cs;
    //cs.Format(_T("Key: %d, Mod: %d, vkey: %d, diff: %d\r\n"), key, mod, vkey, (GetTickCount() - m_firstMapTick));
    //OutputDebugString(cs);
		
	if (m_firstMapTick != 0 &&
		(GetTickCount64() - m_firstMapTick) < CGetSetOptions::m_doubleKeyStrokeTimeout)
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
				//return now as a another command could have a second key defined
				//if they don't press the second key this will be handled by a timer on the outside				
			}
			else
			{
				m_activeFirstKey = key;
				m_firstMapTick = GetTickCount64();
				break;
			}
		}

		if (a.Cmd > 0 && m_activeFirstKey == 0)
		{
			return true;
		}
	}

    return false;
}

bool CAccels::ContainsKey(int vKey)
{
	CString cmdShortcutText;
	for (auto it = m_multiMap.begin(); it != m_multiMap.end(); ++it)
	{
		if (LOBYTE(it->second.Key) == vKey || LOBYTE(it->second.Key2) == vKey)
		{
			return true;
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
