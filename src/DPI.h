#pragma once
#include <ShellScalingAPI.h>

// Definition: relative pixel = 1 pixel at 96 DPI and scaled based on actual DPI.
class CDPI
{
public:
	CDPI(HWND hwnd = NULL) : m_Initialized(false), m_dpi(96)
	{
		m_hWnd = hwnd;
	}

	void Update(int dpi) { m_dpi = dpi;  m_Initialized = true; }

	// Get screen DPI.
	int GetDPI() { Init(); return m_dpi; }

	// Convert between raw pixels and relative pixels.
	int Scale(int x) { Init(); return MulDiv(x, m_dpi, 96); }
	int UnScale(int x) { Init(); return MulDiv(x, 96, m_dpi); }
	
	// Invalidate any cached metrics.
	void Invalidate() { m_Initialized = false; }

	void SetHwnd(HWND hwnd) { m_hWnd = hwnd; m_Initialized = false; Init(); }

private:
	void Init()
	{
		if (m_Initialized == false)
		{
			if (m_hWnd != NULL)
			{
				HMODULE hUser32 = LoadLibrary(_T("USER32.dll"));
				if (hUser32)
				{
					//windows 10
					typedef UINT(__stdcall *GetDpiForWindow)(HWND hwnd);
					GetDpiForWindow getDpi = (GetDpiForWindow)GetProcAddress(hUser32, "GetDpiForWindow");
					if (getDpi)
					{
						int dpi = getDpi(m_hWnd);
						this->Update(dpi);
						m_Initialized = true;
					}
					else
					{
						//windows 8
						auto monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
						HMODULE shCore = LoadLibrary(_T("Shcore.dll"));
						if (shCore)
						{
							typedef HRESULT(__stdcall *GetDpiForMonitor)(HMONITOR, UINT, UINT*, UINT*);
							GetDpiForMonitor monDpi = (GetDpiForMonitor)GetProcAddress(shCore, "GetDpiForMonitor");
							if (monDpi)
							{
								UINT x = 0;
								UINT y = 0;
								monDpi(monitor, MDT_EFFECTIVE_DPI, &x, &y);

								this->Update(x);
								m_Initialized = true;
							}
						}
					}
				}
			}

			if (m_Initialized == false)
			{
				HDC hdc = GetDC(m_hWnd);
				if (hdc)
				{
					m_dpi = GetDeviceCaps(hdc, LOGPIXELSX);
					ReleaseDC(NULL, hdc);
					m_Initialized = true;
				}				
			}
		}
	}

private:
	bool m_Initialized;
	int m_dpi;
	HWND m_hWnd;
};
