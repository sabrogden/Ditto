#include "stdafx.h"
#include ".\theme.h"
#include "shared/TextConvert.h"
#include "Misc.h"
#include "Options.h"
#include "shared/Tokenizer.h"
#include "CP_Main.h"

CTheme::CTheme(void)
{
	m_lFileVersion = 0;
	m_LastWriteTime = 0;
	m_lastTheme = _T("");

	LoadDefaults();
}

CTheme::~CTheme(void)
{
}


void CTheme::LoadDefaults()
{
	m_CaptionLeft = RGB(255, 255, 255);
	m_CaptionRight = RGB(204, 204, 204);

	m_Border = RGB(204, 204, 204);
	m_BorderTopMost = RGB(100, 100, 100);
	m_BorderNotConnected = RGB(255, 0, 0);

	m_CaptionLeftTopMost = RGB(255, 255, 255);
	m_CaptionRightTopMost = RGB(204, 204, 204);
	m_CaptionLeftNotConnected = RGB(255, 255, 255);
	m_CaptionRightNotConnected = RGB(204, 204, 204);
	m_CaptionTextColor = RGB(191, 191, 191);
	m_ListBoxOddRowsBG = RGB(255, 255, 255);
	m_ListBoxEvenRowsBG = RGB(243, 243, 243);
	m_ListBoxOddRowsText = RGB(0, 0, 0);
	m_ListBoxEvenRowsText = RGB(0, 0, 0);
	m_ListBoxSelectedBG = RGB(204, 204, 204);
	m_ListBoxSelectedNoFocusBG = RGB(204, 204, 204);
	m_ListBoxSelectedText = RGB(0, 0, 0);
	m_ListBoxSelectedNoFocusText = RGB(0, 0, 0);
	m_clipPastedColor = RGB(0, 255, 0);
	m_listSmallQuickPasteIndexColor = RGB(180, 180, 180);
	m_mainWindowBG = RGB(240, 240, 240);
	m_searchTextBoxFocusBG = RGB(255, 255, 255);
	m_searchTextBoxFocusText = RGB(0, 0, 0);

	m_groupTreeBG = RGB(240, 240, 240);
	m_groupTreeText = RGB(127, 127, 127);

	m_descriptionWindowBG = RGB(240, 240, 240);// GetSysColor(COLOR_INFOBK);
	m_descriptionWindowText = RGB(0, 0, 0);

	m_captionSize = 25;
	m_captionFontSize = 18;
}

bool CTheme::Load(CString csTheme, bool bHeaderOnly, bool bCheckLastWriteTime)
{
	if (csTheme.IsEmpty() || csTheme == _T("Ditto"))
	{
		LoadDefaults();

		m_LastWriteTime = 0;
		m_lastTheme = _T("");

		return false;
	}

	CString csPath = CGetSetOptions::GetPath(PATH_THEMES);
	csPath += csTheme;
	csPath += ".xml";

	__int64 LastWrite = GetLastWriteTime(csPath);

	if(bCheckLastWriteTime)
	{	
		if(m_lastTheme == csTheme &&
			LastWrite == m_LastWriteTime)
		{
			return true;
		}
	}

	LoadDefaults();

	m_LastWriteTime = LastWrite;
	m_lastTheme = csTheme;

	Log(StrF(_T("Loading Theme %s"), csPath));

	CStringA csPathA = CTextConvert::ConvertToChar(csPath);

	TiXmlDocument doc(csPathA);
	if(!doc.LoadFile())
	{
		m_csLastError.Format(_T("Error loading Theme %s - reason = %s"), csPath, doc.ErrorDesc());
		ASSERT(!m_csLastError);
		Log(m_csLastError);
		return false;
	}

	TiXmlElement *ItemHeader = doc.FirstChildElement("Ditto_Theme_File");
	if(!ItemHeader)
	{
		m_csLastError.Format(_T("Error finding the section Ditto_Theme_File"));
		ASSERT(!m_csLastError);
		Log(m_csLastError);
		return false;
	}

	CString csVersion = ItemHeader->Attribute("Version");
	m_lFileVersion = ATOI(csVersion);
	m_csAuthor = ItemHeader->Attribute("Author");
	m_csNotes = ItemHeader->Attribute("Notes");

	if(bHeaderOnly)
		return true;


	LoadColor(ItemHeader, "CaptionLeft", m_CaptionLeft);
	LoadColor(ItemHeader, "CaptionRight", m_CaptionRight);
	LoadColor(ItemHeader, "CaptionLeftTopMost", m_CaptionLeftTopMost);
	LoadColor(ItemHeader, "CaptionRightTopMost", m_CaptionRightTopMost);
	LoadColor(ItemHeader, "CaptionLeftNotConnected", m_CaptionLeftNotConnected);
	LoadColor(ItemHeader, "CaptionRightNotConnected", m_CaptionRightNotConnected);
	LoadColor(ItemHeader, "CaptionTextColor", m_CaptionTextColor);
	LoadColor(ItemHeader, "ListBoxOddRowsBG", m_ListBoxOddRowsBG);
	LoadColor(ItemHeader, "ListBoxEvenRowsBG", m_ListBoxEvenRowsBG);
	LoadColor(ItemHeader, "ListBoxOddRowsText", m_ListBoxOddRowsText);
	LoadColor(ItemHeader, "ListBoxEvenRowsText", m_ListBoxEvenRowsText);
	LoadColor(ItemHeader, "ListBoxSelectedBG", m_ListBoxSelectedBG);
	LoadColor(ItemHeader, "ListBoxSelectedNoFocusBG", m_ListBoxSelectedNoFocusBG);
	LoadColor(ItemHeader, "ListBoxSelectedText", m_ListBoxSelectedText);
	LoadColor(ItemHeader, "ListBoxSelectedNoFocusText", m_ListBoxSelectedNoFocusText);
	LoadColor(ItemHeader, "ClipPastedColor", m_clipPastedColor);
	LoadColor(ItemHeader, "MainWindowBG", m_mainWindowBG);
	LoadColor(ItemHeader, "SearchTextBoxFocusBG", m_searchTextBoxFocusBG);
	LoadColor(ItemHeader, "SearchTextBoxFocusText", m_searchTextBoxFocusText);

	LoadColor(ItemHeader, "Border", m_Border);
	LoadColor(ItemHeader, "BorderTopMost", m_BorderTopMost);
	LoadColor(ItemHeader, "BorderNotConnected", m_BorderNotConnected);

	LoadColor(ItemHeader, "GroupTreeBG", m_groupTreeBG);
	LoadColor(ItemHeader, "GroupTreeText", m_groupTreeText);
	
	LoadInt(ItemHeader, "CaptionSize", m_captionSize);
	LoadInt(ItemHeader, "CaptionFontSize", m_captionFontSize);

	LoadColor(ItemHeader, "DescriptionWindowBG", m_descriptionWindowBG);
	LoadColor(ItemHeader, "DescriptionWindowText", m_descriptionWindowText);

	return true;
}

bool CTheme::LoadColor(TiXmlElement *pParent, CStringA csNode, COLORREF &Color)
{
	int intValue = 0;
	return LoadElement(pParent, csNode, Color, intValue);
}

bool CTheme::LoadInt(TiXmlElement *pParent, CStringA csNode, int &intValue)
{
	COLORREF colorValue = 0;
	return LoadElement(pParent, csNode, colorValue, intValue);
}

bool CTheme::LoadElement(TiXmlElement *pParent, CStringA csNode, COLORREF &Color, int &intValue)
{
	TiXmlElement *pColorNode = pParent->FirstChildElement(csNode);
	if(pColorNode == NULL)
	{
		m_csLastError.Format(_T("Theme Load, error loading Node = %s"), csNode);
		Log(m_csLastError);
		return false;
	}

	TiXmlNode *pColor = pColorNode->FirstChild();
	if(pColor == NULL)
	{
		m_csLastError.Format(_T("Theme Load, error getting node text for = %s"), csNode);
		Log(m_csLastError);
		return false;
	}
	
	CString csColor = pColor->Value();

	if (csColor == _T(""))
	{
		return false;
	}

	if (csColor.Find(_T("RGB")) >= 0)
	{
		csColor = csColor.Trim();
		csColor.Replace(_T("RGB("), _T(""));
		csColor.Replace(_T(")"), _T(""));

		CTokenizer token(csColor, _T(","));
		CString csR;
		CString csG;
		CString csB;

		token.Next(csR);
		token.Next(csG);
		token.Next(csB);

		csR = csR.Trim();
		csG = csG.Trim();
		csB = csB.Trim();

		//Only the first is valid they entered the RGB value as a single number
		if (csR != "" && csG == "" && csB == "")
		{
			Color = ATOI(csR);
		}
		else
		{
			Color = RGB(ATOI(csR), ATOI(csG), ATOI(csB));
		}
	}
	else
	{
		intValue = ATOI(csColor);
	}

	return true;
}