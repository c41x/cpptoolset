#include "hotkey.hpp"

namespace granite { namespace system {
namespace hotkey {
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
}
}}
