#include "stdafx.h"
#include "SpecialPasteOptions.h"


CSpecialPasteOptions::CSpecialPasteOptions()
{
	m_pasteAsPlainText = false;
	m_pPasteFormats = NULL;
	m_pasteUpperCase = false;
	m_pasteLowerCase = false;
	m_pasteCapitalize = false;
	m_pasteRemoveLineFeeds = false;
	m_pasteAddOneLineFeed = false;
	m_pasteAddTwoLineFeeds = false;
	m_pasteSentenceCase = false;
	m_pasteTypoglycemia = false;
	m_pasteAddingDateTime = false;
	m_delayRenderLockout = 0;
	m_dragDropFilesOnly = false;
	m_pasteScriptGuid = _T("");
}

CSpecialPasteOptions::~CSpecialPasteOptions()
{
}

CString CSpecialPasteOptions::ToString()
{
	CString cs;
	cs.Format(_T("Plain Text: %d"), m_pasteAsPlainText);

	return cs;
}
