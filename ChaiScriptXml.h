#pragma once

#include <vector>

#define ChaiScriptMenuStartId 5001
#define MaxChaiScripts 99

class CDittoChaiScriptXmlItem
{
public:
	CDittoChaiScriptXmlItem()
	{
		m_active = FALSE;
		m_version = "1.0.0.0";
	}
	BOOL m_active;
	CString m_name;
	CString m_description;
	CString m_script;
	CString m_guid;
	CString m_version;
};

class CChaiScriptXml
{
public:
	CChaiScriptXml();
	~CChaiScriptXml();

	CString Save();
	void Load(CString values);
	CString GetScript(CString name, BOOL &active);	
	void AddToMenu(CMenu *pMenu);

	std::vector<CDittoChaiScriptXmlItem> m_list;
	bool m_assignedGuidOnLoad;
};