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
	long			m_ID;
	CClipFormats	m_Formats;

	const CClip& operator=(const CClip &clip);

	// statistics assigned by LoadFromClipboard
	CTime	m_Time;	 // time copied from clipboard
	CString m_Desc;
	ULONG	m_lTotalCopySize;
	long	m_lParent;
	long	m_lDontAutoDelete;
	long	m_lShortCut;
	BOOL	m_bIsGroup;
	DWORD	m_CRC;
	CString m_csQuickPaste;

	static  DWORD m_LastAddedCRC;
	static  long m_LastAddedID;

	CClip();
	~CClip();

	void Clear();
	void EmptyFormats();
	
	bool AddFormat(CLIPFORMAT cfType, void* pData, UINT nLen);
	bool LoadFromClipboard(CClipTypes* pClipTypes);
	
	bool AddToDB(bool bCheckForDuplicates = true);

	void MakeLatestTime();

	BOOL LoadMainTable(long lID);

	DWORD GenerateCRC();

	// Allocates a Global containing the requested Clip's Format Data
	static HGLOBAL LoadFormat(long lID, UINT cfType);
	// Fills "formats" with the Data of all Formats in the db for the given Clip ID
	bool LoadFormats(long lID, bool bOnlyLoad_CF_TEXT = false);
	// Fills "types" with all Types in the db for the given Clip ID
	static void LoadTypes(long lID, CClipTypes& types);

protected:
	bool AddToMainTable();
	bool AddToDataTable();
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
