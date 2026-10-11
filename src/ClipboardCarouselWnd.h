#pragma once

#include "DPI.h"

class CClipboardCarouselWnd : public CWnd
{
public:
	CClipboardCarouselWnd();
	virtual ~CClipboardCarouselWnd();
	bool Create(CWnd* owner);
	void SetPreview(const CString& text, bool imagePreview = false);
	void SetImage(HBITMAP image);
	void ShowPreview();
	void HidePreview();
	void RefreshColors();
	CString SelectedText() const;
	bool HasFocus() const;
	bool IsImagePreview() const { return m_imagePreview; }
	// Image thumbnails are composed onto the content surface they are drawn on.
	COLORREF BackgroundColor() const { return m_surface; }
	virtual BOOL PreTranslateMessage(MSG* message);

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC*);
	afx_msg int OnMouseActivate(CWnd*, UINT, UINT);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctlColor);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	void LayoutPreview();
	void PositionText();
	void RecreateFont();
	void DeleteImage();
	void ApplyWindowStyle(bool highContrast);
	CRect SurfaceRect();
	CRect ContentRect();

	CDPI m_dpi;
	CEdit m_textEdit;
	CFont m_font;
	CBrush m_textBrush;
	CString m_text;
	HBITMAP m_image;
	bool m_imagePreview;
	bool m_positioning;
	POINT m_anchor;
	HMONITOR m_monitor;
	COLORREF m_background;
	COLORREF m_foreground;
	COLORREF m_border;
	COLORREF m_surface;
	COLORREF m_surfaceBorder;
	COLORREF m_muted;
	bool m_roundedByDwm;
};
