#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CProgressWnd : public CWnd
{
// Construction/Destruction
public:
    CProgressWnd();
    CProgressWnd(CWnd* pParent, LPCTSTR pszTitle, BOOL bSmooth = FALSE);
    virtual ~CProgressWnd();

    BOOL Create(CWnd* pParent, LPCTSTR pszTitle, BOOL bSmooth = FALSE);
    BOOL GoModal(LPCTSTR pszTitle =_T("Progress"), BOOL bSmooth = FALSE);

protected:
    void CommonConstruct();

// Operations
public:
    void SetRange(int nLower, int nUpper, int nStep = 1);
                                                    // Set range and step size
    int OffsetPos(int nPos);                        // Same as CProgressCtrl
    int StepIt();                                   //    "
    int SetStep(int nStep);                         //    "
    int SetPos(int nPos);                           //    "

    void SetText(LPCTSTR fmt, ...);                 // Set text in text area

    void Clear();                                   // Clear text, reset bar
    void Hide();                                    // Hide window
    void Show();                                    // Show window
	void HideCancel();
	void ShowCancel();
	int	 GetPos() const		{ return m_nPrevPos;	}

    BOOL Cancelled() { return m_bCancelled; }       // Was "Cancel" hit?

	void SetWindowSize(int nNumTextLines, int nWindowWidth = 390);

    void PeekAndPump(BOOL bCancelOnESCkey = TRUE);  // Message pumping for modal operation   

	void SetTitleText(CString csTitle);
    
// Implementation
protected:
    void GetPreviousSettings();
    void SaveCurrentSettings();

protected:
    BOOL m_bCancelled;
    BOOL m_bModal;
    BOOL m_bPersistantPosition;
    int  m_nPrevPos, m_nPrevPercent;
    int  m_nStep;
    int  m_nMaxValue, m_nMinValue;
    int  m_nNumTextLines;

    CStatic       m_Text;
    CProgressCtrl m_wndProgress;
    CButton       m_CancelButton;
    CString       m_strTitle,
                  m_strCancelLabel;
    CFont         m_font;


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CProgressWnd)
	public:
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
    //{{AFX_MSG(CProgressWnd)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
    afx_msg void OnCancel();
    DECLARE_MESSAGE_MAP()
};