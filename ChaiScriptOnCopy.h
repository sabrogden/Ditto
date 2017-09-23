#pragma once

#include "Shared\IClip.h"
#include <string>

class ChaiScriptOnCopy
{
public:
	ChaiScriptOnCopy();
	~ChaiScriptOnCopy();

	bool ProcessScript(IClip *pClip, std::string script, std::string activeApp);

	CString m_lastError;
};

