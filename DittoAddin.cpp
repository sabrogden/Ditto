#include "stdafx.h"
#include "DittoAddin.h"
#include "Misc.h"
#include "shared/TextConvert.h"

CDittoAddin::CDittoAddin() :
	m_hModule(NULL)
{
}

CDittoAddin::~CDittoAddin()
{
	Cleanup();
}

bool CDittoAddin::DoLoad(LPCTSTR lpszDllName, CDittoInfo DittoInfo)
{
	bool bLoaded = false;
	m_csLastError.Empty();
	Cleanup();

	if(lpszDllName)
	{
		_tcscpy(m_DllName, lpszDllName);

		m_hModule = ::LoadLibrary(lpszDllName);

		if( m_hModule )
		{
			bool (__cdecl *DittoAddin)(const CDittoInfo&, CDittoAddinInfo&);
			DittoAddin = (bool(__cdecl*)(const CDittoInfo&, CDittoAddinInfo&))GetProcAddress(m_hModule, "DittoAddin");
			if(DittoAddin)
			{
				bLoaded = DittoAddin(DittoInfo, m_DittoAddinInfo);
				if(bLoaded)
				{
					m_SupportedFunctions = (bool(__cdecl*)(const CDittoInfo&, FunctionType,std::vector<CFunction>&))GetProcAddress(m_hModule, "SupportedFunctions");
					SupportedFunctions(DittoInfo, eFuncType_PRE_PASTE, m_PrePasteFunctions);
				}
				else
				{
					m_csLastError.Format((_T("Ditto Addin - DittoAddin return false, not loading Addin")));
				}
			}
			else
			{
				m_csLastError.Format((_T("Ditto Addin - Failed to Get Function DittoAddin")));
			}
		}
		else
		{
			m_csLastError.Format((_T("Ditto Addin - Failed to load library on Addin %s"), lpszDllName));
		}
	}

	return bLoaded;
}

void CDittoAddin::Cleanup()
{
	if(m_hModule)
	{
		// release resources to the dll
		::FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

bool CDittoAddin::SupportedFunctions(const CDittoInfo &DittoInfo, FunctionType type, std::vector<CFunction> &Functions)
{
	bool bRet = false;
	m_csLastError.Empty();

	if(m_SupportedFunctions != NULL)
	{
		bRet = m_SupportedFunctions(DittoInfo, type, Functions);
		if(bRet)
		{
			int nCount = Functions.size();
			for(int i = 0; i < nCount; i++)
			{
				CFunction func = Functions[i];
				Log(StrF(_T("Ditto Addin - Supported Function Display: %s, Function: %s, Desc: %s"), func.m_csDisplayName, CTextConvert::MultiByteToUnicodeString(func.m_csFunction), func.m_csDetailDescription));
			}
		}
		else
		{
			m_csLastError = _T("Ditto Addin - m_SupportedFunctions returned false");
		}
	}
	else
	{
		m_csLastError = _T("Ditto Addin - m_SupportedFunctions is null, not call function load supported functions");
	}

	return bRet;
}

bool CDittoAddin::PrePasteFunction(const CDittoInfo &DittoInfo, CStringA Function, IClip *pClip)
{
	bool (__cdecl *PrePasteFunc)(const CDittoInfo &, IClip*);

	PrePasteFunc = (bool(__cdecl*)(const CDittoInfo &, IClip*))GetProcAddress(m_hModule, Function);
	if(PrePasteFunc)
	{
		return PrePasteFunc(DittoInfo, pClip);
	}

	return false;
}
