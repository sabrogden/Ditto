// FormatSQL.h: interface for the CFormatSQL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMATSQL_H__3D7AC79C_FDD8_4948_B7CD_601FB513F208__INCLUDED_)
#define AFX_FORMATSQL_H__3D7AC79C_FDD8_4948_B7CD_601FB513F208__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFormatSQL  
{
public:
	CFormatSQL();
	virtual ~CFormatSQL();

	void Parse(CString cs);

	CString GetSQLString()				{ return m_csWhere; }
	void	SetVariable(CString cs)		{ m_csVariable = cs;}

protected:
	CString m_csWhere;
	CString m_csVariable;
	enum eSpecialTypes{eINVALID, eNOT, eAND, eOR};
	

	bool AddToSQL(CString cs, eSpecialTypes &eNOTValue, eSpecialTypes &eORValue);
	CFormatSQL::eSpecialTypes ConvetToKey(CString cs);
	CString GetKeyWordString(eSpecialTypes eKeyWord);
};

#endif // !defined(AFX_FORMATSQL_H__3D7AC79C_FDD8_4948_B7CD_601FB513F208__INCLUDED_)
