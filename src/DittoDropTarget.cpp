// DittoDropTarget.cpp : implementation file
//

#include "stdafx.h"
#include "CP_Main.h"
#include "DittoDropTarget.h"
#include ".\dittodroptarget.h"


// CDittoDropTarget

IMPLEMENT_DYNAMIC(CDittoDropTarget, COleDropTarget)
CDittoDropTarget::CDittoDropTarget()
{
	m_pTarget = NULL;
}

CDittoDropTarget::~CDittoDropTarget()
{
}

BEGIN_MESSAGE_MAP(CDittoDropTarget, COleDropTarget)
END_MESSAGE_MAP()

BOOL CDittoDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	if(m_pTarget)
		return m_pTarget->Drop(pDataObject, dropEffect, point);

	return COleDropTarget::OnDrop(pWnd, pDataObject, dropEffect, point);
}


DROPEFFECT CDittoDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if(m_pTarget)
		return m_pTarget->DragEnter(pDataObject, dwKeyState, point);

	return COleDropTarget::OnDragEnter(pWnd, pDataObject, dwKeyState, point);
}

void CDittoDropTarget::OnDragLeave(CWnd* pWnd)
{
	if(m_pTarget)
		return m_pTarget->DragLeave();

	COleDropTarget::OnDragLeave(pWnd);
}

DROPEFFECT CDittoDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if(m_pTarget)
		return m_pTarget->DragOver(pDataObject, dwKeyState, point);

	return COleDropTarget::OnDragOver(pWnd, pDataObject, dwKeyState, point);
}
