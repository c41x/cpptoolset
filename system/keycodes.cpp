#include "keycodes.hpp"

namespace granite { namespace system {
#ifdef GE_PLATFORM_WINDOWS
namespace detail {
std::map<string, keyId> s2k;

void initMap() {
	if (s2k.size() > 0)
		return;

	/*
	 * Virtual Keys, Standard Set
	 */

	s2k["LButton"] = VK_LBUTTON;
	s2k["RButton"] = VK_RBUTTON;
	s2k["Cancel"] = VK_CANCEL;
	s2k["MButton"] = VK_MBUTTON;

	#if defined(VK_XBUTTON1)
	s2k["XButton1"] = VK_XBUTTON1;
	s2k["XButton2"] = VK_XBUTTON2;
	#endif

	/*
	 * 0x07 : unassigned
	 */

	s2k["BackSpace"] = VK_BACK;
	s2k["Tab"] = VK_TAB;

	/*
	 * 0x0A - 0x0B : reserved
	 */

	s2k["Clear"] = VK_CLEAR;
	s2k["Return"] = VK_RETURN;

	s2k["Shift"] = VK_SHIFT;
	s2k["Control"] = VK_CONTROL;
	s2k["Menu"] = VK_MENU;
	s2k["Pause"] = VK_PAUSE;
	s2k["Capital"] = VK_CAPITAL;

	s2k["Kana"] = VK_KANA;
	s2k["Hangeul"] = VK_HANGEUL; /* old name - should be here for compatibility */
	s2k["Hangul"] = VK_HANGUL;
	s2k["Junja"] = VK_JUNJA;
	s2k["Final"] = VK_FINAL;
	s2k["Hanja"] = VK_HANJA;
	s2k["Kanji"] = VK_KANJI;

	s2k["Escape"] = VK_ESCAPE;

	s2k["Convert"] = VK_CONVERT;
	s2k["Nonconvert"] = VK_NONCONVERT;
	s2k["Accept"] = VK_ACCEPT;
	s2k["Modechange"] = VK_MODECHANGE;

	s2k["Space"] = VK_SPACE;
	s2k["Prior"] = VK_PRIOR;
	s2k["Next"] = VK_NEXT;
	s2k["End"] = VK_END;
	s2k["Home"] = VK_HOME;
	s2k["Left"] = VK_LEFT;
	s2k["Up"] = VK_UP;
	s2k["Right"] = VK_RIGHT;
	s2k["Down"] = VK_DOWN;
	s2k["Select"] = VK_SELECT;
	s2k["Print"] = VK_PRINT;
	s2k["Execute"] = VK_EXECUTE;
	s2k["Snapshot"] = VK_SNAPSHOT;
	s2k["Insert"] = VK_INSERT;
	s2k["Delete"] = VK_DELETE;
	s2k["Help"] = VK_HELP;

	/*
	 * key vk_0 - key vk_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
	 * 0x40 : unassigned
	 * key vk_A - key vk_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
	 */

	for (char c = '0'; c <= '9'; ++c)
		s2k[toStr(c)] = c;

	for (char c = 'A'; c <= 'Z'; ++c)
		s2k[toStr(c)] = c;

	s2k["LWin"] = VK_LWIN;
	s2k["RWin"] = VK_RWIN;
	s2k["Apps"] = VK_APPS;

	/*
	 * 0x5E : reserved
	 */

	s2k["Sleep"] = VK_SLEEP;

	s2k["KP_0"] = VK_NUMPAD0;
	s2k["KP_1"] = VK_NUMPAD1;
	s2k["KP_2"] = VK_NUMPAD2;
	s2k["KP_3"] = VK_NUMPAD3;
	s2k["KP_4"] = VK_NUMPAD4;
	s2k["KP_5"] = VK_NUMPAD5;
	s2k["KP_6"] = VK_NUMPAD6;
	s2k["KP_7"] = VK_NUMPAD7;
	s2k["KP_8"] = VK_NUMPAD8;
	s2k["KP_9"] = VK_NUMPAD9;
	s2k["Multiply"] = VK_MULTIPLY;
	s2k["Add"] = VK_ADD;
	s2k["Separator"] = VK_SEPARATOR;
	s2k["Subtract"] = VK_SUBTRACT;
	s2k["Decimal"] = VK_DECIMAL;
	s2k["Divide"] = VK_DIVIDE;
	s2k["F1"] = VK_F1;
	s2k["F2"] = VK_F2;
	s2k["F3"] = VK_F3;
	s2k["F4"] = VK_F4;
	s2k["F5"] = VK_F5;
	s2k["F6"] = VK_F6;
	s2k["F7"] = VK_F7;
	s2k["F8"] = VK_F8;
	s2k["F9"] = VK_F9;
	s2k["F10"] = VK_F10;
	s2k["F11"] = VK_F11;
	s2k["F12"] = VK_F12;
	s2k["F13"] = VK_F13;
	s2k["F14"] = VK_F14;
	s2k["F15"] = VK_F15;
	s2k["F16"] = VK_F16;
	s2k["F17"] = VK_F17;
	s2k["F18"] = VK_F18;
	s2k["F19"] = VK_F19;
	s2k["F20"] = VK_F20;
	s2k["F21"] = VK_F21;
	s2k["F22"] = VK_F22;
	s2k["F23"] = VK_F23;
	s2k["F24"] = VK_F24;

	/*
	 * 0x88 - 0x8F : unassigned
	 */

	s2k["Num_Lock"] = VK_NUMLOCK;
	s2k["Scroll_Lock"] = VK_SCROLL;

	/*
	 * 0x97 - 0x9F : unassigned
	 */

	/*
	 * key vk_L* & key vk_R* - left and right Alt, Ctrl and Shift virtual keys.
	 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
	 * No other API or message will distinguish left and right keys in this way.
	 */

	s2k["Shift_L"] = VK_LSHIFT;
	s2k["Shift_R"] = VK_RSHIFT;
	s2k["Control_L"] = VK_LCONTROL;
	s2k["Control_R"] = VK_RCONTROL;
	s2k["Hyper_L"] = VK_LMENU;
	s2k["Hyper_R"] = VK_RMENU;

	s2k["BrowserBack"] = VK_BROWSER_BACK;
	s2k["BrowserForward"] = VK_BROWSER_FORWARD;
	s2k["BrowserRefresh"] = VK_BROWSER_REFRESH;
	s2k["BrowserStop"] = VK_BROWSER_STOP;
	s2k["BrowserSearch"] = VK_BROWSER_SEARCH;
	s2k["BrowserFavorites"] = VK_BROWSER_FAVORITES;
	s2k["BrowserHome"] = VK_BROWSER_HOME;

	s2k["VolumeMute"] = VK_VOLUME_MUTE;
	s2k["VolumeDown"] = VK_VOLUME_DOWN;
	s2k["VolumeUp"] = VK_VOLUME_UP;
	s2k["MediaNextTrack"] = VK_MEDIA_NEXT_TRACK;
	s2k["MediaPrevTrack"] = VK_MEDIA_PREV_TRACK;
	s2k["MediaStop"] = VK_MEDIA_STOP;
	s2k["MediaPlayPause"] = VK_MEDIA_PLAY_PAUSE;
	s2k["LaunchMail"] = VK_LAUNCH_MAIL;
	s2k["LaunchMediaSelect"] = VK_LAUNCH_MEDIA_SELECT;
	s2k["LaunchApp1"] = VK_LAUNCH_APP1;
	s2k["LaunchApp2"] = VK_LAUNCH_APP2;

	/*
	 * 0xB8 - 0xB9 : reserved
	 */

	s2k["OEM1"] = VK_OEM_1; // ';:' for US
	s2k["OEMPlus"] = VK_OEM_PLUS; // '+' any country
	s2k["OEMComma"] = VK_OEM_COMMA; // ',' any country
	s2k["OEMMinus"] = VK_OEM_MINUS; // '-' any country
	s2k["OEMPeriod"] = VK_OEM_PERIOD; // '.' any country
	s2k["OEM2"] = VK_OEM_2; // '/?' for US
	s2k["OEM3"] = VK_OEM_3; // '`~' for US

	/*
	 * 0xC1 - 0xD7 : reserved
	 */

	/*
	 * 0xD8 - 0xDA : unassigned
	 */

	s2k["OEM4"] = VK_OEM_4; //  '[{' for US
	s2k["OEM5"] = VK_OEM_5; //  '\|' for US
	s2k["OEM6"] = VK_OEM_6; //  ']}' for US
	s2k["OEM7"] = VK_OEM_7; //  ''"' for US
	s2k["OEM8"] = VK_OEM_8;

	/*
	 * 0xE0 : reserved
	 */

	/*
	 * Various extended or enhanced keyboards
	 */

	s2k["OEMAx"] = VK_OEM_AX; //  'AX' key on Japanese AX kbd
	s2k["OEM102"] = VK_OEM_102; //  "<>" or "\|" on RT 102-key kbd.
	s2k["ICOHelp"] = VK_ICO_HELP; //  Help key on ICO
	s2k["ICO00"] = VK_ICO_00; //  00 key on ICO
	s2k["ProcessKey"] = VK_PROCESSKEY;
	s2k["ICOClear"] = VK_ICO_CLEAR;
	s2k["Packet"] = VK_PACKET;

	/*
	 * 0xE8 : unassigned
	 */

	/*
	 * Nokia/Ericsson definitions
	 */

	s2k["OEMReset"] = VK_OEM_RESET;
	s2k["OEMJump"] = VK_OEM_JUMP;
	s2k["OEMPa1"] = VK_OEM_PA1;
	s2k["OEMPa2"] = VK_OEM_PA2;
	s2k["OEMPa3"] = VK_OEM_PA3;
	s2k["OEMWsctrl"] = VK_OEM_WSCTRL;
	s2k["OEMCusel"] = VK_OEM_CUSEL;
	s2k["OEMAttn"] = VK_OEM_ATTN;
	s2k["OEMFinish"] = VK_OEM_FINISH;
	s2k["OEMCopy"] = VK_OEM_COPY;
	s2k["OEMAuto"] = VK_OEM_AUTO;
	s2k["OEMEnlw"] = VK_OEM_ENLW;
	s2k["OEMBacktab"] = VK_OEM_BACKTAB;

	s2k["Attn"] = VK_ATTN;
	s2k["Crsel"] = VK_CRSEL;
	s2k["Exsel"] = VK_EXSEL;
	s2k["Ereof"] = VK_EREOF;
	s2k["Play"] = VK_PLAY;
	s2k["Zoom"] = VK_ZOOM;
	s2k["Noname"] = VK_NONAME;
	s2k["Pa1"] = VK_PA1;
	s2k["OemClear"] = VK_OEM_CLEAR;

	/*
	 * 0xFF : reserved
	 */
}
}
#endif


//- Windows
#if defined(GE_PLATFORM_WINDOWS)
modId getModifier(const granite::base::string &mod) {
	modId r = 0;
	for (const auto &c : mod) {
		if (c == 'M') r |= MOD_ALT;
		else if (c == 'C') r |= MOD_CONTROL;
		else if (c == 'S') r |= MOD_SHIFT;
		else if (c == 'W') r |= MOD_WIN;
	}
	return r;
}

keyId getKey(const granite::base::string &key) {
	detail::initMap();
	auto e = detail::s2k.find(key);
	if (e != detail::s2k.end())
		return e->second;
	return 0;
}

//- LINUX (X11)
#elif defined(GE_PLATFORM_LINUX)
modId getModifier(const granite::base::string &mod) {
	modId r = 0;
	for (const auto &c : mod) {
		if (c == 'M') r |= Mod1Mask;
		else if (c == 'C') r |= ControlMask;
		else if (c == 'S') r |= ShiftMask;
		else if (c == 'W') r |= Mod4Mask;
	}
	return r;
}

keyId getKey(const granite::base::string &key) {
	return XStringToKeysym(key.c_str());
}
#endif

}}
