#pragma once

#include "Shared/ArrayEx.h"

class CHotKey
{
public:
	CString	m_Name;
	ATOM	m_Atom;
	DWORD	m_Key; //704 is ctrl-tilda
	bool	m_bIsRegistered;
	bool	m_bUnRegisterOnShowDitto;
	int		m_clipId;
	
	CHotKey( CString name, DWORD defKey = 0, bool bUnregOnShowDitto = false );
	~CHotKey();

	bool	IsRegistered() { return m_bIsRegistered; }
	CString GetName()      { return m_Name; }
	DWORD   GetKey()       { return m_Key; }
	CString GetHotKeyDisplay();

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
	UINT GetModifier() { return GetModifier(HIBYTE(m_Key)); }

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

	INT_PTR Find( CHotKey* pHotKey );
	bool Remove( CHotKey* pHotKey ); // pHotKey is NOT deleted.

	bool Remove(int clipId);

	BOOL ValidateClip(int clipId, DWORD key, CString desc);

	// profile load / save
	void LoadAllKeys();
	void SaveAllKeys();

	void RegisterAll(bool bMsgOnError = false);
	void UnregisterAll(bool bMsgOnError = false, bool bOnShowDitto = false);

	void GetKeys( ARRAY& keys );
	void SetKeys( ARRAY& keys, bool bSave = false ); // caution! this alters hotkeys based upon corresponding indexes

	static bool FindFirstConflict( ARRAY& keys, INT_PTR* pX = NULL, INT_PTR* pY = NULL );
	// if true, pX and pY (if valid) are set to the index of the conflicting hotkeys.
	bool FindFirstConflict( INT_PTR* pX = NULL, INT_PTR* pY = NULL );
};

extern CHotKeys g_HotKeys;