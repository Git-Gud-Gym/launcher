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

std::wstring RegString(HKEY root, const std::wstring& key, const wchar_t* value) {
    wchar_t buf[MAX_PATH * 2]{};
    DWORD size = sizeof(buf);
    if (RegGetValueW(root, key.c_str(), value, RRF_RT_REG_SZ, nullptr, buf, &size) != ERROR_SUCCESS) return {};
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

std::vector<fs::path> Candidates(const ModuleManifest& m) {
    std::vector<fs::path> out;
    const fs::path inSteam = fs::path(L"steamapps") / L"common" / m.steamFolder / m.exeSubdir / m.exe;
    if (!m.steamFolder.empty()) {
        for (const fs::path& lib : SteamLibraries()) out.push_back(lib / inSteam);
    }

    // Steam's uninstall entry knows the install folder even when the library isn't the default one.
    if (m.steamAppId) {
        const std::wstring installed =
            RegString(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App " +
                          std::to_wstring(m.steamAppId),
                      L"InstallLocation");
        if (!installed.empty()) out.push_back(fs::path(installed) / m.exeSubdir / m.exe);
    }

    // Common hand-made locations on every fixed drive.
    const DWORD drives = GetLogicalDrives();
    for (wchar_t letter = L'A'; letter <= L'Z'; ++letter) {
        if (!(drives & (1u << (letter - L'A')))) continue;
        const fs::path root = std::wstring(1, letter) + L":\\";
        if (GetDriveTypeW(root.c_str()) != DRIVE_FIXED) continue;
        for (const wchar_t* dir : {L"SteamLibrary", L"Steam", L"Games\\Steam", L"Games\\SteamLibrary",
                                   L"Program Files (x86)\\Steam", L"Program Files\\Steam"}) {
            out.push_back(root / dir / inSteam);
        }
        if (m.steamFolder.empty()) continue;
        for (const wchar_t* dir : {L"Games", L"", L"Program Files", L"Program Files (x86)"}) {
            out.push_back(root / dir / m.steamFolder / m.exeSubdir / m.exe);
        }
    }
    return out;
}

}  // namespace

bool IsGameExe(const ModuleManifest& m, const fs::path& path) {
    std::error_code ec;
    return _wcsicmp(path.filename().c_str(), m.exe.c_str()) == 0 && fs::is_regular_file(path, ec);
}

std::optional<fs::path> AutoDetect(const ModuleManifest& m) {
    for (const fs::path& candidate : Candidates(m)) {
        if (IsGameExe(m, candidate)) return candidate;
    }
    return std::nullopt;
}

std::optional<fs::path> Browse(HWND owner, const ModuleManifest& m) {
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) {
        return std::nullopt;
    }
    const std::wstring label = L"Game executable (" + m.exe + L")";
    const COMDLG_FILTERSPEC filter[] = {{label.c_str(), m.exe.c_str()}};
    dialog->SetFileTypes(1, filter);
    const std::wstring title = L"Find " + m.exe;
    dialog->SetTitle(title.c_str());
    dialog->SetOptions(FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);

    std::optional<fs::path> result;
    IShellItem* item = nullptr;
    if (SUCCEEDED(dialog->Show(owner)) && SUCCEEDED(dialog->GetResult(&item))) {
        PWSTR path = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
            if (IsGameExe(m, path)) result = fs::path(path);
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
