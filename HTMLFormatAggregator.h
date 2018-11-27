#pragma once
#include "IClipAggregator.h"

class CHTMLFormatAggregator : public IClipAggregator
{
public:
	CHTMLFormatAggregator(CStringA csSepator);
	~CHTMLFormatAggregator(void);

	virtual bool AddClip(LPVOID lpData, int nDataSize, int nPos, int nCount, UINT cfType);
	virtual HGLOBAL GetHGlobal();

protected:
	CStringA m_csSeparator;
	CStringA m_csNewText;
	CStringA m_csSourceURL;
	CStringA m_csVersion;
};

class CHTMFormatStruct
{
public:
	CHTMFormatStruct()
	{
		m_lStartHTML = 0;
		m_lEndHTML = 0;
		m_lStartFragment = 0;
		m_lEndFragment = 0;
	}

	bool GetData(LPCSTR HTML);
	bool Serialize(CStringA &csHTMLFormat);

	const CStringA &GetFragment() { return m_csFragment; }
	const CStringA &GetURL()	  { return m_csSourceURL; }
	const CStringA &GetVersion()  { return m_csVersion; }

	void SetFragment(const CStringA &cs){ m_csFragment = cs; }
	void SetURL(const CStringA &cs)		{ m_csSourceURL = cs; }
	void SetVersion(const CStringA &cs)	{ m_csVersion = cs; }

protected:
	CStringA m_csVersion;
	long m_lStartHTML;
	long m_lEndHTML;
	long m_lStartFragment;
	long m_lEndFragment;
	CStringA m_csSourceURL;
	CStringA m_csFragment;
};
