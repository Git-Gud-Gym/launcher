#pragma once
#include <windows.h>

#include <filesystem>
#include <optional>

#include "modules.h"

// Finds a module's game executable on the user's machine.
namespace locator {

bool IsGameExe(const ModuleManifest& m, const std::filesystem::path& path);

// Checks Steam (registry + every library in libraryfolders.vdf), Steam's uninstall entry,
// and common folders on every fixed drive. Returns the first match.
std::optional<std::filesystem::path> AutoDetect(const ModuleManifest& m);

// Lets the user pick the game's exe. Returns nothing if they cancel or pick something else.
std::optional<std::filesystem::path> Browse(HWND owner, const ModuleManifest& m);

// True for a Steam install, where the game needs steam_appid.txt to start without anti-cheat.
bool IsSteamInstall(const std::filesystem::path& exe);

}  // namespace locator
