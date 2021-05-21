#include "stdafx.h"
#include "DittoChaiScript.h"
#include "Shared\TextConvert.h"
#include "Md5.h"
#include "Misc.h"
#include "CP_Main.h"
#include "Shared\TextConvert.h"

#include <regex>

CDittoChaiScript::CDittoChaiScript(IClip *pClip, std::string activeApp, std::string activeAppTitle)
{
	m_pClip = pClip;
	m_activeApp = activeApp;
	m_activeAppTitle = activeAppTitle;
}


CDittoChaiScript::~CDittoChaiScript()
{
}

std::wstring CDittoChaiScript::GetUnicodeString()
{
	std::wstring s = _T("");
	if (m_pClip)
	{
		IClipFormat* pFormat = m_pClip->Clips()->FindFormatEx(CF_UNICODETEXT);
		if (pFormat)
		{
			s = pFormat->GetAsCString();
		}
		else
		{
			s = CTextConvert::AnsiToUnicode(GetAsciiString().c_str());
		}
	}

	return s;
}

void CDittoChaiScript::SetUnicodeString(std::wstring stringVal)
{
	if (m_pClip)
	{
		m_pClip->Clips()->DeleteAll();

		HGLOBAL hGlobal = ::NewGlobalP((LPVOID)stringVal.c_str(), (stringVal.size() + 1) * 2);
		ASSERT(hGlobal);

		m_pClip->Clips()->AddNew(CF_UNICODETEXT, hGlobal);
	}
}

std::string CDittoChaiScript::GetAsciiString()
{
	std::string s = "";
	if (m_pClip)
	{
		IClipFormat* pFormat = m_pClip->Clips()->FindFormatEx(CF_TEXT);
		if (pFormat)
		{
			s = pFormat->GetAsCStringA();
		}
	}

	return s;
}

void CDittoChaiScript::SetAsciiString(std::string stringVal)
{
	if (m_pClip)
	{
		m_pClip->Clips()->DeleteAll();

		HGLOBAL hGlobal = ::NewGlobalP((LPVOID)stringVal.c_str(), stringVal.size()+1);
		ASSERT(hGlobal);

		m_pClip->Clips()->AddNew(CF_TEXT, hGlobal);
	}
}



std::string CDittoChaiScript::GetClipMD5(std::string clipboardFormat)
{
	CMd5 md5;
	md5.MD5Init();

	std::string md5String;

	if (m_pClip)
	{
		int formatId = GetFormatID(CTextConvert::AnsiToUnicode(clipboardFormat.c_str()));

		IClipFormat *pFormat = m_pClip->Clips()->FindFormatEx(formatId);
		if (pFormat)
		{
			SIZE_T size = ::GlobalSize(pFormat->Data());
			void* pv = GlobalLock(pFormat->Data());
			if (pv != NULL)
			{				
				md5.MD5Update((unsigned char*)pv, (unsigned int)size);

				GlobalUnlock(pFormat->Data());

				md5String = md5.MD5FinalToString();
			}
		}
	}

	return md5String;
}

SIZE_T CDittoChaiScript::GetClipSize(std::string clipboardFormat)
{
	SIZE_T size = 0;
	if (m_pClip)
	{
		int formatId = GetFormatID(CTextConvert::AnsiToUnicode(clipboardFormat.c_str()));

		IClipFormat *pFormat = m_pClip->Clips()->FindFormatEx(formatId);
		if (pFormat)
		{
			size = ::GlobalSize(pFormat->Data());
		}
	}

	return size;
}

BOOL CDittoChaiScript::FormatExists(std::string clipboardFormat)
{
	BOOL exists = FALSE;
	if (m_pClip)
	{
		int formatId = GetFormatID(CTextConvert::AnsiToUnicode(clipboardFormat.c_str()));

		IClipFormat *pFormat = m_pClip->Clips()->FindFormatEx(formatId);
		if (pFormat)
		{
			exists = TRUE;
		}
	}

	return exists;
}

BOOL CDittoChaiScript::RemoveFormat(std::string clipboardFormat)
{
	BOOL removed = FALSE;
	if (m_pClip)
	{
		int formatId = GetFormatID(CTextConvert::AnsiToUnicode(clipboardFormat.c_str()));

		if (m_pClip->Clips()->RemoveFormat(formatId))
		{
			removed = TRUE;
		}
	}

	return removed;
}

BOOL CDittoChaiScript::SetParentId(int parentId)
{
	BOOL set = FALSE;
	if (m_pClip)
	{		
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Main WHERE lID = %d"), parentId);
		if (q.eof() == false)
		{
			m_pClip->Parent(parentId);			
		}
	}

	return set;
}

BOOL CDittoChaiScript::AsciiTextMatchesRegex(std::string regex)
{
	BOOL matches = false;

	auto ascii = GetUnicodeString();
	std::wregex integer(CTextConvert::AnsiToUnicode(regex.c_str()));
	if (regex_match(ascii, integer))
	{
		matches = true;
	}
	return matches;
}

void CDittoChaiScript::AsciiTextReplaceRegex(std::string regex, std::string replaceWith)
{
	if (AsciiTextMatchesRegex(regex))
	{
		CStringA ascii = CTextConvert::UnicodeToUTF8(GetUnicodeString().c_str());
		std::regex integer(regex.c_str());

		auto newAscii = std::regex_replace(ascii.GetBuffer(), integer, replaceWith);
		SetUnicodeString(CTextConvert::Utf8ToUnicode(newAscii.c_str()).GetBuffer());
	}
}


void CDittoChaiScript::SetMakeTopSticky()
{
	m_pClip->SetSaveToDbSticky(AddToDbStickyEnum::MAKE_LAST_STICKY);
}

void CDittoChaiScript::SetMakeLastSticky()
{
	m_pClip->SetSaveToDbSticky(AddToDbStickyEnum::MAKE_LAST_STICKY);
}

void CDittoChaiScript::SetReplaceTopSticky()
{
	m_pClip->SetSaveToDbSticky(AddToDbStickyEnum::REPLACE_TOP_STICKY);
}

BOOL CDittoChaiScript::DescriptionMatchesRegex(std::string regex)
{
	BOOL matches = false;

	if (m_pClip)
	{
		std:string ascii(CTextConvert::UnicodeToAnsi(m_pClip->Description()).GetBuffer());
		std::regex integer(regex);
		if (regex_match(ascii, integer))
		{
			matches = true;
		}
	}

	return matches;
}

void CDittoChaiScript::DescriptionReplaceRegex(std::string regex, std::string replaceWith)
{
	if (m_pClip)
	{
		std:string ascii(CTextConvert::UnicodeToAnsi(m_pClip->Description()).GetBuffer());
		std::regex integer(regex);

		auto newAscii = regex_replace(ascii, integer, replaceWith);

		CString cstr(newAscii.c_str());
		m_pClip->Description(cstr);
	}
}