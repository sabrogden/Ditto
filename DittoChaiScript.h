#pragma once

#include "shared/IClip.h"
#include <string>


class CDittoChaiScript
{
public:
	CDittoChaiScript(IClip *pClip, std::string activeApp);
	~CDittoChaiScript();

	IClip *m_pClip;
	std::string m_activeApp;

	std::string GetAsciiString();
	void SetAsciiString(std::string stringVal);

	std::string GetClipMD5(std::string clipboardFormat);
	SIZE_T GetClipSize(std::string clipboardFormat);

	std::string GetActiveApp() { return m_activeApp; }

	BOOL RemoveFormat(std::string clipboardFormat);
	BOOL FormatExists(std::string clipboardFormat);
	BOOL SetParentId(int parentId);
	BOOL AsciiTextMatchesRegex(std::string regex);
};

