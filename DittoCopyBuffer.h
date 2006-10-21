#pragma once

#include "ClipboardSaveRestore.h"

class CClipboardSaveRestoreCopyBuffer : public CClipboardSaveRestore
{
public:
	CClipboardSaveRestoreCopyBuffer()
	{
		m_lRestoreDelay = 0;
	}
	long m_lRestoreDelay;
};

class CDittoCopyBuffer
{
public:
	CDittoCopyBuffer();
	~CDittoCopyBuffer(void);

	bool StartCopy(long lCopyBuffer);
	bool EndCopy(CClipList *pClips);

	bool PastCopyBuffer(long lCopyBuffer);
	static UINT DelayRestoreClipboard(LPVOID pParam);

protected:
	long m_lCurrentDittoBuffer;
	CClipboardSaveRestore m_SavedClipboard;
};
