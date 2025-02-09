#pragma once

#include "..\Shared\IClip.h"
#include <string>


class CDittoChaiScript
{
public:
	CDittoChaiScript(IClip *pClip, std::string activeApp, std::string activeAppTitle);
	~CDittoChaiScript();

	IClip *m_pClip;
	std::string m_activeApp;
	std::string m_activeAppTitle;

	std::string GetAsciiString();
	void SetAsciiString(std::string stringVal);

	std::wstring CDittoChaiScript::GetUnicodeString();
	void SetUnicodeString(std::wstring stringVal);

	std::string GetClipMD5(std::string clipboardFormat);
	SIZE_T GetClipSize(std::string clipboardFormat);

	std::string GetActiveApp() { return m_activeApp; }
	std::string GetActiveAppTitle() { return m_activeAppTitle; }

	BOOL RemoveFormat(std::string clipboardFormat);
	BOOL FormatExists(std::string clipboardFormat);
	BOOL SetParentId(int parentId);
	BOOL AsciiTextMatchesRegex(std::string regex);
	void AsciiTextReplaceRegex(std::string regex, std::string replaceWith);

	BOOL DescriptionMatchesRegex(std::string regex);
	void DescriptionReplaceRegex(std::string regex, std::string replaceWith);

	void SetMakeTopSticky();
	void SetMakeLastSticky();
	void SetReplaceTopSticky();
};

