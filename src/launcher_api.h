#pragma once

// Declarations for the MCPELauncher APIs used by this mod.
// Sources:
//   https://minecraft-linux.github.io/extra/advanced/modding-api/index.html
//   https://minecraft-linux.github.io/extra/advanced/game-window-api/index.html
//   https://minecraft-linux.github.io/extra/advanced/menu-api/index.html

#include <cstddef>

// ---- Modding API (libmcpelauncher_mod.so) -------------------------------
extern "C" {
__attribute__((weak)) void* mcpelauncher_hook(void* sym, void* hook, void** orig);
}

// ---- Game Window API (libmcpelauncher_gamewindow.so) --------------------
struct GameWindowHandle; // opaque

using GameWindowGetPrimaryFn = GameWindowHandle* (*)();
using GameWindowKeyboardCb = bool (*)(void* user, int keyCode, int action);
using GameWindowAddKeyboardFn = void (*)(GameWindowHandle*, void* user, GameWindowKeyboardCb);
using GameWindowAddCreationFn = void (*)(void* user, void (*onCreated)(void* user));

// ---- Menu API (libmcpelauncher_menu.so) ---------------------------------
struct MenuEntryABI {
    const char* name;
    void* user;
    bool (*selected)(void* user);
    void (*click)(void* user);
    size_t length;
    MenuEntryABI* subentries;
};

struct control {
    int type; // 0 button, 1 slider int, 2 slider float, 3 text, 4 text input
    union {
        struct {
            const char* label;
            void* user;
            void (*onClick)(void* user);
        } button;
        struct {
            const char* label;
            int min;
            int def;
            int max;
            void* user;
            void (*onChange)(void* user, int value);
        } sliderint;
        struct {
            const char* label;
            float min;
            float def;
            float max;
            void* user;
            void (*onChange)(void* user, float value);
        } sliderfloat;
        struct {
            const char* label;
            int size; // 0 = normal, 1 = small title
        } text;
        struct {
            const char* label;
            const char* def;
            const char* placeholder;
            void* user;
            void (*onChange)(void* user, const char* value);
        } textinput;
    } data;
};

using MenuAddFn = void (*)(size_t length, MenuEntryABI* entries);
using MenuShowWindowFn = void (*)(const char* title, int isModal, void* user,
                                  void (*onClose)(void* user), int count, control* controls);
using MenuCloseWindowFn = void (*)(const char* title);
