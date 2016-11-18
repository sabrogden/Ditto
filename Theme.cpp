#include "stdafx.h"
#include ".\theme.h"
#include "shared/TextConvert.h"
#include "Misc.h"
#include "Options.h"
#include "shared/Tokenizer.h"

CTheme::CTheme(void)
{
	m_lFileVersion = 0;
	m_LastWriteTime = 0;
	m_lastTheme = _T("");

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
	m_ListBoxEvenRowsBG = RGB(250, 250, 250);
	m_ListBoxOddRowsText = RGB(0, 0, 0);
	m_ListBoxEvenRowsText = RGB(0, 0, 0);
	m_ListBoxSelectedBG = RGB(204, 204, 204);
	m_ListBoxSelectedNoFocusBG = RGB(204, 204, 204);
	m_ListBoxSelectedText = RGB(0, 0, 0);
	m_ListBoxSelectedNoFocusText = RGB(0, 0, 0);
	m_clipPastedColor = RGB(0, 255, 0);
	m_listSmallQuickPasteIndexColor = RGB(180, 180, 180);
	m_mainWindowBG = RGB(240, 240, 240);
}

CTheme::~CTheme(void)
{
}


bool CTheme::Load(CString csTheme, bool bHeaderOnly, bool bCheckLastWriteTime)
{
	if(csTheme.IsEmpty())
		return false;

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

	LoadElement(ItemHeader, "CaptionLeft", m_CaptionLeft);
	LoadElement(ItemHeader, "CaptionRight", m_CaptionRight);
	LoadElement(ItemHeader, "CaptionLeftTopMost", m_CaptionLeftTopMost);
	LoadElement(ItemHeader, "CaptionRightTopMost", m_CaptionRightTopMost);
	LoadElement(ItemHeader, "CaptionLeftNotConnected", m_CaptionLeftNotConnected);
	LoadElement(ItemHeader, "CaptionRightNotConnected", m_CaptionRightNotConnected);
	LoadElement(ItemHeader, "CaptionTextColor", m_CaptionTextColor);
	LoadElement(ItemHeader, "ListBoxOddRowsBG", m_ListBoxOddRowsBG);
	LoadElement(ItemHeader, "ListBoxEvenRowsBG", m_ListBoxEvenRowsBG);
	LoadElement(ItemHeader, "ListBoxOddRowsText", m_ListBoxOddRowsText);
	LoadElement(ItemHeader, "ListBoxEvenRowsText", m_ListBoxEvenRowsText);
	LoadElement(ItemHeader, "ListBoxSelectedBG", m_ListBoxSelectedBG);
	LoadElement(ItemHeader, "ListBoxSelectedNoFocusBG", m_ListBoxSelectedNoFocusBG);
	LoadElement(ItemHeader, "ListBoxSelectedText", m_ListBoxSelectedText);
	LoadElement(ItemHeader, "ListBoxSelectedNoFocusText", m_ListBoxSelectedNoFocusText);
	LoadElement(ItemHeader, "ClipPastedColor", m_clipPastedColor);

	return true;
}

bool CTheme::LoadElement(TiXmlElement *pParent, CStringA csNode, COLORREF &Color)
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
	if(csR != "" && csG == "" && csB == "")
	{	
		Color = ATOI(csR);
	}
	else
	{
		Color = RGB(ATOI(csR), ATOI(csG), ATOI(csB));
	}

	return true;
}