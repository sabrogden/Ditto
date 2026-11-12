#pragma once
#include "EventThread.h"

// Runs OCR over image clips in the background so screenshots can be
// found by searching for the text inside them. Only runs when the
// ocr option is on, one clip at a time, lowest thread priority.
class COcrThread : public CEventThread
{
public:
	COcrThread();
	~COcrThread();

	void StartOcr();
	void StopOcr();

protected:
	virtual void OnTimeOut(void* param);

	int GetNextClipId();
	bool ProcessClip(int clipId);
	void SaveOcrText(int clipId, CString ocrText);

	bool m_engineUnavailable;
	// ticks to skip before looking for work again, set when the last
	// look found nothing, so an idle database is not scanned every tick
	int m_idleTicks;
};
