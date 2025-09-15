#ifndef _RRECRULER_H_
#define _RRECRULER_H_

class CRRECRuler : public CWnd 
{

public:
// Construction/creation/destruction
	CRRECRuler();
	virtual ~CRRECRuler();
	virtual BOOL Create( const CRect& rect, CWnd* parent, UINT id );

// Implementation
	void	SetMode( int mode );
	int		GetMode() const;
	void	SetMargin( int margin );
	void	SetTabStops( const CDWordArray& arr );

protected:
// Message handlers
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
// Internal data
	int m_margin;		// Left margin of ruler in pixels
	int m_mode;			// MODE_INCH/MODE_METRIC, what units 
						// to use for the ruler measure.
	int m_physicalInch;	// The number of device pixels for a 
						// physical inch of the window CDC

	CDWordArray	m_tabs;	// Tabulator settings in device pixels
};

#endif // _RRECRULER_H_