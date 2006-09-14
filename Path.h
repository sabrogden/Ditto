// ==================================================================
// 
//  Path.h   
//  
//  Created:       03.03.2005
//
//  Copyright (C) Peter Hauptmann
//              
// ------------------------------------------------------------------
// 
/// \page pgDisclaimer Copyright & Disclaimer
/// 
/// Copyright (C) 2004-2005 Peter Hauptmann
///     all rights reserved
/// more info: http://www.codeproject.com/phShellPath.asp
/// Please contact the author with improvements / modifications.
/// 
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted under the following conditions: 
///     - Redistribution of source must retain the copyright above, 
///       and the disclaimer below.
///     - Modifications to the source should be marked clearly, to be 
///       distinguishable from the original sources.
/// 
/// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
/// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
/// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
/// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
/// SUCH DAMAGE.
/// 


#ifndef FILE_PH_PATH_H_200506202_INCLUDED_
#define FILE_PH_PATH_H_200506202_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


#include <comdef.h> // for _bstr_t extractor only, can be removed if you don't need CPath::GetBStr()
#include "shlwapi.h"


#pragma once





/// The nsPath namespace contains the CPath class and global helper functions.
namespace nsPath
{

// ---------------- DECLARATIONS --------------------------------------------

    // ----- CString Helpers ---------
void  Trim(CString & s);                    
TCHAR GetFirstChar(CString const & s);      ///< returns the first char of the string, or 0 if the string length is 0.
TCHAR GetLastChar(CString const & s);       ///< returns the last char of the string, or 0 if the string length is 0.


TCHAR GetDriveLetter(TCHAR ch);
TCHAR GetDriveLetter(LPCTSTR s);

CString QuoteSpaces(CString const & str);


// ==================================================================
//  ERootType
// ------------------------------------------------------------------
/// 
/// Recognized root types for a path. 
/// see nsPath::GetRootType "GetRootType" for more.
/// 
/// \c len refers to the rootLen parameter optionally returned by \c GetRootType.
/// 
enum ERootType
{
   rtNoRoot       =  0,    ///< no, or unknown root (\c len = 0)
   rtDriveRoot    =  1,    ///< a drive specification with backslash ("C:\", \c len = 3)
   rtDriveCur     =  2,    ///< a drive specification without backslash ("C:", \c len = 2)
//   rtPathRoot     =  3,    ///< a path root, i.e. staring with a single backslash (nyi, \c len = 1)
   rtLongPath     =  4,    ///< a UNC long path specification (e.g. "\\?\C:\", \c len is length of path root + 4),  no distinction for "root / current" is made
   rtServerOnly   =  5,    ///< a server only specification (e.g. "\\fileserv" or "\\fileserv\", \c len is the string length)
   rtServerShare  =  6,    ///< server + share specification ("\\server\share", \c len includes the backslash after share if given)
   rtProtocol     =  7,    ///< protocol, ("http://", \c len includes the "://" part)
   rtPseudoProtocol = 8,   ///< pseudo protocol (no slahes) ("mailto:", \c len includes the colon)
   rtServer         = 9,    ///< server with share following (for GetRootType(_,_,greedy=false)
};

ERootType GetRootType(LPCTSTR path, int * pLen, bool greedy = true); 



// ==================================================================
//  nsPath::EPathCleanup
// ------------------------------------------------------------------
/// Flags for nsPath::CPath::Cleanup
///
enum EPathCleanup
{
    epcTrim             =   1,      ///< trim outer whitespace
    epcUnquote          =   2,      ///< remove single or double quotes
    epcTrimInQuote      =   4,      ///< trim whitespaces inside quotes
    epcCanonicalize     =   8,      ///< Canonicalize (collapse "\\..\\" and "\\.\\")
    epcRemoveXXL        =   16,     ///< Remove "\\\\?\\" and "\\\\?\\UNC\\" markers
    epcSlashToBackslash = 32,   ///< replace forward slashes with backslashes
    epcMakePretty       =   64,     ///< Windows' idea of a pretty path
    epcRemoveArgs       =  128,     ///< calls PathRemoveArgs (before trimming/unquoting)
    epcRemoveIconLocation = 256,    ///< Remove Icon location from the path
    epcExpandEnvStrings   = 512,    ///< Expand environment strings

    epc_Default = epcTrim | 
                  epcUnquote | 
                  epcTrimInQuote | 
                  epcCanonicalize | 
                  epcRemoveXXL |
                  epcExpandEnvStrings, ///< default for CTors and Assignment operators
};


// ==================================================================
//  nsPath::EPathPacking
// ------------------------------------------------------------------
/// 
/// Flags for nsPath::CPath::GetStr
/// \note 
/// eppAutoQuote was ignored in Version 1.2 and before. 
/// It is fixed since V.1.3 (August 2005), /// but was removed from 
/// epp_Default for backward compatibility
///
enum EPathPacking
{
    eppAutoQuote        =   1,      ///< Quote the path if it contains spaces
    eppAutoXXL          =   2,      ///< If path length is > MAX_PATH, use "\\\\?\\" syntax
    eppBackslashToSlash =   4,      ///< turn backslashes into forward slashes

    epp_Default = eppAutoXXL,       
};




// ==================================================================
//  nsPath::CPath
// ------------------------------------------------------------------
/// 
/// 
///
class CPath
{
protected:
    CString     m_path;
    void        CAssign(CString const & src);

public:

    //@{ \name Construction
    CPath()                     {}
    CPath(LPCSTR path);             ///< Assigns \c path. CPath::Clean(epc_Default) is called for cleanup
    CPath(LPCWSTR path);            ///< Assigns \c path. CPath::Clean(epc_Default) is called for cleanup
    CPath(CString const & path);    ///< Assigns \c path  CPath::Clean(epc_Default) is called for cleanup
    CPath(CPath const & path);      ///< Assigns \c path to the path. Does \b not modify the assigned path!
    CPath(CString const & path, DWORD cleanup); ///< Assigns \c path, using custom cleanup options (see CPath::Clean)
    //@}


    //@{ \name Assignment
    CPath & operator=(LPCSTR rhs);  ///< Assigns \c path, and calls CPath::Clean(epc_Default)
    CPath & operator=(LPCWSTR rhs); ///< Assigns \c path, and calls CPath::Clean(epc_Default)

    CPath & operator=(CString const & rhs); ///< Assigns \c path, and calls CPath::Clean(epc_Default)
    CPath & operator=(CPath const & rhs);   ///< Assigns \c path Does \b not call CPath::Clean!
    CPath & Assign(CString const & str, DWORD cleanup = epc_Default);
    //@}

    //@{ \name Miscellaneous Query
    operator LPCTSTR () const   { return m_path.operator LPCTSTR(); } 
    int      GetLength() const  { return m_path.GetLength(); }  ///< returns the length of the path, in characters
    //@}

    //@{ \name Path concatenation
    CPath & operator &=(LPCTSTR rhs);   
    CPath & Append(LPCTSTR appendix);
    CPath & AddBackslash();
    CPath & RemoveBackslash();
    //@}

    //@{ \name Splitting into Path Segments
    CString ShellGetRoot() const; 
    CPath   GetPath(bool includeRoot  = true) const;
    CString GetName() const;
    CString GetTitle() const;
    CString GetExtension() const;
    ERootType GetRootType(int * len = 0, bool greedy = true) const;
    CString GetRoot(ERootType * rt = NULL, bool greedy = true) const;
    CString SplitRoot(ERootType * rt = NULL);
    int     GetDriveNumber();
    TCHAR   GetDriveLetter();
    //@}

    //@{ \name  Add / Modify / Remove parts
    CPath & AddExtension(LPCTSTR extension, int len = -1);
    CPath & RemoveExtension();
    CPath & RenameExtension(LPCTSTR newExt);
    CPath & RemoveFileSpec();
    //@}


    //@{ \name Cleanup
    CPath & Trim();
    CPath & Unquote();
    CPath & Canonicalize();
    CPath & ShrinkXXLPath();
    CPath & MakePretty();
    CPath & Clean(DWORD cleanup = epc_Default);
    //@}


    //@{ \name Extractors (with special "packing")
    CString GetStr(DWORD packing = epp_Default) const;
    _bstr_t GetBStr(DWORD packing = epp_Default) const;
    //@}
    
    //@{ \name Static checking (not accessing file system, see also GetRootType)
    bool    IsValid() const;

    bool    IsDot() const;
    bool    IsDotDot() const;
    bool    IsDotty() const;    // IsDot || IsDotDot

    bool    MatchSpec(LPCTSTR spec);
    bool    IsContentType(LPCTSTR contentType)  { return 0 != ::PathIsContentType(m_path, contentType); } ///< compare content type registered for this file, see also MSDN: PathIsContentType
    bool    IsFileSpec()                { return 0 != ::PathIsFileSpec(m_path);         } ///< true if path does not contain backslash, see MSDN: PathIsFileSpec
    bool    IsPrefix(LPCTSTR prefix)    { return 0 != ::PathIsPrefix(m_path, prefix);   } ///< checks if the path starts with a prefix like "C:\\", see MSDN: PathIsPrefix
    bool    IsRelative()                { return 0 != ::PathIsRelative(m_path);         } ///< returns true if the path is relative, see MSDN: PathIsRelative
    bool    IsRoot()                    { return 0 != ::PathIsRoot(m_path);             } ///< returns true if path is a directory root, see MSDN: PathIsRoot
    bool    IsSameRoot(LPCTSTR other)   { return 0 != ::PathIsSameRoot(m_path, other);  } ///< returns true if the path has the same root as \c otherPath, see MSDN: IsSameRoot

    bool    IsUNC()                     { return 0 != ::PathIsUNC(m_path);              } ///< returns true if the path is a UNC specification, see MSDN: PathIsUNC
    bool    IsUNCServer()               { return 0 != ::PathIsUNCServer(m_path);        } ///< returns true if the path is a UNC server specification, see MSDN: PathIsUNCServer
    bool    IsUNCServerShare()          { return 0 != ::PathIsUNCServerShare(m_path);   } ///< returns true if the path is a UNC server + share specification, see MSDN: PathIsUNCServerShare
    bool    IsURL()                     { return 0 != ::PathIsURL(m_path);              } ///< returns true if the path is an URL, see MSDN: PathIsURL

//  bool    IsHTMLFile()                { return 0 != ::PathIsHTMLFile(m_path);         } ///< (missing?) true if content type registered for this file is HTML, see MSDN: PathIsHTMLFile
//  bool    IsLFNFileSpec()             { return 0 != ::PathISLFNFileSpec(m_path);      } ///< (missing?) true if file is not a 8.3 file, see MSDN: PathIsLFNFileSpec
//  bool    IsNetworkPath()             { return 0 != ::PathIsNetworkPath(m_path);      } ///< (missing?) returns true if the path is on a network,  see MSDN: PathIsNetworkPath

    //@}


    //@{ \name Relative Paths
    CPath   GetCommonPrefix(LPCTSTR secondPath);
    CPath   RelativePathTo(LPCTSTR pathTo, bool srcIsDir = true);
    bool    MakeRelative(CPath const & basePath);
    bool    MakeAbsolute(CPath const & basePath);
    //@}

    //@{ \name Dialog control operations
    CString GetCompactStr(HDC dc, UINT dx, DWORD eppFlags = 0);
    CString GetCompactStr(UINT cchMax, DWORD eppFlags = 0, DWORD flags = 0);
    void    SetDlgItem(HWND dlg, UINT dlgCtrlID, DWORD eppFlags = 0);
    //@}


    //@{ \name File System / Environment-Dependent operations
    CPath & SearchAndQualify();
    CPath & FindOnPath(LPCTSTR * additionalDirs = 0);
    bool    Exists() const;
    bool    IsDirectory() const;
    bool    IsSystemFolder(DWORD attrib = FILE_ATTRIBUTE_SYSTEM) const;
    CPath & MakeSystemFolder(bool make = true);
    DWORD   GetAttributes();
    bool    GetAttributes(WIN32_FILE_ATTRIBUTE_DATA & fad);
    CPath & MakeFullPath(); 
    CPath & ExpandEnvStrings();
    bool    EnvUnexpandRoot(LPCTSTR envVar);
    bool    EnvUnexpandDefaultRoots();
    long    ToRegistry(HKEY baseKey, LPCTSTR subkey, LPCTSTR name, bool replaceEnv = true);
    //@}



    

// TODO: Shell 5.0 support
// V5:    CPath & UnexpandEnvStrings();
// V5:    LPCTSTR FindSuffixArray(LPCTSTR suffixes, int numSuffixes);
// V5:    void    PathUndecorate();
// V5:    CPath PathCreateFromURL(LPCTSTR path, DWORD dwReserved = 0);
// V5:    bool    IsDirectoryEmpty() const;
// might be useful for later extensions: PathGetCharType


};

// creation functions:
CPath SplitArgs(CString const & path_args, CString * args = NULL, DWORD cleanup = epc_Default);
CPath SplitIconLocation(CString const & path_icon, int * pIcon = NULL, DWORD cleanup = epc_Default);
CPath BuildRoot(int driveNumber);
CPath GetModuleFileName(HMODULE module = NULL);
CPath GetCurrentDirectory();
CPath FromRegistry(HKEY baseKey, LPCTSTR subkey, LPCTSTR name);


CString ReplaceInvalid(CString const & str, TCHAR replaceChar = '_');


// concatenation 

inline CPath operator & (CPath const & lhs, LPCTSTR rhs) { CPath ret = lhs; ret &= rhs; return ret; }







// ---------------- INLIME IMPLEMENTATIONS ----------------------------------

// ==============================================
// GetFirstChar
// ----------------------------------------------
/// \return [TCHAR]: the first char of the string, or 0 if the string length is 0.
/// \note The implementation takes care that the string is not copied when there are no spaces.
inline TCHAR GetFirstChar(CString const & s)
{
    if (s.GetLength() == 0)
        return 0;
    else
        return s[0];
}

// ==============================================
// GetLastChar
// ----------------------------------------------
/// \return [TCHAR]: the last character in the string, or 0 if the string length is 0.
/// \par Note
/// \b MBCS: if the string ends with a Multibyte character, this 
/// function returns the lead byte of the multibyte sequence.
inline TCHAR GetLastChar(CString const & s)
{
    LPCTSTR pstr = s;
    LPCTSTR pLastChar = _tcsdec(pstr, pstr + s.GetLength());
    if (pLastChar == NULL)
        return 0;
    else 
        return *pLastChar;
}





} // namespace nsPath




#endif // FILE_PH_PATH_H_200506202_INCLUDED_
