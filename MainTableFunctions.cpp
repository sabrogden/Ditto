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
		{
			CppSQLite3Query q = db.execQuery(_T("SELECT lID, lShortCut FROM Main WHERE lShortCut > 0"));
		
			CAccel a;
			while(q.eof() == false)
			{
				a.Cmd = q.getIntField(_T("lID"));
				a.Key = q.getIntField(_T("lShortCut"));
				a.RefId = CHotKey::PASTE_OPEN_CLIP;
			
				accels.AddAccel(a);

				q.nextRow();
			}
		}

		{
			CppSQLite3Query q2 = db.execQuery(_T("SELECT lID, MoveToGroupShortCut FROM Main WHERE MoveToGroupShortCut > 0"));

			CAccel a2;
			while(q2.eof() == false)
			{
				a2.Cmd = q2.getIntField(_T("lID"));
				a2.Key = q2.getIntField(_T("MoveToGroupShortCut"));
				a2.RefId = CHotKey::MOVE_TO_GROUP;

				accels.AddAccel(a2);

				q2.nextRow();
			}
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
	CTokenizer tokenizer(text, "\r\n");
	for(int nLines=0; nLines < 100 && tokenizer.Next(token); nLines++)
	{
		tokens.Add(token);
	}

	// remove each line's indent
	TCHAR chFirst;
	CString line;
	INT_PTR count = tokens.GetSize();
	text = _T("");
	for(int i=0; i < count; i++)
	{
		line = tokens.ElementAt(i);
		chFirst = line.GetAt(0);
		if(chFirst == ' ' || chFirst == '\t')
		{
			text += _T("» "); // show indication that the line is modified
			line.TrimLeft();
			text += line;
		}
		else
		{
			text += line;
		}

		text += _T("\n");
	}

	return text;
}