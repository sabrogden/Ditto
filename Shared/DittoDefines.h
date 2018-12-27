#pragma once

#include <shlwapi.h>

#define DITTO_ADD_IN_VERSION 1

typedef enum
{
	eFuncType_PRE_PASTE
}FunctionType;

class CFunction
{
public:
	CStringA m_csFunction;
	CString m_csDisplayName;
	CString m_csDetailDescription;
};

class CDittoAddinInfo
{
public:
	CDittoAddinInfo()
	{
		m_nPrivateVersion = DITTO_ADD_IN_VERSION;
		m_AddinVersion = 0;
		m_nSizeOfThis = sizeof(CDittoAddinInfo);
	}
	
	bool ValidateSize() const  { return m_nSizeOfThis == sizeof(CDittoAddinInfo); }
	int PrivateVersion() const { return m_nPrivateVersion; }

	int m_nSizeOfThis;
	CString m_Name;
	int m_AddinVersion;

private:
	int m_nPrivateVersion;
};

class CDittoInfo
{
public:
	CDittoInfo()
	{
		m_nVersion = 0;
		m_hWndDitto = NULL;
		m_nSizeOfThis = sizeof(CDittoInfo);
	}

	bool ValidateSize() const  { return m_nSizeOfThis == sizeof(CDittoInfo); }
	int PrivateVersion() const { return m_nPrivateVersion; }

	int m_nSizeOfThis;
	int m_nVersion;
	CString m_csSqliteVersion;
	CString m_csLanguageCode; //http://www.loc.gov/standards/iso639-2/php/code_list.php
	CString m_csDatabasePath;
	HWND m_hWndDitto;

private:
	int m_nPrivateVersion;
};

class DittoAddinHelpers
{
public:
	static void CopyToGlobalHP(HGLOBAL hDest, LPVOID pBuf, ULONG ulBufLen)
	{
		ASSERT(hDest && pBuf && ulBufLen);
		LPVOID pvData = GlobalLock(hDest);
		ASSERT(pvData);
		ULONG size = (ULONG)GlobalSize(hDest);
		ASSERT(size >= ulBufLen);	// assert if hDest isn't big enough
		memcpy(pvData, pBuf, ulBufLen);
		GlobalUnlock(hDest);
	}

	static HGLOBAL NewGlobalP(LPVOID pBuf, UINT nLen)
	{
		ASSERT(pBuf && nLen);
		HGLOBAL hDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, nLen);
		ASSERT(hDest);
		CopyToGlobalHP(hDest, pBuf, nLen);
		return hDest;
	}


	//Do not change these these are stored in the database
	static CLIPFORMAT GetFormatID(LPCTSTR cbName)
	{
		if(StrCmp(cbName, _T("CF_TEXT")) == 0)
			return CF_TEXT;
		else if(StrCmp(cbName, _T("CF_METAFILEPICT")) == 0)
			return CF_METAFILEPICT;
		else if(StrCmp(cbName, _T("CF_SYLK")) == 0)
			return CF_SYLK;
		else if(StrCmp(cbName, _T("CF_DIF")) == 0)
			return CF_DIF;
		else if(StrCmp(cbName, _T("CF_TIFF")) == 0)
			return CF_TIFF;
		else if(StrCmp(cbName, _T("CF_OEMTEXT")) == 0)
			return CF_OEMTEXT;
		else if(StrCmp(cbName, _T("CF_DIB")) == 0)
			return CF_DIB;
		else if(StrCmp(cbName, _T("CF_PALETTE")) == 0)
			return CF_PALETTE;
		else if(StrCmp(cbName, _T("CF_PENDATA")) == 0)
			return CF_PENDATA;
		else if(StrCmp(cbName, _T("CF_RIFF")) == 0)
			return CF_RIFF;
		else if(StrCmp(cbName, _T("CF_WAVE")) == 0)
			return CF_WAVE;
		else if(StrCmp(cbName, _T("CF_UNICODETEXT")) == 0)
			return CF_UNICODETEXT;
		else if(StrCmp(cbName, _T("CF_ENHMETAFILE")) == 0)
			return CF_ENHMETAFILE;
		else if(StrCmp(cbName, _T("CF_HDROP")) == 0)
			return CF_HDROP;
		else if(StrCmp(cbName, _T("CF_LOCALE")) == 0)
			return CF_LOCALE;
		else if(StrCmp(cbName, _T("CF_OWNERDISPLAY")) == 0)
			return CF_OWNERDISPLAY;
		else if(StrCmp(cbName, _T("CF_DSPTEXT")) == 0)
			return CF_DSPTEXT;
		else if(StrCmp(cbName, _T("CF_DSPBITMAP")) == 0)
			return CF_DSPBITMAP;
		else if(StrCmp(cbName, _T("CF_DSPMETAFILEPICT")) == 0)
			return CF_DSPMETAFILEPICT;
		else if(StrCmp(cbName, _T("CF_DSPENHMETAFILE")) == 0)
			return CF_DSPENHMETAFILE;

		return ::RegisterClipboardFormat(cbName);
	}

	//Do not change these these are stored in the database
	static CString GetFormatName(CLIPFORMAT cbType)
	{
		switch(cbType)
		{
		case CF_TEXT:
			return _T("CF_TEXT");
		case CF_BITMAP:
			return _T("CF_BITMAP");
		case CF_METAFILEPICT:
			return _T("CF_METAFILEPICT");
		case CF_SYLK:
			return _T("CF_SYLK");
		case CF_DIF:
			return _T("CF_DIF");
		case CF_TIFF:
			return _T("CF_TIFF");
		case CF_OEMTEXT:
			return _T("CF_OEMTEXT");
		case CF_DIB:
			return _T("CF_DIB");
		case CF_PALETTE:
			return _T("CF_PALETTE");
		case CF_PENDATA:
			return _T("CF_PENDATA");
		case CF_RIFF:
			return _T("CF_RIFF");
		case CF_WAVE:
			return _T("CF_WAVE");
		case CF_UNICODETEXT:
			return _T("CF_UNICODETEXT");
		case CF_ENHMETAFILE:
			return _T("CF_ENHMETAFILE");
		case CF_HDROP:
			return _T("CF_HDROP");
		case CF_LOCALE:
			return _T("CF_LOCALE");
		case CF_OWNERDISPLAY:
			return _T("CF_OWNERDISPLAY");
		case CF_DSPTEXT:
			return _T("CF_DSPTEXT");
		case CF_DSPBITMAP:
			return _T("CF_DSPBITMAP");
		case CF_DSPMETAFILEPICT:
			return _T("CF_DSPMETAFILEPICT");
		case CF_DSPENHMETAFILE:
			return _T("CF_DSPENHMETAFILE");
		default:
			//Not a default type get the name from the clipboard
			if (cbType != 0)
			{
				TCHAR szFormat[256];
				GetClipboardFormatName(cbType, szFormat, 256);
				return szFormat;
			}
			break;
		}

		return _T("ERROR");
	}
};

class AddToDbStickyEnum
{
public:
	enum AddToDbSticky
	{
		INVALID,
		MAKE_TOP_STICKY,
		MAKE_LAST_STICKY,
		REPLACE_TOP_STICKY
	};
};