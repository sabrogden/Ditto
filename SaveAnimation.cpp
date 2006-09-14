#include "stdafx.h"
#include ".\saveanimation.h"
#include "PerfTimer.h"

CSaveAnimation::CSaveAnimation(void)
{
	m_dLeftPercent = 0;
	m_dTopPercent = 0;
	m_dRightPercent = 0;
	m_dBottomPercent = 0;
	m_dSpeed = 40;
}

CSaveAnimation::~CSaveAnimation(void)
{
}

void CSaveAnimation::DoAnimation(CRect crStart, CRect crEnd, CWnd *pWnd)
{
	m_crStart = crStart;
	m_crEnd = crEnd;

	long lMaxDist = GetMaxDistance();
	GetPercentages(lMaxDist);

	CDC* pDC = pWnd->GetDC();
	CRect crCur(m_crStart);
	CRect crPrev(-1, -1, -1, -1);
	MSG msg;
	double dCurLeft = crCur.left;
	double dCurTop = crCur.top;
	double dCurRight = crCur.right;
	double dCurBottom = crCur.bottom;
	CPerfTimer Timer;

	for(int i = 0; i < lMaxDist/m_dSpeed; i++)
	{	
		//don't do the first time
		if(i > 0)
		{
			//wait 20ms between paints
			while(Timer.Elapsedms() < 10)
			{
				Sleep(1);
			}

			//Remove the old focus rect
			pDC->DrawFocusRect(crPrev);
		}
		Timer.Start(TRUE);

		pDC->DrawFocusRect(crCur);

		crPrev = crCur;

		dCurLeft -= m_dLeftPercent * m_dSpeed;
		dCurTop -= m_dTopPercent * m_dSpeed;
		dCurRight -= m_dRightPercent * m_dSpeed;
		dCurBottom -= m_dBottomPercent * m_dSpeed;

		crCur.left = (int)dCurLeft;
		crCur.top = (int)dCurTop;
		crCur.right = (int)dCurRight;
		crCur.bottom = (int)dCurBottom;

		while (PeekMessage(&msg, pWnd->m_hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//Draw one more time to remove the last focus rect
	pDC->DrawFocusRect(crPrev);
	while (PeekMessage(&msg, pWnd->m_hWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	pWnd->ReleaseDC(pDC);
}

long CSaveAnimation::GetMaxDistance()
{
	long lMax = 0;

	if(abs(m_crStart.left - m_crEnd.left) > lMax)
		lMax = abs(m_crStart.left - m_crEnd.left);

	if(abs(m_crStart.top - m_crEnd.top) > lMax)
		lMax = abs(m_crStart.top - m_crEnd.top);

	if(abs(m_crStart.right - m_crEnd.right) > lMax)
		lMax = abs(m_crStart.right - m_crEnd.right);

	if(abs(m_crStart.bottom - m_crEnd.bottom) > lMax)
		lMax = abs(m_crStart.bottom - m_crEnd.bottom);

	return lMax;
}

void CSaveAnimation::GetPercentages(long lMaxDist)
{
	if(lMaxDist > 0)
	{
		m_dLeftPercent = (m_crStart.left - m_crEnd.left) / (double)lMaxDist;
		m_dTopPercent = (m_crStart.top - m_crEnd.top) / (double)lMaxDist;
		m_dRightPercent = (m_crStart.right - m_crEnd.right) / (double)lMaxDist;
		m_dBottomPercent = (m_crStart.bottom - m_crEnd.bottom) / (double)lMaxDist;
	}
}
