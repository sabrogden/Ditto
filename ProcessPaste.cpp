#include "stdafx.h"
#include "CP_Main.h"
#include "ProcessPaste.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// returns the increment necessary to fit "count" elements between (dStart,dEnd)
// if this returns 0, then "count" elements cannot fit between (dStart,dEnd).
double GetFitIncrement( int count, double dStart, double dEnd )
{
	VERIFY( count > 0 && dStart <= dEnd );
	
	double dIncrement = (dEnd - dStart) / ((double) (count+1));
	
	if( dIncrement == 0 )
		return 0;
	
	// verify that each element is unique
	// I'm not sure if this is necessary, but I'm doing it just to be on the safe side
	// I think the resolution of floating points are variable.
	// i.e. we cannot depend upon an increment always effecting a change.
	int i = 0;
	double dPrev = dStart;
	double dOrder = dStart + dIncrement;
	while( dOrder < dEnd )
	{
		i++;
		if( dOrder <= dPrev )
			return 0;
		dPrev = dOrder;
		dOrder = dOrder + dIncrement;
	}
	
	// verify count (and that we're not too close to dEnd)
	if( i < count )
		return 0;
	
	return dIncrement;
}


/*------------------------------------------------------------------*\
ID based Globals
\*------------------------------------------------------------------*/

BOOL MarkClipAsPasted(long lID)
{
	CGetSetOptions::SetTripPasteCount(-1);
	CGetSetOptions::SetTotalPasteCount(-1);
	
	if( !g_Opt.m_bUpdateTimeOnPaste )
		return FALSE;
	
	try
	{	
		//Update the time it was copied so that it appears at the top of the 
		//paste list.  Items are sorted by this time.
		CMainTable ctMain;
		ctMain.Open("SELECT * FROM Main WHERE lID = %d", lID);
		ctMain.Edit();
		
		CTime now = CTime::GetCurrentTime();
		ctMain.m_lDate = (long)now.GetTime();
		
		ctMain.Update();
		ctMain.Close();
		
		return TRUE;
	}
	catch(CDaoException *e)
	{
		ASSERT(FALSE);
		e->Delete();
	}
	
	return FALSE;
}

long NewGroupID( long lParentID, CString text )
{
	long lID=0;
	CTime time;
	time = CTime::GetCurrentTime();
	
	try
	{
		CMainTable recset;
		recset.Open(dbOpenTable, "Main");
		
		recset.AddNew(); // overridden to set m_lID to the new autoincr number
		
		lID = recset.m_lID;
		
		recset.m_lDate = (long) time.GetTime();
		recset.m_lDontAutoDelete = (long) time.GetTime();
		
		if( text != "" )
			recset.m_strText = text;
		else
			recset.m_strText = time.Format("NewGroup %y/%m/%d %H:%M:%S");
		
		recset.m_bIsGroup = TRUE;
		recset.m_lParentID = lParentID;
		
		recset.Update();
		
		//		recset.SetBookmark( recset.GetLastModifiedBookmark() );
		//		lID = recset.m_lID;
		
		recset.Close();
	}
	catch(CDaoException* e)
	{
		e->ReportError();
		ASSERT(FALSE);
		e->Delete();
		return 0;
	}
	
	return lID;
}

// UNTESTED SQL
// creates copies of all lSrc Data and returns the copy's lDataID (or 0 on fail)
long NewCopyDataID( long lSrc )
{
	long lDataID = 0;
	CString sql;
	
	// create the copies
	sql.Format(
		"INSERT INTO Data (strClipBoardFormat, ooData) "
		"SELECT strClipBoardFormat, ooData FROM Data "
		"WHERE lDataID = %d", lSrc );
	
	// each lID should be assigned a unique ID (autoinc)
	// lDataID should be assigned to 0 (default value) or NULL
	ExecuteSQL( sql );
	
	// assign lDataID to the first record's lID
	try
	{
		CDataTable recs;
		recs.Open("SELECT * FROM Data WHERE lDataID = 0 OR lDataID IS NULL");
		if( !recs.IsEOF() )
			lDataID = recs.m_lID;
		recs.Close();
	}
	CATCHDAO
		
		// assign the copies to lDest
		sql.Format(
		"UPDATE Data "
		"SET lDataID = %d "
		"WHERE lDataID = 0 OR lDataID IS NULL", lDataID );
	
	ExecuteSQL( sql );
	
	return lDataID;
}

// deletes the given item
BOOL DeleteID( long lID, bool bDisband )
{
	BOOL bIsGroup;
	long lDataID = 0;
	int i = 0;
	COleVariant varKey( (long) 0 ); // VT_I4
	
	try
	{
		CMainTable recs;
		recs.Open( dbOpenTable, "Main" );
		recs.SetCurrentIndex("lID"); // set "Seek" to use this index.
		
		varKey.lVal = lID;
		// Goto the record whose [lID] field == varKey
		if( !recs.Seek( _T("="), &varKey ) )
		{
			ASSERT(0);
			return FALSE;
		}
		
		lDataID = recs.m_lDataID;
		bIsGroup = recs.m_bIsGroup;
		
		// must delete this record first so that DeleteDataID can properly
		//  determine if any other Clip is using the same Data.
		recs.Delete(); 
		
		if( bIsGroup )
			DeleteGroupID( lID, bDisband );
		else
			DeleteDataID( lDataID );
		
		recs.Close();
	}
	CATCHDAO
		
		return TRUE;
}

// deletes all items in the group
BOOL DeleteGroupID( long lGroupID, bool bDisband )
{
	if( bDisband )
	{
		return ExecuteSQL( 
			StrF("UPDATE Main SET lParentID = 0 WHERE lParentID = %d", lGroupID) );
	}
	
	try
	{
		CMainTable recs;
		recs.Open( "SELECT * FROM Main WHERE lParentID = %d", lGroupID );
		
		while( !recs.IsEOF() )
		{
			DeleteID( recs.m_lID );
			recs.MoveNext();
		}
		
		recs.Close();
	}
	CATCHDAO
		
		return TRUE;
}

// deletes all data for the given ID
// NOTE!: this checks to see if there are any records in Main which reference
//  the given lDataID, and if so, does NOT delete the Data.
// THEREFORE, Main records should be deleted BEFORE calling this function.
BOOL DeleteDataID( long lDataID )
{
	CMainTable recs;
	BOOL bEmpty;
	// check to see if the data is referenced by any Clips
	recs.Open( "SELECT * FROM Main WHERE lDataID = %d", lDataID );
	bEmpty = recs.IsEOF();
	recs.Close();
	// if the data is no longer referenced, delete the data
	if( bEmpty )
        return ExecuteSQL( StrF("DELETE FROM Data WHERE lDataID = %d", lDataID) );
	// else, there are more clips which use the data
	return TRUE;
}

BOOL DeleteAllIDs()
{
	CMainTable MainTable;
	CDataTable DataTable;
	MainTable.DeleteAll();
	DataTable.DeleteAll();
	if( CompactDatabase() )
	{
		RepairDatabase();
	}
	return TRUE;
}

// all "formatIDs" (Data.lID) must be elements of the "lDataID" (Data.lDataID) set
BOOL DeleteFormats( long lDataID, ARRAY& formatIDs )
{
	long lNewTotalSize = 0;
	bool bIsHeadDeleted = false;
	long lNewDataID = 0;
	
	if( formatIDs.GetSize() <= 0 )
		return TRUE;
	
	formatIDs.SortAscending();
	
	try
	{
		CDataTable recs;
		recs.Open("SELECT * FROM Data WHERE lDataID = %d", lDataID);
		//Go through the data table and find the deleted items
		recs.MoveFirst();
		while(!recs.IsEOF())
		{
			if(formatIDs.Find(recs.m_lID))
			{
				// if we are deleting the head, then we need a new head
				// actually, this might not be absolutely necessary if
				//  lID autoincr field never reuses IDs.
				if( lDataID == recs.m_lID )
					bIsHeadDeleted = true;
				
				recs.Delete();
			}
			else
				lNewTotalSize += recs.m_ooData.m_dwDataLength;
			
			recs.MoveNext();
		}
		
		if( bIsHeadDeleted )
		{
			recs.MoveFirst();
			if( !recs.IsEOF() )
				lNewDataID = recs.m_lID;
			recs.Close();
			// update the Main Table with lNewTotalSize and lNewDataID
			ExecuteSQL( StrF(
				"UPDATE Main SET lTotalCopySize = %d, lDataID = %d WHERE lDataID = %d",
				lNewTotalSize, lNewDataID, lDataID) );
			// update the Data Table with lNewDataID
			ExecuteSQL( StrF(
				"UPDATE Data SET lDataID = %d WHERE lDataID = %d",
				lNewDataID, lDataID) );
		}
		else // still update the total copy size
		{
			recs.Close();
			ExecuteSQL( StrF(
				"UPDATE Main SET lTotalCopySize = %d WHERE lDataID = %d",
				lNewTotalSize, lDataID) );
		}
	}
	CATCHDAO
		
		return TRUE;
}


/*------------------------------------------------------------------*\
CClipIDs
\*------------------------------------------------------------------*/

//-------------------
// PASTING FUNCTIONS
//-------------------

// allocate an HGLOBAL of the given Format Type representing these Clip IDs.
HGLOBAL CClipIDs::Render( UINT cfType )
{
	int count = GetSize();
	if(count <= 0)
		return 0;
	if(count == 1)
		return CClip::LoadFormat(ElementAt(0), cfType);
	CString text = AggregateText(CF_TEXT, "\r\n", g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop);

	return NewGlobalP( (void*)(LPCSTR) text, text.GetLength()+1 );
}

void CClipIDs::GetTypes( CClipTypes& types )
{
	int count = GetSize();
	types.RemoveAll();
	if( count > 1 )
		types.Add( CF_TEXT );
	else if( count == 1 )
		CClip::LoadTypes( ElementAt(0), types );
}

// Aggregates the cfType Format Data of the Clip IDs in this array, assuming
//  each Format is NULL terminated and placing pSeparator between them.
// This assumes that the given cfType is a null terminated text type.
CString CClipIDs::AggregateText(UINT cfType, char* pSeparator, BOOL bReverse)
{
	CString csSQL;
	CDataTable recset;
	CString text;
	char* pData = NULL;
	DWORD len;
	DWORD maxLen;

	CLIPFORMAT cfRTF = GetFormatID(CF_RTF);
	
	// maybe we should sum up the "recset.m_ooData.m_dwDataLength" of all IDs first
	//  in order to determine the max space required??  Or would that be wastefull?
	
	// allocate a large initial buffer to minimize realloc for concatenations
	text.GetBuffer(1000);
	text.ReleaseBuffer(0);
	
	int numIDs = GetSize();
	int* pIDs = GetData();
	
	csSQL.Format(
		"SELECT Data.* FROM Data "
		"INNER JOIN Main ON Main.lDataID = Data.lDataID "
		"WHERE Data.strClipBoardFormat = \'%s\' "
		"AND Main.lID = %%d",
		GetFormatName(cfType));
	
	try
	{
		int nIndex;
		for( int i=0; i < numIDs; i++ )
		{
			nIndex = i;
			if(bReverse)
			{
				nIndex = numIDs - i - 1;
			}

			recset.Open( csSQL, pIDs[nIndex] );
			if( !recset.IsBOF() && !recset.IsEOF() )
			{
				maxLen = recset.m_ooData.m_dwDataLength;
				if( maxLen == 0 )
					continue;
				pData = (char*) GlobalLock(recset.m_ooData.m_hData);
				ASSERT( pData );
				
				// verify that pData is null terminated 
				// do a quick check to see if the last character is null
				if( pData[maxLen-1] != '\0' )
				{
					for( len=0; len < maxLen && pData[len] != '\0'; len++ ) {}
					// if it is not null terminated, skip this item
					if( len >= maxLen )
						continue;
				}

//				if(i == 0 && cfType == cfRTF)
//				{
//					pData[maxLen-2] = '\0';
//				}

				text += pData;
				GlobalUnlock(recset.m_ooData.m_hData);
				
				if( pSeparator )
					text += pSeparator;

//				if((i == numIDs-1) && cfType == cfRTF)
//				{
//					text += "}";
//				}
			}
			recset.Close();
		}
	}
	CATCHDAO
		
	return text;
}

//----------------------------------------------
// ELEMENT (Clip or Group) MANAGEMENT FUNCTIONS
//----------------------------------------------

// returns the address of the given id in this array or NULL.
long* CClipIDs::FindID( long lID )
{
	int count = GetSize();
	long* pID = (long*) GetData();
	for( int i=0; i < count; i++ )
	{
		if( *pID == lID )
			return pID;
		pID++;
	}
	return NULL;
}

// Blindly Moves IDs into the lParentID Group sequentially with the given order
// (i.e. this does not check to see if the IDs' order conflict)
// if( dIncrement < 0 ), this does not change the order
BOOL CClipIDs::MoveTo( long lParentID, double dFirst, double dIncrement )
{
	int count = GetSize();
	int i = 0;
	COleVariant varKey( (long) 0 ); // VT_I4
	double dOrder = dFirst;
	BOOL bChangeOrder = (dIncrement >= 0);
	
	try
	{
		CMainTable recs;
		recs.Open( dbOpenTable, "Main" );
		recs.SetCurrentIndex("lID"); // set "Seek" to use this index.
		
		// for each id, assign lParentID
		while( i < count )
		{
			varKey.lVal = ElementAt(i);
			// Goto the record whose [lID] field == varKey
			if( !recs.Seek( _T("="), &varKey ) )
			{
				ASSERT(0);
				break;
			}
			
			// don't allow an item to become its own parent
			// NOTE!: deeper recursion is not checked, so it is theoretically
			//  possible for: A -> B -> A
			if( recs.m_lID != lParentID )
			{
				recs.Edit();
				recs.m_lParentID = lParentID;
				
				recs.m_lDontAutoDelete = (long)CTime::GetCurrentTime().GetTime();
				if( bChangeOrder )
					recs.m_dOrder = dOrder;
				recs.Update();
				dOrder = dOrder + dIncrement;
			}
			
			i++;
		}
		recs.Close();
	}
	CATCHDAO
		
		return (i == count);
}

// reorders the "lParentID" Group, inserting before the given id.
//  if the id cannot be found, this appends the IDs.
BOOL CClipIDs::ReorderGroupInsert( long lParentID, long lInsertBeforeID )
{
	int count = GetSize();
	int i = 1; // start the enumeration
	int insert = 0;
	BOOL bResult;
	
	try
	{
		MoveTo(-1); // move all elements outside any group
		
		CMainTable recs;
		recs.m_strSort = "dOrder ASC";
		recs.Open( "SELECT * FROM Main WHERE lParentID = %d", lParentID );
		
		while( !recs.IsEOF() )
		{
			if( recs.m_lID == lInsertBeforeID )
			{
				insert = i;
				i = insert + count;
			}
			
			recs.Edit();
			recs.m_dOrder = i;
			recs.Update();
			
			i++;
			recs.MoveNext();
		}
		
		recs.Close();
		
		if( insert == 0 )
			insert = i;
		
		// move the elements into their proper position in the group
		bResult = MoveTo( lParentID, (double) insert, (double) 1 );
	}
	CATCHDAO
		
		return bResult;
}

// Empties this array and fills it with the elements of the given group ID
BOOL CClipIDs::LoadElementsOf( long lGroupID )
{
	int fldID; // index of the lID field
	COleVariant varID; // value of the lID field
	int count = 0;
	
	SetSize(0);
	
	try
	{
		CMainTable recs;
		recs.SetBindFields(false);
		recs.Open("SELECT lID FROM Main WHERE lParentID = %d", lGroupID);
		
		fldID = GetFieldPos(recs,"lID");
		VERIFY( fldID == 0 ); // should be 0 since it is the only field
		
		while( !recs.IsEOF() )
		{
			recs.GetFieldValue( fldID, varID );
			Add( varID.lVal );
			recs.MoveNext();
		}
		
		recs.Close();
	}
	CATCHDAO
		
		return GetSize();
}

// Creates copies (duplicates) of all items in this array and assigns the
// lParentID of the copies to the given "lParentID" group.
// - if lParentID <= 0, then the copies have the same parent as the source
// - pCopies is filled with the corresponding duplicate IDs.
// - pAddNewTable and pSeekTable are used for more efficient recursion.
// - the primary overhead for recursion is one ID array per level deep.
//   an alternative design would be to have one CMainTable per level deep,
//   but I thought that might be too costly, so I implemented it this way.
BOOL CClipIDs::CopyTo( long lParentID, CClipIDs* pCopies,
					  CMainTable* pAddNewTable, CMainTable* pSeekTable )
{
	int count = GetSize();
	if( pCopies )
	{
		pCopies->SetSize( count );
		// initialize all IDs to 0
		for( int i=0; i < count; i++ )
			pCopies->ElementAt(i) = 0;
	}
	if( count == 0 )
		return TRUE;
	
	// for Seeking
	BOOL bSeekFailed = FALSE;
	COleVariant varID( (long) 0, VT_I4 );
	
	// for recursing into groups
	CMainTable* pAddTable = pAddNewTable;
	CMainTable* pTable = pSeekTable;
	CClipIDs groupIDs;
	
	long lCopyID;
	
	try
	{
		if( pTable == NULL )
		{
			pTable = new CMainTable;
            pTable->Open(dbOpenTable,"Main");
			pTable->SetCurrentIndex("lID");
		}
		
		if( pAddTable == NULL )
		{
			pAddTable = new CMainTable;
            pAddTable->Open(dbOpenTable,"Main");
			//			pAddTable->SetCurrentIndex("lID");
		}
		
		for( int i=0; i < count; i++ )
		{
			varID.lVal = ElementAt(i);
			// Find first record whose [lID] field == lID
			if( pTable->Seek(_T("="), &varID) )
			{
				// copy the record
				pAddTable->AddNew(); // overridden to fetch autoincr lID
				lCopyID = pAddTable->m_lID;
				pAddTable->CopyRec( *pTable ); // copy the fields
				pAddTable->m_lDontAutoDelete  = (long)CTime::GetCurrentTime().GetTime();
				if( lParentID > 0 ) // if valid, assign the given parent
					pAddTable->m_lParentID = lParentID;
				pAddTable->Update();
				
				// if it's a group, copy its elements
				if( pTable->m_bIsGroup )
				{
					groupIDs.LoadElementsOf( pTable->m_lID );
					// RECURSION
					groupIDs.CopyTo( lCopyID, NULL, pAddTable, pTable );
				}
			}
			else
			{
				ASSERT(0);
				bSeekFailed = TRUE;
				break;
			}
			if( pCopies )
				pCopies->ElementAt(i) = lCopyID;
		}
		
		// if we were not given the table, then we created it, so we must delete it
		if( pAddTable && pAddNewTable == NULL )
		{
			pAddTable->Close();
			delete pAddTable;
		}
		if( pTable && pSeekTable == NULL )
		{
			pTable->Close();
			delete pTable;
		}
	}
	CATCHDAO
		
		return !bSeekFailed;
}

BOOL CClipIDs::DeleteIDs( bool bDisband )
{
	CPopup status(0,0,::GetForegroundWindow());
	bool bAllowShow;
	bAllowShow = IsAppWnd(::GetForegroundWindow());
	
	BOOL bRet = TRUE;
	int count = GetSize();
	
	if(count <= 0)
		return FALSE;
	
	for( int i=0; i < count; i++ )
	{
		if( bAllowShow )
			status.Show( StrF("Deleting %d out of %d...",i+1,count) );
		bRet = bRet && DeleteID( ElementAt(i), bDisband );
	}
	
	return bRet;
}


/*------------------------------------------------------------------*\
COleClipSource
\*------------------------------------------------------------------*/
//IMPLEMENT_DYNAMIC(COleClipSource, COleDataSource)
COleClipSource::COleClipSource()
{
	m_bLoadedFormats = false;
	m_bOnlyPaste_CF_TEXT = false;
	m_bPasteHTMLFormatAs_CF_TEXT = false;
}

COleClipSource::~COleClipSource()
{
}

BOOL COleClipSource::DoDelayRender()
{
	CClipTypes types;
	m_ClipIDs.GetTypes( types );
	
	int count = types.GetSize();
	for( int i=0; i < count; i++ )
		DelayRenderData( types[i] );
	
	return count;
}

BOOL COleClipSource::DoImmediateRender()
{
	if(m_bLoadedFormats)
		return TRUE;

	m_bLoadedFormats = true;
	
	int count = m_ClipIDs.GetSize();
	if(count <= 0)
		return 0;
	if(count == 1)
	{
		CClipFormats formats;
		
		CClip::LoadFormats(m_ClipIDs[0], formats, m_bOnlyPaste_CF_TEXT);
		
		return LoadFormats(&formats, m_bPasteHTMLFormatAs_CF_TEXT);
	}
	
	HGLOBAL hGlobal;
	
	CString text = m_ClipIDs.AggregateText(CF_TEXT, "\r\n", g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop);
	hGlobal = NewGlobalP((void*)(LPCSTR) text, text.GetLength()+1);
	CacheGlobalData(CF_TEXT, hGlobal);
	
//	text = "{\rtf1";
//	text += m_ClipIDs.AggregateText(GetFormatID(CF_RTF), "\r\n", true);
//	text += "}";
//	
//	hGlobal = NewGlobalP((void*)(LPCSTR) text, text.GetLength()+1);
//	CacheGlobalData(GetFormatID(CF_RTF), hGlobal);

	return hGlobal != 0;
}

long COleClipSource::LoadFormats(CClipFormats *pFormats, bool bPasteHTMLFormatAs_CF_TEXT)
{
	CClipFormat* pCF;

	int	count = pFormats->GetSize(); // reusing "count"

	if(bPasteHTMLFormatAs_CF_TEXT)
	{
		//see if the html format is in the list
		//if it is the list we will not paste CF_TEXT
		for(int i = 0; i < count; i++)
		{
			pCF = &pFormats->ElementAt(i);

			if(pCF->m_cfType == theApp.m_HTML_Format)
				break;
		}

		if(i == count)
			bPasteHTMLFormatAs_CF_TEXT = false;
	}

	for(int i = 0; i < count; i++)
	{
		pCF = &pFormats->ElementAt(i);

		if(bPasteHTMLFormatAs_CF_TEXT)
		{
			if(pCF->m_cfType == CF_TEXT)
				continue;

			if(pCF->m_cfType == theApp.m_HTML_Format)
				pCF->m_cfType = CF_TEXT;
		}

		CacheGlobalData( pCF->m_cfType, pCF->m_hgData );
		pCF->m_hgData = 0; // OLE owns it now
	}
	pFormats->RemoveAll();

	m_bLoadedFormats = true;

	return count;
}


BEGIN_MESSAGE_MAP(COleClipSource, COleDataSource)
END_MESSAGE_MAP()

// COleClipSource message handlers

BOOL COleClipSource::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	HGLOBAL hData = m_ClipIDs.Render( lpFormatEtc->cfFormat );
	
	if( !hData )
		return FALSE;
	
	// if phGlobal is null, we can just give the allocated mem
	// else, our data must fit within the GlobalSize(*phGlobal)
	if( *phGlobal == 0 )
		*phGlobal = hData;
	else
	{
		UINT len = min( ::GlobalSize(*phGlobal), ::GlobalSize(hData) );
		if( len )
			CopyToGlobalHH( *phGlobal, hData, len );
		::GlobalFree( hData );
	}
	return TRUE;
}


/*------------------------------------------------------------------*\
CProcessPaste
\*------------------------------------------------------------------*/

CProcessPaste::CProcessPaste()
{
	m_pOle = new COleClipSource;
	m_bSendPaste = true;
	m_bActivateTarget = true;
	m_bOnlyPaste_CF_TEXT = false;
	m_bPasteHTMLFormatAs_CF_TEXT = false;
}

CProcessPaste::~CProcessPaste()
{	
	DELETE_PTR(m_pOle);
}

BOOL CProcessPaste::DoPaste()
{
	m_pOle->m_bOnlyPaste_CF_TEXT = m_bOnlyPaste_CF_TEXT;
	m_pOle->m_bPasteHTMLFormatAs_CF_TEXT = m_bPasteHTMLFormatAs_CF_TEXT;

	if( m_pOle->DoImmediateRender() )
	{
		// MarkAsPasted() must be done first since it makes use of
		//  m_pOle->m_ClipIDs and m_pOle is inaccessible after
		//  SetClipboard is called.
		MarkAsPasted();
		
		// Ignore the clipboard change that we will cause IF:
		// 1) we are pasting a single element, since the element is already
		//    in the db and its lDate was updated by MarkAsPasted().
		// OR
		// 2) we are pasting multiple, but g_Opt.m_bSaveMultiPaste is false
		if( GetClipIDs().GetSize() == 1 || !g_Opt.m_bSaveMultiPaste )
			m_pOle->CacheGlobalData(theApp.m_cfIgnoreClipboard, NewGlobalP("Ignore", sizeof("Ignore")));
		
		m_pOle->SetClipboard(); // m_pOle is now managed by the OLE clipboard
		m_pOle = NULL; // m_pOle should not be accessed past this point

//#ifndef _DEBUG
		if(m_bSendPaste)
			theApp.SendPaste(m_bActivateTarget);
//#endif
		
		return TRUE;
	}
	return FALSE;
}

BOOL CProcessPaste::DoDrag()
{
	m_pOle->DoDelayRender();
	DROPEFFECT de = m_pOle->DoDragDrop( DROPEFFECT_COPY );
	if( de != DROPEFFECT_NONE )
	{
		MarkAsPasted();
		return TRUE;
	}
	return FALSE;
}

void CProcessPaste::MarkAsPasted()
{
	CClipIDs& clips = GetClipIDs();
	if( clips.GetSize() == 1 )
		MarkClipAsPasted( clips.ElementAt(0) );
}

