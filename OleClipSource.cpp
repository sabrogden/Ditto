#include "stdafx.h"
#include "CP_Main.h"
#include "OleClipSource.h"
#include "shared/TextConvert.h"
#include "CF_HDropAggregator.h"
#include "CF_UnicodeTextAggregator.h"
#include "CF_TextAggregator.h"
#include "richtextaggregator.h"
#include "htmlformataggregator.h"
#include "Shared\Tokenizer.h"
#include <random>
#include "Client.h"
#include "sqlite\unicode\unistr.h"
#include "sqlite\unicode\uchar.h"

/*------------------------------------------------------------------*\
COleClipSource
\*------------------------------------------------------------------*/
//IMPLEMENT_DYNAMIC(COleClipSource, COleDataSource)
COleClipSource::COleClipSource()
{
	m_bLoadedFormats = false;
	m_convertToHDROPOnDelayRender = false;
}

COleClipSource::~COleClipSource()
{
	
}

BOOL COleClipSource::DoDelayRender()
{
	CClipTypes types;
	m_ClipIDs.GetTypes(types);

	bool foundHDrop = false;
	
	INT_PTR count = types.GetSize();
	for(int i=0; i < count; i++)
	{
		DelayRenderData(types[i]);

		if (types[i] == CF_HDROP)
		{
			foundHDrop = true;
		}
	}

	if (foundHDrop == false)
	{
		DelayRenderData(CF_HDROP);
		m_convertToHDROPOnDelayRender = true;
	}

	return count > 0;
}

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
		clip.LoadFormats(m_ClipIDs[0], m_pasteOptions.LimitFormatsToText(), m_pasteOptions.IncludeRTFForTextOnly());
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
	else if(m_pasteOptions.m_pasteRemoveLineFeeds)
	{
		RemoveLineFeeds(clip);
	}
	else if(m_pasteOptions.m_pasteAddOneLineFeed)
	{
		AddLineFeeds(clip, 1);
	}
	else if (m_pasteOptions.m_pasteAddTwoLineFeeds)
	{
		AddLineFeeds(clip, 2);
	}
	else if (m_pasteOptions.m_pasteTypoglycemia)
	{
		Typoglycemia(clip);
	}
	else if (m_pasteOptions.m_pasteAddingDateTime)
	{
		AddDateTime(clip);
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
		icu::UnicodeString s = stringData;
		GlobalUnlock(data);		

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		icu::UnicodeString val;
		if (upper)
		{
			val = s.toUpper();
		}
		else
		{
			val = s.toLower();
		}
		
		long lLen = val.length();
		HGLOBAL hGlobal = NewGlobalP((LPVOID)val.getTerminatedBuffer(), ((lLen+1) * sizeof(wchar_t)));
		val.releaseBuffer();

		unicodeTextFormat->Data(hGlobal);		
	}

	IClipFormat *asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		HGLOBAL data = asciiTextFormat->Data();
		char * stringData = (char *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CStringA cs(stringData);
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
		CString cs(stringData);
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		icu::UnicodeString temp = cs;

		CString val = temp.toLower().getTerminatedBuffer();
		long len = val.GetLength();

		if (len > 0)
		{
			wchar_t * pText = val.GetBuffer();
			
			pText[0] = u_toupper(pText[0]);
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
					pText[i] = u_toupper(item);
					capitalize = false;
				}
			}
		}
		
		val.ReleaseBuffer();

		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(), ((len + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
	}

	//my change
	//test
	//second test

	IClipFormat *asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		HGLOBAL data = asciiTextFormat->Data();
		char * stringData = (char *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CStringA cs(stringData);
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
		CString cs(stringData);
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		icu::UnicodeString temp = cs;

		CString val = temp.toLower().getTerminatedBuffer();
		long len = val.GetLength();

		if (len > 0)
		{
			wchar_t * pText = val.GetBuffer();

			pText[0] = u_toupper(pText[0]);
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
					pText[i] = u_toupper(item);
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
		CStringA cs(stringData);
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
			clip.m_Formats.RemoveAt(hDropIndex);

			CClipFormat format(CF_UNICODETEXT, HDrop.GetHGlobalAsString());
			clip.m_Formats.Add(format);
			format.m_autoDeleteData = false; //owned by m_DelayRenderedFormats			
		}
	}
}

void COleClipSource::RemoveLineFeeds(CClip &clip)
{
	IClipFormat *pUnicodeText = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (pUnicodeText != NULL)
	{
		wchar_t *stringData = (wchar_t *) GlobalLock(pUnicodeText->Data());
		if (stringData != NULL)
		{
			CStringW string(stringData);

			GlobalUnlock(pUnicodeText->Data());
			pUnicodeText->Free();

			int count = string.Replace(_T("\r\n"), _T(" "));
			count = string.Replace(_T("\r"), _T(" "));
			count = string.Replace(_T("\n"), _T(" "));
			
			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1) * sizeof(wchar_t)));

			pUnicodeText->Data(hGlobal);
		}
	}

	IClipFormat *pAsciiText = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (pAsciiText != NULL)
	{
		char *stringData = (char *) GlobalLock(pAsciiText->Data());
		if (stringData != NULL)
		{
			CStringA string(stringData);

			GlobalUnlock(pAsciiText->Data());
			pAsciiText->Free();

			int count = string.Replace("\r\n", " ");
			count = string.Replace("\r", " ");
			count = string.Replace("\n", " ");

			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

			pAsciiText->Data(hGlobal);
		}
	}

	IClipFormat *pRTFFormat = clip.m_Formats.FindFormatEx(theApp.m_RTFFormat);
	if (pRTFFormat != NULL)
	{
		char *stringData = (char *) GlobalLock(pRTFFormat->Data());
		if (stringData != NULL)
		{
			CStringA string(stringData);

			GlobalUnlock(pRTFFormat->Data());
			pRTFFormat->Free();

			int count = string.Replace("\\par\r\n", " ");
			int count2 = string.Replace("\\par ", " ");
			int count3 = string.Replace("\\line ", " ");
			
			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

			pRTFFormat->Data(hGlobal);
		}
	}
}

void COleClipSource::AddLineFeeds(CClip &clip, int count)
{
	IClipFormat *pUnicodeText = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (pUnicodeText != NULL)
	{
		wchar_t *stringData = (wchar_t *) GlobalLock(pUnicodeText->Data());
		if (stringData != NULL)
		{
			CStringW string(stringData);

			GlobalUnlock(pUnicodeText->Data());
			pUnicodeText->Free();

			for(int i = 0; i < count; i++)
			{
				string += _T("\r\n");
			}

			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1) * sizeof(wchar_t)));

			pUnicodeText->Data(hGlobal);
		}
	}

	IClipFormat *pAsciiText = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (pAsciiText != NULL)
	{
		char *stringData = (char *) GlobalLock(pAsciiText->Data());
		if (stringData != NULL)
		{
			CStringA string(stringData);

			GlobalUnlock(pAsciiText->Data());
			pAsciiText->Free();

			for (int i = 0; i < count; i++)
			{
				string += _T("\r\n");
			}

			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

			pAsciiText->Data(hGlobal);
		}
	}

	IClipFormat *pRTFFormat = clip.m_Formats.FindFormatEx(theApp.m_RTFFormat);
	if (pRTFFormat != NULL)
	{
		char *stringData = (char *) GlobalLock(pRTFFormat->Data());
		if (stringData != NULL)
		{
			CStringA string(stringData);

			GlobalUnlock(pRTFFormat->Data());
			pRTFFormat->Free();

			for (int i = 0; i < count; i++)
			{
				int pos = string.ReverseFind('}');
				if (pos >= 0)
				{
					int count = string.Insert(pos, "\\par\r\n");
				}
			}

			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

			pRTFFormat->Data(hGlobal);
		}
	}
}

void COleClipSource::AddDateTime(CClip &clip)
{
	IClipFormat *pUnicodeText = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (pUnicodeText != NULL)
	{
		wchar_t *stringData = (wchar_t *)GlobalLock(pUnicodeText->Data());
		if (stringData != NULL)
		{
			CStringW string(stringData);

			GlobalUnlock(pUnicodeText->Data());
			pUnicodeText->Free();

			string += _T("\r\n\r\n");

			COleDateTime now(COleDateTime::GetCurrentTime());
			string += now.Format();			

			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1) * sizeof(wchar_t)));

			pUnicodeText->Data(hGlobal);
		}
	}

	IClipFormat *pAsciiText = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (pAsciiText != NULL)
	{
		char *stringData = (char *)GlobalLock(pAsciiText->Data());
		if (stringData != NULL)
		{
			CStringA string(stringData);

			GlobalUnlock(pAsciiText->Data());
			pAsciiText->Free();

			string += "\r\n\r\n";

			COleDateTime now(COleDateTime::GetCurrentTime());
			string += CTextConvert::UnicodeStringToMultiByte(now.Format());

			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

			pAsciiText->Data(hGlobal);
		}
	}

	IClipFormat *pRTFFormat = clip.m_Formats.FindFormatEx(theApp.m_RTFFormat);
	if (pRTFFormat != NULL)
	{
		char *stringData = (char *)GlobalLock(pRTFFormat->Data());
		if (stringData != NULL)
		{
			CStringA string(stringData);

			GlobalUnlock(pRTFFormat->Data());
			pRTFFormat->Free();
			
			int pos = string.ReverseFind('}');
			if (pos >= 0)
			{
				string += _T("\r\n\r\n");

				COleDateTime now(COleDateTime::GetCurrentTime());
				
				CStringA insert;
				insert.Format("\\par\r\n\\par\r\n%s", CTextConvert::UnicodeStringToMultiByte(now.Format()));

				int count = string.Insert(pos, insert);
			}

			HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

			pRTFFormat->Data(hGlobal);
		}
	}
}

void COleClipSource::Typoglycemia(CClip &clip)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		HGLOBAL data = unicodeTextFormat->Data();
		wchar_t * stringData = (wchar_t *) GlobalLock(data);
		int size = (int) GlobalSize(data);
		CString cs(stringData);
		GlobalUnlock(data);

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		CString newString;
		
		
		CTokenizer token(cs, _T(' '));
		CString word;

		while (token.Next(word))
		{
			if(word.GetLength() > 3)
			{
				int end = word.GetLength();
				
				for (int i = end-1; i >= 0; i--)
				{
					if(word[i] == _T('.') ||
						word[i] == _T('!') ||
						word[i] == _T('?'))
					{
						end--;
					}
					else
					{
						break;
					}
				}

				if (end > 3)
				{
					std::uniform_int_distribution<int> dist(1, end - 2);
					std::random_device rd;

					for (int i = 1; i < end - 1; i++)
					{
						int newPos = dist(rd);

						CString cs;
						cs.Format(_T("pos: %d, rnd: %d\r\n"), i, newPos);
						OutputDebugString(cs);

						TCHAR temp = word.GetAt(i);
						word.SetAt(i, word.GetAt(newPos));
						word.SetAt(newPos, temp);
					}
				}

				newString += word;
			}
			else
			{
				newString += word;
			}

			newString += _T(' ');
		}

		
		long len = newString.GetLength();
		HGLOBAL hGlobal = NewGlobalP(newString.GetBuffer(), ((len + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
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
		CString cs(stringData);
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

				if (m_convertToHDROPOnDelayRender &&
					hData == NULL && 
					lpFormatEtc->cfFormat == CF_HDROP)
				{
					hData = ConvertToFileDrop();
				}
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
		m_DelayRenderedFormats.Add(format);
		format.m_autoDeleteData = false; //owned by m_DelayRenderedFormats
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

HGLOBAL COleClipSource::ConvertToFileDrop()
{
	CString path = CGetSetOptions::GetPath(PATH_DRAG_FILES);	
	CreateDirectory(path, NULL);

	CFileRecieve fileList;

	int dragId = CGetSetOptions::GetDragId();
	int origDragId = dragId;

	for (int i = 0; i < m_ClipIDs.GetCount(); i++)
	{
		CClip fileClip;
		fileClip.LoadFormats(m_ClipIDs[i]);

		CClipFormat *unicodeText = fileClip.m_Formats.FindFormat(CF_UNICODETEXT);
		if (unicodeText)
		{
			CString file;
			file.Format(_T("%stext_%d.txt"), path, dragId++);

			fileClip.WriteTextToFile(file, TRUE, FALSE, FALSE);
			fileList.AddFile(file);
		}
		else
		{
			CClipFormat *asciiText = fileClip.m_Formats.FindFormat(CF_TEXT);
			if (asciiText)
			{
				CString file;
				file.Format(_T("%stext_%d.txt"), path, dragId++);

				fileClip.WriteTextToFile(file, FALSE, TRUE, FALSE);
				fileList.AddFile(file);
			}
			else
			{
				CClipFormat *bitmap = fileClip.m_Formats.FindFormat(CF_DIB);
				if (bitmap)
				{
					CString file;
					file.Format(_T("%simage_%d.png"), path, dragId++);

					LPVOID pvData = GlobalLock(bitmap->m_hgData);
					ULONG size = (ULONG) GlobalSize(bitmap->m_hgData);

					WriteCF_DIBToFile(file, pvData, size);

					GlobalUnlock(bitmap->m_hgData);

					fileList.AddFile(file);
				}
			}
		}
	}

	if(dragId != origDragId)
	{
		CGetSetOptions::SetDragId(dragId);
	}

	HGLOBAL hData = fileList.CreateCF_HDROPBuffer();

	return hData;
}