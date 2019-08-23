// MultiLanguage.h: interface for the CMultiLanguage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTILANGUAGE_H__DA57BA64_C421_4368_9498_1EFCE49A5C52__INCLUDED_)
#define AFX_MULTILANGUAGE_H__DA57BA64_C421_4368_9498_1EFCE49A5C52__INCLUDED_

#include "tinyxml\Tinyxml.h"
#include "tinyxml\tinystr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLangItem
{
public:
	CLangItem()
	{
		m_nID = -1;
		m_csEnglishLang = "";
		m_csForeignLang = "";
		m_csID = "";
	}
	int m_nID;
	CString m_csID;
	CString m_csEnglishLang;
	CString m_csForeignLang;
};

typedef CArray<CLangItem*, CLangItem*> LANGUAGE_ARRAY;

typedef CMap<CString, LPCTSTR, CLangItem*, CLangItem*> LANGUAGE_MAP;

class CMultiLanguage  
{
public:
	CMultiLanguage();
	virtual ~CMultiLanguage();

	bool LoadLanguageFile(CString csFile);

	bool UpdateRightClickMenu(CMenu *pMenu);
	bool UpdateGroupsRightClickMenu(CMenu *pMenu);
	bool UpdateTrayIconRightClickMenu(CMenu *pMenu);

	bool UpdateClipProperties(CWnd *pParent);
	bool UpdateOptionGeneral(CWnd *pParent);
	bool UpdateOptionSupportedTypes(CWnd *pParent);
	bool UpdateOptionShortcuts(CWnd *pParent);
	bool UpdateOptionQuickPaste(CWnd *pParent);
	bool UpdateOptionQuickPasteKeyboard(CWnd *pParent);
	bool UpdateOptionFriends(CWnd *pParent);
	bool UpdateOptionFriendsDetail(CWnd *pParent);
	bool UpdateOptionStats(CWnd *pParent);
	bool UpdateOptionSupportedTypesAdd(CWnd *pParent);
	bool UpdateMoveToGroups(CWnd *pParent);
	bool UpdateOptionsSheet(CWnd *pParent);
	bool UpdateOptionCopyBuffers(CWnd *pParent);
	bool UpdateGlobalHotKeys(CWnd *pParent);
	bool UpdateDeleteClipData(CWnd *pParent);

	CString GetGlobalHotKeyString(CString csID, CString csDefault);
	CString GetDeleteClipDataString(CString csID, CString csDefault);
	CString GetQuickPasteKeyboardString(int id, CString csDefault);

	CString GetString(CString csID, CString csDefault);

	CString GetAuthor()	{ return m_csAuthor;		}
	long	GetVersion(){ return m_lFileVersion;	}
	CString GetNotes()	{ return m_csNotes;			}
	CString GetLangCode() { return m_csLangCode;	}

	void	SetOnlyGetHeader(bool bVal)	{ m_bOnlyGetHeader = true;	}
	static CMenu* GetMenuPos(CMenu *pMenu, const CString &csLookingForMenuText, int &nMenuPos, bool returnChildIfOne = FALSE);

	CString m_csLastError;

protected:
	LANGUAGE_ARRAY m_RightClickMenu;
	LANGUAGE_ARRAY m_GroupsRightClickMenu;
	LANGUAGE_ARRAY m_TrayIconRightClickMenu;
	LANGUAGE_ARRAY m_ClipProperties;
	LANGUAGE_ARRAY m_OptionsGeneral;
	LANGUAGE_ARRAY m_OptionsSupportedTypes;
	LANGUAGE_ARRAY m_OptionsShortcuts;
	LANGUAGE_ARRAY m_OptionsQuickPaste;
	LANGUAGE_ARRAY m_OptionsQuickPasteKeyboard;
	LANGUAGE_ARRAY m_OptionsFriends;
	LANGUAGE_ARRAY m_OptionsFriendsDetail;
	LANGUAGE_ARRAY m_OptionsStats;
	LANGUAGE_ARRAY m_OptionsSupportedTypesAdd;
	LANGUAGE_ARRAY m_MoveToGroups;
	LANGUAGE_ARRAY m_OptionsSheet;
	LANGUAGE_ARRAY m_OptionsCopyBuffers;
	LANGUAGE_ARRAY m_GlobalHotKeys;
	LANGUAGE_ARRAY m_DeleteClipData;

	LANGUAGE_MAP	m_StringMap;

	CString m_csAuthor;
	CString m_csNotes;
	long	m_lFileVersion;
	CString m_csLangCode;

	bool	m_bOnlyGetHeader;

protected:
	bool LoadSection(TiXmlNode &doc, LANGUAGE_ARRAY &Array, CString csSection);
	bool LoadStringTableSection(TiXmlNode &doc, LANGUAGE_MAP &Map, CString csSection);

	bool UpdateMenuToLanguage(CMenu *pMenu, LANGUAGE_ARRAY &Array);
	bool UpdateWindowToLanguage(CWnd *pParent, LANGUAGE_ARRAY &Array);


	void ClearArrays();
	void ClearArray(LANGUAGE_ARRAY &Array);
	void ClearMap(LANGUAGE_MAP &Map);
};

#endif // !defined(AFX_MULTILANGUAGE_H__DA57BA64_C421_4368_9498_1EFCE49A5C52__INCLUDED_)
