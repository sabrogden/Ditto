#pragma once
class CDimWnd : public CFrameWnd
{
public:
	CDimWnd(CWnd *pParent);
	BOOL OnEraseBkgnd(CDC* pDC);

	~CDimWnd();

protected:
	DECLARE_MESSAGE_MAP()
};
