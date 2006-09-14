//
// Assorted defines and ids for the RulerRichEditCtrl
//

#ifndef _IDS_H_
#define _IDS_H_

// Some measures
#define TOOLBAR_HEIGHT		28
#define RULER_HEIGHT		26
#define TOP_HEIGHT			RULER_HEIGHT + TOOLBAR_HEIGHT
#define FONT_COMBO_WIDTH	128
#define SIZE_COMBO_WIDTH	48
#define COMBO_HEIGHT		128
#define COMBO_WIDTH			48
#define COLOR_WIDTH			64

#define FONT_NAME_POS		2
#define FONT_SIZE_POS		4
#define FONT_COLOR_POS		6

// Measures for the ruler
#define MODE_INCH	0
#define MODE_METRIC	1

// ID of sub-controls
#define TOOLBAR_CONTROL	10
#define RULER_CONTROL	11
#define RTF_CONTROL		12

// Toolbar buttons
#define BUTTON_FONT			20
#define BUTTON_COLOR		21
#define BUTTON_BOLD			22
#define BUTTON_ITALIC		23
#define BUTTON_UNDERLINE	24
#define BUTTON_LEFTALIGN	25
#define BUTTON_CENTERALIGN	26
#define BUTTON_RIGHTALIGN	27
#define BUTTON_INDENT		28
#define BUTTON_OUTDENT		29
#define BUTTON_BULLET		30

#define DROPDOWN_FONT		31
#define DROPDOWN_SIZE		32

#define STRING_COLOR		33
#define STRING_DEFAULT		34
#define STRING_CUSTOM		35

// Mouse handling
extern UINT urm_RULERACTION;
extern UINT urm_GETSCROLLPOS;
#define UP		0
#define DOWN	1
#define MOVE	2

#endif // _IDS_H_
