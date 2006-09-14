#pragma once

class CTextConvert
{
public:

	CTextConvert();
	~CTextConvert();

	static bool ConvertFromUTF8(const CStringA &src, CString &dest);
	static bool ConvertToUTF8(const CString &src, CStringA &dest);

	static CStringA UnicodeStringToMultiByte(const CStringW &srcString);
	static CStringW MultiByteToUnicodeString(const CStringA &srcString);

	static CStringA ConvertToChar(const CString &src);

protected:
};