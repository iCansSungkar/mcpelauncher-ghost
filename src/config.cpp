#include "config.h"

#include "log.h"

#include <dlfcn.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace cfg {

namespace {

std::string configPath() {
    // Prefer the folder this .so lives in (the launcher's mods directory).
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&configPath), &info) && info.dli_fname) {
        std::string p = info.dli_fname;
        auto slash = p.find_last_of('/');
        if (slash != std::string::npos) return p.substr(0, slash + 1) + "imghost.cfg";
    }
    return "imghost.cfg";
}

std::string trim(const std::string& s) {
    auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

bool toBool(const std::string& v) {
    return v == "1" || v == "true" || v == "yes" || v == "on";
}

int toInt(const std::string& v, int fallback) {
    char* end = nullptr;
    long n = std::strtol(v.c_str(), &end, 10);
    return (end && end != v.c_str()) ? static_cast<int>(n) : fallback;
}

} // namespace

Config& get() {
    static Config c;
    return c;
}

void load() {
    Config& c = get();
    const std::string path = configPath();
    std::ifstream in(path);
    if (!in) {
        LOGI("no config at %s, using defaults", path.c_str());
        return;
    }
    LOGI("loading config from %s", path.c_str());

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = trim(line.substr(0, eq));
        std::string v = trim(line.substr(eq + 1));

        if (k == "hotkey_key") c.hotkey.key = toInt(v, c.hotkey.key);
        else if (k == "hotkey_alt") c.hotkey.alt = toBool(v);
        else if (k == "hotkey_ctrl") c.hotkey.ctrl = toBool(v);
        else if (k == "hotkey_shift") c.hotkey.shift = toBool(v);
        else if (k == "exit_mode") c.exitMode = (v == "origin") ? 1 : 0;
        else if (k == "debug_keys") c.debugKeys = toBool(v);
        else if (k == "key_alt_l") c.keyAltL = toInt(v, c.keyAltL);
        else if (k == "key_alt_r") c.keyAltR = toInt(v, c.keyAltR);
        else if (k == "key_ctrl_l") c.keyCtrlL = toInt(v, c.keyCtrlL);
        else if (k == "key_ctrl_r") c.keyCtrlR = toInt(v, c.keyCtrlR);
        else if (k == "key_shift_l") c.keyShiftL = toInt(v, c.keyShiftL);
        else if (k == "key_shift_r") c.keyShiftR = toInt(v, c.keyShiftR);
        else if (k == "action_press") c.actionPress = toInt(v, c.actionPress);
        else if (k == "action_release") c.actionRelease = toInt(v, c.actionRelease);
        else if (k == "action_repeat") c.actionRepeat = toInt(v, c.actionRepeat);
        else if (k == "gametype_survival") c.gameTypeSurvival = toInt(v, c.gameTypeSurvival);
        else if (k == "gametype_spectator") c.gameTypeSpectator = toInt(v, c.gameTypeSpectator);
        else if (k == "sym_tick") c.symTick = v;
        else if (k == "sym_get_pos") c.symGetPos = v;
        else if (k == "sym_set_pos") c.symSetPos = v;
        else if (k == "sym_get_gametype") c.symGetGameType = v;
        else if (k == "sym_set_gametype") c.symSetGameType = v;
        else LOGW("unknown config key '%s'", k.c_str());
    }
}

void save() {
    Config& c = get();
    const std::string path = configPath();
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        LOGE("cannot write config to %s", path.c_str());
        return;
    }
    out << "# I'm Ghost configuration\n"
        << "hotkey_key=" << c.hotkey.key << "\n"
        << "hotkey_alt=" << (c.hotkey.alt ? 1 : 0) << "\n"
        << "hotkey_ctrl=" << (c.hotkey.ctrl ? 1 : 0) << "\n"
        << "hotkey_shift=" << (c.hotkey.shift ? 1 : 0) << "\n"
        << "exit_mode=" << (c.exitMode.load() == 1 ? "origin" : "stay") << "\n"
        << "debug_keys=" << (c.debugKeys ? 1 : 0) << "\n";
    LOGI("config saved to %s", path.c_str());
}

std::string describeHotkey(const Hotkey& hk) {
    std::ostringstream s;
    if (hk.ctrl) s << "Ctrl+";
    if (hk.alt) s << "Alt+";
    if (hk.shift) s << "Shift+";
    if (hk.key >= 65 && hk.key <= 90) s << static_cast<char>(hk.key);
    else if (hk.key >= 48 && hk.key <= 57) s << static_cast<char>(hk.key);
    else if (hk.key >= 112 && hk.key <= 123) s << "F" << (hk.key - 111);
    else s << "Key" << hk.key;
    return s.str();
}

} // namespace cfg
