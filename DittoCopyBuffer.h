#pragma once

#include "ClipboardSaveRestore.h"
#include "afxmt.h"

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

	bool Active()	{ return m_bActive; }
	bool StartCopy(long lCopyBuffer, bool bCut = false);
	bool EndCopy(long lID);
	bool PastCopyBuffer(long lCopyBuffer);

	static bool PutClipOnDittoCopyBuffer(long lClipId, long lBuffer);
	static UINT DelayRestoreClipboard(LPVOID pParam);
	static UINT StartCopyTimer(LPVOID pParam);

protected:
	void EndRestoreThread();

protected:
	long m_lCurrentDittoBuffer;
	CClipboardSaveRestore m_SavedClipboard;
	bool m_bActive;
	DWORD m_dwLastPaste;
	CEvent m_ActiveTimer;
	CEvent m_RestoreTimer;
	CEvent m_RestoreActive;
	CClipboardSaveRestoreCopyBuffer *m_pClipboard;
};
