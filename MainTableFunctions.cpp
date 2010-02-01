// MainTableFunctions.cpp: implementation of the CMainTableFunctions class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cp_main.h"
#include "MainTableFunctions.h"
#include "shared/Tokenizer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainTableFunctions::CMainTableFunctions()
{

}

CMainTableFunctions::~CMainTableFunctions()
{

}

void CMainTableFunctions::LoadAcceleratorKeys(CAccels& accels, CppSQLite3DB &db)
{
	try
	{
		CppSQLite3Query q = db.execQuery(_T("SELECT lID, lShortCut FROM Main WHERE lShortCut > 0"));
		
		CAccel a;
		while(q.eof() == false)
		{
			a.Cmd = q.getIntField(_T("lID"));
			a.Key = q.getIntField(_T("lShortCut"));
			
			accels.AddAccel(a);

			q.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION
}

CString CMainTableFunctions::GetDisplayText(int nMaxLines, const CString &OrigText)
{
	CString text = OrigText;
	// assign tabs to 2 spaces (rather than the default 8)
	text.Replace(_T("\t"), _T("  "));

	if(g_Opt.m_bDescShowLeadingWhiteSpace)
		return text;
	// else, remove the leading indent from every line.

	// get the lines
	CString token;
	CStringArray tokens;
	CTokenizer tokenizer(text,"\r\n");
	for(int nLines=0; nLines < nMaxLines && tokenizer.Next(token); nLines++)
	{
		tokens.Add(token);
	}

	// remove each line's indent
	TCHAR chFirst;
	CString line;
	int count = tokens.GetSize();
	text = "";
	for(int i=0; i < count; i++)
	{
		line = tokens.ElementAt(i);
		chFirst = line.GetAt(0);
		if(chFirst == ' ' || chFirst == '\t')
		{
			text += "» "; // show indication that the line is modified
			line.TrimLeft();
			text += line;
		}
		else
			text += line;

		text += "\n";
	}

	return text;
}