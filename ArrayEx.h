#if !defined(AFX_CARRAYEX_H__BE2C5983_CE26_11D3_BAE6_0000C0D475E2__INCLUDED_)
#define AFX_CARRAYEX_H__BE2C5983_CE26_11D3_BAE6_0000C0D475E2__INCLUDED_

//#if _MSC_VER > 1000
#pragma once
//#endif // _MSC_VER > 1000

#include <afxtempl.h>

template <class TYPE> class CArrayEx : public CArray<TYPE,TYPE>
{
public:
	inline const TYPE&	operator[](int nIndex) const
	{
		ASSERT(0 <= nIndex && nIndex < GetSize());
		return (GetData()[nIndex]);
	};

	inline const TYPE&	GetAt(int nIndex) const
	{
		ASSERT(0 <= nIndex && nIndex < GetSize());
		return (GetData()[nIndex]);
	};

	inline TYPE& operator[](int nIndex)
	{
		ASSERT(0 <= nIndex && nIndex < GetSize());
		return (GetData()[nIndex]);
	};

	void SortAscending()
	{
		qsort(GetData(), GetSize(), sizeof(TYPE), CArrayEx::CompareAscending);
	}

	void SortDescending()
	{
		qsort(GetData(), GetSize(), sizeof(TYPE), CArrayEx::CompareDescending);
	}
	
	static int CompareAscending(const void * p1, const void * p2)
	{	
		return *(TYPE *)p1 - *(TYPE *)p2;
	}

	static int CompareDescending(const void * p1, const void * p2)
	{	
		return *(TYPE *)p2 - *(TYPE *)p1;
	}
	
	BOOL Find(TYPE type)
	{
		LPVOID lpVoid = NULL;
		lpVoid = bsearch(&type, GetData(), GetSize(), sizeof(TYPE), CArrayEx::CompareAscending);

		if(lpVoid)
			return TRUE;
		
		return FALSE;
	}
};

typedef CArrayEx<int>				ARRAY;

#endif // !defined(AFX_OCARRAY_H__BE2C5983_CE26_11D3_BAE6_0000C0D475E2__INCLUDED_)
