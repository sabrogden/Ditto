// ProcessPaste.h: interface for the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
#define AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArrayEx.h"
#include "ProcessCopy.h"

// Sends Ctrl-V to hWnd
void SendPaste(HWND hWnd);
// Sets lID's lDate to GetCurrentTime() and updates paste stats
BOOL MarkClipAsPasted(long lID);

/*------------------------------------------------------------------*\
	CClipIDs - an array of Clip IDs
\*------------------------------------------------------------------*/
class CClipIDs : public ARRAY
{
public:
	// allocate an HGLOBAL of the given Format Type representing the Clip IDs in this array.
	HGLOBAL	Render( UINT cfType );
	// Fills "types" with the Format Types corresponding to the Clip IDs in this array.
	void GetTypes( CClipTypes& types );
	// Aggregates the cfType Format Data of the Clip IDs in this array, assuming
	//  each Format is NULL terminated and placing pSeparator between them.
	CString AggregateText( UINT cfType, char* pSeparator );
};

/*------------------------------------------------------------------*\
	COleClipSource
\*------------------------------------------------------------------*/
class COleClipSource : public COleDataSource
{
	DECLARE_DYNAMIC(COleClipSource)

public:
	CClipIDs	m_ClipIDs;

	COleClipSource();
	virtual ~COleClipSource();

	BOOL DoDelayRender();
	BOOL DoImmediateRender();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
};

/*------------------------------------------------------------------*\
	CProcessPaste
\*------------------------------------------------------------------*/
class CProcessPaste
{
public:
	HWND	m_hWnd; // window to paste to
	// true if we should delete m_pOle,
	// false if it will be automatically deleted
	bool	m_bDeleteOle;
	COleClipSource*	m_pOle;

	CProcessPaste( HWND hWnd = 0 );
	~CProcessPaste();

	CClipIDs& GetClipIDs() { return m_pOle->m_ClipIDs; }

	BOOL DoPaste();
	BOOL DoDrag();

	void MarkAsPasted();
};

#endif // !defined(AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
