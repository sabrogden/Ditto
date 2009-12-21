#pragma once
#include "EventThread.h"
#include "Clip.h"
#include "AutoSendToClientThread.h"
#include <afxmt.h>

class CMainFrmThread : public CEventThread
{
public:
    CMainFrmThread(void);
    ~CMainFrmThread(void);

    enum eCMainFrmThreadEvents
    {
        DELETE_ENTRIES, 
        REMOVE_REMOTE_FILES, 
		SAVE_CLIPS,
		SAVE_REMOTE_CLIPS,

        ECMAINFRMTHREADEVENTS_COUNT  //must be last
    };

    void FireDeleteEntries() { FireEvent(DELETE_ENTRIES); }
    void FireRemoveRemoteFiles() { FireEvent(REMOVE_REMOTE_FILES); }

	void AddClipToSave(CClip *pClip);
	void AddRemoteClipToSave(CClipList *pClipList);

protected:
    virtual void OnEvent(int eventId, void *param);

    void OnDeleteEntries();
    void OnRemoveRemoteFiles();
	void OnSaveClips();
	void OnSaveRemoteClips();

	CCriticalSection m_cs;
	CClipList m_saveClips;
	CClipList m_saveRemoteClips;
	CAutoSendToClientThread m_sendToClientThread;
};
