#include "stdafx.h"
#include ".\dittocopybuffer.h"
#include "CP_Main.h"
#include <Mmsystem.h> //play sound

CEvent CDittoCopyBuffer::m_ActiveTimer(TRUE, TRUE);
CEvent CDittoCopyBuffer::m_RestoreTimer(TRUE, TRUE);
CEvent CDittoCopyBuffer::m_RestoreActive(TRUE, TRUE);
CDittoCopyBuffer CDittoCopyBuffer::m_Singleton;

CDittoCopyBuffer::CDittoCopyBuffer()
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

	m_ActiveTimer.SetEvent();
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

	AfxBeginThread(CDittoCopyBuffer::StartCopyTimer, (LPVOID)this, THREAD_PRIORITY_LOWEST);

	return true;
}

UINT CDittoCopyBuffer::StartCopyTimer(LPVOID pParam)
{
	m_ActiveTimer.ResetEvent();

	CDittoCopyBuffer *pBuffer = (CDittoCopyBuffer*)pParam;
	if(pBuffer)
	{
		DWORD dRes = WaitForSingleObject(m_ActiveTimer, 1500);
		if(dRes == WAIT_TIMEOUT)
		{
			pBuffer->m_bActive = false;
		}
	}

	return 0;
}

bool CDittoCopyBuffer::EndCopy(CClipList *pClips)
{
	if(m_lCurrentDittoBuffer < 0 || m_lCurrentDittoBuffer >= 10)
	{
		Log(_T("tried to save copy buffer but copy buffer is empty"));
		return false;
	}

	m_ActiveTimer.SetEvent();
	m_bActive = false;

	Log(StrF(_T("Start - Ditto EndCopy buffer = %d"), m_lCurrentDittoBuffer));

	bool bRet = false;

	if(pClips)
	{
		m_SavedClipboard.Restore();

		try
		{	
			CClip *pClip = pClips->GetHead();
			if(pClip)
			{
				int nCount = pClips->AddToDB(true);
				if(nCount > 0)
				{
					long lID = pClips->GetTail()->m_ID;
					theApp.OnCopyCompleted(lID, nCount);

					//enclose in brackets so the query closes before we update below
					{
						CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM CopyBuffers WHERE lCopyBuffer = %d"), m_lCurrentDittoBuffer);
						if(q.eof())
						{
							theApp.m_db.execDMLEx(_T("INSERT INTO CopyBuffers VALUES(NULL, -1, %d);"), m_lCurrentDittoBuffer);
						}
					}

					theApp.m_db.execDMLEx(_T("UPDATE CopyBuffers SET lClipID = %d WHERE lCopyBuffer = %d"), lID, m_lCurrentDittoBuffer);

					bRet = true;
					
					Log(StrF(_T("Ditto end copy, saved clip successfully Clip ID = %d"), lID));

					PlaySound(_T("ding.wav"), NULL, SND_FILENAME|SND_ASYNC);
				}
				else
				{
					Log(_T("Error saving Ditto Buffer to Database"));
				}
			}
			else
			{
				Log(_T("Error getting clip from cliplist"));
			}
		}
		catch (CppSQLite3Exception& e)
		{
			Log(StrF(_T("SQLITE Exception %d - %s"), e.errorCode(), e.errorMessage()));
			ASSERT(FALSE);
		}	
	}
	else
	{
		Log(_T("Ditto Buffer Ditto did not receive a copy"));
	}

	return bRet;
}

bool CDittoCopyBuffer::PastCopyBuffer(long lCopyBuffer)
{
	DWORD dNow = GetTickCount();
	if(((dNow - m_dwLastPaste) < 500) || (dNow < m_dwLastPaste))
	{
		Log(_T("Copy Buffer pasted to fast"));
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
			CClipboardSaveRestoreCopyBuffer *pClipboard = new CClipboardSaveRestoreCopyBuffer;
			if(pClipboard)
			{
				EndRestoreThread();

				if(pClipboard->Save())
				{
					CProcessPaste paste;
					paste.m_bSendPaste = true;
					paste.m_bActivateTarget = false;
					paste.GetClipIDs().Add(q.getIntField(_T("lID")));
					paste.DoPaste();

					pClipboard->m_lRestoreDelay = g_Opt.GetDittoRestoreClipboardDelay();

					Log(StrF(_T("PastCopyBuffer sent paste, starting thread to restore clipboard, Delay = %d"), pClipboard->m_lRestoreDelay));

					AfxBeginThread(CDittoCopyBuffer::DelayRestoreClipboard, (LPVOID)pClipboard, THREAD_PRIORITY_LOWEST);

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
	m_RestoreTimer.ResetEvent();
	m_RestoreActive.ResetEvent();

	CClipboardSaveRestoreCopyBuffer *pClipboard = (CClipboardSaveRestoreCopyBuffer*)pParam;
	if(pClipboard)
	{
		DWORD dRes = WaitForSingleObject(m_RestoreTimer, pClipboard->m_lRestoreDelay);

		if(GetKeyState(VK_SHIFT) & 0x8000)
		{
			Log(_T("Shift key is down not restoring clipbard, custom Buffer on normal clipboard"));
		}
		else
		{
			pClipboard->Restore();
		}

		delete pClipboard;
		pClipboard = NULL;
	}

	m_RestoreActive.SetEvent();

	return TRUE;
}