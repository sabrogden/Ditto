#include "stdafx.h"
#include "ActionEnums.h"
#include "Accels.h"
#include "Misc.h"

ActionEnums::ActionEnums()
{
}


ActionEnums::~ActionEnums()
{
}


CString ActionEnums::EnumDescription(ActionEnumValues value)
{
	switch (value)
	{
	case SHOWDESCRIPTION:
		return "SHOWDESCRIPTION";
	case NEXTDESCRIPTION:
		return "NEXTDESCRIPTION";
	case PREVDESCRIPTION:
		return "PREVDESCRIPTION";
	case SHOWMENU:
		return "SHOWMENU";
	case NEWGROUP:
		return "NEWGROUP";
	case NEWGROUPSELECTION:
		return "NEWGROUPSELECTION";
	case TOGGLEFILELOGGING:
		return "TOGGLEFILELOGGING";
	case TOGGLEOUTPUTDEBUGSTRING:
		return "TOGGLEOUTPUTDEBUGSTRING";
	case CLOSEWINDOW:
		return "CLOSEWINDOW";
	case NEXTTABCONTROL:
		return "NEXTTABCONTROL";
	case PREVTABCONTROL:
		return "PREVTABCONTROL";
	case SHOWGROUPS:
		return "SHOWGROUPS";
	case NEWCLIP:
		return "NEWCLIP";
	case EDITCLIP:
		return "EDITCLIP";
	case SELECTIONUP:
		return "SELECTIONUP";
	case SELECTIONDOWN:
		return "SELECTIONDOWN";
	case MOVEFIRST:
		return "MOVEFIRST";
	case MOVELAST:
		return "MOVELAST";
	case CANCELFILTER:
		return "CANCELFILTER";
	case HOMELIST:
		return "HOMELIST";
	case BACKGRROUP:
		return "BACKGRROUP";
	case TOGGLESHOWPERSISTANT:
		return "TOGGLESHOWPERSISTANT";
	case PASTE_SELECTED:
		return "PASTE_SELECTED";
	case DELETE_SELECTED:
		return "DELETE_SELECTED";
	case CLIP_PROPERTIES:
		return "CLIP_PROPERTIES";
	case PASTE_SELECTED_PLAIN_TEXT:
		return "PASTE_SELECTED_PLAIN_TEXT";
	case MOVE_CLIP_TO_GROUP:
		return "MOVE_CLIP_TO_GROUP";
	case ELEVATE_PRIVlEGES:
		return "ELEVATE_PRIVlEGES";
	case SHOW_IN_TASKBAR:
		return "SHOW_IN_TASKBAR";
	case COMPARE_SELECTED_CLIPS:
		return "COMPARE_SELECTED_CLIPS";
	case SELECT_LEFT_SIDE_COMPARE:
		return "SELECT_LEFT_SIDE_COMPARE";
	case SELECT_RIGHT_SITE_AND_DO_COMPARE:
		return "SELECT_RIGHT_SITE_AND_DO_COMPARE";
	case EXPORT_TO_TEXT_FILE:
		return "EXPORT_TO_TEXT_FILE";
	case EXPORT_TO_QR_CODE:
		return "EXPORT_TO_QR_CODE";
	case EXPORT_TO_GOOGLE_TRANSLATE:
		return "EXPORT_TO_GOOGLE_TRANSLATE";
	case EXPORT_TO_BITMAP_FILE:
		return "EXPORT_TO_BITMAP_FILE";
	case SAVE_CURRENT_CLIPBOARD:
		return "SAVE_CURRENT_CLIPBOARD";
	case MOVE_CLIP_UP:
		return "MOVE_CLIP_UP";
	case MOVE_CLIP_DOWN:
		return "MOVE_CLIP_DOWN";
	case MOVE_CLIP_TOP:
		return "MOVE_CLIP_TOP";
	case FILTER_ON_SELECTED_CLIP:
		return "FILTER_ON_SELECTED_CLIP";
	case PASTE_UPPER_CASE:
		return "PASTE_UPPER_CASE";
	case PASTE_LOWER_CASE:
		return "PASTE_LOWER_CASE";
	case PASTE_CAPITALiZE:
		return "PASTE_CAPITALiZE";
	case PASTE_SENTENCE_CASE:
		return "PASTE_SENTENCE_CASE";
	case PASTE_REMOVE_LINE_FEEDS:
		return "PASTE_REMOVE_LINE_FEEDS";
	case PASTE_ADD_ONE_LINE_FEED:
		return "PASTE_ADD_ONE_LINE_FEED";
	case PASTE_ADD_TWO_LINE_FEEDS:
		return "PASTE_ADD_TWO_LINE_FEEDS";
	case PASTE_TYPOGLYCEMIA:
		return "PASTE_TYPOGLYCEMIA";
	case SEND_TO_FRIEND_1:
		return "Send to friend 1";
	case SEND_TO_FRIEND_2:
		return "Send to friend 2";
	case SEND_TO_FRIEND_3:
		return "Send to friend 3";
	case SEND_TO_FRIEND_4:
		return "Send to friend 4";
	case SEND_TO_FRIEND_5:
		return "Send to friend 5";
	case SEND_TO_FRIEND_6:
		return "Send to friend 6";
	case SEND_TO_FRIEND_7:
		return "Send to friend 7";
	case SEND_TO_FRIEND_8:
		return "Send to friend 8";
	case SEND_TO_FRIEND_9:
		return "Send to friend 9";
	case SEND_TO_FRIEND_10:
		return "Send to friend 10";
	case SEND_TO_FRIEND_11:
		return "Send to friend 11";
	case SEND_TO_FRIEND_12:
		return "Send to friend 12";
	case SEND_TO_FRIEND_13:
		return "Send to friend 13";
	case SEND_TO_FRIEND_14:
		return "Send to friend 14";
	case SEND_TO_FRIEND_15:
		return "Send to friend 15";
	case PASTE_POSITION_1:
		return "Paste Position 1";
	case PASTE_POSITION_2:
		return "Paste Position 2";
	case PASTE_POSITION_3:
		return "Paste Position 3";
	case PASTE_POSITION_4:
		return "Paste Position 4";
	case PASTE_POSITION_5:
		return "Paste Position 5";
	case PASTE_POSITION_6:
		return "Paste Position 6";
	case PASTE_POSITION_7:
		return "Paste Position 7";
	case PASTE_POSITION_8:
		return "Paste Position 8";
	case PASTE_POSITION_9:
		return "Paste Position 9";
	case PASTE_POSITION_10:
		return "Paste Position 10";
	}

	return "";
}

int ActionEnums::GetDefaultShortCutKeyA(ActionEnumValues value, int pos)
{
	switch(pos)
	{
	case 0:
		switch (value)
		{
		case ActionEnums::SHOWDESCRIPTION:
			return VK_F3;
		case ActionEnums::NEXTDESCRIPTION:
			return 'N';
		case ActionEnums::PREVDESCRIPTION:
			return 'P';		
		case ActionEnums::NEWGROUP:
			return ACCEL_MAKEKEY(VK_F7, HOTKEYF_CONTROL);
		case ActionEnums::NEWGROUPSELECTION:
			return VK_F7;
		case ActionEnums::SHOWGROUPS:
			return ACCEL_MAKEKEY('G', HOTKEYF_CONTROL);
		case ActionEnums::NEWCLIP:
			return ACCEL_MAKEKEY('N', HOTKEYF_CONTROL);
		case ActionEnums::EDITCLIP:
			return ACCEL_MAKEKEY('E', HOTKEYF_CONTROL);
		case ActionEnums::CANCELFILTER:
			return ACCEL_MAKEKEY('C', HOTKEYF_ALT);
		case ActionEnums::TOGGLESHOWPERSISTANT:
			return ACCEL_MAKEKEY(VK_SPACE, HOTKEYF_CONTROL);
		case ActionEnums::CLIP_PROPERTIES:
			return ACCEL_MAKEKEY(VK_RETURN, HOTKEYF_ALT);
		case ActionEnums::PASTE_SELECTED_PLAIN_TEXT:
			return ACCEL_MAKEKEY(VK_RETURN, HOTKEYF_SHIFT);
		case ActionEnums::COMPARE_SELECTED_CLIPS:
			return ACCEL_MAKEKEY(VK_F2, HOTKEYF_CONTROL);
		case ActionEnums::PASTE_SELECTED:
			return VK_RETURN;
		case ActionEnums::SHOWMENU:
			return VK_MOUSE_RIGHT_CLICK;
		case PASTE_POSITION_1:
			return ACCEL_MAKEKEY('1', HOTKEYF_CONTROL);
		case PASTE_POSITION_2:
			return ACCEL_MAKEKEY('2', HOTKEYF_CONTROL);
		case PASTE_POSITION_3:
			return ACCEL_MAKEKEY('3', HOTKEYF_CONTROL);
		case PASTE_POSITION_4:
			return ACCEL_MAKEKEY('4', HOTKEYF_CONTROL);
		case PASTE_POSITION_5:
			return ACCEL_MAKEKEY('5', HOTKEYF_CONTROL);
		case PASTE_POSITION_6:
			return ACCEL_MAKEKEY('6', HOTKEYF_CONTROL);
		case PASTE_POSITION_7:
			return ACCEL_MAKEKEY('7', HOTKEYF_CONTROL);
		case PASTE_POSITION_8:
			return ACCEL_MAKEKEY('8', HOTKEYF_CONTROL);
		case PASTE_POSITION_9:
			return ACCEL_MAKEKEY('9', HOTKEYF_CONTROL);
		case PASTE_POSITION_10:
			return ACCEL_MAKEKEY('0 ', HOTKEYF_CONTROL);
		}
		break;
	case 1:
		switch (value)
		{
		case ActionEnums::PREVDESCRIPTION:
			return VK_UP;
		case ActionEnums::NEXTDESCRIPTION:
			return VK_DOWN;
		case ActionEnums::PASTE_SELECTED:
			return VK_MOUSE_DOUBLE_CLICK;
		case PASTE_POSITION_1:
			return ACCEL_MAKEKEY(VK_NUMPAD1, HOTKEYF_CONTROL);
		case PASTE_POSITION_2:
			return ACCEL_MAKEKEY(VK_NUMPAD2, HOTKEYF_CONTROL);
		case PASTE_POSITION_3:
			return ACCEL_MAKEKEY(VK_NUMPAD3, HOTKEYF_CONTROL);
		case PASTE_POSITION_4:
			return ACCEL_MAKEKEY(VK_NUMPAD4, HOTKEYF_CONTROL);
		case PASTE_POSITION_5:
			return ACCEL_MAKEKEY(VK_NUMPAD5, HOTKEYF_CONTROL);
		case PASTE_POSITION_6:
			return ACCEL_MAKEKEY(VK_NUMPAD6, HOTKEYF_CONTROL);
		case PASTE_POSITION_7:
			return ACCEL_MAKEKEY(VK_NUMPAD7, HOTKEYF_CONTROL);
		case PASTE_POSITION_8:
			return ACCEL_MAKEKEY(VK_NUMPAD8, HOTKEYF_CONTROL);
		case PASTE_POSITION_9:
			return ACCEL_MAKEKEY(VK_NUMPAD9, HOTKEYF_CONTROL);
		case PASTE_POSITION_10:
			return ACCEL_MAKEKEY(VK_NUMPAD0, HOTKEYF_CONTROL);
		}
		break;
	}

	return -1;
}

int ActionEnums::GetDefaultShortCutKeyB(ActionEnumValues value, int pos)
{
	switch (pos)
	{
	case 0:
		//switch (value)
		//{
		//}
		break;
	}

	return -1;
}

bool ActionEnums::UserConfigurable(ActionEnumValues value)
{
	switch (value)
	{
	case ActionEnums::NEXTTABCONTROL:
	case ActionEnums::PREVTABCONTROL:
	case ActionEnums::SELECTIONUP:
	case ActionEnums::SELECTIONDOWN:
	case ActionEnums::MOVEFIRST:
	case ActionEnums::MOVELAST:
	case ActionEnums::BACKGRROUP:
	case ActionEnums::DELETE_SELECTED:
	case ActionEnums::TOGGLEFILELOGGING:
	case ActionEnums::TOGGLEOUTPUTDEBUGSTRING:
	case ActionEnums::HOMELIST:
	case ActionEnums::CLOSEWINDOW:
		return false;
	}

	return true;
}