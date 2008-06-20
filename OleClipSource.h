#pragma once

#include "ClipIds.h"

/*------------------------------------------------------------------*\
	COleClipSource
\*------------------------------------------------------------------*/
class COleClipSource : public COleDataSource
{
	//DECLARE_DYNAMIC(COleClipSource)

public:
	CClipIDs	m_ClipIDs;
	bool		m_bLoadedFormats;
	bool		m_bOnlyPaste_CF_TEXT;
	bool		m_bPasteHTMLFormatAs_CF_TEXT;
	CClipFormats *m_pCustomPasteFormats;

	COleClipSource();
	virtual ~COleClipSource();

	BOOL DoDelayRender();
	BOOL DoImmediateRender();
	long PutFormatOnClipboard(CClipFormats *pFormats, bool bPasteHTMLFormatAs_CF_TEXT);

public:
	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);

protected:
	CClipFormats m_DelayRenderedFormats;
};