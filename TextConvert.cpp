#include "StdAfx.h"
#include "TextConvert.h"

static BYTE kUtf8Limits[5] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

CTextConvert::CTextConvert()
{
}

CTextConvert::~CTextConvert()
{
}

CStringW CTextConvert::MultiByteToUnicodeString(const CStringA &srcString)
{
	CStringW resultString;

	if(!srcString.IsEmpty())
	{
		int numChars = MultiByteToWideChar(CP_ACP, 0, srcString, 
						srcString.GetLength(), resultString.GetBuffer(srcString.GetLength()), 
						srcString.GetLength() + 1);

		resultString.ReleaseBuffer(numChars);
	}
	return resultString;
}

CStringA CTextConvert::UnicodeStringToMultiByte(const CStringW &srcString)
{
	CStringA resultString;
	if(!srcString.IsEmpty())
	{
		int numRequiredBytes = srcString.GetLength() * sizeof(wchar_t);
		int numChars = WideCharToMultiByte(CP_ACP, 0, srcString, 
							srcString.GetLength(), resultString.GetBuffer(numRequiredBytes), 
							numRequiredBytes + 1, NULL, NULL);

		resultString.ReleaseBuffer(numChars);
	}

	return resultString;
}

CStringA CTextConvert::ConvertToChar(const CString &src)
{
#ifdef _UNICODE
	return UnicodeStringToMultiByte(src);
#else
	return src;
#endif
}

bool CTextConvert::ConvertFromUTF8(const CStringA &src, CString &dest)
{
#ifdef _UNICODE	
	dest.Empty();
	for(int i = 0; i < src.GetLength();)
	{
		BYTE c = (BYTE)src[i++];
		if (c < 0x80)
		{
			dest += (wchar_t)c;
			continue;
		}
		if(c < 0xC0)
		{
			dest = src;
			return false;
		}
		int numAdds;
		for (numAdds = 1; numAdds < 5; numAdds++)
			if (c < kUtf8Limits[numAdds])
				break;
		
		UINT value = (c - kUtf8Limits[numAdds - 1]);
		do
		{
			if (i >= src.GetLength())
			{
				dest = src;
				return false;
			}
			BYTE c2 = (BYTE)src[i++];
			if (c2 < 0x80 || c2 >= 0xC0)
			{
				dest = src;
				return false;
			}
			value <<= 6;
			value |= (c2 - 0x80);
			numAdds--;
		}while(numAdds > 0);

		if (value < 0x10000)
		{
			dest += (wchar_t)(value);
		}
		else
		{
			value -= 0x10000;
			if (value >= 0x100000)
			{
				dest = src;
				return false;
			}
			dest += (wchar_t)(0xD800 + (value >> 10));
			dest += (wchar_t)(0xDC00 + (value & 0x3FF));
		}
	}
#else
	dest = src;
#endif
	return true; 
}

bool CTextConvert::ConvertToUTF8(const CString &src, CStringA &dest)
{
#ifdef _UNICODE
	dest.Empty();
	for(int i = 0; i < src.GetLength();)
	{
		UINT value = (UINT)src[i++];
		if (value < 0x80)
		{
			dest += (char)value;
			continue;
		}
		if (value >= 0xD800 && value < 0xE000)
		{
			if (value >= 0xDC00)
			{
				dest = src;
				return false;
			}
			if (i >= src.GetLength())
			{
				dest = src;
				return false;
			}
			UINT c2 = (UINT)src[i++];
			if (c2 < 0xDC00 || c2 >= 0xE000)
			{
				dest = src;
				return false;
			}
			value = ((value - 0xD800) << 10) | (c2 - 0xDC00);
		}
		int numAdds;
		for (numAdds = 1; numAdds < 5; numAdds++)
			if (value < (((UINT)1) << (numAdds * 5 + 6)))
				break;
		dest += (char)(kUtf8Limits[numAdds - 1] + (value >> (6 * numAdds)));
		do
		{
			numAdds--;
			dest += (char)(0x80 + ((value >> (6 * numAdds)) & 0x3F));
		}
		while(numAdds > 0);
	}
#else
	dest = src;
#endif
	return true;
}