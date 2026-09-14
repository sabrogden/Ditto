#include "stdafx.h"
#include "PreviewPane.h"
#include "CP_Main.h"
#include "Misc.h"
#include "ImageHelper.h"
#include "ImageViewerWnd.h"
#include "Sqlite\CppSQLite3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PREVIEW_TEXT_MAX_BYTES	4000
#define PREVIEW_MAX_FILES		12

#define ID_PREVIEW_EDIT			100
#define ID_PREVIEW_IMAGE		101
#define ID_PREVIEW_META			102

BEGIN_MESSAGE_MAP(CPreviewPane, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_CONTROL(STN_CLICKED, ID_PREVIEW_IMAGE, OnImageZoom)
END_MESSAGE_MAP()

CPreviewPane::CPreviewPane() :
	m_pDpi(NULL),
	m_clipId(-1),
	m_bHasMetadata(false),
	m_nTotalSize(0),
	m_bTruncatedText(false),
	m_pBitmap(NULL),
	m_hPreviewBmp(NULL),
	m_bFileListTruncated(false),
	m_nTextLength(-1),
	m_crBg(RGB(255, 255, 255)),
	m_crText(RGB(0, 0, 0)),
	m_crHeaderText(RGB(64, 64, 64)),
	m_bLastShowImage(false)
{
}

CPreviewPane::~CPreviewPane()
{
	ClearImage();
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
}

BOOL CPreviewPane::Create(CWnd* pParentWnd)
{
	DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN;
	CRect rc(0, 0, 0, 0);
	if (CreateEx(0, NULL, _T(""), dwStyle, rc, pParentWnd, 0) == FALSE)
	{
		Log(_T("PreviewPane Create - pane CreateEx failed"));
		return FALSE;
	}

	// read-only rich edit displaying header / formats / files / content
	DWORD dwEditStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL;
	if (m_edit.Create(NULL, NULL, dwEditStyle, CRect(0, 0, 0, 0), this, ID_PREVIEW_EDIT) == FALSE)
	{
		Log(_T("PreviewPane Create - edit create failed"));
		return FALSE;
	}
	UpdateFont();

	// standard static showing the image thumbnail when the clip is an image;
	// SS_REALSIZECONTROL makes it scale the bitmap into the rect we compute,
	// SS_NOTIFY so a click on it can open the full image viewer
	if (m_imgStatic.Create(_T(""), WS_CHILD | SS_BITMAP | SS_REALSIZECONTROL | SS_NOTIFY, CRect(0, 0, 0, 0), this, ID_PREVIEW_IMAGE) == FALSE)
	{
		Log(_T("PreviewPane Create - image static create failed"));
		return FALSE;
	}

	// pinned footer bar for the type/size/chars summary
	if (m_metaStatic.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE | SS_LEFTNOWORDWRAP,
		CRect(0, 0, 0, 0), this, ID_PREVIEW_META) == FALSE)
	{
		Log(StrF(_T("PreviewPane Create - meta static create failed, err=%d"), ::GetLastError()));
		return FALSE;
	}
	m_brBg.CreateSolidBrush(m_crBg);

	m_edit.SetBackgroundColor(FALSE, m_crBg);

	CHARFORMAT cf;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = m_crText;
	m_edit.SetDefaultCharFormat(cf);

	return TRUE;
}

void CPreviewPane::SetColors(COLORREF bg, COLORREF text, COLORREF headerText)
{
	m_crBg = bg;
	m_crText = text;
	m_crHeaderText = headerText;

	if (::IsWindow(m_edit.GetSafeHwnd()))
	{
		m_edit.SetBackgroundColor(FALSE, m_crBg);

		CHARFORMAT cf;
		memset(&cf, 0, sizeof(cf));
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = 0;
		cf.crTextColor = m_crText;
		m_edit.SetDefaultCharFormat(cf);
	}

	if (m_brBg.GetSafeHandle())
	{
		m_brBg.DeleteObject();
	}
	m_brBg.CreateSolidBrush(m_crBg);
	if (::IsWindow(m_metaStatic.GetSafeHwnd()))
	{
		m_metaStatic.Invalidate();
	}
}

// Secondary color derived by blending the text color toward the background.
// Keeps the summary line subtle but readable in any theme.
void CPreviewPane::SetColors(COLORREF bg, COLORREF text)
{
	const int blendPct = 45;
	COLORREF header = RGB(
		(GetRValue(bg) * blendPct + GetRValue(text) * (100 - blendPct)) / 100,
		(GetGValue(bg) * blendPct + GetGValue(text) * (100 - blendPct)) / 100,
		(GetBValue(bg) * blendPct + GetBValue(text) * (100 - blendPct)) / 100);
	SetColors(bg, text, header);
}

void CPreviewPane::ClearImage()
{
	if (m_hPreviewBmp)
	{
		// detach from the static first, drawing with a deleted handle leaves
		// garbage from the previous clip on screen
		if (::IsWindow(m_imgStatic.GetSafeHwnd()) && m_imgStatic.GetBitmap() == m_hPreviewBmp)
		{
			m_imgStatic.SetBitmap(NULL);
		}
		::DeleteObject(m_hPreviewBmp);
		m_hPreviewBmp = NULL;
	}
}

void CPreviewPane::SetClip(int clipId)
{
	if (clipId == m_clipId)
	{
		return;
	}

	m_clipId = clipId;

	// reset state
	m_bHasMetadata = false;
	m_formats.clear();
	m_nTotalSize = 0;
	m_csTextPreview.Empty();
	m_bTruncatedText = false;
	m_nTextLength = -1;
	m_fileNames.RemoveAll();
	m_bFileListTruncated = false;
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
	ClearImage();
	m_csImageFormatName.Empty();

	if (clipId > 0)
	{
		LoadMetadata(clipId);
		LoadTextPreview(clipId);
		LoadFileList(clipId);
		// image loaded in LoadMetadata after we know which image format exists
	}

	UpdateContent();
	LayoutChildren();
}

void CPreviewPane::Clear()
{
	SetClip(-1);
}

void CPreviewPane::LoadMetadata(int clipId)
{
	try
	{
		CString csSQL;
		csSQL.Format(_T("SELECT strClipBoardFormat, length(ooData) AS nDataSize ")
			_T("FROM Data WHERE lParentID = %d ORDER BY Data.lID desc"), clipId);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);

		CString csImageFormat;
		while (q.eof() == false)
		{
			FormatInfo fi;
			fi.m_csName = q.getStringField(_T("strClipBoardFormat"));
			fi.m_nSize = q.getIntField(_T("nDataSize"));
			m_nTotalSize += fi.m_nSize;
			m_formats.push_back(fi);

			if (csImageFormat.IsEmpty() &&
				(fi.m_csName.CompareNoCase(_T("CF_DIB")) == 0 ||
				 fi.m_csName.CompareNoCase(_T("CF_DIBV5")) == 0 ||
				 fi.m_csName.CompareNoCase(_T("PNG")) == 0 ||
				 fi.m_csName.CompareNoCase(_T("CF_BITMAP")) == 0))
			{
				csImageFormat = fi.m_csName;
			}

			q.nextRow();
		}

		m_bHasMetadata = true;

		if (csImageFormat.IsEmpty() == FALSE)
		{
			LoadImagePreview(clipId, csImageFormat);
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CPreviewPane::LoadTextPreview(int clipId)
{
	try
	{
		// always prefer CF_UNICODETEXT: CF_TEXT is a lossy re-encode and its
		// encoding is ambiguous (some apps hand us UTF-8 in CF_TEXT, so decoding
		// it as CP_ACP produces mojibake for non-ASCII text)
		CString csSQL;
		csSQL.Format(_T("SELECT strClipBoardFormat, ooData FROM Data ")
			_T("WHERE lParentID = %d AND (strClipBoardFormat = 'CF_UNICODETEXT' OR strClipBoardFormat = 'CF_TEXT') ")
			_T("ORDER BY CASE WHEN strClipBoardFormat = 'CF_UNICODETEXT' THEN 0 ELSE 1 END, Data.lID desc LIMIT 1"), clipId);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);
		if (q.eof() == false)
		{
			CString csFormat = q.getStringField(_T("strClipBoardFormat"));
			int nLen = 0;
			const unsigned char* pData = q.getBlobField(_T("ooData"), nLen);
			if (pData != NULL && nLen > 0)
			{
				int nMax = nLen < PREVIEW_TEXT_MAX_BYTES ? nLen : PREVIEW_TEXT_MAX_BYTES;
				m_bTruncatedText = nLen > nMax;

				if (csFormat.CompareNoCase(_T("CF_UNICODETEXT")) == 0)
				{
					int nChars = nMax / sizeof(wchar_t);
					wchar_t* pText = new wchar_t[nChars + 1];
					memcpy(pText, pData, nChars * sizeof(wchar_t));
					pText[nChars] = 0;
					m_csTextPreview = pText;
					delete[] pText;

					// count actual characters over the full blob, collapsing
					// surrogate pairs so emoji count as one character
					int nUnits = nLen / (int)sizeof(wchar_t);
					const wchar_t* pAll = (const wchar_t*)pData;
					int nCount = 0;
					for (int i = 0; i < nUnits; i++)
					{
						if (pAll[i] == 0)
						{
							break; // null terminator
						}
						if (pAll[i] >= 0xD800 && pAll[i] <= 0xDBFF && i + 1 < nUnits)
						{
							i++; // skip the low half of a surrogate pair
						}
						nCount++;
					}
					m_nTextLength = nCount;
				}
				else
				{
					int nChars = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pData, nMax, NULL, 0);
					if (nChars > 0)
					{
						wchar_t* pText = new wchar_t[nChars + 1];
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pData, nMax, pText, nChars);
						pText[nChars] = 0;
						m_csTextPreview = pText;
						delete[] pText;
					}

					int nCount = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pData, nLen, NULL, 0);
					if (nCount > 0 && ((const char*)pData)[nLen - 1] == 0)
					{
						nCount--; // null terminator
					}
					m_nTextLength = nCount;
				}
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CPreviewPane::LoadImagePreview(int clipId, const CString& csFormatName)
{
	try
	{
		CString csSQL;
		csSQL.Format(_T("SELECT ooData FROM Data ")
			_T("WHERE lParentID = %d AND strClipBoardFormat = '%s' ")
			_T("ORDER BY Data.lID desc LIMIT 1"), clipId, csFormatName);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);
		if (q.eof() == false)
		{
			int nLen = 0;
			const unsigned char* pData = q.getBlobField(_T("ooData"), nLen);
			if (pData != NULL && nLen > 0)
			{
				HGLOBAL hGlobal = NewGlobalP((LPVOID)pData, nLen);
				if (hGlobal)
				{
					if (csFormatName.CompareNoCase(_T("PNG")) == 0)
					{
						m_pBitmap = PNGImageHelper::GdipImageFromHGLOBAL(hGlobal);
					}
					else
					{
						m_pBitmap = DIBImageHelper::GdipImageFromHGLOBAL(hGlobal);
					}
					GlobalFree(hGlobal);

					// convert to a standard HBITMAP for the SS_BITMAP static control
					if (m_pBitmap &&
						m_pBitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &m_hPreviewBmp) != Gdiplus::Ok)
					{
						m_hPreviewBmp = NULL;
					}
				}

				m_csImageFormatName = csFormatName;
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

void CPreviewPane::LoadFileList(int clipId)
{
	try
	{
		CString csSQL;
		csSQL.Format(_T("SELECT ooData FROM Data ")
			_T("WHERE lParentID = %d AND strClipBoardFormat = 'CF_HDROP' ")
			_T("ORDER BY Data.lID desc LIMIT 1"), clipId);

		CppSQLite3Query q = theApp.m_db.execQuery(csSQL);
		if (q.eof() == false)
		{
			int nLen = 0;
			const unsigned char* pData = q.getBlobField(_T("ooData"), nLen);
			if (pData != NULL && nLen > (int)sizeof(DROPFILES))
			{
				DROPFILES* pDrop = (DROPFILES*)pData;
				if (pDrop->pFiles < (UINT)nLen)
				{
					LPBYTE pFiles = (LPBYTE)pData + pDrop->pFiles;
					if (pDrop->fWide)
					{
						wchar_t* pStr = (wchar_t*)pFiles;
						while (*pStr && m_fileNames.GetCount() < PREVIEW_MAX_FILES)
						{
							m_fileNames.Add(CString(pStr));
							pStr += wcslen(pStr) + 1;
						}
						if (*pStr)
						{
							m_bFileListTruncated = true;
						}
					}
					else
					{
						char* pStr = (char*)pFiles;
						while (*pStr && m_fileNames.GetCount() < PREVIEW_MAX_FILES)
						{
							m_fileNames.Add(CString(pStr));
							pStr += strlen(pStr) + 1;
						}
						if (*pStr)
						{
							m_bFileListTruncated = true;
						}
					}
				}
			}
		}
	}
	CATCH_SQLITE_EXCEPTION
}

CString CPreviewPane::FormatByteSize(__int64 nSize) const
{
	TCHAR szSize[128];
	StrFormatByteSize(nSize, szSize, 128);
	return CString(szSize);
}

CString CPreviewPane::GetFriendlyTypeName() const
{
	bool bText = false, bImage = false, bFiles = false, bRtf = false, bHtml = false;
	for (size_t i = 0; i < m_formats.size(); i++)
	{
		const CString& name = m_formats[i].m_csName;
		if (name.CompareNoCase(_T("CF_UNICODETEXT")) == 0 ||
			name.CompareNoCase(_T("CF_TEXT")) == 0)
		{
			bText = true;
		}
		else if (name.CompareNoCase(_T("CF_DIB")) == 0 ||
			name.CompareNoCase(_T("CF_DIBV5")) == 0 ||
			name.CompareNoCase(_T("PNG")) == 0 ||
			name.CompareNoCase(_T("CF_BITMAP")) == 0)
		{
			bImage = true;
		}
		else if (name.CompareNoCase(_T("CF_HDROP")) == 0)
		{
			bFiles = true;
		}
		else if (name.CompareNoCase(_T("RTF")) == 0)
		{
			bRtf = true;
		}
		else if (name.CompareNoCase(_T("HTML Format")) == 0)
		{
			bHtml = true;
		}
	}

	if (bFiles)
	{
		return theApp.m_Language.GetString("PreviewTypeFiles", "Files");
	}
	if (bImage)
	{
		return theApp.m_Language.GetString("PreviewTypeImage", "Image");
	}
	if (bRtf && !bText)
	{
		return theApp.m_Language.GetString("PreviewTypeRichText", "Rich text");
	}
	if (bHtml && !bText)
	{
		return theApp.m_Language.GetString("PreviewTypeWebPage", "Web page");
	}
	return theApp.m_Language.GetString("PreviewTypeText", "Text");
}

void CPreviewPane::UpdateFont()
{
	if (::IsWindow(m_edit.GetSafeHwnd()) == FALSE)
	{
		return;
	}

	// use the same user-configurable font as the main list, DPI scaled
	LOGFONT lf;
	CGetSetOptions::GetFont(lf);
	lf.lfHeight = m_pDpi ? m_pDpi->Scale(lf.lfHeight) : lf.lfHeight;

	m_Font.DeleteObject();
	m_Font.CreateFontIndirect(&lf);
	m_edit.SetFont(&m_Font);
	// UpdateFont is called from Create before the footer static exists
	if (::IsWindow(m_metaStatic.GetSafeHwnd()))
	{
		m_metaStatic.SetFont(&m_Font);
	}

	// changing the font resets rich edit formatting, re-apply the theme text color
	CHARFORMAT cf;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = m_crText;
	m_edit.SetDefaultCharFormat(cf);

	UpdateContent();
}

void CPreviewPane::UpdateContent()
{
	if (::IsWindow(m_edit.GetSafeHwnd()) == FALSE)
	{
		Log(_T("PreviewPane UpdateContent - edit not ready, skip"));
		return;
	}

	if (m_clipId <= 0)
	{
		if (::IsWindow(m_metaStatic.GetSafeHwnd()))
		{
			m_metaStatic.SetWindowText(_T(""));
		}
		m_edit.SetWindowText(theApp.m_Language.GetString("PreviewNoItemSelected", "No item selected"));
		return;
	}

	// one small gray summary line, pinned to the bottom footer bar
	CString csType = GetFriendlyTypeName();

	// Show the size of the representative format, not the sum of all stored
	// formats (a copied web link also carries an HTML blob, the plain text in
	// two encodings, etc., so the total is misleading to the user)
	__int64 nDisplaySize = m_nTotalSize;
	{
		const TCHAR* preferred[] = {
			_T("CF_UNICODETEXT"), _T("CF_TEXT"), _T("Rich Text Format"),
			_T("CF_DIB"), _T("CF_DIBV5"), _T("PNG"), _T("CF_BITMAP"),
			_T("HTML Format")
		};
		for (int i = 0; i < _countof(preferred); i++)
		{
			for (const FormatInfo& fi : m_formats)
			{
				if (fi.m_csName.CompareNoCase(preferred[i]) == 0)
				{
					nDisplaySize = fi.m_nSize;
					break;
				}
			}
			if (nDisplaySize != m_nTotalSize)
			{
				break;
			}
		}
	}

	CString csMeta;
	if (csType == theApp.m_Language.GetString("PreviewTypeFiles", "Files"))
	{
		CString csFmt = theApp.m_Language.GetString("PreviewFilesSummary", "%d files · %s");
		csMeta.Format(csFmt, m_fileNames.GetCount(), FormatByteSize(m_nTotalSize));
	}
	else
	{
		csMeta.Format(_T("%s · %s"), csType, FormatByteSize(nDisplaySize));
		if (m_nTextLength >= 0)
		{
			CString csChars;
			csChars.Format(theApp.m_Language.GetString("PreviewCharCount", "%d characters"), m_nTextLength);
			csMeta += _T(" · ") + csChars;
		}
		if (m_hPreviewBmp != NULL)
		{
			csMeta += _T(" · ") + theApp.m_Language.GetString("PreviewZoomHint", "click to zoom");
		}
	}

	// the summary lives in the pinned footer bar, the edit holds only content
	if (::IsWindow(m_metaStatic.GetSafeHwnd()))
	{
		m_metaStatic.SetWindowText(csMeta);
	}

	CString cs;

	if (m_fileNames.GetCount() > 0)
	{
		for (int i = 0; i < m_fileNames.GetCount(); i++)
		{
			cs += m_fileNames[i];
			cs += _T("\r\n");
		}
		if (m_bFileListTruncated)
		{
			cs += _T("...\r\n");
		}
	}

	if (m_csTextPreview.IsEmpty() == FALSE)
	{
		if (cs.IsEmpty() == FALSE)
		{
			cs += _T("\r\n");
		}
		cs += m_csTextPreview;
		if (m_bTruncatedText)
		{
			cs += _T("\r\n...");
		}
	}

	// suspend painting while doing the multi-step text update, otherwise each
	// step (set text, scroll) causes its own synchronous repaint and the pane
	// visibly flickers on every selection change
	m_edit.SetRedraw(FALSE);

	m_edit.SetWindowText(cs);

	// deselect and scroll back to the top so the content starts at the top
	m_edit.SetSel(0, 0);
	m_edit.LineScroll(-m_edit.GetLineCount());

	m_edit.SetRedraw(TRUE);
	m_edit.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}

HBRUSH CPreviewPane::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd != NULL && pWnd->GetSafeHwnd() == m_metaStatic.GetSafeHwnd())
	{
		pDC->SetTextColor(m_crHeaderText);
		pDC->SetBkColor(m_crBg);
		return m_brBg;
	}
	return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CPreviewPane::OnImageZoom()
{
	if (m_clipId > 0 && m_hPreviewBmp != NULL)
	{
		CImageViewerWnd::ShowForClip(m_clipId, this);
	}
}

void CPreviewPane::LayoutChildren()
{
	if (::IsWindow(m_edit.GetSafeHwnd()) == FALSE)
	{
		return;
	}

	CRect rcClient;
	GetClientRect(rcClient);

	int margin = m_pDpi ? m_pDpi->Scale(4) : 4;
	rcClient.DeflateRect(margin, margin, margin, margin);

	// carve out the pinned summary footer at the bottom, the content area
	// (image thumbnail + text) gets whatever is left above it
	int nFooterHeight = m_pDpi ? m_pDpi->Scale(20) : 20;
	int nFooterGap = m_pDpi ? m_pDpi->Scale(2) : 2;
	CRect rcMeta(rcClient.left, rcClient.bottom - nFooterHeight, rcClient.right, rcClient.bottom);
	CRect rcContent = rcClient;
	rcContent.bottom = rcMeta.top - nFooterGap;

	// show the image thumbnail on top when available
	CRect rcImage(0, 0, 0, 0);
	CRect rcEdit = rcContent;

	if (m_hPreviewBmp != NULL && m_pBitmap != NULL)
	{
		BITMAP bm;
		if (::GetObject(m_hPreviewBmp, sizeof(bm), &bm) > 0 &&
			bm.bmWidth > 0 && bm.bmHeight > 0)
		{
			double dScale = min((double)rcContent.Width() / bm.bmWidth, 1.0);
			int nDrawWidth = max(1, (int)(bm.bmWidth * dScale));
			int nDrawHeight = max(1, (int)(bm.bmHeight * dScale));

			// keep the thumbnail from eating the whole pane
			int nMaxHeight = rcContent.Height() / 2;
			if (nDrawHeight > nMaxHeight)
			{
				dScale = (double)nMaxHeight / nDrawHeight;
				nDrawWidth = max(1, (int)(nDrawWidth * dScale));
				nDrawHeight = nMaxHeight;
			}

			rcImage = CRect(rcContent.left, rcContent.top, rcContent.left + nDrawWidth, rcContent.top + nDrawHeight);
			rcEdit.top = rcImage.bottom + (m_pDpi ? m_pDpi->Scale(4) : 4);
		}
	}

	if (m_hPreviewBmp != NULL && rcImage.Height() > 0)
	{
		if (m_imgStatic.GetBitmap() != m_hPreviewBmp)
		{
			m_imgStatic.SetBitmap(m_hPreviewBmp);
		}
		if (rcImage != m_rcLastImage || m_bLastShowImage == false)
		{
			m_imgStatic.MoveWindow(rcImage);
			m_imgStatic.ShowWindow(SW_SHOW);
			m_rcLastImage = rcImage;
		}
		m_bLastShowImage = true;
	}
	else
	{
		if (m_bLastShowImage)
		{
			m_imgStatic.ShowWindow(SW_HIDE);
			m_imgStatic.SetBitmap(NULL);
			m_bLastShowImage = false;
		}
	}

	if (rcEdit != m_rcLastEdit)
	{
		m_edit.MoveWindow(rcEdit);
		m_rcLastEdit = rcEdit;
	}

	if (::IsWindow(m_metaStatic.GetSafeHwnd()) && rcMeta != m_rcLastMeta)
	{
		m_metaStatic.MoveWindow(rcMeta);
		m_rcLastMeta = rcMeta;
	}
}

BOOL CPreviewPane::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect(rc);
	pDC->FillSolidRect(rc, m_crBg);
	return TRUE;
}

void CPreviewPane::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	LayoutChildren();
}
