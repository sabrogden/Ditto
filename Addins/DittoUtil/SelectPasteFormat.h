#pragma once
#include "afxwin.h"
#include "resource.h"
#include "Exports.h"
#include "DialogResizer.h"

// CSelectPasteFormat dialog

class CSelectPasteFormat : public CDialog
{
	DECLARE_DYNAMIC(CSelectPasteFormat)

public:
	CSelectPasteFormat(CWnd* pParent, IClipFormats *clipFormats);   // standard constructor
	virtual ~CSelectPasteFormat();

	CLIPFORMAT SelectedFormat() const { return m_selectedFormat; }
	void SelectedFormat(CLIPFORMAT val) { m_selectedFormat = val; }
	bool PasteAsUnicode() { return m_pasteAsUnicode; }

// Dialog Data
	enum { IDD = IDD_DIALOG_SELECT_FORMAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	IClipFormats *m_pClipFormats;
	CLIPFORMAT m_selectedFormat;
	CDialogResizer m_Resize;
	bool m_pasteAsUnicode;
	
	DECLARE_MESSAGE_MAP()
	CListBox m_Formats;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
public:
	afx_msg void OnLbnDblclkList1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
