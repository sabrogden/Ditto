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

// returns the increment necessary to fit "count" elements between (dStart,dEnd)
// if this returns 0, then "count" elements cannot fit between (dStart,dEnd).
double GetFitIncrement( int count, double dStart, double dEnd );

/*------------------------------------------------------------------*\
	ID based Globals
\*------------------------------------------------------------------*/

// Sets lID's lDate to GetCurrentTime() and updates paste stats
BOOL MarkClipAsPasted(long lID);

long NewGroupID( long lParentID = 0, CString text = "" );
// creates copies of all lSrc Data and returns the copy's lDataID (or 0 on fail)
long NewCopyDataID( long lSrc );

BOOL DeleteID( long lID, bool bDisband = true ); // deletes the given item
// deletes all items in the group, but not the group record itself
BOOL DeleteGroupID( long lGroupID, bool bDisband = true );
BOOL DeleteDataID( long lDataID ); // deletes all data for the given ID
// Deletes everything in the Main and Data tables
BOOL DeleteAllIDs();

// all "formatIDs" (Data.lID) must be elements of the "lDataID" (Data.lDataID) set
BOOL DeleteFormats( long lDataID, ARRAY& formatIDs );


/*------------------------------------------------------------------*\
	CClipIDs - an array of Clip IDs
\*------------------------------------------------------------------*/

class CClipIDs : public CArrayEx<int>
{
public:
// PASTING FUNCTIONS

	// allocate an HGLOBAL of the given Format Type representing the Clip IDs in this array.
	HGLOBAL	Render( UINT cfType );
	// Fills "types" with the Format Types corresponding to the Clip IDs in this array.
	void GetTypes( CClipTypes& types );
	// Aggregates the cfType Format Data of the Clip IDs in this array, assuming
	//  each Format is NULL terminated and placing pSeparator between them.
	CString AggregateText( UINT cfType, char* pSeparator );

// MANAGEMENT FUNCTIONS

	// returns the address of the given id in this array or NULL.
	long* FindID( long lID );

	// Blindly Moves IDs into the lParentID Group sequentially with the given order
	// (i.e. this does not check to see if the IDs' order conflict)
	// if( dIncrement < 0 ), this does not change the order
	BOOL MoveTo( long lParentID, double dFirst = 0, double dIncrement = -1 );

	// reorders the "lParentID" Group, inserting before the given id.
	//  if the id cannot be found, this appends the IDs.
	BOOL ReorderGroupInsert( long lParentID, long lInsertBeforeID = 0 );

	// Empties this array and fills it with the elements of the given group ID
	BOOL LoadElementsOf( long lGroupID );

	// Creates copies (duplicates) of all items in this array and assigns the
	// lParentID of the copies to the given "lParentID" group.
	// - if lParentID <= 0, then the copies have the same parent as the source
	// - pCopies is filled with the corresponding duplicate IDs.
	// - pAddNewTable and pSeekTable are used for more efficient recursion.
	BOOL CopyTo( long lParentID, CClipIDs* pCopies = NULL,
		CMainTable* pAddNewTable = NULL, CMainTable* pSeekTable = NULL );

	BOOL DeleteIDs( bool bDisband = true );
};

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

	COleClipSource();
	virtual ~COleClipSource();

	BOOL DoDelayRender();
	BOOL DoImmediateRender();
	long LoadFormats(CClipFormats *pFormats);

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
	COleClipSource*	m_pOle;
	bool m_bSendPaste;
	bool m_bActivateTarget;
	bool m_bOnlyPaste_CF_TEXT;

	CProcessPaste();
	~CProcessPaste();

	CClipIDs& GetClipIDs() { return m_pOle->m_ClipIDs; }

	BOOL DoPaste();
	BOOL DoDrag();

	void MarkAsPasted();
};

#endif // !defined(AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
