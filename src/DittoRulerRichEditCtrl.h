#pragma once
#include "rulerricheditctrl\rulerricheditctrl.h"

#define SAVED_CLIP_TO_DB	1
#define DIDNT_NEED_TO_SAVE	2
class CDittoRulerRichEditCtrl :	public CRulerRichEditCtrl
{
public:
	CDittoRulerRichEditCtrl(void);
	~CDittoRulerRichEditCtrl(void);

	enum eSaveTypes{stNONE = 0x1, stCF_TEXT = 0x2, stCF_UNICODETEXT = 0x4, stRTF = 0x8};

	bool LoadItem(long lID, CString csDesc);
	int SaveToDB(BOOL bUpdateDesc);
	long GetTypeFlags(long lID);
	bool CloseEdit(bool bPrompt, BOOL bUpdateDesc);
	long GetDBID()		{ return m_lID; }
	CString GetDesc()	{ return m_csDescription; }

	void d();
	    
protected:
	long m_lID;
	CString m_csDescription;

	bool LoadRTFData(CClip &Clip);
	bool LoadTextData(CClip &Clip);
};
