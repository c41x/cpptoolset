#include "hotkey.hpp"

namespace granite { namespace system {
namespace hotkey {

#if defined(GE_PLATFORM_WINDOWS)

namespace {
struct hotkeyData {
	std::function<void()> fx;
};

int currentId = 1;
std::map<int, hotkeyData> binds;
}

bool init() {
	return true;
}

int add(keyId key, modId mods, std::function<void()> fx) {
	if (0 == RegisterHotKey(NULL, currentId, mods, key)) {
		logError(strs("could not register hotkey key: ", key, " mod: ", mods));
		return 0;
	}
	binds.insert(std::make_pair(currentId, hotkeyData {fx}));
	return currentId++;
}

bool remove(int id) {
	auto e = binds.find(id);
	if (e != binds.end()) {
		UnregisterHotKey(NULL, e->first);
		binds.erase(e);
		return true;
	}
	return false;
}

void process() {
	MSG msg = {0};
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
		if (msg.message == WM_HOTKEY) {
			auto e = binds.find(msg.wParam);
			if (e != binds.end()) {
				e->second.fx();
			}
		}
	}
}

void shutdown() {
	for (auto &e : binds) {
		UnregisterHotKey(NULL, e.first);
	}
}

#elif defined(GE_PLATFORM_LINUX)

namespace {
struct hotkeyData {
	std::function<void()> fx;
	unsigned int mods;
	unsigned int keycode;
};

int currentId = 1;
std::map<int, hotkeyData> binds;
bool keyBound = false;
int x11errorHandler(Display *, XErrorEvent *) { keyBound = false; return 0; }
Display* dpy = nullptr;
Window root;
}

bool init() {
	if (!dpy) {
		dpy = XOpenDisplay(0);
		root = DefaultRootWindow(dpy);
		return true;
	}
	return false;
}

int add(keyId key, modId mods, std::function<void()> fx) {
	unsigned int keycode = XKeysymToKeycode(dpy, key);

	XSetErrorHandler(x11errorHandler);
	keyBound = true;
	XSync(dpy, 0);
	XGrabKey(dpy, keycode, mods, root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mods | Mod2Mask, root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mods | Mod3Mask, root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mods | LockMask, root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mods | Mod2Mask | Mod3Mask, root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mods | Mod2Mask | LockMask, root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mods | Mod3Mask | LockMask, root, False, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mods | Mod2Mask | Mod3Mask | LockMask, root, False, GrabModeAsync, GrabModeAsync);
	XSync(dpy, 0);

	if (!keyBound) {
		logError(strs("could not register hotkey key: ", key, " mod: ", mods));
		XUngrabKey(dpy, keycode, mods, root);
		XUngrabKey(dpy, keycode, mods | Mod2Mask, root);
		XUngrabKey(dpy, keycode, mods | Mod3Mask, root);
		XUngrabKey(dpy, keycode, mods | LockMask, root);
		XUngrabKey(dpy, keycode, mods | Mod2Mask | Mod3Mask, root);
		XUngrabKey(dpy, keycode, mods | Mod2Mask | LockMask, root);
		XUngrabKey(dpy, keycode, mods | Mod3Mask | LockMask, root);
		XUngrabKey(dpy, keycode, mods | Mod2Mask | Mod3Mask | LockMask, root);
		return 0;
	}

	XSelectInput(dpy, root, KeyPressMask);
	XSetErrorHandler(NULL);
	binds.insert(std::make_pair(currentId, hotkeyData {fx, mods, keycode}));
	return currentId++;
}

bool remove(int id) {
	auto e = binds.find(id);
	if (e != binds.end()) {
		XUngrabKey(dpy, e->second.keycode, e->second.mods, root);
		XUngrabKey(dpy, e->second.keycode, e->second.mods | Mod2Mask, root);
		XUngrabKey(dpy, e->second.keycode, e->second.mods | Mod3Mask, root);
		XUngrabKey(dpy, e->second.keycode, e->second.mods | LockMask, root);
		XUngrabKey(dpy, e->second.keycode, e->second.mods | Mod2Mask | Mod3Mask, root);
		XUngrabKey(dpy, e->second.keycode, e->second.mods | Mod2Mask | LockMask, root);
		XUngrabKey(dpy, e->second.keycode, e->second.mods | Mod3Mask | LockMask, root);
		XUngrabKey(dpy, e->second.keycode, e->second.mods | Mod2Mask | Mod3Mask | LockMask, root);
		binds.erase(e);
		return true;
	}
	return false;
}

void process() {
	if (XPending(dpy) > 0) {
		XEvent ev;
		XNextEvent(dpy, &ev);
		auto mask = ev.xkey.state;

		// ignore num lock, caps lock and scroll lock
		mask &= ~Mod2Mask;
		mask &= ~Mod3Mask;
		mask &= ~LockMask;

		if (ev.type == KeyPress) {
			auto e = std::find_if(binds.begin(), binds.end(), [&ev, mask](const auto &a) -> bool {
					return mask == a.second.mods && ev.xkey.keycode == a.second.keycode;
				});
			if (e != binds.end()) {
				e->second.fx();
			}
		}
	}
}

void shutdown() {
	for (auto &e : binds) {
		XUngrabKey(dpy, e.second.keycode, AnyModifier, root);
	}
	XCloseDisplay(dpy);
}

#else
#error "not implemented"
#endif

}
}}

// TODO: visual c++ test
