#include "stdafx.h"
#include "MainFrmThread.h"
#include "DatabaseUtilities.h"
#include "Options.h"
#include "Misc.h"
#include "cp_main.h"

CMainFrmThread::CMainFrmThread(void)
{
	m_threadName = "CMainFrmThread";
    for(int eventEnum = 0; eventEnum < ECMAINFRMTHREADEVENTS_COUNT; eventEnum++)
    {
        AddEvent(eventEnum);
    }
}

CMainFrmThread::~CMainFrmThread(void)
{

}

void CMainFrmThread::AddClipToSave(CClip *pClip)
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	Log(_T("Adding clip to thread for save to db"));
	m_saveClips.AddTail(pClip);
	FireEvent(SAVE_CLIPS);
}

void CMainFrmThread::AddRemoteClipToSave(CClipList *pClipList)
{
	ATL::CCritSecLock csLock(m_cs.m_sect);

	Log(_T("Adding REMOTE clip to thread for save to db"));
	
	POSITION pos = pClipList->GetHeadPosition();
	while(pos)
	{
		CClip *pClip = pClipList->GetNext(pos);
		m_saveRemoteClips.AddTail(pClip);
	}

	//local cliplist now owns the clip memory
	pClipList->RemoveAll();
	
	FireEvent(SAVE_REMOTE_CLIPS);
}

void CMainFrmThread::OnEvent(int eventId, void *param)
{
    switch((eCMainFrmThreadEvents)eventId)
    {
        case DELETE_ENTRIES:
            OnDeleteEntries();
            break;
        case REMOVE_REMOTE_FILES:
            OnRemoveRemoteFiles();
            break;
		case SAVE_CLIPS:
			OnSaveClips();
			break;
		case SAVE_REMOTE_CLIPS:
			OnSaveRemoteClips();
			break;
		case READ_DB_FILE:
			OnReadDbFile();
			break;
    }
}

//try and keep our db file in windows cache by randomly reading some data
//not sure if this does what i think it does but looking into issues with slow access on large dbs
void CMainFrmThread::OnReadDbFile()
{
	double idle = IdleSeconds();

	if (idle < CGetSetOptions::ReadRandomFileIdleMin())
	{
		CString dbFile = CGetSetOptions::GetDBPath();
		__int64 dbSize = FileSize(dbFile);

		srand(time(NULL));

		int random = rand() % (dbSize - 1024) + 1;

		CFile f;
		if (f.Open(dbFile, CFile::modeRead | CFile::shareDenyNone))
		{
			f.Seek(random, 0);
			char data[1024];
			f.Read(&data, 1024);

			f.Close();
		}
	}
}

void CMainFrmThread::OnDeleteEntries()
{
    RemoveOldEntries(true);
}

void CMainFrmThread::OnRemoveRemoteFiles()
{
    CString csDir = CGetSetOptions::GetPath(PATH_REMOTE_FILES);
    if(FileExists(csDir))
    {
        DeleteReceivedFiles(csDir);
    }
}

void CMainFrmThread::OnSaveClips()
{
	CClipList *pLocalClips = new CClipList();

	//Save the clips locally
	{
		ATL::CCritSecLock csLock(m_cs.m_sect);

		POSITION pos;
		CClip* pClip;

		pos = m_saveClips.GetHeadPosition();
		while(pos)
		{
			pClip = m_saveClips.GetNext(pos);
			pLocalClips->AddTail(pClip);
		}

		//pLocalClips now own, the clips
		m_saveClips.RemoveAll();
	}

	Log(_T("SaveCopyClips Before AddToDb")); 

	int count = pLocalClips->AddToDB(true);

	Log(StrF(_T("SaveCopyclips After AddToDb, Count: %d"), count));

	if(count > 0)
	{
		int Id = pLocalClips->GetTail()->m_id;

		Log(StrF(_T("SaveCopyclips After AddToDb, Id: %d Before OnCopyCopyCompleted"), Id));

		theApp.OnCopyCompleted(Id, count);

		Log(StrF(_T("SaveCopyclips After AddToDb, Id: %d After OnCopyCopyCompleted"), Id));

		if (pLocalClips->GetTail()->m_copyReason == CopyReasonEnum::COPY_TO_GROUP &&
			CGetSetOptions::GetShowMsgWndOnCopyToGroup())
		{
			CString groupName;
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT mText FROM Main WHERE lID = %d"), pLocalClips->GetTail()->m_parentId);
			if (q.eof() == false)
			{
				groupName = q.getStringField(0);
			}

			CString *pMsg = new CString();
			pMsg->Format(_T("Saved new clip \"%s\"\r\ndirectly to the group \"%s\""), pLocalClips->GetTail()->m_Desc.Left(35), groupName);

			theApp.m_pMainFrame->PostMessageW(WM_SHOW_MSG_WINDOW, (WPARAM) pMsg, pLocalClips->GetTail()->m_parentId);
		}
	
		if(g_Opt.m_lAutoSendClientCount > 0)
		{
			m_sendToClientThread.FireSendToClient(pLocalClips);
		}		
	}

	delete pLocalClips;
}

void CMainFrmThread::OnSaveRemoteClips()
{
	LogSendRecieveInfo("---------Start of OnSaveRemoteClips");

	CClipList *pLocalClips = new CClipList();

	//Save the clips locally
	{
		ATL::CCritSecLock csLock(m_cs.m_sect);

		POSITION pos;
		CClip* pClip;

		pos = m_saveRemoteClips.GetHeadPosition();
		while(pos)
		{
			pClip = m_saveRemoteClips.GetNext(pos);
			pLocalClips->AddTail(pClip);
		}

		//pLocalClips now own, the clips
		m_saveRemoteClips.RemoveAll();
	}

	LogSendRecieveInfo("---------OnSaveRemoteClips - Before AddToDB");

	int count = pLocalClips->AddToDB(true);

	LogSendRecieveInfo("---------OnSaveRemoteClips - After AddToDB");

	//are we supposed to add this clip to the clipboard
	CClip *pLastClip = pLocalClips->GetTail();
	if(pLastClip && pLastClip->m_param1 == TRUE)
	{
		LogSendRecieveInfo("---------OnSaveRemoteClips - Before Posting msg to main thread to set clipboard");

		//set the clipboard on the main thread, i was having a problem with setting the clipboard on a thread
		//guess it needs to be set on the main thread
		//main window will clear this memory
		PostMessage(theApp.m_MainhWnd, WM_LOAD_ClIP_ON_CLIPBOARD, (LPARAM)pLastClip, 0);

		LogSendRecieveInfo("---------OnSaveRemoteClips - After Posting msg to main thread to set clipboard");

		pLocalClips->RemoveTail();
	}

	theApp.RefreshView();

	delete pLocalClips;

	LogSendRecieveInfo("---------End of OnSaveRemoteClips");
}