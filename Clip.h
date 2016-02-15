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
#include "tinyxml\tinyxml.h"
#include "Shared\IClip.h"
#include "Misc.h"

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
	int m_dataId;
	int m_parentId;

	CClipFormat(CLIPFORMAT cfType = 0, HGLOBAL hgData = 0, int parentId = -1);
	~CClipFormat();

	void Clear();
	virtual void Free();

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

	virtual int Size() { return (int)this->GetCount(); }
	virtual IClipFormat *GetAt(int nPos) { return &this->ElementAt(nPos); }
	virtual void DeleteAt(int nPos) { this->RemoveAt(nPos); }
	virtual void DeleteAll() { this->RemoveAll(); }
	virtual INT_PTR AddNew(CLIPFORMAT type, HGLOBAL data) {CClipFormat ft(type, data, -1); ft.m_autoDeleteData = false; return this->Add(ft); }
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
	static int m_lastAddedID;

	int m_id;
	CClipFormats m_Formats;
	CTime m_Time;
	CString m_Desc;
	ULONG m_lTotalCopySize;
	int m_parentId;
	int m_dontAutoDelete;
	int m_shortCut;
	BOOL m_bIsGroup;
	DWORD m_CRC;
	CString m_csQuickPaste;
	int m_param1;
	double m_clipOrder;
	double m_clipGroupOrder;
	double m_stickyClipOrder;
	double m_stickyClipGroupOrder;
	BOOL m_globalShortCut;
	CTime m_lastPasteDate;
	int m_moveToGroupShortCut;
	BOOL m_globalMoveToGroupShortCut;
	CopyReasonEnum::CopyReason m_copyReason;

	virtual CString Description() { return m_Desc; }
	virtual void Description(CString csValue) { m_Desc = csValue; }
	virtual CTime PasteTime() { return m_Time; }
	virtual int ID() { return m_id; }
	virtual int Parent() { return m_parentId; }
	virtual void Parent(int nParent) { m_parentId = nParent; }
	virtual int DontAutoDelete() { return m_dontAutoDelete; }
	virtual void DontAutoDelete(int Dont) { m_dontAutoDelete = Dont; }
	virtual CString QuickPaste() { return m_csQuickPaste; }
	virtual void QuickPaste(CString csValue) { m_csQuickPaste = csValue; }

	virtual IClipFormats *Clips() { return (IClipFormats*)&m_Formats; }

	void Clear();
	void EmptyFormats();
	bool AddFormat(CLIPFORMAT cfType, void* pData, UINT nLen);
	bool LoadFromClipboard(CClipTypes* pClipTypes, bool checkClipboardIgnore = true);
	bool SetDescFromText(HGLOBAL hgData, bool unicode);
	bool SetDescFromType();
	bool AddToDB(bool bCheckForDuplicates = true);
	bool ModifyMainTable();
	bool SaveFromEditWnd(BOOL bUpdateDesc);
	void MakeLatestOrder();
	void MakeLatestGroupOrder();
	void MakeStickyTop(int parentId);
	void MakeStickyLast(int parentId);
	void RemoveStickySetting(int parentId);
	BOOL LoadMainTable(int id);
	DWORD GenerateCRC();
	void MoveUp(int parentId);
	void MoveDown(int parentId);

	CStringW GetUnicodeTextFormat();
	CStringA GetCFTextTextFormat();

	BOOL WriteTextToFile(CString path, BOOL unicode, BOOL asci, BOOL utf8);

	// Allocates a Global containing the requested Clip's Format Data
	static HGLOBAL LoadFormat(int id, UINT cfType);
	// Fills "formats" with the Data of all Formats in the db for the given Clip ID
	bool LoadFormats(int id, bool bOnlyLoad_CF_TEXT = false, bool includeRichTextForTextOnly = false);
	// Fills "types" with all Types in the db for the given Clip ID
	static void LoadTypes(int id, CClipTypes& types);

	static double GetNewOrder(int parentId, int clipId);
	static double GetNewTopSticky(int parentId, int clipId);
	static double GetNewLastSticky(int parentId, int clipId);
	
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
	int AddToDB( bool bLatestOrder = false);

	const CClipList& operator=(const CClipList &cliplist);
};

#endif // !defined(AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
