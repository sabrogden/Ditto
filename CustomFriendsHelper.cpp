#include "stdafx.h"
#include "CustomFriendsHelper.h"
#include "CP_Main.h"
#include "Shared\Tokenizer.h"


CCustomFriendsHelper::CCustomFriendsHelper()
{
}


CCustomFriendsHelper::~CCustomFriendsHelper()
{
}

void CCustomFriendsHelper::Load()
{
	m_list.clear();

	CString oldValues = CGetSetOptions::GetCustomSendToList();
	CTokenizer token(oldValues, _T(","));
	CString line;

	bool setSelected = false;

	while (token.Next(line))
	{
		if (line != "")
		{
			m_list.push_back(line);
		}
	}
}

void CCustomFriendsHelper::Save()
{
	CString values = _T("");

	int count = m_list.size();
	for (int i = 0; i < count; i++)
	{
		CString lineValue = m_list[i];
		values += _T(",");
		values += lineValue;
	}

	CGetSetOptions::SetCustomSendToList(values);
}

void CCustomFriendsHelper::AddToMenu(CMenu *pMenu)
{
	bool addedSeparator = false;
	int id = 0;
	for (auto & element : m_list) 
	{
		if (addedSeparator == false)
		{
			pMenu->AppendMenu(MF_SEPARATOR);
			addedSeparator = true;
		}

		pMenu->AppendMenuW(MF_STRING, (CustomFriendStartId + id), element);
		id++;
	}

	if (addedSeparator)
	{
		pMenu->AppendMenu(MF_SEPARATOR);
		pMenu->AppendMenuW(MF_STRING, (CustomFriendStartId + ClearListId), _T("Clear Custom List"));
	}
}

void CCustomFriendsHelper::Add(CString item)
{
	int count = m_list.size();
	if (count < MaxCustomFriends)
	{
		m_list.push_back(item);
		Save();
	}
}

CString CCustomFriendsHelper::GetSendTo(int id)
{
	int index = id - CustomFriendStartId;
	if (index >= 0 && index < m_list.size())
	{
		return m_list[index];
	}

	if (index == ClearListId)
	{
		m_list.clear();
		this->Save();
	}

	return _T("");
}