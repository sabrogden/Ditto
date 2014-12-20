/* DrawHTML()
 * Drop-in replacement for DrawText() supporting a tiny subset of HTML.
 */

#if defined __cplusplus
extern "C"
#endif
int __stdcall DrawHTML(
                       HDC     hdc,        // handle of device context
                       LPCTSTR lpString,   // address of string to draw
                       int     nCount,     // string length, in characters
                       LPRECT  lpRect,     // address of structure with formatting dimensions
                       UINT    uFormat     // text-drawing flags
                      );
