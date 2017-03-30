// AutoRichEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "RichEditCtrlEx.h"
#include "shared/TextConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoRichEditCtrl

_AFX_RICHEDITEX_STATE _afxRichEditStateEx ;

BOOL PASCAL AfxInitRichEditEx()
{
    if( ! ::AfxInitRichEdit() )
    {
        return FALSE ;
    }

    _AFX_RICHEDITEX_STATE* l_pState = &_afxRichEditStateEx ;

    if( l_pState->m_hInstRichEdit20 == NULL )
    {
#ifdef _UNICODE 
        l_pState->m_hInstRichEdit20 = LoadLibraryW(_T("MSFTEDIT.DLL"));
#else
		l_pState->m_hInstRichEdit20 = LoadLibraryA(_T("RICHED20.DLL"));
#endif

    }

    return l_pState->m_hInstRichEdit20 != NULL ;
}


CRichEditCtrlEx::CRichEditCtrlEx()
{
}

CRichEditCtrlEx::~CRichEditCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CRichEditCtrlEx, CRichEditCtrl)
	//{{AFX_MSG_MAP(CRichEditCtrlEx)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlEx message handlers

CString CRichEditCtrlEx::GetRTF()
{
	// Return the RTF string of the text in the control.
	
	// Stream out here.
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamOut;		// Set the callback

	CString sRTF = "";

	es.dwCookie = (DWORD_PTR) &sRTF;	// so sRTF receives the string
	
	StreamOut(SF_RTF, es);			// Call CRichEditCtrl::StreamOut to get the string.
	///

	return sRTF;

}

void CRichEditCtrlEx::SetRTF(const char *pRTF)
{
	// Put the RTF string sRTF into the rich edit control.

	// Read the text in
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamIn;

#ifdef _UNICODE
	CString cs;
	es.dwCookie = (DWORD_PTR) &cs;
#else
	CString cs(pRTF);
	es.dwCookie = (DWORD_PTR) &cs;
#endif

	StreamIn(SF_RTF, es);	// Do it.

#ifdef _UNICODE
	SETTEXTEX stex;
	stex.flags = ST_SELECTION | ST_KEEPUNDO;
	SendMessage(EM_SETTEXTEX, (WPARAM)&stex, (LPARAM)pRTF); 
#endif

}

void CRichEditCtrlEx::SetRTF(CStringA sRTF)
{
	// Put the RTF string sRTF into the rich edit control.

	// Read the text in
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamIn;

#ifdef _UNICODE
	CString cs;
	es.dwCookie = (DWORD_PTR) &cs;
#else
	es.dwCookie = (DWORD_PTR) &sRTF;
#endif

	StreamIn(SF_RTF, es);	// Do it.

#ifdef _UNICODE
	SETTEXTEX stex;
    stex.flags = ST_SELECTION | ST_KEEPUNDO;

    SendMessage(EM_SETTEXTEX, (WPARAM)&stex, (LPARAM)sRTF.GetBuffer(sRTF.GetLength())); 
#endif

}

CString CRichEditCtrlEx::GetText()
{
	CString sText;
	
#ifdef _UNICODE
	GETTEXTEX stex;
	stex.codepage = 1200;  // Unicode code page(set SETTEXTEX documentation)

	int nSize = GetTextLength();
	//increase the size incase of unicode text
	nSize++;
	nSize = nSize * 2;
	stex.cb = nSize;

	TCHAR *pText = new TCHAR[nSize];
	if(pText)
	{
		SendMessage(EM_GETTEXTEX, (WPARAM)&stex, (LPARAM)pText); 
		sText = pText;

		delete []pText;
		pText = NULL;
	}
#else
	// Stream out here.
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamOut;		// Set the callback
	es.dwCookie = (DWORD_PTR) &sText;	// so sRTF receives the string
	StreamOut(SF_TEXT, es);			// Call CRichEditCtrl::StreamOut to get the string.
#endif

	return sText;
}

void CRichEditCtrlEx::SetText(CString sText)
{
	// Put the RTF string sRTF into the rich edit control.

	// Read the text in
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamIn;
#ifdef _UNICODE
	CString cs;
	es.dwCookie = (DWORD_PTR) &cs;
#else
	es.dwCookie = (DWORD_PTR) &sText;
#endif
	StreamIn(SF_TEXT, es);	// Do it.

#ifdef _UNICODE
	SETTEXTEX stex;
    stex.flags = ST_SELECTION | ST_KEEPUNDO;
    stex.codepage = 1200;  // Unicode code page(set SETTEXTEX documentation)
    SendMessage(EM_SETTEXTEX, (WPARAM)&stex, (LPARAM)sText.GetBuffer(sText.GetLength())); 
	sText.ReleaseBuffer();
#endif
}

/*
	Callback function to stream an RTF string into the rich edit control.
*/
DWORD CALLBACK CRichEditCtrlEx::CBStreamIn(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	// We insert the rich text here.

/*	
	This function taken from CodeGuru.com
	http://www.codeguru.com/richedit/rtf_string_streamin.shtml
	Zafir Anjum
*/

	CString *pstr = (CString *) dwCookie;

	if (pstr->GetLength() < cb)
	{
		*pcb = pstr->GetLength();
		memcpy(pbBuff, (LPCTSTR) *pstr, *pcb);
		pstr->Empty();
	}
	else
	{
		*pcb = cb;
		memcpy(pbBuff, (LPCTSTR) *pstr, *pcb);
		*pstr = pstr->Right(pstr->GetLength() - cb);
	}
	///

	return 0;
}

/*
	Callback function to stream the RTF string out of the rich edit control.
*/
DWORD CALLBACK CRichEditCtrlEx::CBStreamOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	// Address of our string var is in psEntry
	CString *psEntry = (CString*) dwCookie;
	

	CString tmpEntry = "";
	tmpEntry = (CString) pbBuff;

	// And write it!!!
	*psEntry += tmpEntry.Left(cb);

	return 0;
}

bool CRichEditCtrlEx::SelectionIsBold()
{
	CHARFORMAT cf = GetCharFormat();	
	
	if (cf.dwEffects & CFM_BOLD)
		return true;
	else
		return false;
}

bool CRichEditCtrlEx::SelectionIsItalic()
{
	CHARFORMAT cf = GetCharFormat();	
	
	if (cf.dwEffects & CFM_ITALIC)
		return true;
	else
		return false;
}

bool CRichEditCtrlEx::SelectionIsUnderlined()
{
	CHARFORMAT cf = GetCharFormat();	
	
	if (cf.dwEffects & CFM_UNDERLINE)
		return true;
	else
		return false;
}

CHARFORMAT CRichEditCtrlEx::GetCharFormat(DWORD dwMask)
{
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);

	cf.dwMask = dwMask;

	GetSelectionCharFormat(cf);

	return cf;
}

void CRichEditCtrlEx::SetCharStyle(int MASK, int STYLE, int nStart, int nEnd)
{
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	//cf.dwMask = MASK;
	
	GetSelectionCharFormat(cf);
	
	if (cf.dwMask & MASK)	// selection is all the same
	{
		cf.dwEffects ^= STYLE; 
	}
	else
	{
		cf.dwEffects |= STYLE;
	}
	
	cf.dwMask = MASK;

	SetSelectionCharFormat(cf);

}

void CRichEditCtrlEx::SetSelectionBold()
{
	long start=0, end=0;
	GetSel(start, end);		// Get the current selection

	SetCharStyle(CFM_BOLD, CFE_BOLD, start, end);	// Make it bold
}

void CRichEditCtrlEx::SetSelectionItalic()
{
	long start=0, end=0;
	GetSel(start, end);

	SetCharStyle(CFM_ITALIC, CFE_ITALIC, start, end);
}

void CRichEditCtrlEx::SetSelectionUnderlined()
{
	long start=0, end=0;
	GetSel(start, end);

	SetCharStyle(CFM_UNDERLINE, CFE_UNDERLINE, start, end);
}

void CRichEditCtrlEx::SetParagraphCenter()
{
	PARAFORMAT paraFormat;    
	paraFormat.cbSize = sizeof(PARAFORMAT);
	paraFormat.dwMask = PFM_ALIGNMENT;    
	paraFormat.wAlignment = PFA_CENTER;
	
	SetParaFormat(paraFormat);	// Set the paragraph.
}

void CRichEditCtrlEx::SetParagraphLeft()
{
	PARAFORMAT paraFormat;
	paraFormat.cbSize = sizeof(PARAFORMAT);
	paraFormat.dwMask = PFM_ALIGNMENT;    
	paraFormat.wAlignment = PFA_LEFT;
	
	SetParaFormat(paraFormat);
}

void CRichEditCtrlEx::SetParagraphRight()
{
	PARAFORMAT paraFormat;
	paraFormat.cbSize = sizeof(PARAFORMAT);
	paraFormat.dwMask = PFM_ALIGNMENT;    
	paraFormat.wAlignment = PFA_RIGHT;
	
	SetParaFormat(paraFormat);
}

bool CRichEditCtrlEx::ParagraphIsCentered()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wAlignment == PFA_CENTER)
		return true;
	else
		return false;
}

bool CRichEditCtrlEx::ParagraphIsLeft()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wAlignment == PFA_LEFT)
		return true;
	else
		return false;
}

bool CRichEditCtrlEx::ParagraphIsRight()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wAlignment == PFA_RIGHT)
		return true;
	else
		return false;
}

PARAFORMAT CRichEditCtrlEx::GetParagraphFormat()
{
	PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);

	pf.dwMask = PFM_ALIGNMENT | PFM_NUMBERING;    	

	GetParaFormat(pf);

	return pf;
}

void CRichEditCtrlEx::SetParagraphBulleted()
{
	PARAFORMAT paraformat = GetParagraphFormat();

	if ( (paraformat.dwMask & PFM_NUMBERING) && (paraformat.wNumbering == PFN_BULLET) )
	{
		paraformat.wNumbering = 0;
		paraformat.dxOffset = 0;
		paraformat.dxStartIndent = 0;
		paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
	}
	else
	{
		paraformat.wNumbering = PFN_BULLET;
		paraformat.dwMask = PFM_NUMBERING;
		if (paraformat.dxOffset == 0)
		{
			paraformat.dxOffset = 4;
			paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
		}
	}
	
	SetParaFormat(paraformat);

}

bool CRichEditCtrlEx::ParagraphIsBulleted()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wNumbering == PFN_BULLET)
		return true;
	else
		return false;
}

void CRichEditCtrlEx::SelectColor()
{
	CColorDialog dlg;

	CHARFORMAT cf = GetCharFormat();

	if (cf.dwEffects & CFE_AUTOCOLOR) cf.dwEffects -= CFE_AUTOCOLOR;

	// Get a color from the common color dialog.
	if( dlg.DoModal() == IDOK )
	{	
		cf.crTextColor = dlg.GetColor();
	}

	cf.dwMask = CFM_COLOR;

	SetSelectionCharFormat(cf);
}

void CRichEditCtrlEx::SetFontName(CString sFontName)
{
	CHARFORMAT cf = GetCharFormat();

	// Set the font name.
	for (int i = 0; i <= sFontName.GetLength()-1; i++)
		cf.szFaceName[i] = (char)sFontName[i];


	cf.dwMask = CFM_FACE;

	SetSelectionCharFormat(cf);
}

void CRichEditCtrlEx::SetFontSize(int nPointSize)
{
	CHARFORMAT cf = GetCharFormat();

	nPointSize *= 20;	// convert from to twips
	cf.yHeight = nPointSize;
	
	cf.dwMask = CFM_SIZE;

	SetSelectionCharFormat(cf);
}

void CRichEditCtrlEx::GetSystemFonts(CStringArray &saFontList)
{
	CDC *pDC = GetDC ();

	EnumFonts (pDC->GetSafeHdc(),NULL,(FONTENUMPROC) CBEnumFonts,(LPARAM)&saFontList);//Enumerate

}

BOOL CALLBACK CRichEditCtrlEx::CBEnumFonts(LPLOGFONT lplf, LPTEXTMETRIC lptm, DWORD dwType, LPARAM lpData)
{
	// This function was written with the help of CCustComboBox, by Girish Bharadwaj.
	// Available from Codeguru.

	if (dwType == TRUETYPE_FONTTYPE) 
	{
		((CStringArray *) lpData)->Add( lplf->lfFaceName );
	}

	return true;
}

CString CRichEditCtrlEx::GetSelectionFontName()
{
	CHARFORMAT cf = GetCharFormat();

	CString sName = cf.szFaceName;

	return sName;
}

long CRichEditCtrlEx::GetSelectionFontSize()
{
	CHARFORMAT cf = GetCharFormat();

	long nSize = cf.yHeight/20;

	return nSize;
}

int CRichEditCtrlEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CRichEditCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

BOOL CRichEditCtrlEx::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CWnd::Create(_T("RichEdit50W"), lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

}
