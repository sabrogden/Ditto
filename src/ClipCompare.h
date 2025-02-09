#pragma once
#include "Clip.h"

class CClipCompare
{
public:
	CClipCompare(void);
	~CClipCompare(void);

	void Compare(int leftId, int rightId);

protected:
	CString SaveToFile(int id, CClip *clip, bool saveW, bool SaveA, bool saveUtf8);
	CString GetComparePath(CString &params);
};

