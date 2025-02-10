#pragma once

#include <vector>
#include <string>

#define MAX_REGEX_FILTERS 15

class CRegExFilterData
{
public:
	std::wstring m_regEx;
	CString m_processFilters;
	CStringArray m_parsedProcessFilters;

	void ParseFilters();

	bool MatchesProcessFilters(CString &activeApp);
	bool MatchesRegEx(std::wstring &copiedText);

	const CRegExFilterData& operator=(const CRegExFilterData &clip)
	{
		m_regEx = clip.m_regEx;
		m_processFilters = clip.m_processFilters;

		ParseFilters();
		
		return *this;
	}
};

class CRegExFilterHelper
{
public:
	CRegExFilterHelper();
	~CRegExFilterHelper();

	void Add(int pos, CRegExFilterData &data);
	void SetRegEx(int pos, std::wstring regEx);
	void SetProcessFilter(int pos, CString processName);

	CRegExFilterData m_filters[MAX_REGEX_FILTERS];

	bool TextMatchFilters(CString &activeApp, std::wstring &copiedText);

private:
	CCriticalSection m_critSection;
};

