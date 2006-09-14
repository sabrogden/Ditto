// AccessToSqlite.h : main header file for the AccessToSqlite DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define _CRT_NON_CONFORMING_SWPRINTFS 1

#include "..\sqlite\CppSQLite3.h"


#pragma warning(disable : 4995)

extern "C" __declspec(dllexport) BOOL ConvertDatabase(const TCHAR *pNewDatabase, const TCHAR *pOldDatabase);


extern class CAccessToSqliteApp theApp;

class CAccessToSqliteApp : public CWinApp
{
public:
	CAccessToSqliteApp();

// Overrides
public:
	virtual BOOL InitInstance();

	CppSQLite3DB m_db;
	CDaoDatabase m_AccessDatabase;

	DECLARE_MESSAGE_MAP()
public:
	virtual int ExitInstance();
};


void CopyToGlobalHP(HGLOBAL hDest, LPVOID pBuf, ULONG ulBufLen);
void CopyToGlobalHH(HGLOBAL hDest, HGLOBAL hSource, ULONG ulBufLen);
HGLOBAL NewGlobalP(LPVOID pBuf, UINT nLen);
HGLOBAL NewGlobalH(HGLOBAL hSource, UINT nLen);
HGLOBAL NewGlobal(UINT nLen);

CString GetFormatName(CLIPFORMAT cbType);
CLIPFORMAT GetFormatID(LPCTSTR cbName);

CString StrF(const TCHAR * pszFormat, ...);

#define Log(msg) log(msg, __FILE__, __LINE__)
void log(const TCHAR* msg, CString csFile = _T(""), long lLine = -1);

