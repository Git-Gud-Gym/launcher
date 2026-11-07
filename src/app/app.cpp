#include "app.h"

#include <mmsystem.h>
#include <shellapi.h>

#include <cstdio>
#include <cstring>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

namespace {

constexpr const wchar_t* kGameExe = L"eldenring.exe";
constexpr const wchar_t* kDllName = L"gitgudgym.dll";
constexpr double kGameWarmupSec = 20;  // let the game finish unpacking before injecting
constexpr const wchar_t* kDefaultGamePath =
    L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\ELDEN RING\\Game\\eldenring.exe";

// eldenring.exe file version -> game patch. Only versions the offsets were verified against are "tested".
struct KnownVersion {
    const char* exe;
    const char* patch;
    bool tested;
};
constexpr KnownVersion kKnownVersions[] = {
    {"2.2.0.0", "1.12.0", true},
};

// Never attach while Easy Anti-Cheat is running: the game must be started offline without it.
bool AntiCheatRunning() {
    for (const wchar_t* name : {L"EasyAntiCheat_EOS.exe", L"EasyAntiCheat.exe", L"start_protected_game.exe"}) {
        if (injector::FindProcess(name)) return true;
    }
    return false;
}

bool ReadBool(const fs::path& ini, const wchar_t* key, bool def) {
    return GetPrivateProfileIntW(L"app", key, def ? 1 : 0, ini.c_str()) != 0;
}

void WriteBool(const fs::path& ini, const wchar_t* key, bool v) {
    WritePrivateProfileStringW(L"app", key, v ? L"1" : L"0", ini.c_str());
}

std::wstring Widen(const std::string& s) {
    std::wstring w(s.size(), L'\0');
    w.resize(MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), static_cast<int>(w.size())));
    return w;
}

std::wstring FormatTime(float sec) {
    const int s = static_cast<int>(sec);
    wchar_t buf[32];
    swprintf_s(buf, L"%d:%02d", s / 60, s % 60);
    return buf;
}

}  // namespace

bool App::Init(HINSTANCE instance, HWND hwnd) {
    window = hwnd;
    wchar_t self[MAX_PATH];
    GetModuleFileNameW(nullptr, self, MAX_PATH);
    dir = fs::path(self).parent_path();
    ini_ = dir / L"gitgudgym.ini";
    dll_ = dir / kDllName;

    wchar_t game[MAX_PATH]{};
    GetPrivateProfileStringW(L"app", L"game_path", kDefaultGamePath, game, MAX_PATH, ini_.c_str());
    gamePath_ = game;

    prefs.toasts = ReadBool(ini_, L"toasts", true);
    prefs.hud = ReadBool(ini_, L"hud", true);
    prefs.soundDeath = ReadBool(ini_, L"sound_death", true);
    prefs.soundDefeat = ReadBool(ini_, L"sound_defeat", true);
    savedPrefs_ = prefs;

    bool created = false;
    shared = ggg::OpenShared(mapping_, created);
    if (!shared) return false;
    // If the mod is already running it owns the live settings; otherwise start from the saved ones.
    if (created) shared->settings = ggg::ReadSettings(ini_);
    savedSettings_ = shared->settings;
    savedSettings_.passThrough = 0;

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
    if (worker_.joinable()) worker_.join();
    overlay_.Shutdown();
    if (shared) UnmapViewOfFile(shared);
    if (mapping_) CloseHandle(mapping_);
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
            ShellExecuteW(nullptr, L"open", gamePath_.c_str(), nullptr, gamePath_.parent_path().c_str(), SW_SHOWNORMAL);
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

void App::OpenStatsFolder() const {
    const fs::path stats = dir / L"stats";
    std::error_code ec;
    fs::create_directories(stats, ec);
    ShellExecuteW(nullptr, L"open", stats.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void App::OpenLog() const {
    ShellExecuteW(nullptr, L"open", (dir / L"gitgudgym.log").c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void App::Tick() {
    std::memcpy(&status, static_cast<const void*>(&shared->status), sizeof(status));
    connected = status.running && GetTickCount64() - status.heartbeat < 2000;
    UpdateConnection();
    ReadEvents(true);

    const bool gameFocused = gameWindow_ && GetForegroundWindow() == gameWindow_;
    overlay_.Update(connected ? gameWindow_ : nullptr, gameFocused, status, prefs.hud, prefs.toasts);
    Persist();
}

void App::UpdateConnection() {
    if (worker_.joinable() && !injecting_) {
        worker_.join();
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
    if (Clock::now() - lastProcessCheck_ < 500ms) return;
    lastProcessCheck_ = Clock::now();

    pid_ = injector::FindProcess(kGameExe);
    gameWindow_ = pid_ ? injector::FindMainWindow(pid_) : nullptr;
    if (!pid_) {
        conn = Conn::NoGame;
        failedPid_ = 0;
        autoAttach_ = true;  // the next launch attaches again
        return;
    }
    if (pid_ != knownPid_) OnGameFound(pid_);
    if (failedPid_ == pid_) return;  // stay in NeedsAdmin / Error until the user acts
    if (AntiCheatRunning()) {
        conn = Conn::AntiCheat;
        return;
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
    worker_ = std::thread([this, pid = pid_] {
        injectError_.clear();
        injectResult_ = injector::IsModuleLoaded(pid, kDllName) ? injector::Result::Ok
                                                                 : injector::Inject(pid, dll_, injectError_);
        injecting_ = false;
    });
}

void App::OnGameFound(DWORD pid) {
    knownPid_ = pid;
    const std::wstring path = injector::ProcessPath(pid);
    if (path.empty()) return;
    // Remember where the game lives so "Launch game" works next time.
    if (fs::path(path) != gamePath_) {
        gamePath_ = path;
        WritePrivateProfileStringW(L"app", L"game_path", path.c_str(), ini_.c_str());
    }
    const std::string exe = injector::FileVersion(path);
    gameVersion = "exe " + exe;
    gameVersionTested = false;
    for (const KnownVersion& v : kKnownVersions) {
        if (exe == v.exe) {
            gameVersion = v.patch;
            gameVersionTested = v.tested;
        }
    }
}

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
    const std::wstring lasted = e.attemptSec > 0 ? L"  \u00B7  attempt " + FormatTime(e.attemptSec) : L"";
    switch (e.type) {
        case ggg::EventType::PlayerDeath:
            overlay_.ShowToast(ToastKind::Death, L"YOU DIED",
                               L"Death #" + std::to_wstring(e.count) + lasted + L"  \u00B7  HP restored");
            if (prefs.soundDeath) PlaySoundW(sound.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            break;
        case ggg::EventType::PlayerFell:
            overlay_.ShowToast(ToastKind::Death, L"YOU FELL",
                               L"Death #" + std::to_wstring(e.count) + lasted + L"  \u00B7  back on solid ground");
            if (prefs.soundDeath) PlaySoundW(sound.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            break;
        case ggg::EventType::BossDefeated:
            overlay_.ShowToast(ToastKind::Defeat, L"BOSS DEFEATED",
                               L"Defeat #" + std::to_wstring(e.count) + lasted + L"  \u00B7  boss HP restored");
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
        ggg::WriteSettings(ini_, now);
        savedSettings_ = now;
    }
    if (!(prefs == savedPrefs_)) {
        WriteBool(ini_, L"toasts", prefs.toasts);
        WriteBool(ini_, L"hud", prefs.hud);
        WriteBool(ini_, L"sound_death", prefs.soundDeath);
        WriteBool(ini_, L"sound_defeat", prefs.soundDefeat);
        savedPrefs_ = prefs;
    }
}
