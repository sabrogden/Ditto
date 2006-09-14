#pragma once
#include "clip.h"

class CClip_ImportExport :	public CClip
{
public:
	CClip_ImportExport(void);
	~CClip_ImportExport(void);

	bool ExportToSqliteDB(CppSQLite3DB &m_db);
	bool ImportFromSqliteDB(CppSQLite3DB &db, bool bAddToDB, bool bPutOnClipboard);
	
	long m_lImportCount;

protected:
	bool ImportFromSqliteV1(CppSQLite3DB &db, CppSQLite3Query &qMain);
	bool Append_CF_TEXT_AND_CF_UNICODETEXT(CStringA &csCF_TEXT, CStringW &csCF_UNICODETEXT);

	bool PlaceFormatsOnclipboard();
	bool PlaceCF_TEXT_AND_CF_UNICODETEXT_OnClipboard(CStringA &csCF_TEXT, CStringW &csCF_UNICODETEXT);
};
