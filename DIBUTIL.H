/*    PortTool v2.2     dibutil.h          */

/*
 *  dibutil.h
 *
 *  Copyright 1991-1998 Microsoft Corporation. All rights reserved.
 *
 *  Header file for Device-Independent Bitmap (DIB) API.  Provides
 *  function prototypes and constants for the following functions:
 *
 *  AllocRoomForDIB()   - Allocates memory for a DIB
 *
 */


/* DIB constants */
#define PALVERSION   0x300

/* DIB macros */
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

/* function prototypes */
HANDLE          AllocRoomForDIB(BITMAPINFOHEADER bi, HBITMAP hBitmap);
