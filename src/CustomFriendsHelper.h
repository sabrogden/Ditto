#pragma once

#include <vector>

#define CustomFriendStartId 4001
#define MaxCustomFriends 99
#define PromptForCustom 100

class CCustomFriendsHelper
{
public:
	CCustomFriendsHelper();
	~CCustomFriendsHelper();
	
	void Add(CString item, CString desc);
	void Load();
	void Save();
	void AddToMenu(CMenu *pMenu);
	CString GetSendTo(int id, bool &showDlg);
	void ClearList();

protected:

	class Name_Desc
	{
	public:
		CString m_name;
		CString m_desc;
	};

	std::vector<Name_Desc> m_list;
};

