#include "stdafx.h"
#include "ConvertRTFToText.h"
#include "..\Shared\TextConvert.h"


BOOL CConvertRTFToText::Create()
{
	// Get the class name and create the window
	CString szClassName = AfxRegisterWndClass(CS_CLASSDC | CS_SAVEBITS, LoadCursor(NULL, IDC_ARROW));

	// Create the window - just don't show it yet.
	if (!CWnd::CreateEx(WS_EX_NOACTIVATE, szClassName, _T(""), WS_POPUP,0, 0, 0, 0, NULL, 0, NULL))
	{
		return FALSE;
	}

	m_richEditTextConverter.Create(_T(""), _T(""), WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_AUTOHSCROLL, CRect(10, 10, 100, 200), this, 1);

	return TRUE;
}

CString CConvertRTFToText::GetTextFromRTF(CStringA rtf)
{
	m_richEditTextConverter.SetRTF(rtf);

	auto x = m_richEditTextConverter.GetRTF();

	int loops = 0;
	MSG msg;
	while (::PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		loops++;
		if (loops > 100)
			break;
	}

	loops = 0;
	while (::PeekMessage(&msg, m_richEditTextConverter.m_hWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		loops++;
		if (loops > 100)
			break;
	}

	Sleep(50);

	CString text = m_richEditTextConverter.GetText();

	return text;
}