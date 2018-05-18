/////////////////////////////////////////////////////////////////////////////
// SimpleBrowser: Web browser control
/////////////////////////////////////////////////////////////////////////////

#if !defined(SimpleBrowser_defined)
#define SimpleBrowser_defined

#include "mshtml.h"

class SimpleBrowser : public CWnd {

public:

	// construction and creation

    SimpleBrowser();
    virtual ~SimpleBrowser();

	BOOL Create(DWORD dwStyle, 
	            const RECT& rect, 
				CWnd* pParentWnd, 
				UINT nID);
		
		// create browser directly

	BOOL CreateFromControl(CWnd *pParentWnd,UINT nID,DWORD dwStyle = WS_CHILD | WS_VISIBLE);

		// create browser in place of dialog control; the dialog control 
		// identified by nID will be destroyed, and the browser will take
		// its place

    // controls

	void Navigate(LPCTSTR URL);
	
		// navigate to URL

	void Write(LPCTSTR string);				
	
		// append string to current document; note that the WebBrowser control tolerates
		// poorly formed documents, like:

		// <html><body>....
		// --- no trailing body or html tags

		// <html><body>...</body></html><html><body>...</body></html>...
		// --- multiple documents

	void Clear();							
	
		// clear current document

	void NavigateResource(int resource_ID);
	
		// navigate to HTML document resource

	void GoBack();							// navigate backward one item
											// in the history list

    void GoForward();						// navigate forward one item
											// in the history list

    void GoHome();							// navigate to current 
											// home or start page

    void Refresh();							// refresh contents

    void Stop();							// stop current activity

    void Print(LPCTSTR header = _T("&w&b&b&p"),
			   LPCTSTR footer = _T("&d &t"));
			   
		// start printing contents; uses same 'metacharacters' for header and
		// footer as Internet Explorer; see IE Page Setup dialog

	void PrintPreview();					// print preview

    bool GetBusy();                         // returns true if browser
                                            // busy downloading or other
                                            // activity

    CString GetLocationName();              // get name of location currently
                                            // being browsed (title, if HTML
                                            // page; UNC path if file)

    CString GetLocationURL();               // get URL of location currently
                                            // being browsed

    READYSTATE GetReadyState();             // get browser ready state

    bool GetSilent();                       // get/set silent property
    void PutSilent(bool silent = false);	// (if true, dialog and message
                                            //  boxes may not be shown)

	IHTMLDocument2 *GetDocument();			// get document interface; returns NULL 
											// if interface is not available 
											// (which is the case if you've navigated to
											//  something that's NOT an HTML document,
											//  like an Excel spreadsheet, which the
											//  WebBrowser control is perfectly willing
											//  to host)

    // events (overridables)

    virtual bool OnBeforeNavigate2(CString	URL,
	                               CString	frame,
								   void		*post_data,int post_data_size,
								   CString	headers);

        // called before navigation begins; URL is destination, frame
        // is frame name ("" if none), post_data is HTTP POST data (NULL if none),
		// and headers are HTTP headers sent to server;
		// return true to cancel navigation, false to continue

    virtual void OnDocumentComplete(CString URL);

        // navigation to document complete; URL is location

    virtual void OnDownloadBegin();
	
		// navigation operation begins

    virtual void OnProgressChange(int progress,int progress_max);

         // navigation progress update

    virtual void OnDownloadComplete();
	
	     // navigation operation completed

    virtual void OnNavigateComplete2(CString URL);

        // navigation to hyperlink complete; URL is location
		// (URL = string if NavigateString or NavigateResource are used)

    virtual void OnStatusTextChange(CString text);

        // status text has changed

    virtual void OnTitleChange(CString text);

		// title has changed

	virtual void OnPrintTemplateInstantiation();
	
		// print template has been instantiated (printing has begun)
		
	virtual void OnPrintTemplateTeardown();
	
		// print template is being destroyed (printing has completed)		

	// notifications
	
	enum NotificationType {					// Note: SimpleBrowser does NOT support the
											//       common notifications (NM_CLICK, etc.)

		BeforeNavigate2,					// set *LRESULT=TRUE to cancel navigation
		DocumentComplete,				
		DownloadBegin,
		ProgressChange,
		DownloadComplete,
		NavigateComplete2,
		StatusTextChange,
		TitleChange,
		PrintTemplateInstantiation,
		PrintTemplateTeardown

	};
	
	class Notification {					// all notifications pass this structure

	public:

		Notification(HWND hwnd,UINT ID,NotificationType type);
		~Notification();

		NMHDR			hdr;				// hdr.hwndFrom = SimpleBrowser's HWND
											// hdr.idFrom   = SimpleBrowser's control ID
											// hdr.code     = <NavigationType>

		CString			URL;				// BeforeNavigate2
											// DocumentComplete
											// NavigateComplete2

		CString			frame;				// BeforeNavigate2
		void			*post_data;			// BeforeNavigate2
		int				post_data_size;
		CString			headers;			// BeforeNavigate2

		int				progress;			// ProgressChange
		int				progress_max;		// ProgressChange

		CString			text;				// StatusTextChange
											// TitleChange

	};

	bool ParsePostData(CString post_data,		// parse URL-encoded POST data
	                   CStringArray &names,		// string into list of names
	                   CStringArray &values);	// and values; returns true if
												// successful

    //{{AFX_VIRTUAL(SimpleBrowser)
	public:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

protected:

    //{{AFX_MSG(SimpleBrowser)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL PreTranslateMessage(MSG *pMsg);
	//}}AFX_MSG

    void _OnBeforeNavigate2(LPDISPATCH lpDisp,
                            VARIANT FAR *URL,
                            VARIANT FAR *Flags,
                            VARIANT FAR *TargetFrameName,
                            VARIANT FAR *PostData,
                            VARIANT FAR *Headers,
                            VARIANT_BOOL *Cancel);
    void _OnDownloadBegin();
    void _OnProgressChange(long progress,long progress_max);
    void _OnDownloadComplete();
    void _OnDocumentComplete(LPDISPATCH lpDisp,VARIANT FAR* URL);
    void _OnNavigateComplete2(LPDISPATCH lpDisp,VARIANT FAR* URL);
    void _OnStatusTextChange(BSTR bstrText);
    void _OnTitleChange(BSTR bstrText);
	void _OnPrintTemplateInstantiation(LPDISPATCH pDisp);
	void _OnPrintTemplateTeardown(LPDISPATCH pDisp);

    DECLARE_MESSAGE_MAP()
    DECLARE_EVENTSINK_MAP()

private:

    CWnd                    _BrowserWindow;     // browser window

protected:

	IWebBrowser2			*_Browser;          // browser control

	bool					_Ready;				// document ready
												// (initial navigation completed)

private:    
    
    IDispatch               *_BrowserDispatch;  // browser control 
												// dispatch interface

	CString					_Content;			// current content set via Write()/Clear()
	
	void _ContentWrite();

};

#endif
