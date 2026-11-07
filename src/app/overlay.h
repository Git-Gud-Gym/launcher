#pragma once
#include <windows.h>

#include <chrono>
#include <string>

#include "shared_state.h"

namespace Gdiplus {
class Bitmap;
class Image;
}  // namespace Gdiplus

enum class ToastKind { Death, Defeat, Warning, Info };

// Click-through, always-on-top windows drawn over the game: event toasts and a small stats HUD.
// Works over borderless/windowed mode; exclusive fullscreen may hide them.
class Overlay {
public:
    bool Init(HINSTANCE instance, const std::wstring& logoPath);
    void Shutdown();

    void ShowToast(ToastKind kind, std::wstring title, std::wstring subtitle);

    // Call every frame. Pass game = nullptr (or unfocused) to hide everything.
    void Update(HWND game, bool gameFocused, const ggg::Status& status, bool showHud, bool showToasts);

private:
    struct Layer {
        HWND hwnd = nullptr;
        bool visible = false;
    };
    struct Toast {
        ToastKind kind = ToastKind::Info;
        std::wstring title, subtitle;
        std::chrono::steady_clock::time_point shownAt;
        bool active = false;
    };

    HWND CreateLayer(HINSTANCE instance);
    void Present(Layer& layer, Gdiplus::Bitmap& bmp, int x, int y, BYTE alpha);
    void Hide(Layer& layer);
    void DrawToast(const RECT& client, float k);
    void DrawHud(const RECT& client, float k, const ggg::Status& status);

    ULONG_PTR gdiplusToken_ = 0;
    Gdiplus::Image* logo_ = nullptr;
    Layer toastLayer_, hudLayer_;
    Toast toast_;
    std::chrono::steady_clock::time_point lastHudDraw_{};
};
