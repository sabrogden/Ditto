#pragma once
#include "Clip.h"

class CSpecialPasteOptions
{
public:
	CSpecialPasteOptions();
	~CSpecialPasteOptions();

	bool m_pasteAsPlainText;
	bool m_pasteUpperCase;
	bool m_pasteLowerCase;
	bool m_pasteCapitalize;
	bool m_pasteSentenceCase;
	CClipFormats *m_pPasteFormats;

	bool LimitFormatsToText() { return m_pasteAsPlainText || m_pasteUpperCase || m_pasteLowerCase || m_pasteCapitalize || m_pasteSentenceCase; }

	CString ToString();
};

