#include "stdafx.h"
#include "ICU_String.h"
#include "Misc.h"


CICU_String::CICU_String()
{
	m_dllHandle = NULL;
	u_isUUppercase = NULL;
	u_tolower = NULL;
	u_toupper = NULL;
	u_strToLower = NULL;
	u_strToUpper = NULL;
}

CICU_String::~CICU_String()
{
	if (m_dllHandle)
	{
		::FreeLibrary(m_dllHandle);
		m_dllHandle = NULL;
	}
}

bool CICU_String::Load()
{
	bool loaded = false;
	m_dllHandle = ::LoadLibrary(_T("icu.dll"));
	if (m_dllHandle != NULL)
	{
		u_isUUppercase = (bool(__cdecl*)(wchar_t c))GetProcAddress(m_dllHandle, "u_isUUppercase");
		u_tolower = (wchar_t(__cdecl*)(wchar_t c))GetProcAddress(m_dllHandle, "u_tolower");
		u_toupper = (wchar_t(__cdecl*)(wchar_t c))GetProcAddress(m_dllHandle, "u_toupper");
		u_strToLower = (int(__cdecl*)(wchar_t* dest, int destCapacity, const wchar_t* src, int srcLength, const char* locale, int* pErrorCode))GetProcAddress(m_dllHandle, "u_strToLower");
		u_strToUpper = (int(__cdecl*)(wchar_t* dest, int destCapacity, const wchar_t* src, int srcLength, const char* locale, int* pErrorCode))GetProcAddress(m_dllHandle, "u_strToUpper");

		Log(_T("Loaded icu.dll, this will be used for upper/lower case calls"));

		loaded = true;
	}
	else
	{
		Log(StrF(_T("Error loading icu.dll, LastError: %d"), ::GetLastError()));
	}

	return loaded;
}

bool CICU_String::IsUpperEx(wchar_t c)
{
	if (m_dllHandle == NULL || u_tolower == NULL)
	{
		return u_isUUppercase(c);
	}

	return ::isupper(c);
}

wchar_t CICU_String::ToLowerEx(wchar_t c)
{
	if (m_dllHandle == NULL || u_tolower == NULL)
	{
		return u_tolower(c);
	}

	return ::tolower(c);
}

wchar_t CICU_String::ToUpperEx(wchar_t c)
{
	if (m_dllHandle == NULL || u_tolower == NULL)
	{
		return u_toupper(c);
	}

	return ::toupper(c);
}

CString CICU_String::ToLowerStringEx(CString source)
{
	if (m_dllHandle == NULL || u_tolower == NULL)
	{
		return CString(source).MakeLower();
	}

	CString dest;

	int length = source.GetLength();
	int errorCode;
	u_strToLower(dest.GetBufferSetLength(length), length, source.GetBuffer(), length, NULL, &errorCode);

	source.ReleaseBuffer();
	dest.ReleaseBuffer();

	return dest;
}

CString CICU_String::ToUpperStringEx(CString source)
{
	if (m_dllHandle == NULL || u_tolower == NULL)
	{
		return CString(source).MakeUpper();
	}

	CString dest;

	int length = source.GetLength();
	int errorCode;
	u_strToUpper(dest.GetBufferSetLength(length), length, source.GetBuffer(), length, NULL, &errorCode);

	source.ReleaseBuffer();
	dest.ReleaseBuffer();

	return dest;
}