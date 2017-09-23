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

	std::string GetClipMD5(int clipboardFormat);
	SIZE_T GetClipSize(int clipboardFormat);

	std::string GetActiveApp() { return m_activeApp; }
};

