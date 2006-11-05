#include "stdafx.h"
#include "CP_Main.h"
#include "OleClipSource.h"
#include "TextConvert.h"

/*------------------------------------------------------------------*\
COleClipSource
\*------------------------------------------------------------------*/
//IMPLEMENT_DYNAMIC(COleClipSource, COleDataSource)
COleClipSource::COleClipSource()
{
	m_bLoadedFormats = false;
	m_bOnlyPaste_CF_TEXT = false;
	m_bPasteHTMLFormatAs_CF_TEXT = false;
}

COleClipSource::~COleClipSource()
{
	
}

BOOL COleClipSource::DoDelayRender()
{
	CClipTypes types;
	m_ClipIDs.GetTypes(types);
	
	int count = types.GetSize();
	for(int i=0; i < count; i++)
		DelayRenderData(types[i]);
	
	return count;
}

#include "Client.h"

BOOL COleClipSource::DoImmediateRender()
{
	if(m_bLoadedFormats)
		return TRUE;

	m_bLoadedFormats = true;
	
	int count = m_ClipIDs.GetSize();
	if(count <= 0)
		return 0;

	if(count == 1)
	{
		CClip clip;
		CClipFormats formats;

		clip.LoadFormats(m_ClipIDs[0], m_bOnlyPaste_CF_TEXT);
		
		return PutFormatOnClipboard(&clip.m_Formats, m_bPasteHTMLFormatAs_CF_TEXT);
	}
	
	HGLOBAL hGlobal;

	CStringA csAText;
	if(m_ClipIDs.AggregateText(CF_TEXT, "\r\n", g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop, csAText))
	{
		long lLen = csAText.GetLength();
		hGlobal = NewGlobalP(csAText.GetBuffer(lLen), lLen+sizeof(char));
		csAText.ReleaseBuffer();
		CacheGlobalData(CF_TEXT, hGlobal);
	}
	
	CStringW Sep = _T("\r\n");
	CStringW csWText;
	if(m_ClipIDs.AggregateUnicodeText(CF_UNICODETEXT, Sep, g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop, csWText))
	{
		long lLen = csWText.GetLength() * sizeof(wchar_t);
		hGlobal = NewGlobalP(csWText.GetBuffer(lLen), lLen+sizeof(wchar_t));
		csWText.ReleaseBuffer();
		CacheGlobalData(CF_UNICODETEXT, hGlobal);
	}

//	text = "{\rtf1";
//	text += m_ClipIDs.AggregateText(GetFormatID(CF_RTF), "\r\n", true);
//	text += "}";
//	
//	hGlobal = NewGlobalP((void*)(LPCSTR) text, text.GetLength()+1);
//	CacheGlobalData(GetFormatID(CF_RTF), hGlobal);

	return hGlobal != 0;
}

long COleClipSource::PutFormatOnClipboard(CClipFormats *pFormats, bool bPasteHTMLFormatAs_CF_TEXT)
{
	CClipFormat* pCF;
	bool bDelayedRenderCF_HDROP = false;
	int	count = pFormats->GetSize();
	int i = 0;

	//see if the html format is in the list
	//if it is the list we will not paste CF_TEXT
	for(i = 0; i < count; i++)
	{
		pCF = &pFormats->ElementAt(i);

		if(bPasteHTMLFormatAs_CF_TEXT)
		{
			if(pCF->m_cfType == theApp.m_HTML_Format)
				break;
		}

		if(pCF->m_cfType == theApp.m_RemoteCF_HDROP)
		{
			bDelayedRenderCF_HDROP = true;
		}
	}

	//Didn't find html format
	if(i == count)
		bPasteHTMLFormatAs_CF_TEXT = false;
	
	for(i = 0; i < count; i++)
	{
		pCF = &pFormats->ElementAt(i);

		if(bDelayedRenderCF_HDROP)
		{
			if(pCF->m_cfType == CF_HDROP)
			{
				DelayRenderData(pCF->m_cfType);
			}

			continue;
		}
		
		if(bPasteHTMLFormatAs_CF_TEXT)
		{
			if(pCF->m_cfType == CF_TEXT)
				continue;

			if(pCF->m_cfType == theApp.m_HTML_Format)
				pCF->m_cfType = CF_TEXT;
		}

		CacheGlobalData( pCF->m_cfType, pCF->m_hgData );
		pCF->m_hgData = 0; // OLE owns it now
	}

	pFormats->RemoveAll();

	m_bLoadedFormats = true;

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
		CClip clip;

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

		//Add to a cache of already rendered data
		//Windows seems to call this function multiple times
		//so only the first time do we need to go get the data
		HGLOBAL hCopy = NULL;
		if(hData)
		{
			hCopy = NewGlobalH(hData, GlobalSize(hData));
		}

		CClipFormat format(lpFormatEtc->cfFormat, hCopy);
		format.bDeleteData = false; //owned by m_DelayRenderedFormats
		m_DelayRenderedFormats.Add(format);
	}

	BOOL bRet = FALSE;
	if(hData)
	{
		// if phGlobal is null, we can just give the allocated mem
		// else, our data must fit within the GlobalSize(*phGlobal)
		if(*phGlobal == 0)
			*phGlobal = hData;
		else
		{
			UINT len = min(::GlobalSize(*phGlobal), ::GlobalSize(hData));
			if(len)
				CopyToGlobalHH(*phGlobal, hData, len);
			::GlobalFree(hData);
		}
		bRet = TRUE;
	}

	bInHere = false;

	return bRet;
}