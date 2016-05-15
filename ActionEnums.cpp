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
	case ActionEnums::SHOWMENU:
		return false;
	}

	return true;
}