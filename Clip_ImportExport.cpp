#include "stdafx.h"
#include "CP_Main.h"
#include ".\clip_importexport.h"
#include "shared/TextConvert.h"
#include "sqlite/CppSQLite3.h"
#include "zlib/zlib.h"
#include "Misc.h"

#define CURRENT_EXPORT_VERSION 1

CClip_ImportExport::CClip_ImportExport(void) :
	m_importCount(0)
{

}

CClip_ImportExport::~CClip_ImportExport(void)
{
}

bool CClip_ImportExport::ExportToSqliteDB(CppSQLite3DB &db)
{
	bool bRet = false;
	try
	{
		//Add to Main Table
		m_Desc.Replace(_T("'"), _T("''"));
		db.execDMLEx(_T("insert into Main values(NULL, %d, '%s');"), CURRENT_EXPORT_VERSION, m_Desc);
		long lId = (long)db.lastRowId();

		//Add to Data table
		CClipFormat* pCF;
		CppSQLite3Statement stmt = db.compileStatement(_T("insert into Data values (NULL, ?, ?, ?, ?);"));

		for(INT_PTR i = m_Formats.GetSize()-1; i >= 0 ; i--)
		{
			pCF = & m_Formats.ElementAt(i);

			stmt.bind(1, lId);
			stmt.bind(2, GetFormatName(pCF->m_cfType));
			INT_PTR originalSize = GlobalSize(pCF->m_hgData);
			stmt.bind(3, (int)originalSize);

			const unsigned char *Data = (const unsigned char *)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				//First compress the data
				INT_PTR zippedSize = compressBound((ULONG)originalSize);
				Bytef *pZipped = new Bytef[zippedSize];
				if(pZipped)
				{				
					INT_PTR zipReturn = compress(pZipped, (uLongf *)&zippedSize, (const Bytef *)Data, (ULONG)originalSize);
					if(zipReturn == Z_OK)
					{
						stmt.bind(4, pZipped, (int)zippedSize);
					}

					delete []pZipped;
					pZipped = NULL;
				}
			}
			GlobalUnlock(pCF->m_hgData);

			stmt.execDML();
			stmt.reset();

			m_Formats.RemoveAt(i);
		}

		bRet = true;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)

	return bRet;
}

bool CClip_ImportExport::ImportFromSqliteDB(CppSQLite3DB &db, bool bAddToDB, bool bPutOnClipboard)
{
	bool bRet = false;
	CStringA csCF_TEXT;
	CStringW csCF_UNICODETEXT;

	try
	{
		CppSQLite3Query q = db.execQuery(_T("Select * from Main"));
		while(q.eof() == false)
		{
			Clear();

			int nVersion = q.getIntField(_T("lVersion"));
			if(nVersion == 1)
			{
				if(ImportFromSqliteV1(db, q))
				{
					if(bAddToDB)
					{
						MakeLatestOrder();
						AddToDB(true);
						bRet = true;
					}
					else if(bPutOnClipboard)
					{
						bRet = true;
					}
				}
			}

			m_importCount++;

			//If putting on the clipboard and there are multiple
			//then append cf_text and cf_unicodetext
			if(bPutOnClipboard)
			{
				Append_CF_TEXT_AND_CF_UNICODETEXT(csCF_TEXT, csCF_UNICODETEXT);
			}

			q.nextRow();
		}

		if(bRet && bAddToDB)
		{
			theApp.RefreshView();
		}
		else if(bRet && m_importCount == 1 && bPutOnClipboard)
		{
			PlaceFormatsOnclipboard();
		}
		else if(bRet && bPutOnClipboard)
		{
			PlaceCF_TEXT_AND_CF_UNICODETEXT_OnClipboard(csCF_TEXT, csCF_UNICODETEXT);
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)

	return bRet;
}

bool CClip_ImportExport::PlaceCF_TEXT_AND_CF_UNICODETEXT_OnClipboard(CStringA &csCF_TEXT, CStringW &csCF_UNICODETEXT)
{
	bool bRet = false;

	if(OpenClipboard(theApp.m_MainhWnd))
	{
		EmptyClipboard();

		if(csCF_TEXT.IsEmpty() == FALSE)
		{
			long lLen = csCF_TEXT.GetLength();
			HGLOBAL hGlobal = NewGlobalP(csCF_TEXT.GetBuffer(lLen), lLen+1);
			csCF_TEXT.ReleaseBuffer();
			SetClipboardData(CF_TEXT, hGlobal);

			bRet = true;
		}
		if(csCF_UNICODETEXT.IsEmpty() == FALSE)
		{
			long lLen = csCF_UNICODETEXT.GetLength() * sizeof(wchar_t);
			HGLOBAL hGlobal = NewGlobalP(csCF_UNICODETEXT.GetBuffer(lLen), lLen+1);
			csCF_UNICODETEXT.ReleaseBuffer();
			SetClipboardData(CF_UNICODETEXT, hGlobal);

			bRet = true;
		}

		CloseClipboard();
	}
	else
	{
		Log(_T("Error opening clipboard"));
	}

	return bRet;
}

bool CClip_ImportExport::PlaceFormatsOnclipboard()
{
	bool bRet = false;

	if(OpenClipboard(theApp.m_MainhWnd))
	{
		EmptyClipboard();

		INT_PTR formatCount = m_Formats.GetSize();
		for(int i = 0; i < formatCount; i++)
		{
			CClipFormat *pCF;
			pCF = &m_Formats.ElementAt(i);
			LPVOID Data = (LPVOID)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				HGLOBAL hGlobal = NewGlobalP(Data, GlobalSize(pCF->m_hgData));			
				if(hGlobal)
				{
					SetClipboardData(pCF->m_cfType, hGlobal);
				}

				GlobalUnlock(pCF->m_hgData);
			}
		}

		CloseClipboard();

		bRet = true;
	}
	else
	{
		Log(_T("PlaceFormatsOnclipboard::Error opening clipboard"));
	}

	return bRet;
}

bool CClip_ImportExport::ImportFromSqliteV1(CppSQLite3DB &db, CppSQLite3Query &qMain)
{
	try
	{
		//Load the Main Table
		m_Desc = qMain.getStringField(_T("mText"));
		long lID = qMain.getIntField(_T("lID"));

		//Load the data Table
		CClipFormat cf;
		HGLOBAL hGlobal = 0;
		m_Formats.RemoveAll();

		CString csSQL;
		csSQL.Format(
			_T("SELECT Data.* FROM Data ")
			_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
			_T("WHERE Main.lID = %d ORDER BY Data.lID desc"), lID);

		CppSQLite3Query qData = db.execQuery(csSQL);
		while(qData.eof() == false)
		{
			cf.m_cfType = GetFormatID(qData.getStringField(_T("strClipBoardFormat")));
			long lOriginalSize = qData.getIntField(_T("lOriginalSize"));

			int nDataLen = 0;
			const unsigned char *cData = qData.getBlobField(_T("ooData"), nDataLen);
			if(cData != NULL)
			{
				Bytef *pUnZippedData = new Bytef[lOriginalSize];
				if(pUnZippedData)
				{
					//the data in the exported file is compressed so uncompress it now
					int nRet = uncompress(pUnZippedData, (uLongf *)&lOriginalSize, (Bytef *)cData, nDataLen);
					if(nRet == Z_OK)
					{
						cf.m_hgData = NewGlobalP(pUnZippedData, lOriginalSize);
						if(cf.m_hgData)
						{
							m_Formats.Add(cf);
							cf.m_hgData = NULL; //m_format owns m_hgData now
						}
						else
						{
							Log(StrF(_T("Error allocating NewGlobalP size = %d"), lOriginalSize));
							ASSERT(FALSE);
						}
					}
					else
					{
						Log(_T("Error uncompressing data from zlib"));
						ASSERT(FALSE);
					}

					delete []pUnZippedData;
					pUnZippedData = NULL;
				}
				else
				{
					Log(StrF(_T("Error allocating memory to unzip size = %d"), lOriginalSize));
					ASSERT(FALSE);
				}
			}

			qData.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)

	return m_Formats.GetSize() > 0;
}

bool CClip_ImportExport::Append_CF_TEXT_AND_CF_UNICODETEXT(CStringA &csCF_TEXT, CStringW &csCF_UNICODETEXT)
{
	bool bRet = false;
	CClipFormat *pCF;
	INT_PTR count = m_Formats.GetSize();
	for(int i = 0; i < count; i++)
	{
		pCF = &m_Formats.ElementAt(i);

		switch(pCF->m_cfType)
		{
		case CF_TEXT:
			{	
				const char *Data = (const char *)GlobalLock(pCF->m_hgData);
				if(Data)
				{
					if(csCF_TEXT.IsEmpty() == FALSE)
						csCF_TEXT += "\r\n";

					csCF_TEXT += Data;
					bRet = true;

					GlobalUnlock(pCF->m_hgData);
				}
			}
			break;
		case CF_UNICODETEXT:
			{	
				const wchar_t *Data = (const wchar_t *)GlobalLock(pCF->m_hgData);
				if(Data)
				{
					if(csCF_UNICODETEXT.IsEmpty() == FALSE)
						csCF_UNICODETEXT += _T("\r\n");

					csCF_UNICODETEXT += Data;
					bRet = true;

					GlobalUnlock(pCF->m_hgData);
				}
			}
			break;
		}
	}

	return bRet;
}