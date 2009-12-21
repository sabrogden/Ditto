#include "stdafx.h"
#include "HotKeys.h"
#include "Options.h"
#include "Misc.h"

CHotKeys g_HotKeys;


CHotKey::CHotKey( CString name, DWORD defKey, bool bUnregOnShowDitto ) 
	: m_Name(name), m_bIsRegistered(false), m_bUnRegisterOnShowDitto(bUnregOnShowDitto)
{
	m_Atom = ::GlobalAddAtom( m_Name );
	ASSERT( m_Atom );
	m_Key = (DWORD) g_Opt.GetProfileLong( m_Name, (long) defKey );
	g_HotKeys.Add( this );
}

CHotKey::~CHotKey()
{
	Unregister();
}

void CHotKey::SetKey( DWORD key, bool bSave )
{
	if(m_Key == key)
	{
		return;
	}

	if( m_bIsRegistered )
		Unregister();
	m_Key = key;
	if( bSave )
		SaveKey();
}

void CHotKey::LoadKey()
{
	SetKey( (DWORD) g_Opt.GetProfileLong( m_Name, 0 ) );
}

bool CHotKey::SaveKey()
{
	return g_Opt.SetProfileLong( m_Name, (long) m_Key ) != FALSE;
}

BOOL CHotKey::ValidateHotKey(DWORD dwHotKey)
{
	ATOM id = ::GlobalAddAtom(_T("HK_VALIDATE"));
	BOOL bResult = ::RegisterHotKey( g_HotKeys.m_hWnd,
		id,
		GetModifier(dwHotKey),
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

	ctrl.SetHotKey(LOBYTE(m_Key), (WORD)lModifiers); 

	if(lModifiers & HOTKEYF_EXT)
	{
		::CheckDlgButton(hParent, nWindowsCBID, BST_CHECKED);
	}
}

UINT CHotKey::GetModifier(DWORD dwHotKey)
{
	UINT uMod = 0;
	if( HIBYTE(dwHotKey) & HOTKEYF_SHIFT )   uMod |= MOD_SHIFT;
	if( HIBYTE(dwHotKey) & HOTKEYF_CONTROL ) uMod |= MOD_CONTROL;
	if( HIBYTE(dwHotKey) & HOTKEYF_ALT )     uMod |= MOD_ALT;
	if( HIBYTE(dwHotKey) & HOTKEYF_EXT )     uMod |= MOD_WIN;

	return uMod;
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
		m_bIsRegistered = true;

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
	int count = GetSize();
	for(int i=0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(pHotKey)
			delete pHotKey;
	}
}

int CHotKeys::Find( CHotKey* pHotKey )
{
	int count = GetSize();
	for(int i=0; i < count; i++)
	{
		if( pHotKey == ElementAt(i) )
			return i;
	}
	return -1;
}

bool CHotKeys::Remove( CHotKey* pHotKey )
{
	int i = Find(pHotKey);
	if(i >= 0)
	{
		RemoveAt(i);
		return true;
	}
	return false;
}

void CHotKeys::LoadAllKeys()
{
	int count = GetSize();
	for(int i=0; i < count; i++)
		ElementAt(i)->LoadKey();
}

void CHotKeys::SaveAllKeys()
{
	int count = GetSize();
	for(int i=0; i < count; i++)
		ElementAt(i)->SaveKey();
}

void CHotKeys::RegisterAll(bool bMsgOnError)
{
	CString str;
	CHotKey* pHotKey;
	int count = GetSize();
	for(int i = 0; i < count; i++)
	{
		pHotKey = ElementAt(i);
		if(!pHotKey->Register())
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
	int count = GetSize();
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
	int count = GetSize();
	keys.SetSize(count);
	for(int i=0; i < count; i++)
		keys[i] = ElementAt(i)->GetKey();
}

// caution! this alters hotkeys based upon corresponding indexes
void CHotKeys::SetKeys(ARRAY& keys, bool bSave)
{
	int count = GetSize();
	ASSERT(count == keys.GetSize());
	for(int i=0; i < count; i++)
		ElementAt(i)->SetKey(keys[i], bSave);
}

bool CHotKeys::FindFirstConflict(ARRAY& keys, int* pX, int* pY)
{
	bool bConflict = false;
	int i, j;
	int count = keys.GetSize();
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
bool CHotKeys::FindFirstConflict(int* pX, int* pY)
{
	ARRAY keys;
	GetKeys(keys);
	return FindFirstConflict(keys, pX, pY);
}
