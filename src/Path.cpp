// ==================================================================
// 
//  Path.cpp   
//  
//  Created:       20.06.2004
//
//  Copyright (C) Peter Hauptmann
//              
// ------------------------------------------------------------------
/// 
/// for copyright & disclaimer see accompanying Path.h
/// 
/// \page pgChangeLog Change Log
/// 
/// June 20, 2004:  Initial release
/// 
/// June 22, 2004   
///     - \c fixed: nsPath::CPath::MakeSystemFolder implements unmake correctly 
///     - \c added: nsPath::CPath::MakeSystemFolder and nsPath::CPath::SearchOnPath 
///       set the windows error code to zero if the function succeeds (thanks Hans Dietrich)
///     - \c fixed: nsPath::CPath compiles correctly with warning level -W4
/// 
/// Mar 3, 2005
///     - fixed eppAutoQuote bug in GetStr (thanks Stlan)
///     - Added: 
///         - \ref nsPath::FromRegistry "FromRegistry"
///         - \ref nsPath::CPath::ToRegistry "ToRegistry"
///         - \ref nsPath::CPath::GetRootType, "GetRootType"
///         - \ref nsPath::CPath::GetRoot "GetRoot" has a new implementation
///         - \ref nsPath::CPath::MakeAbsolute "MakeAbsolute"
///         - \ref nsPath::CPath::MakeRelative "MakeRelative"
///         - \ref nsPath::CPath::MakeFullPath "MakeFullPath"
///         - \ref nsPath::CPath::EnvUnexpandRoot "EnvUnexpandRoot"
///         - \ref nsPath::CPath::EnvUnexpandDefaultRoots "EnvUnexpandDefaultRoots"
///     - \b Breaking \b Changes (sorry)
///         - GetRoot -> ShellGetRoot (to distinct from extended GetRoot implementation)
///         - GetFileName --> GetName (consistency)
///         - GetFileTitle --> GetTitle (consistency)
///         - made the creation functions independent functions in the nsPath namespace
///           (they are well tugged away in the namespace so conflicts are easy to avoid) 
/// 
/// Mar 17, 2005
///     - fixed bug in GetFileName (now: GetName): If the path ends in a backslash,
///       GetFileName did return the entire path instead of an empty string. (thanks woodland)
/// 
/// Aug 21, 2005
///     - fixed bug in GetStr(): re-quoting wasn't applied (doh!)
///     - fixed incompatibility with CStlString
///     - Added IsDot, IsDotDot, IsDotty (better names?)
///     - Added IsValid, ReplaceInvalid
///     - Added SplitRoot
///     - 
///        
/// 
///
// ------ Main Page --------------------
/// @mainpage
/// 
/// \ref pgDisclaimer, \ref pgChangeLog (recent changes August 2005, \b breaking \b changes Mar 2005)
///
///	\par Introduction
///
/// \ref nsPath::CPath "CPath" is a helper class to make manipulating file system path strings easier. 
/// It is complementedby a few non-class functions (see nsPath namespace documentation)
/// 
/// CPath is based on the Shell Lightweight Utility API, but modifies / and extends this functionality
/// (and removes some quirks). It requires CString (see below why, and why this is not too bad).
/// 
/// \par Main Features:
/// 
/// CPath acts as a string class with special "understanding" and operations for path strings.
/// 
/// \code CPath path("c:\\temp"); \endcode
/// constructs a path string, and does some default cleanup (trimming white space, removing quotes, etc.)
/// The cleanup can be customized (see \ref nsPath::CPath::CPath "CPath Constructor", 
///  \ref nsPath::EPathCleanup "EPathCleanup"). You can pass an ANSI or UNICODE string.
/// 
/// \code path = path & "file.txt"; \endcode
/// Appends "foo" to the path, making sure that the two segments are separated by exactly one backslash
/// no matter if the parts end/begin with a backslash.
/// 
/// The following functions give you access to the individual elements of the path:
/// 
///  - \ref nsPath::CPath::GetRoot "GetRoot"
///  - \ref nsPath::CPath::GetPath "GetPath"
///  - \ref nsPath::CPath::GetName "GetName"
///  - \ref nsPath::CPath::GetTitle "GetTitle"
///  - \ref nsPath::CPath::GetExtension "GetExtension"
/// 
/// \code CString s = path.GetStr() \endcode
/// returns a CString that is cleaned up again (re-quoted if necessary, etc). GetBStr() returns an _bstr_t
/// with the same features (that automatically casts to either ANSI or UNICODE).
/// To retrieve the unmodified CPath string, you can rely on the \c operator \c LPCTSTR 
/// 
/// There's much more - see the full nsPath documentation for details!
/// 
///	@sa MSDN library: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/shlwapi/path/pathappend.asp
/// 
/// \par Why not CPathT ?
///  -# the class is intended for a VC6 project that won't move to VC7 to soon
///  -# CPathT contains the same quirks that made me almost give up on the Shell Helper functions. 
///  -# I wanted the class to have additional features (such as the & operator, and automatic cleanup)
/// 
/// \par Why CString ?
///  -# The CString implementation provides a known performance (due to the guaranteed reference counted "copy 
///     on write" implementation). I consider this preferrable over the weaker guarantees made b STL, especially
///     when designing a  "convenient" class interface.
///  -# CString's ref counting mechanism is automatically reused by CPath, constructing a CPath from a CString
///     does not involve a copy operation.
///  -# CString is widely availble independent of MFC (WTL, custom implementations, "extract" macros are
///     available, and VC7 makes CString part of the ATL)
/// 
/// \note if you want to port to STL, it's probably easier to use a vector<TCHAR> instead of std:string 
///   to hold the data internally
/// 
/// \par Why _bstr_t ?
/// To make implementation easier, the class internally works with "application native" strings (that is, 
/// TCHAR strings - which are either ANSI or UNICODE depending on a compile setting). GetBStr provides
/// conversion to ANSI or UNICODE, whichever is required.\n
/// An independent implementation would return a temporary object with cast operators to LPCSTR and LPWSTR 
/// - BUT _bstr_t does exactly that (admittedly, with some overhead). 
/// 


#include "stdafx.h"
#include "Path.h"

#include <shlwapi.h>    // Link to Shell Helper API
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace nsPath
{

/// contains helper classes for nsPath namespace
/// 
namespace nsDetail
{
// ==================================================================
//  CStringLock
// ------------------------------------------------------------------
// 
// Helper class for CString::GetBuffer and CString::ReleaseBuffer
// \todo: consider moving to common utility 
// \todo: additional debug verification on release
//
class CStringLock
{
public:
    CString * m_string;
    LPTSTR    m_buffer;

    static LPTSTR NullBuffer;

public:
    CStringLock(CString & s) : m_string(&s)
    {
        m_buffer = m_string->GetBuffer(0);
        // fixes an incompatibility with CStdString, see "NullBuffer" in .cpp 
        if (!s.GetLength())
            m_buffer = NullBuffer; 
    }

    CStringLock(CString & s, int minChars) : m_string(&s)
    {
        m_buffer = m_string->GetBuffer(minChars);

        // fixes an incompatibility with CStdString, see "NullBuffer" in .cpp 
        if (!s.GetLength() && !minChars)
            m_buffer = NullBuffer; 

    }

    operator LPTSTR() { return m_buffer; }

    void Release(int newLen = -1)
    {
        if (m_string)
        {
            m_string->ReleaseBuffer(newLen);
            m_string = NULL;
            m_buffer = NULL;
        }
    }

    ~CStringLock()  { Release(); }

};



/// CStdString incompatibility:
/// http://www.codeproject.com/string/stdstring.asp
/// If the contained string is empty, CStdString.GetBuffer returns a pointer to a constant
/// empty string, which may cause an access violation when I write the terminating zero
/// (which is in my understanding implicitely allowed the way I read the MSDN docs)
/// Solution: we return a pointer to another buffer
TCHAR NullBufferData[1] = { 0 };
LPTSTR CStringLock::NullBuffer = NullBufferData;


// Helper class for Close-On-Return HKEY
// \todo migrate template class 
class CAutoHKEY
{
private:
    CAutoHKEY const & operator =(CAutoHKEY const & ); // not implemented
    CAutoHKEY(CAutoHKEY const &);   // not implemented

protected:
    HKEY    key;

public:
    CAutoHKEY() : key(0) {}
    CAutoHKEY(HKEY key_) : key(key_) {}
   ~CAutoHKEY()          { Close(); }

    void Close()
    {
        if (key) 
        {
            RegCloseKey(key); 
            key = 0;
        }
    }

   HKEY * OutArg()
   {
       Close();
       return &key;
   }

   operator HKEY() const { return key; }

}; // CAutoHKEY

/// Reads an environment variable into a CString
CString GetEnvVar(LPCTSTR envVar)
{
    SetLastError(0);

    // get length of buffer
    DWORD result = GetEnvironmentVariable(envVar, _T(""), 0);
    if (!result)
        return CString();

    CString s;
    result = GetEnvironmentVariable(envVar, CStringLock(s, result), result);
    return s;
}


/// Replace path root with environment variable
/// If the beginning of \c s matches the value of the environment variable %envVar%,
/// it is replaced with the %envVar% value
/// (e.g. "C:\Windows" with "%windir%"
/// \param s [CString &, in/out]: the string to modify
/// \param envVar [LPCTSTR]: name of the environment variable
/// \returns true if s was modified, false otherwise.
bool EnvUnsubstRoot(CString & s, LPCTSTR envVar)
{
    // get environment value string
    CString envValue = GetEnvVar(envVar);
    if (!envValue.GetLength())
        return false;

    if (s.GetLength() >= envValue.GetLength() && 
        _tcsnicmp(s, envValue, envValue.GetLength())==0)
    {
        CString modified = CString('%');
        modified += envVar;
        modified += '%';
        modified += s.Mid(envValue.GetLength());
        s = modified;
        return true;
    }
    return false;
}
    
} // namespace nsPath::nsDetail


using namespace nsDetail;

const TCHAR Backslash = '\\';


// ==============================================
//  Trim
// ----------------------------------------------
/// Trims whitespaces from left and right side. 
/// \param s [CString]: String to modify in-place.
void Trim(CString & string)
{
    if (_istspace(GetFirstChar(string)))
        string.TrimLeft();

    if (_istspace(GetLastChar(string)))
        string.TrimRight();
}



// ==================================================================
//  GetDriveLetter(ch)
// ------------------------------------------------------------------
/// checks if the specified letter \c ch is a drive letter, and casts it to uppercase
/// 
/// \returns [TCHAR]: if \c is a valid drive letter (A..Z, or a..z), returns the drive letter
///     cast to uppercase (A..Z). >Otherwise, returns 0
TCHAR GetDriveLetter(TCHAR ch)
{
    if ( (ch >= 'A' && ch <= 'Z'))
        return ch;

    if (ch >= 'a' && ch <= 'z')
            return (TCHAR) (ch - 'a' + 'A');

    return 0;
}


// ==================================================================
//  GetDriveLetter(string)
// ------------------------------------------------------------------
/// returnd the drive letter of a path.
/// The drive letter returned is always uppercase ('A'.`'Z'). 
/// \param s [LPCTSTR]: the path string
/// \returns [TCHAR]: the drive letter, converted to uppercase, if the path starts with an 
///                     X: drive specification. Otherwise, returns 0
// 
TCHAR GetDriveLetter(LPCTSTR s)
{
    if (s == NULL || *s == 0 || s[1] != ':')
        return 0;

    return GetDriveLetter(s[0]);
}



// ==================================================================
//  QuoteSpaces
// ------------------------------------------------------------------
///
/// Quotes the string if it is not already quoted, and contains spaces
/// see also MSDN: \c PathQuoteSpaces
/// \note If the string is already quoted, an additional pair of quotes is added.
/// \param str [CString const &]: path string to add quotes to
/// \returns [CString]: path string with quotes added if required
// 
CString QuoteSpaces(CString const & str)
{
    // preserve refcounting if no changes will be made
    if (str.Find(' ')>=0)  // if the string contains any spaces...
    {
        CString copy(str);
        CStringLock buffer(copy, copy.GetLength() + 2);
        PathQuoteSpaces(buffer);
        buffer.Release();
        return copy;
    }

    return str; // unmodified
}



/// helper function for GetRootType
inline ERootType GRT_Return(ERootType type, int len, int * pLen)
{
   if (pLen)
      *pLen = len;
   return type;
}

// ==================================================================
//  GetRootType
// ------------------------------------------------------------------
/// 
/// returns the type of the path root, and it's length.
/// For supported root types, see \ref nsPath::ERootType "ERootType" enumeration
/// 
/// \param path [LPCTSTR]: The path to analyze
/// \param pLen [int *, out]: if not NULL, receives the length of the root part (in characters)
/// \param greedy [bool=true]: Affects len and type of the following root types:
///     - \c "\\server\share" : with greedy=true, it is treated as one \c rtServerShare root,
///       otherwise, it is treated as \c rtServer root
///     
/// \returns [ERootType]: type of the root element 
///         
/// 
ERootType GetRootType(LPCTSTR path, int * pLen, bool greedy)
{
//   ERootType   type = rtNoRoot;
//   int len = 0;

   const TCHAR * invalidChars = _T("\\/:*/\"<>|");
   const TCHAR bk = '\\';

   if (!path || !*path)
      return GRT_Return(rtNoRoot, 0, pLen);

   // drive spec
   if (_istalpha(*path) && path[1] == ':')
   {
      if (path[2] == bk) { return GRT_Return(rtDriveRoot, 3, pLen); }
      else               { return GRT_Return(rtDriveCur, 2, pLen); }
   }

   // anything starting with two backslashes
   if (path[0] == bk && path[1] == bk)
   {
      // UNC long path?
      if (path[2] == '?' && path[3] == bk)
      {
         int extraLen = 0;
         GetRootType(path+4, &extraLen) ;
         return GRT_Return(rtLongPath, 4 + extraLen, pLen);
      }

      // position of next backslash or colon
      int len = 2 + (int)_tcscspn(path+2, invalidChars);
      TCHAR const * end = path+len;

      // server only, no backslash
      if (*end == 0) 
         return GRT_Return(rtServerOnly, len, pLen);

      // server only, terminated with backslash
      if (*end == bk && end[1] == 0) 
         return GRT_Return(rtServerOnly, len+1, pLen); 

      // server, backslash, and more...
      if (*end == bk)
      {
          if (!greedy)  // return server only
              return GRT_Return(rtServer, len, pLen);

         len += 1 + (int)_tcscspn(end+1, invalidChars);
         end = path + len;

         // server, share, no backslash
         if (*end == 0) 
            return GRT_Return(rtServerShare, len, pLen); 

         // server, share, backslash
         if (*end == '\\') 
            return GRT_Return(rtServerShare, len+1, pLen);
      }
      // fall through to other tests
   }

   int len = (int)_tcscspn(path, invalidChars);
   TCHAR const * end = path + len;

   // (pseudo) protocol:
   if (len > 0 && *end == ':')
   {
      if (end[1] == '/' && end[2] == '/') 
         return GRT_Return(rtProtocol, len+3, pLen);
      else 
         return GRT_Return(rtPseudoProtocol, len+1, pLen); 
   }

   return GRT_Return(rtNoRoot, 0, pLen);
}





// ==================================================================
//  CPath::Trim
// ------------------------------------------------------------------
/// 
/// removes leading and trailing spaces.
// 
CPath &  CPath::Trim()
{
    nsPath::Trim(m_path);
    return *this;
}

// ==================================================================
//  CPath::Unquote
// ------------------------------------------------------------------
/// 
/// removes (double) quotes from around the string
// 
CPath & CPath::Unquote()
{
    if (GetFirstChar(m_path) == '"' && GetLastChar(m_path) == '"')
        m_path = m_path.Mid(1, m_path.GetLength()-2);
    return *this;
}

// ==================================================================
//  CPath::Canonicalize
// ------------------------------------------------------------------
///
/// Collapses "\\..\\" and "\\.\\" path parts.
/// see also MSDN: PathCanonicalize
/// \note
/// PathCanonicalize works strange on relative paths like "..\\..\\x" -
/// it is changed to "\x", which is clearly not correct. SearchAndQualify is affected
/// by the same problem
/// \todo handle this separately?
/// 
/// \par Implementation Differences
/// \c PathCanonicalize does turn an empty path into a single backspace.
/// CPath::Canonicalize does not modify an empty path.
// 
CPath & CPath::Canonicalize()
{
    if (!m_path.GetLength())  // PathCanonicalize turns an empty path into "\\" - I don't want this..
        return *this;

    if (m_path.Find(_T("\\."))>=0)
    {
        CString target = m_path;  // PathCanonicalize requires a copy to work with
        CStringLock buffer(target, m_path.GetLength()+2); // might add a backslash sometimes !
        PathCanonicalize(buffer, m_path);
        buffer.Release();
        m_path = target;
    }

    return *this;
}


// ==================================================================
//  CPath::ShrinkXXLPath
// ------------------------------------------------------------------
///
/// Removes an "Extra long file name" specification
/// Unicode API allows pathes longer than MAX_PATH, if they start with "\\\\?\\". This function
/// removes such a specification if present. See also MSDN: "File Name Conventions".
// 
CPath & CPath::ShrinkXXLPath()
{
    if (m_path.GetLength() >= 6 &&   // at least 6 chars for [\\?\C:]
        _tcsncmp(m_path, _T("\\\\?\\"), 4) == 0)
    {
        LPCTSTR path = m_path;

        if (nsPath::GetDriveLetter(path[4]) != 0 && path[5] == ':')
            m_path = m_path.Mid(4);

        else if (m_path.GetLength() >= 8)  // at least 8 chars for [\\?\UNC\]
        {
            if (_tcsnicmp(path + 4, _T("UNC\\"), 4) == 0)
            {
                // remove chars [2]..[7]
                int len = m_path.GetLength() - 8; // 
                CStringLock buffer(m_path);
                memmove(buffer+2, buffer+8, len * sizeof(TCHAR));
                buffer.Release(len+2);
            }

        }
    }

    return *this;
}


// ==================================================================
//  CPath::Assign
// ------------------------------------------------------------------
/// 
/// Assigns a string to the path object, optionally applying cleanup of the path name
///
/// \param str [CString const &]: The string to assign to the path
/// \param cleanup [DWORD, default = epc_Default]: operations to apply to the path
/// \returns [CPath &]: reference to the path object itself
/// 
/// see CPath::Clean for a description of the cleanup options
// 
CPath & CPath::Assign(CString const & str, DWORD cleanup)
{
    m_path = str;
    Clean(cleanup);
    return *this;
}


// ==================================================================
//  CPath::MakePretty
// ------------------------------------------------------------------
///
/// Turns an all-uppercase path to all -lowercase. A path containing any lowercase 
/// character is not modified.
/// (This is Microsoft#s idea of prettyfying a path. I don't know what to say)
/// 
CPath & CPath::MakePretty()
{
    CStringLock buffer(m_path);
    PathMakePretty(buffer);
    return *this;
}


// ==================================================================
//  CPath::Clean
// ------------------------------------------------------------------
///
/// Applies multiple "path cleanup" operations
/// 
/// \param cleanup [DWORD]: a combination of zero or more nsPath::EPathCleanup flags (see below)
/// \returns [CPath &]: a reference to the path object
/// 
/// The following cleanup operations are defined:
///     - \c epcRemoveArgs: call PathRemoveArgs to remove arguments
///     - \c epcRemoveIconLocation: call to PathParseIconLocation to remove icon location
///     - \c \b epcTrim: trim leading and trailing whitespaces
///     - \c \b epcUnquote: remove quotes 
///     - \c \b epcTrimInQuote: trim whitespaces again after unqouting.
///     - \c \b epcCanonicalize: collapse "\\.\\" and "\\..\\" segments
///     - \c \b epcRemoveXXL: remove an "\\\\?\\" prefix for path lengths exceeding \c MAX_PATH
///     - \c epcSlashToBackslash: (not implemented)  change forward slashes to back slashes (does not modify a "xyz://" protocol root)
///     - \c epcMakePretty: applies PathMakePretty
///     - \c epcRemoveArgs: remove command line arguments
///     - \c epcRemoveIconLocation: remove icon location number
///     - \c \b epcExpandEnvStrings: Expand environment strings
/// 
/// This function is called by most assignment constructors and assignment operators, using
/// the \c epc_Default cleanup options (typically those set in bold above, but check the enum 
/// documentation in case I forgot to update this one).
/// 
/// Constructors and Assignment operators that take a string (\c LPCTSTR, \c LPCTSTR, \c CString) call
/// this function. Copy or assignment from another \c CPath object does not call this function.
/// 
/// 
//
CPath & CPath::Clean(DWORD cleanup)
{
    if (cleanup & epcRemoveArgs)
    {
       // remove leading spaces, otherwise PathRemoveArgs considers everything a space
       if (cleanup & epcTrim)
          m_path.TrimLeft();  
       
        PathRemoveArgs(CStringLock(m_path));
    }

    if (cleanup & epcRemoveIconLocation)
        PathParseIconLocation(CStringLock(m_path));


    if (cleanup & epcTrim)
        Trim();

    if (cleanup & epcUnquote)
    {
        Unquote();
        if (cleanup & epcTrimInQuote)
            Trim();
    }

    if (cleanup & epcExpandEnvStrings)
        ExpandEnvStrings();

    if (cleanup & epcCanonicalize)
        Canonicalize();

    if (cleanup & epcRemoveXXL)
        ShrinkXXLPath();

    if (cleanup & epcSlashToBackslash)
       m_path.Replace('/', '\\');

    if (cleanup & epcMakePretty)
        MakePretty();

    return *this;
}


// Extractors
CString CPath::GetStr(DWORD packing) const
{
    CString str = m_path;

//    _ASSERTE(!(packing & eppAutoXXL));   // TODO

    if (packing & eppAutoQuote)
        str = QuoteSpaces(str);

    if (packing & eppBackslashToSlash)
        str.Replace('\\', '/');  // TODO: suport server-share and protocol correctly

    return str;
}


_bstr_t CPath::GetBStr(DWORD packing) const
{
    return _bstr_t( GetStr(packing).operator LPCTSTR());
}



// ==================================================================
//  Constructors 
// ------------------------------------------------------------------
// 
CPath::CPath(LPCSTR path) : m_path(path)    
{ 
    Clean();
}

CPath::CPath(LPCWSTR path) : m_path(path)
{
    Clean();
}

CPath::CPath(CString const & path) : m_path(path)
{
    Clean();
}

CPath::CPath(CPath const & path) : m_path(path.m_path) {}  // we assume it is already cleaned


CPath::CPath(CString const & path,DWORD cleanup) : m_path(path)
{
    Clean(cleanup);
}


// ==================================================================
//  Assignment
// ------------------------------------------------------------------
// 
CPath & CPath::operator=(LPCSTR rhs)
{
#ifndef _UNICODE    // avoidf self-assignment
    if (rhs == m_path.operator LPCTSTR())
        return *this;
#endif
    m_path = rhs;
    Clean();
    return *this;
}

CPath & CPath::operator=(LPCWSTR rhs)
{
#ifdef _UNICODE // avoid self-assignment
    if (rhs == m_path.operator LPCTSTR())
        return *this;
#endif
    m_path = rhs;
    Clean();
    return *this;
}

CPath & CPath::operator=(CString const & rhs)
{
    // our own test for self-assignment, so we can skip CClean in this case
    if (rhs.operator LPCTSTR() == m_path.operator LPCTSTR())
        return *this;
    m_path = rhs;
    Clean();
    return *this;
}

CPath & CPath::operator=(CPath const & rhs)
{
    if (rhs.m_path.operator LPCTSTR() == m_path.operator LPCTSTR())
        return *this;

    m_path = rhs;
    return *this;
}




// ==================================================================
//  CPath::operator &=
// ------------------------------------------------------------------
///
/// appends a path segment, making sure it is separated by exactly one backslash
/// \returns reference to the modified \c CPath instance.
// 
CPath & CPath::operator &=(LPCTSTR rhs)
{
    return CPath::Append(rhs);
}


// ==================================================================
//  CPath::AddBackslash
// ------------------------------------------------------------------
///
/// makes sure the contained path is terminated with a backslash
/// \returns [CPath &]: reference to the modified path
/// see also: \c PathAddBackslash Shell Lightweight Utility API
// 
CPath & CPath::AddBackslash()
{
    if (GetLastChar(m_path) != Backslash)
    {
        CStringLock buffer(m_path, m_path.GetLength()+1);
        PathAddBackslash(buffer);
    }
    return *this;
}

// ==================================================================
//  CPath::RemoveBackslash
// ------------------------------------------------------------------
///
/// If the path ends with a backslash, it is removed.
/// \returns [CPath &]: a reference to the modified path.
// 
CPath & CPath::RemoveBackslash()
{
    if (GetLastChar(m_path) == Backslash)
    {
        CStringLock buffer(m_path, m_path.GetLength()+1);
        PathRemoveBackslash(buffer);
    }
    return *this;
}

// ==================================================================
//  CPath::Append
// ------------------------------------------------------------------
///
/// Concatenates two paths
/// \par Differences to \c PathAddBackslash:
/// Unlike \c PathAddBackslash, \c CPath::Append appends a single backslash if rhs is empty (and
/// the path does not already end with a backslash)
/// 
/// \param rhs [LPCTSTR]: the path component to append
/// \returns [CPath &]: reference to \c *this
// 
CPath & CPath::Append(LPCTSTR rhs)
{
    if (rhs == NULL || *rhs == '\0')
    {
        AddBackslash();
    }
    else
    {
        int rhsLen = rhs ? (int)_tcslen(rhs) : 0;
        CStringLock buffer(m_path, m_path.GetLength()+rhsLen+1);
        PathAppend(buffer, rhs);
    }
    return *this;
}



// ==================================================================
//  CPath::ShellGetRoot
// ------------------------------------------------------------------
///
/// Retrieves the Root of the path, as returned by \c PathSkipRoot.
/// 
/// \note For a more detailed (but "hand-made") implementation see GetRoot and GetRootType.
/// 
/// The functionality of \c PathSkipRoot is pretty much limited:
///     - Drives ("C:\\" but not "C:")
///     - UNC Shares ("\\\\server\\share\\", but neither "\\\\server" nor "\\\\server\\share")
///     - UNC long drive ("\\\\?\\C:\\")
/// 
/// \returns [CString]: the rot part of the string
/// 
CString CPath::ShellGetRoot() const
{
    LPCTSTR path = m_path;
    LPCTSTR rootEnd = PathSkipRoot(path);

    if (rootEnd == NULL)
        return CString();

    return m_path.Left((int)(rootEnd - path));
}


// ==================================================================
//  GetRootType
// ------------------------------------------------------------------
/// 
/// returns the type of the root, and it's length.
/// For supported tpyes, see \ref nsPath::ERootType "ERootType".
/// see also \ref nsPath::GetRootType
/// 
ERootType CPath::GetRootType(int * len, bool greedy) const
{
   return nsPath::GetRootType(m_path, len, greedy);
}

// ==================================================================
//  GetRoot
// ------------------------------------------------------------------
/// 
/// \param rt [ERootType * =NULL]: if given, receives the type of the root segment.
/// \return [CString]: the root, as a string.
/// 
/// For details which root types are supported, and how the length is calculated, see
/// \ref nsPath::ERootType "ERootType" and \ref nsPath::GetRootType
/// 
CString CPath::GetRoot(ERootType * rt, bool greedy) const
{
   int len = 0;
   ERootType rt_ = nsPath::GetRootType(m_path, &len, greedy);
   if (rt)
      *rt = rt_;
   return m_path.Left(len);
}


// ==================================================================
//  CPath::SplitRoot
// ------------------------------------------------------------------
///
/// removes and returns the root element from the contained path
/// You can call SplitRoot repeatedly to retrieve the path segments in order
/// 
/// \param rt [ERootType * =NULL] if not NULL, receives the type of the root element
///     note: the root element type can be recognized correctly only for the first segment
/// \returns [CString]: the root element of the original path
/// \par Side Effects: root element removed from contained path
/// 
CString CPath::SplitRoot(ERootType * rt)
{
    CString head;
 
   if (!m_path.GetLength())
      return head;

   int rootLen = 0;
   ERootType rt_ = nsPath::GetRootType(m_path, &rootLen, false);
   if (rt)
       *rt = rt_;

    if (rt_ == rtNoRoot) // not a typical root element
    {
        int start = 0;
        if (GetFirstChar(m_path) == '\\') // skip leading backslash (double backslas handled before)
            ++start;

        int ipos = m_path.Find('\\', start);
        if (ipos < 0)
        {
            head = start ? m_path.Mid(start) : m_path;
            m_path.Empty();
        }
        else
        {
            head = m_path.Mid(start, ipos-start);
            m_path = m_path.Mid(ipos+1);
        }
    }
    else
    {
        head = m_path.Left(rootLen);

        if (rootLen < m_path.GetLength() && m_path[rootLen] == '\\')
            ++rootLen;
        m_path = m_path.Mid(rootLen);
    }

    return head;
}



// ==================================================================
//  CPath::GetPath
// ------------------------------------------------------------------
///
/// returns the path (without file name and extension)
/// \param includeRoot [bool=true]: if \c true (default), the root is included in the retuerned path.
/// \returns [CPath]: the path, excluding file name and 
/// \par Implementation:
///     Uses \c PathFindFileName, and \c PathSkipRoot
/// \todo
///     - in "c:\\temp\\", \c PathFindFileName considers "temp\\" to be a file name and returns
///       "c:\\" only. This is clearly not my idea of a path
///     - when Extending \c CPath::GetRoot, this function should be adjusted as well
/// 
// 
CPath CPath::GetPath(bool includeRoot ) const
{
    LPCTSTR path = m_path;
    LPCTSTR fileName = PathFindFileName(path);
    if (fileName == NULL) // seems to find something in any way!
        fileName = path + m_path.GetLength();

    LPCTSTR rootEnd = includeRoot ? NULL : PathSkipRoot(path);

    CPath ret;
    if (rootEnd == NULL)  // NULL if root should be included, or no root is found
        return m_path.Left((int)(fileName-path));
    else
        return m_path.Mid((int)(rootEnd-path), (int)(fileName-rootEnd));
}

// ==================================================================
//  CPath::GetName
// ------------------------------------------------------------------
///
/// \returns [CString]: the file name of the path
/// \par Differences to \c PathFindFileName:
/// \c PathFindFileName, always treats the last path segment as file name, even if it ends with a backslash.
/// \c GetName treats such a string as not containing a file name\n
/// \b Example: for "c:\\temp\\", \c PathFindFileName finds "temp\\" as file name. \c GetName returns
/// an empty string.
CString CPath::GetName() const
{
    // fix treating final path segments as file name
    if (GetLastChar(m_path) == '\\')
        return CString();

    LPCTSTR path = m_path;
    LPCTSTR fileName = PathFindFileName(path);
    if (fileName == NULL)
        return CString();

    return m_path.Mid((int)(fileName-path));
}

// ==================================================================
//  CPath::GetTitle
// ------------------------------------------------------------------
///
/// \returns [CString]: the file title, without path or extension
// 
CString CPath::GetTitle() const
{
    LPCTSTR path = m_path;
    LPCTSTR fileName = PathFindFileName(path);
    LPCTSTR ext      = PathFindExtension(path);

    if (fileName == NULL)
        return CString();

    if (ext == NULL)
        return m_path.Mid((int)(fileName-path));

    return m_path.Mid((int)(fileName-path), (int)(ext-fileName));


}

// ==================================================================
//  CPath::GetExtension
// ------------------------------------------------------------------
///
/// \returns [CString]: file extension
/// \par Differences to \c PathFindExtension
/// Unlike \c PathFindExtension, the period is not included in the extension string
// 
CString CPath::GetExtension() const
{
    LPCTSTR path = m_path;
    LPCTSTR ext      = PathFindExtension(path);

    if (ext == NULL)
        return CString();

    if (*ext == '.')        // skip "."
        ++ext;

    return m_path.Mid((int)(ext-path));
}




// ==================================================================
//  CPath::AddExtension
// ------------------------------------------------------------------
///
/// Appends the specified extension to the path. The path remains unmodified if it already contains
/// an extension (that is, the part behind the last backslash contains a period)
/// \par Difference to \c PathAddExtension
/// Unlike CPath::AddExtension adds a period, if \c extension does not start with one
/// \param extension [LPCTSTR]: the extension to append
/// \param len [int, default=-1]: (optional) length of \c extension in characters, not counting the
///     terminating zero. This argument is only for avoiding a call to _tcslen if the caller already
///     knows the length of the string. The string still needs to be zero-terminated and contain exactly 
///     \c len characters. 
/// \returns [CPath &]: reference to the modified Path object
// 
CPath & CPath::AddExtension(LPCTSTR extension, int len)
{
    if (!extension)
        return AddExtension(_T(""), 0);

    if (*extension != '.')
    {
        CString s = CString('.') + extension;
        return AddExtension( s, s.GetLength());
    }

    if (len < 0)
        return AddExtension(extension, (int)_tcslen(extension));

    int totalLen = m_path.GetLength() + len;  // already counts the period

    PathAddExtension(CStringLock(m_path, totalLen), extension);
    return *this;
}


// ==================================================================
//  CPath::RemoveExtension
// ------------------------------------------------------------------
///
/// Removes the extension of the path, if it has any.
/// \returns [CPath &]: reference to the modified path object
// 
CPath& CPath::RemoveExtension()
{
    PathRemoveExtension(CStringLock(m_path));
    return *this;
}


// ==================================================================
//  CPath::RenameExtension
// ------------------------------------------------------------------
///
/// Replaces an existing extension with the new given extension.
/// If the path has no extension, it is appended.
/// \par Difference  to \c PathRenameExtension:
/// Unlike PathRenameExtension, \c newExt needs not start with a period.
/// \param newExt [LPCTSTR ]: newextension
/// \returns [CPath &]: reference to the modified path string
// 
CPath & CPath::RenameExtension(LPCTSTR newExt)
{
    if (newExt == NULL || *newExt != '.')
    {
        RemoveExtension();
        return AddExtension(newExt);
    }

    int maxLen = m_path.GetLength() + (int)_tcslen(newExt) + 1;
    PathRenameExtension(CStringLock(m_path, maxLen), newExt);
    return *this;
}


// ==================================================================
//  ::RemoveFileSpec
// ------------------------------------------------------------------
///
/// Removes the file specification (amd extension) from the path.
/// \returns [CPath &]: a reference to the modified path object
// 
CPath & CPath::RemoveFileSpec()
{
    PathRemoveFileSpec(CStringLock(m_path));
    return *this;
}


// ==================================================================
//  CPath::SplitArgs
// ------------------------------------------------------------------
///
/// (static) Separates a path string from command line arguments
/// 
/// \param path_args [CString const &]: the path string with additional command line arguments
/// \param args [CString *, out]: if not \c NULL, receives the arguments separated from the path
/// \param cleanup [DWORD, = epc_Default]: the "cleanup" treatment to apply to the path, see \c CPath::Clean
/// \returns [CPath]: a new path without the arguments
// 
CPath SplitArgs(CString const & path_args, CString * args, DWORD cleanup)
{
   CString pathWithArgs = path_args;

   // when "trim" is given, trim left spaces, so PathRemoveArgsworks correctly and returns 
   // the path part with the correct length
   if (cleanup & epcTrim)
      pathWithArgs.TrimLeft();

    // assign with only removing the arguments
    CPath path(pathWithArgs, epcRemoveArgs);

    // cut non-argument part away from s
    if (args)
    {
        *args = pathWithArgs.Mid(path.GetLength());
        args->TrimLeft();
    }

    // now, clean the contained string (last since it might shorten the path string)
    path.Clean(cleanup &~ epcRemoveArgs);

    return path;
}


// ==================================================================
//  CPath::SplitIconLocation
// ------------------------------------------------------------------
/// 
/// (static) Splits a path string containing an icon location into path and icon index
///
/// \param path_icon [CString const &]: the string containing an icon location
/// \param pIcon [int *, NULL]: if not NULL, receives the icon index 
/// \param cleanup [DWORD, epc_Default]: additional cleanup to apply to the returned path
/// \returns [CPath]: the path contained in \c path_icon (without the icon location)
// 
CPath SplitIconLocation(CString const & path_icon, int * pIcon, DWORD cleanup)
{
    CString strpath = path_icon;
    int icon = PathParseIconLocation( CStringLock(strpath) );
    if (pIcon)
        *pIcon = icon;

    return CPath(strpath, cleanup & ~epcRemoveIconLocation);
}


// ==================================================================
//  CPath::BuildRoot
// ------------------------------------------------------------------
///
/// (static) Creates a root path from a drive index (0..25)
/// \param driveNumber [int]: Number of the drive, 0 == 'A', etc.
/// \returns [CPath]: a path consisitng only of a drive root
// 
CPath BuildRoot(int driveNumber)
{
    CString strDriveRoot;
    ::PathBuildRoot(CStringLock(strDriveRoot, 3), driveNumber);
    return CPath(strDriveRoot, 0);
}



// ==================================================================
//  CPath::GetModuleFileName
// ------------------------------------------------------------------
///
/// Returns the path of the dspecified module handle
/// Path is limited to \c MAX_PATH characters
/// see Win32: GetModuleFileName for more information
/// \param module [HMODULE =NULL ]: DLL module handle, or NULL for path to exe
/// \returns [CPath]: path to the specified module, or to the application exe if \c module==NULL.
///         If an error occurs, the function returrns an empty string. 
///         Call \c GetLastError() for more information.
/// 
CPath GetModuleFileName(HMODULE module)
{
    CString path;
    DWORD ok = ::GetModuleFileName(module, CStringLock(path, MAX_PATH), MAX_PATH+1);
    if (ok == 0)
        return CPath();
    return CPath(path);
}



// ==================================================================
//  CPath::GetCurrentDirectory
// ------------------------------------------------------------------
///
/// \returns [CPath]: the current directory, se Win32: \c GetCurrentDirectory.
/// \remarks
/// If an error occurs the function returns an empty string. More information is available
/// through \c GetLastError.
/// 
CPath GetCurrentDirectory()
{
    CString path;
    CStringLock buffer(path, MAX_PATH);
    DWORD chars = ::GetCurrentDirectory(MAX_PATH+1, buffer);
    buffer.Release(chars);
    return CPath(path);
}


// ==================================================================
//  CPath::GetCommonPrefix
// ------------------------------------------------------------------
///
/// Returns the common prefix of this path and the given other path,
/// e.g. for "c:\temp\foo\foo.txt" and "c:\temp\bar\bar.txt", it returns
/// "c:\temp".
/// \param secondPath [LPCTSTR]: the path to compare to
/// \returns [CPath]: a new path, containing the part that is identical
// 
CPath CPath::GetCommonPrefix(LPCTSTR secondPath)
{
    CString prefix;
    PathCommonPrefix(m_path, secondPath, CStringLock(prefix, m_path.GetLength()));
    return CPath(prefix, 0);


}



// ==================================================================
//  CPath::GetDriveNumber
// ------------------------------------------------------------------
///
/// \returns [int]: the driver number (0..25 for A..Z), or -1 if the 
///     path does not start with a drive letter
// 
int CPath::GetDriveNumber()
{
    return PathGetDriveNumber(m_path);
}

// ==================================================================
//  CPath::GetDriveLetter
// ------------------------------------------------------------------
///
/// \returns [TCHAR]: the drive letter in uppercase, or 0
// 
TCHAR CPath::GetDriveLetter()
{
    int driveNum = GetDriveNumber();
    if (driveNum < 0)
        return 0;
    return (TCHAR)(driveNum + 'A');
}


// ==================================================================
//  CPath::RelativePathTo
// ------------------------------------------------------------------
///
/// Determines a relative path from the contained path to the specified \c pathTo
/// \par Difference to \c PathRelativeTo:
///  - instead of a file attributes value, you specify a flag (this is a probelm only
///     if the function supports other attribues than FILE_ATTRIBUTE_DIRECTORY in the future)
///  - no flag / attribute is specified for the destination (it does not seem to make a difference)
/// \param pathTo [LPCTSTR]: the target path or drive
/// \param srcIsDir [bool =false]: determines whether the current path is as a directory or a file
/// \returns [CPath]: a relative path from this to \c pathTo
// 
CPath CPath::RelativePathTo(LPCTSTR pathTo,bool srcIsDir)
{
    CString path;
    if (!pathTo)
        return CPath();

    // maximum length estimate: 
    // going up to the root of a path like "c:\a\b\c\d\e", and then append the entire to path
    int maxLen = 3*m_path.GetLength() / 2 +1  + (int)_tcslen(pathTo); 

    PathRelativePathTo( CStringLock(path, maxLen), 
                        m_path, 
                        srcIsDir ? FILE_ATTRIBUTE_DIRECTORY : 0,
                        pathTo, 0);

    return CPath(path, 0);
}


// ==================================================================
//  MakeRelative
// ------------------------------------------------------------------
/// 
/// Of the path contained is below \c basePath, it is made relative.
/// Otherwise, it remains unmodified. 
/// 
/// Unlike RelativePathTo, the path is made relative only if the base path
/// matches completely, and does not generate ".." elements.
/// 
/// \return [bool] true if the path was modified, false otherwise.
/// 
bool CPath::MakeRelative(CPath const & basePath)
{
   CPath basePathBS = basePath;
   basePathBS.AddBackslash(); // add backslash so that "c:\a" is not a base path for "c:\alqueida\files"

   if (m_path.GetLength() > basePathBS.GetLength())
   {
      if (0 == _tcsnicmp(basePathBS, m_path, basePathBS.GetLength()))
      {
         m_path = m_path.Mid(basePathBS.GetLength());
         return true;
      }
   }
   return false;
}

// ==================================================================
//  MakeAbsolute
// ------------------------------------------------------------------
/// 
/// If the contained path is relative, it is prefixed by \c basePath.
/// Otherwise it remains unmodified.
/// 
/// Use: as anti-MakeRelative.
/// 
bool CPath::MakeAbsolute(CPath const & basePath)
{
   if (IsRelative())
   {
      m_path = basePath & m_path;
      return true;
   }
   return false;
}




// ==================================================================
//  CPath::MatchSpec
// ------------------------------------------------------------------
///
/// Checks if the path matches a certain specification
/// \param spec [LPCTSTR]: File specification (like "*.txt")
/// \returns [bool]: true if the path matches the specification
// 
bool CPath::MatchSpec(LPCTSTR spec)
{
    return PathMatchSpec(m_path, spec) != 0;
}



// ==================================================================
//  CPath::ExpandEnvStrings
// ------------------------------------------------------------------
///
/// replaces environment string references with their current value.
/// See MSDN: \c ExpandEnvironmentStrings for more information
/// \returns [CPath &]: reference to the modified path
// 
CPath & CPath::ExpandEnvStrings()
{
    CString target;

    DWORD len = m_path.GetLength();

    DWORD required = ExpandEnvironmentStrings(
                m_path,
                CStringLock(target, len), len+1);

    if (required > len+1)
        ExpandEnvironmentStrings(
                m_path,
                CStringLock(target, required), required+1);

    m_path = CPath(target, 0);
    return *this;
}


// ==================================================================
//  CPath::GetCompactStr
// ------------------------------------------------------------------
///
/// Inserts ellipses so the path fits into the specified number of pixels
/// See also SMDN: \c PathCompactPath 
/// \param dc [HDC]: device context where the path is displayed
/// \param dx [UINT]: number of pixels where to display
/// \param eppFlags [DWORD, =0]: combination of \c EPathPacking flags indicating how to prepare the path
/// \returns [CString]: path string prepared for display
// 
CString CPath::GetCompactStr(HDC dc,UINT dx, DWORD eppFlags)
{
    CString ret = GetStr(eppFlags);
	PathCompactPath(dc, CStringLock(ret), dx);
    return ret;
}

// ==================================================================
//  CPath::GetCompactStr
// ------------------------------------------------------------------
///
/// Inserts ellipses so the path does not exceed the given number of characters
/// \param cchMax [UINT]: maximum number of characters
/// \param eppFlags [DWORD, =0]: combination of \c EPathPacking flags indicating how to prepare the path
/// \param flags [DWORD, =0]: reserved, must be 0
/// \returns [CString]: path string prepared for display
// 
CString CPath::GetCompactStr(UINT cchMax,DWORD flags, DWORD eppFlags )
{
    CString cleanPath = GetStr(eppFlags);
    CString ret;

    PathCompactPathEx(CStringLock(ret, cleanPath.GetLength()), cleanPath, cchMax, flags);
    return ret;
}

// ==================================================================
//  CPath::SetDlgItem
// ------------------------------------------------------------------
///
/// Sets the text of a child control of a window or dialog, 
/// PathCompactPath to make it fit
/// 
/// \param dlg [HWND]: the window handle of the parent window
/// \param dlgCtrlID [UINT]: ID of the child control
/// \param eppFlags [DWORD, =0]: combination of \c EPathPacking flags indicating how to prepare the path
/// \returns [void]:
// 
void CPath::SetDlgItem(HWND dlg,UINT dlgCtrlID, DWORD eppFlags)
{
    CString cleanPath = GetStr(eppFlags);
    PathSetDlgItemPath(dlg, dlgCtrlID, cleanPath);
}


// ==================================================================
//  ::SearchAndQualify
// ------------------------------------------------------------------
///
/// Searches for the given file on the search path. If it exists in the search path, 
/// it is qualified with the full path of the first occurence found. Otherwise, it is 
/// qualified with the current path. An absolute file paths remains unchanged. 
/// 
/// \note
/// SearchAndQualify seems to be affected by the same problem
/// as \ref nsPath::CPath::Canonicalize "Canonicalize" : a path like "..\\..\\x" 
/// is changed to "\\x", which is clearly not correct (IMHO).
/// \n
/// compare also: \ref nsPath::CPath::FindOnPath "FindOnPath":
/// FindOnPath allows to specify custom directories to be searched before the search path, and
/// behaves differently in some cases.
/// If the file is not found on the search path, \c FindOnPath leaves the file name unchanged.
/// SearchAndQualify qualifies the path with the current directory in this case
/// 
// 
CPath & CPath::SearchAndQualify()
{
    if (!m_path.GetLength())
        return *this;

    CString qualified;
    DWORD len = m_path.GetLength();
    while (qualified.GetLength() == 0)
    {
        PathSearchAndQualify(m_path, CStringLock(qualified, len), len+1);
        len *= 2;
    }
    m_path = qualified;
    return *this;
}



// ==================================================================
//  CPath::FindOnPath
// ------------------------------------------------------------------
///
/// Similar to SearchAndQualify, but 
/// \note
///  -# the return value of PathFindOnPath does \b not indicate whether the file
///     exisits, that's why we don't return it either. If you want to check if the file
///     really is there use \c FileExists
///  -# PathFindOnPath does not check for string overflow. Documentation recommends to use a buffer
///     of length MAX_PATH. I don't trust it to be fail safe in case a path plus the string
///     exceeds this length (note that the file would not be found in this case - but the shell
///     API might be tempted to build the string inside the buffer)\n
///     If you don't need the "additional Dirs" functionality, it is recommended to use
///     SearchAndQualify instead
/// 
/// \param additionalDirs [LPCTSTR *, = NULL]: Additional NULL-terminated array of directories 
///                                            to search first
/// \returns [CPath &]: a reference to the fully qualified file path
/// 
/// \par error handling:
///   If the function succeeds, \c GetLastError returns 0. Otherwise, \c GetLastError returns a Win32 error code.
/// 
CPath & CPath::FindOnPath(LPCTSTR * additionalDirs)
{
    DWORD len = m_path.GetLength() + 1 + MAX_PATH;
    bool ok = PathFindOnPath(CStringLock(m_path, len), additionalDirs) != 0;
    if (ok)
       SetLastError(0);
    return *this;
}




// ==================================================================
//  CPath::Exists
// ------------------------------------------------------------------
///
/// \returns [bool]: true if the file exists on the file system, false otherwise.
// 
bool CPath::Exists() const
{
    return PathFileExists(m_path) != 0;
}

// ==================================================================
//  CPath::IsDirectory
// ------------------------------------------------------------------
///
/// \returns [bool]: true if the contained path specifies a directory 
///                  that exists on the file system
// 
bool CPath::IsDirectory() const
{
    return PathIsDirectory(m_path) != 0;
}


// ==================================================================
//  CPath::IsSystemFolder
// ------------------------------------------------------------------
///
/// \param  attrib [DWORD, default = FILE_ATTRIBUTE_SYSTEM]: the attributes that 
///                 identify a system folder
/// \returns [bool]: true if the specified path exists and is a system folder
// 
bool CPath::IsSystemFolder(DWORD attrib) const
{
    return PathIsSystemFolder(m_path, attrib) != 0;
}

// ==================================================================
//  CPath::MakeSystemFolder
// ------------------------------------------------------------------
///
/// \param make [bool, default=true]: true to set the "System Folder" state, false to reset it
/// \par error handling:
///  If the function succeeds, \c GetLastError returns 0. Otherwise, \c GetLastError returns a Win32 error code.
// 
CPath & CPath::MakeSystemFolder(bool make)
{
   bool ok = make ? PathMakeSystemFolder(m_path) != 0 : PathUnmakeSystemFolder(m_path) != 0;
   if (ok) 
      SetLastError(0);
   return *this;
}


// ==================================================================
//  MakeFullPath
// ------------------------------------------------------------------
/// 
/// Makes a absolute path from a relative path, using the current working directory.
/// 
/// If the path is already absolute, it is not changed.
/// 
CPath & CPath::MakeFullPath()
{
   if (!IsRelative())
      return *this;

   LPTSTR dummy = NULL;
   DWORD chars = GetFullPathName(m_path, 0, NULL, &dummy);
   _ASSERTE(chars > 0);

   CString fullStr;
   chars = GetFullPathName(m_path, chars, CStringLock(fullStr, chars), &dummy);
   m_path = fullStr;
   return *this;
}



// ==================================================================
//  CPath::GetAttributes
// ------------------------------------------------------------------
///
/// \returns [DWORD]: the file attributes of the specified path or file, or -1 if it 
///                   does not exist.
// 
DWORD CPath::GetAttributes()
{
    return ::GetFileAttributes(m_path);
	// 
}

// ==================================================================
//  CPath::GetAttributes
// ------------------------------------------------------------------
///
/// retrives the \c GetFileExInfoStandard File Attribute information
/// 
/// \param fad [WIN32_FILE_ATTRIBUTE_DATA &, out]: receives the extended file attribute
///     information (like size, timestamps) for the specified file
/// \returns [bool]: true if the file is found and the query was successful, false otherwise
// 
bool CPath::GetAttributes(WIN32_FILE_ATTRIBUTE_DATA & fad)
{
    ZeroMemory(&fad, sizeof(fad));
    return ::GetFileAttributesEx(m_path, GetFileExInfoStandard, &fad) != 0;
}


// ==================================================================
//  CPath::EnvUnexpandRoot
// ------------------------------------------------------------------
///
/// replaces path start with matching environment variable
/// If the path starts with the value of the environment variable %envVar%,
/// The beginning of the path is replaced with the environment variable.
/// 
/// e.g. when specifying "WinDir" as \c envVar, "C:\\Windows\\foo.dll" is replaced by
/// "%WINDIR%\foo.dll"
/// 
/// \param envVar [LPCTSTR]: environment variable to check 
/// \returns \c true if the path was modified.
/// 
/// If the environment variable does not exist, or the value of the environment variable
/// does not match the beginning of the path, the path is unmodified and the function returns 
/// false.
/// 
bool CPath::EnvUnexpandRoot(LPCTSTR envVar)
{
    return nsDetail::EnvUnsubstRoot(m_path, envVar);
}

// ==================================================================
//  CPath::EnvUnexpandDefaultRoots
// ------------------------------------------------------------------
///
/// Tries to replace the path root with a matching environment variable.
/// 
/// 
/// Checks a set of default environment variables, if they match the beginning of the path.
/// If one of them matches, the beginning of the path is replaced with the environment
/// variable specification, and the function returns true.
/// Otherwise, the path remains unmodified and the function returns false.
/// 
/// see EnvUnexpandRoot for details.
/// 
bool CPath::EnvUnexpandDefaultRoots()
{
    // note: Order is important
    return EnvUnexpandRoot(_T("APPDATA")) ||
           EnvUnexpandRoot(_T("USERPROFILE")) ||
           EnvUnexpandRoot(_T("ALLUSERSPROFILE")) ||
           EnvUnexpandRoot(_T("ProgramFiles")) ||
           EnvUnexpandRoot(_T("SystemRoot")) ||
           EnvUnexpandRoot(_T("WinDir")) ||
           EnvUnexpandRoot(_T("SystemDrive"));
}


// ==================================================================
//  CPath::FromRegistry
// ------------------------------------------------------------------
/// 
/// Reads a path string from the registry.
/// \param baseKey [HKEY]: base key for registry path
/// \param subkey [LPCTSTR]: registry path
/// \param name [LPCTSTR] name of the value
/// \returns [CPath]: a path string read from the specified location
/// 
/// If the path is stored as REG_EXPAND_SZ, environment strings are expanded.
/// Otherwise, the string remains unmodified.
/// 
/// \par Error Handling:
///   If an error occurs, the return value is an empty string, and GetLastError() returns the respective
///   error code. In particular, if the registry value exists but does not contain a string, GetLastError()
///   returns ERROR_INVALID_DATA
///   \n\n
///   If the function succeeds, GetLastError() returns zero.
/// 
/// See also nsPath::ToRegistry
///             
CPath FromRegistry(HKEY baseKey, LPCTSTR subkey, LPCTSTR name)
{
   SetLastError(0);

   CAutoHKEY key;
   DWORD ok = RegOpenKeyEx(baseKey, subkey, 0, KEY_READ, key.OutArg());
   if (ok != ERROR_SUCCESS)
   {
       SetLastError(ok);
       return CPath();
   }

   DWORD len = 256;
   DWORD type = 0;

   CString path;

   do 
   {
       CStringLock buffer(path, len);
       if (!buffer)
       {
           SetLastError(ERROR_OUTOFMEMORY);
           return CPath();
       }

       DWORD size = (len+1) * sizeof(TCHAR); // size includes terminating zero
       ok = RegQueryValueEx(key, name, NULL, &type, 
                            (LPBYTE) buffer.operator LPTSTR(), &size );

       // read successfully:
       if (ok == ERROR_SUCCESS)
       {
           if (type != REG_SZ && type != REG_EXPAND_SZ)
           {
               SetLastError(ERROR_INVALID_DATA);
               return CPath();
           }
           break; // accept string
       }

       // buffer to small
       if (ok == ERROR_MORE_DATA)
       {
           len = (size + sizeof(TCHAR) - 1) / sizeof(TCHAR);
           continue;
       }

       // otherwise, an error occured
       SetLastError(ok);
       return CPath();
   } while(1);

   DWORD cleanup = epc_Default;
   if (type == REG_SZ)
       cleanup &= ~epcExpandEnvStrings;
   else
       cleanup |= epcExpandEnvStrings; // on by default, but I might change my mind..

   return CPath(path, cleanup);
}





// ==================================================================
//  CPath::ToRegistry
// ------------------------------------------------------------------
///
/// Writes the path to the registry
/// 
/// \param baseKey: root of registry path
/// \param subkey: registry path where to store
/// \param name: name to store the key under
/// \param replaceEnv [bool=true]: If true (default), environment strings will be replaced
///     with environment variables, and the string is stored as REG_EXPAND_SZ. 
///     Otherwise, the string is stored unmodified as REG_SZ.
/// 
/// See also nsPath::FromRegistry
/// 
long CPath::ToRegistry(HKEY baseKey,LPCTSTR subkey,LPCTSTR name,bool replaceEnv)
{
    CAutoHKEY key;
    DWORD ok = RegCreateKeyEx(baseKey, subkey, NULL, NULL, 0, KEY_WRITE, NULL, key.OutArg(), NULL);
    if (ok != ERROR_SUCCESS)
        return ok;

    CString path;
    if (replaceEnv)
    { 
        CPath ptemp = path;
        ptemp.EnvUnexpandDefaultRoots();
        path = ptemp.GetStr();
    }
    else
        path = GetStr();

    ok = RegSetValueEx(key, name, 0, replaceEnv ? REG_EXPAND_SZ : REG_SZ,
                        (BYTE *) path.operator LPCTSTR(), 
                        (path.GetLength()+1) * sizeof(TCHAR) );
    return ok;
}



// ==================================================================
//  IsDot, IsDotDot, IsDotty
// ------------------------------------------------------------------
///
bool CPath::IsDot() const 
{ 
    return m_path.GetLength() == 1 && m_path[0] == '.';
}

bool CPath::IsDotDot() const
{
    return m_path.GetLength() == 2 && m_path[0] == '.' && m_path[1] == '.';
}

bool CPath::IsDotty() const
{
    return IsDot() || IsDotDot();
}


const LPCTSTR InvalidChars_Windows =
	_T("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F")
	_T("\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F")
	_T("\\/:*?\"<>|");



// ==================================================================
//  IsValid
// ------------------------------------------------------------------
/// 
/// returns true if the path satisfies Windows naming conventions
///
bool CPath::IsValid() const
{
    if (!m_path.GetLength()) return false;
    if (m_path.FindOneOf(InvalidChars_Windows) >= 0) return false;
    if (GetLastChar(m_path) == '.') // may not end in '.', except "." and ".."
    {
        if (m_path.GetLength() > 2 || m_path[0] != '.')
            return false;
    }
    return true;
}




// ==================================================================
//  ReplaceInvalid
// ------------------------------------------------------------------
/// 
/// replaces all invalid file name characters  inc \c s with \c replaceChar
/// This is helpful when generating names based on user input
/// 
CString ReplaceInvalid(CString const & str, TCHAR replaceChar)
{
    if (!str.GetLength() || CPath(str).IsDotty())
        return str;

    CString s = str;

    for(int i=0; i<s.GetLength(); ++i)
    {
        TCHAR ch = s.GetAt(i);
        if (_tcschr(InvalidChars_Windows, ch))
            s.SetAt(i, replaceChar);
    }

    // last one may not be a dot
    int len = s.GetLength();
    if (s[len-1] == '.')
        s.SetAt(len-1, replaceChar);
    return s;
}



// ==================================================================

} // namespace nsPath



