#pragma once

#include "IClipAggregator.h"
#include "sqlite\CppSQLite3.h"

class CClipIDs : public CArrayEx<int>
{
public:
// PASTING FUNCTIONS

	// allocate an HGLOBAL of the given Format Type representing the Clip IDs in this array.
	HGLOBAL	Render(UINT cfType);
	// Fills "types" with the Format Types corresponding to the Clip IDs in this array.
	void GetTypes(CClipTypes& types);
	bool AggregateData(IClipAggregator &Aggregator, UINT cfType, BOOL bReverse);

// MANAGEMENT FUNCTIONS

	// Blindly Moves IDs into the lParentID Group sequentially with the given order
	// (i.e. this does not check to see if the IDs' order conflict)
	// if( dIncrement < 0 ), this does not change the order
	BOOL MoveTo(long lParentID, double dFirst = 0, double dIncrement = -1);

	// reorders the "lParentID" Group, inserting before the given id.
	//  if the id cannot be found, this appends the IDs.
//	BOOL ReorderGroupInsert( long lParentID, long lInsertBeforeID = 0 );

	// Empties this array and fills it with the elements of the given group ID
	BOOL LoadElementsOf(int groupId);

	BOOL CopyTo(int parentId);

	BOOL DeleteIDs(bool fromClipWindow, CppSQLite3DB& db);

	BOOL Export(CString csFilePath);
	
protected:
	BOOL CreateExportSqliteDB(CppSQLite3DB &db);

protected:
	
};