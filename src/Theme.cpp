#include "stdafx.h"
#include ".\theme.h"
#include "..\Shared\TextConvert.h"
#include "Misc.h"
#include "Options.h"
#include "..\Shared\Tokenizer.h"
#include "CP_Main.h"

#include <cmath>     // For fmod, round (or std::round in C++11)
#include <tchar.h>   // For _ttoi, _ttof, _stscanf_s, _tcstol
#include <algorithm> // For std::max, std::min

// -- START HELPER FUNCTION DECLARATIONS (STATIC WITHIN CPP) --
// These functions are only used by LoadElement within this file.

// Converts HSL (Hue, Saturation, Lightness) to RGB COLORREF
// h: 0-360, s: 0.0-1.0, l: 0.0-1.0
static COLORREF HslToRgb(float h, float s, float l)
{
	float r_f, g_f, b_f;

	if (s == 0)
	{
		r_f = g_f = b_f = l; // achromatic
	}
	else
	{
		auto hueToRgbComponent = [](float p, float q, float t)
		{
			if (t < 0.0f)
				t += 1.0f;
			if (t > 1.0f)
				t -= 1.0f;
			if (t < 1.0f / 6.0f)
				return p + (q - p) * 6.0f * t;
			if (t < 1.0f / 2.0f)
				return q;
			if (t < 2.0f / 3.0f)
				return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
			return p;
		};

		float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
		float p = 2.0f * l - q;
		float hk = h / 360.0f; // Normalize H to 0-1

		r_f = hueToRgbComponent(p, q, hk + 1.0f / 3.0f);
		g_f = hueToRgbComponent(p, q, hk);
		b_f = hueToRgbComponent(p, q, hk - 1.0f / 3.0f);
	}

	return RGB(static_cast<int>(r_f * 255.0f),
		   static_cast<int>(g_f * 255.0f),
		   static_cast<int>(b_f * 255.0f));
}

// Parses a string value that can be a percentage "N%" or a float "N.N"
// Returns a float (e.g., "50%" -> 0.5, "0.6" -> 0.6)
static float ParsePercentageOrFloat(const CString &csValInput)
{
	CString s = csValInput;
	s.Trim();
	s.Replace(',', '.');
	if (s.Right(1) == _T("%"))
	{
		s = s.Left(s.GetLength() - 1);
		return (float)_ttof(s) / 100.0f;
	}
	return (float)_ttof(s);
}

// Parses an RGB component string that can be "N" or "N%"
// Returns an int (0-255)
static int ParseRgbValue(const CString &csValInput)
{
	CString s = csValInput;
	s.Trim();
	if (s.Right(1) == _T("%"))
	{
		s = s.Left(s.GetLength() - 1);
		float percent_val = (float)_ttof(s);
		// Round to nearest integer after conversion
		return std::clamp(static_cast<int>(round((percent_val / 100.0f) * 255.0f)), 0, 255);
	}
	return _ttoi(s);
}

// -- END HELPER FUNCTION DECLARATIONS --

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
	m_BorderTopMost = RGB(204, 204, 204);
	m_BorderNotConnected = RGB(204, 204, 204);

	m_CaptionLeftTopMost = RGB(255, 255, 255);
	m_CaptionRightTopMost = RGB(204, 204, 204);

	m_CaptionLeftNotConnected = RGB(255, 255, 255);
	m_CaptionRightNotConnected = RGB(255, 255, 0);

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
	m_searchTextBoxFocusBorder = RGB(255, 255, 255);
	m_searchTextHighlight = RGB(255, 0, 0);

	m_groupTreeBG = RGB(240, 240, 240);
	m_groupTreeText = RGB(127, 127, 127);

	m_descriptionWindowBG = RGB(240, 240, 240);
	m_descriptionWindowText = RGB(0, 0, 0);

	m_captionSize = 25;
	m_captionFontSize = 19;
}

bool CTheme::Load(CString csTheme, bool bHeaderOnly, bool bCheckLastWriteTime)
{
	bool followWindows10Theme = false;
	if (csTheme.IsEmpty())
	{
		followWindows10Theme = true;

		if (DarkAppWindows10Setting())
		{
			csTheme = _T("DarkerDitto");
			Log(_T("Loading theme based on windows setting of dark mode for apps"));
		}
	}

	if (csTheme.IsEmpty() || csTheme == _T("Ditto") || csTheme == _T("(Default)") || csTheme == _T("(Ditto)"))
	{
		LoadDefaults();

		if (followWindows10Theme)
		{
			LoadWindowsAccentColor();
		}

		m_LastWriteTime = 0;
		m_lastTheme = _T("");

		Log(_T("Loading default ditto values for themes"));

		return false; // Assuming false indicates default loaded, not an error
	}

	CString csPath = CGetSetOptions::GetPath(PATH_THEMES);
	csPath += csTheme;
	csPath += ".xml";

	__int64 LastWrite = GetLastWriteTime(csPath);

	if (bCheckLastWriteTime)
	{
		if (m_lastTheme == csTheme &&
			LastWrite == m_LastWriteTime)
		{
			return true;
		}
	}

	LoadDefaults();

	m_LastWriteTime = LastWrite;
	m_lastTheme = csTheme;

	Log(StrF(_T("Loading Theme %s"), csPath));

	TiXmlDocument doc;
	if (!doc.LoadFile(CT2A(csPath))) // Use CT2A for CString to const char* conversion for TinyXML
	{
		m_csLastError.Format(_T("Error loading Theme %s - reason = %s"), csPath, CA2T(doc.ErrorDesc())); // Use CA2T for const char* to CString
		// ASSERT(!m_csLastError); // Consider removing ASSERT for release builds or handle differently
		Log(m_csLastError);
		return false;
	}

	TiXmlElement *ItemHeader = doc.FirstChildElement("Ditto_Theme_File");
	if (!ItemHeader)
	{
		m_csLastError.Format(_T("Error finding the section Ditto_Theme_File in theme %s"), csPath);
		// ASSERT(!m_csLastError);
		Log(m_csLastError);
		return false;
	}

	// Attributes are char*, convert to CString if needed for member variables
	const char *versionAttr = ItemHeader->Attribute("Version");
	m_lFileVersion = versionAttr ? ATOI(versionAttr) : 0; // Use ATOI for const char*

	const char *authorAttr = ItemHeader->Attribute("Author");
	m_csAuthor = authorAttr ? CA2T(authorAttr) : _T("");

	const char *notesAttr = ItemHeader->Attribute("Notes");
	m_csNotes = notesAttr ? CA2T(notesAttr) : _T("");

	if (bHeaderOnly)
		return true;

	// Load actual theme elements
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
	LoadColor(ItemHeader, "ListSmallQuickPasteIndexColor", m_listSmallQuickPasteIndexColor); // Added this based on LoadDefaults
	LoadColor(ItemHeader, "MainWindowBG", m_mainWindowBG);
	LoadColor(ItemHeader, "SearchTextBoxFocusBG", m_searchTextBoxFocusBG);
	LoadColor(ItemHeader, "SearchTextBoxFocusText", m_searchTextBoxFocusText);
	LoadColor(ItemHeader, "SearchTextBoxFocusBorder", m_searchTextBoxFocusBorder);
	LoadColor(ItemHeader, "SearchTextHighlight", m_searchTextHighlight);

	LoadColor(ItemHeader, "Border", m_Border);
	LoadColor(ItemHeader, "BorderTopMost", m_BorderTopMost);
	LoadColor(ItemHeader, "BorderNotConnected", m_BorderNotConnected);

	LoadColor(ItemHeader, "GroupTreeBG", m_groupTreeBG);
	LoadColor(ItemHeader, "GroupTreeText", m_groupTreeText);

	LoadInt(ItemHeader, "CaptionSize", m_captionSize);
	LoadInt(ItemHeader, "CaptionFontSize", m_captionFontSize);

	LoadColor(ItemHeader, "DescriptionWindowBG", m_descriptionWindowBG);
	LoadColor(ItemHeader, "DescriptionWindowText", m_descriptionWindowText);

	if (followWindows10Theme)
	{
		LoadWindowsAccentColor();
	}

	return true;
}

void CTheme::LoadWindowsAccentColor()
{
	DWORD accent = Windows10AccentColor();
	if (accent != (DWORD)-1) // Check against -1 cast to DWORD
	{
		// Windows accent color is often BGR, convert to RGB for COLORREF
		BYTE b = GetBValue(accent);
		BYTE g = GetGValue(accent);
		BYTE r = GetRValue(accent);

		COLORREF windowsAccentRGB = RGB(r, g, b);

		m_clipPastedColor = windowsAccentRGB;
		m_searchTextBoxFocusBorder = windowsAccentRGB;
		m_searchTextHighlight = windowsAccentRGB;

		// Example: Optionally theme other elements
		// if (Windows10ColorTitleBar()) // Assuming Windows10ColorTitleBar() is defined
		// {
		// 	m_CaptionRight = windowsAccentRGB;
		// 	m_CaptionLeft = windowsAccentRGB; // Or a lighter/darker variant
		// 	m_Border = windowsAccentRGB;
		// }
	}
}

bool CTheme::LoadColor(TiXmlElement *pParent, CStringA csNodeName, COLORREF &Color)
{
	int dummyIntValue = 0; // Not used by LoadColor for its result
	return LoadElement(pParent, csNodeName, Color, dummyIntValue);
}

bool CTheme::LoadInt(TiXmlElement *pParent, CStringA csNodeName, int &intValue)
{
	COLORREF dummyColorValue = 0; // Not used by LoadInt for its result
	return LoadElement(pParent, csNodeName, dummyColorValue, intValue);
}

// Main updated function to parse different color formats and integers
bool CTheme::LoadElement(TiXmlElement *pParent, CStringA csNodeNameA, COLORREF &ColorRef, int &intValueRef)
{
	TiXmlElement *pNode = pParent->FirstChildElement(csNodeNameA.GetString()); // Use GetString() for CStringA with TinyXML
	if (pNode == nullptr)
	{
		// Log quietly or not at all for missing optional elements, or handle as needed.
		// m_csLastError.Format(_T("Theme Load, optional node not found = %S"), csNodeNameA.GetString());
		// Log(m_csLastError);
		return false; // Node not found
	}

	TiXmlNode *pValueNode = pNode->FirstChild();
	if (pValueNode == nullptr || pValueNode->Type() != TiXmlNode::TINYXML_TEXT)
	{
		m_csLastError.Format(_T("Theme Load, error getting node text for = %S"), csNodeNameA.GetString());
		Log(m_csLastError);
		return false; // No text value in the node
	}

	CString csValue = CA2T(pValueNode->Value()); // Convert const char* from TinyXML to CString
	csValue.Trim();

	if (csValue.IsEmpty())
	{
		return false; // No value to parse
	}

	bool bParsedAsColor = false;
	COLORREF tempColor = 0;

	// 1. Try HEX: #RGB or #RRGGBB
	if (csValue.Left(1) == _T("#"))
	{
		CString hexPart = csValue.Mid(1);
		int r_val = 0, g_val = 0, b_val = 0;
		if (hexPart.GetLength() == 3) // #RGB format (e.g., #F0A -> FF00AA)
		{
			unsigned int r_nibble, g_nibble, b_nibble;
			// Use _stscanf_s for safer parsing. Ensure hexPart is null-terminated if needed by _stscanf_s.
			// CString.GetBuffer() can be used, then ReleaseBuffer().
			if (_stscanf_s(hexPart, _T("%1x%1x%1x"), &r_nibble, &g_nibble, &b_nibble) == 3)
			{
				r_val = r_nibble * 17; // Expands F to FF (15*16 + 15 = 255)
				g_val = g_nibble * 17;
				b_val = b_nibble * 17;
				tempColor = RGB(r_val, g_val, b_val);
				bParsedAsColor = true;
			}
		}
		else if (hexPart.GetLength() == 6) // #RRGGBB format
		{
			unsigned int ur, ug, ub;
			if (_stscanf_s(hexPart, _T("%2x%2x%2x"), &ur, &ug, &ub) == 3)
			{
				r_val = ur;
				g_val = ug;
				b_val = ub;
				tempColor = RGB(r_val, g_val, b_val);
				bParsedAsColor = true;
			}
		}
	}
	// 2. Try rgb(...) or rgb(r,g,b) or rgb(r g b)
	// Make comparison case-insensitive for "rgb("
	else if (csValue.GetLength() > 4 && csValue.Left(4).MakeLower() == _T("rgb("))
	{
		CString content = csValue.Mid(4);
		int closingParen = content.Find(_T(")"));
		if (closingParen != -1)
		{
			content = content.Left(closingParen);
		}
		else // Malformed - no closing parenthesis
		{
			// Log error or handle as invalid format
		}
		content.Trim();

		CTokenizer token(content, _T(", ")); // Handles space and comma delimiters
		CString csR, csG, csB;
		token.Next(csR);
		csR.Trim();
		token.Next(csG);
		csG.Trim();
		token.Next(csB);
		csB.Trim();

		if (!csR.IsEmpty() && !csG.IsEmpty() && !csB.IsEmpty())
		{
			int r_val = _ttoi(csR);		// R component in rgb() is usually not a percentage
			int g_val = ParseRgbValue(csG); // G can be N or N%
			int b_val = ParseRgbValue(csB); // B can be N or N%
			tempColor = RGB(r_val, g_val, b_val);
			bParsedAsColor = true;
		}
	}
	// 3. Try hsl(...) or hsl(h,s,l) or hsl(h s l)
	// Make comparison case-insensitive for "hsl("
	else if (csValue.GetLength() > 4 && csValue.Left(4).MakeLower() == _T("hsl("))
	{
		CString content = csValue.Mid(4);
		int closingParen = content.Find(_T(")"));
		if (closingParen != -1)
		{
			content = content.Left(closingParen);
		}
		else // Malformed
		{
			// Log error or handle
		}
		content.Trim();

		CTokenizer token(content, _T(", "));
		CString csH, csS, csL;
		token.Next(csH);
		csH.Trim();
		token.Next(csS);
		csS.Trim();
		token.Next(csL);
		csL.Trim();

		if (!csH.IsEmpty() && !csS.IsEmpty() && !csL.IsEmpty())
		{
			float h_val = (float)_ttof(csH);	   // Hue (0-360)
			float s_val = ParsePercentageOrFloat(csS); // Saturation (0.0-1.0)
			float l_val = ParsePercentageOrFloat(csL); // Lightness (0.0-1.0)

			// Normalize/clamp HSL values
			h_val = fmod(h_val, 360.0f);
			if (h_val < 0)
				h_val += 360.0f;
			s_val = std::max(0.0f, std::min(1.0f, s_val));
			l_val = std::max(0.0f, std::min(1.0f, l_val));

			tempColor = HslToRgb(h_val, s_val, l_val);
			bParsedAsColor = true;
		}
	}
	// 4. Fallback to original RGB(R,G,B) format (case-sensitive as in original code)
	// This also handles the special case of RGB(single_integer_COLORREF_value)
	else if (csValue.Find(_T("RGB(")) == 0)
	{
		CString temp = csValue; // Work with a copy
		temp.Replace(_T("RGB("), _T(""));
		temp.Replace(_T(")"), _T(""));

		CTokenizer token(temp, _T(",")); // Original used comma only
		CString csR_orig, csG_orig, csB_orig;
		token.Next(csR_orig);
		csR_orig.Trim();
		token.Next(csG_orig);
		csG_orig.Trim();
		token.Next(csB_orig);
		csB_orig.Trim();

		if (!csR_orig.IsEmpty() && csG_orig.IsEmpty() && csB_orig.IsEmpty())
		{
			// Single number was allowed for direct COLORREF
			LPTSTR endptr;
			long raw_color_val = _tcstol(csR_orig, &endptr, 10);
			if (*endptr == _T('\0') && csR_orig.GetLength() > 0) // Check if entire string was a valid number
			{
				tempColor = static_cast<COLORREF>(raw_color_val);
				bParsedAsColor = true;
			}
		}
		else if (!csR_orig.IsEmpty() && !csG_orig.IsEmpty() && !csB_orig.IsEmpty())
		{
			tempColor = RGB(_ttoi(csR_orig), _ttoi(csG_orig), _ttoi(csB_orig));
			bParsedAsColor = true;
		}
	}

	// If parsed successfully as any color format, assign to ColorRef
	if (bParsedAsColor)
	{
		ColorRef = tempColor;
	}
	else // Not parsed as any color format, try to parse as a plain integer for intValueRef
	{
		LPTSTR endptr;
		long val = _tcstol(csValue, &endptr, 10);
		// Check if the entire string was consumed and is a valid number
		// Ensure that csValue is not empty and starts with a digit or '-'
		if (csValue.GetLength() > 0 && ((csValue[0] >= _T('0') && csValue[0] <= _T('9')) || csValue[0] == _T('-')) && *endptr == _T('\0'))
		{
			intValueRef = static_cast<int>(val);
		}
		else
		{
			// Fallback for non-numeric strings when an int is expected, or malformed colors.
			// Original code used ATOI/_ttoi, which returns 0 for non-convertible strings.
			intValueRef = _ttoi(csValue);
			// Optionally log if csValue was expected to be an int but wasn't purely numeric
			// if (csNodeNameA is an int type and *endptr != _T('\0')) { Log("Warning: Expected int, got non-numeric value..."); }
		}
	}

	return true; // Return true if node existed and value was processed (even if parsing to int as fallback)
}
