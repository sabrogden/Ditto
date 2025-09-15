#include "stdafx.h"
#include "clip.h"
#include "CP_Main.h"
#include "..\shared\TextConvert.h"
#include ".\dittorulerricheditctrl.h"
#include "CopyProperties.h"

CDittoRulerRichEditCtrl::CDittoRulerRichEditCtrl(void)
{	
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
		m_rtf.SetModify(FALSE);
		return false;
	}

	Clip.m_cfType = RegisterClipboardFormat(CF_RTF);
	if(theApp.GetClipData(lID, Clip) && Clip.m_hgData)
	{
		CString cs(Clip.GetAsCStringA());
		SetRTF(cs);
		bSetText = true;		

		Clip.Free();
		Clip.Clear();
	}

	if(bSetText == false)
	{
		Clip.m_cfType = CF_UNICODETEXT;
		if(theApp.GetClipData(lID, Clip) && Clip.m_hgData)
		{
			SetText(Clip.GetAsCString());
			bSetText = true;		

			Clip.Free();
			Clip.Clear();
		}
	}

	if(bSetText == false)
	{
		Clip.m_cfType = CF_TEXT;
		if(theApp.GetClipData(lID, Clip) && Clip.m_hgData)
		{
			CString csText(Clip.GetAsCStringA());
			SetText(csText);

			bSetText = true;
		
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

	if(m_rtf.GetModify() == FALSE)
	{
		Log(_T("Clip has not been modified"));
		return DIDNT_NEED_TO_SAVE;
	}

	bool bSetModifyToFalse = true;
	try
	{
		//only save the types if they have them set as save types, mainly rtf type
		int saveTypes = 0;
		CClipTypes* pTypes = theApp.LoadTypesFromDB();

		INT_PTR numTypes = pTypes->GetSize();
		for (int i = 0; i < numTypes; i++)
		{
			if (pTypes->ElementAt(i) == theApp.m_RTFFormat)
			{
				saveTypes |= stRTF;
			}
			else if (pTypes->ElementAt(i) == CF_TEXT ||
				pTypes->ElementAt(i) == CF_UNICODETEXT)
			{
				saveTypes |= stCF_TEXT;
				saveTypes |= stCF_UNICODETEXT;
			}
		}

		CClip Clip;
		Clip.m_id = m_lID;
		if(saveTypes & stRTF)
		{
			LoadRTFData(Clip);
		}

		if(saveTypes & stCF_TEXT || saveTypes & stCF_UNICODETEXT)
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
			Clip.MakeLatestOrder();
			CCopyProperties Prop(-1, this, &Clip);
			Prop.SetHandleKillFocus(true);
			Prop.SetToTopMost(false);
			if(Prop.DoModal() == IDOK)
			{
				Clip.AddToDB();
				m_csDescription = Clip.m_Desc;
				m_lID = Clip.m_id;
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

	//remove the line feed at the end, not sure why the righ text always adds this
	CString right = csRTFOriginal.Right(9);
	if (right == _T("\\par\r\n}\r\n"))
	{
		CString r = csRTFOriginal.Left(csRTFOriginal.GetLength() - 9);
		r += _T("}");
		csRTFOriginal = r;
	}

	CStringA csRTF = CTextConvert::UnicodeToAnsi(csRTFOriginal);
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

	Clip.SetDescFromText(format.m_hgData, true);
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