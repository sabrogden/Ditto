/*    PortTool v2.2     dibapi.h          */

/*
 *  dibapi.h
 *
 *  ** Win32 Version **
 *
 *  Copyright 1991-1998 Microsoft Corporation. All rights reserved
 *
 *  Header file for Device-Independent Bitmap (DIB) API.  Provides
 *  function prototypes and constants for the following functions:
 *
 *  BitmapToDIB()        - Creates a DIB from a bitmap
 *  ChangeBitmapFormat() - Changes a bitmap to a specified DIB format
 *  ChangeDIBFormat()    - Changes a DIB's BPP and/or compression format
 *  CopyScreenToBitmap() - Copies entire screen to a standard Bitmap
 *  CopyScreenToDIB()    - Copies entire screen to a DIB
 *  CopyWindowToBitmap() - Copies a window to a standard Bitmap
 *  CopyWindowToDIB()    - Copies a window to a DIB
 *  CreateDIBPalette()   - Creates a palette from a DIB
 *  CreateDIB()          - Creates a new DIB
 *  DestroyDIB()         - Deletes DIB when finished using it
 *  DIBError()           - Displays message box with error message
 *  DIBHeight()          - Gets the DIB height
 *  DIBNumColors()       - Calculates number of colors in the DIB's color table
 *  DIBToBitmap()        - Creates a bitmap from a DIB
 *  DIBWidth()           - Gets the DIB width
 *  FindDIBBits()        - Sets pointer to the DIB bits
 *  GetSystemPalette()   - Gets the current palette
 *  LoadDIB()            - Loads a DIB from a file
 *  PaintBitmap()        - Displays standard bitmap in the specified DC
 *  PaintDIB()           - Displays DIB in the specified DC
 *  PalEntriesOnDevice() - Gets the number of palette entries
 *  PaletteSize()        - Calculates the buffer size required by a palette
 *  PrintDIB()           - Prints the specified DIB
 *  PrintScreen()        - Prints the entire screen
 *  PrintWindow()        - Prints all or part of a window
 *  SaveDIB()            - Saves the specified dib in a file
 *
 * See the file DIBAPI.TXT for more information about these functions.
 *
 */


/* Handle to a DIB */
#define HDIB HANDLE


/* Print Area selection */
#define PW_WINDOW        1
#define PW_CLIENT        2


/* Print Options selection */
#define PW_BESTFIT       1
#define PW_STRETCHTOPAGE 2
#define PW_SCALE         3

/* DIB Macros*/

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed 
// to hold those bits.

#ifndef __AMVIDEO__
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

/* Error constants */
enum {
      ERR_MIN = 0,                     // All error #s >= this value
      ERR_NOT_DIB = 0,                 // Tried to load a file, NOT a DIB!
      ERR_MEMORY,                      // Not enough memory!
      ERR_READ,                        // Error reading file!
      ERR_LOCK,                        // Error on a GlobalLock()!
      ERR_OPEN,                        // Error opening a file!
      ERR_CREATEPAL,                   // Error creating palette.
      ERR_GETDC,                       // Couldn't get a DC.
      ERR_CREATEDDB,                   // Error create a DDB.
      ERR_STRETCHBLT,                  // StretchBlt() returned failure.
      ERR_STRETCHDIBITS,               // StretchDIBits() returned failure.
      ERR_SETDIBITSTODEVICE,           // SetDIBitsToDevice() failed.
      ERR_STARTDOC,                    // Error calling StartDoc().
      ERR_NOGDIMODULE,                 // Couldn't find GDI module in memory.
      ERR_SETABORTPROC,                // Error calling SetAbortProc().
      ERR_STARTPAGE,                   // Error calling StartPage().
      ERR_NEWFRAME,                    // Error calling NEWFRAME escape.
      ERR_ENDPAGE,                     // Error calling EndPage().
      ERR_ENDDOC,                      // Error calling EndDoc().
      ERR_SETDIBITS,                   // Error calling SetDIBits().
      ERR_FILENOTFOUND,                // Error opening file in GetDib()
      ERR_INVALIDHANDLE,               // Invalid Handle
      ERR_DIBFUNCTION,                 // Error on call to DIB function
      ERR_MAX                          // All error #s < this value
     };



/* Function prototypes */

HDIB BitmapToDIB (HBITMAP hBitmap, HPALETTE hPal);
HDIB ChangeBitmapFormat (HBITMAP	hBitmap,
                                   WORD     wBitCount,
                                   DWORD    dwCompression,
                                   HPALETTE hPal);
HDIB ChangeDIBFormat (HDIB hDIB, WORD wBitCount,
                                DWORD dwCompression);
HBITMAP CopyScreenToBitmap (LPRECT);
HDIB CopyScreenToDIB (LPRECT);
HBITMAP CopyWindowToBitmap (HWND, WORD);
HDIB CopyWindowToDIB (HWND, WORD);
HPALETTE CreateDIBPalette (HDIB);
HDIB CreateDIB(DWORD, DWORD, WORD);
WORD DestroyDIB (HDIB);
void DIBError (int ErrNo);
DWORD DIBHeight (LPSTR lpDIB);
WORD DIBNumColors (LPSTR lpDIB);
HBITMAP DIBToBitmap (HDIB hDIB, HPALETTE hPal);
DWORD DIBWidth (LPSTR lpDIB);
LPSTR FindDIBBits (LPSTR lpDIB);
HPALETTE GetSystemPalette (void);
HDIB LoadDIB (LPSTR);
BOOL PaintBitmap (HDC, LPRECT, HBITMAP, LPRECT, HPALETTE);
BOOL PaintDIB (HDC, LPRECT, HDIB, LPRECT, HPALETTE);
int PalEntriesOnDevice (HDC hDC);
WORD PaletteSize (LPSTR lpDIB);
WORD PrintDIB (HDIB, WORD, WORD, WORD, LPSTR);
WORD PrintScreen (LPRECT, WORD, WORD, WORD, LPSTR);
WORD PrintWindow (HWND, WORD, WORD, WORD, WORD, LPSTR);
WORD SaveDIB (HDIB, LPSTR);
