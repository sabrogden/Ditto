// ProcessCopy.h: classes for saving the clipboard to db
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
#define AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxole.h>
#include <afxtempl.h>
#include "MainTable.h"

class CClip;
class CCopyThread;

typedef CArray<CLIPFORMAT, CLIPFORMAT> CClipTypes;

/*----------------------------------------------------------------------------*\
	COleDataObjectEx
\*----------------------------------------------------------------------------*/
class COleDataObjectEx : public COleDataObject
{
public:
	// creates global from IStream if necessary
	HGLOBAL GetGlobalData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);
};

/*----------------------------------------------------------------------------*\
	CClipFormat - holds the data of one clip format.
\*----------------------------------------------------------------------------*/
class CClipFormat
{
public:
	CLIPFORMAT	m_cfType;
    HGLOBAL		m_hgData;
	bool		bDeleteData;

	CClipFormat(CLIPFORMAT cfType = 0, HGLOBAL hgData = 0);
	~CClipFormat();

	void Clear();
	void Free();
};

/*----------------------------------------------------------------------------*\
	CClipFormats - holds an array of CClipFormat
\*----------------------------------------------------------------------------*/
class CClipFormats : public CArray<CClipFormat,CClipFormat&>
{
public:
	// returns a pointer to the CClipFormat in this array which matches the given type
	//  or NULL if that type doesn't exist in this array.
	CClipFormat* FindFormat(UINT cfType); 
};


/*----------------------------------------------------------------------------*\
	CClip - holds multiple CClipFormats and clip statistics
	- provides static functions for manipulating a Clip as a single unit.
\*----------------------------------------------------------------------------*/
class CClip
{
public:
	long			m_ID; // 0 if it hasn't yet been saved or is unknown
	long			m_DataID;
	CClipFormats	m_Formats; // actual format data

	const CClip& operator=(const CClip &clip);

	// statistics assigned by LoadFromClipboard
	CTime	m_Time;	 // time copied from clipboard
	CString m_Desc;
	ULONG	m_lTotalCopySize;

	CClip();
	~CClip();

	void Clear();
	void EmptyFormats();
	
	// Adds a new Format to this Clip by copying the given data.
	bool AddFormat(CLIPFORMAT cfType, void* pData, UINT nLen);
	// Fills this CClip with the contents of the clipboard.
	bool LoadFromClipboard(CClipTypes* pClipTypes);
	bool SetDescFromText(HGLOBAL hgData);
	bool SetDescFromType();

	// Immediately save this clip to the db (empties m_Formats due to AddToDataTable).
	bool AddToDB(bool bCheckForDuplicates = true);
	bool AddToMainTable();
	bool AddToDataTable();

	// if a duplicate exists, set recset to the duplicate and return true
	bool FindDuplicate(CMainTable& recset, BOOL bCheckLastOnly = FALSE);
	int  CompareFormatDataTo(long lDataID);

	// changes m_Time to be later than the latest clip entry in the db
	void MakeLatestTime();

//	void PrependDateToFormat(CClipFormat &cf);

// STATICS
	// Allocates a Global containing the requested Clip's Format Data
	static HGLOBAL LoadFormat(long lID, UINT cfType);
	// Fills "formats" with the Data of all Formats in the db for the given Clip ID
	static bool LoadFormats(long lID, CClipFormats& formats, bool bOnlyLoad_CF_TEXT = false);
	// Fills "types" with all Types in the db for the given Clip ID
	static void LoadTypes(long lID, CClipTypes& types);
};


/*----------------------------------------------------------------------------*\
	CClipList
\*----------------------------------------------------------------------------*/

class CClipList : public CList<CClip*,CClip*>
{
public:
	~CClipList();
	// returns the number of clips actually saved
	// while this does empty the Format Data, it does not delete the Clips.
	int AddToDB( bool bLatestTime = false, bool bShowStatus = true );

	const CClipList& operator=(const CClipList &cliplist);
};

#endif // !defined(AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
