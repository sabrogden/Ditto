#include "stdafx.h"
#include "CP_Main.h"
#include "ClipIds.h"
#include "tinyxml.h"
#include "shared/TextConvert.h"
#include "Clip_ImportExport.h"
#include "CF_HDropAggregator.h"
#include "CF_UnicodeTextAggregator.h"
#include "CF_TextAggregator.h"
#include "richtextaggregator.h"
#include "htmlformataggregator.h"
#include "Popup.h"

// allocate an HGLOBAL of the given Format Type representing these Clip IDs.
HGLOBAL CClipIDs::Render(UINT cfType)
{
	INT_PTR count = GetSize();
	if(count <= 0)
	{
		return 0;
	}

	if(count == 1)
	{
		return CClip::LoadFormat(ElementAt(0), cfType);
	}

	CStringA SepA = CTextConvert::ConvertToChar(g_Opt.GetMultiPasteSeparator());
	CStringW SepW = CTextConvert::ConvertToUnicode(g_Opt.GetMultiPasteSeparator());

	if(cfType == CF_TEXT)
	{
		CCF_TextAggregator CFText(SepA);
		if(AggregateData(CFText, CF_TEXT, g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop))
		{
			return CFText.GetHGlobal();
		}
	}
	else if(cfType == CF_UNICODETEXT)
	{
		CCF_UnicodeTextAggregator CFUnicodeText(SepW);
		if(AggregateData(CFUnicodeText, CF_UNICODETEXT, g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop))
		{
			return CFUnicodeText.GetHGlobal();
		}
	}
	else if(cfType == CF_HDROP)
	{
		CCF_HDropAggregator HDrop;
		if(AggregateData(HDrop, CF_HDROP, g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop))
		{
			return HDrop.GetHGlobal();
		}
	}
	else if(cfType == theApp.m_HTML_Format)
	{
		CHTMLFormatAggregator Html(SepA);
		if(AggregateData(Html, theApp.m_HTML_Format, g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop))
		{
			return Html.GetHGlobal();
		}
	}
	else if(cfType == theApp.m_RTFFormat)
	{
		CRichTextAggregator RichText(SepA);
		if(AggregateData(RichText, theApp.m_RTFFormat, g_Opt.m_bMultiPasteReverse && g_Opt.m_bHistoryStartTop))
		{
			return RichText.GetHGlobal();
		}
	}
	
	return NULL;
}

void CClipIDs::GetTypes(CClipTypes& types)
{
	INT_PTR count = GetSize();
	types.RemoveAll();

	if(count == 1)
	{
		CClip::LoadTypes(ElementAt(0), types);
	}
	else if(count > 1)
	{
		//Add the types that are common across all paste ids
		long lCount;
		CMap<CLIPFORMAT, CLIPFORMAT, long, long> RenderTypes;

		for(int nIDPos = 0; nIDPos < count; nIDPos++)
		{
			CClipTypes CurrTypes;
			CClip::LoadTypes(ElementAt(nIDPos), CurrTypes);

			INT_PTR typeCount = CurrTypes.GetSize();

			for(int type = 0; type < typeCount; type++)
			{	
				lCount = 0;
				if(nIDPos == 0 || RenderTypes.Lookup(CurrTypes[type], lCount) == TRUE)
				{
					lCount++;
					RenderTypes.SetAt(CurrTypes[type], lCount);
				}
			}
		}

		CLIPFORMAT Format;
		POSITION pos = RenderTypes.GetStartPosition();
		while(pos)
		{
			RenderTypes.GetNextAssoc(pos, Format, lCount);
			if(lCount == count)
			{
				types.Add(Format);
			}			
		}

		//If there were no common types add the first clip
		if(types.GetSize() <= 0)
		{
			CClip::LoadTypes(ElementAt(0), types);
		}
	}
}

bool CClipIDs::AggregateData(IClipAggregator &Aggregator, UINT cfType, BOOL bReverse)
{
	CString csSQL;
	LPWSTR Text = NULL;
	int nTextSize = 0;
	INT_PTR numIDs = GetSize();
	bool bRet = false;

	try
	{
		INT_PTR nIndex;
		for(int i=0; i < numIDs; i++)
		{
			nIndex = i;
			if(bReverse)
			{
				nIndex = numIDs - i - 1;
			}

			csSQL.Format(_T("SELECT * FROM Data ")
				_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
				_T("WHERE Data.strClipBoardFormat = '%s' ")
				_T("AND Main.lID = %d"),
				GetFormatName(cfType),
				this[nIndex]);

			CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

			if(q.eof() == false)
			{
				int nDataLen = 0;
				LPVOID pData = (LPVOID)q.getBlobField(_T("ooData"), nDataLen);
				if(pData == NULL)
				{
					continue;
				}

				if(Aggregator.AddClip(pData, nDataLen, (int)i, (int)numIDs))
				{
					bRet = true;
				}
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
		catch(...)
	{

	}

	return bRet;
}

//----------------------------------------------
// ELEMENT (Clip or Group) MANAGEMENT FUNCTIONS
//----------------------------------------------

// returns the address of the given id in this array or NULL.
int* CClipIDs::FindID(int id)
{
	INT_PTR count = GetSize();
	int* pID = GetData();

	for(int i=0; i < count; i++)
	{
		if(*pID == id)
		{
			return pID;
		}
		pID++;
	}
	return NULL;
}

// Blindly Moves IDs into the lParentID Group sequentially with the given order
BOOL CClipIDs::MoveTo(long lParentID, double dFirst, double dIncrement)
{
	try
	{
		INT_PTR count = GetSize();
		for(int i = 0; i < count; i++)
		{
			CString sql = StrF(_T("UPDATE Main SET lParentID = %d ")
								_T("WHERE lID = %d AND lID <> %d;"), 
								lParentID,
								ElementAt(i),
								lParentID);

			theApp.m_db.execDMLEx(sql);
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return (TRUE);
}

// Empties this array and fills it with the elements of the given group ID
BOOL CClipIDs::LoadElementsOf(int groupId)
{
	SetSize(0);
	
	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Main WHERE lParentID = %d"), groupId);
		while(q.eof() == false)
		{
			Add(q.getIntField(_T("lID")));
			q.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return GetSize() > 0;
}

// Creates copies (duplicates) of all items in this array and assigns the
// lParentID of the copies to the given "lParentID" group.
// - if lParentID <= 0, then the copies have the same parent as the source
// - pCopies is filled with the corresponding duplicate IDs.
// - pAddNewTable and pSeekTable are used for more efficient recursion.
// - the primary overhead for recursion is one ID array per level deep.
//   an alternative design would be to have one CMainTable per level deep,
//   but I thought that might be too costly, so I implemented it this way.

BOOL CClipIDs::CopyTo(int parentId)
{
	INT_PTR count = GetSize();
	if(count == 0)
		return TRUE;
		
	try
	{
		theApp.m_db.execDML(_T("begin transaction;"));

		for(int i = 0; i < count; i++)
		{
			int nID = ElementAt(i);

			CClip clip;

			if(clip.LoadMainTable(nID))
			{
				if(clip.LoadFormats(nID))
				{
					clip.MakeLatestTime();

					clip.m_shortCut = 0;
					clip.m_parentId = parentId;
					clip.m_csQuickPaste = "";

					if(clip.AddToDB(false) == false)
					{
						Log(_T("failed to add copy to database"));
					}
				}
			}
		}

		theApp.m_db.execDML(_T("commit transaction;"));
	}
	CATCH_SQLITE_EXCEPTION
		
	return TRUE;
}

BOOL CClipIDs::DeleteIDs(bool fromClipWindow, CppSQLite3DB& db)
{
	CPopup status(0, 0, ::GetForegroundWindow());
	bool bAllowShow;
	bAllowShow = IsAppWnd(::GetForegroundWindow());
	
	BOOL bRet = TRUE;
	INT_PTR count = GetSize();
	int batchCount = 25;

	Log(StrF(_T("Begin delete clips, Count: %d from Window: %d"), count, fromClipWindow));
	
	if(count <= 0)
		return FALSE;

	try
	{
		CString sql = _T("DELETE FROM Main where lId in(");
		CString sqlIn = _T("");
		CString workingString = _T("Deleting clips, building query statement");
		INT_PTR startIndex = 0;
		INT_PTR index = 0;

		if(bAllowShow)
		{
			status.Show(workingString);
		}

		for(index = 0; index < count; index++)
		{
			int clipId = ElementAt(index);
			if(clipId <= 0)
				continue;

			Log(StrF(_T("Delete clip Id: %d"), clipId));

			bool cont = false;
			bool bGroup = false;
			{
				CppSQLite3Query q = db.execQueryEx(_T("SELECT bIsGroup FROM Main WHERE lId = %d"), clipId);
				cont = !q.eof();
				if(cont)
				{
					bGroup = q.getIntField(_T("bIsGroup")) > 0;
				}
			}

			if(cont)
			{			
				if(bGroup)
				{
					db.execDMLEx(_T("UPDATE Main SET lParentID = -1 WHERE lParentID = %d;"), clipId);
				}

				if(sqlIn.GetLength() > 0)
				{
					sqlIn += ", ";
				}
				sqlIn += StrF(_T("%d"), clipId);
			}

			if(index > 0 && 
				(index % batchCount) == 0)
			{
				if(bAllowShow)
				{
					status.Show(StrF(_T("Deleting %d - %d of %d..."), startIndex+1, index, count));
				}
				startIndex = index;

				db.execDMLEx(sql + sqlIn + _T(")"));
				sqlIn = "";
				bRet = TRUE;

				if(bAllowShow)
				{
					status.Show(workingString);
				}
			}

			if(fromClipWindow == false)
			{
				theApp.OnDeleteID(clipId);
			}
		}

		if(sqlIn.GetLength() > 0)
		{
			if(bAllowShow)
			{
				status.Show(StrF(_T("Deleting %d - %d of %d..."), startIndex+1, index, count));
			}

			db.execDMLEx(sql + sqlIn + _T(")"));
			bRet = TRUE;
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)
	
	Log(StrF(_T("End delete clips, Count: %d"), count));

	return bRet;
}

BOOL CClipIDs::CreateExportSqliteDB(CppSQLite3DB &db)
{
	BOOL bRet = FALSE;
	try
	{
		db.execDML(_T("CREATE TABLE Main(")
			_T("lID INTEGER PRIMARY KEY AUTOINCREMENT, ")
			_T("lVersion INTEGER, ")
			_T("mText TEXT);"));

		db.execDML(_T("CREATE TABLE Data(")
			_T("lID INTEGER PRIMARY KEY AUTOINCREMENT, ")
			_T("lParentID INTEGER, ")
			_T("strClipBoardFormat TEXT, ")
			_T("lOriginalSize INTEGER, ")
			_T("ooData BLOB);"));

		db.execDML(_T("CREATE UNIQUE INDEX Main_ID on Main(lID ASC)"));
		db.execDML(_T("CREATE UNIQUE INDEX Data_ID on Data(lID ASC)"));

		db.execDML(_T("CREATE TRIGGER delete_data_trigger BEFORE DELETE ON Main FOR EACH ROW\n")
			_T("BEGIN\n")
			_T("DELETE FROM Data WHERE lParentID = old.lID;\n")
			_T("END\n"));

		bRet = TRUE;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)

	return bRet;
}

BOOL CClipIDs::Export(CString csFilePath)
{    
	INT_PTR count = GetSize();
	if(count == 0)
		return TRUE;

	BOOL bRet = FALSE;

	if(FileExists(csFilePath) && DeleteFile(csFilePath) == FALSE)
	{
		Log(StrF(_T("Export::Error deleting the file %s"), csFilePath));
		return FALSE;
	}

	try
	{		
		CppSQLite3DB db;
		db.open(csFilePath);

		if(CreateExportSqliteDB(db) == FALSE)
			return FALSE;
	
		for(int i = 0; i < count; i++)
		{
			int nID = ElementAt(i);

			CClip_ImportExport clip;
			
			if(clip.LoadMainTable(nID))
			{
				if(clip.LoadFormats(nID))
				{
					clip.ExportToSqliteDB(db);
					bRet = TRUE;
				}
			}
		}

		db.close();
	}
	CATCH_SQLITE_EXCEPTION

	return bRet;
}