#include "StdAfx.h"
#include "ReadOnlyFlag.h"
#include "../../Shared/Tokenizer.h"
#include "../../Shared/TextConvert.h"


CReadOnlyFlag::CReadOnlyFlag(void)
{
}


CReadOnlyFlag::~CReadOnlyFlag(void)
{
}


bool CReadOnlyFlag::ResetReadOnlyFlag(const CDittoInfo &DittoInfo, IClip *pClip, bool resetFlag)
{
	IClipFormats *pFormats = pClip->Clips();
	if(pFormats)
	{
		CStringArray lines;

		LoadHDropFiles(lines, pFormats);

		if(lines.GetSize() <= 0)
		{
			LoadUnicodeFiles(lines, pFormats);
		}

		if(lines.GetSize() <= 0)
		{
			LoadTextFiles(lines, pFormats);
		}

		for(int i = 0; i < lines.GetSize(); i++)
		{
			CString file = lines[i].TrimLeft(' ').TrimRight(' ').MakeLower();

			//Find the first occurance of a file // or \\ for a network file or a->z:\\ for a local files
			int pos = file.Find(_T("//"));
			if(pos >= 0)
			{
				file = file.Mid(pos);
			}
			else
			{
				pos = file.Find(_T("\\\\"));
				if(pos >= 0)
				{
					file = file.Mid(pos);
				}
				else
				{
					for(wchar_t drive = 'a'; drive <= 'z'; drive++)
					{
						CString csDrive(drive);
						csDrive += _T(":\\");

						pos = file.Find(csDrive);
						if(pos >= 0)
						{
							file = file.Mid(pos);
							break;
						}
					}
				}
			}

			if(resetFlag)
			{
				::SetFileAttributes(file, FILE_ATTRIBUTE_NORMAL);
			}
			else
			{
				::SetFileAttributes(file, FILE_ATTRIBUTE_READONLY);
			}
		}
	}

	return true;
}

bool CReadOnlyFlag::LoadUnicodeFiles(CStringArray &lines, IClipFormats *pFormats)
{		
	IClipFormat *pFormat = pFormats->FindFormatEx(CF_UNICODETEXT);
	if(pFormat != NULL)
	{
		wchar_t *stringData = (wchar_t *)GlobalLock(pFormat->Data());
		if(stringData != NULL)
		{
			CString string(stringData);
			CString delim(_T("\r\n"));

			CTokenizer token(string, delim);
			CString line;
			while(token.Next(line))
			{
				lines.Add(line);
			}

			GlobalUnlock(pFormat->Data());
		}
	}

	return lines.GetSize() > 0;
}

bool CReadOnlyFlag::LoadTextFiles(CStringArray &lines, IClipFormats *pFormats)
{		
	IClipFormat *pFormat = pFormats->FindFormatEx(CF_TEXT);
	if(pFormat != NULL)
	{
		char *stringData = (char *)GlobalLock(pFormat->Data());
		if(stringData != NULL)
		{
			CStringA string(stringData);
			CStringW unicodeString(CTextConvert::MultiByteToUnicodeString(string));
			CString delim(_T("\r\n"));

			CTokenizer token(unicodeString, delim);
			CString line;
			while(token.Next(line))
			{
				lines.Add(line);
			}

			GlobalUnlock(pFormat->Data());
		}
	}

	return lines.GetSize() > 0;
}

bool CReadOnlyFlag::LoadHDropFiles(CStringArray &lines, IClipFormats *pFormats)
{
	IClipFormat *pFormat = pFormats->FindFormatEx(CF_HDROP);
	if(pFormat != NULL)
	{
		HDROP drop = (HDROP)GlobalLock(pFormat->Data());
		if(drop)
		{
			int nNumFiles = DragQueryFile(drop, -1, NULL, 0);
			TCHAR file[MAX_PATH];

			for(int nFile = 0; nFile < nNumFiles; nFile++)
			{
				if(DragQueryFile(drop, nFile, file, sizeof(file)) > 0)
				{
					lines.Add(file);
				}
			}

			GlobalUnlock(pFormat->Data());
		}
	}

	return lines.GetSize() > 0;
}