#pragma once

#include "ArrayEx.h"

class CHotKey
{
public:
	CString	m_Name;
	ATOM	m_Atom;
	DWORD	m_Key; //704 is ctrl-tilda
	bool	m_bIsRegistered;
	bool	m_bUnRegisterOnShowDitto;
	
	CHotKey( CString name, DWORD defKey = 0, bool bUnregOnShowDitto = false );
	~CHotKey();

	bool	IsRegistered() { return m_bIsRegistered; }
	CString GetName()      { return m_Name; }
	DWORD   GetKey()       { return m_Key; }

	void SetKey( DWORD key, bool bSave = false );
	// profile
	void LoadKey();
	bool SaveKey();

	void CopyFromCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID);
	void CopyToCtrl(CHotKeyCtrl& ctrl, HWND hParent, int nWindowsCBID);

//	CString GetKeyAsText();
//	void SetKeyFromText( CString text );

	static BOOL ValidateHotKey(DWORD dwHotKey);
	static UINT GetModifier(DWORD dwHotKey);
	UINT GetModifier() { return GetModifier(m_Key); }

	bool Register();
	bool Unregister(bool bOnShowingDitto = false);
};


/*------------------------------------------------------------------*\
	CHotKeys - Manages system-wide hotkeys
\*------------------------------------------------------------------*/

class CHotKeys : public CArray<CHotKey*,CHotKey*>
{
public:
	HWND	m_hWnd;

	CHotKeys();
	~CHotKeys();

	void Init( HWND hWnd ) { m_hWnd = hWnd; }

	int Find( CHotKey* pHotKey );
	bool Remove( CHotKey* pHotKey ); // pHotKey is NOT deleted.

	// profile load / save
	void LoadAllKeys();
	void SaveAllKeys();

	void RegisterAll(bool bMsgOnError = false);
	void UnregisterAll(bool bMsgOnError = false, bool bOnShowDitto = false);

	void GetKeys( ARRAY& keys );
	void SetKeys( ARRAY& keys, bool bSave = false ); // caution! this alters hotkeys based upon corresponding indexes

	static bool FindFirstConflict( ARRAY& keys, int* pX = NULL, int* pY = NULL );
	// if true, pX and pY (if valid) are set to the index of the conflicting hotkeys.
	bool FindFirstConflict( int* pX = NULL, int* pY = NULL );
};

extern CHotKeys g_HotKeys;