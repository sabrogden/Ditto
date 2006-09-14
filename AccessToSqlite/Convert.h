#pragma once

#include "Clip.h"
#include "MainTable.h"
#include "../progresswnd.h"

class CConvert
{
public:
	CConvert(void);
	~CConvert(void);

	bool ConvertGroups(long lParentID);
	bool ConvertNonGroups();
	bool ConvertTypes();

	void SetupProgressWnd();

protected:
	bool LoadFormats(long lID, CClipFormats& formats);
	bool ConvertEntry(CMainTable &recset);

	CMap<long, long, long, long> m_ParentMap;

	CProgressWnd m_Progress;
};
