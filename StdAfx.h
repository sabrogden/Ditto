// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__56F3D184_7208_47FE_AFE2_E270325F356A__INCLUDED_)
#define AFX_STDAFX_H__56F3D184_7208_47FE_AFE2_E270325F356A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4995)

//remove 2005 warnings
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1

#define HITTEST_RET UINT

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxole.h>
#include <Winsock2.h>

#include "UnicodeMacros.h"

#include <imm.h>
#import "riched20.dll" raw_interfaces_only, raw_native_types, no_namespace, named_guids


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__56F3D184_7208_47FE_AFE2_E270325F356A__INCLUDED_)
