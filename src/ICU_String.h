#pragma once
class CICU_String
{
public:

	CICU_String();
	virtual ~CICU_String();

	bool Load();

	bool IsUpperEx(wchar_t c);
	wchar_t ToLowerEx(wchar_t c);
	wchar_t ToUpperEx(wchar_t c);

	CString ToLowerStringEx(CString source);
	CString ToUpperStringEx(CString source);

private:

	HMODULE m_dllHandle;

	bool(__cdecl* u_isUUppercase)(wchar_t c);
	wchar_t(__cdecl* u_tolower)(wchar_t c);
	wchar_t(__cdecl* u_toupper)(wchar_t c);

	int(__cdecl* u_strToLower)(wchar_t* dest, int destCapacity, const wchar_t* src, int srcLength, const char* locale, int* pErrorCode);
	int(__cdecl* u_strToUpper)(wchar_t* dest, int destCapacity, const wchar_t* src, int srcLength, const char* locale, int* pErrorCode);
};

