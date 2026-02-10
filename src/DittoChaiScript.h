#pragma once

#include "..\Shared\IClip.h"
#include <string>
#include <map>
#include <vector>


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

	// 模板变量管理
	std::map<std::string, std::string> GetTemplateVariables();
	void SetTemplateVariables(std::map<std::string, std::string> vars);
	std::string ReplaceTemplateVariables(std::string text);

	// 内置变量
	std::string GetCurrentDate();
	std::string GetCurrentTime();
	std::string GetCurrentDateTime();
	std::string GetUserName();
	std::string GetComputerName();
	std::string GenerateGUID();
	std::string GetClipboardText();

	// 模板检测
	bool IsTemplate(std::string text);
	std::vector<std::string> ExtractVariables(std::string text);

private:
	std::map<std::string, std::string> m_templateVariables;
};

