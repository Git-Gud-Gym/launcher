#include "modules.h"

#include <windows.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <tuple>

#include "json.hpp"
#include "paths.h"

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

std::wstring Widen(const std::string& s) {
    std::wstring w(s.size(), L'\0');
    w.resize(MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(),
                                 static_cast<int>(w.size())));
    return w;
}

bool ValidId(const std::string& id) {
    return !id.empty() && id.size() < 40 &&
           std::all_of(id.begin(), id.end(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); });
}

void Scan(const fs::path& root, bool dev, std::vector<InstalledModule>& out) {
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(root, ec)) {
        if (!entry.is_directory()) continue;
        std::ifstream in(entry.path() / L"module.json");
        if (!in) continue;
        std::stringstream text;
        text << in.rdbuf();
        std::string error;
        auto manifest = ParseManifest(text.str(), error);
        if (!manifest || !fs::exists(entry.path() / Widen(manifest->dll))) continue;
        // A dev build replaces an installed copy of the same module.
        out.erase(std::remove_if(out.begin(), out.end(),
                                 [&](const InstalledModule& m) { return m.manifest.id == manifest->id; }),
                  out.end());
        out.push_back({*manifest, entry.path(), dev});
    }
}

}  // namespace

std::optional<ModuleManifest> ParseManifest(const std::string& text, std::string& error) {
    try {
        const json j = json::parse(text);
        ModuleManifest m;
        m.id = j.at("id").get<std::string>();
        m.name = j.at("name").get<std::string>();
        m.version = j.at("version").get<std::string>();
        m.tagline = j.value("tagline", "");
        m.dll = j.at("dll").get<std::string>();
        m.files = j.value("files", std::vector<std::string>{});
        m.features = j.value("features", std::vector<std::string>{});
        const json& g = j.at("game");
        m.exe = Widen(g.at("exe").get<std::string>());
        m.steamAppId = g.value("steamAppId", 0u);
        m.steamFolder = Widen(g.value("steamFolder", ""));
        m.exeSubdir = Widen(g.value("exeSubdir", ""));
        for (const auto& p : g.value("antiCheatProcesses", std::vector<std::string>{})) {
            m.antiCheatProcesses.push_back(Widen(p));
        }
        for (const auto& v : g.value("versions", json::array())) {
            m.gameVersions.push_back({v.at("exe").get<std::string>(), v.at("patch").get<std::string>(),
                                      v.value("tested", false)});
        }
        // The id becomes a folder name and the dll a file name: keep both boring.
        if (!ValidId(m.id) || m.dll.find_first_of("/\\:") != std::string::npos) {
            error = "Invalid module id or dll name";
            return std::nullopt;
        }
        return m;
    } catch (const std::exception& e) {
        error = std::string("Invalid module.json: ") + e.what();
        return std::nullopt;
    }
}

std::vector<InstalledModule> ScanInstalledModules(const fs::path& devDir) {
    std::vector<InstalledModule> out;
    Scan(ggg::paths::ModulesDir(), false, out);
    Scan(devDir, true, out);
    std::sort(out.begin(), out.end(),
              [](const InstalledModule& a, const InstalledModule& b) { return a.manifest.name < b.manifest.name; });
    return out;
}

int CompareVersions(const std::string& a, const std::string& b) {
    auto parse = [](const std::string& v) {
        int parts[3] = {0, 0, 0};
        sscanf_s(v.c_str(), "%d.%d.%d", &parts[0], &parts[1], &parts[2]);
        return std::make_tuple(parts[0], parts[1], parts[2]);
    };
    const auto pa = parse(a), pb = parse(b);
    return pa < pb ? -1 : (pa > pb ? 1 : 0);
}
