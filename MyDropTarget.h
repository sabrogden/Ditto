#pragma once
#include "afxole.h"

class CMyDropTarget : public COleDropTarget
{
public:
	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	void OnDragLeave(CWnd* pWnd);

	CMyDropTarget(CWnd *pMainWnd);
	virtual ~CMyDropTarget();

private:
	CWnd *m_pParent;
};

