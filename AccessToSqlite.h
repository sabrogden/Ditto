#pragma once

typedef BOOL (*PLUGIN_CONVERT_DATABASE)(const char *, const char *);

class CAccessToSqlite
{
public:
	CAccessToSqlite(void);

	BOOL ConvertDatabase(const TCHAR *pNewDatabase, const TCHAR *pOldDatabase);
public:
	~CAccessToSqlite(void);
};
