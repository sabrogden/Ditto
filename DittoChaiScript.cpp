#include "stdafx.h"
#include "DittoChaiScript.h"

#include "Md5.h"
#include "Misc.h"

CDittoChaiScript::CDittoChaiScript(IClip *pClip, std::string activeApp)
{
	m_pClip = pClip;
	m_activeApp = activeApp;
}


CDittoChaiScript::~CDittoChaiScript()
{
}


std::string CDittoChaiScript::GetAsciiString()
{
	std::string s = "";
	if (m_pClip)
	{
		IClipFormat *pFormat = m_pClip->Clips()->FindFormatEx(CF_TEXT);
		if (pFormat)
		{
			char *stringData = (char *)GlobalLock(pFormat->Data());
			if (stringData != NULL)
			{
				s = stringData;

				GlobalUnlock(pFormat->Data());
			}
		}
	}

	return s;
}

void CDittoChaiScript::SetAsciiString(std::string stringVal)
{
	if (m_pClip)
	{
		m_pClip->Clips()->DeleteAll();

		//HGLOBAL hGlobal = ::NewGlobalP(stringVal.c_str, 4);
	//	ASSERT(hGlobal);

		//m_pClip->Clips()->AddNew(CF_TEXT, hGlobal);
	}
}

std::string CDittoChaiScript::GetClipMD5(int clipboardFormat)
{
	CMd5 md5;
	md5.MD5Init();

	std::string md5String;

	if (m_pClip)
	{
		IClipFormat *pFormat = m_pClip->Clips()->FindFormatEx(clipboardFormat);
		if (pFormat)
		{
			SIZE_T size = ::GlobalSize(pFormat->Data());
			void* pv = GlobalLock(pFormat->Data());
			if (pv != NULL)
			{				
				md5.MD5Update((unsigned char*)pv, (unsigned int)size);

				GlobalUnlock(pFormat->Data());

				md5String = md5.MD5FinalToString();
			}
		}
	}

	return md5String;
}

SIZE_T CDittoChaiScript::GetClipSize(int clipboardFormat)
{
	SIZE_T size = 0;
	if (m_pClip)
	{
		IClipFormat *pFormat = m_pClip->Clips()->FindFormatEx(clipboardFormat);
		if (pFormat)
		{
			size = ::GlobalSize(pFormat->Data());
		}
	}

	return size;
}