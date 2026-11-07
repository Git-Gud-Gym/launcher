#pragma once
#include <windows.h>

#include <filesystem>
#include <optional>

// Finds eldenring.exe on the user's machine.
namespace locator {

bool IsGameExe(const std::filesystem::path& path);

// Checks Steam (registry + every library in libraryfolders.vdf), the uninstall entry,
// and common folders on every fixed drive. Returns the first eldenring.exe found.
std::optional<std::filesystem::path> AutoDetect();

// Lets the user pick eldenring.exe. Returns nothing if they cancel.
std::optional<std::filesystem::path> Browse(HWND owner);

// True for a Steam install, where the game needs steam_appid.txt to start without Easy Anti-Cheat.
bool IsSteamInstall(const std::filesystem::path& exe);

}  // namespace locator
