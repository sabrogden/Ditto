#pragma once

#include "DittoChaiScript.h"
#include <string>

class ChaiScriptOnCopy
{
public:
	ChaiScriptOnCopy();
	~ChaiScriptOnCopy();

	bool ProcessScript(CDittoChaiScript &clipData, std::string script);

	CString m_lastError;
};

