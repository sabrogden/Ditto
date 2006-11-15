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
	~CDittoCopyBuffer(void);

	static CDittoCopyBuffer *GetDittoCopyBuffer()	{ return &m_Singleton; }

	bool Active()	{ return m_bActive; }
	bool StartCopy(long lCopyBuffer, bool bCut = false);
	bool EndCopy(CClipList *pClips);
	bool PastCopyBuffer(long lCopyBuffer);

	static UINT DelayRestoreClipboard(LPVOID pParam);
	static UINT StartCopyTimer(LPVOID pParam);

protected:
	void EndRestoreThread();

protected:
	CDittoCopyBuffer();
	static CDittoCopyBuffer m_Singleton;

protected:
	long m_lCurrentDittoBuffer;
	CClipboardSaveRestore m_SavedClipboard;
	bool m_bActive;
	DWORD m_dwLastPaste;
	static CEvent m_ActiveTimer;
	static CEvent m_RestoreTimer;
	static CEvent m_RestoreActive;
};
