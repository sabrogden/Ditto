#include "StdAfx.h"
#include "RemoveLineFeeds.h"

#include "../../Shared/Tokenizer.h"
#include "../../Shared/TextConvert.h"


CRemoveLineFeeds::CRemoveLineFeeds(void)
{
}


CRemoveLineFeeds::~CRemoveLineFeeds(void)
{
}

bool CRemoveLineFeeds::RemoveLineFeeds(const CDittoInfo &DittoInfo, IClip *pClip)
{
	bool didSomething = false;
	IClipFormats *pFormats = pClip->Clips();
	if(pFormats)
	{
		didSomething = Handle_CF_TEXT(pFormats);

		didSomething |= Handle_CF_UNICODETEXT(pFormats);

		didSomething |= Handle_RichText(pFormats);
	}

	return didSomething;
}

bool CRemoveLineFeeds::Handle_CF_TEXT(IClipFormats *pFormats)
{	
	bool didSomething = false;

	IClipFormat *pFormat = pFormats->FindFormatEx(CF_TEXT);
	if(pFormat != NULL)
	{
		char *stringData = (char *)GlobalLock(pFormat->Data());
		if(stringData != NULL)
		{
			CStringA string(stringData);

			int count = string.Replace("\r\n", " ");

			int size = GlobalSize(pFormat->Data());
			strcpy_s(stringData, GlobalSize(pFormat->Data()), string);

			GlobalUnlock(pFormat->Data());

			didSomething = true;
		}
	}

	return didSomething;
}

bool CRemoveLineFeeds::Handle_CF_UNICODETEXT(IClipFormats *pFormats)
{	
	bool didSomething = false;

	IClipFormat *pFormat = pFormats->FindFormatEx(CF_UNICODETEXT);
	if(pFormat != NULL)
	{
		wchar_t *stringData = (wchar_t *)GlobalLock(pFormat->Data());
		if(stringData != NULL)
		{
			CStringW string(stringData);

			int count = string.Replace(_T("\r\n"), _T(" "));

			wcscpy_s(stringData, GlobalSize(pFormat->Data())/2, string);

			GlobalUnlock(pFormat->Data());

			didSomething = true;
		}
	}

	return didSomething;
}

bool CRemoveLineFeeds::Handle_RichText(IClipFormats *pFormats)
{	
	bool didSomething = false;

	CLIPFORMAT m_RTFFormat = ::RegisterClipboardFormat(_T("Rich Text Format"));

	IClipFormat *pFormat = pFormats->FindFormatEx(m_RTFFormat);
	if(pFormat != NULL)
	{
		char *stringData = (char *)GlobalLock(pFormat->Data());
		if(stringData != NULL)
		{
			CStringA string(stringData);

			int count = string.Replace("\\par\r\n", " ");
			int count2 = string.Replace("\\par ", " ");
			int count3 = string.Replace("\\line ", " ");

			strcpy_s(stringData, GlobalSize(pFormat->Data()), string);

			GlobalUnlock(pFormat->Data());

			didSomething = true;
		}
	}

	return didSomething;
}
