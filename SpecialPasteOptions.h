#pragma once
#include "Clip.h"

class CSpecialPasteOptions
{
public:
	CSpecialPasteOptions();
	~CSpecialPasteOptions();

	bool m_pasteAsPlainText;
	bool m_pasteUpperCase;
	bool m_pasteCamelCase;
	bool m_pasteImagesHorizontal;
	bool m_pasteImagesVertically;
	bool m_pasteLowerCase;
	bool m_pasteCapitalize;
	bool m_pasteSentenceCase;
	bool m_pasteRemoveLineFeeds;
	bool m_pasteAddOneLineFeed;
	bool m_pasteAddTwoLineFeeds;
	bool m_pasteTypoglycemia;
	bool m_pasteAddingDateTime;
	CClipFormats *m_pPasteFormats;
	DWORD m_delayRenderLockout;
	bool m_dragDropFilesOnly;
	CString m_pasteScriptGuid;
	bool m_updateClipOrder;
	bool m_trimWhiteSpace;
	bool m_pasteSlugify;
	bool m_invertCase;
	bool m_placeCF_HDROP_OnDrag;
	bool m_pasteAsciiOnly;

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
			m_pasteTypoglycemia ||
			m_pasteAddingDateTime || 
			m_trimWhiteSpace ||
			m_pasteSlugify || 
			m_invertCase ||
			m_pasteCamelCase ||
			m_pasteAsciiOnly;
	}

	bool IncludeRTFForTextOnly()
	{
		return m_pasteRemoveLineFeeds ||
			m_pasteAddOneLineFeed ||
			m_pasteAddTwoLineFeeds ||
			m_pasteAddingDateTime;
	}

	CString ToString();
};

