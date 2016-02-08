#include "stdafx.h"
#include "CP_Main.h"
#include "OleClipSource.h"
#include "shared/TextConvert.h"
#include "CF_HDropAggregator.h"
#include "CF_UnicodeTextAggregator.h"
#include "CF_TextAggregator.h"
#include "richtextaggregator.h"
#include "htmlformataggregator.h"

/*------------------------------------------------------------------*\
COleClipSource
\*------------------------------------------------------------------*/
//IMPLEMENT_DYNAMIC(COleClipSource, COleDataSource)
COleClipSource::COleClipSource()
{
	m_bLoadedFormats = false;
}

COleClipSource::~COleClipSource()
{
	
}

BOOL COleClipSource::DoDelayRender()
{
	CClipTypes types;
	m_ClipIDs.GetTypes(types);
	
	INT_PTR count = types.GetSize();
	for(int i=0; i < count; i++)
	{
		DelayRenderData(types[i]);
	}

	return count > 0;
}

#include "Client.h"

BOOL COleClipSource::DoImmediateRender()
{
	if(m_bLoadedFormats)
		return TRUE;

	m_bLoadedFormats = true;

	if(m_pasteOptions.m_pPasteFormats != NULL)
	{
		return PutFormatOnClipboard(m_pasteOptions.m_pPasteFormats) > 0;
	}
	
	INT_PTR count = m_ClipIDs.GetSize();
	if(count <= 0)
		return 0;
	
	CClip clip;

	if(count > 1)
	{
		CStringA SepA = CTextConvert::ConvertToChar(g_Opt.GetMultiPasteSeparator());
		CCF_TextAggregator CFText(SepA);
		if(m_ClipIDs.AggregateData(CFText, CF_TEXT, g_Opt.m_bMultiPasteReverse))
		{
			CClipFormat cf(CF_TEXT, CFText.GetHGlobal());
			clip.m_Formats.Add(cf);
			//clip.m_Formats now owns the global data
			cf.m_autoDeleteData = false;
		}

		CStringW SepW = CTextConvert::ConvertToUnicode(g_Opt.GetMultiPasteSeparator());
		CCF_UnicodeTextAggregator CFUnicodeText(SepW);
		if(m_ClipIDs.AggregateData(CFUnicodeText, CF_UNICODETEXT, g_Opt.m_bMultiPasteReverse))
		{
			CClipFormat cf(CF_UNICODETEXT, CFUnicodeText.GetHGlobal());
			clip.m_Formats.Add(cf);
			//clip.m_Formats now owns the global data
			cf.m_autoDeleteData = false;
		}

		if ((m_pasteOptions.LimitFormatsToText()) &&
			clip.m_Formats.GetCount() == 0)
		{
			CCF_HDropAggregator HDrop;
			if (m_ClipIDs.AggregateData(HDrop, CF_HDROP, g_Opt.m_bMultiPasteReverse))
			{
				CClipFormat cf(CF_UNICODETEXT, HDrop.GetHGlobalAsString());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}
		}
		else if (m_pasteOptions.LimitFormatsToText() == false)
		{
			CCF_HDropAggregator HDrop;
			if(m_ClipIDs.AggregateData(HDrop, CF_HDROP, g_Opt.m_bMultiPasteReverse))
			{
				CClipFormat cf(CF_HDROP, HDrop.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}

			CRichTextAggregator RichText(SepA);
			if(m_ClipIDs.AggregateData(RichText, theApp.m_RTFFormat, g_Opt.m_bMultiPasteReverse))
			{
				CClipFormat cf(theApp.m_RTFFormat, RichText.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}

			CHTMLFormatAggregator Html(SepA);
			if(m_ClipIDs.AggregateData(Html, theApp.m_HTML_Format, g_Opt.m_bMultiPasteReverse))
			{
				CClipFormat cf(theApp.m_HTML_Format, Html.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}
		}
	}

	if (count >= 1 && clip.m_Formats.GetCount() == 0)
	{
		clip.LoadFormats(m_ClipIDs[0], m_pasteOptions.LimitFormatsToText());
	}

	if (m_pasteOptions.LimitFormatsToText())
	{
		PlainTextFilter(clip);
	}

	if(m_pasteOptions.m_pasteUpperCase ||
		m_pasteOptions.m_pasteLowerCase)
	{
		DoUpperLowerCase(clip, m_pasteOptions.m_pasteUpperCase);
	}
	else if(m_pasteOptions.m_pasteCapitalize)
	{
		Capitalize(clip);
	}
	else if(m_pasteOptions.m_pasteSentenceCase)
	{
		SentenceCase(clip);
	}

	return PutFormatOnClipboard(&clip.m_Formats) > 0;
}

void COleClipSource::DoUpperLowerCase(CClip &clip, bool upper)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		HGLOBAL data = unicodeTextFormat->Data();
		wchar_t * stringData = (wchar_t *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CString cs(stringData, (size / sizeof(wchar_t)));
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		CString val;
		if (upper)
		{
			val = cs.MakeUpper();
		}
		else
		{
			val = cs.MakeLower();
		}
		
		long lLen = val.GetLength();
		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(), ((lLen+1) * sizeof(wchar_t)));
		val.ReleaseBuffer();

		unicodeTextFormat->Data(hGlobal);		
	}

	IClipFormat *asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		HGLOBAL data = asciiTextFormat->Data();
		char * stringData = (char *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CStringA cs(stringData, size);
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		asciiTextFormat->Free();
		
		CString val;
		if (upper)
		{
			val = cs.MakeUpper();
		}
		else
		{
			val = cs.MakeLower();
		}

		long lLen = val.GetLength();
		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(lLen), lLen + sizeof(char));
		val.ReleaseBuffer();

		asciiTextFormat->Data(hGlobal);
	}
}

void COleClipSource::Capitalize(CClip &clip)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		HGLOBAL data = unicodeTextFormat->Data();
		wchar_t * stringData = (wchar_t *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CString cs(stringData, (size / sizeof(wchar_t)));
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		CString val = cs.MakeLower();		
		long len = val.GetLength();

		if (len > 0)
		{
			wchar_t * pText = val.GetBuffer();
			
			pText[0] = toupper(pText[0]);
			bool capitalize = false;
			
			for (int i = 1; i < len; i++)
			{
				wchar_t item = pText[i];
				if (item == ' ')
				{
					capitalize = true;
				}
				else if (capitalize)
				{
					pText[i] = toupper(item);
					capitalize = false;
				}
			}
		}
		
		val.ReleaseBuffer();

		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(), ((len + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
	}

	//test
	//second test

	IClipFormat *asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		HGLOBAL data = asciiTextFormat->Data();
		char * stringData = (char *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CStringA cs(stringData, size);
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		asciiTextFormat->Free();

		CStringA val = cs.MakeLower();
		long len = val.GetLength();

		if (len > 0)
		{
			char * pText = val.GetBuffer();

			pText[0] = toupper(pText[0]);
			bool capitalize = false;

			for (int i = 1; i < len; i++)
			{
				wchar_t item = pText[i];
				if (item == ' ')
				{
					capitalize = true;
				}
				else if (capitalize)
				{
					pText[i] = toupper(item);
					capitalize = false;
				}
			}
		}

		val.ReleaseBuffer();

		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(), (len + 1));

		asciiTextFormat->Data(hGlobal);
	}
}

void COleClipSource::SentenceCase(CClip &clip)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		HGLOBAL data = unicodeTextFormat->Data();
		wchar_t * stringData = (wchar_t *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CString cs(stringData, (size / sizeof(wchar_t)));
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		CString val = cs.MakeLower();
		long len = val.GetLength();

		if (len > 0)
		{
			wchar_t * pText = val.GetBuffer();

			pText[0] = toupper(pText[0]);
			bool capitalize = false;

			for (int i = 1; i < len; i++)
			{
				wchar_t item = pText[i];
				if (item == '.' ||
					item == '!' ||
					item == '?')
				{
					capitalize = true;
				}
				else if (capitalize && item != ' ')
				{
					pText[i] = toupper(item);
					capitalize = false;
				}
			}
		}


		val.ReleaseBuffer();

		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(), ((len + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
	}

	IClipFormat *asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		HGLOBAL data = asciiTextFormat->Data();
		char * stringData = (char *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CStringA cs(stringData, size);
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		asciiTextFormat->Free();

		CStringA val = cs.MakeLower();
		long len = val.GetLength();

		if (len > 0)
		{
			char * pText = val.GetBuffer();

			pText[0] = toupper(pText[0]);
			bool capitalize = false;

			for (int i = 1; i < len; i++)
			{
				wchar_t item = pText[i];
				if (item == '.' ||
					item == '!' ||
					item == '?')
				{
					capitalize = true;
				}
				else if (capitalize && item != ' ')
				{
					pText[i] = toupper(item);
					capitalize = false;
				}
			}
		}

		val.ReleaseBuffer();

		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(), (len + 1));

		asciiTextFormat->Data(hGlobal);
	}
}

void COleClipSource::PlainTextFilter(CClip &clip)
{
	bool foundText = false;
	INT_PTR hDropIndex = -1;
	INT_PTR	count = clip.m_Formats.GetCount();
	for (INT_PTR i = 0; i < count; i++)
	{
		CClipFormat *pCF = &clip.m_Formats.ElementAt(i);

		if (pCF->m_cfType == CF_TEXT ||
			pCF->m_cfType == CF_UNICODETEXT)
		{
			foundText = true;
		}
		else if (pCF->m_cfType == CF_HDROP)
		{
			hDropIndex = i;
		}
	}

	if (foundText &&
		hDropIndex > -1)
	{
		clip.m_Formats.RemoveAt(hDropIndex);
	}
	else if (foundText == false &&
		hDropIndex > -1)
	{
		CCF_HDropAggregator HDrop;
		if (m_ClipIDs.AggregateData(HDrop, CF_HDROP, g_Opt.m_bMultiPasteReverse))
		{
			clip.m_Formats.Add(CClipFormat(CF_UNICODETEXT, HDrop.GetHGlobalAsString()));
		}
	}
}

INT_PTR COleClipSource::PutFormatOnClipboard(CClipFormats *pFormats)
{
	Log(_T("Start of put format on clipboard"));

	CClipFormat* pCF;
	INT_PTR	count = pFormats->GetSize();
	bool bDelayedRenderCF_HDROP = false;
	INT_PTR i = 0;

	//see if the html format is in the list
	//if it is the list we will not paste CF_TEXT
	for(i = 0; i < count; i++)
	{
		pCF = &pFormats->ElementAt(i);

		if(pCF->m_cfType == theApp.m_RemoteCF_HDROP)
		{
			bDelayedRenderCF_HDROP = true;
		}
	}

	for(i = 0; i < count; i++)
	{
		pCF = &pFormats->ElementAt(i);

		if(bDelayedRenderCF_HDROP)
		{
			if(pCF->m_cfType == CF_HDROP)
			{
				LogSendRecieveInfo("Added delayed cf_hdrop to clipboard");
				DelayRenderData(pCF->m_cfType);
			}

			continue;
		}

		wchar_t * stringData = (wchar_t *) GlobalLock(pCF->m_hgData);
		int size = (int) GlobalSize(pCF->m_hgData);
		CString cs(stringData, (size / sizeof(wchar_t)));
		GlobalUnlock(pCF->m_hgData);

		Log(StrF(_T("Setting clipboard type: %s to the clipboard"), GetFormatName(pCF->m_cfType)));

		CacheGlobalData(pCF->m_cfType, pCF->m_hgData);
		pCF->m_hgData = 0; // OLE owns it now
	}

	pFormats->RemoveAll();

	m_bLoadedFormats = true;

	Log(_T("End of put format on clipboard"));

	return count;
}  

BOOL COleClipSource::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	static bool bInHere = false;

	if(bInHere)
	{
		return FALSE;
	}
	bInHere = true;

	HGLOBAL hData = NULL;

	CClipFormat *pFind = m_DelayRenderedFormats.FindFormat(lpFormatEtc->cfFormat);

	if(pFind)
	{
		if(pFind->m_hgData)
		{
			hData = NewGlobalH(pFind->m_hgData, GlobalSize(pFind->m_hgData));
		}
	}
	else
	{
		LogSendRecieveInfo("Delayed Render, getting data from remote machine");

		CClip clip;

		if(m_ClipIDs.GetCount() > 0)
		{
			clip.LoadFormats(m_ClipIDs[0]);

			CClipFormat *pDittoDelayCF_HDROP = clip.m_Formats.FindFormat(theApp.m_RemoteCF_HDROP);
			CClipFormat *pCF_HDROP = clip.m_Formats.FindFormat(CF_HDROP);

			if(pDittoDelayCF_HDROP && pCF_HDROP)
			{
				CDittoCF_HDROP *pData = (CDittoCF_HDROP*)GlobalLock(pDittoDelayCF_HDROP->m_hgData);
				if(pData)
				{
					CString csComputerName;
					CString csIP;

					CTextConvert::ConvertFromUTF8(pData->m_cIP, csIP);
					CTextConvert::ConvertFromUTF8(pData->m_cComputerName, csComputerName);
					
					GlobalUnlock(pDittoDelayCF_HDROP->m_hgData);

					CClient cl;
					hData = cl.RequestCopiedFiles(*pCF_HDROP, csIP, csComputerName);
				}
			}
			else
			{
				hData = m_ClipIDs.Render(lpFormatEtc->cfFormat);
			}
		}

		//Add to a cache of already rendered data
		//Windows seems to call this function multiple times
		//so only the first time do we need to go get the data
		HGLOBAL hCopy = NULL;
		if(hData)
		{
			hCopy = NewGlobalH(hData, GlobalSize(hData));
		}

		CClipFormat format(lpFormatEtc->cfFormat, hCopy);
		format.m_autoDeleteData = false; //owned by m_DelayRenderedFormats
		m_DelayRenderedFormats.Add(format);
	}

	BOOL bRet = FALSE;
	if(hData)
	{
		// if phGlobal is null, we can just give the allocated mem
		// else, our data must fit within the GlobalSize(*phGlobal)
		if(*phGlobal == 0)
		{
			*phGlobal = hData;
		}
		else
		{
			SIZE_T len = min(::GlobalSize(*phGlobal), ::GlobalSize(hData));
			if(len)
			{
				CopyToGlobalHH(*phGlobal, hData, len);
			}
			::GlobalFree(hData);
		}
		bRet = TRUE;
	}

	bInHere = false;

	return bRet;
}