#pragma once

#include <vector>

#define CustomFriendStartId 4001
#define MaxCustomFriends 99
#define ClearListId 100

class CCustomFriendsHelper
{
public:
	CCustomFriendsHelper();
	~CCustomFriendsHelper();

	void Add(CString item);
	void Load();
	void Save();
	void AddToMenu(CMenu *pMenu);
	CString GetSendTo(int id);

protected:
	std::vector<CString> m_list;
};

