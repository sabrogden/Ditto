#pragma once

// Colour emoji in the clip list (issue #251).
// GDI DrawText renders colour fonts (Segoe UI Emoji) black and white, so rows
// that contain emoji are drawn through Direct2D/DirectWrite instead.
namespace ColorEmojiText
{
	// Draws text the way CDC::DrawText(text, rc, DT_EXPANDTABS | DT_NOPREFIX) does,
	// with the font and text colour currently selected into pDC, but keeps colour glyphs.
	// Returns false without drawing anything when the text has no emoji-like characters
	// or DirectWrite failed - the caller then draws with GDI as before.
	bool DrawColorText(CDC* pDC, const CString& text, const CRect& rc);
}
