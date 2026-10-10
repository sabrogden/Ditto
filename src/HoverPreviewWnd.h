#pragma once

#include "DPI.h"

class CRichEditCtrlEx;
struct IFormattedTextDraw;

//---------------------------------------------------------------------------
// CHoverPreviewWnd
//
// Own hover preview popup for the quick paste list.
//
// This is deliberately NOT the existing description window (CToolTipEx).
class CHoverPreviewWnd : public CWnd
{
public:
	// Shows the preview of lClipId with csInfo drawn underneath for images
	// and colours.  rcAnchor is the screen rectangle of the list row the
	// mouse is over.  bHasColorCode tells the popup that the clip text is a
	// colour code, which only the list control can work out.  Returns false
	// when the clip has nothing to preview.
	static bool ShowForClip(CWnd* pOwner, long lClipId, const CString& csInfo, const CRect& rcAnchor,
		bool bHasColorCode, COLORREF crColorCode, int nColorAlpha);

	// Hides the popup when it is visible, otherwise does nothing.
	static void HideIfVisible();

	// true when the popup is visible and currently showing lClipId.
	static bool IsShowingClip(long lClipId);

private:
	enum HoverPreviewKind
	{
		KIND_NONE = 0,
		KIND_IMAGE,
		KIND_RTF,
		KIND_COLOR,
		KIND_HTML,
		// nothing but text, the popup then shows the text the tool tip bubble
		// would have shown
		KIND_TEXT
	};

	CHoverPreviewWnd();
	virtual ~CHoverPreviewWnd();

	// Returns the one instance, created on first use and kept for the life
	// time of the process.
	static CHoverPreviewWnd* Get();

	bool CreateIfNeeded(CWnd* pOwner);
	bool LoadContent(long lClipId, const CString& csInfo, bool bHasColorCode, COLORREF crColorCode, int nColorAlpha);
	bool LoadImage(long lClipId);
	bool LoadRtf(long lClipId);
	bool HasClipFormat(long lClipId, CLIPFORMAT cfType);
	// pixel size of the image plus, when the clip happens to hold jpeg data
	// as well, the exif of that jpeg
	void BuildImageInfo(long lClipId);
	bool LoadExif(long lClipId, CString& csText);
	void CleanDisplayText(CString& csText) const;
	// Frees the decoded content, called when the popup is hidden so that a
	// decoded image is not held on to while it is not shown
	void ReleaseContent();
	void UpdateFont();
	void Layout(const CString& csInfo, const CRect& rcAnchor);
	int MeasureTextHeight(const CString& csText, int nWidth, int nMaxHeight);
	int MeasureTextWidth(const CString& csText);
	int MeasureContentHeight(int nWidth, int nMaxHeight, const CString& csText);
	// DrawText only breaks lines at spaces, so a single long word (a url, a
	// path, a line of code) has to be wrapped first or it runs past the edge
	void WrapTextToWidth(CString& csText, int nWidth);
	// Replaces the decoded image with a copy at the given size.  Only the
	// scaled copy is ever drawn, so holding on to the full size decode would
	void ScaleBitmapTo(int nWidth, int nHeight);
	void DrawCheckerboard(CDC* pDC, const CRect& rc);

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

	static CHoverPreviewWnd* s_pInstance;

	HoverPreviewKind m_kind;
	Gdiplus::Bitmap* m_pBitmap;
	IFormattedTextDraw* m_pRtfDraw;
	CRichEditCtrlEx* m_pRtfNormalizer;
	COLORREF m_crColor;
	int m_nColorAlpha;
	CString m_csHtmlText;
	CString m_csClipText;
	CString m_csDetails;
	CString m_csInfo;
	CString m_csInfoExtra;
	CRect m_rcContent;
	CRect m_rcText;
	CFont m_font;
	CDPI m_dpi;
	long m_lClipId;
};
