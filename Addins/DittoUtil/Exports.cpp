#include "StdAfx.h"
#include ".\exports.h"
#include "PasteAnyAsText.h"
#include ".\pasteimageashtmlimage.h"
#include "ReadOnlyFlag.h"


bool DittoAddin(const CDittoInfo &DittoInfo, CDittoAddinInfo &info)
{
	if(DittoInfo.ValidateSize() == false || info.ValidateSize() == false)
	{
		CString csError;
		csError.Format(_T("PasteAnyAsText Addin - Passed in structures are of different size, DittoInfo Passed: %d, Local: %d, DittoAddinInfo Passed: %d, Local: %d"), DittoInfo.m_nSizeOfThis, sizeof(CDittoInfo), info.m_nSizeOfThis, sizeof(CDittoAddinInfo));
		OutputDebugString(csError);
		return false;
	}

	info.m_Name = _T("Ditto Utils");
	info.m_AddinVersion = 1;

	return true;
}

bool SupportedFunctions(const CDittoInfo &DittoInfo, FunctionType type, std::vector<CFunction> &Functions)
{
	switch(type)
	{
	case eFuncType_PRE_PASTE:
		{
			CFunction func;
			func.m_csFunction = _T("PasteAnyAsText");
			func.m_csDisplayName = _T("Paste Any Clip As Text");
			func.m_csDetailDescription = _T("Displays a list of clip formats allowing you to select one and paste the contents as text");

			Functions.push_back(func);

			CFunction func2;
			func2.m_csFunction = _T("ConvertPathToHtmlImageTag");
			func2.m_csDisplayName = _T("Past As html image link");
			func2.m_csDetailDescription = _T("Converts a CF_DIB or CF_HDROP to a html format for pasting into outlook express");

			Functions.push_back(func2);

			CFunction func3;
			func3.m_csFunction = _T("ClearReadOnlyFlag");
			func3.m_csDisplayName = _T("Clear read only flag");
			func3.m_csDetailDescription = _T("Clears read only flag on the types CF_HDROP, or files paths in text");

			Functions.push_back(func3);

			CFunction func4;
			func4.m_csFunction = _T("SetReadOnlyFlag");
			func4.m_csDisplayName = _T("Set read only flag");
			func4.m_csDetailDescription = _T("Sets the read only flag on the types CF_HDROP, or files paths in text");

			Functions.push_back(func4);
		}
		break;
	}
	
	return true;
}

bool PasteAnyAsText(const CDittoInfo &DittoInfo, IClip *pClip)
{
	return PasteAnyAsText::SelectClipToPasteAsText(DittoInfo, pClip);
}

bool ConvertPathToHtmlImageTag(const CDittoInfo &DittoInfo, IClip *pClip)
{
	CPasteImageAsHtmlImage convert;
	return convert.ConvertPathToHtmlImageTag(DittoInfo, pClip);
}

bool ClearReadOnlyFlag(const CDittoInfo &DittoInfo, IClip *pClip)
{
	CReadOnlyFlag readOnly;
	readOnly.ResetReadOnlyFlag(DittoInfo, pClip, true);

	//return false so the clip is not pasted
	return false;
}

bool SetReadOnlyFlag(const CDittoInfo &DittoInfo, IClip *pClip)
{
	CReadOnlyFlag readOnly;
	readOnly.ResetReadOnlyFlag(DittoInfo, pClip, false);

	//return false so the clip is not pasted
	return false;
}
