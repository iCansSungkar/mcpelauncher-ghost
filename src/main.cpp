#include "config.h"
#include "ghost.h"
#include "input.h"
#include "log.h"
#include "menu.h"

#include <dlfcn.h>

namespace {
bool g_initialised = false;
}

// Called by the launcher once libminecraftpe.so is loaded.
extern "C" __attribute__((visibility("default"))) void mod_init() {
    if (g_initialised) return;
    g_initialised = true;

    LOGI("I'm Ghost: mod_init");
    cfg::load();

    // Handle of the already-loaded game library, used to resolve its symbols.
    void* game = dlopen("libminecraftpe.so", RTLD_NOW);
    const bool hooked = ghost::install(game);

    // Hotkey and menu are installed even if hooks failed, so the user can at
    // least see the mod loaded and read the reason in logcat.
    input::install();
    menu::install();

    LOGI("I'm Ghost: ready (hooks %s)", hooked ? "ok" : "FAILED");
}
