#if !defined(AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_)
#define AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ArrayEx.h"
#include "Misc.h"

class CMainTable : public CDaoRecordset
{
public:
	CMainTable(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CMainTable)


// Field/Param Data
	//{{AFX_FIELD(CMainTable, CDaoRecordset)
	long	m_lID;
	long	m_lDate;
	CString	m_strText;
	long	m_lShortCut;
	long	m_lDontAutoDelete;
	long	m_lTotalCopySize;

	BOOL	m_bIsGroup;
	long	m_lParentID;
	double	m_dOrder;
	long	m_lDataID;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainTable)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	virtual void Open(int nOpenType = AFX_DAO_USE_DEFAULT_TYPE, LPCTSTR lpszSql = NULL, int nOptions = 0);
	virtual void Requery();
	//}}AFX_VIRTUAL

public:
	CString GetDisplayText();
	void AddNew(); // assigns the new autoincr ID to m_lID
	void OnQuery();

	int		m_nFieldCount;
	bool	m_bBindFields;
	bool SetBindFields(bool bVal);

//	long GetID();

//	int		m_nCurPos;
//	long	m_lCurID; // used to validate m_nCurPos

	// copies the current source record to this current record
	void CopyRec( CMainTable& source );
	// makes a new copy of the current record and moves to the copy record
	void NewCopyRec();

	// ONLY deletes from Main
	static BOOL DeleteAll();
	
	static void LoadAcceleratorKeys( CAccels& accels );
	void Open(LPCTSTR lpszFormat,...);

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINTABLE_H__F5CFB0A6_A6E1_4C55_A685_AB5F1A0FCF53__INCLUDED_)
