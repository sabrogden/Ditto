// MultiLanguage.cpp: implementation of the CMultiLanguage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "MultiLanguage.h"
#include "shared/TextConvert.h"
#include "shared/TextConvert.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiLanguage::CMultiLanguage()
{
	m_csAuthor = "";
	m_csLangCode = "";
	m_lFileVersion = 0;
	m_bOnlyGetHeader = false;
}

CMultiLanguage::~CMultiLanguage()
{
	ClearArrays();
}

void CMultiLanguage::ClearArrays()
{
	m_csAuthor = "";
	m_csLangCode = "";
	m_lFileVersion = 0;
	m_bOnlyGetHeader = false;

	ClearArray(m_RightClickMenu);
	ClearArray(m_ClipProperties);
	ClearArray(m_OptionsGeneral);
	ClearArray(m_OptionsSupportedTypes);
	ClearArray(m_OptionsShortcuts);
	ClearArray(m_OptionsQuickPaste);
	ClearArray(m_OptionsFriends);
	ClearArray(m_OptionsFriendsDetail);
	ClearArray(m_OptionsStats);
	ClearArray(m_OptionsSupportedTypesAdd);
	ClearArray(m_MoveToGroups);
	ClearArray(m_TrayIconRightClickMenu);
	ClearArray(m_OptionsSheet);
	ClearArray(m_OptionsCopyBuffers);
	ClearArray(m_GlobalHotKeys);

	ClearMap(m_StringMap);
}

void CMultiLanguage::ClearArray(LANGUAGE_ARRAY &Array)
{
	INT_PTR size = Array.GetSize();
	for(int i = 0; i < size; i++)
	{
		CLangItem *plItem = Array[i];

		delete plItem;
		plItem = NULL;
	}
	Array.RemoveAll();
}

void CMultiLanguage::ClearMap(LANGUAGE_MAP &Map)
{
	POSITION pos = Map.GetStartPosition();

	CLangItem *plItem;
	CString csKey;

	while(pos)
	{
		Map.GetNextAssoc(pos, csKey, plItem);

		if(plItem)
		{
			delete plItem;
			plItem = NULL;
		}
	}

	Map.RemoveAll();
}

CString CMultiLanguage::GetString(CString csID, CString csDefault)
{
	CLangItem *pItem;
	
	if(m_StringMap.Lookup(csID, pItem) == FALSE)
	{
		return csDefault;
	}

	if(pItem->m_csForeignLang.GetLength() <= 0)
		return csDefault;

	return pItem->m_csForeignLang;
}

CString CMultiLanguage::GetGlobalHotKeyString(CString csID, CString csDefault)
{
	INT_PTR size = m_GlobalHotKeys.GetSize();
	for(int i = 0; i < size; i++)
	{
		CLangItem *plItem = m_GlobalHotKeys[i];
		if(plItem->m_csID == csID)
		{
			return plItem->m_csForeignLang;
		}
	}

	return csDefault;
}

bool CMultiLanguage::UpdateRightClickMenu(CMenu *pMenu)
{
	return UpdateMenuToLanguage(pMenu, m_RightClickMenu);
}

bool CMultiLanguage::UpdateTrayIconRightClickMenu(CMenu *pMenu)
{
	return UpdateMenuToLanguage(pMenu, m_TrayIconRightClickMenu);
}

bool CMultiLanguage::UpdateClipProperties(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_ClipProperties);
}

bool CMultiLanguage::UpdateOptionGeneral(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsGeneral);
}

bool CMultiLanguage::UpdateOptionSupportedTypes(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsSupportedTypes);
}

bool CMultiLanguage::UpdateOptionShortcuts(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsShortcuts);
}

bool CMultiLanguage::UpdateOptionQuickPaste(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsQuickPaste);
}

bool CMultiLanguage::UpdateOptionFriends(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsFriends);
}

bool CMultiLanguage::UpdateOptionFriendsDetail(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsFriendsDetail);
}

bool CMultiLanguage::UpdateOptionStats(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsStats);
}

bool CMultiLanguage::UpdateOptionSupportedTypesAdd(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsSupportedTypesAdd);
}

bool CMultiLanguage::UpdateMoveToGroups(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_MoveToGroups);
}

bool CMultiLanguage::UpdateOptionsSheet(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsSheet);
}

bool CMultiLanguage::UpdateOptionCopyBuffers(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_OptionsCopyBuffers);
}

bool CMultiLanguage::UpdateGlobalHotKeys(CWnd *pParent)
{
	return UpdateWindowToLanguage(pParent, m_GlobalHotKeys);
}

bool CMultiLanguage::UpdateMenuToLanguage(CMenu *pMenu, LANGUAGE_ARRAY &Array)
{
	INT_PTR size = Array.GetSize();
	for(int i = 0; i < size; i++)
	{
		CLangItem *plItem = Array[i];

		if(plItem->m_csForeignLang.GetLength() > 0)
		{
			if(plItem->m_nID > 0)
			{
				pMenu->ModifyMenu(plItem->m_nID, MF_BYCOMMAND, plItem->m_nID, plItem->m_csForeignLang);
			}
			else
			{
				//If an item doesn't have a menu id then its a group menu
				//just search for the text and update the text with the foreign text
				int nMenuPos;
				CMenu *pNewMenu = GetMenuPos(pMenu, plItem->m_csEnglishLang, nMenuPos);
				if(pNewMenu)
				{
					pNewMenu->ModifyMenu(nMenuPos, MF_BYPOSITION, -1, plItem->m_csForeignLang);
				}
			}
		}
	}

	return true;
}

bool CMultiLanguage::UpdateWindowToLanguage(CWnd *pParent, LANGUAGE_ARRAY &Array)
{
	INT_PTR size = Array.GetSize();
	for(int i = 0; i < size; i++)
	{
		CLangItem *plItem = Array[i];

		if(plItem->m_csForeignLang.GetLength() > 0)
		{
			if(plItem->m_nID > 0)
			{
				CWnd *pWnd = pParent->GetDlgItem(plItem->m_nID);
				if(pWnd)
				{
					pWnd->SetWindowText(plItem->m_csForeignLang);
				}
			}
			//If item id is -1 then set the title for the dialog
			else if(plItem->m_nID == -1)
			{
				pParent->SetWindowText(plItem->m_csForeignLang);
			}
		}
	}

	return true;
}


CMenu * CMultiLanguage::GetMenuPos(CMenu *pMenu, const CString &csLookingForMenuText, int &nMenuPos)
{
	CMenu *pSubMenu;
	CString csMenuText;

	int nCount = pMenu->GetMenuItemCount();
	for(int i = 0; i < nCount; i++)
	{
		pMenu->GetMenuString(i, csMenuText, MF_BYPOSITION);

		if(csMenuText == csLookingForMenuText)
		{
			nMenuPos = i;
			return pMenu;
		}

		pSubMenu = pMenu->GetSubMenu(i);
		if(pSubMenu)
		{
			CMenu *pMenuReturn = GetMenuPos(pSubMenu, csLookingForMenuText, nMenuPos);
			if(pMenuReturn)
				return pMenuReturn;
		}
	}

	return NULL;
}

bool CMultiLanguage::LoadLanguageFile(CString csFile)
{
	m_csLastError = "";
	
	CString csPath = CGetSetOptions::GetPath(PATH_LANGUAGE);
	csPath += csFile;
	csPath += ".xml";

	ClearArrays();

	if(csFile.GetLength() <= 0)
	{
		m_csLastError = "Language file is blank";
		return false;
	}

	CStringA csPathA = CTextConvert::ConvertToChar(csPath);

	TiXmlDocument doc(csPathA);
	if(!doc.LoadFile())
	{
		m_csLastError.Format(_T("Error loading file %s - reason = %s"), csFile, CTextConvert::ConvertToUnicode(doc.ErrorDesc()));
		Log(m_csLastError);
		return false;
	}

	TiXmlElement *ItemHeader = doc.FirstChildElement("Ditto_Language_File");
	if(!ItemHeader)
	{
		m_csLastError.Format(_T("Error finding the section Ditto_Language_File"));
		ASSERT(!m_csLastError);
		Log(m_csLastError);
		return false;
	}

	CString csVersion = ItemHeader->Attribute("Version");
	m_lFileVersion = ATOI(csVersion);
	m_csAuthor = ItemHeader->Attribute("Author");
	m_csNotes = ItemHeader->Attribute("Notes");
	m_csLangCode = ItemHeader->Attribute("LanguageCode");

	if(m_bOnlyGetHeader)
		return true;

	bool bRet = LoadSection(*ItemHeader, m_RightClickMenu, "Ditto_Right_Click_Menu");
	bRet = LoadSection(*ItemHeader, m_OptionsGeneral, "Ditto_Options_General");
	bRet = LoadSection(*ItemHeader, m_ClipProperties, "Ditto_Clip_Properties");
	bRet = LoadSection(*ItemHeader, m_OptionsSupportedTypes, "Ditto_Options_Supported_Types");
	bRet = LoadSection(*ItemHeader, m_OptionsShortcuts, "Ditto_Options_Shortcuts");
	bRet = LoadSection(*ItemHeader, m_OptionsQuickPaste, "Ditto_Options_Quick_Paste");
	bRet = LoadSection(*ItemHeader, m_OptionsFriends, "Ditto_Options_Friends");
	bRet = LoadSection(*ItemHeader, m_OptionsFriendsDetail, "Ditto_Options_Friends_Detail");
	bRet = LoadSection(*ItemHeader, m_OptionsStats, "Ditto_Options_Stats");
	bRet = LoadSection(*ItemHeader, m_OptionsSupportedTypesAdd, "Ditto_Options_Supported_Types_Add");
	bRet = LoadSection(*ItemHeader, m_MoveToGroups, "Ditto_Move_To_Groups");
	bRet = LoadSection(*ItemHeader, m_OptionsSheet, "Ditto_Options_Sheet");
	bRet = LoadSection(*ItemHeader, m_TrayIconRightClickMenu, "Ditto_Tray_Icon_Menu");
	bRet = LoadSection(*ItemHeader, m_OptionsCopyBuffers, "Ditto_Options_CopyBuffers");
	bRet = LoadSection(*ItemHeader, m_GlobalHotKeys, "Ditto_GlobalHotKeys");
	
	bRet = LoadStringTableSection(*ItemHeader, m_StringMap, "Ditto_String_Table");

	return true;
}

bool CMultiLanguage::LoadSection(TiXmlNode &doc, LANGUAGE_ARRAY &Array, CString csSection)
{
	CStringA csSectionA = CTextConvert::ConvertToChar(csSection);
	TiXmlNode *node = doc.FirstChild(csSectionA);
	if(!node)
	{
		m_csLastError.Format(_T("Error finding the section %s"), csSection);
		ASSERT(!m_csLastError);
		Log(m_csLastError);
		return false;
	}

	TiXmlNode* ForeignNode;
	CString csID;
	CString csLineFeed("\n");
		
	TiXmlElement *ItemElement = node->FirstChildElement();

	//load all items for this section
	//they look like
	//<Item English_Text = "Use Ctrl - Num" ID= "32777"></Item>
	while(ItemElement)
	{
 		ForeignNode = ItemElement->FirstChild();
 		if(ForeignNode)
 		{
			CLangItem *plItem = new CLangItem;
			if(plItem)
			{
				plItem->m_csEnglishLang = ItemElement->Attribute("English_Text");
				csID = ItemElement->Attribute("ID");
				plItem->m_nID = ATOI(csID);
				if(plItem->m_nID == 0)
				{
					plItem->m_csID = csID;
				}
 				
				LPCSTR Value = ForeignNode->Value();
				CTextConvert::ConvertFromUTF8(Value, plItem->m_csForeignLang);

				//Replace the literal "\n" with line feeds
 				plItem->m_csForeignLang.Replace(_T("\\n"), csLineFeed);

				Array.Add(plItem);
			}
 		}		

		ItemElement = ItemElement->NextSiblingElement();
	}
	
	return true;
}

bool CMultiLanguage::LoadStringTableSection(TiXmlNode &doc, LANGUAGE_MAP &Map, CString csSection)
{
	CStringA csSectionA = CTextConvert::ConvertToChar(csSection);
	TiXmlNode *node = doc.FirstChild(csSectionA);
	if(!node)
	{
		CString cs;
		cs.Format(_T("Error finding the section %s"), csSection);
		ASSERT(!cs);
		Log(cs);
		return false;
	}

	CString csLineFeed("\n");
	TiXmlNode* ForeignNode;
		
	TiXmlElement *ItemElement = node->FirstChildElement();

	//load all items for this section
	//they look like
	//<Item English_Text = "Use Ctrl - Num" ID= "32777"></Item>
	while(ItemElement)
	{
		CLangItem *plItem = new CLangItem;

		plItem->m_csEnglishLang = ItemElement->Attribute("English_Text");
		plItem->m_csID = ItemElement->Attribute("ID");

		ForeignNode = ItemElement->FirstChild();
		if(ForeignNode) 
		{
			LPCSTR Value = ForeignNode->Value();
			CTextConvert::ConvertFromUTF8(Value, plItem->m_csForeignLang);

			//Replace the literal "\n" with line feeds
			plItem->m_csForeignLang.Replace(_T("\\n"), csLineFeed);
		}

		Map.SetAt(plItem->m_csID, plItem);

		ItemElement = ItemElement->NextSiblingElement();
	}

	return true;
}
