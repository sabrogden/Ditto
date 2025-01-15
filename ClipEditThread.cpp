#include "stdafx.h"
#include "ClipEditThread.h"
#include "Options.h"
#include "Misc.h"
#include "Clip.h"
#include "CP_Main.h"
#include "ConvertRTFToText.h"
#include "Shared/TextConvert.h"

#define EVENT_FILE_CHANGED 1

#define MAX_TIMEOUT 86400

CClipEditThread::CClipEditThread()
{
	m_folderHandle = INVALID_HANDLE_VALUE;
	m_threadName = _T("ClipEditTrackingThread");
	m_waitTimeout = MAX_TIMEOUT;
}

CClipEditThread::~CClipEditThread()
{
	Close();
}

void CClipEditThread::Close()
{
	Stop();
		
	if (m_folderHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_folderHandle);
		m_folderHandle = INVALID_HANDLE_VALUE;
	}

	RemoveEvent(EVENT_FILE_CHANGED);
	m_overlapped.hEvent = INVALID_HANDLE_VALUE;	

	CString editClipFolder = CGetSetOptions::GetPath(PATH_EDIT_CLIPS);
	DeleteFolderFiles(editClipFolder, TRUE, CTimeSpan(7, 0, 0, 0));
}

void CClipEditThread::StartWatchingFolderForChanges()
{
	CString editClipFolder = CGetSetOptions::GetPath(PATH_EDIT_CLIPS);

	m_folderHandle = CreateFileW(editClipFolder, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
	
	m_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	AddEvent(EVENT_FILE_CHANGED, m_overlapped.hEvent);	

	RefreshWatch();		

	Start();
}

void CClipEditThread::WatchFile(CString filePath)
{
	ATL::CCritSecLock csLock(m_fileEditsLock.m_sect);

	nsPath::CPath path(filePath);

	//start the edit count at 0, the first edit notification is us saving the file, after that handle the file change
	m_fileEditStarts[path.GetName()] = CTime::GetCurrentTime();
}

void CClipEditThread::RefreshWatch()
{
	memset(m_fileChangeBuffer, 0, sizeof(m_fileChangeBuffer));

	DWORD bytesReturned = 0;
	ReadDirectoryChangesW(m_folderHandle, m_fileChangeBuffer, sizeof(m_fileChangeBuffer), FALSE, 
		FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE, 
		&bytesReturned, &m_overlapped, NULL);
}

void CClipEditThread::OnTimeOut(void* param)
{
	if (m_waitTimeout == MAX_TIMEOUT)
	{
		CString editClipFolder = CGetSetOptions::GetPath(PATH_EDIT_CLIPS);
		DeleteFolderFiles(editClipFolder, TRUE, CTimeSpan(7, 0, 0, 0));

		//cleanup up the list of edits that we started, after 7 days just keeps the list from growing too large, hopefull they don't have millions of edits in 7 days
		for (auto it = m_fileEditStarts.begin(); it != m_fileEditStarts.end();)
		{
			auto diff = CTime::GetCurrentTime() - it->second;
			if (diff.GetTotalSeconds() > (86400 * 7))
			{
				it = m_fileEditStarts.erase(it);
			}
			else
			{
				it++;
			}
		}		
	}
	else
	{
		for (auto const& toSave : m_filesToSave)
		{
			CString fileName = toSave.first;
			CString editClipFileName = _T("EditClip_");
			CString newClipFileName = _T("NewClip_");

			if (fileName.Find(editClipFileName, 0) == 0)
			{
				nsPath::CPath path(fileName);
				CString idString = path.GetTitle().Mid(editClipFileName.GetLength());

				int id = _wtoi(idString);
				if (id > 0)
				{
					SaveToClip(fileName, id);
				}
			}
			else if (fileName.Find(newClipFileName, 0) == 0)
			{
				SaveToClip(fileName, -1);
			}
		}

		m_filesToSave.clear();
		m_waitTimeout = MAX_TIMEOUT;
	}
}

void CClipEditThread::OnEvent(int eventId, void* param)
{
	switch (eventId)
	{
		case EVENT_FILE_CHANGED:
		{
			OnFileChanged();
			break;
		}
	}
}

void CClipEditThread::OnFileChanged()
{
	FILE_NOTIFY_INFORMATION* pNotify = m_fileChangeBuffer;
	int loopCount = 0;
	bool fileModified = false;
	CString editClipFileName = _T("EditClip_");
	CString newClipFileName = _T("NewClip_");

	while (true)
	{
		CString fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
				
		//we can't filter by the action, modify as ms word doesn't modify the file they replace it
		///so just look for anything that matches our file names

		if (pNotify->Action != FILE_ACTION_ADDED && pNotify->Action != FILE_ACTION_REMOVED)
		{
			bool addToChanges = true;			

			{
				ATL::CCritSecLock csLock(m_fileEditsLock.m_sect);
				auto exists = m_fileEditStarts.find(fileName);
				if (exists != m_fileEditStarts.end())
				{
					auto startEdit = m_fileEditStarts[fileName];
					auto diff = CTime::GetCurrentTime() - startEdit;
					if (diff.GetTotalSeconds() < 2)
					{
						Log(StrF(_T("%s has changed close to when we started editing the file, diff: %d, not handling change"), fileName, diff.GetTotalSeconds()));
						addToChanges = false;
					}
				}
				else //not in our list of files we initiated the change with
				{
					if (fileName.Find(newClipFileName, 0) == 0)
					{
						Log(StrF(_T("New clip file changed: %s, this was not in Ditto list of files we initiated the change for, not handling change"), fileName));						
						addToChanges = false;
					}
				}
			}

			if (fileName.Find(editClipFileName, 0) == -1 && fileName.Find(newClipFileName, 0) == -1)
			{
				addToChanges = false;
				Log(StrF(_T("File %s is not a Ditto file of format EditClip or NewClip, not handling change"), fileName));
			}

			if (addToChanges)
			{				
				Log(StrF(_T("%s file changed, adding to list to be saved back to Ditto"), fileName));
				m_filesToSave[fileName] = true;
				fileModified = true;				
			}
		}

		if (pNotify->NextEntryOffset <= 0 || loopCount > 1000)
		{
			break;
		}

		pNotify = (FILE_NOTIFY_INFORMATION*)((BYTE*)pNotify + pNotify->NextEntryOffset);

		if (pNotify == nullptr)
		{
			break;
		}

		loopCount++;
	}

	RefreshWatch();

	if (fileModified)
	{
		m_waitTimeout = 2000;
	}
}

bool CClipEditThread::SaveToClip(CString filePath, int id)
{
	bool savedClip = false;

	Log(StrF(_T("ClipFile: %s, ClipId: %d, has changed saving back to Ditto"), filePath, id));

	if (id < 0)
	{
		auto exists = m_newClipIds.find(filePath);
		if (exists != m_newClipIds.end())
		{
			id = m_newClipIds[filePath];
		}
	}

	CClip clip;
	if (id >= 0)
	{
		if (clip.LoadMainTable(id) == FALSE)
		{
			Log(StrF(_T("Error loading clip id: %d, not saving"), id));
			return false;
		}

		clip.LoadFormats(id);
	}			

	CString unicodeText;
	CStringA utf8Text;
	bool unicode = false;
	
	CString editClipFolder = CGetSetOptions::GetPath(PATH_EDIT_CLIPS);
	CString fullFilePath = editClipFolder + filePath;
			
	if (ReadFile(fullFilePath, unicode, unicodeText, utf8Text) == false)
	{
		Log(StrF(_T("Error reading file %s, clip id: %d, not saving"), fullFilePath, id));
		return false;
	}

	if (id < 0 &&
		unicodeText == _T("") &&
		utf8Text == "")
	{
		Log(StrF(_T("Not saving new clip that is empty, path: %s, clip id: %d, not saving"), fullFilePath, id));
		return false;
	}	

	nsPath::CPath path(filePath);

	BOOL modifyDescription = CGetSetOptions::GetUpdateDescWhenSavingClip();

	auto extenstion = path.GetExtension().MakeLower();
	if (extenstion == _T("txt"))
	{
		if (unicode)
		{
			clip.SaveFormats(&unicodeText, nullptr, nullptr, modifyDescription);
		}
		else
		{
			unicodeText = CTextConvert::Utf8ToUnicode(utf8Text);
			clip.SaveFormats(&unicodeText, nullptr, nullptr, modifyDescription);
		}
	}
	else if (extenstion == _T("rtf"))
	{
		if (GetTextFromRTF(utf8Text, unicodeText))
		{
			clip.SaveFormats(&unicodeText, nullptr, &utf8Text, modifyDescription);
		}
		else
		{
			clip.SaveFormats(nullptr, nullptr, &utf8Text, modifyDescription);
		}
	}

	//refresh the clip in the UI
	if (id == -1)
	{
		m_newClipIds[filePath] = clip.m_id;
		theApp.RefreshView(CopyReasonEnum::COPY_TO_UNKOWN);
	}
	else if (id > 0)
	{
		theApp.RefreshClipInUI(id, UPDATE_CLIP_DESCRIPTION);
	}			

	savedClip = true;	

	return savedClip;
}

bool CClipEditThread::ReadFile(CString filePath, bool &unicode, CString &unicodeText, CStringA &utf8Text)
{
	CFile file;
	CFileException ex;
	if (!file.Open(filePath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone, &ex))
	{
		CString error;
		ex.GetErrorMessage(error.GetBufferSetLength(200), 200);
		error.ReleaseBuffer();
		log(StrF(_T("LoadFormatsFromFile - Error opening file: %s, Error: %s\r\n"), filePath, error));
		return false;
	}

	if (file.GetLength() >= 2)
	{
		wchar_t header;
		file.Read(&header, sizeof(wchar_t));
		if (header == 0xFEFF)
		{
			unicode = true;
		}
		else
		{
			file.SeekToBegin();
		}
	}

	if (unicode)
	{
		const UINT bufferSize = (UINT)((file.GetLength() - 2) / 2);
		file.Read(unicodeText.GetBufferSetLength(bufferSize), bufferSize * 2);
		unicodeText.ReleaseBuffer();
	}
	else
	{
		const UINT bufferSize = (UINT)(file.GetLength());
		file.Read(utf8Text.GetBufferSetLength(bufferSize), bufferSize);
		utf8Text.ReleaseBuffer();
	}

	return true;
}

BOOL CClipEditThread::GetTextFromRTF(CStringA rtf, CString &unicodeText)
{
	CConvertRTFToText cc;
	if (cc.Create())
	{
		unicodeText = cc.GetTextFromRTF(rtf);
		cc.DestroyWindow();

		if (rtf != "" && unicodeText == "")
		{
			log(StrF(_T("Failed to convert rtf to text, rtf text is not empty but text is empty")));
		}

		return true;
	}
	else
	{
		log(StrF(_T("Failed to create rtf to text window")));
	}	

	return false;
}