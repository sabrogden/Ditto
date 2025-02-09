#pragma once

#ifdef _UNICODE				
	#define STRLEN(pText)	wcslen(pText)
#else
	#define STRLEN(pText)	strlen(pText)
#endif

#ifdef _UNICODE				
	#define STRSTR(a, b)	wcsstr(a, b)
#else
	#define STRSTR(a, b)	strstr(a, b)
#endif


#ifdef _UNICODE				
	#define STRCMP(a, b)	wcscmp(a, b)
#else
	#define STRCMP(a, b)	strcmp(a, b)
#endif

#ifdef _UNICODE				
	#define STRTOK(a, b)	wcstok(a, b)
#else
	#define STRTOK(a, b)	strtok(a, b)
#endif

#ifdef _UNICODE				
	#define STRCPY(a, b)	wcscpy(a, b)
#else
	#define STRCPY(a, b)	strcpy(a, b)
#endif

#ifdef _UNICODE				
	#define STRNCPY(a, b, l)	wcsncpy(a, b, l)
#else
	#define STRNCPY(a, b, l)	strncpy(a, b, l)
#endif

#ifdef _UNICODE				
	#define SPRINTF	wsprintf
#else
	#define SPRINTF	sprintf
#endif

#ifdef _UNICODE				
	#define STRNCPY(a, b, t)	wcsncpy(a, b, t)
#else
	#define STRNCPY(a, b, t)	strncpy(a, b, t)
#endif

#ifdef _UNICODE				
	#define ATOL(a)	_wtol(a)
#else
	#define ATOL(a)	atol(a)
#endif

#ifdef _UNICODE				
	#define ATOI(a)	_wtoi(a)
#else
	#define ATOI(a)	atoi(a)
#endif

#ifdef _UNICODE				
	#define SPLITPATH	_wsplitpath
#else
	#define SPLITPATH	_splitpath
#endif

#ifdef _UNICODE
	#define STAT	_wstat
#else
	#define STAT	_stat
#endif

#ifdef _UNICODE
#define STRICMP	_wcsicmp
#else
#define STRICMP	_stricmp
#endif

#ifdef _UNICODE
#define GETENV _wgetenv
#else
#define GETENV getenv
#endif
