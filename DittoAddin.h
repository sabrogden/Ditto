#pragma once

#include "Shared\DittoDefines.h"
#include "Shared\IClip.h"
#include <vector>

class CDittoAddin
{
public:
	CDittoAddin();
	virtual ~CDittoAddin();

	bool DoLoad(LPCTSTR lpszDllName, CDittoInfo DittoInfo);

	std::vector<CFunction> m_PrePasteFunctions;
	bool PrePasteFunction(const CDittoInfo &DittoInfo, CStringA Function, IClip *pClip);

	CString DisplayName() { return m_DittoAddinInfo.m_Name; }
	CString LastError()	{ return m_csLastError; }

protected:
	TCHAR m_DllName[MAX_PATH];
	HMODULE m_hModule;
	CDittoAddinInfo m_DittoAddinInfo;
	CString m_csLastError;
	
protected:
	void Cleanup();

	bool (__cdecl *m_SupportedFunctions)(const CDittoInfo&, FunctionType,std::vector<CFunction>&);
	bool SupportedFunctions(const CDittoInfo &DittoInfo, FunctionType type, std::vector<CFunction> &Functions);	
};
