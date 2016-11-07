#include "stdafx.h"
#include ".\dittocopybuffer.h"
#include "CP_Main.h"
#include <Mmsystem.h> //play sound


CDittoCopyBuffer::CDittoCopyBuffer() :
	m_ActiveTimer(TRUE, TRUE),
	m_RestoreTimer(TRUE, TRUE),
	m_Pasting(TRUE, TRUE)
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

	if(m_SavedClipboard.Save(FALSE))
	{
		if(bCut)
		{
			theApp.m_activeWnd.SendCut();
		}
		else
		{
			theApp.m_activeWnd.SendCopy(CopyReasonEnum::COPY_TO_BUFFER);
		}

		//Create a thread to track if they have copied anything, if thread has exited before they have
		//copied something then the copy buffer copy is canceled
		AfxBeginThread(CDittoCopyBuffer::StartCopyTimer, (LPVOID)this, THREAD_PRIORITY_LOWEST);

		m_bActive = true;
		m_lCurrentDittoBuffer = lCopyBuffer;
	}
	else
	{
		Log(_T("Start of Ditto Failed to save buffer"));
	}

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
			pBuffer->m_SavedClipboard.Clear();
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

		CCopyBufferItem Item;
		g_Opt.GetCopyBufferItem(lBuffer, Item);
		if(Item.m_bPlaySoundOnCopy)
		{
			PlaySound(_T("ding.wav"), NULL, SND_FILENAME|SND_ASYNC);
		}

		return true;
	}
	CATCH_SQLITE_EXCEPTION

	return false;
}

bool CDittoCopyBuffer::PastCopyBuffer(long lCopyBuffer)
{
	//Can't paste while another is still active
	if(WaitForSingleObject(m_Pasting, 1) == WAIT_TIMEOUT)
	{
		Log(_T("Copy Buffer pasted to fast"));
		return false;
	}

	m_RestoreTimer.ResetEvent();
	m_Pasting.ResetEvent();
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
				//Save the clipboard, 
				//then put the new data on the clipboard
				//then send a paste
				//then wait a little and restore the original clipboard data
				if(m_pClipboard->Save(false))
				{
					theApp.m_pMainFrame->PasteOrShowGroup(q.getIntField(_T("lID")), -1, FALSE, TRUE, false);

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

	if(bRet == false)
		m_Pasting.SetEvent();

	return bRet;
}

void CDittoCopyBuffer::EndRestoreThread()
{
	//Tell the thread to stop waiting and restore the clipboard
	m_RestoreTimer.SetEvent();

	//make sure it's ended
	WaitForSingleObject(m_Pasting, 5000);
}

UINT CDittoCopyBuffer::DelayRestoreClipboard(LPVOID pParam)
{
	CDittoCopyBuffer *pBuffer = (CDittoCopyBuffer*)pParam;
	if(pBuffer)
	{
		CClipboardSaveRestoreCopyBuffer *pLocalClipboard = pBuffer->m_pClipboard;

		DWORD dRes = WaitForSingleObject(pBuffer->m_RestoreTimer, pLocalClipboard->m_lRestoreDelay);

		if(GetKeyState(VK_SHIFT) & 0x8000)
		{
			Log(_T("Shift key is down not restoring clipbard, custom Buffer on normal clipboard"));
		}
		else
		{
			if(pLocalClipboard->Restore())
			{
				Log(_T("CDittoCopyBuffer::DelayRestoreClipboard Successfully"));
			}
			else
			{
				Log(_T("CDittoCopyBuffer::DelayRestoreClipboard Failed to restore"));
			}
		}

		delete pLocalClipboard;
		pLocalClipboard = NULL;

		pBuffer->m_Pasting.SetEvent();
	}

	return TRUE;
}