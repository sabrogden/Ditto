// ProcessPaste.h: interface for the CProcessCopy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
#define AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CProcessPaste
{
public:
	CProcessPaste();
	virtual ~CProcessPaste();

	BOOL LoadDataAndPaste(long lID, HWND hWnd);
	BOOL LoadDataAndDrag(long lID);

protected:
	void SendPaste(HWND hWnd);
	BOOL LoadData(long lID, COleDataSource *pData);
	BOOL MarkDataAsPasted(long lID);
};

#endif // !defined(AFX_PROCESSPASTE_H__185CBB6F_4B63_4397_8FF9_E18D777DA506__INCLUDED_)
