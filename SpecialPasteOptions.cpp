#include "stdafx.h"
#include "SpecialPasteOptions.h"


CSpecialPasteOptions::CSpecialPasteOptions()
{
	m_pasteAsPlainText = false;
	m_pPasteFormats = NULL;
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
