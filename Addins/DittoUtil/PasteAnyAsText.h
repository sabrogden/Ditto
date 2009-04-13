#pragma once
#include "..\..\Shared\DittoDefines.h"
#include "..\..\Shared\IClip.h"


class PasteAnyAsText
{
public:
	PasteAnyAsText(void);
	~PasteAnyAsText(void);

	static bool SelectClipToPasteAsText(const CDittoInfo &DittoInfo, IClip *pClip);
};
