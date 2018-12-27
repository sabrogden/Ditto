#pragma once

#include "ClipIds.h"
#include "SpecialPasteOptions.h"

/*------------------------------------------------------------------*\
	COleClipSource
\*------------------------------------------------------------------*/
class COleClipSource : public COleDataSource
{
	//DECLARE_DYNAMIC(COleClipSource)

public:
	CClipIDs	m_ClipIDs;
	bool		m_bLoadedFormats;
	CSpecialPasteOptions m_pasteOptions;

	COleClipSource();
	virtual ~COleClipSource();

	BOOL DoDelayRender();
	BOOL DoImmediateRender();

	void PlainTextFilter(CClip &clip);

	INT_PTR PutFormatOnClipboard(CClipFormats *pFormats);

public:
	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);

protected:
	CClipFormats m_DelayRenderedFormats;
	bool m_convertToHDROPOnDelayRender;

	void DoUpperLowerCase(CClip &clip, bool upper);
	void Capitalize(CClip &clip);
	void SentenceCase(CClip &clip);
	void RemoveLineFeeds(CClip &clip);
	void AddLineFeeds(CClip &clip, int count);
	void Typoglycemia(CClip &clip);
	HGLOBAL ConvertToFileDrop();
	void AddDateTime(CClip &clip);
	void SaveDittoFileDataToFile(CClip &clip);
	void TrimWhiteSpace(CClip &clip);
	void Slugify(CClip &clip);
	void InvertCase(CClip &clip);
};