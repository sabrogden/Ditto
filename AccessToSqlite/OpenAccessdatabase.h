#pragma once

#define ERROR_OPENING_DATABASE -2

#pragma warning(disable : 4995)

class COpenAccessdatabase
{
public:
	COpenAccessdatabase(void);
	~COpenAccessdatabase(void);

	BOOL ValidDB(CString csPath, BOOL bUpgrade);
};


class CDaoTableDefEx : public CDaoTableDef
{
public:
	CDaoTableDefEx(CDaoDatabase* pDatabase)
		:CDaoTableDef(pDatabase)
	{
	}

	BOOL CreateIndex(BOOL bPrimaryKey, CString csField)
	{
		try
		{
			CDaoIndexInfo IndexInfo;
			CDaoIndexFieldInfo IndexFieldInfo[1];
			IndexFieldInfo[0].m_strName = csField;
			IndexFieldInfo[0].m_bDescending = TRUE;
			IndexInfo.m_strName = csField;
			IndexInfo.m_pFieldInfos = IndexFieldInfo;
			IndexInfo.m_nFields = 1;
			IndexInfo.m_bPrimary = bPrimaryKey;
			IndexInfo.m_bUnique = FALSE;
			IndexInfo.m_bIgnoreNulls = FALSE;
			IndexInfo.m_bRequired = FALSE;

			CDaoTableDef::CreateIndex( IndexInfo );
		}
		catch(CDaoException *e)
		{
			ASSERT(FALSE);
			e->Delete();
			return FALSE;
		}

		return TRUE;
	}

	BOOL CreateField(LPCTSTR lpszName, short nType, long lSize, long lAttributes = 0, CString csDefault = _T(""))
	{
		try
		{
			CDaoFieldInfo fieldinfo;

			// Initialize everything so only correct properties will be set
			fieldinfo.m_strName = lpszName;
			fieldinfo.m_nType = nType;
			fieldinfo.m_lSize = lSize;
			fieldinfo.m_lAttributes = lAttributes;
			fieldinfo.m_nOrdinalPosition = 0;
			fieldinfo.m_bRequired = FALSE;
			fieldinfo.m_bAllowZeroLength = FALSE;
			fieldinfo.m_lCollatingOrder = 0;
			fieldinfo.m_strDefaultValue = csDefault;

			CDaoTableDef::CreateField(fieldinfo);

			return TRUE;
		}
		catch(CDaoException *e)
		{
			e->Delete();
			ASSERT(FALSE);
		}

		return FALSE;
	}

};
