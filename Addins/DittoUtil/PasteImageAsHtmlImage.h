#pragma once

#include "..\..\Shared\DittoDefines.h"
#include "..\..\Shared\IClip.h"


class CPasteImageAsHtmlImage
{
public:
	CPasteImageAsHtmlImage(void);
	~CPasteImageAsHtmlImage(void);

	bool ConvertPathToHtmlImageTag(const CDittoInfo &DittoInfo, IClip *pClip);
	static bool CleanupPastedImages();

private:
	bool WriteDataToFile(CString csPath, LPVOID data, ULONG size);	
	static void CreateLocalPath(bool bCreateDir);
};
