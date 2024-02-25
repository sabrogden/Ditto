////////////////////////////////////////////////////////////////////////////////
// CppSQLite3 - A C++ wrapper around the SQLite3 embedded database library.
//
// Copyright (c) 2004 Rob Groves. All Rights Reserved. rob.groves@btinternet.com
// 
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice, 
// this paragraph and the following two paragraphs appear in all copies, 
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// V3.0		03/08/2004	-Initial Version for sqlite3
//
// V3.1		16/09/2004	-Implemented getXXXXField using sqlite3 functions
//						-Added CppSQLiteDB3::tableExists()
////////////////////////////////////////////////////////////////////////////////
#ifndef _CppSQLite3_H_
#define _CppSQLite3_H_

#include "sqlite3.h"
#include <cstdio>
#include <cstring>

#define CPPSQLITE_ERROR 1000

#ifdef _UNICODE
	#define SQLITE3_ERRMSG(mpDB) const TCHAR* szError = (const TCHAR*)sqlite3_errmsg16(mpDB)
#else							
	#define SQLITE3_ERRMSG(mpDB) const TCHAR* szError = sqlite3_errmsg(mpDB)
#endif		

int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out);
int sqlite3_decode_binary(const unsigned char *in, unsigned char *out);

class CppSQLite3Exception
{
public:

    CppSQLite3Exception(const int nErrCode,
                    TCHAR* szErrMess,
                    bool bDeleteMsg=true);

    CppSQLite3Exception(const CppSQLite3Exception&  e);

    virtual ~CppSQLite3Exception();

    const int errorCode() { return mnErrCode; }

    const TCHAR* errorMessage() { return mpszErrMess; }

    static const TCHAR* errorCodeAsString(int nErrCode);

private:

    int mnErrCode;
    TCHAR mpszErrMess[1000];
};

class CppSQLite3Query
{
public:

    CppSQLite3Query();

    CppSQLite3Query(const CppSQLite3Query& rQuery);

    CppSQLite3Query(sqlite3* pDB,
				sqlite3_stmt* pVM,
                bool bEof,
                bool bOwnVM=true);

    CppSQLite3Query& operator=(const CppSQLite3Query& rQuery);

    virtual ~CppSQLite3Query();

    int numFields();

    int fieldIndex(const TCHAR* szField);
    const TCHAR* fieldName(int nCol);

    const TCHAR* fieldDeclType(int nCol);
    int fieldDataType(int nCol);

    const TCHAR* fieldValue(int nField);
    const TCHAR* fieldValue(const TCHAR* szField);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(const TCHAR* szField, int nNullValue=0);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(const TCHAR* szField, double fNullValue=0.0);

    const TCHAR* getStringField(int nField, const TCHAR* szNullValue=_T(""));
    const TCHAR* getStringField(const TCHAR* szField, const TCHAR* szNullValue=_T(""));

    const unsigned char* getBlobField(int nField, int& nLen);
    const unsigned char* getBlobField(const TCHAR* szField, int& nLen);

	int getBlobFieldSize(const TCHAR* szField);
	int getBlobFieldSize(int nField);

    bool fieldIsNull(int nField);
    bool fieldIsNull(const TCHAR* szField);

    bool eof();

    void nextRow();

    void finalize();

private:

    void checkVM();

	sqlite3* mpDB;
    sqlite3_stmt* mpVM;
    bool mbEof;
    int mnCols;
    bool mbOwnVM;
};

class CppSQLite3Statement
{
public:

    CppSQLite3Statement();

    CppSQLite3Statement(const CppSQLite3Statement& rStatement);

    CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pVM);

    virtual ~CppSQLite3Statement();

    CppSQLite3Statement& operator=(const CppSQLite3Statement& rStatement);

    int execDML();

    CppSQLite3Query execQuery();

    void bind(int nParam, const TCHAR* szValue);
    void bind(int nParam, const int nValue);
    void bind(int nParam, const double dwValue);
    void bind(int nParam, const unsigned char* blobValue, int nLen);
    void bindNull(int nParam);

    void reset();

    void finalize();

private:

    void checkDB();
    void checkVM();

    sqlite3* mpDB;
    sqlite3_stmt* mpVM;
};


class CppSQLite3DB
{
public:

    CppSQLite3DB();

    virtual ~CppSQLite3DB();

    void open(const TCHAR* szFile);

    void SetRegexCaseInsensitive(bool insensitive);

    bool close();

	bool tableExists(const TCHAR* szTable);

	int execDMLEx(LPCTSTR szSQL,...);
    int execDML(const TCHAR* szSQL);

	CppSQLite3Query execQueryEx(LPCTSTR szSQL,...);
    CppSQLite3Query execQuery(const TCHAR* szSQL);

	int execScalarEx(LPCTSTR szSQL,...);
    int execScalar(const TCHAR* szSQL);

    CppSQLite3Statement compileStatement(const TCHAR* szSQL);

    sqlite_int64 lastRowId();

    void interrupt() { sqlite3_interrupt(mpDB); }

    void setBusyTimeout(int nMillisecs);

    static const TCHAR* SQLiteVersion() { return _T(SQLITE_VERSION); }

    bool IsDatabaseOpen() { return mpDB != NULL; }

private:

    CppSQLite3DB(const CppSQLite3DB& db);
    CppSQLite3DB& operator=(const CppSQLite3DB& db);

    sqlite3_stmt* compile(const TCHAR* szSQL);

    void checkDB();

    sqlite3* mpDB;
    int mnBusyTimeoutMs;
};

#endif
