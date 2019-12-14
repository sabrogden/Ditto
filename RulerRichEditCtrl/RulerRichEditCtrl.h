#if !defined(AFX_RULERRICHEDITCTRL_H__4CD13283_82E4_484A_83B4_DBAD5B64F17C__INCLUDED_)
#define AFX_RULERRICHEDITCTRL_H__4CD13283_82E4_484A_83B4_DBAD5B64F17C__INCLUDED_

#include "RulerRichEdit.h"
#include "RRECRuler.h"
#include "RRECToolbar.h"

#include "ids.h"

#include "../RichEditCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// Helper structs

#ifdef _UNICODE
struct CharFormat : public CHARFORMATW
#else
struct CharFormat : public CHARFORMAT
#endif
{
	CharFormat()
	{
		memset( this, 0, sizeof ( CharFormat ) );
		cbSize = sizeof( CharFormat );
	};

};

struct ParaFormat : public PARAFORMAT
{
	ParaFormat( DWORD mask )
	{
		memset( this, 0, sizeof ( ParaFormat ) );
		cbSize = sizeof( ParaFormat );
		dwMask = mask;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CRulerRichEditCtrl window

class CRulerRichEditCtrl : public CWnd
{

public:
// Construction/creation/destruction
	CRulerRichEditCtrl();
	virtual ~CRulerRichEditCtrl();
	virtual BOOL Create( DWORD dwStyle, const RECT &rect, CWnd* pParentWnd, UINT nID, BOOL autohscroll = FALSE );

// Attributes
	void	SetMode( int mode );
	int		GetMode() const;

	void ShowToolbar( BOOL show = TRUE );
	void ShowRuler( BOOL show = TRUE );

	BOOL IsToolbarVisible() const;
	BOOL IsRulerVisible() const;

	CRichEditCtrl& GetRichEditCtrl( );

// Implementation
	CString GetRTF();
	void	SetRTF( const CString& rtf );
	void	SetText(CString sText);
	CString GetText();
	BOOL	Save( CString& filename );
	BOOL	Load( CString& filename );

	void SetReadOnly( BOOL readOnly );
	BOOL GetReadOnly() const;

// Formatting
	virtual void DoFont();
	virtual void DoColor();
	virtual void DoBold();
	virtual void DoItalic();
	virtual void DoUnderline();
	virtual void DoLeftAlign();
	virtual void DoCenterAlign();
	virtual void DoRightAlign();
	virtual void DoIndent();
	virtual void DoOutdent();
	virtual void DoBullet();
	virtual void DoWrap();

	void SetCurrentFontName( const CString& font );
	void SetCurrentFontSize( int points );
	void SetCurrentFontColor( COLORREF color );

// Overrides
	//{{AFX_VIRTUAL(CRulerRichEditCtrl)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

protected:
// Message handlers
	//{{AFX_MSG(CRulerRichEditCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonFont();
	afx_msg void OnButtonColor();
	afx_msg void OnButtonBold();
	afx_msg void OnButtonItalic();
	afx_msg void OnButtonUnderline();
	afx_msg void OnButtonLeftAlign();
	afx_msg void OnButtonCenterAlign();
	afx_msg void OnButtonRightAlign();
	afx_msg void OnButtonIndent();
	afx_msg void OnButtonOutdent();
	afx_msg void OnButtonBullet();
	afx_msg void OnButtonWrap();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnSetText (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetText (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetTextLength (WPARAM wParam, LPARAM lParam);
	afx_msg void OnLink(NMHDR* pnm, LRESULT* pResult);
	//}}AFX_MSG

	LRESULT OnTrackRuler(WPARAM mode, LPARAM pt);
	LRESULT OnGetScrollPos(WPARAM, LPARAM);
	LRESULT OnSetCurrentFontName(WPARAM font, LPARAM size);
	LRESULT OnSetCurrentFontSize(WPARAM font, LPARAM size);
	LRESULT OnSetCurrentFontColor(WPARAM font, LPARAM size);

	DECLARE_MESSAGE_MAP()

protected:
	// Internal data
	int				m_rulerPosition;	// The x-position of the ruler line when dragging a tab
	CPen			m_pen;				// The pen to use for drawing the XORed ruler line

	CDWordArray		m_tabs;				// An array containing the tab-positions in device pixels
	int				m_margin;			// The margin to use for the ruler and buttons

	int				m_physicalInch;		// The number of pixels for an inch on screen
	int				m_movingtab;		// The tab-position being moved, or -1 if none
	int				m_offset;			// Internal offset of the tab-marker being moved.

	BOOL			m_showToolbar;
	BOOL			m_showRuler;
	BOOL			m_readOnly;

	BOOL			m_bInWrapMode;

	// Sub-controls
	CRulerRichEdit	m_rtf;
	CRRECToolbar	m_toolbar;
	CRRECRuler		m_ruler;

	// Private helpers
	void	SetTabStops( LPLONG tabs, int size );
	void	UpdateTabStops();

	BOOL	CreateToolbar();
	BOOL	CreateRuler();
	BOOL	CreateRTFControl( BOOL autohscroll );
	void	CreateMargins();

	void	UpdateToolbarButtons();

	void	SetEffect( int mask, int effect );
	void	SetAlignment( int alignment );

	void	LayoutControls( int width, int height );

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif // !defined(AFX_RULERRICHEDITCTRL_H__4CD13283_82E4_484A_83B4_DBAD5B64F17C__INCLUDED_)
