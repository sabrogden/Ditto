#include "stdafx.h"
#include ".\cf_hdropaggregator.h"

CCF_HDropAggregator::CCF_HDropAggregator(void)
{
}

CCF_HDropAggregator::~CCF_HDropAggregator(void)
{
}

bool CCF_HDropAggregator::AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType)
{
	HDROP drop = (HDROP)GlobalLock((HDROP)lpData);
	int nNumFiles = DragQueryFile(drop, -1, NULL, 0);
	TCHAR file[MAX_PATH];

	for(int nFile = 0; nFile < nNumFiles; nFile++)
	{
		if(DragQueryFile(drop, nFile, file, sizeof(file)) > 0)
		{
			m_DropFiles.AddFile(file);
		}
	}

	return true;
}

HGLOBAL CCF_HDropAggregator::GetHGlobal()
{
	return m_DropFiles.CreateCF_HDROPBuffer();
}

HGLOBAL CCF_HDropAggregator::GetHGlobalAsString()
{
	return m_DropFiles.CreateCF_HDROPBufferAsString();
}