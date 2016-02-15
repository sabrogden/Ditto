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
	bool m_pasteRemoveLineFeeds;
	bool m_pasteAddOneLineFeed;
	bool m_pasteAddTwoLineFeeds;
	bool m_pasteTypoglycemia;
	CClipFormats *m_pPasteFormats;

	bool LimitFormatsToText() 
	{ 
		return m_pasteAsPlainText ||
			m_pasteUpperCase ||
			m_pasteLowerCase ||
			m_pasteCapitalize ||
			m_pasteSentenceCase ||
			m_pasteRemoveLineFeeds ||
			m_pasteAddOneLineFeed ||
			m_pasteAddTwoLineFeeds ||
			m_pasteTypoglycemia;
	}

	bool IncludeRTFForTextOnly()
	{
		return m_pasteRemoveLineFeeds ||
			m_pasteAddOneLineFeed ||
			m_pasteAddTwoLineFeeds;
	}

	CString ToString();
};

