#include "stdafx.h"
#include "HotKeys.h"
#include "Options.h"
#include "Misc.h"
#include "SendKeys.h"

CHotKeys g_HotKeys;

int CHotKey::m_nextId = 0;

CHotKey::CHotKey(CString name, DWORD defKey, bool bUnregOnShowDitto, HotKeyType hkType) 
	: m_Name(name), 
	m_bIsRegistered(false), 
	m_bUnRegisterOnShowDitto(bUnregOnShowDitto),
	m_clipId(0)
{
	m_Atom = ::GlobalAddAtom(StrF(_T("%s_%d"), m_Name, hkType));
	ASSERT(m_Atom);
	m_Key = (DWORD)g_Opt.GetProfileLong(m_Name, (long) defKey);
	m_globalId = m_nextId;
	m_nextId++;
	m_hkType = hkType;
	g_HotKeys.Add(this);
}

CHotKey::~CHotKey()
{
	Unregister();
}

CString CHotKey::GetHotKeyDisplay()
{
	return GetHotKeyDisplayStatic(m_Key);
}

CString CHotKey::GetHotKeyDisplayStatic(DWORD dwHotKey)
{
	WORD vk = LOBYTE(dwHotKey);
	CString keyDisplay;
	UINT modifiers = GetModifier(HIBYTE(dwHotKey));
	if(modifiers & MOD_SHIFT)
	{
		keyDisplay += _T("Shift + ");
	}

	if(modifiers & MOD_CONTROL)
	{
		keyDisplay += _T("Ctrl + ");
	}

	if(modifiers & MOD_ALT)
	{
		keyDisplay += _T("Alt + ");
	}

	if(modifiers & MOD_WIN)
	{
		keyDisplay += _T("Win + ");
	}

	switch (vk)
	{
	case VK_MOUSE_CLICK:
		keyDisplay += "Click";
		break;
	case VK_MOUSE_DOUBLE_CLICK:
		keyDisplay += "Double Click";
		break;
	case VK_MOUSE_RIGHT_CLICK:
		keyDisplay += "Right Click";
		break;
	case VK_MOUSE_MIDDLE_CLICK:
		keyDisplay += "Middle Click";
		break;
	default:
		keyDisplay += GetVirKeyName(vk);
		break;
	}	

	return keyDisplay;
}

//http://www.ffuts.org/blog/mapvirtualkey-getkeynametext-and-a-story-of-how-to/
CString CHotKey::GetVirKeyName(unsigned int virtualKey)
{
	unsigned int scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

	// because MapVirtualKey strips the extended bit for some keys
	switch (virtualKey)
	{
	case VK_LEFT: 
	case VK_UP: 
	case VK_RIGHT: 
	case VK_DOWN: // arrow keys
	case VK_PRIOR: 
	case VK_NEXT: // page up and page down
	case VK_END: 
	case VK_HOME:
	case VK_INSERT: 
	case VK_DELETE:
	case VK_DIVIDE: // numpad slash
	case VK_NUMLOCK:
		{
			scanCode |= 0x100; // set extended bit
			break;
		}
	}

	wchar_t keyName[50];
	if (GetKeyNameText(scanCode << 16, keyName, sizeof(keyName)) != 0)
	{
		return keyName;
	}
	else
	{
		return "[Error]";
	}
}

UINT CHotKey::GetModifier(DWORD dwHotKey)
{
	UINT uMod = 0;

	if(dwHotKey & HOTKEYF_SHIFT)   
		uMod |= MOD_SHIFT;
	if(dwHotKey & HOTKEYF_CONTROL) 
		uMod |= MOD_CONTROL;
	if(dwHotKey & HOTKEYF_ALT)     
		uMod |= MOD_ALT;
	if(dwHotKey & HOTKEYF_EXT)     
		uMod |= MOD_WIN;

	return uMod;
}

void CHotKey::SetKey( DWORD key, bool bSave )
{
	if(m_Key == key)
	{
		return;
	}

	if(m_bIsRegistered)
		Unregister();
	m_Key = key;
	if(bSave)
		SaveKey();
}

void CHotKey::LoadKey()
{
	SetKey((DWORD) g_Opt.GetProfileLong(m_Name, 0));
}

bool CHotKey::SaveKey()
{
	if(m_clipId <= 0)
	{
		return g_Opt.SetProfileLong( m_Name, (long) m_Key ) != FALSE;
	}


	return false;
}

BOOL CHotKey::ValidateHotKey(DWORD dwHotKey)
{
	ATOM id = ::GlobalAddAtom(_T("HK_VALIDATE"));
	BOOL bResult = ::RegisterHotKey( g_HotKeys.m_hWnd,
		id,
		GetModifier(HIBYTE(dwHotKey)),
		LOBYTE(dwHotKey) );

	if(bResult)
		::UnregisterHotKey(g_HotKeys.m_hWnd, id);

	::GlobalDeleteAtom(id);

	return bResult;
}

void CHotKey::CopyFromCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID) 
{ 
	long lHotKey = ctrl.GetHotKey();

	short sKeyKode = LOBYTE(lHotKey);
	short sModifers = HIBYTE(lHotKey);

	if(lHotKey && ::IsDlgButtonChecked(hParent, nWindowsCBID))
	{
		sModifers |= HOTKEYF_EXT;
	}

	SetKey(MAKEWORD(sKeyKode, sModifers)); 
}

void CHotKey::CopyToCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID)
{
	long lModifiers = HIBYTE(m_Key);
	long keys = LOBYTE(m_Key);

	ctrl.SetHotKey((WORD)keys, (WORD)lModifiers & ~HOTKEYF_EXT); 

	if(lModifiers & HOTKEYF_EXT)
	{
		::CheckDlgButton(hParent, nWindowsCBID, BST_CHECKED);
	}
}

bool CHotKey::Register()
{
	if(m_Key)
	{
		if(m_bIsRegistered == false)
		{
			ASSERT(g_HotKeys.m_hWnd);
			m_bIsRegistered = ::RegisterHotKey(g_HotKeys.m_hWnd,
				m_Atom,
				GetModifier(),
				LOBYTE(m_Key) ) == TRUE;
		}
	}
	else
	{
		m_bIsRegistered = false;
	}

	return m_bIsRegistered;
}
bool CHotKey::Unregister(bool bOnShowingDitto)
{
	if(!m_bIsRegistered)
		return true;

	if(bOnShowingDitto)
	{
		if(m_bUnRegisterOnShowDitto == false)
			return true;
	}

	if(m_Key)
	{
		ASSERT(g_HotKeys.m_hWnd);
		if(::UnregisterHotKey( g_HotKeys.m_hWnd, m_Atom))
		{
			m_bIsRegistered = false;
			return true;
		}
		else
		{
			Log(_T("Unregister FAILED!"));
			ASSERT(0);
		}
	}
	else
	{
		m_bIsRegistered = false;
		return true;
	}

	return false;
}

CHotKeys::CHotKeys() : m_hWnd(NULL) 
{

}

CHotKeys::~CHotKeys()
{
	CHotKey* pHotKey;
	INT_PTR count = GetSize();
	for(int i=0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(pHotKey)
		{
			delete pHotKey;
		}
	}
}

INT_PTR CHotKeys::Find(CHotKey* pHotKey)
{
	INT_PTR count = GetSize();
	for(int i=0; i < count; i++)
	{
		if(pHotKey == ElementAt(i))
		{
			return i;
		}
	}
	return -1;
}

bool CHotKeys::Remove(CHotKey* pHotKey)
{
	INT_PTR i = Find(pHotKey);
	if(i >= 0)
	{
		RemoveAt(i);
		return true;
	}
	return false;
}

bool CHotKeys::Remove(int clipId, CHotKey::HotKeyType hkType)
{
	INT_PTR count = GetSize();
	for(int i=0; i < count; i++)
	{
		if(ElementAt(i) != NULL && 
			ElementAt(i)->m_clipId == clipId &&
			ElementAt(i)->m_hkType == hkType)
		{
			CHotKey *pKey = ElementAt(i);

			RemoveAt(i);

			delete pKey;

			return true;
		}
	}
	return false;
}

BOOL CHotKeys::ValidateClip(int clipId, DWORD key, CString desc, CHotKey::HotKeyType hkType)
{
	CHotKey *pKey = NULL;
	INT_PTR count = GetSize();
	for(int i=0; i < count; i++)
	{
		if(ElementAt(i) != NULL && 
			ElementAt(i)->m_clipId == clipId &&
			ElementAt(i)->m_hkType == hkType)
		{
			pKey = ElementAt(i);
			break;
		}
	}

	if(pKey == NULL)
	{
		pKey = new CHotKey(desc, key, true, hkType);
	}
	   
	BOOL ret = FALSE;

	if(pKey != NULL)
	{
		pKey->m_Key = key;
		pKey->m_Name = desc;
		pKey->m_clipId = clipId;

		ret = CHotKey::ValidateHotKey(key);
	}

	return ret;
}

void CHotKeys::LoadAllKeys()
{
	INT_PTR count = GetSize();
	for(int i=0; i < count; i++)
	{
		ElementAt(i)->LoadKey();
	}
}

void CHotKeys::SaveAllKeys()
{
	INT_PTR count = GetSize();
	for(int i=0; i < count; i++)
	{
		ElementAt(i)->SaveKey();
	}
}

void CHotKeys::RegisterAll(bool bMsgOnError)
{
	CString str;
	CHotKey* pHotKey;
	INT_PTR count = GetSize();
	for(int i = 0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(!pHotKey->Register() && pHotKey->m_Key > 0)
		{
			str =  "Error Registering ";
			str += pHotKey->GetName();
			Log(str);
			if(bMsgOnError)
				AfxMessageBox(str);
		}
	}
}

void CHotKeys::UnregisterAll(bool bMsgOnError, bool bOnShowDitto)
{
	CString str;
	CHotKey* pHotKey;
	INT_PTR count = GetSize();
	for(int i = 0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(!pHotKey->Unregister(bOnShowDitto))
		{
			str = "Error Unregistering ";
			str += pHotKey->GetName();
			Log(str);
			if(bMsgOnError)
				AfxMessageBox(str);
		}
	}
}

void CHotKeys::GetKeys(ARRAY& keys)
{
	INT_PTR count = GetSize();
	keys.SetSize(count);
	for(int i=0; i < count; i++)
	{
		keys[i] = ElementAt(i)->GetKey();
	}
}

// caution! this alters hotkeys based upon corresponding indexes
void CHotKeys::SetKeys(ARRAY& keys, bool bSave)
{
	INT_PTR count = GetSize();
	ASSERT(count == keys.GetSize());
	for(int i=0; i < count; i++)
	{
		ElementAt(i)->SetKey(keys[(INT)i], bSave);
	}
}

bool CHotKeys::FindFirstConflict(ARRAY& keys, INT_PTR* pX, INT_PTR* pY)
{
	bool bConflict = false;
	INT_PTR i, j;
	INT_PTR count = keys.GetSize();
	DWORD key;
	for(i = 0; i < count && !bConflict; i++)
	{
		key = keys.ElementAt(i);
		// only check valid keys
		if(key == 0)
			continue;

		// scan the array for a duplicate
		for(j = i+1; j < count; j++ )
		{
			if(keys.ElementAt(j) == key)
			{
				bConflict = true;
				break;
			}
		}
	}

	if(bConflict)
	{
		if(pX)
			*pX = i-1;
		if(pY)
			*pY = j;
	}

	return bConflict;
}

// if true, pX and pY (if valid) are set to the indexes of the conflicting hotkeys.
bool CHotKeys::FindFirstConflict(INT_PTR* pX, INT_PTR* pY)
{
	ARRAY keys;
	GetKeys(keys);
	return FindFirstConflict(keys, pX, pY);
}
