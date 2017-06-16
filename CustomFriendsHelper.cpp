#include "stdafx.h"
#include "CustomFriendsHelper.h"
#include "CP_Main.h"
#include "Shared\Tokenizer.h"
#include "Shared\TextConvert.h"


CCustomFriendsHelper::CCustomFriendsHelper()
{
}


CCustomFriendsHelper::~CCustomFriendsHelper()
{
}

void CCustomFriendsHelper::Load()
{
	m_list.clear();

	CString oldValues = CGetSetOptions::GetCustomSendToList();

	TiXmlDocument doc;
	CStringA xmlA;
	CTextConvert::ConvertToUTF8(oldValues, xmlA);
	doc.Parse(xmlA);

	TiXmlElement *ItemHeader = doc.FirstChildElement("CustomFriends");

	if (ItemHeader != NULL)
	{
		TiXmlElement *ItemElement = ItemHeader->FirstChildElement();

		while (ItemElement)
		{
			Name_Desc array_item;
			array_item.m_name = ItemElement->Attribute("name");
			array_item.m_desc = ItemElement->Attribute("desc");
			
			m_list.push_back(array_item);

			ItemElement = ItemElement->NextSiblingElement();
		}
	}
}

void CCustomFriendsHelper::Save()
{
	/*CString values = _T("");

	int count = m_list.size();
	for (int i = 0; i < count; i++)
	{
		CString lineValue = m_list[i];
		values += _T(",");
		values += lineValue;
	}*/

	TiXmlDocument doc;

	TiXmlElement* friendOuter = new TiXmlElement("CustomFriends");
	doc.LinkEndChild(friendOuter);

	for (auto & listItem : m_list)
	{
		TiXmlElement* friendElement = new TiXmlElement("Friend");

		CStringA nameA;
		CTextConvert::ConvertToUTF8(listItem.m_name, nameA);
		friendElement->SetAttribute("name",  nameA);

		CStringA descA;
		CTextConvert::ConvertToUTF8(listItem.m_desc, descA);
		friendElement->SetAttribute("desc", descA);

		friendOuter->LinkEndChild(friendElement);
	}
	
	TiXmlPrinter printer;
	doc.Accept(&printer);
	CString cs = printer.CStr();

	CGetSetOptions::SetCustomSendToList(cs);
}

void CCustomFriendsHelper::AddToMenu(CMenu *pMenu)
{
	bool addedItem = false;
	int id = 0;
	for (auto & element : m_list) 
	{
		if (addedItem == false)
		{
			addedItem = true;
		}

		CString cs;
		if (element.m_desc != _T(""))
		{
			cs.Format(_T("(%s) - %s"), element.m_name, element.m_desc);
		}
		else
		{
			cs.Format(_T("%s"), element.m_name);
		}

		pMenu->AppendMenuW(MF_STRING, (CustomFriendStartId + id), cs);
		id++;
	}

	if (addedItem)
	{
		pMenu->AppendMenu(MF_SEPARATOR);
	}
	
	pMenu->AppendMenuW(MF_STRING, (CustomFriendStartId + PromptForCustom), _T("Prompt For Name"));	
}

void CCustomFriendsHelper::Add(CString item, CString desc)
{
	int count = m_list.size();
	if (count < MaxCustomFriends)
	{
		Name_Desc array_item;
		array_item.m_name = item;
		array_item.m_desc = desc;
		m_list.push_back(array_item);
		Save();
	}
}

CString CCustomFriendsHelper::GetSendTo(int id, bool &showDlg)
{
	int index = id - CustomFriendStartId;
	if (index >= 0 && index < m_list.size())
	{
		return m_list[index].m_name;
	}

	if (index == PromptForCustom)
	{
		showDlg = true;
	}

	return _T("");
}

void CCustomFriendsHelper::ClearList()
{
	m_list.clear();
	this->Save();
}