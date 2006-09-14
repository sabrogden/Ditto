#include "stdafx.h"
#include "clip.h"
#include "CP_Main.h"
#include "TextConvert.h"
#include ".\dittorulerricheditctrl.h"
#include "CopyProperties.h"

CDittoRulerRichEditCtrl::CDittoRulerRichEditCtrl(void)
{
	m_SaveTypes = stNONE;
	m_lID = -1;
}

CDittoRulerRichEditCtrl::~CDittoRulerRichEditCtrl(void)
{
}

bool CDittoRulerRichEditCtrl::LoadItem(long lID, CString csDesc)
{	
	bool bSetText = false;
	CClipFormat Clip;
	m_lID = lID;
	m_csDescription = csDesc;
	
	//If creating a new clip
	if(m_lID < 0)
	{
		m_SaveTypes = stCF_UNICODETEXT|stRTF;
		m_rtf.SetModify(FALSE);
		return false;
	}

	m_SaveTypes = GetTypeFlags(m_lID);

	if(m_SaveTypes == stNONE)
	{
		CString csFirst = theApp.m_Language.GetString("NotEditable", "Current Item is not editable.  A clip must have one of the clip types");
		CString csSecond = theApp.m_Language.GetString("Editable", "to be editable");

		CString cs;
		cs.Format(_T("%s\n\n")
					_T("- Rich Text\n")
					_T("- CF_TEXT\n")
					_T("- CF_UNICODETEXT\n\n")
					_T("%s."), csFirst, csSecond);
		SetText(cs);
		SetReadOnly(TRUE);
		m_rtf.SetModify(FALSE);
		return false;
	}

	Clip.m_cfType = RegisterClipboardFormat(CF_RTF);
	if(theApp.GetClipData(lID, Clip) && Clip.m_hgData)
	{
		LPVOID pvData = GlobalLock(Clip.m_hgData);
		if(pvData)
		{
			SetRTF((char*)pvData);
			bSetText = true;
		}

		GlobalUnlock(Clip.m_hgData);

		Clip.Free();
		Clip.Clear();
	}

	if(bSetText == false)
	{
		Clip.m_cfType = CF_UNICODETEXT;
		if(theApp.GetClipData(lID, Clip) && Clip.m_hgData)
		{
			LPVOID pvData = GlobalLock(Clip.m_hgData);
			if(pvData)
			{
				CString csText = (WCHAR*)pvData;
				SetText(csText);
				bSetText = true;
			}

			GlobalUnlock(Clip.m_hgData);

			Clip.Free();
			Clip.Clear();
		}
	}

	if(bSetText == false)
	{
		Clip.m_cfType = CF_TEXT;
		if(theApp.GetClipData(lID, Clip) && Clip.m_hgData)
		{
			LPVOID pvData = GlobalLock(Clip.m_hgData);
			if(pvData)
			{
				CString csText = (char*)pvData;
				SetText(csText);

				bSetText = true;
			}

			GlobalUnlock(Clip.m_hgData);

			Clip.Free();
			Clip.Clear();
		}
	}

	m_rtf.SetModify(FALSE);

	return bSetText;
}

long CDittoRulerRichEditCtrl::GetTypeFlags(long lID)
{
	long lRet = stNONE;

	try
	{
		CLIPFORMAT cfType = CF_TEXT;
		CppSQLite3Query q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Data WHERE lParentID = %d AND strClipboardFormat = '%s'"), lID, GetFormatName(cfType));
		if(q.eof() == false)
		{
			lRet |= stCF_TEXT;
		}

		cfType = CF_UNICODETEXT;
		q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Data WHERE lParentID = %d AND strClipboardFormat = '%s'"), lID, GetFormatName(cfType));
		if(q.eof() == false)
		{
			lRet |= stCF_UNICODETEXT;
		}

		cfType = RegisterClipboardFormat(_T("Rich Text Format"));
		q = theApp.m_db.execQueryEx(_T("SELECT lID FROM Data WHERE lParentID = %d AND strClipboardFormat = '%s'"), lID, GetFormatName(cfType));
		if(q.eof() == false)
		{
			lRet |= stRTF;
		}
	}
	CATCH_SQLITE_EXCEPTION

	return lRet;
}

void CDittoRulerRichEditCtrl::d()
{
	CString cs = m_rtf.GetText();
	CString s;
	s.Format(_T("error = %d, %s"), GetLastError(), cs);
	MessageBox(s);
}

int CDittoRulerRichEditCtrl::SaveToDB(BOOL bUpdateDesc)
{
	int nRet = FALSE;
	if(m_SaveTypes == stNONE && m_lID >= 0)
	{
		return DIDNT_NEED_TO_SAVE;
	}

	if(m_rtf.GetModify() == FALSE)
	{
		Log(_T("Clip has not been modified"));
		return DIDNT_NEED_TO_SAVE;
	}

	bool bSetModifyToFalse = true;
	try
	{
		CClip Clip;
		Clip.m_ID = m_lID;
		if(m_SaveTypes & stRTF)
		{
			LoadRTFData(Clip);
		}

		if(m_SaveTypes & stCF_TEXT || m_SaveTypes & stCF_UNICODETEXT)
		{
			LoadTextData(Clip);
		}

		if(Clip.m_Formats.GetSize() <= 0)
		{
			return FALSE;
		}

		theApp.m_db.execDML(_T("begin transaction;"));

		if(m_lID >= 0)
		{
			Clip.SaveFromEditWnd(bUpdateDesc);
		}
		else
		{
			bSetModifyToFalse = false;
			Clip.MakeLatestTime();
			CCopyProperties Prop(-1, this, &Clip);
			Prop.SetHandleKillFocus(true);
			Prop.SetToTopMost(false);
			if(Prop.DoModal() == IDOK)
			{
				Clip.AddToDB();
				m_csDescription = Clip.m_Desc;
				m_lID = Clip.m_ID;
				bUpdateDesc = TRUE;
				bSetModifyToFalse = true;
			}
		}

		nRet = SAVED_CLIP_TO_DB;

		theApp.m_db.execDML(_T("commit transaction;"));

		if(bUpdateDesc)
			theApp.RefreshView();
	}
	CATCH_SQLITE_EXCEPTION

	if(bSetModifyToFalse)
		m_rtf.SetModify(FALSE);

	return nRet;
}

bool CDittoRulerRichEditCtrl::LoadRTFData(CClip &Clip)
{
	CString csRTFOriginal = GetRTF();
	if(csRTFOriginal.IsEmpty())
	{
		Log(_T("Rtf is empty, returning"));
		return false;
	}

	CStringA csRTF = CTextConvert::ConvertToChar(csRTFOriginal);
	CClipFormat format;
	format.m_cfType = RegisterClipboardFormat(_T("Rich Text Format"));
	int nLength = csRTF.GetLength() + 1;
	format.m_hgData = NewGlobalP(csRTF.GetBuffer(nLength), nLength);
	Clip.m_Formats.Add(format);
	format.m_hgData = NULL; //Clip.m_formats owns data now

	return true;
}

bool CDittoRulerRichEditCtrl::LoadTextData(CClip &Clip)
{
	CString csText = GetText();
	if(csText.IsEmpty())
	{
		for(int i = 0; i < 20; i++)
		{
			Sleep(100);
			csText = GetText();
			if(csText.IsEmpty() == FALSE)
				break;

			Log(StrF(_T("Get Text still empty pass = %d"), i));
		}
		if(csText.IsEmpty())
		{
			Log(_T("Get Text still empty pass returning"));
			return false;
		}
	}

	CClipFormat format;

#ifdef _UNICODE
	format.m_cfType = CF_UNICODETEXT;
#else
	format.m_cfType = CF_TEXT;
#endif

	int nLength = csText.GetLength() * sizeof(TCHAR) + sizeof(TCHAR);
	format.m_hgData = NewGlobalP(csText.GetBuffer(nLength), nLength);

	Clip.SetDescFromText(format.m_hgData);
	m_csDescription = Clip.m_Desc;
	m_csDescription = m_csDescription.Left(15);

	Clip.m_Formats.Add(format);
	format.m_hgData = NULL; //Clip.m_formats owns data now

	return true;
}

bool CDittoRulerRichEditCtrl::CloseEdit(bool bPrompt, BOOL bUpdateDesc)
{
	if(m_rtf.GetModify())
	{		
		int nRet = IDYES;
		
		if(bPrompt)
		{
			CString cs;
			cs.Format(_T("%s '%s'"), theApp.m_Language.GetString("SaveChanges", "Do you want to save changes to"), m_csDescription);

			::SetForegroundWindow(m_hWnd);
			nRet = MessageBox(cs, _T("Ditto"), MB_YESNOCANCEL);
		}

		if(nRet == IDYES)
		{
			if(SaveToDB(bUpdateDesc) == false)
			{
				CString cs;
				cs.Format(_T("%s '%s'"), theApp.m_Language.GetString("ErrorSaving", "Error saving clip"), m_csDescription);
				MessageBox(cs, _T("Ditto"), MB_OK);
			}
		}
		else if(nRet == IDCANCEL)
		{
			return false;
		}
	}

	return true;
}