#include "stdafx.h"
#include ".\dittocopybuffer.h"
#include "CP_Main.h"
#include <Mmsystem.h> //play sound


CDittoCopyBuffer::CDittoCopyBuffer() :
	m_ActiveTimer(TRUE, TRUE),
	m_RestoreTimer(TRUE, TRUE),
	m_RestoreActive(TRUE, TRUE)
{
	m_bActive = false;
	m_dwLastPaste = 0;
}

CDittoCopyBuffer::~CDittoCopyBuffer(void)
{
}


bool CDittoCopyBuffer::StartCopy(long lCopyBuffer, bool bCut)
{
	Log(StrF(_T("Start of Ditto Copy buffer = %d"), lCopyBuffer));

	//Tell the timer thread to exit
	m_ActiveTimer.SetEvent();
	//Make sure the end copy thread has exited
	EndRestoreThread();

	m_bActive = true;
	m_lCurrentDittoBuffer = lCopyBuffer;
	m_SavedClipboard.Save();
	if(bCut)
	{
		theApp.SendCut();
	}
	else
	{
		theApp.SendCopy();
	}

	//Create a thread to track if they have copied anything, if thread has exited before they have
	//copied something then the copy buffer copy is canceled
	AfxBeginThread(CDittoCopyBuffer::StartCopyTimer, (LPVOID)this, THREAD_PRIORITY_LOWEST);

	return true;
}

UINT CDittoCopyBuffer::StartCopyTimer(LPVOID pParam)
{
	CDittoCopyBuffer *pBuffer = (CDittoCopyBuffer*)pParam;
	if(pBuffer)
	{
		pBuffer->m_ActiveTimer.ResetEvent();

		DWORD dRes = WaitForSingleObject(pBuffer->m_ActiveTimer, 1500);
		if(dRes == WAIT_TIMEOUT)
		{
			pBuffer->m_bActive = false;
		}
	}

	return 0;
}

bool CDittoCopyBuffer::EndCopy(long lID)
{
	if(m_lCurrentDittoBuffer < 0 || m_lCurrentDittoBuffer >= 10)
	{
		Log(_T("tried to save copy buffer but copy buffer is empty"));
		return false;
	}

	if(m_bActive == false)
	{
		Log(_T("Current buffer is not active can't save copy buffer to db"));
		return false;
	}

	m_ActiveTimer.SetEvent();
	m_bActive = false;

	Log(StrF(_T("Start - Ditto EndCopy buffer = %d"), m_lCurrentDittoBuffer));

	bool bRet = false;

	//put the data that we stored at the start of this action back on the standard clipboard
	m_SavedClipboard.Restore();
	
	if(PutClipOnDittoCopyBuffer(lID, m_lCurrentDittoBuffer))
	{
		Log(StrF(_T("Ditto end copy, saved clip successfully Clip ID = %d"), lID));

		CCopyBufferItem Item;
		g_Opt.GetCopyBufferItem(m_lCurrentDittoBuffer, Item);
		if(Item.m_bPlaySoundOnCopy)
		{
			PlaySound(_T("ding.wav"), NULL, SND_FILENAME|SND_ASYNC);
		}

		bRet = true;
	}
	else
	{
		Log(StrF(_T("Ditto end copy, ERROR associating clip to Copy buffer ID = %d"), lID));
	}

	return bRet;
}

bool CDittoCopyBuffer::PutClipOnDittoCopyBuffer(long lClipId, long lBuffer)
{
	try
	{
		//enclose in brackets so the query closes before we update below
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM CopyBuffers WHERE lCopyBuffer = %d"), lBuffer);
			if(q.eof())
			{
				theApp.m_db.execDMLEx(_T("INSERT INTO CopyBuffers VALUES(NULL, -1, %d);"), lBuffer);
			}
		}

		theApp.m_db.execDMLEx(_T("UPDATE CopyBuffers SET lClipID = %d WHERE lCopyBuffer = %d"), lClipId, lBuffer);

		return true;
	}
	CATCH_SQLITE_EXCEPTION

	return false;
}

bool CDittoCopyBuffer::PastCopyBuffer(long lCopyBuffer)
{
	DWORD dNow = GetTickCount();
	if(((dNow - m_dwLastPaste) < 500) || (dNow < m_dwLastPaste))
	{
		Log(_T("Copy Buffer pasted to fast"));
		m_dwLastPaste = GetTickCount();
		return false;
	}
	m_dwLastPaste = GetTickCount();
	bool bRet = false;

	Log(StrF(_T("Start - PastCopyBuffer buffer = %d"), m_lCurrentDittoBuffer));

	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT Main.lID FROM Main ")
													_T("INNER JOIN CopyBuffers ON CopyBuffers.lClipID = Main.lID ")
													_T("WHERE CopyBuffers.lCopyBuffer = %d"), lCopyBuffer);

		if(q.eof() == false)
		{
			m_pClipboard = new CClipboardSaveRestoreCopyBuffer;
			if(m_pClipboard)
			{
				//make sure the previouse paste is done
				EndRestoreThread();

				if(m_pClipboard->Save())
				{
					CProcessPaste paste;
					paste.m_bSendPaste = true;
					paste.m_bActivateTarget = false;
					paste.GetClipIDs().Add(q.getIntField(_T("lID")));
					paste.DoPaste();

					m_pClipboard->m_lRestoreDelay = g_Opt.GetDittoRestoreClipboardDelay();

					Log(StrF(_T("PastCopyBuffer sent paste, starting thread to restore clipboard, Delay = %d"), m_pClipboard->m_lRestoreDelay));

					AfxBeginThread(CDittoCopyBuffer::DelayRestoreClipboard, (LPVOID)this, THREAD_PRIORITY_LOWEST);

					bRet = true;
				}
				else
				{
					Log(_T("PastCopyBuffer failed to save clipboard"));
				}
			}
		}
	}
	CATCH_SQLITE_EXCEPTION

	return bRet;
}

void CDittoCopyBuffer::EndRestoreThread()
{
	//Tell the thread to stop waiting and restore the clipboard
	m_RestoreTimer.SetEvent();
	//make sure it's ended
	WaitForSingleObject(m_RestoreActive, 5000);
}

UINT CDittoCopyBuffer::DelayRestoreClipboard(LPVOID pParam)
{
	CDittoCopyBuffer *pBuffer = (CDittoCopyBuffer*)pParam;
	if(pBuffer)
	{
		pBuffer->m_RestoreTimer.ResetEvent();
		pBuffer->m_RestoreActive.ResetEvent();

		DWORD dRes = WaitForSingleObject(pBuffer->m_RestoreTimer, pBuffer->m_pClipboard->m_lRestoreDelay);

		if(GetKeyState(VK_SHIFT) & 0x8000)
		{
			Log(_T("Shift key is down not restoring clipbard, custom Buffer on normal clipboard"));
		}
		else
		{
			pBuffer->m_pClipboard->Restore();
		}

		delete pBuffer->m_pClipboard;
		pBuffer->m_pClipboard = NULL;

		pBuffer->m_RestoreActive.SetEvent();
	}

	return TRUE;
}