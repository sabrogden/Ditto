#include "stdafx.h"
#include "CP_Main.h"
#include "RegExFilterHelper.h"
#include "Shared\Tokenizer.h"
#include "Options.h"
#include "WildCardMatch.h"

#include <regex>
#include <string>

void CRegExFilterData::ParseFilters()
{
	m_parsedProcessFilters.RemoveAll();

	CTokenizer token(m_processFilters, CGetSetOptions::GetCopyAppSeparator());
	CString line;

	while (token.Next(line))
	{
		if (line != "")
		{
			m_parsedProcessFilters.Add(line);
		}
	}
}

bool CRegExFilterData::MatchesProcessFilters(CString &activeApp)
{
	if (activeApp == _T(""))
	{
		return true;
	}

	int count = m_parsedProcessFilters.GetCount();

	if (count == 0)
	{
		return true;
	}

	for (int i = 0; i < count; i++)
	{
		if (CWildCardMatch::WildMatch(m_parsedProcessFilters[i], activeApp, ""))
		{
			return true;
		}
	}

	return false;
}

bool CRegExFilterData::MatchesRegEx(std::wstring &copiedText)
{
	//std::wregex integer(_T("(\\+|-)?[[:digit:]]+"));
	//std::wstring input(copiedText);
	if (m_regEx != _T(""))
	{
		try 
		{
			std::wregex integer(m_regEx);
			if (regex_match(copiedText, integer))
			{
				return true;
			}
		}
		catch (regex_error e) 
		{
			CString w(e.what());
			Log(StrF(_T("MatchesRegEx exception: %s, Code Is: %d"), w, e.code()));
		}
	}

	return false;
}


CRegExFilterHelper::CRegExFilterHelper()
{
}


CRegExFilterHelper::~CRegExFilterHelper()
{
}

void CRegExFilterHelper::Add(int pos, CRegExFilterData &data)
{
	if (pos >= 0 && pos < MAX_REGEX_FILTERS)
	{
		ATL::CCritSecLock csLock(m_critSection.m_sect);
		m_filters[pos] = data;
	}
}

void CRegExFilterHelper::SetRegEx(int pos, std::wstring regEx)
{
	if (pos >= 0 && pos < MAX_REGEX_FILTERS)
	{
		ATL::CCritSecLock csLock(m_critSection.m_sect);
		m_filters[pos].m_regEx = regEx;
	}
}

void CRegExFilterHelper::SetProcessFilter(int pos, CString processName)
{
	if (pos >= 0 && pos < MAX_REGEX_FILTERS)
	{
		ATL::CCritSecLock csLock(m_critSection.m_sect);
		m_filters[pos].m_processFilters = processName;
		m_filters[pos].ParseFilters();
	}
}

bool CRegExFilterHelper::TextMatchFilters(CString &activeApp, std::wstring &copiedText)
{
	ATL::CCritSecLock csLock(m_critSection.m_sect);

	for (int i = 0; i < MAX_REGEX_FILTERS; i++)
	{
		if (m_filters[i].MatchesProcessFilters(activeApp))
		{
			if (m_filters[i].MatchesRegEx(copiedText))
			{
				Log(StrF(_T("regex matches copied text NOT SAVING CLIP, regex: %s, text: %s, active app: %s"), m_filters[i].m_regEx.c_str(), copiedText.c_str(), activeApp));
				return true;
			}
		}
	}

	return false;
}