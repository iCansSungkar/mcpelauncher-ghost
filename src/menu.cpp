#include "menu.h"

#include "config.h"
#include "ghost.h"
#include "input.h"
#include "launcher_api.h"
#include "log.h"

#include <dlfcn.h>

#include <cstdio>

namespace menu {

namespace {

MenuShowWindowFn g_showWindow = nullptr;
MenuCloseWindowFn g_closeWindow = nullptr;

constexpr const char* kRebindWindow = "I'm Ghost - Set hotkey";

// Text of the hotkey entry; rewritten in place so the pointer stays valid.
char g_hotkeyLabel[80] = "Set hotkey";

void refreshHotkeyLabel() {
    std::snprintf(g_hotkeyLabel, sizeof(g_hotkeyLabel), "Set hotkey (now: %s)",
                  cfg::describeHotkey(cfg::get().hotkey).c_str());
}

// ---- Ghost mode toggle ----------------------------------------------------
bool ghostSelected(void*) { return ghost::isActive(); }
void ghostClick(void*) { ghost::requestToggle(); }

// ---- Exit mode toggle -----------------------------------------------------
bool exitSelected(void*) {
    return cfg::get().exitMode.load() == static_cast<int>(cfg::ExitMode::Origin);
}
void exitClick(void*) {
    cfg::Config& c = cfg::get();
    c.exitMode = (c.exitMode.load() == static_cast<int>(cfg::ExitMode::Origin))
                     ? static_cast<int>(cfg::ExitMode::Stay)
                     : static_cast<int>(cfg::ExitMode::Origin);
    cfg::save();
}

// ---- Hotkey rebind --------------------------------------------------------
void onRebindDone() {
    refreshHotkeyLabel();
    if (g_closeWindow) g_closeWindow(kRebindWindow);
}

void onRebindWindowClosed(void*) {
    input::cancelRebind(); // no-op if the key was already captured
}

void onRebindCancel(void*) {
    input::cancelRebind();
    if (g_closeWindow) g_closeWindow(kRebindWindow);
}

bool alwaysFalse(void*) { return false; }

void rebindClick(void*) {
    if (!g_showWindow) return;

    static control controls[2];
    controls[0] = control{};
    controls[0].type = 3; // static text
    controls[0].data.text.label =
        "Hold any modifiers (Alt/Ctrl/Shift), then press the key you want.";
    controls[0].data.text.size = 0;

    controls[1] = control{};
    controls[1].type = 0; // button
    controls[1].data.button.label = "Cancel";
    controls[1].data.button.user = nullptr;
    controls[1].data.button.onClick = &onRebindCancel;

    input::beginRebind(&onRebindDone);
    g_showWindow(kRebindWindow, /*isModal=*/0, nullptr, &onRebindWindowClosed, 2, controls);
}

// Entry tables must outlive the call to mcpelauncher_addmenu.
MenuEntryABI g_sub[3];
MenuEntryABI g_root[1];

} // namespace

void install() {
    void* lib = dlopen("libmcpelauncher_menu.so", RTLD_NOW);
    if (!lib) {
        LOGW("libmcpelauncher_menu.so not available: no menu entry (hotkey still works)");
        return;
    }

    auto addMenu = reinterpret_cast<MenuAddFn>(dlsym(lib, "mcpelauncher_addmenu"));
    g_showWindow = reinterpret_cast<MenuShowWindowFn>(dlsym(lib, "mcpelauncher_show_window"));
    g_closeWindow = reinterpret_cast<MenuCloseWindowFn>(dlsym(lib, "mcpelauncher_close_window"));
    if (!addMenu) {
        LOGW("mcpelauncher_addmenu missing: no menu entry");
        return;
    }

    refreshHotkeyLabel();

    g_sub[0] = MenuEntryABI{"Ghost mode", nullptr, &ghostSelected, &ghostClick, 0, nullptr};
    g_sub[1] = MenuEntryABI{"Return to start position on exit", nullptr, &exitSelected, &exitClick,
                            0, nullptr};
    g_sub[2] = MenuEntryABI{g_hotkeyLabel, nullptr, &alwaysFalse, &rebindClick, 0, nullptr};

    g_root[0] = MenuEntryABI{"I'm Ghost", nullptr, nullptr, nullptr, 3, g_sub};

    addMenu(1, g_root);
    LOGI("menu entry added");
}

} // namespace menu
