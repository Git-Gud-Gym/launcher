#pragma once
#include <windows.h>

#include <atomic>
#include <chrono>
#include <deque>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "catalog.h"
#include "imgui.h"
#include "injector.h"
#include "modules.h"
#include "overlay.h"
#include "shared_state.h"

// App-only preferences (the gameplay toggles live in ggg::Settings, shared with the mod).
struct Prefs {
    bool toasts = true;
    bool hud = true;
    bool soundDeath = true;
    bool soundDefeat = true;
    bool operator==(const Prefs&) const = default;
};

enum class Page { Library, Game };
enum class Conn { NoGame, GameStarting, Attaching, Connected, Detached, NeedsAdmin, AntiCheat, Error };

struct FeedItem {
    std::string time;
    ggg::EventType type;
    std::string text;
};

struct Fonts {
    ImFont* body = nullptr;
    ImFont* semibold = nullptr;
    ImFont* note = nullptr;  // small print (not "small": that is a Windows macro)
    ImFont* heading = nullptr;
    ImFont* big = nullptr;
    ImFont* brand = nullptr;
    ImFont* title = nullptr;  // game names on library cards and page headers
};

// One entry in the library: a game that is installed, published in the catalog, or both.
struct LibraryEntry {
    std::string id, name, tagline;
    std::vector<std::string> features;
    const InstalledModule* installed = nullptr;
    const CatalogModule* published = nullptr;
    bool planned = false;
    bool updateAvailable = false;
};

class App {
public:
    bool Init(HINSTANCE instance, HWND window);
    void Shutdown();
    // Once per frame, before drawing: connection, events, downloads, overlay, persistence.
    void Tick();

    // Navigation
    Page page = Page::Library;
    void OpenLibrary() { page = Page::Library; }
    void OpenGame(const std::string& id);
    const InstalledModule* Active() const;  // the game shown on the game page and watched for attaching

    // Library
    std::vector<LibraryEntry> Library() const;
    void RefreshCatalog();
    void Install(const std::string& id);
    // Installs id as soon as the library has loaded (the --install command-line option).
    void InstallWhenReady(const std::string& id) { pendingInstall_ = id; }
    void Uninstall(const std::string& id);
    bool Installing(const std::string& id) const { return installing_ && installId_ == id; }
    float installProgress = 0;  // 0..1 for the module in Installing()
    std::string libraryMessage;  // last install/uninstall outcome
    bool CatalogLoading() const { return catalogLoading_; }
    std::string catalogStatus;
    std::string catalogError;  // details for the status tooltip
    bool AppUpdateAvailable() const;
    std::string AppUpdateUrl() const;

    // Game page actions
    void SendCommand(ggg::Command cmd);
    void PrimaryAction();  // the game header's main button
    void LaunchGame();
    void BrowseForGame();
    void DetectGame();
    void OpenDataFolder() const;
    void OpenLog() const;

    // Read by the UI
    std::vector<InstalledModule> installed;
    std::optional<Catalog> catalog;
    ggg::SharedState* shared = nullptr;
    ggg::Status status{};
    bool connected = false;
    Conn conn = Conn::NoGame;
    std::string connDetail;
    std::string gameVersion;  // e.g. "1.12.0", or the raw exe version if unknown
    bool gameVersionTested = true;
    std::filesystem::path gamePath;  // the active game's exe
    bool gameFound = false;
    std::string locateMessage;
    Prefs prefs;
    std::deque<FeedItem> feed;
    Fonts fonts;
    ImTextureID logo = 0;
    float scale = 1.0f;
    HWND window = nullptr;
    std::filesystem::path dir;  // where GitGudGym.exe lives

private:
    void ScanModules();
    void LoadActiveGame();
    void SetGamePath(const std::filesystem::path& exe);
    void UpdateConnection();
    void OnGameFound(DWORD pid);
    void ReadEvents(bool live);
    void OnEvent(const ggg::Event& e, bool live);
    void FinishBackgroundWork();
    void Persist();
    bool ModuleInUse(const std::string& id) const;

    std::filesystem::path settingsIni_;
    std::string activeId_;
    HANDLE mapping_ = nullptr;
    Overlay overlay_;
    uint32_t lastEventSeq_ = 0;

    // Attaching
    DWORD pid_ = 0;
    DWORD knownPid_ = 0;      // the game process we last read the path/version of
    std::string attachedId_;  // module we loaded into the running game
    HWND gameWindow_ = nullptr;
    bool autoAttach_ = true;
    DWORD failedPid_ = 0;
    std::chrono::steady_clock::time_point lastProcessCheck_{}, attachStarted_{};
    std::thread injectWorker_;
    std::atomic<bool> injecting_{false};
    injector::Result injectResult_ = injector::Result::Ok;
    std::string injectError_;

    // Catalog + installs: background threads hand their results over under mu_.
    std::mutex mu_;
    std::thread catalogWorker_, installWorker_;
    std::atomic<bool> catalogLoading_{false}, installing_{false};
    std::optional<Catalog> fetchedCatalog_;
    std::string fetchedStatus_, installId_, installResult_;
    bool installDone_ = false;
    std::string pendingInstall_;
    std::atomic<float> installProgressShared_{0};

    ggg::Settings savedSettings_;
    Prefs savedPrefs_;
};
