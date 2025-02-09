// MainTableFunctions.h: interface for the CMainTableFunctions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINTABLEFUNCTIONS_H__3AE1D19B_D68D_48E7_80CA_AB62B0447883__INCLUDED_)
#define AFX_MAINTABLEFUNCTIONS_H__3AE1D19B_D68D_48E7_80CA_AB62B0447883__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMainTableFunctions  
{
public:
	CMainTableFunctions();
	virtual ~CMainTableFunctions();

	static void LoadAcceleratorKeys(CAccels& accels, CppSQLite3DB &db);
	static CString GetDisplayText(int nMaxLines, const CString &OrigText);
};

#endif // !defined(AFX_MAINTABLEFUNCTIONS_H__3AE1D19B_D68D_48E7_80CA_AB62B0447883__INCLUDED_)
