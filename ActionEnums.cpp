#include "stdafx.h"
#include "ActionEnums.h"
#include "Accels.h"
#include "Misc.h"
#include "CP_Main.h"

ActionEnums::ActionEnums()
{
}


ActionEnums::~ActionEnums()
{
}


CString ActionEnums::EnumDescription(ActionEnumValues value)
{
	CString val = _T("");

	switch (value)
	{
	case SHOWDESCRIPTION:
		val = "View Full Description";
		break;
	case NEXTDESCRIPTION:
		val = "Next Full Description";
		break;
	case PREVDESCRIPTION:
		val = "Previous Full Description";
		break;
	case SHOWMENU:
		val = "Show Context Menu";
		break;
	case NEWGROUP:
		val = "New Group";
		break;
	case NEWGROUPSELECTION:
		val = "New Group Selection";
		break;
	case TOGGLEFILELOGGING:
		val = "Toggle On File Logging";
		break;
	case TOGGLEOUTPUTDEBUGSTRING:
		val = "Toggle OutputDebugString Logging";
		break;
	case CLOSEWINDOW:
		val = "Close Window";
		break;
	case NEXTTABCONTROL:
		val = "NEXTTABCONTROL";
		break;
	case PREVTABCONTROL:
		val = "PREVTABCONTROL";
		break;
	case SHOWGROUPS:
		val = "View Groups";
		break;
	case NEWCLIP:
		val = "New Clip";
		break;
	case EDITCLIP:
		val = "Edit Clip";
		break;
	case MODIFIER_ACTVE_SELECTIONUP:
		val = "MODIFIER_ACTVE_SELECTIONUP";
		break;
	case MODIFIER_ACTVE_SELECTIONDOWN:
		val = "MODIFIER_ACTVE_SELECTIONDOWN";
		break;
	case MODIFIER_ACTVE_MOVEFIRST:
		val = "MODIFIER_ACTVE_MOVEFIRST";
		break;
	case MODIFIER_ACTVE_MOVELAST:
		val = "MODIFIER_ACTVE_MOVELAST";
		break;
	case CANCELFILTER:
		val = "Cancel Filter";
		break;
	case HOMELIST:
		val = "HOMELIST";
		break;
	case BACKGRROUP:
		val = "Back Group";
		break;
	case TOGGLESHOWPERSISTANT:
		val = "Toggle Show Persistant";
		break;
	case PASTE_SELECTED:
		val = "Paste Selected";
		break;
	case DELETE_SELECTED:
		val = "Delete Selected";
		break;
	case CLIP_PROPERTIES:
		val = "Clip Properties";
		break;
	case PASTE_SELECTED_PLAIN_TEXT:
		val = "Paste Selected Plain Text";
		break;
	case MOVE_CLIP_TO_GROUP:
		val = "Move Clip To Group";
		break;
	case ELEVATE_PRIVlEGES:
		val = "Option - Elevate Privleges";
		break;
	case SHOW_IN_TASKBAR:
		val = "Option - Show In TaskBar";
		break;
	case COMPARE_SELECTED_CLIPS:
		val = "Compare Selected Clips";
		break;
	case SELECT_LEFT_SIDE_COMPARE:
		val = "Select Left File For Compare";
		break;
	case SELECT_RIGHT_SITE_AND_DO_COMPARE:
		val = "Select Right File And Do Compare";
		break;
	case EXPORT_TO_TEXT_FILE:
		val = "Export To Text File";
		break;
	case EXPORT_TO_QR_CODE:
		val = "Export To QR Code";
		break;
	case EXPORT_TO_GOOGLE_TRANSLATE:
		val = "Export To Google Translate";
		break;
	case EXPORT_TO_BITMAP_FILE:
		val = "Export To Image File";
		break;
	case SAVE_CURRENT_CLIPBOARD:
		val = "Save Current Clipboard";
		break;
	case MOVE_CLIP_UP:
		val = "Move Clip Up";
		break;
	case MOVE_CLIP_DOWN:
		val = "Move Clip Down";
		break;
	case MOVE_CLIP_TOP:
		val = "Move Clip Top";
		break;
	case FILTER_ON_SELECTED_CLIP:
		val = "Filter On Selected Clip";
		break;
	case PASTE_UPPER_CASE:
		val = "Paste Upper Case";
		break;
	case PASTE_LOWER_CASE:
		val = "Paste Lower Case";
		break;
	case PASTE_CAPITALiZE:
		val = "Paste Capitalize";
		break;
	case PASTE_SENTENCE_CASE:
		val = "Paste Sentence Case";
		break;
	case PASTE_REMOVE_LINE_FEEDS:
		val = "Paste Remove Line Feeds";
		break;
	case PASTE_ADD_ONE_LINE_FEED:
		val = "Paste Add One Line Feed";
		break;
	case PASTE_ADD_TWO_LINE_FEEDS:
		val = "Paste Add Two Line Feeds";
		break;
	case PASTE_TYPOGLYCEMIA:
		val = "Paste Typoglycemia";
		break;
	case SEND_TO_FRIEND_1:
		val = "Send to friend 1";
		break;
	case SEND_TO_FRIEND_2:
		val = "Send to friend 2";
		break;
	case SEND_TO_FRIEND_3:
		val = "Send to friend 3";
		break;
	case SEND_TO_FRIEND_4:
		val = "Send to friend 4";
		break;
	case SEND_TO_FRIEND_5:
		val = "Send to friend 5";
		break;
	case SEND_TO_FRIEND_6:
		val = "Send to friend 6";
		break;
	case SEND_TO_FRIEND_7:
		val = "Send to friend 7";
		break;
	case SEND_TO_FRIEND_8:
		val = "Send to friend 8";
		break;
	case SEND_TO_FRIEND_9:
		val = "Send to friend 9";
		break;
	case SEND_TO_FRIEND_10:
		val = "Send to friend 10";
		break;
	case SEND_TO_FRIEND_11:
		val = "Send to friend 11";
		break;
	case SEND_TO_FRIEND_12:
		val = "Send to friend 12";
		break;
	case SEND_TO_FRIEND_13:
		val = "Send to friend 13";
		break;
	case SEND_TO_FRIEND_14:
		val = "Send to friend 14";
		break;
	case SEND_TO_FRIEND_15:
		val = "Send to friend 15";
		break;
	case PASTE_POSITION_1:
		val = "Paste Position 1";
		break;
	case PASTE_POSITION_2:
		val = "Paste Position 2";
		break;
	case PASTE_POSITION_3:
		val = "Paste Position 3";
		break;
	case PASTE_POSITION_4:
		val = "Paste Position 4";
		break;
	case PASTE_POSITION_5:
		val = "Paste Position 5";
		break;
	case PASTE_POSITION_6:
		val = "Paste Position 6";
		break;
	case PASTE_POSITION_7:
		val = "Paste Position 7";
		break;
	case PASTE_POSITION_8:
		val = "Paste Position 8";
		break;
	case PASTE_POSITION_9:
		val = "Paste Position 9";
		break;
	case PASTE_POSITION_10:
		val = "Paste Position 10";
		break;
	case CONFIG_SHOW_FIRST_TEN_TEXT:
		val = "Option - Show text for first ten copy hot keys";
		break;
	case CONFIG_SHOW_CLIP_WAS_PASTED:
		val = "Option - Show indicator a clip has been pasted";
		break;
	case TOGGLE_LAST_GROUP_TOGGLE:
		val = "Toggle Last Group Toggle";
		break;
	case MAKE_TOP_STICKY:
		val = "Make Top Sticky Clip";
		break;
	case MAKE_LAST_STICKY:
		val = "Make Last Sticky Clip";
		break;
	case REMOVE_STICKY:
		val = "Remove Sticky Setting";
		break;
	case PASTE_ADD_CURRENT_TIME:
		val = "Paste Add Current Time";
		break;
	case IMPORT_CLIP:
		val = "Import Clip";
		break;
	case GLOBAl_HOTKEYS:
		val = "Global HotKeys";
		break;
	case DELETE_CLIP_DATA:
		val = "Delete Clip Data";
		break;
	case REPLACE_TOP_STICKY_CLIP:
		val = "Replace Top Sticky Clip";
		break;
	case PROMPT_SEND_TO_FRIEND:
		val = "Prompt send to friend";
		break;
	case SAVE_CF_HDROP_FIlE_DATA:
		val = "Save copied file (cf_hdrop) contents into Ditto";
		break;
	case TOGGLE_CLIPBOARD_CONNECTION:
		val = "Toggle clipboard connection";
		break;
	}

	CString translatedValue = theApp.m_Language.GetQuickPasteKeyboardString(value, val);

	if (translatedValue != _T(""))
	{
		return translatedValue;
	}

	return val;
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