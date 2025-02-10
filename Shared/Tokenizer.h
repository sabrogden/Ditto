#pragma once
#include "ArrayEx.h"

class CTokenizer
{
public:
	CString m_cs;
	CArrayEx < TCHAR > m_delim;
	int m_nCurPos;

	CTokenizer(const CString& cs, const CString& csDelim) : m_cs(cs), m_nCurPos(0)
	{
		SetDelimiters(csDelim);
	}

	void SetDelimiters(const CString& csDelim)
	{
		for (int i = 0; i < csDelim.GetLength(); ++i)
		{
			m_delim.Add(csDelim[i]);
		}

		m_delim.SortAscending();
	}

	bool Next(CString& cs)
	{
		cs.Empty();
		int len = m_cs.GetLength();

		while (m_nCurPos < len && m_delim.Find(m_cs[m_nCurPos]))
		{
			++m_nCurPos;
		}

		if (m_nCurPos >= len)
		{
			return false;
		}

		int nStartPos = m_nCurPos;

		while (m_nCurPos < len && !m_delim.Find(m_cs[m_nCurPos]))
		{
			++m_nCurPos;
		}

		cs = m_cs.Mid(nStartPos, m_nCurPos - nStartPos);

		return true;
	}

	CString Tail()
	{
		int len = m_cs.GetLength();
		int nCurPos = m_nCurPos;

		while (nCurPos < len && m_delim.Find(m_cs[nCurPos]))
		{
			++nCurPos;
		}

		CString csResult;
		if (nCurPos < len)
		{
			csResult = m_cs.Mid(nCurPos);
		}

		return csResult;
	}
};
