/*
 * Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2010, Intel Corporation.
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */


#include <X11/keysym.h>
#include <qstring.h>

#include "qt-keysym-map.h"
#include "debug.h"


typedef struct {
	uint XKeySym;
	int QtKey;
} KeySymMap;

// keyboard mapping table, modified from QKeymapper_x11.cpp
static const KeySymMap QtKeyXSymMaps[] = {

// misc keys

	{ XK_Escape,                  Qt::Key_Escape },
	{ XK_Tab,                     Qt::Key_Tab },
	{ XK_ISO_Left_Tab,            Qt::Key_Backtab },
	{ XK_BackSpace,               Qt::Key_Backspace },
	{ XK_Return,                  Qt::Key_Return },
	{ XK_Insert,                  Qt::Key_Insert },
	{ XK_Delete,                  Qt::Key_Delete },
	{ XK_Clear,                   Qt::Key_Delete },
	{ XK_Pause,                   Qt::Key_Pause },
	{ XK_Print,                   Qt::Key_Print },
	{ 0x1005FF60,                 Qt::Key_SysReq },         // hardcoded Sun SysReq
	{ 0x1007ff00,                 Qt::Key_SysReq },         // hardcoded X386 SysReq

// cursor movement

	{ XK_Home,                    Qt::Key_Home },
	{ XK_End,                     Qt::Key_End },
	{ XK_Left,                    Qt::Key_Left },
	{ XK_Up,                      Qt::Key_Up },
	{ XK_Right,                   Qt::Key_Right },
	{ XK_Down,                    Qt::Key_Down },
	{ XK_Prior,                   Qt::Key_PageUp },
	{ XK_Next,                    Qt::Key_PageDown },

// modifiers

	{ XK_Shift_L,                 Qt::Key_Shift },
	{ XK_Shift_R,                 Qt::Key_Shift },
	{ XK_Shift_Lock,              Qt::Key_Shift },
	{ XK_Control_L,               Qt::Key_Control },
	{ XK_Control_R,               Qt::Key_Control },
	{ XK_Meta_L,                  Qt::Key_Meta },
	{ XK_Meta_R,                  Qt::Key_Meta },
	{ XK_Alt_L,                   Qt::Key_Alt },
	{ XK_Alt_R,                   Qt::Key_Alt },
	{ XK_Caps_Lock,               Qt::Key_CapsLock },
	{ XK_Num_Lock,                Qt::Key_NumLock },
	{ XK_Scroll_Lock,             Qt::Key_ScrollLock },
	{ XK_Super_L,                 Qt::Key_Super_L },
	{ XK_Super_R,                 Qt::Key_Super_R },
	{ XK_Menu,                    Qt::Key_Menu },
	{ XK_Hyper_L,                 Qt::Key_Hyper_L },
	{ XK_Hyper_R,                 Qt::Key_Hyper_R },
	{ XK_Help,                    Qt::Key_Help },
	{ 0x1000FF74,                 Qt::Key_Backtab },        // hardcoded HP backtab
	{ 0x1005FF10,                 Qt::Key_F11 },            // hardcoded Sun F36 (labeled F11)
	{ 0x1005FF11,                 Qt::Key_F12 },            // hardcoded Sun F37 (labeled F12)

// function keys

	{ XK_F1,                   Qt::Key_F1 },
	{ XK_F2,                   Qt::Key_F2 },
	{ XK_F3,                   Qt::Key_F3 },
	{ XK_F4,                   Qt::Key_F4 },
	{ XK_F5,                   Qt::Key_F5 },
	{ XK_F6,                   Qt::Key_F6 },
	{ XK_F7,                   Qt::Key_F7 },
	{ XK_F8,                   Qt::Key_F8 },
	{ XK_F9,                   Qt::Key_F9 },
	{ XK_F10,                  Qt::Key_F10 },
	{ XK_F11,                  Qt::Key_F11 },
	{ XK_F12,                  Qt::Key_F12 },
	{ XK_F13,                  Qt::Key_F13 },
	{ XK_F14,                  Qt::Key_F14 },
	{ XK_F15,                  Qt::Key_F15 },
	{ XK_F16,                  Qt::Key_F16 },
	{ XK_F17,                  Qt::Key_F17 },
	{ XK_F18,                  Qt::Key_F18 },
	{ XK_F19,                  Qt::Key_F19 },
	{ XK_F20,                  Qt::Key_F20 },

// numeric and function keypad keys

	{ XK_KP_Space,                Qt::Key_Space },
	{ XK_KP_Tab,                  Qt::Key_Tab },
	{ XK_KP_Enter,                Qt::Key_Enter },
	{ XK_KP_F1,                   Qt::Key_F1 },
	{ XK_KP_F2,                   Qt::Key_F2 },
	{ XK_KP_F3,                   Qt::Key_F3 },
	{ XK_KP_F4,                   Qt::Key_F4 },

	{ XK_KP_Home,                 Qt::Key_Home },
	{ XK_KP_Left,                 Qt::Key_Left },
	{ XK_KP_Up,                   Qt::Key_Up },
	{ XK_KP_Right,                Qt::Key_Right },
	{ XK_KP_Down,                 Qt::Key_Down },
	{ XK_KP_Prior,                Qt::Key_PageUp },
	{ XK_KP_Next,                 Qt::Key_PageDown },
	{ XK_KP_End,                  Qt::Key_End },
	{ XK_KP_Begin,                Qt::Key_Clear },
	{ XK_KP_Insert,               Qt::Key_Insert },
	{ XK_KP_Delete,               Qt::Key_Delete },
	{ XK_KP_Equal,                Qt::Key_Equal },
	{ XK_KP_Multiply,             Qt::Key_Asterisk },
	{ XK_KP_Add,                  Qt::Key_Plus },
	{ XK_KP_Separator,            Qt::Key_Comma },
	{ XK_KP_Subtract,             Qt::Key_Minus },
	{ XK_KP_Decimal,              Qt::Key_Period },
	{ XK_KP_Divide,               Qt::Key_Slash },

	{ XK_KP_0, 	              Qt::Key_0 },
	{ XK_KP_1, 	              Qt::Key_1 },
	{ XK_KP_2, 	              Qt::Key_2 },
	{ XK_KP_3, 	              Qt::Key_3 },
	{ XK_KP_4, 	              Qt::Key_4 },
	{ XK_KP_5, 	              Qt::Key_5 },
	{ XK_KP_6, 	              Qt::Key_6 },
	{ XK_KP_7, 	              Qt::Key_7 },
	{ XK_KP_8, 	              Qt::Key_8 },
	{ XK_KP_9, 	              Qt::Key_9 },

//International input method support keys
//International & multi-key character composition

	{ XK_ISO_Level3_Shift,        Qt::Key_AltGr },
	{ XK_Multi_key,		Qt::Key_Multi_key },
	{ XK_Codeinput,		Qt::Key_Codeinput },
	{ XK_SingleCandidate,		Qt::Key_SingleCandidate },
	{ XK_MultipleCandidate,	Qt::Key_MultipleCandidate },
	{ XK_PreviousCandidate,	Qt::Key_PreviousCandidate },

// Misc Functions
	{ XK_Mode_switch,		Qt::Key_Mode_switch },
	{ XK_script_switch,		Qt::Key_Mode_switch },

// Japanese keyboard support
	{ XK_Kanji,			Qt::Key_Kanji },
	{ XK_Muhenkan,		Qt::Key_Muhenkan },
	//{ XK_Henkan_Mode,		Qt::Key_Henkan_Mode },
	{ XK_Henkan_Mode,		Qt::Key_Henkan },
	{ XK_Henkan,			Qt::Key_Henkan },
	{ XK_Romaji,			Qt::Key_Romaji },
	{ XK_Hiragana,		Qt::Key_Hiragana },
	{ XK_Katakana,		Qt::Key_Katakana },
	{ XK_Hiragana_Katakana,	Qt::Key_Hiragana_Katakana },
	{ XK_Zenkaku,			Qt::Key_Zenkaku },
	{ XK_Hankaku,			Qt::Key_Hankaku },
	{ XK_Zenkaku_Hankaku,		Qt::Key_Zenkaku_Hankaku },
	{ XK_Touroku,			Qt::Key_Touroku },
	{ XK_Massyo,			Qt::Key_Massyo },
	{ XK_Kana_Lock,		Qt::Key_Kana_Lock },
	{ XK_Kana_Shift,		Qt::Key_Kana_Shift },
	{ XK_Eisu_Shift,		Qt::Key_Eisu_Shift },
	{ XK_Eisu_toggle,		Qt::Key_Eisu_toggle },
	//{ XK_Kanji_Bangou,		Qt::Key_Kanji_Bangou },
	//{ XK_Zen_Koho,		Qt::Key_Zen_Koho },
	//{ XK_Mae_Koho,		Qt::Key_Mae_Koho },
	{ XK_Kanji_Bangou,		Qt::Key_Codeinput },
	{ XK_Zen_Koho,		Qt::Key_MultipleCandidate },
	{ XK_Mae_Koho,		Qt::Key_PreviousCandidate },

#ifdef XK_KOREAN
// Korean keyboard support
	{ XK_Hangul,			Qt::Key_Hangul },
	{ XK_Hangul_Start,		Qt::Key_Hangul_Start },
	{ XK_Hangul_End,		Qt::Key_Hangul_End },
	{ XK_Hangul_Hanja,		Qt::Key_Hangul_Hanja },
	{ XK_Hangul_Jamo,		Qt::Key_Hangul_Jamo },
	{ XK_Hangul_Romaja,		Qt::Key_Hangul_Romaja },
	//{ XK_Hangul_Codeinput,	Qt::Key_Hangul_Codeinput },
	{ XK_Hangul_Codeinput,	Qt::Key_Codeinput },
	{ XK_Hangul_Jeonja,		Qt::Key_Hangul_Jeonja },
	{ XK_Hangul_Banja,		Qt::Key_Hangul_Banja },
	{ XK_Hangul_PreHanja,		Qt::Key_Hangul_PreHanja },
	{ XK_Hangul_PostHanja,	Qt::Key_Hangul_PostHanja },
	//{ XK_Hangul_SingleCandidate,Qt::Key_Hangul_SingleCandidate },
	//{ XK_Hangul_MultipleCandidate,Qt::Key_Hangul_MultipleCandidate },
	//{ XK_Hangul_PreviousCandidate,Qt::Key_Hangul_PreviousCandidate },
	{ XK_Hangul_SingleCandidate,	Qt::Key_SingleCandidate },
	{ XK_Hangul_MultipleCandidate,Qt::Key_MultipleCandidate },
	{ XK_Hangul_PreviousCandidate,Qt::Key_PreviousCandidate },
	{ XK_Hangul_Special,		Qt::Key_Hangul_Special },
	//{ XK_Hangul_switch,		Qt::Key_Hangul_switch },
	{ XK_Hangul_switch,		Qt::Key_Mode_switch },
#endif  // XK_KOREAN

// dead keys
	{ XK_dead_grave,              Qt::Key_Dead_Grave },
	{ XK_dead_acute,              Qt::Key_Dead_Acute },
	{ XK_dead_circumflex,         Qt::Key_Dead_Circumflex },
	{ XK_dead_tilde,              Qt::Key_Dead_Tilde },
	{ XK_dead_macron,             Qt::Key_Dead_Macron },
	{ XK_dead_breve,              Qt::Key_Dead_Breve },
	{ XK_dead_abovedot,           Qt::Key_Dead_Abovedot },
	{ XK_dead_diaeresis,          Qt::Key_Dead_Diaeresis },
	{ XK_dead_abovering,          Qt::Key_Dead_Abovering },
	{ XK_dead_doubleacute,        Qt::Key_Dead_Doubleacute },
	{ XK_dead_caron,              Qt::Key_Dead_Caron },
	{ XK_dead_cedilla,            Qt::Key_Dead_Cedilla },
	{ XK_dead_ogonek,             Qt::Key_Dead_Ogonek },
	{ XK_dead_iota,               Qt::Key_Dead_Iota },
	{ XK_dead_voiced_sound,       Qt::Key_Dead_Voiced_Sound },
	{ XK_dead_semivoiced_sound,   Qt::Key_Dead_Semivoiced_Sound },
	{ XK_dead_belowdot,           Qt::Key_Dead_Belowdot },
	{ XK_dead_hook,               Qt::Key_Dead_Hook },
	{ XK_dead_horn,               Qt::Key_Dead_Horn },

	{ 0,                          0}
};

int
QtKeyToXKeySym(int qtKey) {

	// TODO: Need to complete this function for all scenario.
	int i;
	int count = sizeof(QtKeyXSymMaps)/sizeof(KeySymMap);

	if (qtKey < 0x1000) {
		//return QChar(qtKey).toLower().unicode();
		return qtKey;
	}

	for(i = 0; i < count; i++ ) {
		if( QtKeyXSymMaps[i].QtKey == qtKey ) {
			return QtKeyXSymMaps[i].XKeySym;
		}
	}

        return 0;
}


int
XKeySymToQTKey(uint keySym)
{
	int i;
	int count = sizeof(QtKeyXSymMaps)/sizeof(KeySymMap);

	if ((keySym < 0x1000)) {
		//if (keySym >= 'a' && keySym <= 'z')
		//	return QChar(keySym).toUpper().toAscii();
		return keySym;
	}

#ifdef Q_WS_WIN
        if(keySym < 0x3000)
                return keySym;
#else
	if (keySym < 0x3000 )
		return keySym;

	for(i = 0; i < count; i++)
		if(QtKeyXSymMaps[i].XKeySym == keySym)
			return QtKeyXSymMaps[i].QtKey;
#endif
        return Qt::Key_unknown;
}

