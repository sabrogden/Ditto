// OptionsCopyBuffers.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "OptionsCopyBuffers.h"


// COptionsCopyBuffers dialog

IMPLEMENT_DYNCREATE(COptionsCopyBuffers, CPropertyPage)

COptionsCopyBuffers::COptionsCopyBuffers() : CPropertyPage(COptionsCopyBuffers::IDD)
{
}

COptionsCopyBuffers::~COptionsCopyBuffers()
{
}

void COptionsCopyBuffers::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COPY_1, m_CopyBuffer1);
	DDX_Control(pDX, IDC_PASTE_1, m_PasteBuffer1);
	DDX_Control(pDX, IDC_CUT_1, m_CutBuffer1);
	DDX_Control(pDX, IDC_COPY_2, m_CopyBuffer2);
	DDX_Control(pDX, IDC_PASTE_2, m_PasteBuffer2);
	DDX_Control(pDX, IDC_CUT_2, m_CutBuffer2);
	DDX_Control(pDX, IDC_COPY_3, m_CopyBuffer3);
	DDX_Control(pDX, IDC_PASTE_3, m_PasteBuffer3);
	DDX_Control(pDX, IDC_CUT_3, m_CutBuffer3);
}


BEGIN_MESSAGE_MAP(COptionsCopyBuffers, CPropertyPage)
END_MESSAGE_MAP()

BOOL COptionsCopyBuffers::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CCopyBufferItem Item;
	
	g_Opt.GetCopyBufferItem(0, Item);
	theApp.m_pCopyBuffer1->CopyToCtrl(m_CopyBuffer1, m_hWnd, IDC_WIN_COPY_1);
	theApp.m_pPasteBuffer1->CopyToCtrl(m_PasteBuffer1, m_hWnd, IDC_WIN_PASTE_1);
	theApp.m_pCutBuffer1->CopyToCtrl(m_CutBuffer1, m_hWnd, IDC_WIN_CUT_1);
	CheckDlgButton(IDC_PLAY_SOUND_1, Item.m_bPlaySoundOnCopy);

	g_Opt.GetCopyBufferItem(1, Item);
	theApp.m_pCopyBuffer2->CopyToCtrl(m_CopyBuffer2, m_hWnd, IDC_WIN_COPY_2);
	theApp.m_pPasteBuffer2->CopyToCtrl(m_PasteBuffer2, m_hWnd, IDC_WIN_PASTE_2);
	theApp.m_pCutBuffer2->CopyToCtrl(m_CutBuffer2, m_hWnd, IDC_WIN_CUT_2);
	CheckDlgButton(IDC_PLAY_SOUND_2, Item.m_bPlaySoundOnCopy);

	g_Opt.GetCopyBufferItem(2, Item);
	theApp.m_pCopyBuffer3->CopyToCtrl(m_CopyBuffer3, m_hWnd, IDC_WIN_COPY_3);
	theApp.m_pPasteBuffer3->CopyToCtrl(m_PasteBuffer3, m_hWnd, IDC_WIN_PASTE_3);
	theApp.m_pCutBuffer3->CopyToCtrl(m_CutBuffer3, m_hWnd, IDC_WIN_CUT_3);
	CheckDlgButton(IDC_PLAY_SOUND_3, Item.m_bPlaySoundOnCopy);

	return TRUE;
}

BOOL COptionsCopyBuffers::OnApply()
{
	ARRAY keys;
	g_HotKeys.GetKeys(keys); // save old keys just in case new ones are invalid

	theApp.m_pCopyBuffer1->CopyFromCtrl(m_CopyBuffer1, m_hWnd, IDC_WIN_COPY_1);
	theApp.m_pPasteBuffer1->CopyFromCtrl(m_PasteBuffer1, m_hWnd, IDC_WIN_PASTE_1);
	theApp.m_pCutBuffer1->CopyFromCtrl(m_CutBuffer1, m_hWnd, IDC_WIN_CUT_1);

	theApp.m_pCopyBuffer2->CopyFromCtrl(m_CopyBuffer2, m_hWnd, IDC_WIN_COPY_2);
	theApp.m_pPasteBuffer2->CopyFromCtrl(m_PasteBuffer2, m_hWnd, IDC_WIN_PASTE_2);
	theApp.m_pCutBuffer2->CopyFromCtrl(m_CutBuffer2, m_hWnd, IDC_WIN_CUT_2);

	theApp.m_pCopyBuffer3->CopyFromCtrl(m_CopyBuffer3, m_hWnd, IDC_WIN_COPY_3);
	theApp.m_pPasteBuffer3->CopyFromCtrl(m_PasteBuffer3, m_hWnd, IDC_WIN_PASTE_3);
	theApp.m_pCutBuffer3->CopyFromCtrl(m_CutBuffer3, m_hWnd, IDC_WIN_CUT_3);

	CCopyBufferItem Item;
	g_Opt.GetCopyBufferItem(0, Item);
	Item.m_bPlaySoundOnCopy = IsDlgButtonChecked(IDC_PLAY_SOUND_1);
	g_Opt.SetCopyBufferItem(0, Item);

	g_Opt.GetCopyBufferItem(1, Item);
	Item.m_bPlaySoundOnCopy = IsDlgButtonChecked(IDC_PLAY_SOUND_2);
	g_Opt.SetCopyBufferItem(1, Item);

	g_Opt.GetCopyBufferItem(2, Item);
	Item.m_bPlaySoundOnCopy = IsDlgButtonChecked(IDC_PLAY_SOUND_3);
	g_Opt.SetCopyBufferItem(2, Item);

	int x;
	int y;
	ARRAY NewKeys;
	g_HotKeys.GetKeys(NewKeys);

	if(g_HotKeys.FindFirstConflict(NewKeys, &x, &y))
	{
		CString str =  g_HotKeys.ElementAt(x)->GetName();
		str += " and ";
		str += g_HotKeys.ElementAt(y)->GetName();
		str += " cannot be the same.";
		MessageBox(str);
		g_HotKeys.SetKeys(keys); // restore the original values
		return FALSE;
	}

	g_HotKeys.SaveAllKeys();
	g_HotKeys.RegisterAll(true);

	return CPropertyPage::OnApply();
}

