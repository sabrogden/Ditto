// ProcessCopy.h: interface for the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
#define AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxole.h>
#include <afxtempl.h>

class COleDataObjectEx : public COleDataObject
{
public:
	HGLOBAL GetGlobalData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);
};


#define LENGTH_OF_TEXT_SNIPET	249

class CProcessCopy
{
public:
	CProcessCopy();
	virtual ~CProcessCopy();

	BOOL DoCopy();
	static CString GetDBString(CLIPFORMAT cbType);
	static CString GetDescriptionString(CLIPFORMAT cbType);

protected:
	//Functions
	BOOL IsSupportedType(CLIPFORMAT cbType);
	long StartCopyProcess();
	BOOL GetDescriptionText(CString &csText);
	BOOL LoadSupportedTypes();
	BOOL GetCopyInfo(CLIPFORMAT &MainType);
	BOOL DoesCopyEntryExist();
	BOOL IsExactMatch(long lParentID);
	void ShowCopyProperties(long lRecID);

	//Variables
	COleDataObjectEx m_oleData;
	CArray<CLIPFORMAT, CLIPFORMAT> m_SupportedTypes;
	ULONG	m_lTotalCopySize;
	long	m_lSupportedTypesAvailable;
	CRITICAL_SECTION m_CriticalSection;
};

#endif // !defined(AFX_PROCESSCOPY_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
