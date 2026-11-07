#include "locator.h"

#include <shobjidl.h>

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace locator {

namespace {

constexpr const wchar_t* kExeName = L"eldenring.exe";
const fs::path kSteamRelative = fs::path(L"steamapps") / L"common" / L"ELDEN RING" / L"Game" / kExeName;

std::wstring RegString(HKEY root, const wchar_t* key, const wchar_t* value) {
    wchar_t buf[MAX_PATH * 2]{};
    DWORD size = sizeof(buf);
    if (RegGetValueW(root, key, value, RRF_RT_REG_SZ, nullptr, buf, &size) != ERROR_SUCCESS) return {};
    return buf;
}

// Steam's own folder plus every extra library listed in steamapps\libraryfolders.vdf.
std::vector<fs::path> SteamLibraries() {
    std::vector<fs::path> libs;
    for (const std::wstring& steam : {RegString(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath"),
                                      RegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Valve\\Steam",
                                                L"InstallPath")}) {
        if (steam.empty()) continue;
        const fs::path root = fs::path(steam).make_preferred();
        libs.push_back(root);

        std::ifstream vdf(root / L"steamapps" / L"libraryfolders.vdf");
        std::stringstream text;
        text << vdf.rdbuf();
        const std::string content = text.str();
        static const std::regex pathEntry(R"re("path"\s+"([^"]+)")re");
        for (std::sregex_iterator it(content.begin(), content.end(), pathEntry), end; it != end; ++it) {
            std::string p = (*it)[1].str();
            // VDF escapes backslashes.
            for (size_t pos = 0; (pos = p.find("\\\\", pos)) != std::string::npos; ++pos) p.erase(pos, 1);
            libs.push_back(fs::path(p).make_preferred());
        }
    }
    return libs;
}

std::vector<fs::path> Candidates() {
    std::vector<fs::path> out;
    for (const fs::path& lib : SteamLibraries()) out.push_back(lib / kSteamRelative);

    // Steam's uninstall entry knows the install folder even when the library isn't the default one.
    const std::wstring installed = RegString(
        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 1245620",
        L"InstallLocation");
    if (!installed.empty()) out.push_back(fs::path(installed) / L"Game" / kExeName);

    // Common hand-made locations on every fixed drive.
    const DWORD drives = GetLogicalDrives();
    for (wchar_t letter = L'A'; letter <= L'Z'; ++letter) {
        if (!(drives & (1u << (letter - L'A')))) continue;
        const std::wstring root = std::wstring(1, letter) + L":\\";
        if (GetDriveTypeW(root.c_str()) != DRIVE_FIXED) continue;
        for (const wchar_t* dir : {L"SteamLibrary", L"Steam", L"Games\\Steam", L"Games\\SteamLibrary",
                                   L"Program Files (x86)\\Steam", L"Program Files\\Steam"}) {
            out.push_back(fs::path(root) / dir / kSteamRelative);
        }
        for (const wchar_t* dir : {L"Games\\ELDEN RING", L"ELDEN RING", L"Program Files\\ELDEN RING",
                                   L"Program Files (x86)\\ELDEN RING"}) {
            out.push_back(fs::path(root) / dir / L"Game" / kExeName);
        }
    }
    return out;
}

}  // namespace

bool IsGameExe(const fs::path& path) {
    std::error_code ec;
    return _wcsicmp(path.filename().c_str(), kExeName) == 0 && fs::is_regular_file(path, ec);
}

std::optional<fs::path> AutoDetect() {
    for (const fs::path& candidate : Candidates()) {
        if (IsGameExe(candidate)) return candidate;
    }
    return std::nullopt;
}

std::optional<fs::path> Browse(HWND owner) {
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) {
        return std::nullopt;
    }
    const COMDLG_FILTERSPEC filter[] = {{L"Elden Ring (eldenring.exe)", kExeName}};
    dialog->SetFileTypes(1, filter);
    dialog->SetTitle(L"Find eldenring.exe (inside the game's \"Game\" folder)");
    dialog->SetOptions(FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);

    std::optional<fs::path> result;
    IShellItem* item = nullptr;
    if (SUCCEEDED(dialog->Show(owner)) && SUCCEEDED(dialog->GetResult(&item))) {
        PWSTR path = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
            if (IsGameExe(path)) result = fs::path(path);
            CoTaskMemFree(path);
        }
        item->Release();
    }
    dialog->Release();
    return result;
}

bool IsSteamInstall(const fs::path& exe) {
    std::wstring p = exe.wstring();
    std::transform(p.begin(), p.end(), p.begin(), ::towlower);
    return p.find(L"\\steamapps\\common\\") != std::wstring::npos;
}

}  // namespace locator
