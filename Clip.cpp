// ProcessCopy.cpp: implementation of the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "Clip.h"
#include "DatabaseUtilities.h"
#include "Crc32Dynamic.h"
#include "sqlite\CppSQLite3.h"
#include "shared/TextConvert.h"
#include "zlib/zlib.h"
#include "Misc.h"
#include "Md5.h"
#include "ChaiScriptOnCopy.h"
#include "DittoChaiScript.h"

#include <Mmsystem.h>

#include "Path.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/*----------------------------------------------------------------------------*\
COleDataObjectEx
\*----------------------------------------------------------------------------*/

HGLOBAL COleDataObjectEx::GetGlobalData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc)
{
    HGLOBAL hGlobal = COleDataObject::GetGlobalData(cfFormat, lpFormatEtc);
	if(hGlobal)
	{
		if(!::IsValid(hGlobal))
		{
			Log( StrF(
				_T("COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned."),
				GetFormatName(cfFormat) ) );
			::GlobalFree( hGlobal );
			hGlobal = NULL;
		}
		return hGlobal;
	}
	
	// The data isn't in global memory, so try getting an IStream interface to it.
	STGMEDIUM stg;
	
	if(!GetData(cfFormat, &stg))
	{
		return 0;
	}
	
	switch(stg.tymed)
	{
	case TYMED_HGLOBAL:
		hGlobal = stg.hGlobal;
		break;
		
	case TYMED_ISTREAM:
		{
			UINT            uDataSize;
			LARGE_INTEGER	li;
			ULARGE_INTEGER	uli;
			
			li.HighPart = li.LowPart = 0;
			
			if ( SUCCEEDED( stg.pstm->Seek ( li, STREAM_SEEK_END, &uli )))
			{
				hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, uli.LowPart );
				
				void* pv = GlobalLock(hGlobal);
				stg.pstm->Seek(li, STREAM_SEEK_SET, NULL);
				HRESULT result = stg.pstm->Read(pv, uli.LowPart, (PULONG)&uDataSize);
				GlobalUnlock(hGlobal);
				
				if( FAILED(result) )
					hGlobal = GlobalFree(hGlobal);
			}
			break;  // case TYMED_ISTREAM
		}
	} // end switch
	
	ReleaseStgMedium(&stg);
	
	if(hGlobal && !::IsValid(hGlobal))
	{
		Log( StrF(
			_T("COleDataObjectEx::GetGlobalData(\"%s\"): ERROR: Invalid (NULL) data returned."),
			GetFormatName(cfFormat)));
		::GlobalFree(hGlobal);
		hGlobal = NULL;
	}
	
	return hGlobal;
}

/*----------------------------------------------------------------------------*\
CClipFormat - holds the data of one clip format.
\*----------------------------------------------------------------------------*/
CClipFormat::CClipFormat(CLIPFORMAT cfType, HGLOBAL hgData, int parentId)
{
	m_cfType = cfType;
	m_hgData = hgData;
	m_autoDeleteData = true;
	m_parentId = parentId;
}

CClipFormat::~CClipFormat() 
{ 
	Free(); 
}

void CClipFormat::Clear()
{
	m_cfType = 0;
	m_hgData = 0;
	m_dataId = -1;
	m_parentId = -1;
}

void CClipFormat::Free()
{
	if(m_autoDeleteData)
	{
		if(m_hgData)
		{
			m_hgData = ::GlobalFree( m_hgData );
			m_hgData = NULL;
		}
	}
}

Gdiplus::Bitmap *CClipFormat::CreateGdiplusBitmap()
{
	Gdiplus::Bitmap *gdipBitmap = NULL;
	IStream* pIStream = NULL;

	if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pIStream) == S_OK)
	{
		if (this->m_cfType == CF_DIB)
		{
			LPVOID pvData = GlobalLock(this->m_hgData);
			ULONG size = (ULONG)GlobalSize(this->m_hgData);

			BITMAPINFO *lpBI = (BITMAPINFO *)pvData;

			int nPaletteEntries = 1 << lpBI->bmiHeader.biBitCount;
			if (lpBI->bmiHeader.biBitCount > 8)
				nPaletteEntries = 0;
			else if (lpBI->bmiHeader.biClrUsed != 0)
				nPaletteEntries = lpBI->bmiHeader.biClrUsed;

			BITMAPFILEHEADER BFH;
			memset(&BFH, 0, sizeof(BITMAPFILEHEADER));
			BFH.bfType = 'MB';
			BFH.bfSize = sizeof(BITMAPFILEHEADER) + size;
			BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nPaletteEntries * sizeof(RGBQUAD);

			pIStream->Write(&BFH, sizeof(BITMAPFILEHEADER), NULL);
			pIStream->Write(pvData, size, NULL);

			GlobalUnlock(this->m_hgData);

			gdipBitmap = Gdiplus::Bitmap::FromStream(pIStream);
		}
		else if (this->m_cfType == theApp.m_PNG_Format)
		{
			LPVOID pvData = GlobalLock(this->m_hgData);
			ULONG size = (ULONG)GlobalSize(this->m_hgData);
			pIStream->Write(pvData, size, NULL);

			GlobalUnlock(this->m_hgData);

			gdipBitmap = Gdiplus::Bitmap::FromStream(pIStream);
		}

		pIStream->Release();
	}

	return gdipBitmap;
}



/*----------------------------------------------------------------------------*\
CClipFormats - holds an array of CClipFormat
\*----------------------------------------------------------------------------*/
// returns a pointer to the CClipFormat in this array which matches the given type
//  or NULL if that type doesn't exist in this array.
CClipFormat* CClipFormats::FindFormat(UINT cfType)
{
	CClipFormat* pCF;
	INT_PTR count = GetSize();

	for(int i=0; i < count; i++)
	{
		pCF = &ElementAt(i);
		if(pCF->m_cfType == cfType)
			return pCF;
	}
	return NULL;
}




/*----------------------------------------------------------------------------*\
CClip - holds multiple CClipFormats and CopyClipboard() statistics
\*----------------------------------------------------------------------------*/

DWORD CClip::m_LastAddedCRC = 0;
int CClip::m_lastAddedID = -1;

CClip::CClip() : 
	m_id(0), 
	m_CRC(0),
	m_parentId(-1),
	m_dontAutoDelete(FALSE),
	m_shortCut(0),
	m_bIsGroup(FALSE),
	m_param1(0),
	m_clipOrder(0),
	m_stickyClipOrder(INVALID_STICKY),
	m_stickyClipGroupOrder(INVALID_STICKY),
	m_clipGroupOrder(0),
	m_globalShortCut(FALSE),
	m_moveToGroupShortCut(0),
	m_globalMoveToGroupShortCut(FALSE)
{
	m_copyReason = CopyReasonEnum::COPY_TO_UNKOWN;
}

CClip::~CClip()
{
	EmptyFormats();
}

void CClip::Clear()
{
	m_id = -1;
	m_Time = 0;
	m_Desc = "";
	m_CRC = 0;
	m_parentId = -1;
	m_dontAutoDelete = FALSE;
	m_shortCut = 0;
	m_bIsGroup = FALSE;
	m_csQuickPaste = "";
	m_param1 = 0;
	m_globalShortCut = FALSE;
	m_moveToGroupShortCut = 0;
	m_globalMoveToGroupShortCut = 0;
	
	EmptyFormats();
}

const CClip& CClip::operator=(const CClip &clip)
{
	const CClipFormat* pCF;

	m_id = clip.m_id;
	m_Time = clip.m_Time;
	m_lastPasteDate = clip.m_lastPasteDate;
	m_CRC = clip.m_CRC;
	m_parentId = clip.m_parentId;
	m_dontAutoDelete = clip.m_dontAutoDelete;
	m_shortCut = clip.m_shortCut;
	m_bIsGroup = clip.m_bIsGroup;
	m_csQuickPaste = clip.m_csQuickPaste;
	m_moveToGroupShortCut = clip.m_moveToGroupShortCut;
	m_globalMoveToGroupShortCut = clip.m_globalMoveToGroupShortCut;

	INT_PTR nCount = clip.m_Formats.GetSize();
	
	for(int i = 0; i < nCount; i++)
	{
		pCF = &clip.m_Formats.GetData()[i];

		LPVOID pvData = GlobalLock(pCF->m_hgData);
		if(pvData)
		{
			AddFormat(pCF->m_cfType, pvData, (UINT)GlobalSize(pCF->m_hgData));
		}
		GlobalUnlock(pCF->m_hgData);
	}

	//Set this after since in could get the wrong description in AddFormat
	m_Desc = clip.m_Desc;

	return *this;
}

void CClip::EmptyFormats()
{
	// free global memory in m_Formats
	for(INT_PTR i = m_Formats.GetSize()-1; i >= 0; i--)
	{
		m_Formats[i].Free();
		m_Formats.RemoveAt(i);
	}
}

// Adds a new Format to this Clip by copying the given data.
bool CClip::AddFormat(CLIPFORMAT cfType, void* pData, UINT nLen, bool setDesc)
{
	ASSERT(pData && nLen);
	HGLOBAL hGlobal = ::NewGlobalP(pData, nLen);
	ASSERT(hGlobal);

	// update the Clip statistics
	m_Time = m_Time.GetCurrentTime();

	if (setDesc)
	{
		if (cfType != CF_UNICODETEXT || !SetDescFromText(hGlobal, true))
			SetDescFromType();
	}
	
	CClipFormat format(cfType,hGlobal);
	CClipFormat *pFormat;
	
	pFormat = m_Formats.FindFormat(cfType);
	// if the format type already exists as part of this clip, replace the data
	if(pFormat)
	{
		pFormat->Free();
		pFormat->m_hgData = format.m_hgData;
	}
	else
	{
		m_Formats.Add(format);
	}
	
	format.m_hgData = 0; // now owned by m_Formats
	return true;
}

// Fills this CClip with the contents of the clipboard.
int CClip::LoadFromClipboard(CClipTypes* pClipTypes, bool checkClipboardIgnore, CString activeApp)
{
	COleDataObjectEx oleData;
	CClipTypes defaultTypes;
	CClipTypes* pTypes = pClipTypes;

	// m_Formats should be empty when this is called.
	ASSERT(m_Formats.GetSize() == 0);
	
	// If the data is supposed to be private, then return
	if(::IsClipboardFormatAvailable(theApp.m_cfIgnoreClipboard))
	{
		Log(_T("Clipboard ignore type is on the clipboard, skipping this clipboard change"));
		return FALSE;
	}

	//If we are saving a multi paste then delay us connecting to the clipboard
	//to allow the ctrl-v to do a paste
	if(::IsClipboardFormatAvailable(theApp.m_cfDelaySavingData))
	{
		Log(_T("Delay clipboard type is on the clipboard, delaying 1500 ms to allow ctrl-v to work"));
		Sleep(1500);
	}
		
	//Attach to the clipboard
	if(!oleData.AttachClipboard())
	{
		Log(_T("failed to attache to clipboard, skipping this clipboard change"));
		ASSERT(0); // does this ever happen?
		return FALSE;
	}
	
	oleData.EnsureClipboardObject();
	
	// if no types were given, get only the first (most important) type.
	//  (subsequent types could be synthetic due to automatic type conversions)
	if(pTypes == NULL || pTypes->GetSize() == 0)
	{
		ASSERT(0); // this feature is not currently used... it is an error if it is.
		
		FORMATETC formatEtc;
		oleData.BeginEnumFormats();
		oleData.GetNextFormat(&formatEtc);
		defaultTypes.Add(formatEtc.cfFormat);
		pTypes = &defaultTypes;
	}
	
	m_Desc = "[Ditto Error] BAD DESCRIPTION";
	
	// Get Description String
	// NOTE: We make sure that the description always corresponds to the
	//  data saved by using the exact same globalmem instance as the source
	//  for both... i.e. we only fetch the description format type once.
	CClipFormat cfDesc;
	bool bIsDescSet = false;

	cfDesc.m_cfType = CF_UNICODETEXT;	
	if(oleData.IsDataAvailable(cfDesc.m_cfType))
	{
		for (int i = 0; i < 10; i++)
		{
			cfDesc.m_hgData = oleData.GetGlobalData(cfDesc.m_cfType);
			if (cfDesc.m_hgData == NULL)
			{
				Log(StrF(_T("Tried to set description from cf_unicode, data is NULL, try: %d"), i+1));
			}
			else
			{
				break;
			}
			Sleep(10);
		}
		bIsDescSet = SetDescFromText(cfDesc.m_hgData, true);

		if (activeApp != _T(""))
		{
			TCHAR* text = (TCHAR *)GlobalLock(cfDesc.m_hgData);
			if (text != NULL)
			{
				std::wstring stringData(text);
				GlobalUnlock(cfDesc.m_hgData);
				if (g_Opt.m_regexHelper.TextMatchFilters(activeApp, stringData))
				{
					return -1;
				}
			}
			else
			{
				GlobalUnlock(cfDesc.m_hgData);
			}
		}

		Log(StrF(_T("Tried to set description from cf_unicode text, Set: %d, Desc: [%s]"), bIsDescSet, m_Desc.Left(30)));
	}

	if(bIsDescSet == false)
	{
		cfDesc.m_cfType = CF_TEXT;	
		if(oleData.IsDataAvailable(cfDesc.m_cfType))
		{
			for (int i = 0; i < 10; i++)
			{
				cfDesc.m_hgData = oleData.GetGlobalData(cfDesc.m_cfType);
				if (cfDesc.m_hgData == NULL)
				{
					Log(StrF(_T("Tried to set description from cf_text, data is NULL, try: %d"), i + 1));
				}
				else
				{
					break;
				}
				Sleep(10);
			}

			bIsDescSet = SetDescFromText(cfDesc.m_hgData, false);

			Log(StrF(_T("Tried to set description from cf_text text, Set: %d, Desc: [%s]"), bIsDescSet, m_Desc.Left(30)));
		}
	}

	INT_PTR nSize;
	CClipFormat cf;
	INT_PTR numTypes = pTypes->GetSize();

	Log(StrF(_T("Begin enumerating over supported types, Count: %d"), numTypes));

	for(int i = 0; i < numTypes; i++)
	{
		cf.m_cfType = pTypes->ElementAt(i);

		if (cf.m_cfType == CF_DIB &&
			g_Opt.m_excludeCF_DIBInExcel &&
			activeApp.MakeLower() == _T("excel.exe"))
		{
			continue;
		}

		BOOL bSuccess = false;
		Log(StrF(_T("Begin try and load type %s"), GetFormatName(cf.m_cfType)));
		
		// is this the description we already fetched?
		if(cf.m_cfType == cfDesc.m_cfType)
		{
			cf = cfDesc;
			cfDesc.m_hgData = 0; // cf owns it now (to go into m_Formats)
		}
		else if(!oleData.IsDataAvailable(cf.m_cfType))
		{
			Log(StrF(_T("End of load - Data is not available for type %s"), GetFormatName(cf.m_cfType)));
			continue;
		}
		else
		{
			for (int i = 0; i < 2; i++)
			{
				cf.m_hgData = oleData.GetGlobalData(cf.m_cfType);
				if (cf.m_hgData != NULL)
				{
					break;
				}

				Log(StrF(_T("Tried to get data for type: %s, data is NULL, try: %d"), GetFormatName(cf.m_cfType), i + 1));
				Sleep(5);
			}
		}
		
		if(cf.m_hgData)
		{
			nSize = GlobalSize(cf.m_hgData);
			if(nSize > 0)
			{
				if(g_Opt.m_lMaxClipSizeInBytes > 0 && (int)nSize > g_Opt.m_lMaxClipSizeInBytes)
				{
					CString cs;
					cs.Format(_T("Maximum clip size reached max size = %d, clip size = %d"), g_Opt.m_lMaxClipSizeInBytes, nSize);
					Log(cs);

					oleData.Release();
					return -1;
				}

				ASSERT(::IsValid(cf.m_hgData));
				
				m_Formats.Add(cf);
				bSuccess = true;
			}
			else
			{
				ASSERT(FALSE); // a valid GlobalMem with 0 size is strange
				cf.Free();
				Log(StrF(_T("Data length is 0 for type %s"), GetFormatName(cf.m_cfType)));
			}
			cf.m_hgData = 0; // m_Formats owns it now
		}

		Log(StrF(_T("End of load - type %s, Success: %d"), GetFormatName(cf.m_cfType), bSuccess));
	}

	Log(StrF(_T("End enumerating over supported types, Count: %d"), numTypes));
	
	m_Time = CTime::GetCurrentTime();
	
	if(!bIsDescSet)
	{
		SetDescFromType();

		Log(StrF(_T("Setting description from type, Desc: [%s]"), m_Desc.Left(30)));
	}
	
	// if the description was in a type that is not supported,
	//we have to free it since it wasn't added to m_Formats
	if(cfDesc.m_hgData)
	{
		cfDesc.Free();
	}
	
	oleData.Release();

	if (!bIsDescSet &&
		this->m_Desc != _T(""))
	{
		std::wstring stringData(this->m_Desc);
		if (g_Opt.m_regexHelper.TextMatchFilters(activeApp, stringData))
		{
			return -1;
		}
	}
	
	if(m_Formats.GetSize() == 0)
	{
		Log(_T("No clip types were in supported types array"));
		return FALSE;
	}

	try
	{
		for (auto & listItem : g_Opt.m_copyScripts.m_list)
		{
			if (listItem.m_active)
			{
				Log(StrF(_T("Start of process copy name: %s, script: %s"), listItem.m_name, listItem.m_script));

				ChaiScriptOnCopy onCopy;
				CDittoChaiScript clipData(this, (LPCSTR)CTextConvert::ConvertToChar(activeApp));
				if (onCopy.ProcessScript(clipData, (LPCSTR)CTextConvert::ConvertToChar(listItem.m_script)) == false)
				{
					Log(StrF(_T("End of process copy name: %s, returned false, not saving this copy to Ditto, last Error: %s"), listItem.m_name, onCopy.m_lastError));

					return -1;
				}

				Log(StrF(_T("End of process copy name: %s, returned true, last Error: %s"), listItem.m_name, onCopy.m_lastError));
			}
			else
			{
				Log(StrF(_T("Script is not active, not processing name: %s, script: %s"), listItem.m_name, listItem.m_script));
			}
		}
	}
	catch (CException *ex)
	{
		TCHAR szCause[255];
		ex->GetErrorMessage(szCause, 255);
		CString cs;
		cs.Format(_T("save copy exception: %s"), szCause);
		Log(cs);
	}
	catch (...)
	{
		Log(_T("save copy exception 2"));
	}

	return TRUE;
}

bool CClip::SetDescFromText(HGLOBAL hgData, bool unicode)
{
	if(hgData == 0)
		return false;
	
	bool bRet = false;
	INT_PTR bufLen = 0;

	if(unicode)
	{
		TCHAR* text = (TCHAR *) GlobalLock(hgData);
		bufLen = GlobalSize(hgData);

		m_Desc = text;
		bRet = true;
	}
	else
	{
		char* text = (char *) GlobalLock(hgData);
		bufLen = GlobalSize(hgData);
	
		m_Desc = text;
		bRet = true;
	}
		
	if(bufLen > g_Opt.m_bDescTextSize)
	{
		m_Desc = m_Desc.Left(g_Opt.m_bDescTextSize);
	}
	
	//Unlock the data
	GlobalUnlock(hgData);
	
	return bRet;
}

bool CClip::SetDescFromType()
{
	INT_PTR size = m_Formats.GetSize();
	if(size <= 0)
	{
		return false;
	}

	int nCF_HDROPIndex = -1;
	for(int i = 0; i < size; i++)
	{
		if(m_Formats[i].m_cfType == CF_HDROP)
		{
			nCF_HDROPIndex = i;
		}
	}

	if(nCF_HDROPIndex >= 0)
	{
		using namespace nsPath;

		HDROP drop = (HDROP)GlobalLock(m_Formats[nCF_HDROPIndex].m_hgData);
		int nNumFiles = min(5, DragQueryFile(drop, -1, NULL, 0));

		if(nNumFiles > 1)
			m_Desc = "Copied Files - ";
		else
			m_Desc = "Copied File - ";

		TCHAR file[MAX_PATH];
		
		for(int nFile = 0; nFile < nNumFiles; nFile++)
		{
			if(DragQueryFile(drop, nFile, file, sizeof(file)) > 0)
			{
				CPath path(file);
				m_Desc += path.GetName();
				m_Desc += " - ";
				m_Desc += file;
				m_Desc += "\n";
			}
		}
		
		GlobalUnlock(m_Formats[nCF_HDROPIndex].m_hgData);
	}
	else
	{
		m_Desc = GetFormatName(m_Formats[0].m_cfType);
	}

	return m_Desc.GetLength() > 0;
}

bool CClip::AddToDB(bool bCheckForDuplicates)
{
	bool bResult;
	try
	{
		m_CRC = GenerateCRC();

		if(bCheckForDuplicates &&
			m_parentId < 0)
		{	
			int nID = FindDuplicate();
			if(nID >= 0)
			{
				MakeLatestOrder();
				MakeLatestGroupOrder();

				CString sql;
				
				sql.Format(_T("UPDATE Main SET clipOrder = %f where lID = %d;"), 
								m_clipOrder, nID);

				int ret = theApp.m_db.execDML(sql);

				int groupRet = -1;

				if(m_parentId > -1)
				{
					sql.Format(_T("UPDATE Main SET clipGroupOrder = %f where lID = %d;"), 
						m_clipGroupOrder, nID);

					groupRet = theApp.m_db.execDML(sql);
				}


				m_id = nID;

				Log(StrF(_T("Found duplicate clip in db, Id: %d, ParentId: %d crc: %d, NewOrder: %f, GroupOrder %f, Ret: %d, GroupRet: %d, SQL: %s"), 
										nID, m_parentId, m_CRC, m_clipOrder, m_clipGroupOrder, ret, groupRet, sql));

				return true;
			}
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
	
	bResult = false;
	if(AddToMainTable())
	{		
		bResult = AddToDataTable();
	}

	if(bResult)
	{
		if(g_Opt.m_csPlaySoundOnCopy.IsEmpty() == FALSE)
			PlaySound(g_Opt.m_csPlaySoundOnCopy, NULL, SND_FILENAME|SND_ASYNC);
	}
	
	// should be emptied by AddToDataTable
	//ASSERT(m_Formats.GetSize() == 0);
	
	return bResult;
}

// if a duplicate exists, set recset to the duplicate and return true
int CClip::FindDuplicate()
{
	try
	{
		//If they are allowing duplicates still check 
		//the last copied item
		if(g_Opt.m_bAllowDuplicates)
		{
			if(m_CRC == m_LastAddedCRC)
				return m_lastAddedID;
		}
		else
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Main WHERE CRC = %d"), m_CRC);
				
			if(q.eof() == false)
			{
				return q.getIntField(_T("lID"));
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return -1;
}



DWORD CClip::GenerateCRC()
{
	CClipFormat* pCF;
	DWORD dwCRC = 0xFFFFFFFF;

	CCrc32Dynamic *pCrc32 = new CCrc32Dynamic;
	if(pCrc32)
	{
		//Generate a CRC value for all copied data

		INT_PTR size = m_Formats.GetSize();
		for(int i = 0; i < size ; i++)
		{
			pCF = & m_Formats.ElementAt(i);
			
			const unsigned char *Data = (const unsigned char *)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				if (CGetSetOptions::GetAdjustClipsForCRC())
				{
					//Try and remove known things that change in rtf (word and outlook)
					if (pCF->m_cfType == theApp.m_RTFFormat)
					{
						CStringA CStringData((char*)Data);

						//In word and outlook I was finding that data in the \\datastore section was always changing, remove this for the crc check
						RemoveRTFSection(CStringData, "{\\*\\datastore");

						//In word and outlook rsid values are always changing, remove these for the crc check
						DeleteParamFromRTF(CStringData, "\\rsid", true);
						DeleteParamFromRTF(CStringData, "\\insrsid", true);
						DeleteParamFromRTF(CStringData, "\\mdispDef1", false);

						pCrc32->GenerateCrc32((const LPBYTE)CStringData.GetBuffer(), (DWORD)CStringData.GetLength(), dwCRC);
					}
					else
					{
						//i've seen examble where the text size was 10 but the data size was 20, leading to random crc values
						//try and only check the crc for the actual text
						int dataLength = (int)GlobalSize(pCF->m_hgData);
						if (pCF->m_cfType == CF_TEXT)
						{
							dataLength = min(dataLength, ((int)strlen((char*)Data) + 1));
						}
						else if (pCF->m_cfType == CF_UNICODETEXT)
						{
							dataLength = min(dataLength, (((int)wcslen((wchar_t*)Data) + 1) * 2));
						}
						pCrc32->GenerateCrc32((const LPBYTE)Data, (DWORD)dataLength, dwCRC);
					}
				}
				else
				{
					pCrc32->GenerateCrc32((const LPBYTE)Data, (DWORD)GlobalSize(pCF->m_hgData), dwCRC);
				}
			}
			GlobalUnlock(pCF->m_hgData);
		}

		dwCRC = ~dwCRC;

		delete pCrc32;
	}

	return dwCRC;
}

// assigns m_ID
bool CClip::AddToMainTable()
{
	try
	{
		m_Desc.Replace(_T("'"), _T("''"));
		m_csQuickPaste.Replace(_T("'"), _T("''"));

		CString cs;
		cs.Format(_T("INSERT into Main (lDate, mText, lShortCut, lDontAutoDelete, CRC, bIsGroup, lParentID, QuickPasteText, clipOrder, clipGroupOrder, globalShortCut, lastPasteDate, stickyClipOrder, stickyClipGroupOrder, MoveToGroupShortCut, GlobalMoveToGroupShortCut) ")
						_T("values(%d, '%s', %d, %d, %d, %d, %d, '%s', %f, %f, %d, %d, %f, %f, %d, %d);"),
							(int)m_Time.GetTime(),
							m_Desc,
							m_shortCut,
							m_dontAutoDelete,
							m_CRC,
							m_bIsGroup,
							m_parentId,
							m_csQuickPaste,
							m_clipOrder,
							m_clipGroupOrder,
							m_globalShortCut,
							(int)CTime::GetCurrentTime().GetTime(),
							m_stickyClipOrder,
							m_stickyClipGroupOrder,
							m_moveToGroupShortCut,
							m_globalMoveToGroupShortCut);

		theApp.m_db.execDML(cs);

		m_id = (long)theApp.m_db.lastRowId();

		Log(StrF(_T("Added clip to main table, Id: %d, ParentId: %d Desc: %s, Order: %f, GroupOrder: %f"), m_id, m_parentId, m_Desc, m_clipOrder, m_clipGroupOrder));

		m_LastAddedCRC = m_CRC;
		m_lastAddedID = m_id;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
	
	return true;
}

bool CClip::ModifyMainTable()
{
	bool bRet = false;
	try
	{
		m_Desc.Replace(_T("'"), _T("''"));
		m_csQuickPaste.Replace(_T("'"), _T("''"));

		theApp.m_db.execDMLEx(_T("UPDATE Main SET lShortCut = %d, ")
			_T("mText = '%s', ")
			_T("lParentID = %d, ")
			_T("lDontAutoDelete = %d, ")
			_T("QuickPasteText = '%s', ")
			_T("clipOrder = %f, ")
			_T("clipGroupOrder = %f, ")
			_T("globalShortCut = %d, ")
			_T("stickyClipOrder = %f, ")
			_T("stickyClipGroupOrder = %f, ")
			_T("MoveToGroupShortCut = %d, ")
			_T("GlobalMoveToGroupShortCut = %d ")
			_T("WHERE lID = %d;"), 
			m_shortCut, 
			m_Desc, 
			m_parentId, 
			m_dontAutoDelete, 
			m_csQuickPaste,
			m_clipOrder,
			m_clipGroupOrder,
			m_globalShortCut,
			m_stickyClipOrder,
			m_stickyClipGroupOrder,
			m_moveToGroupShortCut,
			m_globalMoveToGroupShortCut,
			m_id);

		bRet = true;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)

	return bRet;
}

bool CClip::ModifyDescription()
{
	bool bRet = false;
	try
	{
		m_Desc.Replace(_T("'"), _T("''"));

		theApp.m_db.execDMLEx(_T("UPDATE Main SET mText = '%s' ")
			_T("WHERE lID = %d;"),
			m_Desc,
			m_id);

		bRet = true;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)

		return bRet;
}

// Empties m_Formats as it saves them to the Data Table.
bool CClip::AddToDataTable()
{
	CClipFormat* pCF;

	try
	{
		CppSQLite3Statement stmt = theApp.m_db.compileStatement(_T("insert into Data values (NULL, ?, ?, ?);"));
		
		for(INT_PTR i = m_Formats.GetSize()-1; i >= 0 ; i--)
		{
			pCF = &m_Formats.ElementAt(i);

			CString formatName = GetFormatName(pCF->m_cfType);
			int clipSize = 0;
			
			stmt.bind(1, m_id);
			stmt.bind(2, formatName);

			const unsigned char *Data = (const unsigned char *)GlobalLock(pCF->m_hgData);
			if(Data)
			{
				clipSize = (int)GlobalSize(pCF->m_hgData);
				stmt.bind(3, Data, clipSize);
			}
			GlobalUnlock(pCF->m_hgData);
			
			stmt.execDML();
			stmt.reset();

			pCF->m_dataId = (long)theApp.m_db.lastRowId();

			Log(StrF(_T("Added ClipData to DB, Id: %d, ParentId: %d Type: %s, size: %d"), pCF->m_dataId, m_id, formatName, clipSize));
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
		
	return true;
}

void CClip::MoveUp(int parentId)
{
	try
	{
		//In a group, not a sticky
		if(parentId > -1 && m_stickyClipGroupOrder == INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, clipGroupOrder FROM Main Where lParentID = %d AND stickyClipGroupOrder == -(2147483647) AND clipGroupOrder > %f ORDER BY clipGroupOrder ASC LIMIT 1"), parentId, m_clipGroupOrder);
			if (q.eof() == false)
			{
				int idAbove = q.getIntField(_T("lID"));
				double orderAbove = q.getFloatField(_T("clipGroupOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, clipGroupOrder FROM Main Where lParentID = %d AND clipGroupOrder > %f AND stickyClipGroupOrder == -(2147483647) ORDER BY clipGroupOrder ASC LIMIT 1"), parentId, orderAbove);
				if (q2.eof() == false)
				{ 
					int idTwoAbove = q2.getIntField(_T("lID"));
					double orderTwoAbove = q2.getFloatField(_T("clipGroupOrder"));

					m_clipGroupOrder = orderAbove + (orderTwoAbove - orderAbove) / 2.0;
				}
				else
				{
					m_clipGroupOrder = orderAbove + 1;
				}
			}
		}
		// main group, not a sticky
		else if(parentId <= -1 && m_stickyClipOrder == INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, clipOrder FROM Main Where clipOrder > %f AND stickyClipOrder == -(2147483647) ORDER BY clipOrder ASC LIMIT 1"), m_clipOrder);
			if (q.eof() == false)
			{
				int idAbove = q.getIntField(_T("lID"));
				double orderAbove = q.getFloatField(_T("clipOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, clipOrder FROM Main Where clipOrder > %f AND stickyClipOrder == -(2147483647) ORDER BY clipOrder ASC LIMIT 1"), orderAbove);
				if (q2.eof() == false)
				{ 
					int idTwoAbove = q2.getIntField(_T("lID"));
					double orderTwoAbove = q2.getFloatField(_T("clipOrder"));

					m_clipOrder = orderAbove + (orderTwoAbove - orderAbove) / 2.0;
				}
				else
				{
					m_clipOrder = orderAbove + 1;
				}
			}
		}
		//In a group, a sticky clip
		else if(parentId > -1 && m_stickyClipGroupOrder != INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipGroupOrder FROM Main Where lParentID = %d AND stickyClipGroupOrder <> -(2147483647) AND stickyClipGroupOrder > %f ORDER BY stickyClipGroupOrder ASC LIMIT 1"), parentId, m_stickyClipGroupOrder);
			if (q.eof() == false)
			{
				int idAbove = q.getIntField(_T("lID"));
				double orderAbove = q.getFloatField(_T("stickyClipGroupOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipGroupOrder FROM Main Where lParentID = %d AND stickyClipGroupOrder <> -(2147483647) AND stickyClipGroupOrder > %f ORDER BY stickyClipGroupOrder ASC LIMIT 1"), parentId, orderAbove);
				if (q2.eof() == false)
				{
					int idTwoAbove = q2.getIntField(_T("lID"));
					double orderTwoAbove = q2.getFloatField(_T("stickyClipGroupOrder"));

					m_stickyClipGroupOrder = orderAbove + (orderTwoAbove - orderAbove) / 2.0;
				}
				else
				{
					m_stickyClipGroupOrder = orderAbove + 1;
				}
			}
		}
		//not in a group, a sticky clip
		else if(parentId <= -1 && m_stickyClipOrder != INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipOrder FROM Main Where stickyClipOrder <> -(2147483647) AND stickyClipOrder > %f ORDER BY stickyClipOrder ASC LIMIT 1"), m_stickyClipOrder);
			if (q.eof() == false)
			{
				int idAbove = q.getIntField(_T("lID"));
				double orderAbove = q.getFloatField(_T("stickyClipOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipOrder FROM Main Where stickyClipOrder <> -(2147483647) AND stickyClipOrder > %f ORDER BY stickyClipOrder ASC LIMIT 1"), orderAbove);
				if (q2.eof() == false)
				{
					int idTwoAbove = q2.getIntField(_T("lID"));
					double orderTwoAbove = q2.getFloatField(_T("stickyClipOrder"));

					m_stickyClipOrder = orderAbove + (orderTwoAbove - orderAbove) / 2.0;
				}
				else
				{
					m_stickyClipOrder = orderAbove + 1;
				}
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CClip::MoveDown(int parentId)
{
	try
	{
		//In a group, not a sticky
		if(parentId > -1 && m_stickyClipGroupOrder == INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, clipGroupOrder FROM Main Where lParentID = %d AND clipGroupOrder < %f AND stickyClipGroupOrder = -(2147483647) ORDER BY clipGroupOrder DESC LIMIT 1"), parentId, m_clipGroupOrder);
			if (q.eof() == false)
			{
				int idBelow = q.getIntField(_T("lID"));
				double orderBelow = q.getFloatField(_T("clipGroupOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, clipGroupOrder FROM Main Where lParentID = %d AND clipGroupOrder < %f AND stickyClipGroupOrder = -(2147483647) ORDER BY clipGroupOrder DESC LIMIT 1"), parentId, orderBelow);
				if (q2.eof() == false)
				{ 
					int idTwoBelow = q2.getIntField(_T("lID"));
					double orderTwoBelow = q2.getFloatField(_T("clipGroupOrder"));

					m_clipGroupOrder = orderBelow + (orderTwoBelow - orderBelow) / 2.0;
				}
				else
				{
					m_clipGroupOrder = orderBelow - 1;
				}
			}
		}
		// main group, not a sticky
		else if(parentId <= -1 && m_stickyClipOrder == INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, clipOrder FROM Main Where clipOrder < %f AND stickyClipOrder = -(2147483647) ORDER BY clipOrder DESC LIMIT 1"), m_clipOrder);
			if (q.eof() == false)
			{
				int idBelow = q.getIntField(_T("lID"));
				double orderBelow = q.getFloatField(_T("clipOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, clipOrder FROM Main Where clipOrder < %f AND stickyClipOrder = -(2147483647) ORDER BY clipOrder DESC LIMIT 1"), orderBelow);
				if (q2.eof() == false)
				{ 
					int idTwoBelow = q2.getIntField(_T("lID"));
					double orderTwoBelow = q2.getFloatField(_T("clipOrder"));

					m_clipOrder = orderBelow + (orderTwoBelow - orderBelow) / 2.0;
				}
				else
				{
					m_clipOrder = orderBelow - 1;
				}
			}
		}
		//In a group, a sticky clip
		else if(parentId > -1 && m_stickyClipGroupOrder != INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipGroupOrder FROM Main Where lParentID = %d AND stickyClipGroupOrder <> -(2147483647) AND stickyClipGroupOrder < %f ORDER BY stickyClipGroupOrder DESC LIMIT 1"), parentId, m_stickyClipGroupOrder);
			if (q.eof() == false)
			{
				int idBelow = q.getIntField(_T("lID"));
				double orderBelow = q.getFloatField(_T("stickyClipGroupOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipGroupOrder FROM Main Where lParentID = %d AND stickyClipGroupOrder <> -(2147483647) AND stickyClipGroupOrder < %f ORDER BY stickyClipGroupOrder DESC LIMIT 1"), parentId, orderBelow);
				if (q2.eof() == false)
				{
					int idTwoBelow = q2.getIntField(_T("lID"));
					double orderTwoBelow = q2.getFloatField(_T("stickyClipGroupOrder"));

					m_stickyClipGroupOrder = orderBelow + (orderTwoBelow - orderBelow) / 2.0;
				}
				else
				{
					m_stickyClipGroupOrder = orderBelow - 1;
				}
			}
		}
		//not in a group, a sticky clip
		else if(parentId <= -1 && m_stickyClipOrder != INVALID_STICKY)
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipOrder FROM Main Where stickyClipOrder <> -(2147483647) AND stickyClipOrder < %f ORDER BY stickyClipOrder DESC LIMIT 1"), m_stickyClipOrder);
			if (q.eof() == false)
			{
				int idBelow = q.getIntField(_T("lID"));
				double orderBelow = q.getFloatField(_T("stickyClipOrder"));

				CppSQLite3Query q2 = theApp.m_db.execQueryEx(_T("SELECT lID, stickyClipOrder FROM Main Where stickyClipOrder <> -(2147483647) AND stickyClipOrder < %f ORDER BY stickyClipOrder DESC LIMIT 1"), orderBelow);
				if (q2.eof() == false)
				{
					int idTwoBelow = q2.getIntField(_T("lID"));
					double orderTwoBelow = q2.getFloatField(_T("stickyClipOrder"));

					m_stickyClipOrder = orderBelow + (orderTwoBelow - orderBelow) / 2.0;
				}
				else
				{
					m_stickyClipOrder = orderBelow - 1;
				}
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CClip::MakeStickyTop(int parentId)
{
	if (parentId < 0)
	{
		m_stickyClipOrder = GetNewTopSticky(parentId, m_id);
	}
	else
	{
		m_stickyClipGroupOrder = GetNewTopSticky(parentId, m_id);
	}
}

void CClip::MakeStickyLast(int parentId)
{
	if (parentId < 0)
	{
		m_stickyClipOrder = GetNewLastSticky(parentId, m_id);
	}
	else
	{
		m_stickyClipGroupOrder = GetNewLastSticky(parentId, m_id);
	}
}

bool CClip::RemoveStickySetting(int parentId)
{
	bool reset = false;
	if (parentId < 0)
	{
		if (m_stickyClipOrder != INVALID_STICKY)
		{
			m_stickyClipOrder = INVALID_STICKY;
			reset = true;
		}
	}
	else
	{
		if (m_stickyClipGroupOrder != INVALID_STICKY)
		{
			m_stickyClipGroupOrder = INVALID_STICKY;
			reset = true;
		}
	}

	return reset;
}

double CClip::GetNewTopSticky(int parentId, int clipId)
{
	double newOrder = 1;
	double existingMaxOrder = 0;
	CString existingDesc = _T("");

	try
	{
		if (parentId < 0)
		{
			CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT stickyClipOrder, mText FROM Main WHERE stickyClipOrder <> -(2147483647) ORDER BY stickyClipOrder DESC LIMIT 1"));
			if (q.eof() == false)
			{
				existingMaxOrder = q.getFloatField(_T("stickyClipOrder"));
				existingDesc = q.getStringField(_T("mText"));
				newOrder = existingMaxOrder + 1;
			}
		}
		else
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT stickyClipGroupOrder, mText FROM Main WHERE lParentID = %d AND stickyClipGroupOrder <> -(2147483647) ORDER BY stickyClipGroupOrder DESC LIMIT 1"), parentId);
			if (q.eof() == false)
			{
				existingMaxOrder = q.getFloatField(_T("stickyClipGroupOrder"));
				newOrder = existingMaxOrder + 1;
			}
		}

		if (newOrder == 0.0)
			newOrder += 1;

		Log(StrF(_T("GetNewTopSticky, Id: %d, parentId: %d, CurrentMax: %f, CurrentDesc: %s, NewMax: %f"), clipId, parentId, existingMaxOrder, existingDesc, newOrder));
	}
	CATCH_SQLITE_EXCEPTION

	return newOrder;
}

double CClip::GetNewLastSticky(int parentId, int clipId)
{
	double newOrder = 1;
	double existingMaxOrder = 0;
	CString existingDesc = _T("");

	try
	{
		if (parentId < 0)
		{
			CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT stickyClipOrder, mText FROM Main WHERE stickyClipOrder <> -(2147483647) ORDER BY stickyClipOrder LIMIT 1"));
			if (q.eof() == false)
			{
				existingMaxOrder = q.getFloatField(_T("stickyClipOrder"));
				existingDesc = q.getStringField(_T("mText"));
				newOrder = existingMaxOrder - 1;
			}
		}
		else
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT stickyClipGroupOrder, mText FROM Main WHERE lParentID = %d AND stickyClipGroupOrder <> -(2147483647) ORDER BY stickyClipGroupOrder LIMIT 1"), parentId);
			if (q.eof() == false)
			{
				existingMaxOrder = q.getFloatField(_T("stickyClipGroupOrder"));
				newOrder = existingMaxOrder - 1;
			}
		}

		if (newOrder == 0.0)
			newOrder -= 1;

		Log(StrF(_T("GetNewLastSticky, Id: %d, parentId: %d, CurrentMax: %f, CurrentDesc: %s, NewMax: %f"), clipId, parentId, existingMaxOrder, existingDesc, newOrder));
	}
	CATCH_SQLITE_EXCEPTION

		return newOrder;
}

void CClip::MakeLatestOrder()
{
	m_clipOrder = GetNewOrder(-1, m_id);
}

void CClip::MakeLatestGroupOrder()
{
	if(m_parentId > -1)
	{
		m_clipGroupOrder = GetNewOrder(m_parentId, m_id);
	}
}

void CClip::MakeLastOrder()
{
	m_clipOrder = GetNewLastOrder(-1, m_id);
}

void CClip::MakeLastGroupOrder()
{
	if (m_parentId > -1)
	{
		m_clipGroupOrder = GetNewLastOrder(m_parentId, m_id);
	}
}

double CClip::GetNewOrder(int parentId, int clipId)
{
	double newOrder = 0;
	double existingMaxOrder = 0;
	CString existingDesc = _T("");

	try
	{
		if(parentId < 0)
		{
			CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT clipOrder, mText FROM Main ORDER BY clipOrder DESC LIMIT 1"));			
			if(q.eof() == false)
			{
				existingMaxOrder = q.getFloatField(_T("clipOrder"));
				existingDesc = q.getStringField(_T("mText"));
				newOrder = existingMaxOrder + 1;
			}
		}
		else
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT clipGroupOrder, mText FROM Main WHERE lParentID = %d ORDER BY clipGroupOrder DESC LIMIT 1"), parentId);			
			if(q.eof() == false)
			{
				existingMaxOrder = q.getFloatField(_T("clipGroupOrder"));
				newOrder = existingMaxOrder + 1;
			}
		}

		Log(StrF(_T("GetNewOrder, Id: %d, parentId: %d, CurrentMax: %f, CurrentDesc: %s, NewMax: %f"), clipId, parentId, existingMaxOrder, existingDesc, newOrder));
	}
	CATCH_SQLITE_EXCEPTION

	return newOrder;
}

double CClip::GetNewLastOrder(int parentId, int clipId)
{
	double newOrder = 0;
	double existingMinOrder = 0;
	CString existingDesc = _T("");

	try
	{
		if (parentId < 0)
		{
			CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT clipOrder, mText FROM Main where clipOrder notnull ORDER BY clipOrder ASC LIMIT 1"));
			if (q.eof() == false)
			{
				existingMinOrder = q.getFloatField(_T("clipOrder"));
				existingDesc = q.getStringField(_T("mText"));
				newOrder = existingMinOrder - 1;
			}
		}
		else
		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT clipGroupOrder, mText FROM Main WHERE lParentID = %d AND clipGroupOrder notnull ORDER BY clipGroupOrder ASC LIMIT 1"), parentId);
			if (q.eof() == false)
			{
				existingMinOrder = q.getFloatField(_T("clipGroupOrder"));
				newOrder = existingMinOrder - 1;
			}
		}

		Log(StrF(_T("GetLastOrder, Id: %d, parentId: %d, CurrentMin: %f, CurrentDesc: %s, NewMax: %f"), clipId, parentId, existingMinOrder, existingDesc, newOrder));
	}
	CATCH_SQLITE_EXCEPTION

		return newOrder;
}

BOOL CClip::LoadMainTable(int id)
{
	bool bRet = false;
	try
	{
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT * FROM Main WHERE lID = %d"), id);

		if(q.eof() == false)
		{
			m_Time = q.getIntField(_T("lDate"));
			m_Desc = q.getStringField(_T("mText"));
			m_CRC = q.getIntField(_T("CRC"));
			m_parentId = q.getIntField(_T("lParentID"));
			m_dontAutoDelete = q.getIntField(_T("lDontAutoDelete"));
			m_shortCut = q.getIntField(_T("lShortCut"));
			m_bIsGroup = q.getIntField(_T("bIsGroup"));
			m_csQuickPaste = q.getStringField(_T("QuickPasteText"));
			m_clipOrder = q.getFloatField(_T("clipOrder"));
			m_clipGroupOrder = q.getFloatField(_T("clipGroupOrder"));
			m_globalShortCut = q.getIntField(_T("globalShortCut"));
			m_lastPasteDate = q.getIntField(_T("lastPasteDate"));
			m_stickyClipOrder = q.getFloatField(_T("stickyClipOrder"));
			m_stickyClipGroupOrder = q.getFloatField(_T("stickyClipGroupOrder"));
			m_moveToGroupShortCut = q.getIntField(_T("MoveToGroupShortCut"));
			m_globalMoveToGroupShortCut = q.getIntField(_T("GlobalMoveToGroupShortCut"));

			m_id = id;

			bRet = true;
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(FALSE)

	return bRet;
}

// STATICS

// Allocates a Global containing the requested Clip Format Data
HGLOBAL CClip::LoadFormat(int id, UINT cfType)
{
	HGLOBAL hGlobal = 0;
	try
	{
		CString csSQL;
		
		csSQL.Format(
			_T("SELECT Data.ooData FROM Data ")
			_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
			_T("WHERE Main.lID = %d ")
			_T("AND Data.strClipBoardFormat = \'%s\'"),
			id,
			GetFormatName(cfType));

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

		if(q.eof() == false)
		{
			int nDataLen = 0;
			const unsigned char *cData = q.getBlobField(0, nDataLen);
			if(cData == NULL)
			{
				return false;
			}

			hGlobal = NewGlobalP((LPVOID)cData, nDataLen);
		}
	}
	CATCH_SQLITE_EXCEPTION
		
	return hGlobal;
}

bool CClip::LoadFormats(int id, bool bOnlyLoad_CF_TEXT, bool includeRichTextForTextOnly)
{
	DWORD startTick = GetTickCount();
	CClipFormat cf;
	HGLOBAL hGlobal = 0;
	m_Formats.RemoveAll();

	try
	{	
		//Open the data table for all that have the parent id

		//Order by Data.lID so that when generating CRC it's always in the same order as the first time
		//we generated it
		CString csSQL;

		CString textFilter = _T("");
		if(bOnlyLoad_CF_TEXT)
		{
			textFilter = _T("(strClipBoardFormat = 'CF_TEXT' OR strClipBoardFormat = 'CF_UNICODETEXT' OR strClipBoardFormat = 'CF_HDROP'");

			if(includeRichTextForTextOnly)
			{
				textFilter = textFilter + _T(" OR strClipBoardFormat = 'Rich Text Format') AND ");
			}
			else
			{
				textFilter = textFilter + _T(") AND ");
			}
		}

		csSQL.Format(
			_T("SELECT lID, lParentID, strClipBoardFormat, ooData FROM Data ")
			_T("WHERE %s lParentID = %d ORDER BY Data.lID desc"), textFilter, id);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

		while(q.eof() == false)
		{
			cf.m_dataId = q.getIntField(_T("lID"));
			cf.m_parentId = q.getIntField(_T("lParentID"));
			cf.m_cfType = GetFormatID(q.getStringField(_T("strClipBoardFormat")));
			
			if(bOnlyLoad_CF_TEXT)
			{
				if(cf.m_cfType != CF_TEXT && 
					cf.m_cfType != CF_UNICODETEXT &&
					cf.m_cfType != CF_HDROP &&
					(cf.m_cfType != theApp.m_RTFFormat && !includeRichTextForTextOnly))
				{
					q.nextRow();
					continue;
				}
			}

			int nDataLen = 0;
			const unsigned char *cData = q.getBlobField(_T("ooData"), nDataLen);
			if(cData != NULL)
			{
				hGlobal = NewGlobalP((LPVOID)cData, nDataLen);
			}
			
			cf.m_hgData = hGlobal;
			m_Formats.Add(cf);

			q.nextRow();
		}

		// formats owns all the data
		cf.m_hgData = 0;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
		
	DWORD endTick = GetTickCount();
	if((endTick-startTick) > 150)
		Log(StrF(_T("Paste Timing LoadFormats: %d, ClipId: %d"), endTick-startTick, id));

	return m_Formats.GetSize() > 0;
}

void CClip::LoadTypes(int id, CClipTypes& types)
{
	types.RemoveAll();
	try
	{
		CString csSQL;
		// get formats for Clip "lID" (Main.lID) using the corresponding Main.lDataID
		
		//Order by Data.lID so that when generating CRC it's always in the same order as the first time
		//we generated it
		csSQL.Format(
			_T("SELECT strClipBoardFormat FROM Data ")
			_T("INNER JOIN Main ON Main.lID = Data.lParentID ")
			_T("WHERE Main.lID = %d ORDER BY Data.lID desc"), id);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);			

		while(q.eof() == false)
		{		
			types.Add(GetFormatID(q.getStringField(0)));
			q.nextRow();
		}
	}
	CATCH_SQLITE_EXCEPTION
}

bool CClip::SaveFromEditWnd(BOOL bUpdateDesc)
{
	bool bRet = false;

	try
	{
		theApp.m_db.execDMLEx(_T("DELETE FROM Data WHERE lParentID = %d;"), m_id);

		DWORD CRC = GenerateCRC();

		AddToDataTable();

		theApp.m_db.execDMLEx(_T("UPDATE Main SET CRC = %d WHERE lID = %d"), CRC, m_id);
		
		if(bUpdateDesc)
		{
			m_Desc.Replace(_T("'"), _T("''"));
			theApp.m_db.execDMLEx(_T("UPDATE Main SET mText = '%s' WHERE lID = %d"), m_Desc, m_id);
		}

		bRet = true;
	}
	CATCH_SQLITE_EXCEPTION

	return bRet;
}

CStringW CClip::GetUnicodeTextFormat()
{
	IClipFormat *pFormat = this->Clips()->FindFormatEx(CF_UNICODETEXT);
	if(pFormat != NULL)
	{
		wchar_t *stringData = (wchar_t *)GlobalLock(pFormat->Data());
		if(stringData != NULL)
		{
			CStringW string(stringData);

			GlobalUnlock(pFormat->Data());

			return string;
		}
	}

	return _T("");
}

CStringA CClip::GetCFTextTextFormat()
{
	IClipFormat *pFormat = this->Clips()->FindFormatEx(CF_TEXT);
	if(pFormat != NULL)
	{
		char *stringData = (char *)GlobalLock(pFormat->Data());
		if(stringData != NULL)
		{
			CStringA string(stringData);
			
			GlobalUnlock(pFormat->Data());

			return string;
		}
	}

	return _T("");
}

BOOL CClip::WriteTextToFile(CString path, BOOL unicode, BOOL asci, BOOL utf8)
{
	BOOL ret = false;

	CFile f;
	if(f.Open(path, CFile::modeWrite|CFile::modeCreate))
	{
		CStringW w = GetUnicodeTextFormat();
		CStringA a = GetCFTextTextFormat();
		
		if(utf8 && w != _T(""))
		{
			CStringA convToUtf8;
			CTextConvert::ConvertToUTF8(w, convToUtf8);
			byte header[2];
			header[0] = 0xEF;
			header[1] = 0xBB;
			f.Write(&header, 2);
			f.Write(convToUtf8.GetBuffer(), convToUtf8.GetLength());

			ret = true;
		}
		else if(unicode && w != _T(""))
		{
			byte header[2];
			header[0] = 0xFF;
			header[1] = 0xFE;
			f.Write(&header, 2);
			f.Write(w.GetBuffer(), w.GetLength() * sizeof(wchar_t));

			ret = true;
		}
		else if(asci && a != _T(""))
		{
			f.Write(a.GetBuffer(), a.GetLength());

			ret = true;
		}

		f.Close();
	}

	return ret;
}

BOOL CClip::WriteImageToFile(CString path)
{
	BOOL ret = false;

	CClipFormat *bitmap = this->m_Formats.FindFormat(CF_DIB);
	if (bitmap)
	{
		LPVOID pvData = GlobalLock(bitmap->m_hgData);
		ULONG size = (ULONG)GlobalSize(bitmap->m_hgData);

		BITMAPINFO *lpBI = (BITMAPINFO *)pvData;

		int nPaletteEntries = 1 << lpBI->bmiHeader.biBitCount;
		if (lpBI->bmiHeader.biBitCount > 8)
			nPaletteEntries = 0;
		else if (lpBI->bmiHeader.biClrUsed != 0)
			nPaletteEntries = lpBI->bmiHeader.biClrUsed;

		BITMAPFILEHEADER BFH;
		memset(&BFH, 0, sizeof(BITMAPFILEHEADER));
		BFH.bfType = 'MB';
		BFH.bfSize = sizeof(BITMAPFILEHEADER) + size;
		BFH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nPaletteEntries * sizeof(RGBQUAD);

		// Create stream with 0 size
		IStream* pIStream = NULL;
		if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pIStream) == S_OK)
		{

			//write the file to the stream object
			pIStream->Write(&BFH, sizeof(BITMAPFILEHEADER), NULL);
			pIStream->Write(pvData, size, NULL);

			CImage i;
			i.Load(pIStream);

			if (i.Save(path) == S_OK)
			{
				ret = true;
			}

			pIStream->Release();
		}
	}
	else
	{
		CClipFormat *png = this->m_Formats.FindFormat(theApp.m_PNG_Format);
		if (png)
		{
			IStream* pIStream = NULL;
			if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pIStream) == S_OK)
			{
				LPVOID pvData = GlobalLock(png->m_hgData);
				ULONG size = (ULONG)GlobalSize(png->m_hgData);

				pIStream->Write(pvData, size, NULL);

				GlobalUnlock(png->m_hgData);

				CImage i;
				i.Load(pIStream);

				if (i.Save(path) == S_OK)
				{
					ret = true;
				}

				pIStream->Release();
			}
		}
	}
	return ret;
}

bool CClip::AddFileDataToData(CString &errorMessage)
{
	INT_PTR size = m_Formats.GetSize();
	if (size <= 0)
	{
		errorMessage = _T("No CF_HDROP formats to convert");
		return false;
	}

	bool addedFileData = false;

	int nCF_HDROPIndex = -1;
	int dittoDataIndex = -1;
	for (int i = 0; i < size; i++)
	{
		if (m_Formats[i].m_cfType == CF_HDROP)
		{
			nCF_HDROPIndex = i;
		}
		else if(m_Formats[i].m_cfType == theApp.m_DittoFileData)
		{
			dittoDataIndex = i;
		}
	}	

	if (nCF_HDROPIndex < 0)
	{
		errorMessage = _T("No CF_HDROP formats to convert");
		return false;
	}
	else if (dittoDataIndex >= 0)
	{
		return false;
	}
	else
	{
		using namespace nsPath;

		HDROP drop = (HDROP)GlobalLock(m_Formats[nCF_HDROPIndex].m_hgData);
		int nNumFiles = DragQueryFile(drop, -1, NULL, 0);
		
		TCHAR filePath[MAX_PATH];

		CString newDesc = _T("File Contents - ");
				
		for (int nFile = 0; nFile < nNumFiles; nFile++)
		{
			if (DragQueryFile(drop, nFile, filePath, sizeof(filePath)) > 0)
			{
				CFile file;
				CFileException ex;
				if (file.Open(filePath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone, &ex))
				{
					int fileSize = (int)file.GetLength();
					int maxSize = CGetSetOptions::GetMaxFileContentsSize();
					if (fileSize < maxSize)
					{
						CString src(filePath);
						CStringA csFilePath;
						CTextConvert::ConvertToUTF8(src, csFilePath);
						
						int bufferSize = (int)fileSize + csFilePath.GetLength() + 1 + md5StringLength + 1;;
						char *pBuffer = new char[bufferSize];
						if (pBuffer != NULL)
						{
							//data contents
							//original file<null terminator>md5<null terminator>file data

							memset(pBuffer, 0, bufferSize);
							strncpy(pBuffer, csFilePath, csFilePath.GetLength());
						
							//move the buffer start past the file path and md5 string
							char *bufferStart = pBuffer + csFilePath.GetLength() + 1 + md5StringLength + 1;

							int readBytes = (int)file.Read(bufferStart, fileSize);

							CMd5 md5;
							CStringA md5String = md5.CalcMD5FromString(bufferStart, fileSize);

							char *bufferMd5 = pBuffer + csFilePath.GetLength() + 1;
							strncpy(bufferMd5, md5String, md5StringLength);

							AddFormat(theApp.m_DittoFileData, pBuffer, bufferSize);

							addedFileData = true;

							newDesc += filePath;
							newDesc += _T("\n");

							Log(StrF(_T("Saving file contents to Ditto Database, file: %s, size: %d, md5: %s"), filePath, fileSize, md5String));
						}
					}
					else
					{
						const int MAX_FILE_SIZE_BUFFER = 255;
						TCHAR szFileSize[MAX_FILE_SIZE_BUFFER];
						TCHAR szMaxFileSize[MAX_FILE_SIZE_BUFFER];
						StrFormatByteSize(fileSize, szFileSize, MAX_FILE_SIZE_BUFFER);
						StrFormatByteSize(maxSize, szMaxFileSize, MAX_FILE_SIZE_BUFFER);

						errorMessage += StrF(_T("File is to large: %s, Size: %s, Max Size: %s\r\n"), filePath, szFileSize, szMaxFileSize);
					}
				}
				else
				{
					TCHAR szError[200];
					ex.GetErrorMessage(szError, 200);
					errorMessage += StrF(_T("Error opening file: %s, Error: %s\r\n"), filePath, szError);
				}
			}
		}

		GlobalUnlock(m_Formats[nCF_HDROPIndex].m_hgData);

		if (addedFileData)
		{
			for (int i = 0; i < size; i++)
			{
				this->m_Formats.RemoveAt(i, 1);
			}

			this->m_Desc = newDesc;

			if (this->ModifyDescription())
			{
				if (this->AddToDataTable() == FALSE)
				{
					errorMessage += _T("Error saving data to database.");
				}
			}
			else
			{
				errorMessage += _T("Error saving main table to database.");
			}
		}
	}

	return addedFileData;
}

Gdiplus::Bitmap *CClip::CreateGdiplusBitmap()
{
	Gdiplus::Bitmap *gdipBitmap = NULL;

	CClipFormat *png = this->m_Formats.FindFormat(GetFormatID(_T("PNG")));
	if (png != NULL)
	{
		gdipBitmap = png->CreateGdiplusBitmap();
	}
	else
	{
		CClipFormat *dib = this->m_Formats.FindFormat(CF_DIB);
		if (dib != NULL)
		{
			gdipBitmap = dib->CreateGdiplusBitmap();
		}
	}

	return gdipBitmap;
}

/*----------------------------------------------------------------------------*\
CClipList
\*----------------------------------------------------------------------------*/

CClipList::~CClipList()
{
	CClip* pClip;
	while(GetCount())
	{
		pClip = RemoveHead();
		delete pClip;
	}
}

// returns the number of clips actually saved
// while this does empty the Format Data, it does not delete the Clips.
int CClipList::AddToDB(bool bLatestOrder)
{
	Log(_T("AddToDB - Start"));

	int savedCount = 0;
	CClip* pClip;
	POSITION pos;
	bool bResult;
	
	INT_PTR remaining = GetCount();
	pos = GetHeadPosition();
	while(pos)
	{
		Log(StrF(_T("AddToDB - while(pos), Start Remaining %d"), remaining));
		remaining--;
		
		pClip = GetNext(pos);
		ASSERT(pClip);
		
		if(bLatestOrder)
		{
			pClip->MakeLatestOrder();
			pClip->MakeLatestGroupOrder();
		}

		pClip->m_Time = CTime::GetCurrentTime().GetTime();

		bResult = pClip->AddToDB();
		if(bResult)
		{
			savedCount++;
		}

		Log(StrF(_T("AddToDB - while(pos), End Remaining %d, save count: %d"), remaining, savedCount));
	}

	Log(StrF(_T("AddToDB - Start, count: %d"), savedCount));
	
	return savedCount;
}

const CClipList& CClipList::operator=(const CClipList &cliplist)
{
	POSITION pos;
	CClip* pClip;
	
	pos = cliplist.GetHeadPosition();
	while(pos)
	{
		pClip = cliplist.GetNext(pos);
		ASSERT(pClip);

		CClip *pNewClip = new CClip;
		if(pNewClip)
		{
			*pNewClip = *pClip;
			
			AddTail(pNewClip);
		}
	}
	
	return *this;
}
