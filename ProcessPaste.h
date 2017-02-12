// ProcessPaste.h: interface for the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
#define AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Shared/ArrayEx.h"
#include "Clip.h"
#include "ClipIds.h"
#include "OleClipSource.h"
#include "SpecialPasteOptions.h"

/*------------------------------------------------------------------*\
	CProcessPaste
\*------------------------------------------------------------------*/
class CProcessPaste
{
public:
	COleClipSource*	m_pOle;
	bool m_bSendPaste;
	bool m_bActivateTarget;
	CSpecialPasteOptions m_pasteOptions;
	bool m_pastedFromGroup;

	struct MarkAsPastedData 
	{
		CClipIDs ids;
		bool pastedFromGroup;
	};
	
	CProcessPaste();
	~CProcessPaste();

	CClipIDs& GetClipIDs() { return m_pOle->m_ClipIDs; }

	BOOL DoPaste();
	BOOL DoDrag();

	void MarkAsPasted();
	static UINT MarkAsPastedThread(LPVOID pParam);
};

#endif // !defined(AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
