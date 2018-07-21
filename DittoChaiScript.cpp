#include "stdafx.h"
#include "DittoChaiScript.h"
#include "Shared\TextConvert.h"
#include "Md5.h"
#include "Misc.h"
#include "CP_Main.h"

#include <regex>

CDittoChaiScript::CDittoChaiScript(IClip *pClip, std::string activeApp)
{
	m_pClip = pClip;
	m_activeApp = activeApp;
}


CDittoChaiScript::~CDittoChaiScript()
{
}


std::string CDittoChaiScript::GetAsciiString()
{
	std::string s = "";
	if (m_pClip)
	{
		IClipFormat *pFormat = m_pClip->Clips()->FindFormatEx(CF_TEXT);
		if (pFormat)
		{
			char *stringData = (char *)GlobalLock(pFormat->Data());
			if (stringData != NULL)
			{
				s = stringData;

				GlobalUnlock(pFormat->Data());
			}
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
		int formatId = GetFormatID(CTextConvert::MultiByteToUnicodeString(clipboardFormat.c_str()));

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
		int formatId = GetFormatID(CTextConvert::MultiByteToUnicodeString(clipboardFormat.c_str()));

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
		int formatId = GetFormatID(CTextConvert::MultiByteToUnicodeString(clipboardFormat.c_str()));

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
		int formatId = GetFormatID(CTextConvert::MultiByteToUnicodeString(clipboardFormat.c_str()));

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

	auto ascii = GetAsciiString();
	std::regex integer(regex);
	if (regex_match(ascii, integer))
	{
		matches = true;
	}
	return matches;
}