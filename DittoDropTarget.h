
#pragma once


// CDittoDropTarget command target

class IDittoDropTarget
{
public:
	virtual DROPEFFECT DragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) = 0;
	virtual DROPEFFECT DragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) = 0;
	virtual void DragLeave(void) = 0;
	virtual BOOL Drop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) = 0;
};


class CDittoDropTarget : public COleDropTarget
{
	DECLARE_DYNAMIC(CDittoDropTarget)

public:
	CDittoDropTarget();
	virtual ~CDittoDropTarget();

	void SetDropTarget(IDittoDropTarget *pTarget) { m_pTarget = pTarget; }

protected:
	IDittoDropTarget *m_pTarget;

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
};


