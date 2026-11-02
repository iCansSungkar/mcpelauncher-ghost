#include "input.h"

#include "config.h"
#include "ghost.h"
#include "launcher_api.h"
#include "log.h"

#include <dlfcn.h>

#include <atomic>

namespace input {

namespace {

GameWindowGetPrimaryFn g_getPrimary = nullptr;
GameWindowAddKeyboardFn g_addKeyboard = nullptr;
GameWindowAddCreationFn g_addCreation = nullptr;

bool g_registered = false;

// Modifier state, tracked from the key events themselves.
bool g_altDown = false;
bool g_ctrlDown = false;
bool g_shiftDown = false;

// Consume the release of the hotkey key too, so the game never sees half a key press.
bool g_swallowRelease = false;
int g_swallowKey = -1;

std::atomic<bool> g_rebinding{false};
void (*g_rebindDone)() = nullptr;

bool isAlt(const cfg::Config& c, int k) { return k == c.keyAltL || k == c.keyAltR; }
bool isCtrl(const cfg::Config& c, int k) { return k == c.keyCtrlL || k == c.keyCtrlR; }
bool isShift(const cfg::Config& c, int k) { return k == c.keyShiftL || k == c.keyShiftR; }

// Return true to consume the event (the game will not see it).
bool onKey(void* /*user*/, int keyCode, int action) {
    cfg::Config& c = cfg::get();

    if (c.debugKeys) LOGI("key event: code=%d action=%d", keyCode, action);

    const bool pressed = (action == c.actionPress);
    const bool released = (action == c.actionRelease);
    const bool repeated = (action == c.actionRepeat);

    // Keep modifier state up to date; never swallow modifier events.
    if (isAlt(c, keyCode)) {
        g_altDown = !released;
        return false;
    }
    if (isCtrl(c, keyCode)) {
        g_ctrlDown = !released;
        return false;
    }
    if (isShift(c, keyCode)) {
        g_shiftDown = !released;
        return false;
    }

    if (released && g_swallowRelease && keyCode == g_swallowKey) {
        g_swallowRelease = false;
        return true;
    }

    // Rebind mode: the next real key press becomes the hotkey.
    if (g_rebinding.load() && pressed) {
        c.hotkey.key = keyCode;
        c.hotkey.alt = g_altDown;
        c.hotkey.ctrl = g_ctrlDown;
        c.hotkey.shift = g_shiftDown;
        cfg::save();
        g_rebinding = false;
        g_swallowRelease = true;
        g_swallowKey = keyCode;
        LOGI("hotkey set to %s", cfg::describeHotkey(c.hotkey).c_str());
        if (g_rebindDone) g_rebindDone();
        return true;
    }

    const cfg::Hotkey& hk = c.hotkey;
    const bool comboMatches = keyCode == hk.key && g_altDown == hk.alt && g_ctrlDown == hk.ctrl &&
                              g_shiftDown == hk.shift;

    if (comboMatches && (pressed || repeated)) {
        if (pressed) ghost::requestToggle(); // ignore auto-repeat
        g_swallowRelease = true;
        g_swallowKey = keyCode;
        return true;
    }

    return false;
}

void registerOnWindow() {
    if (g_registered) return;
    GameWindowHandle* window = g_getPrimary ? g_getPrimary() : nullptr;
    if (!window) {
        LOGW("primary game window not available yet");
        return;
    }
    g_addKeyboard(window, nullptr, &onKey);
    g_registered = true;
    LOGI("keyboard callback registered, hotkey = %s",
         cfg::describeHotkey(cfg::get().hotkey).c_str());
}

void onWindowCreated(void* /*user*/) {
    registerOnWindow();
}

} // namespace

void install() {
    void* lib = dlopen("libmcpelauncher_gamewindow.so", RTLD_NOW);
    if (!lib) {
        LOGE("cannot load libmcpelauncher_gamewindow.so: hotkey disabled");
        return;
    }

    g_getPrimary = reinterpret_cast<GameWindowGetPrimaryFn>(
        dlsym(lib, "game_window_get_primary_window"));
    g_addKeyboard = reinterpret_cast<GameWindowAddKeyboardFn>(
        dlsym(lib, "game_window_add_keyboard_callback"));
    g_addCreation = reinterpret_cast<GameWindowAddCreationFn>(
        dlsym(lib, "game_window_add_window_creation_callback"));

    if (!g_getPrimary || !g_addKeyboard) {
        LOGE("game window API symbols missing: hotkey disabled");
        return;
    }

    registerOnWindow();
    // If the window did not exist yet, get told when it does.
    if (!g_registered && g_addCreation) g_addCreation(nullptr, &onWindowCreated);
}

void beginRebind(void (*onDone)()) {
    g_rebindDone = onDone;
    g_rebinding = true;
}

void cancelRebind() {
    g_rebinding = false;
}

bool isRebinding() {
    return g_rebinding.load();
}

} // namespace input
