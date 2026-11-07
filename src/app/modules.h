#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// A game module's module.json: what the app needs to know about the game it supports.
struct KnownGameVersion {
    std::string exe;    // eldenring.exe file version, e.g. "2.2.0.0"
    std::string patch;  // what players call it, e.g. "1.12.0"
    bool tested = false;
};

struct ModuleManifest {
    std::string id, name, version, tagline, dll;
    std::vector<std::string> files, features;
    std::wstring exe, steamFolder, exeSubdir;
    unsigned steamAppId = 0;
    std::vector<std::wstring> antiCheatProcesses;
    std::vector<KnownGameVersion> gameVersions;
};

std::optional<ModuleManifest> ParseManifest(const std::string& json, std::string& error);

struct InstalledModule {
    ModuleManifest manifest;
    std::filesystem::path dir;
    bool dev = false;  // a local build in <app dir>\modules\, used instead of a downloaded copy
};

// Installed modules from %LOCALAPPDATA%\GitGudGym\modules plus development builds in devDir.
std::vector<InstalledModule> ScanInstalledModules(const std::filesystem::path& devDir);

// Semantic version comparison of "x.y.z" strings: <0, 0, >0.
int CompareVersions(const std::string& a, const std::string& b);
