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
#include "Path.h"
#include "Md5.h"
#include "DittoChaiScript.h"
#include "ChaiScriptOnCopy.h"
#include "Slugify.h"
#include "ImageFormatAggregator.h"

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
		if (m_pasteOptions.m_dragDropFilesOnly)
		{
			if (types[i] == CF_HDROP)
			{
				DelayRenderData(types[i]);
			}
		}
		else
		{
			DelayRenderData(types[i]);
		}

		if (types[i] == CF_HDROP)
		{
			foundHDrop = true;
		}
	}

	if (m_pasteOptions.m_placeCF_HDROP_OnDrag &&
		foundHDrop == false)
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
		if (m_pasteOptions.m_pasteImagesHorizontal ||
			m_pasteOptions.m_pasteImagesVertically)
		{
			CImageFormatAggregator bigImage(m_pasteOptions.m_pasteImagesHorizontal);
			if (m_ClipIDs.AggregateData(bigImage, CF_DIB, g_Opt.m_bMultiPasteReverse, m_pasteOptions.LimitFormatsToText()))
			{
				CClipFormat cf(CF_DIB, bigImage.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}
		}
		else
		{
			CStringA SepA = CTextConvert::UnicodeToAnsi(g_Opt.GetMultiPasteSeparator());
			CCF_TextAggregator CFText(SepA);
			if (m_ClipIDs.AggregateData(CFText, CF_TEXT, g_Opt.m_bMultiPasteReverse, m_pasteOptions.LimitFormatsToText()))
			{
				CClipFormat cf(CF_TEXT, CFText.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}

			CStringW SepW = g_Opt.GetMultiPasteSeparator();
			CCF_UnicodeTextAggregator CFUnicodeText(SepW);
			if (m_ClipIDs.AggregateData(CFUnicodeText, CF_UNICODETEXT, g_Opt.m_bMultiPasteReverse, m_pasteOptions.LimitFormatsToText()))
			{
				CClipFormat cf(CF_UNICODETEXT, CFUnicodeText.GetHGlobal());
				clip.m_Formats.Add(cf);
				//clip.m_Formats now owns the global data
				cf.m_autoDeleteData = false;
			}

			if (m_pasteOptions.LimitFormatsToText() == false)
			{
				CCF_HDropAggregator HDrop;
				if (m_ClipIDs.AggregateData(HDrop, CF_HDROP, g_Opt.m_bMultiPasteReverse, m_pasteOptions.LimitFormatsToText()))
				{
					CClipFormat cf(CF_HDROP, HDrop.GetHGlobal());
					clip.m_Formats.Add(cf);
					//clip.m_Formats now owns the global data
					cf.m_autoDeleteData = false;
				}

				CRichTextAggregator RichText(SepA);
				if (m_ClipIDs.AggregateData(RichText, theApp.m_RTFFormat, g_Opt.m_bMultiPasteReverse, m_pasteOptions.LimitFormatsToText()))
				{
					CClipFormat cf(theApp.m_RTFFormat, RichText.GetHGlobal());
					clip.m_Formats.Add(cf);
					//clip.m_Formats now owns the global data
					cf.m_autoDeleteData = false;
				}

				CHTMLFormatAggregator Html(SepA);
				if (m_ClipIDs.AggregateData(Html, theApp.m_HTML_Format, g_Opt.m_bMultiPasteReverse, m_pasteOptions.LimitFormatsToText()))
				{
					CClipFormat cf(theApp.m_HTML_Format, Html.GetHGlobal());
					clip.m_Formats.Add(cf);
					//clip.m_Formats now owns the global data
					cf.m_autoDeleteData = false;
				}
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
	else if (m_pasteOptions.m_trimWhiteSpace)
	{
		TrimWhiteSpace(clip);
	}
	else if (m_pasteOptions.m_pasteSlugify)
	{
		Slugify(clip);
	}
	else if (m_pasteOptions.m_invertCase)
	{
		InvertCase(clip);
	}
	else if (m_pasteOptions.m_pasteCamelCase)
	{
		CamelCase(clip);
	}
	else if (m_pasteOptions.m_pasteAsciiOnly)
	{
		AsciiOnly(clip);
	}
	
	SaveDittoFileDataToFile(clip);

	if (m_pasteOptions.m_pasteScriptGuid != _T(""))
	{
		for (auto & element : g_Opt.m_pasteScripts.m_list)
		{
			if (element.m_guid == m_pasteOptions.m_pasteScriptGuid)
			{
				try
				{
					Log(StrF(_T("Start of paste script name: %s, script: %s"), element.m_name, element.m_script));

					ChaiScriptOnCopy onPaste;
					CDittoChaiScript clipData(&clip, "", "");
					if (onPaste.ProcessScript(clipData, (LPCSTR)CTextConvert::UnicodeToAnsi(element.m_script)) == false)
					{
						Log(StrF(_T("End of paste script name: %s, returned false, not saving this copy to Ditto, last Error: %s"), element.m_name, onPaste.m_lastError));

						return FALSE;
					}

					Log(StrF(_T("End of paste script name: %s, returned true, last Error: %s"), element.m_name, onPaste.m_lastError));
				}
				catch (CException *ex)
				{
					TCHAR szCause[255];
					ex->GetErrorMessage(szCause, 255);
					CString cs;
					cs.Format(_T("chai script paste exception: %s"), szCause);
					Log(cs);
				}
				catch (...)
				{
					Log(_T("chai script paste exception 2"));
				}

				break;
			}
		}
	}

	return PutFormatOnClipboard(&clip.m_Formats) > 0;
}

void COleClipSource::DoUpperLowerCase(CClip &clip, bool upper)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{		
		CString cs = unicodeTextFormat->GetAsCString();

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();
				
		CString val;
		if (upper)
		{
			val = theApp.m_icuString.ToUpperStringEx(cs);
		}
		else
		{
			val = theApp.m_icuString.ToLowerStringEx(cs);
		}
		
		long lLen = val.GetLength();
		HGLOBAL hGlobal = NewGlobalP(val.GetBuffer(lLen), ((lLen+1) * sizeof(wchar_t)));
		val.ReleaseBuffer();

		unicodeTextFormat->Data(hGlobal);		
	}

	IClipFormat *asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		CStringA cs(asciiTextFormat->GetAsCStringA());

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

void COleClipSource::InvertCase(CClip &clip)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		CString cs(unicodeTextFormat->GetAsCString());	

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		const int len = cs.GetLength();
		if (len > 0)
		{
			wchar_t* pText = cs.GetBuffer();
			
			for (int i = 0; i < len; i++)
			{
				wchar_t item = pText[i];
				if (theApp.m_icuString.IsUpperEx(item))
				{
					pText[i] = theApp.m_icuString.ToLowerEx(item);
				}
				else
				{
					pText[i] = theApp.m_icuString.ToUpperEx(item);
				}
			}
		}

		cs.ReleaseBuffer();

		HGLOBAL hGlobal = NewGlobalP(cs.GetBuffer(), ((len + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
	}

	IClipFormat *asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		CStringA cs(asciiTextFormat->GetAsCStringA());

		//free the old text we are going to replace it below with an upper case version
		asciiTextFormat->Free();

		long len = cs.GetLength();

		if (len > 0)
		{
			char * pText = cs.GetBuffer();

			for (int i = 0; i < len; i++)
			{
				char item = pText[i];
				if (::isupper(item))
				{
					pText[i] = ::tolower(item);
				}
				else
				{
					pText[i] = ::toupper(item);
				}
			}
		}

		cs.ReleaseBuffer();

		HGLOBAL hGlobal = NewGlobalP(cs.GetBuffer(), (len + 1));

		asciiTextFormat->Data(hGlobal);
	}
}

void COleClipSource::CamelCase(CClip& clip)
{
	IClipFormat* unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		CString cs(unicodeTextFormat->GetAsCString());

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		const int len = cs.GetLength();
		if (len > 0)
		{
			wchar_t* pText = cs.GetBuffer();

			bool setCapital = false;
			for (int i = 0; i < len; i++)
			{
				wchar_t item = pText[i];
				if (item == ' ')
				{
					setCapital = true;
				}
				else if (setCapital || i == 0)
				{
					if (theApp.m_icuString.IsUpperEx(item) == false)
					{
						pText[i] = theApp.m_icuString.ToUpperEx(item);
					}
					setCapital = false;
				}
				else if (theApp.m_icuString.IsUpperEx(item))
				{
					pText[i] = theApp.m_icuString.ToLowerEx(item);
				}
			}
		}

		cs.ReleaseBuffer();

		cs.Remove(' ');

		HGLOBAL hGlobal = NewGlobalP(cs.GetBuffer(), ((len + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
	}

	IClipFormat* asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		CStringA cs(asciiTextFormat->GetAsCStringA());

		//free the old text we are going to replace it below with an upper case version
		asciiTextFormat->Free();

		long len = cs.GetLength();

		if (len > 0)
		{
			char* pText = cs.GetBuffer();

			bool setCapital = false;
			for (int i = 0; i < len; i++)
			{
				char item = pText[i];
				if (item == ' ')
				{
					setCapital = true;
				}
				else if (setCapital || i == 0)
				{
					if (::isupper(item) == false)
					{
						pText[i] = ::toupper(item);
					}
					setCapital = false;
				}
				else if(::isupper(item))
				{
					pText[i] = ::tolower(item);
				}
			}
		}

		cs.ReleaseBuffer();
		cs.Remove(' ');

		HGLOBAL hGlobal = NewGlobalP(cs.GetBuffer(), (len + 1));

		asciiTextFormat->Data(hGlobal);
	}
}

void COleClipSource::Capitalize(CClip &clip)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		CString cs(unicodeTextFormat->GetAsCString());	

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		CString val = theApp.m_icuString.ToLowerStringEx(cs);
		long len = val.GetLength();

		if (len > 0)
		{
			wchar_t * pText = val.GetBuffer();
			
			pText[0] = theApp.m_icuString.ToUpperEx(pText[0]);
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
					pText[i] = theApp.m_icuString.ToUpperEx(item);
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
		CStringA cs(asciiTextFormat->GetAsCStringA());

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
				char item = pText[i];
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
		CString cs(unicodeTextFormat->GetAsCString());

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		CString val = theApp.m_icuString.ToLowerStringEx(cs);;
		long len = val.GetLength();

		if (len > 0)
		{
			wchar_t * pText = val.GetBuffer();

			pText[0] = theApp.m_icuString.ToUpperEx(pText[0]);
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
					pText[i] = theApp.m_icuString.ToUpperEx(item);
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
		CStringA cs(asciiTextFormat->GetAsCStringA());

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
				char item = pText[i];
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

void COleClipSource::AsciiOnly(CClip& clip)
{
	IClipFormat* unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		CString cs(unicodeTextFormat->GetAsCString());
		CString newString;

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();
		
		long len = cs.GetLength();

		if (len > 0)
		{
			for (int i = 0; i < len; i++)
			{
				wchar_t item = cs[i];
				if (item >= 0x00 && item <= 0x7F)
				{
					newString += item;
				}				
			}
		}

		HGLOBAL hGlobal = NewGlobalP(newString.GetBuffer(), ((newString.GetLength() + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
	}

	IClipFormat* asciiTextFormat = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (asciiTextFormat != NULL)
	{
		CStringA cs(asciiTextFormat->GetAsCStringA());
		CStringA newString;

		//free the old text we are going to replace it below with an upper case version
		asciiTextFormat->Free();
				
		long len = cs.GetLength();

		if (len > 0)
		{
			for (int i = 0; i < len; i++)
			{
				char item = cs[i];
				if (item >= 0x00 && item <= 0x7F)
				{
					newString += item;
				}
			}
		}

		HGLOBAL hGlobal = NewGlobalP(newString.GetBuffer(), (newString.GetLength() + 1));

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
		if (m_ClipIDs.AggregateData(HDrop, CF_HDROP, g_Opt.m_bMultiPasteReverse, m_pasteOptions.LimitFormatsToText()))
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
		CStringW string(pUnicodeText->GetAsCString());
		
		pUnicodeText->Free();

		int count = string.Replace(_T("\r\n"), _T(" "));
		count = string.Replace(_T("\r"), _T(" "));
		count = string.Replace(_T("\n"), _T(" "));
			
		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1) * sizeof(wchar_t)));

		pUnicodeText->Data(hGlobal);
	}

	IClipFormat *pAsciiText = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (pAsciiText != NULL)
	{
		CStringA string(pAsciiText->GetAsCStringA());

		pAsciiText->Free();

		int count = string.Replace("\r\n", " ");
		count = string.Replace("\r", " ");
		count = string.Replace("\n", " ");

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

		pAsciiText->Data(hGlobal);
	}

	IClipFormat *pRTFFormat = clip.m_Formats.FindFormatEx(theApp.m_RTFFormat);
	if (pRTFFormat != NULL)
	{
		CStringA string(pRTFFormat->GetAsCStringA());
		
		pRTFFormat->Free();

		int count = string.Replace("\\par\r\n", " ");
		int count2 = string.Replace("\\par ", " ");
		int count3 = string.Replace("\\line ", " ");
			
		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

		pRTFFormat->Data(hGlobal);
	}
}

void COleClipSource::AddLineFeeds(CClip &clip, int count)
{
	IClipFormat *pUnicodeText = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (pUnicodeText != NULL)
	{		
		CStringW string(pUnicodeText->GetAsCString());
		
		pUnicodeText->Free();

		for(int i = 0; i < count; i++)
		{
			string += _T("\r\n");
		}

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1) * sizeof(wchar_t)));

		pUnicodeText->Data(hGlobal);
	}

	IClipFormat *pAsciiText = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (pAsciiText != NULL)
	{		
		CStringA string(pAsciiText->GetAsCStringA());

		pAsciiText->Free();

		for (int i = 0; i < count; i++)
		{
			string += "\r\n";
		}

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

		pAsciiText->Data(hGlobal);
	}

	IClipFormat *pRTFFormat = clip.m_Formats.FindFormatEx(theApp.m_RTFFormat);
	if (pRTFFormat != NULL)
	{
		CStringA string(pRTFFormat->GetAsCStringA());
			
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

void COleClipSource::AddDateTime(CClip &clip)
{
	IClipFormat *pUnicodeText = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (pUnicodeText != NULL)
	{
		CStringW string(pUnicodeText->GetAsCString());
		pUnicodeText->Free();

		string += _T("\r\n");

		COleDateTime now(COleDateTime::GetCurrentTime());
		string += now.Format();			

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1) * sizeof(wchar_t)));

		pUnicodeText->Data(hGlobal);
	}

	IClipFormat *pAsciiText = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (pAsciiText != NULL)
	{
		CStringA string(pAsciiText->GetAsCStringA());
		pAsciiText->Free();

		string += "\r\n\r\n";

		COleDateTime now(COleDateTime::GetCurrentTime());
		string += CTextConvert::UnicodeToAnsi(now.Format());

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

		pAsciiText->Data(hGlobal);
	}

	IClipFormat *pRTFFormat = clip.m_Formats.FindFormatEx(theApp.m_RTFFormat);
	if (pRTFFormat != NULL)
	{
		CStringA string(pRTFFormat->GetAsCStringA());

		pRTFFormat->Free();
			
		int pos = string.ReverseFind('}');
		if (pos >= 0)
		{
			string += _T("\r\n\r\n");

			COleDateTime now(COleDateTime::GetCurrentTime());
				
			CStringA insert;
			insert.Format("\\par\r\n\\par\r\n%s", CTextConvert::UnicodeToAnsi(now.Format()));

			int count = string.Insert(pos, insert);
		}

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

		pRTFFormat->Data(hGlobal);
	}
}

void COleClipSource::TrimWhiteSpace(CClip &clip)
{
	IClipFormat *pUnicodeText = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (pUnicodeText != NULL)
	{		
		CStringW string(pUnicodeText->GetAsCString());

		pUnicodeText->Free();

		string = string.Trim();
		string = string.Trim(_T("\t"));
		string = string.Trim(_T("\r"));
		string = string.Trim(_T("\n"));			

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1) * sizeof(wchar_t)));

		pUnicodeText->Data(hGlobal);
	}

	IClipFormat *pAsciiText = clip.m_Formats.FindFormatEx(CF_TEXT);
	if (pAsciiText != NULL)
	{		
		CStringA string(pAsciiText->GetAsCStringA());
				
		pAsciiText->Free();

		string = string.Trim();
		string = string.Trim("\t");
		string = string.Trim("\r");
		string = string.Trim("\n");

		HGLOBAL hGlobal = NewGlobalP(string.GetBuffer(), ((string.GetLength() + 1)));

		pAsciiText->Data(hGlobal);
	}
}

void COleClipSource::SaveDittoFileDataToFile(CClip &clip)
{
	CFileRecieve hDrpData;
	CClipFormat* pCF;
	int hDropIndex = -1;
	bool savedFile = false;
	INT_PTR	count = clip.m_Formats.GetSize();
	for (int i = 0; i < count; i++)
	{
		pCF = &clip.m_Formats.ElementAt(i);

		if (pCF->m_cfType == theApp.m_DittoFileData)
		{
			IClipFormat *dittoFileData = &clip.m_Formats.ElementAt(i);
			if (dittoFileData == NULL) 
				continue;

			HGLOBAL data = dittoFileData->Data();
			char * stringData = (char *)GlobalLock(data);

			//original source is store in the first string ending in the null terminator
			CStringA src(stringData);
			stringData += src.GetLength() + 1;

			CStringA originalMd5(stringData);
			stringData += originalMd5.GetLength() + 1;

			int dataSize = (int)GlobalSize(data) - (src.GetLength() + 1) - (originalMd5.GetLength() + 1);

			CMd5 calcMd5;
			CStringA md5String = calcMd5.CalcMD5FromString(stringData, dataSize);

			CString unicodeFilePath = CTextConvert::Utf8ToUnicode(src);

			CString unicodeMd5 = CTextConvert::Utf8ToUnicode(md5String);

			Log(StrF(_T("Saving file contents from Ditto, original file: %s, size: %d, md5: %s"), unicodeFilePath, dataSize, unicodeMd5));

			if (md5String != originalMd5)
			{
				Log(StrF(_T("MD5 ERROR, file: %s, original md5: %s, calc md5: %s"), unicodeFilePath, originalMd5, md5String));
				continue;
			}

			using namespace nsPath;
			CPath path(unicodeFilePath);
			CString fileName = path.GetName();

			CString newFilePath = CGetSetOptions::GetPath(PATH_DRAG_FILES);
			newFilePath += fileName;

			CFile f;
			if (f.Open(newFilePath, CFile::modeWrite | CFile::modeCreate))
			{
				f.Write(stringData, dataSize);

				f.Close();

				savedFile = true;
				hDrpData.AddFile(newFilePath);
			}
			else
			{
				Log(StrF(_T("Error saving file: %s"), unicodeFilePath));
			}

		}
		else if (pCF->m_cfType == CF_HDROP)
		{
			hDropIndex = i;
		}
	}
	
	if (savedFile)
	{
		if (hDropIndex >= 0)
		{
			clip.m_Formats.RemoveAt(hDropIndex);
		}

		CClipFormat cf(CF_HDROP, hDrpData.CreateCF_HDROPBuffer());
		clip.m_Formats.Add(cf);

		//clip.m_Formats now owns the global data
		cf.m_autoDeleteData = false;
	}
}

void COleClipSource::Typoglycemia(CClip &clip)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		CString cs(unicodeTextFormat->GetAsCString());	

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
	bool dittoFileData = false;
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

		if (pCF->m_cfType == theApp.m_DittoFileData)
		{
			dittoFileData = true;

			//save file data
			//adjust hdrop
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
		if (m_pasteOptions.m_delayRenderLockout > 0 &&
			(GetTickCount() - m_pasteOptions.m_delayRenderLockout) < (DWORD)CGetSetOptions::GetDelayRenderLockout())
		{
			bInHere = false;
			return false;
		}

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

					csIP = CTextConvert::Utf8ToUnicode(pData->m_cIP);
					csComputerName = CTextConvert::Utf8ToUnicode(pData->m_cComputerName);
					
					GlobalUnlock(pDittoDelayCF_HDROP->m_hgData);

					CString ipPort = csIP;
					if (pData->respondPort > 0)
					{
						ipPort.Format(_T("%s:%d"), csIP, pData->respondPort);
					}

					CString namePort = csComputerName;
					if (pData->respondPort > 0)
					{
						namePort.Format(_T("%s:%d"), csComputerName, pData->respondPort);
					}

					CClient cl;
					hData = cl.RequestCopiedFiles(*pCF_HDROP, ipPort, namePort);
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

	auto customDragName = g_Opt.GetTempDragFileName();
	if (customDragName != _T(""))
	{
		dragId = 1;
	}

	for (int i = 0; i < m_ClipIDs.GetCount(); i++)
	{
		CClip fileClip;
		fileClip.LoadFormats(m_ClipIDs[i]);

		CClipFormat *unicodeText = fileClip.m_Formats.FindFormat(CF_UNICODETEXT);
		if (unicodeText)
		{
			CString name = _T("text");
			CString file;
			if (customDragName != _T(""))
			{
				name = customDragName;
				file.Format(_T("%s%s.txt"), path, name);
			}
			else
			{				
				file.Format(_T("%s%s_%d.txt"), path, name, dragId++);
			}

			fileClip.WriteTextToFile(file, TRUE, FALSE, FALSE);
			fileList.AddFile(file);
			continue;
		}

		CClipFormat *asciiText = fileClip.m_Formats.FindFormat(CF_TEXT);
		if (asciiText)
		{
			CString name = _T("text");
			CString file;
			if (customDragName != _T(""))
			{
				name = customDragName;
				file.Format(_T("%s%s.txt"), path, name);
			}
			else
			{
				file.Format(_T("%s%s_%d.txt"), path, name, dragId++);
			}

			fileClip.WriteTextToFile(file, FALSE, TRUE, FALSE);
			fileList.AddFile(file);
			continue;
		}

		CClipFormat *png = fileClip.m_Formats.FindFormat(theApp.m_PNG_Format);
		CClipFormat *bitmap = fileClip.m_Formats.FindFormat(CF_DIB);
		if (bitmap != NULL ||
			png != NULL)
		{
			CString name = _T("image");
			CString file;
			if (customDragName != _T(""))
			{
				name = customDragName;
				file.Format(_T("%s%s.png"), path, name);
			}
			else
			{
				file.Format(_T("%s%s_%d.png"), path, name, dragId++);
			}

			if (fileClip.WriteImageToFile(file))
			{
				fileList.AddFile(file);
			}
		}
	}

	if(customDragName == _T("") &&
		dragId != origDragId)
	{
		CGetSetOptions::SetDragId(dragId);
	}

	HGLOBAL hData = fileList.CreateCF_HDROPBuffer();

	return hData;
}

void COleClipSource::Slugify(CClip &clip)
{
	IClipFormat *unicodeTextFormat = clip.m_Formats.FindFormatEx(CF_UNICODETEXT);
	if (unicodeTextFormat != NULL)
	{
		CString cs(unicodeTextFormat->GetAsCString());	

		//free the old text we are going to replace it below with an upper case version
		unicodeTextFormat->Free();

		CString newString = slugify(cs.GetString(), CGetSetOptions::GetSlugifySeparator().GetString()).c_str();

		long len = newString.GetLength();
		HGLOBAL hGlobal = NewGlobalP(newString.GetBuffer(), ((len + 1) * sizeof(wchar_t)));

		unicodeTextFormat->Data(hGlobal);
	}
}