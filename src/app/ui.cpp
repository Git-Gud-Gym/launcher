#include "ui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "imgui_internal.h"
#include "version.h"

namespace {

// Palette taken from the logo: near-black with antique gold.
const ImVec4 kBg = ImVec4(0.047f, 0.051f, 0.071f, 1.0f);       // #0C0D12
const ImVec4 kSidebar = ImVec4(0.063f, 0.067f, 0.090f, 1.0f);  // #101117
const ImVec4 kPanel = ImVec4(0.082f, 0.090f, 0.118f, 1.0f);    // #15171E
const ImVec4 kPanelHi = ImVec4(0.110f, 0.118f, 0.153f, 1.0f);  // #1C1E27
const ImVec4 kBorder = ImVec4(0.153f, 0.165f, 0.208f, 1.0f);   // #272A35
const ImVec4 kGold = ImVec4(0.831f, 0.659f, 0.294f, 1.0f);     // #D4A84B
const ImVec4 kGoldHi = ImVec4(0.902f, 0.749f, 0.396f, 1.0f);
const ImVec4 kRed = ImVec4(0.898f, 0.282f, 0.302f, 1.0f);      // #E5484D
const ImVec4 kGreen = ImVec4(0.275f, 0.655f, 0.345f, 1.0f);    // #46A758
const ImVec4 kAmber = ImVec4(0.941f, 0.667f, 0.235f, 1.0f);
const ImVec4 kText = ImVec4(0.925f, 0.918f, 0.902f, 1.0f);
const ImVec4 kMuted = ImVec4(0.541f, 0.565f, 0.612f, 1.0f);

ImU32 U32(const ImVec4& c, float alpha = 1.0f) { return ImGui::GetColorU32(ImVec4(c.x, c.y, c.z, c.w * alpha)); }

float g_scale = 1.0f;  // DPI scale, set by ApplyTheme

float S(float v) { return v * g_scale; }

std::string Thousands(long long v) {
    std::string s = std::to_string(v < 0 ? -v : v);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(i, ",");
    return v < 0 ? "-" + s : s;
}

std::string FormatTime(float sec) {
    if (sec < 0) return "-";
    char buf[32];
    const int s = static_cast<int>(sec);
    std::snprintf(buf, sizeof(buf), "%d:%02d", s / 60, s % 60);
    return buf;
}

// Smoothly animated 0..1 value stored per widget id.
float Animate(ImGuiID id, float target, float speed = 14.0f) {
    ImGuiStorage* st = ImGui::GetStateStorage();
    float v = st->GetFloat(id, target);
    v += (target - v) * std::min(1.0f, ImGui::GetIO().DeltaTime * speed);
    st->SetFloat(id, v);
    return v;
}

bool Toggle(const char* id, bool* value) {
    const float h = S(24), w = S(44);
    const ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(id, ImVec2(w, h));
    const bool clicked = ImGui::IsItemClicked();
    if (clicked) *value = !*value;
    const bool hovered = ImGui::IsItemHovered();
    if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    const float t = Animate(ImGui::GetItemID(), *value ? 1.0f : 0.0f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec4 off = hovered ? ImVec4(0.22f, 0.235f, 0.29f, 1) : ImVec4(0.18f, 0.19f, 0.235f, 1);
    const ImVec4 track = ImLerp(off, kGold, t);
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), U32(track), h * 0.5f);
    const float r = h * 0.5f - S(3);
    const ImVec2 knob(p.x + h * 0.5f + t * (w - h), p.y + h * 0.5f);
    dl->AddCircleFilled(knob, r, U32(ImLerp(kMuted, kBg, t)));
    return clicked;
}

void KeyChip(const char* key) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 sz = ImGui::CalcTextSize(key);
    const ImVec2 pad(S(7), S(3));
    const ImVec2 p = ImGui::GetCursorScreenPos();
    dl->AddRectFilled(p, ImVec2(p.x + sz.x + pad.x * 2, p.y + sz.y + pad.y * 2), U32(kPanelHi), S(5));
    dl->AddRect(p, ImVec2(p.x + sz.x + pad.x * 2, p.y + sz.y + pad.y * 2), U32(kBorder), S(5));
    dl->AddText(ImVec2(p.x + pad.x, p.y + pad.y), U32(kMuted), key);
    ImGui::Dummy(ImVec2(sz.x + pad.x * 2, sz.y + pad.y * 2));
}

// A setting row: title and description on the left, hotkey chip and switch on the right.
bool ToggleRow(App& app, const char* title, const char* desc, const char* key, bool* value) {
    ImGui::PushID(title);
    const float right = S(44) + (key ? S(46) : 0);
    const float x0 = ImGui::GetCursorPosX();
    const float width = ImGui::GetContentRegionAvail().x;
    const float y0 = ImGui::GetCursorPosY();

    ImGui::PushFont(app.fonts.semibold);
    ImGui::TextUnformatted(title);
    ImGui::PopFont();
    if (desc) {
        ImGui::PushFont(app.fonts.note);
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::PushTextWrapPos(x0 + width - right - S(12));
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    const float y1 = ImGui::GetCursorPosY();

    ImGui::SetCursorPos(ImVec2(x0 + width - right, y0 + (desc ? S(4) : -S(2))));
    bool changed = false;
    if (key) {
        ImGui::PushFont(app.fonts.note);
        KeyChip(key);
        ImGui::PopFont();
        ImGui::SameLine(x0 + width - S(44));
    }
    changed = Toggle("##switch", value);
    ImGui::SetCursorPos(ImVec2(x0, std::max(y1, y0 + S(26)) + S(4)));
    ImGui::PopID();
    return changed;
}

bool ToggleRow(App& app, const char* title, const char* desc, const char* key, int32_t* shared) {
    bool v = ggg::Load(*shared) != 0;
    if (!ToggleRow(app, title, desc, key, &v)) return false;
    ggg::Store(*shared, static_cast<int32_t>(v));
    return true;
}

bool BeginCard(const char* id, const char* title, App& app, float height = 0) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanel);
    ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(18), S(14)));
    const ImGuiChildFlags flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding |
                                  (height == 0 ? ImGuiChildFlags_AutoResizeY : 0);
    const bool open = ImGui::BeginChild(id, ImVec2(0, height), flags,
                                        height == 0 ? ImGuiWindowFlags_NoScrollbar : 0);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    if (title) {
        ImGui::PushFont(app.fonts.heading);
        ImGui::TextUnformatted(title);
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(0, S(2)));
    }
    return open;
}

void EndCard() {
    ImGui::EndChild();
    ImGui::Dummy(ImVec2(0, S(6)));
}

bool Button(const char* label, bool primary, float width = 0, bool enabled = true) {
    ImGui::BeginDisabled(!enabled);
    ImGui::PushStyleColor(ImGuiCol_Button, primary ? kGold : kPanelHi);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, primary ? kGoldHi : ImVec4(0.15f, 0.16f, 0.2f, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, primary ? kGold : kBorder);
    ImGui::PushStyleColor(ImGuiCol_Text, primary ? kBg : kText);
    const bool clicked = ImGui::Button(label, ImVec2(width, S(38)));
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::PopStyleColor(4);
    ImGui::EndDisabled();
    return clicked;
}

void Bar(float fraction, const ImVec4& color, const char* overlay, float height) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float w = ImGui::GetContentRegionAvail().x;
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + height), U32(kPanelHi), S(4));
    if (fraction > 0) {
        dl->AddRectFilled(p, ImVec2(p.x + w * std::clamp(fraction, 0.0f, 1.0f), p.y + height), U32(color), S(4));
    }
    if (overlay) {
        const ImVec2 ts = ImGui::CalcTextSize(overlay);
        dl->AddText(ImVec2(p.x + S(8), p.y + (height - ts.y) * 0.5f), U32(kText), overlay);
    }
    ImGui::Dummy(ImVec2(w, height));
}

void SmallLabel(App& app, const char* text, const ImVec4& color = kMuted) {
    ImGui::PushFont(app.fonts.note);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

void StatTile(App& app, const char* id, const char* label, const std::string& value, const ImVec4& color,
              const std::string& sub, float width) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanel);
    ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(18), S(10)));
    ImGui::BeginChild(id, ImVec2(width, S(96)), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
                      ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    SmallLabel(app, label);
    ImGui::PushFont(app.fonts.big);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(value.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
    SmallLabel(app, sub.c_str());
    ImGui::EndChild();
}

void TitleBar(App& app) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float h = S(kTitleBarHeight);
    const float w = ImGui::GetWindowWidth();
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(w, h), U32(kSidebar));
    dl->AddLine(ImVec2(0, h - 1), ImVec2(w, h - 1), U32(kBorder));

    float x = S(16);
    if (app.logo) {
        dl->AddImage(app.logo, ImVec2(x, S(10)), ImVec2(x + S(32), S(42)));
        x += S(44);
    }
    ImGui::PushFont(app.fonts.brand);
    const float brandY = (h - ImGui::GetFontSize()) * 0.5f;
    dl->AddText(ImVec2(x, brandY), U32(kGold), "GIT GUD GYM");
    const float brandW = ImGui::CalcTextSize("GIT GUD GYM").x;
    ImGui::PopFont();
    ImGui::PushFont(app.fonts.note);
    dl->AddText(ImVec2(x + brandW + S(12), (h - ImGui::GetFontSize()) * 0.5f + S(1)), U32(kMuted),
                "boss training  \u00B7  v" GGG_VERSION);
    ImGui::PopFont();

    // Window buttons
    const float bw = S(kTitleButtonsWidth) / 2;
    auto button = [&](const char* id, float bx, bool close) {
        ImGui::SetCursorScreenPos(ImVec2(bx, 0));
        const bool clicked = ImGui::InvisibleButton(id, ImVec2(bw, h - 1));
        const bool hovered = ImGui::IsItemHovered();
        if (hovered) dl->AddRectFilled(ImVec2(bx, 0), ImVec2(bx + bw, h - 1), U32(close ? kRed : kPanelHi));
        const ImVec2 c(bx + bw * 0.5f, h * 0.5f);
        const float r = S(5);
        const ImU32 col = U32(hovered ? kText : kMuted);
        if (close) {
            dl->AddLine(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), col, S(1.5f));
            dl->AddLine(ImVec2(c.x - r, c.y + r), ImVec2(c.x + r, c.y - r), col, S(1.5f));
        } else {
            dl->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x + r, c.y), col, S(1.5f));
        }
        return clicked;
    };
    if (button("##min", w - 2 * bw, false)) ShowWindow(app.window, SW_MINIMIZE);
    if (button("##close", w - bw, true)) PostMessageW(app.window, WM_CLOSE, 0, 0);
}

void GameCard(App& app) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanel);
    ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(16), S(16)));
    ImGui::BeginChild("game", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding |
                                               ImGuiChildFlags_AutoResizeY);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    ImGui::PushFont(app.fonts.heading);
    ImGui::TextUnformatted("ELDEN RING");
    ImGui::PopFont();

    struct View {
        const char* status;
        ImVec4 color;
        const char* detail;
        const char* button;
        bool primary;
    };
    View v{};
    switch (app.conn) {
        case Conn::NoGame:
            v = app.gameFound
                    ? View{"Not running", kMuted, "Launch the game here or start it yourself; the mod attaches automatically.",
                           "Launch game", true}
                    : View{"Game not found", kAmber, "Point Git Gud Gym at eldenring.exe to launch from here.",
                           "Locate game", true};
            break;
        case Conn::GameStarting:
            v = {"Game starting", kAmber, "Waiting for the game to finish loading...", nullptr, false};
            break;
        case Conn::Attaching:
            v = {"Attaching", kAmber, "Loading the mod into the game...", nullptr, false};
            break;
        case Conn::Connected:
            v = {"Active", kGreen, "Mods are live. Closing this app turns them off.", "Unload mod", false};
            break;
        case Conn::Detached:
            v = {"Unloaded", kMuted, "The game is running without the mod.", "Attach mod", true};
            break;
        case Conn::NeedsAdmin:
            v = {"Needs admin", kAmber,
                 "The game runs as administrator, so this app must too. (Or untick \"Run as administrator\" in "
                 "eldenring.exe's compatibility settings.)",
                 "Restart as admin", true};
            break;
        case Conn::AntiCheat:
            v = {"Anti-cheat running", kRed,
                 "Easy Anti-Cheat is active, so the mod won't attach. Restart the game offline without "
                 "anti-cheat (see the README).",
                 nullptr, false};
            break;
        case Conn::Error:
            v = {"Couldn't attach", kRed, app.connDetail.c_str(), "Retry", true};
            break;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float pulse = app.conn == Conn::Connected ? 0.6f + 0.4f * std::sin(static_cast<float>(ImGui::GetTime()) * 3) : 1;
    dl->AddCircleFilled(ImVec2(p.x + S(5), p.y + ImGui::GetFontSize() * 0.5f + S(1)), S(4.5f), U32(v.color, pulse));
    ImGui::SetCursorScreenPos(ImVec2(p.x + S(16), p.y));
    ImGui::PushFont(app.fonts.semibold);
    ImGui::PushStyleColor(ImGuiCol_Text, v.color);
    ImGui::TextUnformatted(v.status);
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::PushFont(app.fonts.note);
    ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
    ImGui::PushTextWrapPos(0);
    ImGui::TextUnformatted(v.detail);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
    ImGui::PopFont();

    if (!app.gameVersion.empty() && app.conn != Conn::NoGame) {
        const std::string line = "Game " + app.gameVersion +
                                 (app.gameVersionTested ? "  \u00B7  tested" : "  \u00B7  untested version, see README");
        SmallLabel(app, line.c_str(), app.gameVersionTested ? kMuted : kAmber);
    }

    if (v.button) {
        ImGui::Dummy(ImVec2(0, S(4)));
        if (Button(v.button, v.primary, -FLT_MIN)) app.PrimaryAction();
    }
    ImGui::EndChild();
}

// Where eldenring.exe is, with ways to change it.
void GameLocation(App& app) {
    SmallLabel(app, "GAME LOCATION");
    ImGui::PushFont(app.fonts.note);
    ImGui::PushTextWrapPos(0);
    if (app.gameFound) {
        // The interesting part is the folder; show it without the exe name.
        const std::string folder = app.gamePath.parent_path().string();
        ImGui::TextUnformatted(folder.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", app.gamePath.string().c_str());
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, kAmber);
        ImGui::TextUnformatted("Not set");
        ImGui::PopStyleColor();
    }
    if (!app.locateMessage.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TextUnformatted(app.locateMessage.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::PopTextWrapPos();
    ImGui::PopFont();

    const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2;
    if (Button("Browse...", false, half)) app.BrowseForGame();
    ImGui::SameLine();
    if (Button("Auto-detect", false, half)) app.DetectGame();
}

void Sidebar(App& app) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kSidebar);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(18), S(18)));
    ImGui::BeginChild("sidebar", ImVec2(S(270), 0), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    SmallLabel(app, "GAME");
    GameCard(app);

    ImGui::Dummy(ImVec2(0, S(10)));
    GameLocation(app);

    ImGui::Dummy(ImVec2(0, S(10)));
    SmallLabel(app, "HOTKEYS (IN GAME)");
    ImGui::Dummy(ImVec2(0, S(2)));
    const char* keys[][2] = {{"F5", "Immortality on / off"}, {"F6", "Next boss"},     {"F7", "Let boss die once"},
                             {"F8", "Probe (writes log)"},   {"F9", "New session"},   {"F10", "Unload mod"}};
    for (const auto& k : keys) {
        ImGui::PushFont(app.fonts.note);
        KeyChip(k[0]);
        ImGui::PopFont();
        ImGui::SameLine(S(62));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + S(2));
        ImGui::TextUnformatted(k[1]);
    }

    const float bottom = ImGui::GetWindowHeight() - S(18) - S(38) * 2 - ImGui::GetStyle().ItemSpacing.y;
    if (ImGui::GetCursorPosY() < bottom) ImGui::SetCursorPosY(bottom);
    if (Button("Open stats folder", false, -FLT_MIN)) app.OpenStatsFolder();
    if (Button("Open log", false, -FLT_MIN)) app.OpenLog();
    ImGui::EndChild();
}

void LiveCard(App& app) {
    const ggg::Status& s = app.status;
    BeginCard("live", "Live fight", app);
    char buf[160];

    // Boss
    ImGui::PushFont(app.fonts.semibold);
    ImGui::TextUnformatted("Boss");
    ImGui::PopFont();
    if (s.bossTracked) {
        ImGui::SameLine();
        std::snprintf(buf, sizeof(buf), "%s  ·  %.0f m away", s.bossManual ? "picked manually" : "auto-detected",
                      s.bossDistance);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + S(2));
        SmallLabel(app, buf);
        std::snprintf(buf, sizeof(buf), "%s / %s  (%.0f%%)", Thousands(s.bossHp).c_str(), Thousands(s.bossMaxHp).c_str(),
                      100.0f * s.bossHp / std::max(1, s.bossMaxHp));
        Bar(static_cast<float>(s.bossHp) / std::max(1, s.bossMaxHp), kGold, buf, S(22));
    } else {
        Bar(0, kGold, app.connected ? "No boss in range. Walk into an arena." : "Not attached", S(22));
    }

    // Player
    ImGui::PushFont(app.fonts.semibold);
    ImGui::TextUnformatted("You");
    ImGui::PopFont();
    if (s.playerLoaded) {
        const float f = static_cast<float>(s.playerHp) / std::max(1, s.playerMaxHp);
        std::snprintf(buf, sizeof(buf), "%s / %s", Thousands(s.playerHp).c_str(), Thousands(s.playerMaxHp).c_str());
        Bar(f, f < 0.3f ? kRed : kGreen, buf, S(22));
    } else {
        Bar(0, kGreen, app.connected ? "Load into the world" : "Not attached", S(22));
    }
    ImGui::Dummy(ImVec2(0, S(2)));

    // Current attempt
    if (s.attemptActive) {
        std::snprintf(buf, sizeof(buf), "Attempt #%d  ·  %s", s.attemptIndex, FormatTime(s.attemptSec).c_str());
        ImGui::PushFont(app.fonts.semibold);
        ImGui::TextUnformatted(buf);
        ImGui::PopFont();
        std::snprintf(buf, sizeof(buf), "Dealt %s  ·  Taken %s  ·  Hits %d / %d  ·  Boss low %.0f%%",
                      Thousands(s.attemptDealt).c_str(), Thousands(s.attemptTaken).c_str(), s.attemptHitsLanded,
                      s.attemptHitsTaken, s.attemptBossLowPct);
        SmallLabel(app, buf);
    } else {
        std::snprintf(buf, sizeof(buf), "Attempt #%d starts on the first hit", s.attemptIndex ? s.attemptIndex : 1);
        ImGui::PushFont(app.fonts.semibold);
        ImGui::TextUnformatted(buf);
        ImGui::PopFont();
        SmallLabel(app, "Attempts end on a death, a boss defeat, or leaving the fight.");
    }
    if (Button("Next boss  (F6)", false, 0, app.connected)) app.SendCommand(ggg::Command::NextBoss);
    EndCard();
}

void SessionCard(App& app) {
    const ggg::Status& s = app.status;
    BeginCard("session", "Session", app);
    struct Cell {
        const char* label;
        std::string value;
    } cells[] = {
        {"DAMAGE DEALT", Thousands(s.damageDealt)},     {"DAMAGE TAKEN", Thousands(s.damageTaken)},
        {"HITS LANDED", Thousands(s.hitsLanded)},  {"HITS TAKEN", Thousands(s.hitsTaken)},
        {"HEALING", Thousands(s.healing)},              {"FALLS", std::to_string(s.falls)},
        {"FASTEST DEFEAT", FormatTime(s.fastestDefeatSec)}, {"AVG ATTEMPT", s.attempts ? FormatTime(s.avgAttemptSec) : "-"},
    };
    if (ImGui::BeginTable("cells", 4)) {
        for (const auto& c : cells) {
            ImGui::TableNextColumn();
            SmallLabel(app, c.label);
            ImGui::PushFont(app.fonts.heading);
            ImGui::TextUnformatted(c.value.c_str());
            ImGui::PopFont();
        }
        ImGui::EndTable();
    }
    if (Button("New session  (F9)", false, 0, app.connected)) app.SendCommand(ggg::Command::NewSession);
    ImGui::SameLine();
    if (Button("Probe  (F8)", false, 0, app.connected)) app.SendCommand(ggg::Command::Probe);
    EndCard();
}

void ActivityCard(App& app) {
    // Fill whatever is left of the column so it never needs to scroll.
    BeginCard("activity", "Activity", app, std::max(ImGui::GetContentRegionAvail().y - S(6), S(90)));
    if (app.feed.empty()) SmallLabel(app, "Deaths, defeats and boss changes show up here.");
    for (const FeedItem& f : app.feed) {
        ImVec4 dot = kMuted;
        switch (f.type) {
            case ggg::EventType::PlayerDeath:
            case ggg::EventType::PlayerFell: dot = kRed; break;
            case ggg::EventType::BossDefeated: dot = kGold; break;
            case ggg::EventType::Warning: dot = kAmber; break;
            case ggg::EventType::BossTracked: dot = kGreen; break;
            default: break;
        }
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(p.x + S(4), p.y + ImGui::GetFontSize() * 0.55f), S(3.5f),
                                                    U32(dot));
        ImGui::SetCursorScreenPos(ImVec2(p.x + S(16), p.y));
        if (!f.time.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
            ImGui::TextUnformatted(f.time.c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::PushTextWrapPos(0);
        ImGui::TextUnformatted(f.text.c_str());
        ImGui::PopTextWrapPos();
    }
    EndCard();
}

void ModsCard(App& app) {
    ggg::Settings& set = app.shared->settings;
    BeginCard("mods", "Training mods", app);
    if (!app.connected) {
        SmallLabel(app, "Changes apply as soon as the mod attaches.", kAmber);
        ImGui::Dummy(ImVec2(0, S(2)));
    }
    ToggleRow(app, "Player can't die", "Lethal hits refill your HP and count as a death.", "F5",
              &set.playerImmortal);
    ToggleRow(app, "Boss can't die", "The boss refills at 0 HP and it counts as a defeat. Its phase is kept.",
              "F5", &set.bossImmortal);
    ToggleRow(app, "Falling counts as death",
              "Fall too far below solid ground and you respawn there. Counts as a death.",
              nullptr, &set.fallDeath);
    if (ggg::Load(set.fallDeath)) {
        float height = ggg::Load(set.fallHeight);
        ImGui::PushFont(app.fonts.note);
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::SliderFloat("##fall", &height, 8.0f, 60.0f, "Fall distance: %.0f m")) {
            ggg::Store(set.fallHeight, height);
        }
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(0, S(2)));
    }
    ToggleRow(app, "Let boss die once",
              "For phase changes that need a real death (Rennala, Radagon).", "F7",
              &set.passThrough);
    EndCard();
}

void OverlayCard(App& app) {
    BeginCard("overlay", "Overlay & sound", app);
    ToggleRow(app, "On-screen notifications",
              "Pop-ups on deaths and defeats. Borderless or windowed mode only.", nullptr,
              &app.prefs.toasts);
    ToggleRow(app, "Mini HUD", "Counters and boss HP in the corner of the game.", nullptr,
              &app.prefs.hud);
    ToggleRow(app, "Sound on death", nullptr, nullptr, &app.prefs.soundDeath);
    ToggleRow(app, "Sound on boss defeat", nullptr, nullptr, &app.prefs.soundDefeat);
    EndCard();
}

void Main(App& app) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(24), S(20)));
    ImGui::BeginChild("main", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    const ggg::Status& s = app.status;
    const float gap = ImGui::GetStyle().ItemSpacing.x;
    const float tileW = (ImGui::GetContentRegionAvail().x - 3 * gap) / 4;
    StatTile(app, "deaths", "DEATHS", std::to_string(s.deaths), kRed,
             s.falls ? std::to_string(s.falls) + " from falls" : "this session", tileW);
    ImGui::SameLine();
    StatTile(app, "defeats", "BOSS DEFEATS", std::to_string(s.defeats), kGold,
             s.fastestDefeatSec >= 0 ? "fastest " + FormatTime(s.fastestDefeatSec) : "this session", tileW);
    ImGui::SameLine();
    StatTile(app, "attempts", "ATTEMPTS", std::to_string(s.attempts), kText,
             s.attempts ? "avg " + FormatTime(s.avgAttemptSec) : "this session", tileW);
    ImGui::SameLine();
    char best[16];
    std::snprintf(best, sizeof(best), "%.0f%%", s.bestBossHpPct);
    StatTile(app, "best", "BEST BOSS HP", s.attempts || s.attemptActive ? best : "-", kGoldHi,
             "lowest the boss has been", tileW);
    ImGui::Dummy(ImVec2(0, S(4)));

    const float leftW = (ImGui::GetContentRegionAvail().x - gap) * 0.56f;
    ImGui::BeginChild("left", ImVec2(leftW, 0), 0, ImGuiWindowFlags_NoScrollbar);
    LiveCard(app);
    SessionCard(app);
    ActivityCard(app);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right", ImVec2(0, 0));
    ModsCard(app);
    OverlayCard(app);
    ImGui::EndChild();

    ImGui::EndChild();
}

}  // namespace

void ApplyTheme(float scale) {
    ImGuiStyle& st = ImGui::GetStyle();
    st = ImGuiStyle();
    st.WindowRounding = 0;
    st.ChildRounding = 12;
    st.FrameRounding = 8;
    st.GrabRounding = 8;
    st.PopupRounding = 8;
    st.ScrollbarRounding = 8;
    st.WindowBorderSize = 0;
    st.ChildBorderSize = 1;
    st.FrameBorderSize = 0;
    st.FramePadding = ImVec2(14, 9);
    st.ItemSpacing = ImVec2(12, 8);
    st.ScrollbarSize = 8;
    st.GrabMinSize = 14;
    st.ScaleAllSizes(scale);
    g_scale = scale;

    ImVec4* c = st.Colors;
    c[ImGuiCol_WindowBg] = kBg;
    c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_Text] = kText;
    c[ImGuiCol_TextDisabled] = kMuted;
    c[ImGuiCol_Border] = kBorder;
    c[ImGuiCol_FrameBg] = kPanelHi;
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.16f, 0.2f, 1);
    c[ImGuiCol_FrameBgActive] = kBorder;
    c[ImGuiCol_SliderGrab] = kGold;
    c[ImGuiCol_SliderGrabActive] = kGoldHi;
    c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ScrollbarGrab] = kBorder;
    c[ImGuiCol_ScrollbarGrabHovered] = kPanelHi;
    c[ImGuiCol_ScrollbarGrabActive] = kGold;
    c[ImGuiCol_Button] = kPanelHi;
    c[ImGuiCol_Separator] = kBorder;
}

void DrawUi(App& app) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("root", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    TitleBar(app);
    ImGui::SetCursorPos(ImVec2(0, S(kTitleBarHeight)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, ImGui::GetStyle().ItemSpacing.y));
    Sidebar(app);
    ImGui::SameLine();
    ImGui::PopStyleVar();
    Main(app);

    ImGui::End();
}
