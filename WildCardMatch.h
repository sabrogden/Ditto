#pragma once
class CWildCardMatch
{
public:
	CWildCardMatch(void);
	~CWildCardMatch(void);

	static BOOL WildMatch(CString sWild, CString sString, CString sLimitChar);
};

