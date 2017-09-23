#include "stdafx.h"
#include "ChaiScriptXml.h"

#include "tinyxml\tinyxml.h"

#include "Shared\TextConvert.h"


CChaiScriptXml::CChaiScriptXml()
{
}


CChaiScriptXml::~CChaiScriptXml()
{
}

CString CChaiScriptXml::GetScript(CString name, BOOL &active)
{
	CString script;
	for (auto & listItem : m_list)
	{
		if (listItem.m_name == name)
		{
			active = listItem.m_active;
			script = listItem.m_name;
			break;
		}
	}

	return script;
}

void CChaiScriptXml::Load(CString values)
{
	m_list.clear();
	
	TiXmlDocument doc;
	CStringA xmlA;
	CTextConvert::ConvertToUTF8(values, xmlA);
	doc.Parse(xmlA);

	TiXmlElement *ItemHeader = doc.FirstChildElement("ChaiScripts");

	if (ItemHeader != NULL)
	{
		TiXmlElement *ItemElement = ItemHeader->FirstChildElement();

		while (ItemElement)
		{
			CDittoChaiScriptXmlItem array_item;
			ItemElement->Attribute("active", &array_item.m_active);
			array_item.m_name = ItemElement->Attribute("name");
			array_item.m_description = ItemElement->Attribute("description");
			array_item.m_script = ItemElement->Attribute("script");

			m_list.push_back(array_item);

			ItemElement = ItemElement->NextSiblingElement();
		}
	}
}

CString CChaiScriptXml::Save()
{
	TiXmlDocument doc;

	TiXmlElement* friendOuter = new TiXmlElement("ChaiScripts");
	doc.LinkEndChild(friendOuter);

	for (auto & listItem : m_list)
	{
		TiXmlElement* friendElement = new TiXmlElement("ChaiScriptItem");

		friendElement->SetAttribute("active", listItem.m_active);

		CStringA name;
		CTextConvert::ConvertToUTF8(listItem.m_name, name);
		friendElement->SetAttribute("name", name);

		CStringA desc;
		CTextConvert::ConvertToUTF8(listItem.m_description, desc);
		friendElement->SetAttribute("description", desc);

		CStringA script;
		CTextConvert::ConvertToUTF8(listItem.m_script, script);
		friendElement->SetAttribute("script", script);

		friendOuter->LinkEndChild(friendElement);
	}

	TiXmlPrinter printer;
	printer.SetLineBreak("");
	doc.Accept(&printer);
	CString cs = printer.CStr();

	return cs;
}