#pragma once
#include "EventThread.h"
#include "Path.h"

class CClipEditThread : public CEventThread
{
public:
	CClipEditThread();
	virtual ~CClipEditThread();
		
	void Close();
	void StartWatchingFolderForChanges();

	void WatchFile(CString filePath);

private:
	void OnEvent(int eventId, void* param) override;
	void OnFileChanged();
	void OnTimeOut(void* param) override;
	bool ReadFile(CString filePath, bool& unicode, CString& unicodeText, CStringA& utf8Text);
	BOOL GetTextFromRTF(CStringA rtf, CString& unicodeText);
	void RefreshWatch();
	bool SaveToClip(CString filePath, int id);
		
	HANDLE m_folderHandle;
	FILE_NOTIFY_INFORMATION m_fileChangeBuffer[10000];
	OVERLAPPED m_overlapped;
	std::map<CString, bool> m_filesToSave;
	std::map<CString, int> m_newClipIds;

	CCriticalSection m_fileEditsLock;
	std::map<CString, CTime> m_fileEditStarts;
};

