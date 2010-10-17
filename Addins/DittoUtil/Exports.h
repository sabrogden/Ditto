#pragma once

#include "..\..\Shared\DittoDefines.h"
#include "..\..\Shared\IClip.h"
#include <vector>

extern "C" 
{
	bool __declspec(dllexport) DittoAddin(const CDittoInfo &DittoInfo, CDittoAddinInfo &info);
	bool __declspec(dllexport) SupportedFunctions(const CDittoInfo &DittoInfo, FunctionType type, std::vector<CFunction> &Functions);
	bool __declspec(dllexport) PasteAnyAsText(const CDittoInfo &DittoInfo, IClip *pClip);	
	bool __declspec(dllexport) ConvertPathToHtmlImageTag(const CDittoInfo &DittoInfo, IClip *pClip);
	bool __declspec(dllexport) ClearReadOnlyFlag(const CDittoInfo &DittoInfo, IClip *pClip);
	bool __declspec(dllexport) SetReadOnlyFlag(const CDittoInfo &DittoInfo, IClip *pClip);
	bool __declspec(dllexport) RemoveLineFeeds(const CDittoInfo &DittoInfo, IClip *pClip);
}