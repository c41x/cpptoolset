/*
 * granite engine 1.0 | 2006-2016 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: keycodes
 * created: 22-02-2016
 *
 * description: portable key codes (X11 / WinAPI)
 *
 * changelog:
 * - 22-02-2016: file created
 */

#pragma once
#include "includes.hpp"

//- Windows
#if defined(GE_PLATFORM_WINDOWS)
typedef UINT keyId;
typedef UINT modId;

inline modId getModifier(const granite::base::string &mod) {
	if (mod == "alt") return MOD_ALT;
	else if (mod == "control") return MOD_CONTROL;
	else if (mod == "shift") return MOD_SHIFT;
	else if (mod == "win") return MOD_WIN;
	return 0;
}

inline keyId getKey(const granite::base::string &key) {
	// TODO: -
}

#define modAlt MOD_ALT
#define modControl MOD_CONTROL
#define modShift MOD_SHIFT
#define modWin MOD_WIN

/*
 * Virtual Keys, Standard Set
 */

#define keyLButton VK_LBUTTON
#define keyRButton VK_RBUTTON
#define keyCancel VK_CANCEL
#define keyMButton VK_MBUTTON

#if defined(VK_XBUTTON1)
	#define keyXButton1 VK_XBUTTON1
	#define keyXButton2 VK_XBUTTON2
#endif

/*
 * 0x07 : unassigned
 */

#define keyBack VK_BACK
#define keyTab VK_TAB

/*
 * 0x0A - 0x0B : reserved
 */

#define keyClear VK_CLEAR
#define keyReturn VK_RETURN

#define keyShift VK_SHIFT
#define keyControl VK_CONTROL
#define keyMenu VK_MENU
#define keyPause VK_PAUSE
#define keyCapital VK_CAPITAL

#define keyKana VK_KANA
#define keyHangeul VK_HANGEUL /* old name - should be here for compatibility */
#define keyHangul VK_HANGUL
#define keyJunja VK_JUNJA
#define keyFinal VK_FINAL
#define keyHanja VK_HANJA
#define keyKanji VK_KANJI

#define keyEscape VK_ESCAPE

#define keyConvert VK_CONVERT
#define keyNonconvert VK_NONCONVERT
#define keyAccept VK_ACCEPT
#define keyModechange VK_MODECHANGE

#define keySpace VK_SPACE
#define keyPrior VK_PRIOR
#define keyNext VK_NEXT
#define keyEnd VK_END
#define keyHome VK_HOME
#define keyLeft VK_LEFT
#define keyUp VK_UP
#define keyRight VK_RIGHT
#define keyDown VK_DOWN
#define keySelect VK_SELECT
#define keyPrint VK_PRINT
#define keyExecute VK_EXECUTE
#define keySnapshot VK_SNAPSHOT
#define keyInsert VK_INSERT
#define keyDelete VK_DELETE
#define keyHelp VK_HELP

/*
 * key vk_0 - key vk_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x40 : unassigned
 * key vk_A - key vk_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */

#define keyLWin VK_LWIN
#define keyRWin VK_RWIN
#define keyApps VK_APPS

/*
 * 0x5E : reserved
 */

#define keySleep VK_SLEEP

#define keyNumpad0 VK_NUMPAD0
#define keyNumpad1 VK_NUMPAD1
#define keyNumpad2 VK_NUMPAD2
#define keyNumpad3 VK_NUMPAD3
#define keyNumpad4 VK_NUMPAD4
#define keyNumpad5 VK_NUMPAD5
#define keyNumpad6 VK_NUMPAD6
#define keyNumpad7 VK_NUMPAD7
#define keyNumpad8 VK_NUMPAD8
#define keyNumpad9 VK_NUMPAD9
#define keyMultiply VK_MULTIPLY
#define keyAdd VK_ADD
#define keySeparator VK_SEPARATOR
#define keySubtract VK_SUBTRACT
#define keyDecimal VK_DECIMAL
#define keyDivide VK_DIVIDE
#define keyF1 VK_F1
#define keyF2 VK_F2
#define keyF3 VK_F3
#define keyF4 VK_F4
#define keyF5 VK_F5
#define keyF6 VK_F6
#define keyF7 VK_F7
#define keyF8 VK_F8
#define keyF9 VK_F9
#define keyF10 VK_F10
#define keyF11 VK_F11
#define keyF12 VK_F12
#define keyF13 VK_F13
#define keyF14 VK_F14
#define keyF15 VK_F15
#define keyF16 VK_F16
#define keyF17 VK_F17
#define keyF18 VK_F18
#define keyF19 VK_F19
#define keyF20 VK_F20
#define keyF21 VK_F21
#define keyF22 VK_F22
#define keyF23 VK_F23
#define keyF24 VK_F24

/*
 * 0x88 - 0x8F : unassigned
 */

#define keyNumlock VK_NUMLOCK
#define keyScroll VK_SCROLL

/*
 * 0x97 - 0x9F : unassigned
 */

/*
 * key vk_L* & key vk_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */

#define keyLShift VK_LSHIFT
#define keyRShift VK_RSHIFT
#define keyLControl VK_LCONTROL
#define keyRControl VK_RCONTROL
#define keyLMenu VK_LMENU
#define keyRMenu VK_RMENU

#define keyBrowserBack VK_BROWSER_BACK
#define keyBrowserForward VK_BROWSER_FORWARD
#define keyBrowserRefresh VK_BROWSER_REFRESH
#define keyBrowserStop VK_BROWSER_STOP
#define keyBrowserSearch VK_BROWSER_SEARCH
#define keyBrowserFavorites VK_BROWSER_FAVORITES
#define keyBrowserHome VK_BROWSER_HOME

#define keyVolumeMute VK_VOLUME_MUTE
#define keyVolumeDown VK_VOLUME_DOWN
#define keyVolumeUp VK_VOLUME_UP
#define keyMediaNextTrack VK_MEDIA_NEXT_TRACK
#define keyMediaPrevTrack VK_MEDIA_PREV_TRACK
#define keyMediaStop VK_MEDIA_STOP
#define keyMediaPlayPause VK_MEDIA_PLAY_PAUSE
#define keyLaunchMail VK_LAUNCH_MAIL
#define keyLaunchMediaSelect VK_LAUNCH_MEDIA_SELECT
#define keyLaunchApp1 VK_LAUNCH_APP1
#define keyLaunchApp2 VK_LAUNCH_APP2

/*
 * 0xB8 - 0xB9 : reserved
 */

#define keyOEM1 VK_OEM_1 // ';:' for US
#define keyOEMPlus VK_OEM_PLUS // '+' any country
#define keyOEMComma VK_OEM_COMMA // ',' any country
#define keyOEMMinus VK_OEM_MINUS // '-' any country
#define keyOEMPeriod VK_OEM_PERIOD // '.' any country
#define keyOEM2 VK_OEM_2 // '/?' for US
#define keyOEM3 VK_OEM_3 // '`~' for US

/*
 * 0xC1 - 0xD7 : reserved
 */

/*
 * 0xD8 - 0xDA : unassigned
 */

#define keyOEM4 VK_OEM_4 //  '[{' for US
#define keyOEM5 VK_OEM_5 //  '\|' for US
#define keyOEM6 VK_OEM_6 //  ']}' for US
#define keyOEM7 VK_OEM_7 //  ''"' for US
#define keyOEM8 VK_OEM_8

/*
 * 0xE0 : reserved
 */

/*
 * Various extended or enhanced keyboards
 */

#define keyOEMAx VK_OEM_AX //  'AX' key on Japanese AX kbd
#define keyOEM102 VK_OEM_102 //  "<>" or "\|" on RT 102-key kbd.
#define keyICOHelp VK_ICO_HELP //  Help key on ICO
#define keyICO00 VK_ICO_00 //  00 key on ICO
#define keyProcessKey VK_PROCESSKEY
#define keyICOClear VK_ICO_CLEAR
#define keyPacket VK_PACKET

/*
 * 0xE8 : unassigned
 */

/*
 * Nokia/Ericsson definitions
 */

#define keyOEMReset VK_OEM_RESET
#define keyOEMJump VK_OEM_JUMP
#define keyOEMPa1 VK_OEM_PA1
#define keyOEMPa2 VK_OEM_PA2
#define keyOEMPa3 VK_OEM_PA3
#define keyOEMWsctrl VK_OEM_WSCTRL
#define keyOEMCusel VK_OEM_CUSEL
#define keyOEMAttn VK_OEM_ATTN
#define keyOEMFinish VK_OEM_FINISH
#define keyOEMCopy VK_OEM_COPY
#define keyOEMAuto VK_OEM_AUTO
#define keyOEMEnlw VK_OEM_ENLW
#define keyOEMBacktab VK_OEM_BACKTAB

#define keyAttn VK_ATTN
#define keyCrsel VK_CRSEL
#define keyExsel VK_EXSEL
#define keyEreof VK_EREOF
#define keyPlay VK_PLAY
#define keyZoom VK_ZOOM
#define keyNoname VK_NONAME
#define keyPa1 VK_PA1
#define keyOem_CLEAR VK_OEM_CLEAR

/*
 * 0xFF : reserved
 */

//- LINUX (X11)
#elif defined(GE_PLATFORM_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef unsigned int keyId;
typedef unsigned int modId;

#define modAlt Mod1Mask
#define modControl ControlMask
#define modShift ShiftMask
#define modWin Mod4Mask

inline modId getModifier(const granite::base::string &mod) {
	if (mod == "alt") return Mod1Mask;
	else if (mod == "control") return ControlMask;
	else if (mod == "shift") return ShiftMask;
	else if (mod == "win") return Mod4Mask;
	return 0;
}

inline keyId getKey(const granite::base::string &key) {
	return (keyId)XStringToKeysym(key.c_str());
}

#endif
