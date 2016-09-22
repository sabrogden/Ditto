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
		return "View Full Description";
	case NEXTDESCRIPTION:
		return "Next Full Description";
	case PREVDESCRIPTION:
		return "Previous Full Description";
	case SHOWMENU:
		return "Show Context Menu";
	case NEWGROUP:
		return "New Group";
	case NEWGROUPSELECTION:
		return "New Group Selection";
	case TOGGLEFILELOGGING:
		return "Toggle On File Logging";
	case TOGGLEOUTPUTDEBUGSTRING:
		return "Toggle OutputDebugString Logging";
	case CLOSEWINDOW:
		return "Close Window";
	case NEXTTABCONTROL:
		return "NEXTTABCONTROL";
	case PREVTABCONTROL:
		return "PREVTABCONTROL";
	case SHOWGROUPS:
		return "View Groups";
	case NEWCLIP:
		return "New Clip";
	case EDITCLIP:
		return "Edit Clip";
	case MODIFIER_ACTVE_SELECTIONUP:
		return "MODIFIER_ACTVE_SELECTIONUP";
	case MODIFIER_ACTVE_SELECTIONDOWN:
		return "MODIFIER_ACTVE_SELECTIONDOWN";
	case MODIFIER_ACTVE_MOVEFIRST:
		return "MODIFIER_ACTVE_MOVEFIRST";
	case MODIFIER_ACTVE_MOVELAST:
		return "MODIFIER_ACTVE_MOVELAST";
	case CANCELFILTER:
		return "Cancel Filter";
	case HOMELIST:
		return "HOMELIST";
	case BACKGRROUP:
		return "Back Group";
	case TOGGLESHOWPERSISTANT:
		return "Toggle Show Persistant";
	case PASTE_SELECTED:
		return "Paste Selected";
	case DELETE_SELECTED:
		return "Delete Selected";
	case CLIP_PROPERTIES:
		return "Clip Properties";
	case PASTE_SELECTED_PLAIN_TEXT:
		return "Paste Selected Plain Text";
	case MOVE_CLIP_TO_GROUP:
		return "Move Clip To Group";
	case ELEVATE_PRIVlEGES:
		return "Option - Elevate Privleges";
	case SHOW_IN_TASKBAR:
		return "Option - Show In TaskBar";
	case COMPARE_SELECTED_CLIPS:
		return "Compare Selected Clips";
	case SELECT_LEFT_SIDE_COMPARE:
		return "Select Left File For Compare";
	case SELECT_RIGHT_SITE_AND_DO_COMPARE:
		return "Select Right File And Do Compare";
	case EXPORT_TO_TEXT_FILE:
		return "Export To Text File";
	case EXPORT_TO_QR_CODE:
		return "Export To QR Code";
	case EXPORT_TO_GOOGLE_TRANSLATE:
		return "Export To Google Translate";
	case EXPORT_TO_BITMAP_FILE:
		return "Export To Image File";
	case SAVE_CURRENT_CLIPBOARD:
		return "Save Current Clipboard";
	case MOVE_CLIP_UP:
		return "Move Clip Up";
	case MOVE_CLIP_DOWN:
		return "Move Clip Down";
	case MOVE_CLIP_TOP:
		return "Move Clip Top";
	case FILTER_ON_SELECTED_CLIP:
		return "Filter On Selected Clip";
	case PASTE_UPPER_CASE:
		return "Paste Upper Case";
	case PASTE_LOWER_CASE:
		return "Paste Lower Case";
	case PASTE_CAPITALiZE:
		return "Paste Capitalize";
	case PASTE_SENTENCE_CASE:
		return "Paste Sentence Case";
	case PASTE_REMOVE_LINE_FEEDS:
		return "Paste Remove Line Feeds";
	case PASTE_ADD_ONE_LINE_FEED:
		return "Paste Add One Line Feed";
	case PASTE_ADD_TWO_LINE_FEEDS:
		return "Paste Add Two Line Feeds";
	case PASTE_TYPOGLYCEMIA:
		return "Paste Typoglycemia";
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
	case CONFIG_SHOW_FIRST_TEN_TEXT:
		return "Option - Show text for first ten copy hot keys";
	case CONFIG_SHOW_CLIP_WAS_PASTED:
		return "Option - Show indicator a clip has been pasted";
	case TOGGLE_LAST_GROUP_TOGGLE:
		return "Toggle Last Group Toggle";
	case MAKE_TOP_STICKY:
		return "Make Top Sticky Clip";
	case MAKE_LAST_STICKY:
		return "Make Last Sticky Clip";
	case REMOVE_STICKY:
		return "Remove Sticky Setting";
	case PASTE_ADD_CURRENT_TIME:
		return "Paste Add Current Time";
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
			return ACCEL_MAKEKEY('0', HOTKEYF_CONTROL);
		case CLOSEWINDOW:
			return VK_ESCAPE;
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
	case ActionEnums::MODIFIER_ACTVE_SELECTIONUP:
	case ActionEnums::MODIFIER_ACTVE_SELECTIONDOWN:
	case ActionEnums::MODIFIER_ACTVE_MOVEFIRST:
	case ActionEnums::MODIFIER_ACTVE_MOVELAST:
	case ActionEnums::BACKGRROUP:
	case ActionEnums::DELETE_SELECTED:
	case ActionEnums::TOGGLEFILELOGGING:
	case ActionEnums::TOGGLEOUTPUTDEBUGSTRING:
	case ActionEnums::HOMELIST:
		return false;
	}

	return true;
}