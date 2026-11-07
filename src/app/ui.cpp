#include "ui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include <shellapi.h>

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

std::string Narrow(const std::wstring& w) {
    std::string s(w.size() * 3, '\0');
    s.resize(WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), s.data(),
                                 static_cast<int>(s.size()), nullptr, nullptr));
    return s;
}

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
                "boss training  \u00B7  v" GGG_V_STRING);
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

void Chip(const char* text, const ImVec4& color, ImVec2 at) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 sz = ImGui::CalcTextSize(text);
    const ImVec2 pad(S(8), S(3));
    dl->AddRectFilled(at, ImVec2(at.x + sz.x + pad.x * 2, at.y + sz.y + pad.y * 2), U32(color, 0.16f), S(20));
    dl->AddRect(at, ImVec2(at.x + sz.x + pad.x * 2, at.y + sz.y + pad.y * 2), U32(color, 0.55f), S(20));
    dl->AddText(ImVec2(at.x + pad.x, at.y + pad.y), U32(color), text);
}

float ChipWidth(const char* text) { return ImGui::CalcTextSize(text).x + S(16); }

struct ConnView {
    const char* status;
    ImVec4 color;
    std::string detail;
    const char* button;
    bool primary;
};

ConnView Describe(const App& app) {
    const InstalledModule* m = app.Active();
    const std::string exe = m ? Narrow(m->manifest.exe) : "the game";
    switch (app.conn) {
        case Conn::NoGame:
            return app.gameFound
                       ? ConnView{"Not running", kMuted, "Launch the game here or start it yourself; the mod attaches automatically.",
                                  "Launch game", true}
                       : ConnView{"Game not found", kAmber, "Point Git Gud Gym at " + exe + " to launch from here.",
                                  "Locate game", true};
        case Conn::GameStarting:
            return {"Game starting", kAmber, "Waiting for the game to finish loading...", nullptr, false};
        case Conn::Attaching:
            return {"Attaching", kAmber, "Loading the mod into the game...", nullptr, false};
        case Conn::Connected:
            return {"Active", kGreen, "Mods are live. Closing this app turns them off.", "Unload mod", false};
        case Conn::Detached:
            return {"Unloaded", kMuted, "The game is running without the mod.", "Attach mod", true};
        case Conn::NeedsAdmin:
            return {"Needs admin", kAmber, "The game runs as administrator, so this app must too.", "Restart as admin", true};
        case Conn::AntiCheat:
            return {"Anti-cheat running", kRed,
                    "Anti-cheat is active, so the mod won't attach. Restart the game offline with Launch game.", nullptr,
                    false};
        case Conn::Error:
            return {"Couldn't attach", kRed, app.connDetail, "Retry", true};
    }
    return {};
}

// Top of the game page: which game, whether the mod is live, and where the game is installed.
void GameHeader(App& app) {
    const InstalledModule* m = app.Active();
    if (!m) return;
    const ConnView v = Describe(app);

    BeginCard("header", nullptr, app);
    if (ImGui::BeginTable("hdr", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("info", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed, S(330));
        ImGui::TableNextColumn();

        ImGui::PushFont(app.fonts.title);
        ImGui::TextUnformatted(m->manifest.name.c_str());
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::PushFont(app.fonts.note);
        const std::string version = (m->dev ? "dev build v" : "module v") + m->manifest.version;
        const ImVec2 at = ImGui::GetCursorScreenPos();
        Chip(version.c_str(), m->dev ? kAmber : kGold, ImVec2(at.x + S(4), at.y + S(10)));
        ImGui::PopFont();
        ImGui::NewLine();

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
        if (!app.gameVersion.empty() && app.conn != Conn::NoGame) {
            ImGui::SameLine();
            const std::string line = "·  game " + app.gameVersion +
                                     (app.gameVersionTested ? " (tested)" : " (untested version, see the docs)");
            SmallLabel(app, line.c_str(), app.gameVersionTested ? kMuted : kAmber);
        }
        ImGui::PushTextWrapPos(0);
        SmallLabel(app, v.detail.c_str());
        ImGui::PopTextWrapPos();

        ImGui::TableNextColumn();
        if (v.button) {
            if (Button(v.button, v.primary, -FLT_MIN)) app.PrimaryAction();
        } else {
            Button(app.conn == Conn::AntiCheat ? "Blocked" : "Working...", false, -FLT_MIN, false);
        }
        ImGui::PushFont(app.fonts.note);
        ImGui::PushStyleColor(ImGuiCol_Text, app.gameFound ? kMuted : kAmber);
        const std::string where = app.gameFound ? app.gamePath.parent_path().string() : "Game location not set";
        ImGui::TextUnformatted(where.c_str());
        if (app.gameFound && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", app.gamePath.string().c_str());
        ImGui::PopStyleColor();
        if (!app.locateMessage.empty()) SmallLabel(app, app.locateMessage.c_str());
        ImGui::PopFont();
        const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2;
        if (Button("Browse...", false, half)) app.BrowseForGame();
        ImGui::SameLine();
        if (Button("Auto-detect", false, half)) app.DetectGame();
        ImGui::EndTable();
    }
    EndCard();
}

bool NavItem(App& app, const char* id, const char* label, bool selected, const ImVec4* dot, const char* chip) {
    const float w = ImGui::GetContentRegionAvail().x, h = S(40);
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const bool clicked = ImGui::InvisibleButton(id, ImVec2(w, h));
    const bool hovered = ImGui::IsItemHovered();
    if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (selected || hovered) dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), U32(selected ? kPanelHi : kPanel), S(8));
    if (selected) dl->AddRectFilled(ImVec2(p.x, p.y + S(10)), ImVec2(p.x + S(3), p.y + h - S(10)), U32(kGold), S(2));

    ImGui::PushFont(app.fonts.semibold);
    const float ty = p.y + (h - ImGui::GetFontSize()) * 0.5f;
    dl->AddText(ImVec2(p.x + S(14), ty), U32(selected ? kText : ImLerp(kMuted, kText, hovered ? 0.6f : 0.25f)), label);
    ImGui::PopFont();
    float right = p.x + w - S(12);
    if (dot) {
        const float pulse = 0.6f + 0.4f * std::sin(static_cast<float>(ImGui::GetTime()) * 3);
        dl->AddCircleFilled(ImVec2(right - S(4), p.y + h * 0.5f), S(4), U32(*dot, pulse));
        right -= S(16);
    }
    if (chip) {
        ImGui::PushFont(app.fonts.note);
        const float cw = ChipWidth(chip);
        Chip(chip, kAmber, ImVec2(right - cw, p.y + (h - ImGui::GetFontSize()) * 0.5f - S(3)));
        ImGui::PopFont();
    }
    return clicked;
}

void Sidebar(App& app) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kSidebar);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(14), S(18)));
    ImGui::BeginChild("sidebar", ImVec2(S(240), 0), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    if (NavItem(app, "##library", "Library", app.page == Page::Library, nullptr, nullptr)) app.OpenLibrary();
    ImGui::Dummy(ImVec2(0, S(8)));
    SmallLabel(app, "  YOUR GAMES");
    if (app.installed.empty()) {
        ImGui::PushTextWrapPos(0);
        SmallLabel(app, "  Nothing installed yet. Add a game from the library.");
        ImGui::PopTextWrapPos();
    }
    const InstalledModule* active = app.Active();
    for (const InstalledModule& m : app.installed) {
        const bool isActive = active == &m;
        const ImVec4* dot = isActive && app.connected ? &kGreen : nullptr;
        const std::string id = "##game_" + m.manifest.id;
        if (NavItem(app, id.c_str(), m.manifest.name.c_str(), app.page == Page::Game && isActive, dot,
                    m.dev ? "dev" : nullptr)) {
            app.OpenGame(m.manifest.id);
        }
    }

    // Bottom: app update notice and version.
    const bool update = app.AppUpdateAvailable();
    const float bottom = ImGui::GetWindowHeight() - S(18) - ImGui::GetFontSize() - (update ? S(96) : 0);
    if (ImGui::GetCursorPosY() < bottom) ImGui::SetCursorPosY(bottom);
    if (update) {
        const std::string text = "Git Gud Gym " + app.catalog->appVersion + " is out";
        SmallLabel(app, text.c_str(), kGold);
        if (Button("Download update", true, -FLT_MIN)) {
            ShellExecuteA(nullptr, "open", app.AppUpdateUrl().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
        ImGui::Dummy(ImVec2(0, S(4)));
    }
    SmallLabel(app, "Git Gud Gym v" GGG_V_STRING);
    ImGui::EndChild();
}

// Procedural cover art: our own look, no publisher artwork.
void CoverArt(App& app, const std::string& id, const std::string& name, ImVec2 p, ImVec2 size, bool dim) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    unsigned h = 2166136261u;
    for (char c : id) h = (h ^ static_cast<unsigned char>(c)) * 16777619u;
    // A slightly different warm tint per game, always within the gold palette.
    const float tint = (h % 100) / 100.0f;
    const ImVec4 a = ImLerp(ImVec4(0.10f, 0.08f, 0.05f, 1), ImVec4(0.12f, 0.06f, 0.06f, 1), tint);
    const ImVec4 b = ImVec4(0.05f, 0.055f, 0.075f, 1);
    const ImVec2 q(p.x + size.x, p.y + size.y);
    dl->PushClipRect(p, q, true);
    dl->AddRectFilledMultiColor(p, q, U32(a), U32(b), U32(b), U32(a));
    // Diagonal hairlines.
    for (float x = p.x - size.y; x < q.x; x += S(14)) {
        dl->AddLine(ImVec2(x, q.y), ImVec2(x + size.y, p.y), U32(kGold, 0.05f), S(1));
    }
    if (app.logo) {
        const float s = size.y * 1.3f;
        dl->AddImage(app.logo, ImVec2(q.x - s * 0.8f, p.y - s * 0.15f), ImVec2(q.x - s * 0.8f + s, p.y + s * 0.85f),
                     ImVec2(0, 0), ImVec2(1, 1), U32(ImVec4(1, 1, 1, 0.10f)));
    }
    dl->AddRectFilledMultiColor(ImVec2(p.x, q.y - size.y * 0.6f), q, U32(kBg, 0), U32(kBg, 0), U32(kBg, 0.85f),
                                U32(kBg, 0.85f));
    ImGui::PushFont(app.fonts.title);
    std::string upper = name;
    for (char& c : upper) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    dl->AddText(ImVec2(p.x + S(18), q.y - ImGui::GetFontSize() - S(14)), U32(dim ? kMuted : kText), upper.c_str());
    ImGui::PopFont();
    dl->PopClipRect();
}

void GameTile(App& app, const LibraryEntry& e, float width) {
    const float height = S(290), art = S(140);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kPanel);
    ImGui::PushStyleColor(ImGuiCol_Border, kBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    const std::string id = "tile_" + e.id;
    ImGui::BeginChild(id.c_str(), ImVec2(width, height), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    const ImVec2 p = ImGui::GetCursorScreenPos();
    CoverArt(app, e.id, e.name, p, ImVec2(width, art), e.planned);

    // Status chip in the corner.
    ImGui::PushFont(app.fonts.note);
    std::string chip;
    ImVec4 chipColor = kMuted;
    if (e.installed && e.installed->dev) {
        chip = "Dev build", chipColor = kAmber;
    } else if (e.updateAvailable) {
        chip = "Update " + e.published->version, chipColor = kGold;
    } else if (e.installed) {
        chip = "Installed", chipColor = kGreen;
    } else if (e.planned) {
        chip = "Planned";
    } else if (e.published) {
        chip = "v" + e.published->version, chipColor = kGold;
    } else {
        chip = "Coming soon";
    }
    Chip(chip.c_str(), chipColor, ImVec2(p.x + width - ChipWidth(chip.c_str()) - S(12), p.y + S(12)));
    ImGui::PopFont();

    ImGui::SetCursorScreenPos(ImVec2(p.x + S(18), p.y + art + S(14)));
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width - S(36));
    SmallLabel(app, e.tagline.c_str());
    ImGui::PopTextWrapPos();

    // Actions pinned to the bottom of the tile.
    ImGui::SetCursorScreenPos(ImVec2(p.x + S(18), p.y + height - S(38) - S(18)));
    const float full = width - S(36);
    ImGui::PushID(e.id.c_str());
    if (app.Installing(e.id)) {
        char text[48];
        std::snprintf(text, sizeof(text), "Downloading... %.0f%%", app.installProgress * 100);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX());
        ImGui::BeginGroup();
        ImGui::PushItemWidth(full);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, kGold);
        ImGui::ProgressBar(app.installProgress, ImVec2(full, S(38)), text);
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        ImGui::EndGroup();
    } else if (e.installed) {
        const float gap = ImGui::GetStyle().ItemSpacing.x;
        const float side = e.updateAvailable ? (full - gap * 2) / 3 : (full - gap) / 2;
        if (Button("Open", true, e.updateAvailable ? side : full - side - gap)) app.OpenGame(e.id);
        if (e.updateAvailable) {
            ImGui::SameLine();
            if (Button("Update", false, side)) app.Install(e.id);
        }
        ImGui::SameLine();
        if (Button("Remove", false, side)) {
            const std::string q = "Remove the " + e.name + " module? Your stats are kept.";
            if (MessageBoxA(app.window, q.c_str(), "Git Gud Gym", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                app.Uninstall(e.id);
            }
        }
    } else if (e.published) {
        std::string label = "Install  v" + e.published->version;
        uint64_t bytes = 0;
        for (const CatalogFile& f : e.published->files) bytes += f.size;
        char size[32];
        std::snprintf(size, sizeof(size), "  ·  %.1f MB", bytes / 1048576.0);
        label += size;
        if (Button(label.c_str(), true, full)) app.Install(e.id);
    } else if (e.planned && app.catalog && !app.catalog->repo.empty()) {
        if (Button("Request this game", false, full)) {
            const std::string url = "https://github.com/" + app.catalog->repo + "/issues/new/choose";
            ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
    }
    ImGui::PopID();
    ImGui::EndChild();
}

void LibraryPage(App& app) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(28), S(24)));
    ImGui::BeginChild("library", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    ImGui::PushFont(app.fonts.title);
    ImGui::TextUnformatted("Library");
    ImGui::PopFont();
    SmallLabel(app, "Add the games you play. Each game is a small module that installs and updates on its own.");

    // Catalog status + refresh, right-aligned on the title row.
    const float refreshW = S(110);
    const ImVec2 back = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - S(28) - refreshW, S(24)));
    if (Button(app.CatalogLoading() ? "Checking..." : "Refresh", false, refreshW, !app.CatalogLoading())) {
        app.RefreshCatalog();
    }
    if (!app.catalogStatus.empty()) {
        ImGui::PushFont(app.fonts.note);
        const float w = ImGui::CalcTextSize(app.catalogStatus.c_str()).x;
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - S(28) - refreshW - w - S(14), S(36)));
        ImGui::PopFont();
        SmallLabel(app, app.catalogStatus.c_str());
        if (!app.catalogError.empty() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", app.catalogError.c_str());
    }
    ImGui::SetCursorPos(back);
    ImGui::Dummy(ImVec2(0, S(6)));
    if (!app.libraryMessage.empty()) {
        SmallLabel(app, app.libraryMessage.c_str(), kGold);
        ImGui::Dummy(ImVec2(0, S(2)));
    }

    const std::vector<LibraryEntry> entries = app.Library();
    if (entries.empty()) {
        SmallLabel(app, app.CatalogLoading() ? "Loading the game library..."
                                             : "The game library couldn't be loaded. Check your connection and Refresh.");
    }
    const float gap = S(18);
    const float avail = ImGui::GetContentRegionAvail().x;
    const int cols = std::max(1, static_cast<int>((avail + gap) / (S(300) + gap)));
    const float width = (avail - gap * (cols - 1)) / cols;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, gap));
    for (size_t i = 0; i < entries.size(); ++i) {
        if (i % cols) ImGui::SameLine();
        GameTile(app, entries[i], width);
    }
    ImGui::PopStyleVar();
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
    ImGui::SameLine();
    if (Button("Stats folder", false)) app.OpenDataFolder();
    ImGui::SameLine();
    if (Button("Log", false)) app.OpenLog();
    EndCard();
}

void ActivityCard(App& app) {
    // Fixed height; the feed scrolls inside the card.
    BeginCard("activity", "Activity", app, S(240));
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

void GamePage(App& app) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(24), S(20)));
    ImGui::BeginChild("main", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();

    GameHeader(app);

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

    // Columns size to their content and the whole page scrolls, so nothing is clipped at high DPI scaling.
    const float leftW = (ImGui::GetContentRegionAvail().x - gap) * 0.56f;
    const ImGuiChildFlags column = ImGuiChildFlags_AutoResizeY;
    ImGui::BeginChild("left", ImVec2(leftW, 0), column, ImGuiWindowFlags_NoScrollbar);
    LiveCard(app);
    SessionCard(app);
    ActivityCard(app);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right", ImVec2(0, 0), column, ImGuiWindowFlags_NoScrollbar);
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
    if (app.page == Page::Game && app.Active()) {
        GamePage(app);
    } else {
        LibraryPage(app);
    }

    ImGui::End();
}
