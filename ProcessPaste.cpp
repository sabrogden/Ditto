#include "stdafx.h"
#include "CP_Main.h"
#include "ProcessPaste.h"
#include "ClipIds.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CProcessPaste::CProcessPaste()
{
	m_pOle = new COleClipSource;
	m_bSendPaste = true;
	m_bActivateTarget = true;
	m_bOnlyPaste_CF_TEXT = false;
	m_bPasteHTMLFormatAs_CF_TEXT = false;
}

CProcessPaste::~CProcessPaste()
{	
	DELETE_PTR(m_pOle);
}

BOOL CProcessPaste::DoPaste()
{
	Log(_T("Do Paste"));

	m_pOle->m_bOnlyPaste_CF_TEXT = m_bOnlyPaste_CF_TEXT;
	m_pOle->m_bPasteHTMLFormatAs_CF_TEXT = m_bPasteHTMLFormatAs_CF_TEXT;

	if(m_pOle->DoImmediateRender())
	{
		// MarkAsPasted() must be done first since it makes use of
		//  m_pOle->m_ClipIDs and m_pOle is inaccessible after
		//  SetClipboard is called.
		MarkAsPasted();
		
		// Ignore the clipboard change that we will cause IF:
		// 1) we are pasting a single element, since the element is already
		//    in the db and its lDate was updated by MarkAsPasted().
		// OR
		// 2) we are pasting multiple, but g_Opt.m_bSaveMultiPaste is false
		if(GetClipIDs().GetSize() == 1 || !g_Opt.m_bSaveMultiPaste)
		{
			m_pOle->CacheGlobalData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));
		}
		else
		{
			m_pOle->CacheGlobalData(theApp.m_cfDelaySavingData, NewGlobalP("Delay", sizeof("Delay")));
		}
		
		m_pOle->SetClipboard(); // m_pOle is now managed by the OLE clipboard

		// The Clipboard now owns the allocated memory
		// and will delete this data object
		// when new data is put on the Clipboard
		m_pOle = NULL; // m_pOle should not be accessed past this point

#ifndef _DEBUG
		if(m_bSendPaste)
		{
			theApp.SendPaste(m_bActivateTarget);
		}
#else
		if(m_bActivateTarget)
		{
			theApp.ActivateTarget();
		}
#endif

		Log(_T("Do Paste RETURN TRUE"));

		return TRUE;
	}
	return FALSE;
}

BOOL CProcessPaste::DoDrag()
{
	m_pOle->DoDelayRender();
	DROPEFFECT de = m_pOle->DoDragDrop(DROPEFFECT_COPY);
	if(de != DROPEFFECT_NONE)
	{
		MarkAsPasted();
		return TRUE;
	}
	return FALSE;
}

void CProcessPaste::MarkAsPasted()
{
	CClipIDs& clips = GetClipIDs();
	if(clips.GetSize() == 1)
		MarkClipAsPasted(clips.ElementAt(0));
}

