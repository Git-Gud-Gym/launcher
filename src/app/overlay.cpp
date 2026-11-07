#include "overlay.h"

#include <objidl.h>
#include <gdiplus.h>
#include <shlwapi.h>

#include "resources.h"

#include <algorithm>
#include <cstring>

using namespace Gdiplus;
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

namespace {

constexpr wchar_t kClassName[] = L"GitGudGymOverlay";
constexpr float kToastSeconds = 3.2f;

const Color kBg(235, 14, 15, 20);
const Color kGold(255, 212, 168, 75);
const Color kRed(255, 229, 72, 77);
const Color kAmber(255, 240, 170, 60);
const Color kText(255, 236, 234, 230);
const Color kMuted(255, 150, 155, 165);

void AddRoundRect(GraphicsPath& path, RectF r, float radius) {
    const float d = radius * 2;
    path.AddArc(r.X, r.Y, d, d, 180, 90);
    path.AddArc(r.GetRight() - d, r.Y, d, d, 270, 90);
    path.AddArc(r.GetRight() - d, r.GetBottom() - d, d, d, 0, 90);
    path.AddArc(r.X, r.GetBottom() - d, d, d, 90, 90);
    path.CloseFigure();
}

void Panel(Graphics& g, RectF r, float radius, const Color& fill, const Color& border, float borderWidth) {
    GraphicsPath path;
    AddRoundRect(path, r, radius);
    SolidBrush brush(fill);
    g.FillPath(&brush, &path);
    Pen pen(border, borderWidth);
    g.DrawPath(&pen, &path);
}

void Text(Graphics& g, const std::wstring& s, const wchar_t* family, float size, int style, const Color& color,
          float x, float y) {
    FontFamily ff(family);
    Font font(ff.IsAvailable() ? &ff : FontFamily::GenericSansSerif(), size, style, UnitPixel);
    SolidBrush brush(color);
    g.DrawString(s.c_str(), -1, &font, PointF(x, y), &brush);
}

Color KindColor(ToastKind kind) {
    switch (kind) {
        case ToastKind::Death: return kRed;
        case ToastKind::Defeat: return kGold;
        case ToastKind::Warning: return kAmber;
        default: return kMuted;
    }
}

std::wstring FormatTime(float sec) {
    const int s = static_cast<int>(sec);
    wchar_t buf[32];
    swprintf_s(buf, L"%d:%02d", s / 60, s % 60);
    return buf;
}

void SetupGraphics(Graphics& g) {
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAlias);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.Clear(Color(0, 0, 0, 0));
}

}  // namespace

bool Overlay::Init(HINSTANCE instance) {
    GdiplusStartupInput input;
    if (GdiplusStartup(&gdiplusToken_, &input, nullptr) != Ok) return false;

    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = instance;
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);
    toastLayer_.hwnd = CreateLayer(instance);
    hudLayer_.hwnd = CreateLayer(instance);

    const auto png = EmbeddedResource(IDR_LOGO, RT_RCDATA);
    logoStream_ = png.empty() ? nullptr : SHCreateMemStream(png.data(), static_cast<UINT>(png.size()));
    logo_ = logoStream_ ? Image::FromStream(logoStream_) : nullptr;
    if (logo_ && logo_->GetLastStatus() != Ok) {
        delete logo_;
        logo_ = nullptr;
    }
    return toastLayer_.hwnd && hudLayer_.hwnd;
}

void Overlay::Shutdown() {
    delete logo_;
    logo_ = nullptr;
    if (logoStream_) logoStream_->Release();
    logoStream_ = nullptr;
    if (toastLayer_.hwnd) DestroyWindow(toastLayer_.hwnd);
    if (hudLayer_.hwnd) DestroyWindow(hudLayer_.hwnd);
    if (gdiplusToken_) GdiplusShutdown(gdiplusToken_);
}

HWND Overlay::CreateLayer(HINSTANCE instance) {
    return CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                           kClassName, L"", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
}

void Overlay::ShowToast(ToastKind kind, std::wstring title, std::wstring subtitle) {
    toast_ = {kind, std::move(title), std::move(subtitle), Clock::now(), true};
}

void Overlay::Update(HWND game, bool gameFocused, const ggg::Status& status, bool showHud, bool showToasts) {
    if (!game || !gameFocused || IsIconic(game)) {
        Hide(toastLayer_);
        Hide(hudLayer_);
        return;
    }
    RECT client;
    GetClientRect(game, &client);
    POINT origin{0, 0};
    ClientToScreen(game, &origin);
    OffsetRect(&client, origin.x, origin.y);
    // Scale with the game's resolution so the overlay looks the same at 1080p and 4K.
    const float k = std::clamp((client.bottom - client.top) / 1080.0f, 0.75f, 2.5f);

    if (showToasts && toast_.active) {
        DrawToast(client, k);
    } else {
        Hide(toastLayer_);
    }

    if (showHud && status.running) {
        if (Clock::now() - lastHudDraw_ > 100ms || !hudLayer_.visible) {
            DrawHud(client, k, status);
            lastHudDraw_ = Clock::now();
        }
    } else {
        Hide(hudLayer_);
    }
}

void Overlay::DrawToast(const RECT& client, float k) {
    const float t = std::chrono::duration<float>(Clock::now() - toast_.shownAt).count();
    if (t > kToastSeconds) {
        toast_.active = false;
        Hide(toastLayer_);
        return;
    }
    // Fade in quickly, hold, fade out.
    const float alpha = std::min({1.0f, t / 0.15f, (kToastSeconds - t) / 0.5f});

    const int w = static_cast<int>(500 * k), h = static_cast<int>(96 * k);
    Bitmap bmp(w, h, PixelFormat32bppPARGB);
    Graphics g(&bmp);
    SetupGraphics(g);

    const Color accent = KindColor(toast_.kind);
    Panel(g, RectF(1, 1, w - 2.0f, h - 2.0f), 12 * k, kBg, Color(140, accent.GetR(), accent.GetG(), accent.GetB()),
          1.5f * k);
    SolidBrush stripe(accent);
    g.FillRectangle(&stripe, RectF(1, 18 * k, 4 * k, h - 36 * k));

    float textX = 24 * k;
    if (logo_) {
        g.DrawImage(logo_, RectF(18 * k, 16 * k, 64 * k, 64 * k));
        textX = 96 * k;
    }
    Text(g, toast_.title, L"Bahnschrift", 28 * k, FontStyleBold, accent, textX, 14 * k);
    Text(g, toast_.subtitle, L"Segoe UI", 15 * k, FontStyleRegular, kMuted, textX + 2 * k, 54 * k);

    const int x = (client.left + client.right - w) / 2;
    const int y = client.top + static_cast<int>((client.bottom - client.top) * 0.12f);
    Present(toastLayer_, bmp, x, y, static_cast<BYTE>(255 * std::max(0.0f, alpha)));
}

void Overlay::DrawHud(const RECT& client, float k, const ggg::Status& s) {
    const int w = static_cast<int>(320 * k), h = static_cast<int>(78 * k);
    Bitmap bmp(w, h, PixelFormat32bppPARGB);
    Graphics g(&bmp);
    SetupGraphics(g);
    Panel(g, RectF(1, 1, w - 2.0f, h - 2.0f), 10 * k, Color(205, 12, 13, 18), Color(90, 212, 168, 75), 1.0f * k);

    struct Column {
        const wchar_t* label;
        std::wstring value;
        Color color;
    } cols[] = {
        {L"DEATHS", std::to_wstring(s.deaths), kRed},
        {L"DEFEATS", std::to_wstring(s.defeats), kGold},
        {s.attemptActive ? L"ATTEMPT" : L"NEXT TRY", s.attemptActive ? FormatTime(s.attemptSec) : L"#" + std::to_wstring(s.attemptIndex), kText},
    };
    for (int i = 0; i < 3; ++i) {
        const float x = (16 + i * 100) * k;
        Text(g, cols[i].label, L"Segoe UI", 11 * k, FontStyleBold, kMuted, x, 10 * k);
        Text(g, cols[i].value, L"Bahnschrift", 24 * k, FontStyleBold, cols[i].color, x - 1 * k, 24 * k);
    }

    // Boss HP bar along the bottom.
    const RectF bar(16 * k, h - 16 * k, w - 32 * k, 5 * k);
    SolidBrush track(Color(255, 40, 42, 50));
    g.FillRectangle(&track, bar);
    if (s.bossTracked && s.bossMaxHp > 0) {
        SolidBrush fill(kGold);
        g.FillRectangle(&fill, RectF(bar.X, bar.Y, bar.Width * std::clamp(1.0f * s.bossHp / s.bossMaxHp, 0.0f, 1.0f),
                                     bar.Height));
    }

    Present(hudLayer_, bmp, client.left + static_cast<int>(24 * k), client.top + static_cast<int>(24 * k), 255);
}

void Overlay::Present(Layer& layer, Bitmap& bmp, int x, int y, BYTE alpha) {
    const int w = static_cast<int>(bmp.GetWidth()), h = static_cast<int>(bmp.GetHeight());
    HDC screen = GetDC(nullptr);
    HDC mem = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);

    // GDI+ PARGB is exactly the premultiplied BGRA that UpdateLayeredWindow wants.
    BitmapData data;
    Rect rect(0, 0, w, h);
    if (dib && bmp.LockBits(&rect, ImageLockModeRead, PixelFormat32bppPARGB, &data) == Ok) {
        for (int row = 0; row < h; ++row) {
            std::memcpy(static_cast<BYTE*>(bits) + row * w * 4, static_cast<BYTE*>(data.Scan0) + row * data.Stride,
                        w * 4);
        }
        bmp.UnlockBits(&data);

        HGDIOBJ old = SelectObject(mem, dib);
        POINT dst{x, y}, src{0, 0};
        SIZE size{w, h};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA};
        UpdateLayeredWindow(layer.hwnd, screen, &dst, &size, mem, &src, 0, &blend, ULW_ALPHA);
        SelectObject(mem, old);
    }
    if (dib) DeleteObject(dib);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);

    if (!layer.visible) {
        ShowWindow(layer.hwnd, SW_SHOWNOACTIVATE);
        SetWindowPos(layer.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        layer.visible = true;
    }
}

void Overlay::Hide(Layer& layer) {
    if (!layer.visible) return;
    ShowWindow(layer.hwnd, SW_HIDE);
    layer.visible = false;
}
