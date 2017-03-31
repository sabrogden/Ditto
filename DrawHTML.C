/* DrawHTML()
 * Drop-in replacement for DrawText() supporting a tiny subset of HTML.
 */
#include <windows.h>
#include <tchar.h>
#include <assert.h>

#define ENDFLAG   0x100
enum { tNONE, tB, tBR, tFONT, tI, tP, tSUB, tSUP, tU, tNUMTAGS };
struct
{
	char *mnemonic;
	short token, param, block;
} Tags[] = {
  { NULL,         tNONE, 0, 0},
  { _T("从@#$"),   tFONT, 1, 0 },
  /*{ _T("b"),      tB,    0, 0},
  { _T("br"),     tBR,   0, 1},
  { _T("em"),     tI,    0, 0},
  { _T("font"),   tFONT, 1, 0},
  { _T("i"),      tI,    0, 0},
  { _T("p"),      tP,    0, 1},
  { _T("strong"), tB,    0, 0},
  { _T("sub"),    tSUB,  0, 0},
  { _T("sup"),    tSUP,  0, 0},
  { _T("u"),      tU,    0, 0},*/
};

static int GetToken(LPCTSTR *String, int *Size, int *TokenLength, BOOL *WhiteSpace)
{
	LPCTSTR Start, EndToken;
	int Length, EntryWhiteSpace, Index, IsEndTag;

	assert(String != NULL && *String != NULL);
	assert(Size != NULL);
	Start = *String;

	/* check for leading white space, then skip it */
	if (WhiteSpace != NULL)
	{
		EntryWhiteSpace = *WhiteSpace;
		*WhiteSpace = EntryWhiteSpace || _istspace(*Start);
	}
	else
	{
		EntryWhiteSpace = FALSE;
	} /* if */
	while (*Size > 0 && _istspace(*Start))
	{
		Start++;
		*Size -= 1;
	} /* if */
	if (*Size <= 0)
		return -1;  /* no printable text left */

	EndToken = Start;
	Length = 0;
	IsEndTag = 0;
	if (*EndToken == _T('<'))
	{
		/* might be a HTML tag, check */
		EndToken++;
		Length++;
		if (Length < *Size && *EndToken == _T('/'))
		{
			IsEndTag = ENDFLAG;
			EndToken++;
			Length++;
		} /* if */
		while (Length < *Size && !_istspace(*EndToken)
			&& *EndToken != _T('<') && *EndToken != _T('>'))
		{
			EndToken++;
			Length++;
		} /* while */
		for (Index = sizeof Tags / sizeof Tags[0] - 1; Index > 0; Index--)
			if (!_tcsnicmp(Start + (IsEndTag ? 2 : 1), Tags[Index].mnemonic,
				_tcslen(Tags[Index].mnemonic)))
				break;
		if (Index > 0)
		{
			/* so it is a tag, see whether to accept parameters */
			if (Tags[Index].param && !IsEndTag)
			{
				while (Length < *Size
					&& *EndToken != _T('<') && *EndToken != _T('>'))
				{
					EndToken++;
					Length++;
				} /* while */
			}
			else if (*EndToken != _T('>'))
			{
				/* no parameters, then '>' must follow the tag */
				Index = 0;
			} /* if */
			if (WhiteSpace != NULL && Tags[Index].block)
				*WhiteSpace = FALSE;
		} /* if */
		if (*EndToken == _T('>'))
		{
			EndToken++;
			Length++;
		} /* if */
		/* skip trailing white space in some circumstances */
		if (Index > 0 && (Tags[Index].block || EntryWhiteSpace))
		{
			while (Length < *Size && _istspace(*EndToken))
			{
				EndToken++;
				Length++;
			} /* while */
		} /* if */

	}
	else
	{
		/* normal word (no tag) */
		Index = 0;
		while (Length < *Size && !_istspace(*EndToken) && *EndToken != _T('<'))
		{
			EndToken++;
			Length++;
		} /* while */
	} /* if */

	if (TokenLength != NULL)
		*TokenLength = Length;
	*Size -= Length;
	*String = Start;
	return Tags[Index].token | IsEndTag;
}

static int HexDigit(TCHAR ch)
{
	if (ch >= _T('0') && ch <= _T('9'))
		return ch - _T('0');
	if (ch >= _T('A') && ch <= _T('F'))
		return ch - _T('A') + 10;
	if (ch >= _T('a') && ch <= _T('f'))
		return ch - _T('a') + 10;
	return 0;
}

static COLORREF ParseColor(LPCTSTR String)
{
	int Red, Green, Blue;

	if (*String == _T('\'') || *String == _T('"'))
		String++;
	if (*String == _T('#'))
		String++;
	Red = (HexDigit(String[0]) << 4) | HexDigit(String[1]);
	Green = (HexDigit(String[2]) << 4) | HexDigit(String[3]);
	Blue = (HexDigit(String[4]) << 4) | HexDigit(String[5]);
	return RGB(Red, Green, Blue);
}

#define STACKSIZE   8
static COLORREF stack[STACKSIZE];
static int stacktop;

static BOOL PushColor(HDC hdc, COLORREF clr)
{
	if (stacktop < STACKSIZE)
		stack[stacktop++] = GetTextColor(hdc);
	SetTextColor(hdc, clr);
	return TRUE;
}

static BOOL PopColor(HDC hdc)
{
	COLORREF clr;
	BOOL okay = (stacktop > 0);

	if (okay)
		clr = stack[--stacktop];
	else
		clr = stack[0];
	SetTextColor(hdc, clr);
	return okay;
}


#define FV_BOLD        0x01
#define FV_ITALIC      (FV_BOLD << 1)
#define FV_UNDERLINE   (FV_ITALIC << 1)
#define FV_SUPERSCRIPT (FV_UNDERLINE << 1)
#define FV_SUBSCRIPT   (FV_SUPERSCRIPT << 1)
#define FV_NUMBER      (FV_SUBSCRIPT << 1)

static HFONT GetFontVariant(HDC hdc, HFONT hfontSource, int Styles)
{
	LOGFONT logFont = { 0 };

	SelectObject(hdc, (HFONT)GetStockObject(SYSTEM_FONT));
	if (!GetObject(hfontSource, sizeof logFont, &logFont))
		return NULL;

	/* set parameters, create new font */
	logFont.lfWeight = (Styles & FV_BOLD) ? FW_BOLD : FW_NORMAL;
	logFont.lfItalic = (BYTE)(Styles & FV_ITALIC) != 0;
	logFont.lfUnderline = (BYTE)(Styles & FV_UNDERLINE) != 0;
	if (Styles & (FV_SUPERSCRIPT | FV_SUBSCRIPT))
		logFont.lfHeight = logFont.lfHeight * 7 / 10;
	return CreateFontIndirect(&logFont);
}

#if defined __cplusplus
extern "C"
#endif
int __stdcall DrawHTML(
	HDC     hdc,        // handle of device context
	LPCTSTR lpString,   // address of string to draw
	int     nCount,     // string length, in characters
	LPRECT  lpRect,     // address of structure with formatting dimensions
	UINT    uFormat     // text-drawing flags
)
{
	LPCTSTR Start;
	int Left, Top, MaxWidth, MinWidth, Height, MaxHeight;
	int SavedDC;
	int Tag, TokenLength;
	HFONT hfontBase, hfontSpecial[FV_NUMBER];
	int Styles, CurStyles;
	SIZE size;
	int Index, LineHeight;
	POINT CurPos;
	int WidthOfSPace, XPos;
	BOOL WhiteSpace;
	RECT rc;

	if (hdc == NULL || lpString == NULL)
		return 0;
	if (nCount < 0)
		nCount = _tcslen(lpString);

	MaxHeight = INT_MAX;

	if (lpRect != NULL)
	{
		Left = lpRect->left;
		Top = lpRect->top;
		MaxWidth = lpRect->right - lpRect->left;
		MaxHeight = lpRect->bottom - lpRect->top;
	}
	else
	{
		GetCurrentPositionEx(hdc, &CurPos);
		Left = CurPos.x;
		Top = CurPos.y;
		MaxWidth = GetDeviceCaps(hdc, HORZRES) - Left;
	} /* if */
	if (MaxWidth < 0)
		MaxWidth = 0;

	/* toggle flags we do not support */
	uFormat &= ~(DT_CENTER | DT_RIGHT | DT_TABSTOP);
	uFormat |= (DT_LEFT | DT_NOPREFIX);

	/* get the "default" font from the DC */
	SavedDC = SaveDC(hdc);
	hfontBase = SelectObject(hdc, (HFONT)GetStockObject(SYSTEM_FONT));
	SelectObject(hdc, hfontBase);
	/* clear the other fonts, they are created "on demand" */
	for (Index = 0; Index < FV_NUMBER; Index++)
		hfontSpecial[Index] = NULL;
	hfontSpecial[0] = hfontBase;
	Styles = 0; /* assume the active font is normal weight, roman, non-underlined */

	/* get font height (use characters with ascender and descender);
	 * we make the assumption here that changing the font style will
	 * not change the font height
	 */
	GetTextExtentPoint32(hdc, _T("Åy"), 2, &size);
	LineHeight = size.cy;

	/* run through the string, word for word */
	XPos = 0;
	MinWidth = 0;
	stacktop = 0;
	CurStyles = -1; /* force a select of the proper style */
	Height = 0;
	WhiteSpace = FALSE;

	Start = lpString;
	for (;; )
	{
		Tag = GetToken(&Start, &nCount, &TokenLength, &WhiteSpace);
		if (Tag < 0)
			break;
		switch (Tag & ~ENDFLAG)
		{
		case tP:
			if ((Tag & ENDFLAG) == 0 && (uFormat & DT_SINGLELINE) == 0)
			{
				if (Start != lpString)
					Height += 3 * LineHeight / 2;
				XPos = 0;
			} /* if */
			break;
		case tBR:
			if ((Tag & ENDFLAG) == 0 && (uFormat & DT_SINGLELINE) == 0)
			{
				Height += LineHeight;
				XPos = 0;
			} /* if */
			break;
		case tB:
			Styles = (Tag & ENDFLAG) ? Styles & ~FV_BOLD : Styles | FV_BOLD;
			break;
		case tI:
			Styles = (Tag & ENDFLAG) ? Styles & ~FV_ITALIC : Styles | FV_ITALIC;
			break;
		case tU:
			Styles = (Tag & ENDFLAG) ? Styles & ~FV_UNDERLINE : Styles | FV_UNDERLINE;
			break;
		case tSUB:
			Styles = (Tag & ENDFLAG) ? Styles & ~FV_SUBSCRIPT : Styles | FV_SUBSCRIPT;
			break;
		case tSUP:
			Styles = (Tag & ENDFLAG) ? Styles & ~FV_SUPERSCRIPT : Styles | FV_SUPERSCRIPT;
			break;
		case tFONT:
			if ((Tag & ENDFLAG) == 0)
			{
				if (_tcsnicmp(Start + 6, _T("color="), 6) == 0)
					PushColor(hdc, ParseColor(Start + 12));
			}
			else
			{
				PopColor(hdc);
			} /* if */
			break;
		default:
			if (Tag == (tNONE | ENDFLAG))
				break;
			if (CurStyles != Styles)
			{
				if (hfontSpecial[Styles] == NULL)
					hfontSpecial[Styles] = GetFontVariant(hdc, hfontBase, Styles);
				CurStyles = Styles;
				SelectObject(hdc, hfontSpecial[Styles]);
				/* get the width of a space character (for word spacing) */
				GetTextExtentPoint32(hdc, _T(" "), 1, &size);
				WidthOfSPace = size.cx;
			} /* if */
			/* check word length, check whether to wrap around */
			GetTextExtentPoint32(hdc, Start, TokenLength, &size);
			if (size.cx > MaxWidth)
				MaxWidth = size.cx;   /* must increase width: long non-breakable word */
			if (WhiteSpace)
				XPos += WidthOfSPace;
			if (XPos + size.cx > MaxWidth && WhiteSpace)
			{
				if ((uFormat & DT_WORDBREAK) != 0)
				{
					/* word wrap */
					Height += LineHeight;
					XPos = 0;
				}
				else
				{
					/* no word wrap, must increase the width */
					MaxWidth = XPos + size.cx;
				} /* if */
			} /* if */
			/* output text (unless DT_CALCRECT is set) */
			if ((uFormat & DT_CALCRECT) == 0)
			{
				/* handle negative heights, too (suggestion of "Sims")  */
				if (Top < 0)
					SetRect(&rc, Left + XPos, Top - Height,
						Left + MaxWidth, Top - (Height + LineHeight));
				else
					SetRect(&rc, Left + XPos, Top + Height,
						Left + MaxWidth, Top + Height + LineHeight);

				/* reposition subscript text to align below the baseline */
				DrawText(hdc, Start, TokenLength, &rc,
					uFormat | ((Styles & FV_SUBSCRIPT) ? DT_BOTTOM | DT_SINGLELINE : 0));

				/* for the underline style, the spaces between words should be
				 * underlined as well
				 */
				if (WhiteSpace && (Styles & FV_UNDERLINE) && XPos >= WidthOfSPace)
				{
					if (Top < 0)
						SetRect(&rc, Left + XPos - WidthOfSPace, Top - Height,
							Left + XPos, Top - (Height + LineHeight));
					else
						SetRect(&rc, Left + XPos - WidthOfSPace, Top + Height,
							Left + XPos, Top + Height + LineHeight);
					DrawText(hdc, " ", 1, &rc, uFormat);
				} /* if */
			} /* if */
			/* update current position */
			XPos += size.cx;
			if (XPos > MinWidth)
				MinWidth = XPos;
			WhiteSpace = FALSE;
		} /* if */

		if ((Height + LineHeight) >= MaxHeight)
			break;

		Start += TokenLength;
	} /* for */

	RestoreDC(hdc, SavedDC);
	for (Index = 1; Index < FV_NUMBER; Index++) /* do not erase hfontSpecial[0] */
		if (hfontSpecial[Index] != NULL)
			DeleteObject(hfontSpecial[Index]);

	/* store width and height back into the lpRect structure */
	if ((uFormat & DT_CALCRECT) != 0 && lpRect != NULL)
	{
		lpRect->right = lpRect->left + MinWidth;
		if (lpRect->top < 0)
			lpRect->bottom = lpRect->top - (Height + LineHeight);
		else
			lpRect->bottom = lpRect->top + Height + LineHeight;
	} /* if */

	return Height;
}
