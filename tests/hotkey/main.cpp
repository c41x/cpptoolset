#include <system/system.hpp>
/*#include <X11/Xlib.h>
  #include <X11/Xutil.h>*/

using namespace granite;
using namespace granite::base;
using namespace granite::system;

//bool keyBound = false;

//int x11errorHandler(Display *, XErrorEvent *) { keyBound = false; return 0; }

int main(int argc, char**argv) {
	bool run = true;
	hotkey::init();
	hotkey::add('B', modAlt, []() {
			std::cout << "hotkey ALT + B pressed!!!" << std::endl;
		});
	hotkey::add(keyMediaPlayPause, modControl, [&run]() {
			std::cout << "hotkey CTRL + B pressed!!!" << std::endl;
			run = false;
		});
	while (run) {
		hotkey::process();
	}
	hotkey::shutdown();


/*
	Display* dpy = XOpenDisplay(0);
	Window root = DefaultRootWindow(dpy);
	XEvent ev;

	unsigned int modifiers = AnyModifier; // num lock, caps lock are modifiers too
	int keycode = XKeysymToKeycode(dpy, XK_K);
	Window grab_window = root;
	Bool owner_events = False;
	int pointer_mode = GrabModeAsync;
	int keyboard_mode = GrabModeAsync;

	XSetErrorHandler(x11errorHandler);
	keyBound = true;
	XSync(dpy, 0);
	XGrabKey(dpy, keycode, modifiers, grab_window, owner_events, pointer_mode, keyboard_mode);
	XSync(dpy, 0);

	if (!keyBound) {
		std::cout << "could not register hotkey" << std::endl;
		XUngrabKey(dpy, keycode, modifiers, grab_window);
		XCloseDisplay(dpy);
		return 0;
	}
	XSelectInput(dpy, root, KeyPressMask);
	XSetErrorHandler(NULL);

	bool run = true;
	while (run) {
		XNextEvent(dpy, &ev);
		auto mask = ev.xkey.state;

		// ignore num lock, caps lock and scroll lock
		mask &= ~Mod2Mask;
		mask &= ~Mod3Mask;
		mask &= ~LockMask;

		switch(ev.type) {
			case KeyPress: {
				if (ev.xkey.state & ShiftMask != 0) {
					std::cout << "Hot key pressed!" << std::endl;
					XUngrabKey(dpy, keycode, modifiers, grab_window);
					run = false;
				}
			}
		}
	}

	XCloseDisplay(dpy);*/
	return 0;
}
