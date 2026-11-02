#include "ghost.h"

#include "config.h"
#include "launcher_api.h"
#include "log.h"

#include <dlfcn.h>

#include <atomic>

namespace ghost {

namespace {

struct Vec3 {
    float x, y, z;
};

// Itanium ABI: a non-static member function is a free function whose first
// argument is `this`.
using TickFn = void (*)(void* self);
using GetPosFn = const Vec3& (*)(void* actor);
using SetPosFn = void (*)(void* actor, const Vec3& pos);
using GetGameTypeFn = int (*)(void* player);
using SetGameTypeFn = void (*)(void* player, int gameType);

TickFn g_origTick = nullptr;
GetPosFn g_getPos = nullptr;
SetPosFn g_setPos = nullptr;
GetGameTypeFn g_getGameType = nullptr;
SetGameTypeFn g_setGameType = nullptr;

std::atomic<bool> g_toggleRequested{false};
std::atomic<bool> g_active{false};

// Game-thread only state.
void* g_player = nullptr;
int g_previousGameType = 0;
Vec3 g_origin{0, 0, 0};

void enterGhost(void* player) {
    const cfg::Config& c = cfg::get();

    g_origin = g_getPos(player);
    g_previousGameType = g_getGameType(player);
    // Never "restore" into spectator (e.g. if the world was already in it).
    if (g_previousGameType == c.gameTypeSpectator) g_previousGameType = c.gameTypeSurvival;

    // Direct call on the player: no command is run, so the world's
    // "cheats enabled" flag (and therefore Hardcore) is irrelevant.
    g_setGameType(player, c.gameTypeSpectator);
    g_active = true;
    LOGI("ghost ON  (origin %.2f %.2f %.2f, previous gametype %d)", g_origin.x, g_origin.y,
         g_origin.z, g_previousGameType);
}

void leaveGhost(void* player) {
    const cfg::Config& c = cfg::get();

    // Position of the ghost at the moment it turns back into a player.
    const Vec3 last = g_getPos(player);

    g_setGameType(player, g_previousGameType);

    if (c.exitMode.load() == static_cast<int>(cfg::ExitMode::Origin)) {
        g_setPos(player, g_origin);
        LOGI("ghost OFF (returned to origin %.2f %.2f %.2f)", g_origin.x, g_origin.y, g_origin.z);
    } else {
        // Re-apply the ghost's last position so changing game type cannot move us.
        g_setPos(player, last);
        LOGI("ghost OFF (stayed at %.2f %.2f %.2f)", last.x, last.y, last.z);
    }
    g_active = false;
}

void onLocalPlayerTick(void* player) {
    // A different player object means a new world/session: the old state is void.
    if (player != g_player) {
        g_player = player;
        g_active = false;
        g_toggleRequested = false;
    }

    if (!g_toggleRequested.exchange(false)) return;

    if (g_active.load()) leaveGhost(player);
    else enterGhost(player);
}

void hookedTick(void* self) {
    g_origTick(self);
    onLocalPlayerTick(self);
}

void* resolve(void* lib, const std::string& name, const char* what) {
    void* p = dlsym(lib, name.c_str());
    if (!p) LOGE("symbol for %s not found: %s", what, name.c_str());
    else LOGI("resolved %s -> %s", what, name.c_str());
    return p;
}

} // namespace

bool install(void* gameLib) {
    if (!gameLib) {
        LOGE("libminecraftpe.so handle is null");
        return false;
    }
    if (!mcpelauncher_hook) {
        LOGE("mcpelauncher_hook is not provided by the launcher");
        return false;
    }

    const cfg::Config& c = cfg::get();

    void* tick = resolve(gameLib, c.symTick, "player tick");
    g_getPos = reinterpret_cast<GetPosFn>(resolve(gameLib, c.symGetPos, "get position"));
    g_setPos = reinterpret_cast<SetPosFn>(resolve(gameLib, c.symSetPos, "set position"));
    g_getGameType =
        reinterpret_cast<GetGameTypeFn>(resolve(gameLib, c.symGetGameType, "get game type"));
    g_setGameType =
        reinterpret_cast<SetGameTypeFn>(resolve(gameLib, c.symSetGameType, "set game type"));

    if (!tick || !g_getPos || !g_setPos || !g_getGameType || !g_setGameType) {
        LOGE("missing symbols: ghost mode disabled. Check them with "
             "`nm -D libminecraftpe.so` and override in imghost.cfg (sym_* keys)");
        return false;
    }

    void* hook = mcpelauncher_hook(tick, reinterpret_cast<void*>(&hookedTick),
                                   reinterpret_cast<void**>(&g_origTick));
    if (!hook || !g_origTick) {
        LOGE("failed to hook player tick");
        return false;
    }

    LOGI("ghost hooks installed");
    return true;
}

void requestToggle() {
    g_toggleRequested = true;
}

bool isActive() {
    return g_active.load();
}

} // namespace ghost
