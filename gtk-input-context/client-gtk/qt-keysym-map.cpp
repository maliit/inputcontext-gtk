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


#include <gdk/gdkkeysyms.h>
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

	{ GDK_KEY_Escape,                  Qt::Key_Escape },
	{ GDK_KEY_Tab,                     Qt::Key_Tab },
	{ GDK_KEY_ISO_Left_Tab,            Qt::Key_Backtab },
	{ GDK_KEY_BackSpace,               Qt::Key_Backspace },
	{ GDK_KEY_Return,                  Qt::Key_Return },
	{ GDK_KEY_Insert,                  Qt::Key_Insert },
	{ GDK_KEY_Delete,                  Qt::Key_Delete },
	{ GDK_KEY_Clear,                   Qt::Key_Delete },
	{ GDK_KEY_Pause,                   Qt::Key_Pause },
	{ GDK_KEY_Print,                   Qt::Key_Print },
	{ 0x1005FF60,                      Qt::Key_SysReq },         // hardcoded Sun SysReq
	{ 0x1007ff00,                      Qt::Key_SysReq },         // hardcoded X386 SysReq

// cursor movement

	{ GDK_KEY_Home,                    Qt::Key_Home },
	{ GDK_KEY_End,                     Qt::Key_End },
	{ GDK_KEY_Left,                    Qt::Key_Left },
	{ GDK_KEY_Up,                      Qt::Key_Up },
	{ GDK_KEY_Right,                   Qt::Key_Right },
	{ GDK_KEY_Down,                    Qt::Key_Down },
	{ GDK_KEY_Prior,                   Qt::Key_PageUp },
	{ GDK_KEY_Next,                    Qt::Key_PageDown },

// modifiers

	{ GDK_KEY_Shift_L,                 Qt::Key_Shift },
	{ GDK_KEY_Shift_R,                 Qt::Key_Shift },
	{ GDK_KEY_Shift_Lock,              Qt::Key_Shift },
	{ GDK_KEY_Control_L,               Qt::Key_Control },
	{ GDK_KEY_Control_R,               Qt::Key_Control },
	{ GDK_KEY_Meta_L,                  Qt::Key_Meta },
	{ GDK_KEY_Meta_R,                  Qt::Key_Meta },
	{ GDK_KEY_Alt_L,                   Qt::Key_Alt },
	{ GDK_KEY_Alt_R,                   Qt::Key_Alt },
	{ GDK_KEY_Caps_Lock,               Qt::Key_CapsLock },
	{ GDK_KEY_Num_Lock,                Qt::Key_NumLock },
	{ GDK_KEY_Scroll_Lock,             Qt::Key_ScrollLock },
	{ GDK_KEY_Super_L,                 Qt::Key_Super_L },
	{ GDK_KEY_Super_R,                 Qt::Key_Super_R },
	{ GDK_KEY_Menu,                    Qt::Key_Menu },
	{ GDK_KEY_Hyper_L,                 Qt::Key_Hyper_L },
	{ GDK_KEY_Hyper_R,                 Qt::Key_Hyper_R },
	{ GDK_KEY_Help,                    Qt::Key_Help },
	{ 0x1000FF74,                      Qt::Key_Backtab },        // hardcoded HP backtab
	{ 0x1005FF10,                      Qt::Key_F11 },            // hardcoded Sun F36 (labeled F11)
	{ 0x1005FF11,                      Qt::Key_F12 },            // hardcoded Sun F37 (labeled F12)

// function keys

	{ GDK_KEY_F1,                      Qt::Key_F1 },
	{ GDK_KEY_F2,                      Qt::Key_F2 },
	{ GDK_KEY_F3,                      Qt::Key_F3 },
	{ GDK_KEY_F4,                      Qt::Key_F4 },
	{ GDK_KEY_F5,                      Qt::Key_F5 },
	{ GDK_KEY_F6,                      Qt::Key_F6 },
	{ GDK_KEY_F7,                      Qt::Key_F7 },
	{ GDK_KEY_F8,                      Qt::Key_F8 },
	{ GDK_KEY_F9,                      Qt::Key_F9 },
	{ GDK_KEY_F10,                     Qt::Key_F10 },
	{ GDK_KEY_F11,                     Qt::Key_F11 },
	{ GDK_KEY_F12,                     Qt::Key_F12 },
	{ GDK_KEY_F13,                     Qt::Key_F13 },
	{ GDK_KEY_F14,                     Qt::Key_F14 },
	{ GDK_KEY_F15,                     Qt::Key_F15 },
	{ GDK_KEY_F16,                     Qt::Key_F16 },
	{ GDK_KEY_F17,                     Qt::Key_F17 },
	{ GDK_KEY_F18,                     Qt::Key_F18 },
	{ GDK_KEY_F19,                     Qt::Key_F19 },
	{ GDK_KEY_F20,                     Qt::Key_F20 },

// numeric and function keypad keys

	{ GDK_KEY_KP_Space,                Qt::Key_Space },
	{ GDK_KEY_KP_Tab,                  Qt::Key_Tab },
	{ GDK_KEY_KP_Enter,                Qt::Key_Enter },
	{ GDK_KEY_KP_F1,                   Qt::Key_F1 },
	{ GDK_KEY_KP_F2,                   Qt::Key_F2 },
	{ GDK_KEY_KP_F3,                   Qt::Key_F3 },
	{ GDK_KEY_KP_F4,                   Qt::Key_F4 },

	{ GDK_KEY_KP_Home,                 Qt::Key_Home },
	{ GDK_KEY_KP_Left,                 Qt::Key_Left },
	{ GDK_KEY_KP_Up,                   Qt::Key_Up },
	{ GDK_KEY_KP_Right,                Qt::Key_Right },
	{ GDK_KEY_KP_Down,                 Qt::Key_Down },
	{ GDK_KEY_KP_Prior,                Qt::Key_PageUp },
	{ GDK_KEY_KP_Next,                 Qt::Key_PageDown },
	{ GDK_KEY_KP_End,                  Qt::Key_End },
	{ GDK_KEY_KP_Begin,                Qt::Key_Clear },
	{ GDK_KEY_KP_Insert,               Qt::Key_Insert },
	{ GDK_KEY_KP_Delete,               Qt::Key_Delete },
	{ GDK_KEY_KP_Equal,                Qt::Key_Equal },
	{ GDK_KEY_KP_Multiply,             Qt::Key_Asterisk },
	{ GDK_KEY_KP_Add,                  Qt::Key_Plus },
	{ GDK_KEY_KP_Separator,            Qt::Key_Comma },
	{ GDK_KEY_KP_Subtract,             Qt::Key_Minus },
	{ GDK_KEY_KP_Decimal,              Qt::Key_Period },
	{ GDK_KEY_KP_Divide,               Qt::Key_Slash },

	{ GDK_KEY_KP_0,                    Qt::Key_0 },
	{ GDK_KEY_KP_1,                    Qt::Key_1 },
	{ GDK_KEY_KP_2,                    Qt::Key_2 },
	{ GDK_KEY_KP_3,                    Qt::Key_3 },
	{ GDK_KEY_KP_4,                    Qt::Key_4 },
	{ GDK_KEY_KP_5,                    Qt::Key_5 },
	{ GDK_KEY_KP_6,                    Qt::Key_6 },
	{ GDK_KEY_KP_7,                    Qt::Key_7 },
	{ GDK_KEY_KP_8,                    Qt::Key_8 },
	{ GDK_KEY_KP_9,                    Qt::Key_9 },

//International input method support keys
//International & multi-key character composition

	{ GDK_KEY_ISO_Level3_Shift,        Qt::Key_AltGr },
	{ GDK_KEY_Multi_key,               Qt::Key_Multi_key },
	{ GDK_KEY_Codeinput,               Qt::Key_Codeinput },
	{ GDK_KEY_SingleCandidate,         Qt::Key_SingleCandidate },
	{ GDK_KEY_MultipleCandidate,       Qt::Key_MultipleCandidate },
	{ GDK_KEY_PreviousCandidate,       Qt::Key_PreviousCandidate },

// Misc Functions
	{ GDK_KEY_Mode_switch,             Qt::Key_Mode_switch },
	{ GDK_KEY_script_switch,           Qt::Key_Mode_switch },

// Japanese keyboard support
	{ GDK_KEY_Kanji,                   Qt::Key_Kanji },
	{ GDK_KEY_Muhenkan,                Qt::Key_Muhenkan },
	//{ GDK_KEY_Henkan_Mode,             Qt::Key_Henkan_Mode },
	{ GDK_KEY_Henkan_Mode,             Qt::Key_Henkan },
	{ GDK_KEY_Henkan,                  Qt::Key_Henkan },
	{ GDK_KEY_Romaji,                  Qt::Key_Romaji },
	{ GDK_KEY_Hiragana,                Qt::Key_Hiragana },
	{ GDK_KEY_Katakana,                Qt::Key_Katakana },
	{ GDK_KEY_Hiragana_Katakana,       Qt::Key_Hiragana_Katakana },
	{ GDK_KEY_Zenkaku,                 Qt::Key_Zenkaku },
	{ GDK_KEY_Hankaku,                 Qt::Key_Hankaku },
	{ GDK_KEY_Zenkaku_Hankaku,         Qt::Key_Zenkaku_Hankaku },
	{ GDK_KEY_Touroku,                 Qt::Key_Touroku },
	{ GDK_KEY_Massyo,                  Qt::Key_Massyo },
	{ GDK_KEY_Kana_Lock,               Qt::Key_Kana_Lock },
	{ GDK_KEY_Kana_Shift,              Qt::Key_Kana_Shift },
	{ GDK_KEY_Eisu_Shift,              Qt::Key_Eisu_Shift },
	{ GDK_KEY_Eisu_toggle,             Qt::Key_Eisu_toggle },
	//{ GDK_KEY_Kanji_Bangou,            Qt::Key_Kanji_Bangou },
	//{ GDK_KEY_Zen_Koho,                Qt::Key_Zen_Koho },
	//{ GDK_KEY_Mae_Koho,                Qt::Key_Mae_Koho },
	{ GDK_KEY_Kanji_Bangou,            Qt::Key_Codeinput },
	{ GDK_KEY_Zen_Koho,                Qt::Key_MultipleCandidate },
	{ GDK_KEY_Mae_Koho,                Qt::Key_PreviousCandidate },

// Korean keyboard support
	{ GDK_KEY_Hangul,                  Qt::Key_Hangul },
	{ GDK_KEY_Hangul_Start,            Qt::Key_Hangul_Start },
	{ GDK_KEY_Hangul_End,              Qt::Key_Hangul_End },
	{ GDK_KEY_Hangul_Hanja,            Qt::Key_Hangul_Hanja },
	{ GDK_KEY_Hangul_Jamo,             Qt::Key_Hangul_Jamo },
	{ GDK_KEY_Hangul_Romaja,           Qt::Key_Hangul_Romaja },
	//{ GDK_KEY_Hangul_Codeinput,        Qt::Key_Hangul_Codeinput },
	{ GDK_KEY_Hangul_Codeinput,        Qt::Key_Codeinput },
	{ GDK_KEY_Hangul_Jeonja,           Qt::Key_Hangul_Jeonja },
	{ GDK_KEY_Hangul_Banja,            Qt::Key_Hangul_Banja },
	{ GDK_KEY_Hangul_PreHanja,         Qt::Key_Hangul_PreHanja },
	{ GDK_KEY_Hangul_PostHanja,        Qt::Key_Hangul_PostHanja },
	//{ GDK_KEY_Hangul_SingleCandidate,Qt::Key_Hangul_SingleCandidate },
	//{ GDK_KEY_Hangul_MultipleCandidate,Qt::Key_Hangul_MultipleCandidate },
	//{ GDK_KEY_Hangul_PreviousCandidate,Qt::Key_Hangul_PreviousCandidate },
	{ GDK_KEY_Hangul_SingleCandidate,  Qt::Key_SingleCandidate },
	{ GDK_KEY_Hangul_MultipleCandidate,Qt::Key_MultipleCandidate },
	{ GDK_KEY_Hangul_PreviousCandidate,Qt::Key_PreviousCandidate },
	{ GDK_KEY_Hangul_Special,          Qt::Key_Hangul_Special },
	//{ GDK_KEY_Hangul_switch,         Qt::Key_Hangul_switch },
	{ GDK_KEY_Hangul_switch,           Qt::Key_Mode_switch },

// dead keys
	{ GDK_KEY_dead_grave,              Qt::Key_Dead_Grave },
	{ GDK_KEY_dead_acute,              Qt::Key_Dead_Acute },
	{ GDK_KEY_dead_circumflex,         Qt::Key_Dead_Circumflex },
	{ GDK_KEY_dead_tilde,              Qt::Key_Dead_Tilde },
	{ GDK_KEY_dead_macron,             Qt::Key_Dead_Macron },
	{ GDK_KEY_dead_breve,              Qt::Key_Dead_Breve },
	{ GDK_KEY_dead_abovedot,           Qt::Key_Dead_Abovedot },
	{ GDK_KEY_dead_diaeresis,          Qt::Key_Dead_Diaeresis },
	{ GDK_KEY_dead_abovering,          Qt::Key_Dead_Abovering },
	{ GDK_KEY_dead_doubleacute,        Qt::Key_Dead_Doubleacute },
	{ GDK_KEY_dead_caron,              Qt::Key_Dead_Caron },
	{ GDK_KEY_dead_cedilla,            Qt::Key_Dead_Cedilla },
	{ GDK_KEY_dead_ogonek,             Qt::Key_Dead_Ogonek },
	{ GDK_KEY_dead_iota,               Qt::Key_Dead_Iota },
	{ GDK_KEY_dead_voiced_sound,       Qt::Key_Dead_Voiced_Sound },
	{ GDK_KEY_dead_semivoiced_sound,   Qt::Key_Dead_Semivoiced_Sound },
	{ GDK_KEY_dead_belowdot,           Qt::Key_Dead_Belowdot },
	{ GDK_KEY_dead_hook,               Qt::Key_Dead_Hook },
	{ GDK_KEY_dead_horn,               Qt::Key_Dead_Horn },

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
		return keySym | Qt::UNICODE_ACCEL;

	for(i = 0; i < count; i++)
		if(QtKeyXSymMaps[i].XKeySym == keySym)
			return QtKeyXSymMaps[i].QtKey;
#endif
        return Qt::Key_unknown;
}

