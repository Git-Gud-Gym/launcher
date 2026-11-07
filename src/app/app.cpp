#include "app.h"

#include <mmsystem.h>
#include <shellapi.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <utility>

#include "http.h"
#include "locator.h"
#include "paths.h"
#include "version.h"

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

namespace {

// The game library, published with the website. Overridable with [app] catalog_url in settings.ini.
constexpr const wchar_t* kDefaultCatalogUrl = L"https://git-gud-gym.github.io/registry/catalog.json";
constexpr double kGameWarmupSec = 20;  // let the game finish unpacking before injecting

std::wstring Widen(const std::string& s) {
    std::wstring w(s.size(), L'\0');
    w.resize(MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(),
                                 static_cast<int>(w.size())));
    return w;
}

std::string Narrow(const std::wstring& w) {
    std::string s(w.size() * 3, '\0');
    s.resize(WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), s.data(),
                                 static_cast<int>(s.size()), nullptr, nullptr));
    return s;
}

std::wstring ReadIni(const fs::path& file, const std::wstring& section, const wchar_t* key) {
    wchar_t buf[1024]{};
    GetPrivateProfileStringW(section.c_str(), key, L"", buf, 1024, file.c_str());
    return buf;
}

void WriteIni(const fs::path& file, const std::wstring& section, const wchar_t* key, const std::wstring& value) {
    WritePrivateProfileStringW(section.c_str(), key, value.c_str(), file.c_str());
}

bool ReadBool(const fs::path& file, const wchar_t* key, bool def) {
    return GetPrivateProfileIntW(L"app", key, def ? 1 : 0, file.c_str()) != 0;
}

std::wstring FormatTime(float sec) {
    const int s = static_cast<int>(sec);
    wchar_t buf[32];
    swprintf_s(buf, L"%d:%02d", s / 60, s % 60);
    return buf;
}

std::string ReadFile(const fs::path& file) {
    std::ifstream in(file, std::ios::binary);
    std::stringstream text;
    text << in.rdbuf();
    return text.str();
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// Startup and shutdown

bool App::Init(HINSTANCE instance, HWND hwnd) {
    window = hwnd;
    wchar_t self[MAX_PATH];
    GetModuleFileNameW(nullptr, self, MAX_PATH);
    dir = fs::path(self).parent_path();
    settingsIni_ = ggg::paths::SettingsIni();

    prefs.toasts = ReadBool(settingsIni_, L"toasts", true);
    prefs.hud = ReadBool(settingsIni_, L"hud", true);
    prefs.soundDeath = ReadBool(settingsIni_, L"sound_death", true);
    prefs.soundDefeat = ReadBool(settingsIni_, L"sound_defeat", true);
    savedPrefs_ = prefs;

    bool created = false;
    shared = ggg::OpenShared(mapping_, created);
    if (!shared) return false;
    // If a mod is already running it owns the live settings; otherwise start from the saved ones.
    if (created) shared->settings = ggg::ReadSettings(settingsIni_);
    savedSettings_ = shared->settings;
    savedSettings_.passThrough = 0;

    ScanModules();
    activeId_ = Narrow(ReadIni(settingsIni_, L"app", L"last_game"));
    if (!Active() && !installed.empty()) activeId_ = installed.front().manifest.id;
    LoadActiveGame();
    page = Active() ? Page::Game : Page::Library;

    // Show the last known library immediately, then refresh it in the background.
    std::string error;
    if (auto cached = ParseCatalog(ReadFile(ggg::paths::Root() / L"catalog.json"), error)) {
        catalog = std::move(cached);
        catalogStatus = "Offline copy";
    }
    RefreshCatalog();

    ReadEvents(false);  // show recent history without replaying toasts and sounds
    overlay_.Init(instance, (dir / L"assets" / L"logo.png").wstring());
    return true;
}

void App::Shutdown() {
    if (connected) {
        // Closing the app returns the game to vanilla.
        SendCommand(ggg::Command::Unload);
        for (int i = 0; i < 30 && ggg::Load(shared->status.running); ++i) Sleep(50);
    }
    for (std::thread* t : {&injectWorker_, &catalogWorker_, &installWorker_}) {
        if (t->joinable()) t->join();
    }
    overlay_.Shutdown();
    if (shared) UnmapViewOfFile(shared);
    if (mapping_) CloseHandle(mapping_);
}

void App::Tick() {
    FinishBackgroundWork();
    std::memcpy(&status, static_cast<const void*>(&shared->status), sizeof(status));
    connected = status.running && GetTickCount64() - status.heartbeat < 2000;
    UpdateConnection();
    ReadEvents(true);

    const bool gameFocused = gameWindow_ && GetForegroundWindow() == gameWindow_;
    overlay_.Update(connected ? gameWindow_ : nullptr, gameFocused, status, prefs.hud, prefs.toasts);
    Persist();
}

// ---------------------------------------------------------------------------------------------
// Library: installed modules + the published catalog

void App::ScanModules() {
    installed = ScanInstalledModules(dir / L"modules");
}

const InstalledModule* App::Active() const {
    for (const InstalledModule& m : installed) {
        if (m.manifest.id == activeId_) return &m;
    }
    return nullptr;
}

std::vector<LibraryEntry> App::Library() const {
    std::vector<LibraryEntry> out;
    if (catalog) {
        for (const CatalogModule& c : catalog->modules) {
            LibraryEntry e;
            e.id = c.id;
            e.name = c.name;
            e.tagline = c.tagline;
            e.features = c.features;
            e.planned = c.status == "planned";
            if (!c.files.empty() && !c.version.empty()) e.published = &c;
            out.push_back(e);
        }
    }
    for (const InstalledModule& m : installed) {
        auto it = std::find_if(out.begin(), out.end(), [&](const LibraryEntry& e) { return e.id == m.manifest.id; });
        if (it == out.end()) {
            LibraryEntry e;
            e.id = m.manifest.id;
            e.name = m.manifest.name;
            e.tagline = m.manifest.tagline;
            e.features = m.manifest.features;
            out.push_back(e);
            it = std::prev(out.end());
        }
        it->installed = &m;
        it->planned = false;
        it->updateAvailable = !m.dev && it->published &&
                              CompareVersions(it->published->version, m.manifest.version) > 0;
    }
    // Installed first, then available, then planned.
    std::stable_sort(out.begin(), out.end(), [](const LibraryEntry& a, const LibraryEntry& b) {
        auto rank = [](const LibraryEntry& e) { return e.installed ? 0 : (e.planned ? 2 : 1); };
        return rank(a) < rank(b);
    });
    return out;
}

void App::RefreshCatalog() {
    if (catalogLoading_) return;
    if (catalogWorker_.joinable()) catalogWorker_.join();
    std::wstring url = ReadIni(settingsIni_, L"app", L"catalog_url");
    if (url.empty()) url = kDefaultCatalogUrl;
    catalogLoading_ = true;
    catalogWorker_ = std::thread([this, url = Narrow(url)] {
        std::string body, error;
        std::optional<Catalog> parsed;
        if (http::Get(url, body, error)) parsed = ParseCatalog(body, error);
        if (parsed) std::ofstream(ggg::paths::Root() / L"catalog.json", std::ios::binary) << body;
        std::lock_guard lock(mu_);
        fetchedCatalog_ = std::move(parsed);
        fetchedStatus_ = fetchedCatalog_ ? "Up to date" : error;
        catalogLoading_ = false;
    });
}

bool App::AppUpdateAvailable() const {
    return catalog && !catalog->appVersion.empty() && CompareVersions(catalog->appVersion, GGG_V_STRING) > 0;
}

std::string App::AppUpdateUrl() const { return catalog ? catalog->appReleaseUrl : std::string(); }

bool App::ModuleInUse(const std::string& id) const { return connected && attachedId_ == id; }

void App::Install(const std::string& id) {
    if (installing_) return;
    const CatalogModule* source = nullptr;
    if (catalog) {
        for (const CatalogModule& c : catalog->modules) {
            if (c.id == id && !c.files.empty()) source = &c;
        }
    }
    if (!source) {
        libraryMessage = "This game has nothing to download yet.";
        return;
    }
    if (ModuleInUse(id)) {
        libraryMessage = "Unload the mod (or close the game) before updating it.";
        return;
    }
    if (installWorker_.joinable()) installWorker_.join();

    installing_ = true;
    installId_ = id;
    installProgressShared_ = 0;
    libraryMessage.clear();
    installWorker_ = std::thread([this, module = *source] {
        const fs::path root = ggg::paths::ModulesDir();
        const std::wstring wid = Widen(module.id);
        const fs::path staging = root / (wid + L".download");
        std::error_code ec;
        fs::remove_all(staging, ec);
        fs::create_directories(staging, ec);

        uint64_t total = 0, finished = 0;
        for (const CatalogFile& f : module.files) total += f.size;
        std::string error;
        bool ok = true;
        for (const CatalogFile& f : module.files) {
            if (f.name.empty() || f.name.find_first_of("/\\:") != std::string::npos) {
                error = "The catalog lists an invalid file name";
                ok = false;
                break;
            }
            const fs::path target = staging / Widen(f.name);
            ok = http::Download(
                f.url, target,
                [&](uint64_t done, uint64_t) {
                    if (total) installProgressShared_ = static_cast<float>(finished + done) / total;
                },
                error);
            if (!ok) break;
            // Never install a file that doesn't match what the catalog published.
            if (http::Sha256File(target) != f.sha256) {
                error = f.name + " failed its checksum; try again";
                ok = false;
                break;
            }
            finished += f.size;
        }

        if (ok) {
            std::string parseError;
            auto manifest = ParseManifest(ReadFile(staging / L"module.json"), parseError);
            if (!manifest || manifest->id != module.id) {
                error = manifest ? "The download is for a different game" : parseError;
                ok = false;
            }
        }
        if (ok) {
            // Swap the new files in; the old copy moves aside first so a locked DLL fails cleanly.
            const fs::path final = root / wid, old = root / (wid + L".old");
            fs::remove_all(old, ec);
            if (fs::exists(final)) fs::rename(final, old, ec);
            if (ec) {
                error = "The current version is in use. Close the game and try again.";
                ok = false;
            } else {
                fs::rename(staging, final, ec);
                if (ec) {
                    error = "Couldn't move the new files into place";
                    ok = false;
                    fs::rename(old, final, ec);
                }
                fs::remove_all(old, ec);
            }
        }
        if (!ok) fs::remove_all(staging, ec);

        std::lock_guard lock(mu_);
        installResult_ = ok ? module.name + " " + module.version + " installed." : error;
        installDone_ = true;
        installing_ = false;
    });
}

void App::Uninstall(const std::string& id) {
    auto it = std::find_if(installed.begin(), installed.end(),
                           [&](const InstalledModule& m) { return m.manifest.id == id; });
    if (it == installed.end()) return;
    if (it->dev) {
        libraryMessage = "That's a development build; delete it from " + Narrow((dir / L"modules").wstring()) + ".";
        return;
    }
    if (ModuleInUse(id)) {
        libraryMessage = "Unload the mod (or close the game) before uninstalling it.";
        return;
    }
    std::error_code ec;
    fs::remove_all(it->dir, ec);
    libraryMessage = ec ? "Couldn't remove the files (are they in use?)" : it->manifest.name + " uninstalled.";
    const bool wasActive = id == activeId_;
    ScanModules();
    if (wasActive) {
        activeId_.clear();
        LoadActiveGame();
        page = Page::Library;
    }
}

void App::FinishBackgroundWork() {
    std::lock_guard lock(mu_);
    installProgress = installProgressShared_;
    if (!catalogLoading_ && catalogWorker_.joinable()) {
        catalogWorker_.join();
        if (fetchedCatalog_) {
            catalog = std::move(fetchedCatalog_);
            catalogStatus = fetchedStatus_;
            catalogError.clear();
            if (!pendingInstall_.empty()) Install(std::exchange(pendingInstall_, {}));
        } else {
            catalogStatus = catalog ? "Offline copy: library unreachable" : "Library unreachable";
            catalogError = fetchedStatus_;
        }
        fetchedCatalog_.reset();
    }
    if (installDone_) {
        installDone_ = false;
        if (installWorker_.joinable()) installWorker_.join();
        libraryMessage = installResult_;
        ScanModules();
        if (!Active()) {
            activeId_ = installId_;
        }
        LoadActiveGame();
    }
}

// ---------------------------------------------------------------------------------------------
// The active game

void App::OpenGame(const std::string& id) {
    if (id != activeId_) {
        if (connected) SendCommand(ggg::Command::Unload);  // one game at a time
        activeId_ = id;
        WriteIni(settingsIni_, L"app", L"last_game", Widen(id));
        LoadActiveGame();
    }
    page = Page::Game;
}

void App::LoadActiveGame() {
    pid_ = knownPid_ = failedPid_ = 0;
    gameWindow_ = nullptr;
    lastProcessCheck_ = {};
    autoAttach_ = true;
    gameVersion.clear();
    locateMessage.clear();
    const InstalledModule* m = Active();
    if (!m) {
        gamePath.clear();
        gameFound = false;
        return;
    }
    gamePath = ReadIni(settingsIni_, L"game." + Widen(m->manifest.id), L"path");
    gameFound = locator::IsGameExe(m->manifest, gamePath);
    if (!gameFound) DetectGame();
}

void App::SetGamePath(const fs::path& exe) {
    const InstalledModule* m = Active();
    if (!m) return;
    gamePath = exe;
    gameFound = locator::IsGameExe(m->manifest, exe);
    WriteIni(settingsIni_, L"game." + Widen(m->manifest.id), L"path", exe.wstring());
}

void App::DetectGame() {
    const InstalledModule* m = Active();
    if (!m) return;
    if (auto found = locator::AutoDetect(m->manifest)) {
        SetGamePath(*found);
        locateMessage = "Found automatically.";
    } else {
        locateMessage = "Not found in your Steam libraries or the usual folders. Use Browse.";
    }
}

void App::BrowseForGame() {
    const InstalledModule* m = Active();
    if (!m) return;
    if (auto picked = locator::Browse(window, m->manifest)) {
        SetGamePath(*picked);
        locateMessage.clear();
    }
}

void App::LaunchGame() {
    const InstalledModule* m = Active();
    if (!m) return;
    if (!gameFound) {
        BrowseForGame();
        if (!gameFound) return;
    }
    // A Steam copy started directly relaunches through Steam with anti-cheat unless steam_appid.txt exists.
    const fs::path appId = gamePath.parent_path() / L"steam_appid.txt";
    if (m->manifest.steamAppId && locator::IsSteamInstall(gamePath) && !fs::exists(appId)) {
        const std::wstring text = L"To start " + Widen(m->manifest.name) +
                                  L" offline without anti-cheat, Git Gud Gym will create steam_appid.txt in the "
                                  L"game folder.\n\nDelete that file to play online again.\n\nContinue?";
        if (MessageBoxW(window, text.c_str(), L"Git Gud Gym", MB_YESNO | MB_ICONINFORMATION) != IDYES) return;
        std::ofstream(appId) << m->manifest.steamAppId;
    }
    ShellExecuteW(nullptr, L"open", gamePath.c_str(), nullptr, gamePath.parent_path().c_str(), SW_SHOWNORMAL);
}

void App::SendCommand(ggg::Command cmd) {
    ggg::Store(shared->cmd, cmd);
    ggg::Store(shared->cmdSeq, ggg::Load(shared->cmdSeq) + 1);
}

void App::PrimaryAction() {
    switch (conn) {
        case Conn::Connected:
            autoAttach_ = false;
            SendCommand(ggg::Command::Unload);
            break;
        case Conn::NoGame:
            LaunchGame();
            break;
        case Conn::NeedsAdmin:
            if (injector::RelaunchElevated()) PostMessageW(window, WM_CLOSE, 0, 0);
            break;
        case Conn::Detached:
        case Conn::Error:
            autoAttach_ = true;
            failedPid_ = 0;
            lastProcessCheck_ = {};
            break;
        default:
            break;
    }
}

void App::OpenDataFolder() const {
    if (activeId_.empty()) return;
    ShellExecuteW(nullptr, L"open", ggg::paths::DataDir(Widen(activeId_)).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void App::OpenLog() const {
    if (activeId_.empty()) return;
    const fs::path log = ggg::paths::DataDir(Widen(activeId_)) / L"gitgudgym.log";
    ShellExecuteW(nullptr, L"open", log.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void App::UpdateConnection() {
    if (injectWorker_.joinable() && !injecting_) {
        injectWorker_.join();
        if (injectResult_ == injector::Result::Ok) {
            attachStarted_ = Clock::now();
        } else if (injectResult_ == injector::Result::AccessDenied && !injector::IsElevated()) {
            conn = Conn::NeedsAdmin;
            failedPid_ = pid_;
        } else {
            conn = Conn::Error;
            connDetail = injectError_.empty() ? "Access denied" : injectError_;
            failedPid_ = pid_;
        }
    }
    if (connected) {
        conn = Conn::Connected;
        connDetail.clear();
        return;
    }
    if (injecting_) {
        conn = Conn::Attaching;
        return;
    }
    const InstalledModule* m = Active();
    if (!m) {
        conn = Conn::NoGame;
        return;
    }
    if (Clock::now() - lastProcessCheck_ < 500ms) return;
    lastProcessCheck_ = Clock::now();

    pid_ = injector::FindProcess(m->manifest.exe.c_str());
    gameWindow_ = pid_ ? injector::FindMainWindow(pid_) : nullptr;
    if (!pid_) {
        conn = Conn::NoGame;
        failedPid_ = 0;
        autoAttach_ = true;  // the next launch attaches again
        return;
    }
    if (pid_ != knownPid_) OnGameFound(pid_);
    if (failedPid_ == pid_) return;  // stay in NeedsAdmin / Error until the user acts
    // Never attach while anti-cheat is running: the game must be started offline without it.
    for (const std::wstring& name : m->manifest.antiCheatProcesses) {
        if (injector::FindProcess(name.c_str())) {
            conn = Conn::AntiCheat;
            return;
        }
    }
    if (!autoAttach_) {
        conn = Conn::Detached;
        return;
    }
    if (Clock::now() - attachStarted_ < 15s) {
        conn = Conn::Attaching;  // loaded; the mod is still finding its way around the game
        return;
    }
    const double age = injector::ProcessAgeSec(pid_);
    if (age >= 0 && age < kGameWarmupSec) {
        conn = Conn::GameStarting;
        return;
    }

    conn = Conn::Attaching;
    injecting_ = true;
    attachedId_ = m->manifest.id;
    injectWorker_ = std::thread([this, pid = pid_, dll = m->dir / Widen(m->manifest.dll)] {
        injectError_.clear();
        injectResult_ = injector::IsModuleLoaded(pid, dll.filename().c_str()) ? injector::Result::Ok
                                                                             : injector::Inject(pid, dll, injectError_);
        injecting_ = false;
    });
}

void App::OnGameFound(DWORD pid) {
    knownPid_ = pid;
    const InstalledModule* m = Active();
    const std::wstring path = injector::ProcessPath(pid);
    if (!m || path.empty()) return;
    // Remember where the game lives so "Launch game" works next time.
    if (fs::path(path) != gamePath) SetGamePath(path);
    const std::string exe = injector::FileVersion(path);
    gameVersion = "exe " + exe;
    gameVersionTested = false;
    for (const KnownGameVersion& v : m->manifest.gameVersions) {
        if (exe == v.exe) {
            gameVersion = v.patch;
            gameVersionTested = v.tested;
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Events from the mod

void App::ReadEvents(bool live) {
    const uint32_t seq = ggg::Load(shared->eventSeq);
    if (seq < lastEventSeq_) lastEventSeq_ = 0;  // the mapping was re-created
    if (seq - lastEventSeq_ > ggg::kMaxEvents) lastEventSeq_ = seq - ggg::kMaxEvents;
    for (uint32_t s = lastEventSeq_ + 1; s <= seq; ++s) {
        ggg::Event e;
        std::memcpy(&e, static_cast<const void*>(&shared->events[(s - 1) % ggg::kMaxEvents]), sizeof(e));
        e.text[sizeof(e.text) - 1] = '\0';
        if (e.seq == s) OnEvent(e, live);
    }
    lastEventSeq_ = seq;
}

void App::OnEvent(const ggg::Event& e, bool live) {
    SYSTEMTIME t;
    GetLocalTime(&t);
    char time[16];
    std::snprintf(time, sizeof(time), "%02u:%02u:%02u", t.wHour, t.wMinute, t.wSecond);
    feed.push_front({live ? time : "", e.type, e.text});
    while (feed.size() > 200) feed.pop_back();
    if (!live) return;

    const std::wstring sound = (dir / L"assets" / L"notify.wav").wstring();
    const std::wstring lasted = e.attemptSec > 0 ? L"  ·  attempt " + FormatTime(e.attemptSec) : L"";
    switch (e.type) {
        case ggg::EventType::PlayerDeath:
            overlay_.ShowToast(ToastKind::Death, L"YOU DIED",
                               L"Death #" + std::to_wstring(e.count) + lasted + L"  ·  HP restored");
            if (prefs.soundDeath) PlaySoundW(sound.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            break;
        case ggg::EventType::PlayerFell:
            overlay_.ShowToast(ToastKind::Death, L"YOU FELL",
                               L"Death #" + std::to_wstring(e.count) + lasted + L"  ·  back on solid ground");
            if (prefs.soundDeath) PlaySoundW(sound.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            break;
        case ggg::EventType::BossDefeated:
            overlay_.ShowToast(ToastKind::Defeat, L"BOSS DEFEATED",
                               L"Defeat #" + std::to_wstring(e.count) + lasted + L"  ·  boss HP restored");
            if (prefs.soundDefeat) PlaySoundW(sound.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            break;
        case ggg::EventType::Warning:
            overlay_.ShowToast(ToastKind::Warning, L"HEADS UP", Widen(e.text));
            break;
        default:
            break;
    }
}

void App::Persist() {
    ggg::Settings now = shared->settings;
    now.passThrough = 0;  // one-shot, never saved
    if (std::memcmp(&now, &savedSettings_, sizeof(now)) != 0) {
        ggg::WriteSettings(settingsIni_, now);
        savedSettings_ = now;
    }
    if (!(prefs == savedPrefs_)) {
        WriteIni(settingsIni_, L"app", L"toasts", prefs.toasts ? L"1" : L"0");
        WriteIni(settingsIni_, L"app", L"hud", prefs.hud ? L"1" : L"0");
        WriteIni(settingsIni_, L"app", L"sound_death", prefs.soundDeath ? L"1" : L"0");
        WriteIni(settingsIni_, L"app", L"sound_defeat", prefs.soundDefeat ? L"1" : L"0");
        savedPrefs_ = prefs;
    }
}
