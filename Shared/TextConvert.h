#pragma once

class CTextConvert
{
public:

	static CStringA UnicodeToAnsi(CString unicode)
	{
		CStringA ansi = CW2A(unicode, CP_ACP);
		return ansi;
	}

	static CStringA UnicodeToUTF8(CString unicode)
	{
		CStringA utf8 = CW2A(unicode, CP_UTF8);
		return utf8;
	}

	static CString AnsiToUnicode(CStringA ansi)
	{
		CString unicode = CA2W(ansi);
		return unicode;
	}

	static CString Utf8ToUnicode(CStringA ansi)
	{
		CString unicode = CA2W(ansi, CP_UTF8);
		return unicode;
	}

protected:
};