#pragma once

#include "DittoAddin.h"
#include <vector>
#include <afxtempl.h>

class CDittoAddins
{
public:
	CDittoAddins(void);
	~CDittoAddins(void);

	bool LoadAll();
	bool UnloadAll();

	bool Loaded()	{ return m_Addins.size() > 0; }

	bool AddPrePasteAddinsToMenu(CMenu *pMenu);
	bool CallPrePasteFunction(int Id, IClip *pClip);
	void AboutScreenText(CStringArray &arr);

protected:
	std::vector<CDittoAddin*> m_Addins;

	class CFunctionLookup
	{
	public:
		CDittoAddin *m_pAddin;
		CStringA m_csFunctionName;
	};

	CMap<int, int, CFunctionLookup, CFunctionLookup> m_FunctionMap;

protected:
	void LoadDittoInfo(CDittoInfo &DittoInfo);
};
