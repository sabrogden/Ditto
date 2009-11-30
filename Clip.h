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
#include "tinyxml.h"
#include "Shared\IClip.h"

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
class CClipFormat : public IClipFormat
{
public:
	CLIPFORMAT m_cfType;
    HGLOBAL m_hgData;
	bool m_autoDeleteData;
	long m_lDBID;

	CClipFormat(CLIPFORMAT cfType = 0, HGLOBAL hgData = 0, long lDBID = -1);
	~CClipFormat();

	void Clear();
	void Free();

	virtual CLIPFORMAT Type() { return m_cfType; }
	virtual HGLOBAL Data() { return m_hgData; }
	virtual void Type(CLIPFORMAT type) { m_cfType = type; }
	virtual void Data(HGLOBAL data) { m_hgData = data; }
	virtual void AutoDeleteData(bool autoDeleteData) { m_autoDeleteData = autoDeleteData; }
	virtual bool AutoDeleteData()	{ return m_autoDeleteData; }
};

/*----------------------------------------------------------------------------*\
	CClipFormats - holds an array of CClipFormat
\*----------------------------------------------------------------------------*/
class CClipFormats : public CArray<CClipFormat,CClipFormat&>, public IClipFormats
{
public:
	// returns a pointer to the CClipFormat in this array which matches the given type
	//  or NULL if that type doesn't exist in this array.
	CClipFormat* FindFormat(UINT cfType); 

	virtual int Size() { return this->GetCount(); }
	virtual IClipFormat *GetAt(int nPos) { return &this->ElementAt(nPos); }
	virtual void DeleteAt(int nPos) { this->RemoveAt(nPos); }
	virtual void DeleteAll() { this->RemoveAll(); }
	virtual int AddNew(CLIPFORMAT type, HGLOBAL data) {CClipFormat ft(type, data, -1); ft.m_autoDeleteData = false; return this->Add(ft); }
	virtual IClipFormat *FindFormatEx(CLIPFORMAT type)	{ return FindFormat((UINT)type); }
};


/*----------------------------------------------------------------------------*\
	CClip - holds multiple CClipFormats and clip statistics
	- provides static functions for manipulating a Clip as a single unit.
\*----------------------------------------------------------------------------*/
class CClip : public IClip
{
public:
	CClip();
	~CClip();
	const CClip& operator=(const CClip &clip);

	static DWORD m_LastAddedCRC;
	static long m_LastAddedID;

	long m_ID;
	CClipFormats m_Formats;
	CTime m_Time;
	CString m_Desc;
	ULONG m_lTotalCopySize;
	long m_lParent;
	long m_lDontAutoDelete;
	long m_lShortCut;
	BOOL m_bIsGroup;
	DWORD m_CRC;
	CString m_csQuickPaste;

	virtual CString Description() { return m_Desc; }
	virtual void Description(CString csValue) { m_Desc = csValue; }
	virtual CTime PasteTime() { return m_Time; }
	virtual int ID() { return m_ID; }
	virtual int Parent() { return m_lParent; }
	virtual void Parent(int nParent) { m_lParent = nParent; }
	virtual int DontAutoDelete() { return m_lDontAutoDelete; }
	virtual void DontAutoDelete(int Dont) { m_lDontAutoDelete = Dont; }
	virtual CString QuickPaste() { return m_csQuickPaste; }
	virtual void QuickPaste(CString csValue) { m_csQuickPaste = csValue; }

	virtual IClipFormats *Clips() { return (IClipFormats*)&m_Formats; }

	void Clear();
	void EmptyFormats();
	bool AddFormat(CLIPFORMAT cfType, void* pData, UINT nLen);
	bool LoadFromClipboard(CClipTypes* pClipTypes);
	bool SetDescFromText(HGLOBAL hgData);
	bool SetDescFromType();
	bool AddToDB(bool bCheckForDuplicates = true);
	bool ModifyMainTable();
	bool SaveFromEditWnd(BOOL bUpdateDesc);
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
	int FindDuplicate();
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
