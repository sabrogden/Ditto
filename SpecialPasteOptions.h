#pragma once
#include "Clip.h"

class CSpecialPasteOptions
{
public:
	CSpecialPasteOptions();
	~CSpecialPasteOptions();

	bool m_pasteAsPlainText;
	bool m_pasteRTFNoTextHighlight;
	CClipFormats *m_pPasteFormats;

	CString ToString();
};

