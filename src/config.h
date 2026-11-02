#pragma once

#include <atomic>
#include <string>

namespace cfg {

// What happens to the player's position when ghost mode is switched off.
enum class ExitMode : int {
    Stay = 0,   // stay exactly where the ghost was (default)
    Origin = 1, // go back to where ghost mode was started
};

struct Hotkey {
    int key = 88; // 'X' (see note on key codes in README)
    bool alt = true;
    bool ctrl = false;
    bool shift = false;
};

struct Config {
    Hotkey hotkey;
    std::atomic<int> exitMode{static_cast<int>(ExitMode::Stay)};
    bool debugKeys = false; // log every key code/action to logcat

    // Key codes used to track modifier state. Defaults follow the Windows
    // virtual-key layout; override in imghost.cfg if debug_keys shows otherwise.
    int keyAltL = 18, keyAltR = 165;
    int keyCtrlL = 17, keyCtrlR = 163;
    int keyShiftL = 16, keyShiftR = 161;

    // Key actions as delivered by the game window callback.
    int actionPress = 0, actionRelease = 1, actionRepeat = 2;

    // GameType values (Bedrock): Survival=0, Creative=1, Adventure=2, Spectator=6
    int gameTypeSurvival = 0;
    int gameTypeSpectator = 6;

    // Symbols looked up in libminecraftpe.so. Override them in imghost.cfg
    // when the game version you run uses different signatures.
    std::string symTick = "_ZN11LocalPlayer10normalTickEv";
    std::string symGetPos = "_ZNK5Actor11getPositionEv";
    std::string symSetPos = "_ZN5Actor6setPosERK4Vec3";
    std::string symGetGameType = "_ZNK6Player17getPlayerGameTypeEv";
    std::string symSetGameType = "_ZN6Player17setPlayerGameTypeE8GameType";
};

Config& get();

// Load imghost.cfg (next to the mod .so when possible). Missing file = defaults.
void load();
// Write the user-editable settings (hotkey, exit mode) back to disk.
void save();

std::string describeHotkey(const Hotkey& hk);

} // namespace cfg
