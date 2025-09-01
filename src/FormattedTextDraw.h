// Feel free to use this code in your own applications.
// The Author does not guarantee anything about this code.
// Author : Yves Maurer

// FormattedTextDraw.h : Declaration of the CFormattedTextDraw

#ifndef __FORMATTEDTEXTDRAW_H_
#define __FORMATTEDTEXTDRAW_H_

#include "../resource.h"       // main symbols

// be sure to link this project with riched20.lib
#include <richedit.h>
#include <textserv.h>

#ifndef LY_PER_INCH
#define LY_PER_INCH   1440
#define HOST_BORDER 0
#endif

typedef struct tagCOOKIE
{
	BSTR	bstrText;
	DWORD	dwSize;
	DWORD	dwCount;
} COOKIE, *PCOOKIE;

// These definitions might give you problems.  If they do, open your
// textserv.h and comment the following lines out. They are right at the top
// EXTERN_C const IID IID_ITextServices;
// EXTERN_C const IID IID_ITextHost;

const IID IID_ITextServicesEx = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
    0x8d33f740,
    0xcf58,
    0x11ce,
    {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
 };

//const IID IID_ITextHosts = { // c5bdd8d0-d26e-11ce-a89e-00aa006cadc5
//    0xc5bdd8d0,
//    0xd26e,
//    0x11ce,
//    {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
//  };

/////////////////////////////////////////////////////////////////////////////
// IFormatttedTextDraw
interface IFormattedTextDraw
{
public:
	virtual ~IFormattedTextDraw() {};
	virtual HRESULT get_NaturalHeight(long Width, /*[out, retval]*/ long *pVal) = 0;
	virtual HRESULT get_NaturalWidth(long Height, /*[out, retval]*/ long *pVal) = 0;
	virtual HRESULT Create() = 0;
	virtual HRESULT Draw(void *hdcDraw, RECT *prc) = 0;
	virtual HRESULT get_RTFText(/*[out, retval]*/ BSTR *pVal) = 0;
	virtual HRESULT put_RTFText(/*[in]*/ BSTR newVal) = 0;

// COM-like functions
    virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
    virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// CFormattedTextDraw
class CFormattedTextDraw : 
	public ITextHost,
	public IFormattedTextDraw
{
public:
	CFormattedTextDraw()
	{
		HDC hdcScreen;

		m_RTFText = NULL;

		hdcScreen = GetDC(NULL);
		nPixelsPerInchX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
		nPixelsPerInchY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
		ReleaseDC(NULL, hdcScreen);

		SetRectEmpty(&m_rcClient);
		SetRectEmpty(&m_rcViewInset);

		m_pCF = (CHARFORMAT2W*) malloc(sizeof(CHARFORMAT2W));

		InitDefaultCharFormat();
		InitDefaultParaFormat();
		m_spTextServices = NULL;
		m_spTextDocument = NULL;

		m_dwPropertyBits = TXTBIT_RICHTEXT | TXTBIT_MULTILINE | TXTBIT_WORDWRAP | TXTBIT_USECURRENTBKG;
		m_dwScrollbar = 0;
		m_dwMaxLength = INFINITE;
	}

	~CFormattedTextDraw()
	{
		if (m_RTFText != NULL)
			SysFreeString(m_RTFText);

		free(m_pCF);
		if (m_spTextServices != NULL)
			m_spTextServices->Release();
		if (m_spTextDocument != NULL)
			m_spTextDocument->Release();
	}

// Minimal COM functionality
    HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		*ppvObject = NULL;
		return S_FALSE;
	}
    
    ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 0;
	}
    
    ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}

// IFormattedTextDraw
public:
	HRESULT get_NaturalHeight(long Width, /*[out, retval]*/ long *pVal);
	HRESULT get_NaturalWidth(long Height, /*[out, retval]*/ long *pVal);
	HRESULT Create();
	HRESULT Draw(void *hdcDraw, RECT *prc);
	HRESULT get_RTFText(/*[out, retval]*/ BSTR *pVal);
	HRESULT put_RTFText(/*[in]*/ BSTR newVal);

// ITextHost
	HDC TxGetDC();
	INT TxReleaseDC(HDC hdc);
	BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
	BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);
	BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
	BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);
	void TxInvalidateRect(LPCRECT prc, BOOL fMode);
	void TxViewChange(BOOL fUpdate);
	BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
	BOOL TxShowCaret(BOOL fShow);
	BOOL TxSetCaretPos(INT x, INT y);
	BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
	void TxKillTimer(UINT idTimer);
	void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
	void TxSetCapture(BOOL fCapture);
	void TxSetFocus();
	void TxSetCursor(HCURSOR hcur, BOOL fText);
	BOOL TxScreenToClient(LPPOINT lppt);
	BOOL TxClientToScreen(LPPOINT lppt);
	HRESULT	TxActivate(LONG * plOldState);
	HRESULT	TxDeactivate(LONG lNewState);
	HRESULT	TxGetClientRect(LPRECT prc);
	HRESULT	TxGetViewInset(LPRECT prc);
	HRESULT TxGetCharFormat(const CHARFORMATW **ppCF);
	HRESULT	TxGetParaFormat(const PARAFORMAT **ppPF);
	COLORREF TxGetSysColor(int nIndex);
	HRESULT	TxGetBackStyle(TXTBACKSTYLE *pstyle);
	HRESULT	TxGetMaxLength(DWORD *plength);
	HRESULT	TxGetScrollBars(DWORD *pdwScrollBar);
	HRESULT	TxGetPasswordChar(TCHAR *pch);
	HRESULT	TxGetAcceleratorPos(LONG *pcp);
	HRESULT	TxGetExtent(LPSIZEL lpExtent);
	HRESULT OnTxCharFormatChange(const CHARFORMATW * pcf);
	HRESULT	OnTxParaFormatChange(const PARAFORMAT * ppf);
	HRESULT	TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);
	HRESULT	TxNotify(DWORD iNotify, void *pv);
	HIMC TxImmGetContext();
	void TxImmReleaseContext(HIMC himc);
	HRESULT	TxGetSelectionBarWidth(LONG *lSelBarWidth);

// Custom functions
	HRESULT CharFormatFromHFONT(CHARFORMAT2W* pCF, HFONT hFont);
	HRESULT InitDefaultCharFormat();
	HRESULT InitDefaultParaFormat();
	HRESULT CreateTextServicesObject();

// Variables
	RECT			m_rcClient;			// Client Rect
	RECT			m_rcViewInset;		// view rect inset
	SIZEL			m_sizelExtent;		// Extent array

	int				nPixelsPerInchX;    // Pixels per logical inch along width
	int				nPixelsPerInchY;    // Pixels per logical inch along height

	CHARFORMAT2W	*m_pCF;
	PARAFORMAT2		m_PF;
	DWORD			m_dwScrollbar;		// Scroll bar style
	DWORD			m_dwPropertyBits;	// Property bits
	DWORD			m_dwMaxLength;
	COOKIE			m_editCookie;

	ITextServices	*m_spTextServices;
	ITextDocument	*m_spTextDocument;
	BSTR			m_RTFText;
};

#endif //__FORMATTEDTEXTDRAW_H_
