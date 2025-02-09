#include "stdafx.h"
#include "MyDropTarget.h"
#include "QPasteWnd.h"


CMyDropTarget::CMyDropTarget(CWnd *pMainWnd)
{
	m_pParent = pMainWnd;
}

CMyDropTarget::~CMyDropTarget()
{

}

DROPEFFECT CMyDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return ((CQPasteWnd*) m_pParent)->OnDragEnter(pDataObject, dwKeyState, point);
}

DROPEFFECT CMyDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return ((CQPasteWnd*) m_pParent)->OnDragOver(pDataObject, dwKeyState, point);
}

BOOL CMyDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	return ((CQPasteWnd*) m_pParent)->OnDrop(pDataObject, dropEffect, point);
}

void CMyDropTarget::OnDragLeave(CWnd* pWnd)
{
	return ((CQPasteWnd*) m_pParent)->OnDragLeave();
}

