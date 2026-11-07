#include "config.h"

#include <windows.h>

#include <cwchar>

namespace {

std::wstring ReadStr(const std::filesystem::path& ini, const wchar_t* section, const wchar_t* key) {
    wchar_t buf[512]{};
    GetPrivateProfileStringW(section, key, L"", buf, static_cast<DWORD>(std::size(buf)), ini.c_str());
    return buf;
}

// Accepts decimal or 0x-prefixed hex; keeps the default when the key is missing or empty.
template <typename T>
void ReadNum(const std::filesystem::path& ini, const wchar_t* section, const wchar_t* key, T& out) {
    std::wstring s = ReadStr(ini, section, key);
    if (s.empty()) return;
    if constexpr (std::is_floating_point_v<T>) {
        out = static_cast<T>(std::wcstod(s.c_str(), nullptr));
    } else {
        out = static_cast<T>(std::wcstoll(s.c_str(), nullptr, 0));
    }
}

std::string ToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
    return s;
}

}  // namespace

Config LoadConfig(const std::filesystem::path& ini) {
    Config c;
    ReadNum(ini, L"general", L"poll_interval_ms", c.pollIntervalMs);
    c.sessionLabel = ToUtf8(ReadStr(ini, L"general", L"session_label"));
    ReadNum(ini, L"boss", L"search_radius", c.bossSearchRadius);
    ReadNum(ini, L"boss", L"min_max_hp", c.bossMinMaxHp);

    Offsets& o = c.off;
    ReadNum(ini, L"offsets", L"player", o.player);
    ReadNum(ini, L"offsets", L"chr_list_begin", o.chrListBegin);
    ReadNum(ini, L"offsets", L"chr_list_end", o.chrListEnd);
    ReadNum(ini, L"offsets", L"chr_modules", o.chrModules);
    ReadNum(ini, L"offsets", L"module_data", o.moduleData);
    ReadNum(ini, L"offsets", L"module_physics", o.modulePhysics);
    ReadNum(ini, L"offsets", L"physics_pos", o.physicsPos);
    ReadNum(ini, L"offsets", L"data_hp", o.dataHp);
    ReadNum(ini, L"offsets", L"data_max_hp", o.dataMaxHp);
    ReadNum(ini, L"offsets", L"data_flags", o.dataFlags);
    ReadNum(ini, L"offsets", L"no_dead_bit", o.noDeadBit);
    ReadNum(ini, L"offsets", L"use_dbg_player_no_dead", o.useDbgPlayerNoDead);
    ReadNum(ini, L"offsets", L"dbg_player_no_dead", o.dbgPlayerNoDead);

    if (c.pollIntervalMs < 1) c.pollIntervalMs = 1;
    if (o.noDeadBit > 7) o.noDeadBit = 0;
    return c;
}
