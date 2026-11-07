#pragma once
#include <windows.h>

#include <atomic>
#include <chrono>
#include <deque>
#include <filesystem>
#include <string>
#include <thread>

#include "../common/shared_state.h"
#include "imgui.h"
#include "injector.h"
#include "overlay.h"

// App-only preferences (the gameplay toggles live in ggg::Settings, shared with the mod).
struct Prefs {
    bool toasts = true;
    bool hud = true;
    bool soundDeath = true;
    bool soundDefeat = true;
    bool operator==(const Prefs&) const = default;
};

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
};

class App {
public:
    bool Init(HINSTANCE instance, HWND window);
    void Shutdown();
    // Once per frame, before drawing: connection, events, overlay, persistence.
    void Tick();

    // UI actions
    void SendCommand(ggg::Command cmd);
    void PrimaryAction();  // the game card's button
    void OpenStatsFolder() const;
    void OpenLog() const;

    // Read by the UI
    ggg::SharedState* shared = nullptr;
    ggg::Status status{};
    bool connected = false;
    Conn conn = Conn::NoGame;
    std::string connDetail;
    std::string gameVersion;     // e.g. "1.12.0", or the raw exe version if unknown
    bool gameVersionTested = true;
    Prefs prefs;
    std::deque<FeedItem> feed;
    Fonts fonts;
    ImTextureID logo = 0;
    float scale = 1.0f;
    HWND window = nullptr;
    std::filesystem::path dir;

private:
    void UpdateConnection();
    void ReadEvents(bool live);
    void OnEvent(const ggg::Event& e, bool live);
    void Persist();
    void OnGameFound(DWORD pid);

    std::filesystem::path ini_, dll_, gamePath_;
    HANDLE mapping_ = nullptr;
    Overlay overlay_;
    uint32_t lastEventSeq_ = 0;

    DWORD pid_ = 0;
    DWORD knownPid_ = 0;  // the game process we last read the path/version of
    HWND gameWindow_ = nullptr;
    bool autoAttach_ = true;
    DWORD failedPid_ = 0;
    std::chrono::steady_clock::time_point lastProcessCheck_{}, attachStarted_{};
    std::thread worker_;
    std::atomic<bool> injecting_{false};
    injector::Result injectResult_ = injector::Result::Ok;
    std::string injectError_;

    ggg::Settings savedSettings_;
    Prefs savedPrefs_;
};
