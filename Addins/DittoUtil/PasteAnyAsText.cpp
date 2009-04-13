#include "StdAfx.h"
#include ".\pasteanyastext.h"
#include "SelectPasteFormat.h"

PasteAnyAsText::PasteAnyAsText(void)
{
}

PasteAnyAsText::~PasteAnyAsText(void)
{
}

bool PasteAnyAsText::SelectClipToPasteAsText(const CDittoInfo &DittoInfo, IClip *pClip)
{
	bool ret = false;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	IClipFormats *pFormats = pClip->Clips();

	CWnd* pWnd = CWnd::FromHandle(DittoInfo.m_hWndDitto);
	CSelectPasteFormat dlg(pWnd, pFormats);

	if(dlg.DoModal() == IDOK)
	{
		//Find the format that was selected, remove all then readd the data as type CF_TEXT
		CLIPFORMAT format = dlg.SelectedFormat();
		if(format > 0)
		{
			IClipFormat *pText = pFormats->FindFormatEx(format);
			if(pText != NULL)
			{
				//We own the data, when we call DeleteAll tell it to not free the data
				pText->AutoDeleteData(false);
				HGLOBAL data = pText->Data();

				//Remove all over formats and add the selected date back as CF_TEXT
				pFormats->DeleteAll();

				pFormats->AddNew(CF_TEXT, data);

				IClipFormat *pText = pFormats->FindFormatEx(CF_TEXT);
				if(pText != NULL)
				{
					pText->AutoDeleteData(true);
				}

				ret = true;
			}
		}
	}

	return ret;
}