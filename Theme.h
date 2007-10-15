#pragma once

#include "Tinyxml.h"
#include "tinystr.h"

class CTheme
{
public:
	CTheme(void);
	~CTheme(void);

	bool Load(CString csTheme, bool bHeaderOnly = false, bool bCheckLastWriteTime = false);

	COLORREF CaptionLeft() const { return m_CaptionLeft; }
	COLORREF CaptionRight() const { return m_CaptionRight; }
	COLORREF CaptionLeftTopMost() const { return m_CaptionLeftTopMost; }
	COLORREF CaptionRightTopMost() const { return m_CaptionRightTopMost; }
	COLORREF CaptionLeftNotConnected() const { return m_CaptionLeftNotConnected; }
	COLORREF CaptionRightNotConnected() const { return m_CaptionRightNotConnected; }
	COLORREF CaptionTextColor() const { return m_CaptionTextColor; }
	
	COLORREF ListBoxOddRowsBG() const { return m_ListBoxOddRowsBG; }
	COLORREF ListBoxEvenRowsBG() const { return m_ListBoxEvenRowsBG; }
	COLORREF ListBoxOddRowsText() const { return m_ListBoxOddRowsText; }
	COLORREF ListBoxEvenRowsText() const { return m_ListBoxEvenRowsText; }
	COLORREF ListBoxSelectedBG() const { return m_ListBoxSelectedBG; }
	COLORREF ListBoxSelectedNoFocusBG() const { return m_ListBoxSelectedNoFocusBG; }
	COLORREF ListBoxSelectedText() const { return m_ListBoxSelectedText; }
	COLORREF ListBoxSelectedNoFocusText() const { return m_ListBoxSelectedNoFocusText; }

	CString Notes() const { return m_csNotes; }
	CString Author() const { return m_csAuthor; }
	long FileVersion() const { return m_lFileVersion; }

	CString LastError() const { return m_csLastError; }

protected:
	bool LoadElement(TiXmlElement *pParent, CStringA csNode, COLORREF &Color);

protected:
	COLORREF m_CaptionLeft;
	COLORREF m_CaptionRight;
	COLORREF m_CaptionLeftTopMost;
	COLORREF m_CaptionRightTopMost;
	COLORREF m_CaptionLeftNotConnected;
	COLORREF m_CaptionRightNotConnected;
	COLORREF m_CaptionTextColor;

	COLORREF m_ListBoxOddRowsBG;
	COLORREF m_ListBoxEvenRowsBG;
	COLORREF m_ListBoxOddRowsText;
	COLORREF m_ListBoxEvenRowsText;
	COLORREF m_ListBoxSelectedBG;
	COLORREF m_ListBoxSelectedNoFocusBG;
	COLORREF m_ListBoxSelectedText;
	COLORREF m_ListBoxSelectedNoFocusText;	

	CString m_csLastError;
	long m_lFileVersion;
	CString m_csAuthor;
	CString m_csNotes;

	__int64 m_LastWriteTime;
};
