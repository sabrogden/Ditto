// ProcessCopy.cpp: implementation of the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "ProcessCopy.h"
#include "DatabaseUtilities.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

HGLOBAL COleDataObjectEx::GetGlobalData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc)
{
    bool bRet = theApp.m_bHandleClipboardDataChange;
    theApp.m_bHandleClipboardDataChange = false;

    HGLOBAL global = COleDataObject::GetGlobalData(cfFormat, lpFormatEtc);

    theApp.m_bHandleClipboardDataChange = bRet;

    return global;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessCopy::CProcessCopy()
{
	InitializeCriticalSection(&m_CriticalSection);
}

CProcessCopy::~CProcessCopy()
{
	DeleteCriticalSection(&m_CriticalSection);
}

#define EXIT_DO_COPY(ret)	{ m_oleData.Release(); LeaveCriticalSection(&m_CriticalSection); return(ret);	}

BOOL CProcessCopy::DoCopy()
{
	EnterCriticalSection(&m_CriticalSection);
	
	//Attach the clip board
	if(!m_oleData.AttachClipboard())
		EXIT_DO_COPY(FALSE);

	m_oleData.EnsureClipboardObject();

	//Included in copy if the data is supposed to be private
	//If it is there then return
	if(m_oleData.IsDataAvailable(RegisterClipboardFormat("Clipboard Viewer Ignore")))
	{
		EXIT_DO_COPY(FALSE);
	}

	if((theApp.m_bReloadTypes) || (m_SupportedTypes.GetSize() == 0))
		LoadSupportedTypes();
	
	//Start the copy process
	//This checks if valid data is available
	//If so it adds the text the the main table and returns the id
	long lID = StartCopyProcess();
	if(lID == -1)
	{
		EXIT_DO_COPY(FALSE);
	}

	try
	{
		//open the data table and add the data for supported types
		CDataTable recset;
		CString csDbString;

		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, "SELECT * FROM Data" ,NULL);
		
		CLIPFORMAT format;
		int nSize = m_SupportedTypes.GetSize();
		for(int i = 0; i < nSize; i++)
		{
			format = m_SupportedTypes[i];

			if(m_oleData.IsDataAvailable(format))
			{
				recset.AddNew();

				recset.m_lParentID = lID;
				recset.m_strClipBoardFormat = GetFormatName(format);

				HGLOBAL hgData = m_oleData.GetGlobalData(format);

				bool bReadData = false;
				if(hgData != NULL)
				{
					recset.SetData(hgData);
				
					//update the DB
					recset.Update();
					bReadData = true;

					// Free the memory that GetGlobalData() allocated for us.
					GlobalUnlock(hgData);
					GlobalFree(hgData);
					hgData = NULL;
				}
				else
				{
					// The data isn't in global memory, so try getting an IStream 
					// interface to it.
					STGMEDIUM stg;
					
					if(m_oleData.GetData(format, &stg))
					{
						switch(stg.tymed)
						{
							case TYMED_HGLOBAL:
							{
								recset.SetData(stg.hGlobal);

								//Update the db
								recset.Update();
								bReadData = true;
							}
							break;
							
							case TYMED_ISTREAM:
							{
								UINT            uDataSize;
								LARGE_INTEGER	li;
								ULARGE_INTEGER	uli;
								
								li.HighPart = li.LowPart = 0;
								
								if ( SUCCEEDED( stg.pstm->Seek ( li, STREAM_SEEK_END, &uli )))
								{
									HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, uli.LowPart );
									void* pv = GlobalLock(hg);
									
									stg.pstm->Seek(li, STREAM_SEEK_SET, NULL);
                            
									if(SUCCEEDED(stg.pstm->Read(pv, uli.LowPart, (PULONG)&uDataSize)))
									{
										GlobalUnlock(hg);

										recset.SetData(hg);

										//Update the DB
										recset.Update();
										bReadData = true;
										
										// Free the memory we just allocated.
										GlobalFree(hg);
									}
									else
									{
										GlobalUnlock(hg);
									}
								}
							}
							break;  // case TYMED_ISTREAM
						}
						
						ReleaseStgMedium(&stg);
					}
				}

				if(bReadData)
				{
					GlobalUnlock(recset.m_ooData.m_hData);
					GlobalFree(recset.m_ooData.m_hData);
					recset.m_ooData.m_hData = NULL;
				}
			}
		}
		
		recset.Close();

		RemoveOldEntries();
		CGetSetOptions::SetTripCopyCount(-1);
		CGetSetOptions::SetTotalCopyCount(-1);
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
	}
	
	EXIT_DO_COPY(TRUE);
}

long CProcessCopy::StartCopyProcess()
{
	CString			csDescription;
	BOOL			bReturn = FALSE;
	CLIPFORMAT		MainType;
	long			lReturnID = -1;

	if(!GetCopyInfo(MainType))
		return -1;

	if(!GetDescriptionText(csDescription))
		return -1;

	long lRecID;
	if((lRecID = DoesCopyEntryExist()) >= 0)
	{
		CMainTable recset;
		recset.Open("SELECT * FROM Main WHERE lID = %d", lRecID);

		if(!recset.IsEOF())
		{
			recset.Edit();

			//Set the time to now since the entry was allready in the db
			CTime now = CTime::GetCurrentTime();
			recset.m_lDate = (long)now.GetTime();

			recset.Update();

			ShowCopyProperties(lRecID);
			return -1;
		}
		else
			ASSERT(FALSE);
	}
	
	try
	{
		//There were valid types present add the text to the main table
		CMainTable recset;

		recset.Open("SELECT * FROM Main");
			
		recset.AddNew();

		CTime now = CTime::GetCurrentTime();

		recset.m_lDate = (long)now.GetTime();

		recset.m_strType = GetDescriptionString(MainType);
		recset.m_strText = csDescription;

		recset.m_lTotalCopySize = m_lTotalCopySize;
		
		recset.Update();

		recset.MoveLast();
		
		lReturnID = recset.m_lID;

		recset.Close();

		ShowCopyProperties(lReturnID);
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		return -1;
	}

	return lReturnID;
	

	return -1;
}

void CProcessCopy::ShowCopyProperties(long lRecID)
{
	if(theApp.ShowCopyProperties)
	{
		HWND hWndFocus = GetActiveWnd();

		::SendMessage(theApp.m_MainhWnd, WM_COPYPROPERTIES, lRecID, 0);
		theApp.ShowCopyProperties = false;

		::SetForegroundWindow(hWndFocus);
	}
}

BOOL CProcessCopy::GetCopyInfo(CLIPFORMAT &MainType)
{
	m_lTotalCopySize = 0;
	MainType = 0;
	m_lSupportedTypesAvailable = 0;

	//For each type we want to save
	for(int i = 0; i < m_SupportedTypes.GetSize(); i++)
	{
		//Is data available for that type
		if(m_oleData.IsDataAvailable(m_SupportedTypes[i]))
		{
			m_lSupportedTypesAvailable++;

			HGLOBAL hgData = m_oleData.GetGlobalData(m_SupportedTypes[i]);

			//Get the size of the data
			if(hgData != NULL)
				m_lTotalCopySize += GlobalSize(hgData);
		
			if(MainType == 0)
				MainType = m_SupportedTypes[i];			
		}
	}

	return(m_lTotalCopySize > 0);
}

BOOL CProcessCopy::GetDescriptionText(CString &csText)
{
	BOOL bRet = FALSE;

	if(m_oleData.IsDataAvailable(CF_TEXT))
	{
		//Get the text version of the data and return the text
		char *text;
		HGLOBAL hgData = m_oleData.GetGlobalData(CF_TEXT);

		if(hgData != NULL)
		{
			//Get the text from the clipboard
			text = (char *)GlobalLock(hgData);
			ULONG ulBufLen = GlobalSize(hgData);

			if(ulBufLen > LENGTH_OF_TEXT_SNIPET)
				ulBufLen = LENGTH_OF_TEXT_SNIPET;

			if( ulBufLen > 0 )
			{
				char* buf = csText.GetBuffer(ulBufLen);
				memcpy(buf, text, ulBufLen); // in most cases, last char == null
				buf[ulBufLen-1] = '\0'; // just in case not null terminated
				csText.ReleaseBuffer(); // scans for the null
			}
					
			//Unlock the data
			GlobalUnlock(hgData);

			// Free the memory that GetGlobalData() allocated for us.
            GlobalFree(hgData);

			bRet = TRUE;
		}
	}
	else
	{
		//If the text is not available then get the description
		for(int i = 0; i < m_SupportedTypes.GetSize(); i++)
		{
			if(m_oleData.IsDataAvailable(m_SupportedTypes[i]))
			{
				csText = GetDescriptionString(m_SupportedTypes[i]);
				bRet = TRUE;
				break;
			}
		}
	}

	return bRet;
}

BOOL CProcessCopy::DoesCopyEntryExist()
{
	try
	{
		//Look for any other entries that have the save size
		CMainTable recset;

		recset.Open("SELECT * FROM Main WHERE lTotalCopySize = %d", m_lTotalCopySize);
		
		while(!recset.IsEOF())
		{
			//if there is any then look if it is an exact match
			if(IsExactMatch(recset.m_lID))
				return recset.m_lID;

			recset.MoveNext();
		}

		recset.Close();
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
	}

	return -1;
}

BOOL CProcessCopy::IsExactMatch(long lParentID)
{
	CDataTable recset;

	//First check if they both have the same number of saved types
	recset.Open("SELECT * FROM Data WHERE lParentID = %d", lParentID);
	recset.MoveLast();
	if(recset.GetRecordCount() != m_lSupportedTypesAvailable)
		return FALSE;
	recset.Close();
	
	//For each supported type
	for(int i = 0; i < m_SupportedTypes.GetSize(); i++)
	{
		//That data is a available
		if(m_oleData.IsDataAvailable(m_SupportedTypes[i]))
		{
			//look if there is a data with the same clipboard format name 'CF_TEXT' 'CF_DIB'
			recset.Open("SELECT * FROM Data WHERE lParentID = %d AND strClipBoardFormat = \'%s\'", 
							lParentID, GetFormatName(m_SupportedTypes[i]));

			//Move last to get the total returned
			recset.MoveLast();

			long lRecordCount = recset.GetRecordCount();

			//If it found one - it should only find one
			if(lRecordCount == 1)
			{
				//Check the internal data of each if it is not a match then return FALSE
				if(!recset.DataEqual(m_oleData.GetGlobalData(m_SupportedTypes[i])))
					return FALSE;
			}
			else if(lRecordCount == 0)
				return FALSE;
			else if(lRecordCount > 1)
			{
				ASSERT(FALSE);
				return FALSE;
			}

			recset.Close();
		}
	}

	return TRUE;
}

CString CProcessCopy::GetDescriptionString(CLIPFORMAT cbType)
{
	switch(cbType)
	{
	case CF_TEXT:
		return "CF_TEXT";
	case CF_BITMAP:
		return "CF_BITMAP";
	case CF_METAFILEPICT:
		return "CF_METAFILEPICT";
	case CF_SYLK:
		return "CF_SYLK";
	case CF_DIF:
		return "CF_DIF";
	case CF_TIFF:
		return "CF_TIFF";
	case CF_OEMTEXT:
		return "CF_OEMTEXT";
	case CF_DIB:
		return "CF_DIB";
	case CF_PALETTE:
		return "CF_PALETTE";
	case CF_PENDATA:
		return "CF_PENDATA";
	case CF_RIFF:
		return "CF_RIFF";
	case CF_WAVE:
		return "CF_WAVE";
	case CF_UNICODETEXT:
		return "CF_UNICODETEXT";
	case CF_ENHMETAFILE:
		return "CF_ENHMETAFILE";
	case CF_HDROP:
		return "CF_HDROP";
	case CF_LOCALE:
		return "CF_LOCALE";
	case CF_OWNERDISPLAY:
		return "CF_OWNERDISPLAY";
	case CF_DSPTEXT:
		return "CF_DSPTEXT";
	case CF_DSPBITMAP:
		return "CF_DSPBITMAP";
	case CF_DSPMETAFILEPICT:
		return "CF_DSPMETAFILEPICT";
	case CF_DSPENHMETAFILE:
		return "CF_DSPENHMETAFILE";
	default:
	{
		//Not a default type get the name from the clipboard
		if (cbType != 0)
		{
			TCHAR szFormat[256];
            GetClipboardFormatName(cbType, szFormat, 256);
			return szFormat;
		}
		break;
	}
	}

	return "ERROR";
}

BOOL CProcessCopy::LoadSupportedTypes()
{
	m_SupportedTypes.RemoveAll();

	try
	{
		CTypesTable recset;
		recset.Open(AFX_DAO_USE_DEFAULT_TYPE, "SELECT * FROM Types" ,NULL);
		if(recset.IsEOF())
		{
			m_SupportedTypes.Add(CF_TEXT);
			m_SupportedTypes.Add(RegisterClipboardFormat(CF_RTF));
		}
		while(!recset.IsEOF())
		{
			m_SupportedTypes.Add(GetFormatID(recset.m_TypeText));
			recset.MoveNext();
		}
	}
	catch(CDaoException* e)
	{
		ASSERT(FALSE);
		e->Delete();
		m_SupportedTypes.Add(CF_TEXT);
		m_SupportedTypes.Add(RegisterClipboardFormat(CF_RTF));
	}

	theApp.m_bReloadTypes = false;

	return TRUE;
}