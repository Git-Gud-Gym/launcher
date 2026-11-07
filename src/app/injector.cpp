#include "injector.h"

#include <shellapi.h>
#include <tlhelp32.h>

#include <cstdio>
#include <vector>

namespace injector {

DWORD FindProcess(const wchar_t* name) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W e{sizeof(e)};
    for (BOOL ok = Process32FirstW(snap, &e); ok; ok = Process32NextW(snap, &e)) {
        if (_wcsicmp(e.szExeFile, name) == 0) {
            pid = e.th32ProcessID;
            break;
        }
    }
    CloseHandle(snap);
    return pid;
}

bool IsModuleLoaded(DWORD pid, const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W e{sizeof(e)};
    bool found = false;
    for (BOOL ok = Module32FirstW(snap, &e); ok && !found; ok = Module32NextW(snap, &e)) {
        found = _wcsicmp(e.szModule, name) == 0;
    }
    CloseHandle(snap);
    return found;
}

double ProcessAgeSec(DWORD pid) {
    HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!proc) return -1;
    FILETIME created, exited, kernel, user, now;
    const BOOL ok = GetProcessTimes(proc, &created, &exited, &kernel, &user);
    CloseHandle(proc);
    if (!ok) return -1;
    GetSystemTimeAsFileTime(&now);
    auto ticks = [](const FILETIME& f) { return (static_cast<ULONGLONG>(f.dwHighDateTime) << 32) | f.dwLowDateTime; };
    return (ticks(now) - ticks(created)) / 1e7;
}

HWND FindMainWindow(DWORD pid) {
    struct Search {
        DWORD pid;
        HWND best = nullptr;
        LONG bestArea = 0;
    } search{pid};
    EnumWindows(
        [](HWND hwnd, LPARAM param) -> BOOL {
            auto* s = reinterpret_cast<Search*>(param);
            DWORD owner = 0;
            GetWindowThreadProcessId(hwnd, &owner);
            if (owner != s->pid || !IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER)) return TRUE;
            RECT r;
            GetClientRect(hwnd, &r);
            const LONG area = (r.right - r.left) * (r.bottom - r.top);
            if (area > s->bestArea) {
                s->best = hwnd;
                s->bestArea = area;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&search));
    return search.best;
}

Result Inject(DWORD pid, const std::filesystem::path& dll, std::string& error) {
    HANDLE proc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
                                  PROCESS_VM_WRITE | PROCESS_VM_READ,
                              FALSE, pid);
    if (!proc) {
        if (GetLastError() == ERROR_ACCESS_DENIED) return Result::AccessDenied;
        error = "Couldn't open the game process (error " + std::to_string(GetLastError()) + ")";
        return Result::Failed;
    }

    const std::wstring path = dll.wstring();
    const size_t bytes = (path.size() + 1) * sizeof(wchar_t);
    Result result = Result::Failed;
    void* remote = VirtualAllocEx(proc, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remote && WriteProcessMemory(proc, remote, path.c_str(), bytes, nullptr)) {
        // kernel32 is mapped at the same address in every process, so our LoadLibraryW is theirs too.
        auto loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
        if (HANDLE thread = CreateRemoteThread(proc, nullptr, 0, loadLibrary, remote, 0, nullptr)) {
            WaitForSingleObject(thread, 15000);
            DWORD code = 0;
            GetExitCodeThread(thread, &code);
            CloseHandle(thread);
            if (code) {
                result = Result::Ok;
            } else {
                error = "The game refused to load gitgudgym.dll";
            }
        } else {
            error = "Couldn't start the loader thread (error " + std::to_string(GetLastError()) + ")";
        }
    } else {
        error = "Couldn't write into the game process";
    }
    if (remote) VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
    CloseHandle(proc);
    return result;
}

std::wstring ProcessPath(DWORD pid) {
    HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!proc) return {};
    wchar_t path[MAX_PATH];
    DWORD size = MAX_PATH;
    const BOOL ok = QueryFullProcessImageNameW(proc, 0, path, &size);
    CloseHandle(proc);
    return ok ? std::wstring(path, size) : std::wstring();
}

std::string FileVersion(const std::wstring& path) {
    DWORD handle = 0;
    const DWORD size = GetFileVersionInfoSizeW(path.c_str(), &handle);
    if (!size) return {};
    std::vector<BYTE> data(size);
    VS_FIXEDFILEINFO* info = nullptr;
    UINT len = 0;
    if (!GetFileVersionInfoW(path.c_str(), 0, size, data.data()) ||
        !VerQueryValueW(data.data(), L"\\", reinterpret_cast<void**>(&info), &len) || !info) {
        return {};
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%u.%u.%u.%u", HIWORD(info->dwFileVersionMS), LOWORD(info->dwFileVersionMS),
                  HIWORD(info->dwFileVersionLS), LOWORD(info->dwFileVersionLS));
    return buf;
}

bool IsElevated() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return false;
    TOKEN_ELEVATION elevation{};
    DWORD size = 0;
    const bool ok = GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size);
    CloseHandle(token);
    return ok && elevation.TokenIsElevated;
}

bool RelaunchElevated() {
    wchar_t self[MAX_PATH];
    GetModuleFileNameW(nullptr, self, MAX_PATH);
    SHELLEXECUTEINFOW info{sizeof(info)};
    info.lpVerb = L"runas";
    info.lpFile = self;
    info.nShow = SW_SHOWNORMAL;
    return ShellExecuteExW(&info) != FALSE;
}

}  // namespace injector
