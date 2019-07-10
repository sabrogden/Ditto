#pragma once

#include "tinyxml\Tinyxml.h"
#include "tinyxml\tinystr.h"

class CTheme
{
public:
	CTheme(void);
	~CTheme(void);

	bool Load(CString csTheme, bool bHeaderOnly = false, bool bCheckLastWriteTime = false);	

	COLORREF CaptionLeft() const { return m_CaptionLeft; }
	COLORREF CaptionRight() const { return m_CaptionRight; }
	COLORREF Border() const { return m_Border; }
	COLORREF BorderTopMost() const { return m_BorderTopMost; }
	COLORREF BorderNotConnected() const { return m_BorderNotConnected; }
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
	COLORREF ClipPastedColor() const { return m_clipPastedColor; }

	COLORREF ListSmallQuickPasteIndexColor() const { return m_listSmallQuickPasteIndexColor;  }
	COLORREF MainWindowBG() const { return m_mainWindowBG; }
	COLORREF SearchTextBoxFocusBG() const { return m_searchTextBoxFocusBG; }
	COLORREF SearchTextBoxFocusText() const { return m_searchTextBoxFocusText; }
	COLORREF SearchTextBoxFocusBorder() const { return m_searchTextBoxFocusBorder; }
	COLORREF SearchTextHighlight() const { return m_searchTextHighlight; }

	COLORREF GroupTreeBG() const { return m_groupTreeBG; }
	COLORREF GroupTreeText() const { return m_groupTreeText; }

	int GetCaptionSize() const { return m_captionSize; }
	int GetCaptionFontSize() const { return m_captionFontSize; }

	COLORREF DescriptionWindowBG() const { return m_descriptionWindowBG; }
	COLORREF DescriptionWindowText() const { return m_descriptionWindowText; }

	CString Notes() const { return m_csNotes; }
	CString Author() const { return m_csAuthor; }
	long FileVersion() const { return m_lFileVersion; }

	CString LastError() const { return m_csLastError; }

protected:
	bool LoadElement(TiXmlElement *pParent, CStringA csNode, COLORREF &Color, int &intValue);

	bool LoadInt(TiXmlElement *pParent, CStringA csNode, int &intValue);
	bool LoadColor(TiXmlElement *pParent, CStringA csNode, COLORREF &Color);
	void LoadWindowsAccentColor();

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
	COLORREF m_clipPastedColor;
	COLORREF m_listSmallQuickPasteIndexColor;
	COLORREF m_mainWindowBG;
	COLORREF m_Border;
	COLORREF m_BorderTopMost;
	COLORREF m_BorderNotConnected;
	COLORREF m_searchTextBoxFocusBG;
	COLORREF m_searchTextBoxFocusText;
	COLORREF m_searchTextBoxFocusBorder;
	COLORREF m_searchTextHighlight;

	COLORREF m_groupTreeBG;
	COLORREF m_groupTreeText;

	COLORREF m_descriptionWindowBG;
	COLORREF m_descriptionWindowText;

	int m_captionSize;
	int m_captionFontSize;

	CString m_csLastError;
	long m_lFileVersion;
	CString m_csAuthor;
	CString m_csNotes;

	__int64 m_LastWriteTime;
	CString m_lastTheme;

	void LoadDefaults();
};
