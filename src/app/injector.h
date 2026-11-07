#pragma once
#include <windows.h>

#include <filesystem>
#include <string>

namespace injector {

DWORD FindProcess(const wchar_t* exeName);
bool IsModuleLoaded(DWORD pid, const wchar_t* moduleName);
// Seconds since the process started, or -1 if unknown.
double ProcessAgeSec(DWORD pid);
// Biggest visible top-level window owned by the process.
HWND FindMainWindow(DWORD pid);

enum class Result { Ok, AccessDenied, Failed };
Result Inject(DWORD pid, const std::filesystem::path& dll, std::string& error);

// Full path of the process's exe (works even when the process is elevated).
std::wstring ProcessPath(DWORD pid);
// "a.b.c.d" from the file's version resource, or empty.
std::string FileVersion(const std::wstring& path);

bool IsElevated();
// Restarts this exe elevated (UAC prompt). Returns false if the user declined.
bool RelaunchElevated();

}  // namespace injector
