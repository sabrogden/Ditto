// AutoRichEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "RichEditCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoRichEditCtrl

CRichEditCtrlEx::CRichEditCtrlEx()
{
}

CRichEditCtrlEx::~CRichEditCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CRichEditCtrlEx, CRichEditCtrl)
	//{{AFX_MSG_MAP(CRichEditCtrlEx)
		// NOTE - the ClassWizard will add and remove mapping macros here.
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

	es.dwCookie = (DWORD) &sRTF;	// so sRTF receives the string
	
	StreamOut(SF_RTF, es);			// Call CRichEditCtrl::StreamOut to get the string.
	///

	return sRTF;

}

void CRichEditCtrlEx::SetRTF(CString sRTF)
{
	// Put the RTF string sRTF into the rich edit control.

	// Read the text in
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamIn;
	es.dwCookie = (DWORD) &sRTF;
	StreamIn(SF_RTF, es);	// Do it.
	
}

CString CRichEditCtrlEx::GetText()
{
	// Return the RTF string of the text in the control.
	
	// Stream out here.
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamOut;		// Set the callback

	CString sText = "";

	es.dwCookie = (DWORD) &sText;	// so sRTF receives the string
	
	StreamOut(SF_TEXT, es);			// Call CRichEditCtrl::StreamOut to get the string.

	return sText;

}

void CRichEditCtrlEx::SetText(CString sText)
{
	// Put the RTF string sRTF into the rich edit control.

	// Read the text in
	EDITSTREAM es;
	es.dwError = 0;
	es.pfnCallback = CBStreamIn;
	es.dwCookie = (DWORD) &sText;
	StreamIn(SF_TEXT, es);	// Do it.
}

/*
	Callback function to stream an RTF string into the rich edit control.
*/
DWORD CALLBACK CRichEditCtrlEx::CBStreamIn(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
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
		memcpy(pbBuff, (LPCSTR) *pstr, *pcb);
		pstr->Empty();
	}
	else
	{
		*pcb = cb;
		memcpy(pbBuff, (LPCSTR) *pstr, *pcb);
		*pstr = pstr->Right(pstr->GetLength() - cb);
	}
	///

	return 0;
}

/*
	Callback function to stream the RTF string out of the rich edit control.
*/
DWORD CALLBACK CRichEditCtrlEx::CBStreamOut(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
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
		cf.szFaceName[i] = sFontName[i];


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
