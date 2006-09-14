#pragma once

class CSaveAnimation
{
public:
	CSaveAnimation();
	~CSaveAnimation(void);

	void DoAnimation(CRect crStart, CRect crEnd, CWnd *pWnd);
	void GetPercentages(long lMaxDist);
	long GetMaxDistance();
	
	CRect m_crStart;
	CRect m_crEnd;

	double m_dLeftPercent;
	double m_dTopPercent;
	double m_dRightPercent;
	double m_dBottomPercent;

	double m_dSpeed;
};