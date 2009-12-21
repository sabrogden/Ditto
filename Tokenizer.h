#pragma once
#include "ArrayEx.h"

class CTokenizer
{
public:
    CString m_cs;
    CArrayEx < TCHAR > m_delim;
    int m_nCurPos;

    CTokenizer(const CString &cs, const CString &csDelim);
    void SetDelimiters(const CString &csDelim);

    bool Next(CString &cs);
    CString Tail();
};
