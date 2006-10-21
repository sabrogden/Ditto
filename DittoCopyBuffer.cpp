#include "stdafx.h"
#include ".\dittocopybuffer.h"
#include "CP_Main.h"

CDittoCopyBuffer::CDittoCopyBuffer()
{
}

CDittoCopyBuffer::~CDittoCopyBuffer(void)
{
}


bool CDittoCopyBuffer::StartCopy(long lCopyBuffer)
{
	m_lCurrentDittoBuffer = lCopyBuffer;
	m_SavedClipboard.Save();
	theApp.SendCopy();

	return true;
}

bool CDittoCopyBuffer::EndCopy(CClipList *pClips)
{
	if(m_lCurrentDittoBuffer < 0 || m_lCurrentDittoBuffer >= 10)
	{
		Log(_T("tried to save copy buffer but copy buffer is empty"));
		return false;
	}

	m_SavedClipboard.Restore();

	bool bRet = false;

	if(pClips)
	{
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
	bool bRet = false;

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
				pClipboard->Save();

				CProcessPaste paste;
				paste.m_bSendPaste = true;
				paste.m_bActivateTarget = false;
				paste.GetClipIDs().Add(q.getIntField(_T("lID")));
				paste.DoPaste();

				pClipboard->m_lRestoreDelay = g_Opt.GetDittoRestoreClipboardDelay();

				AfxBeginThread(CDittoCopyBuffer::DelayRestoreClipboard, (LPVOID)pClipboard, THREAD_PRIORITY_LOWEST);

				bRet = true;
			}
		}
	}
	CATCH_SQLITE_EXCEPTION

	return bRet;
}

UINT CDittoCopyBuffer::DelayRestoreClipboard(LPVOID pParam)
{
	CClipboardSaveRestoreCopyBuffer *pClipboard = (CClipboardSaveRestoreCopyBuffer*)pParam;

	if(pClipboard)
	{
		Sleep(pClipboard->m_lRestoreDelay);
		pClipboard->Restore();
	}

	return TRUE;
}