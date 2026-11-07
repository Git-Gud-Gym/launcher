// Git Gud Gym desktop app: attaches the mod to the game and exposes its toggles and stats.
#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <wincodec.h>
#include <windowsx.h>

#include <algorithm>
#include <filesystem>
#include <vector>

#include "app.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "ui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {

ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_rtv = nullptr;
UINT g_resizeW = 0, g_resizeH = 0;
float g_scale = 1.0f;

void CreateRenderTarget() {
    ID3D11Texture2D* back = nullptr;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&back));
    g_device->CreateRenderTargetView(back, nullptr, &g_rtv);
    back->Release();
}

void CleanupRenderTarget() {
    if (g_rtv) {
        g_rtv->Release();
        g_rtv = nullptr;
    }
}

bool CreateDevice(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2,
                                               D3D11_SDK_VERSION, &sd, &g_swapChain, &g_device, &level, &g_context);
    if (hr == DXGI_ERROR_UNSUPPORTED) {
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 2, D3D11_SDK_VERSION,
                                           &sd, &g_swapChain, &g_device, &level, &g_context);
    }
    if (FAILED(hr)) return false;
    CreateRenderTarget();
    return true;
}

void CleanupDevice() {
    CleanupRenderTarget();
    if (g_swapChain) g_swapChain->Release();
    if (g_context) g_context->Release();
    if (g_device) g_device->Release();
}

// Decodes an image with WIC and uploads it as a texture, downscaled to size x size.
ImTextureID LoadTexture(const std::wstring& path, UINT size) {
    IWICImagingFactory* factory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICBitmapScaler* scaler = nullptr;
    IWICFormatConverter* converter = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    std::vector<BYTE> pixels(static_cast<size_t>(size) * size * 4);

    bool ok = SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))) &&
              SUCCEEDED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                                           WICDecodeMetadataCacheOnDemand, &decoder)) &&
              SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(factory->CreateBitmapScaler(&scaler)) &&
              SUCCEEDED(scaler->Initialize(frame, size, size, WICBitmapInterpolationModeHighQualityCubic)) &&
              SUCCEEDED(factory->CreateFormatConverter(&converter)) &&
              SUCCEEDED(converter->Initialize(scaler, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr,
                                              0, WICBitmapPaletteTypeCustom)) &&
              SUCCEEDED(converter->CopyPixels(nullptr, size * 4, static_cast<UINT>(pixels.size()), pixels.data()));
    if (ok) {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = desc.Height = size;
        desc.MipLevels = desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA data{pixels.data(), size * 4, 0};
        ID3D11Texture2D* tex = nullptr;
        if (SUCCEEDED(g_device->CreateTexture2D(&desc, &data, &tex))) {
            g_device->CreateShaderResourceView(tex, nullptr, &srv);
            tex->Release();
        }
    }
    for (IUnknown* u : std::initializer_list<IUnknown*>{converter, scaler, frame, decoder, factory}) {
        if (u) u->Release();
    }
    return reinterpret_cast<ImTextureID>(srv);
}

void LoadFonts(App& app, float scale) {
    ImGuiIO& io = ImGui::GetIO();
    static const ImWchar ranges[] = {0x0020, 0x00FF, 0x2013, 0x2026, 0};
    auto font = [&](const wchar_t* file, float px) -> ImFont* {
        wchar_t dir[MAX_PATH];
        GetWindowsDirectoryW(dir, MAX_PATH);
        const std::filesystem::path path = std::filesystem::path(dir) / L"Fonts" / file;
        if (!std::filesystem::exists(path)) return nullptr;
        return io.Fonts->AddFontFromFileTTF(path.string().c_str(), px * scale, nullptr, ranges);
    };
    app.fonts.body = font(L"segoeui.ttf", 16);
    if (!app.fonts.body) app.fonts.body = io.Fonts->AddFontDefault();
    auto orBody = [&](ImFont* f) { return f ? f : app.fonts.body; };
    app.fonts.semibold = orBody(font(L"seguisb.ttf", 16));
    app.fonts.note = orBody(font(L"segoeui.ttf", 13.5f));
    app.fonts.heading = orBody(font(L"bahnschrift.ttf", 21));
    app.fonts.big = orBody(font(L"bahnschrift.ttf", 36));
    app.fonts.brand = orBody(font(L"bahnschrift.ttf", 22));
    io.FontDefault = app.fonts.body;
}

LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;
    switch (msg) {
        case WM_NCHITTEST: {
            // Borderless window: the title bar (minus its buttons) drags the window.
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(hwnd, &pt);
            RECT rc;
            GetClientRect(hwnd, &rc);
            if (pt.y >= 0 && pt.y < kTitleBarHeight * g_scale && pt.x < rc.right - kTitleButtonsWidth * g_scale) {
                return HTCAPTION;
            }
            return HTCLIENT;
        }
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                g_resizeW = LOWORD(lParam);
                g_resizeH = HIWORD(lParam);
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    // One instance only: a second copy would fight over attaching.
    HANDLE single = CreateMutexW(nullptr, TRUE, L"Local\\GitGudGym.App");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND other = FindWindowW(L"GitGudGymApp", nullptr)) {
            ShowWindow(other, SW_RESTORE);
            SetForegroundWindow(other);
        }
        return 0;
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    g_scale = GetDpiForSystem() / 96.0f;

    WNDCLASSEXW wc{sizeof(wc)};
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(1));
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"GitGudGymApp";
    RegisterClassExW(&wc);

    RECT work;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    const int w = static_cast<int>(1200 * g_scale);
    const int h = std::min(static_cast<int>(880 * g_scale), static_cast<int>(work.bottom - work.top - 32 * g_scale));
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Git Gud Gym", WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
                                work.left + (work.right - work.left - w) / 2, work.top + (work.bottom - work.top - h) / 2,
                                w, h, nullptr, nullptr, instance, nullptr);

    // Windows 11: rounded corners and a dark frame shadow.
    const DWORD round = 2;  // DWMWCP_ROUND
    DwmSetWindowAttribute(hwnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &round, sizeof(round));
    const BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));

    if (!CreateDevice(hwnd)) {
        CleanupDevice();
        MessageBoxW(nullptr, L"Couldn't initialize Direct3D 11.", L"Git Gud Gym", MB_ICONERROR);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);

    App app;
    app.scale = g_scale;
    if (!app.Init(instance, hwnd)) {
        MessageBoxW(hwnd, L"Couldn't create the shared memory block for the mod.", L"Git Gud Gym", MB_ICONERROR);
        return 1;
    }
    LoadFonts(app, g_scale);
    ApplyTheme(g_scale);
    app.logo = LoadTexture((app.dir / L"assets" / L"logo.png").wstring(), 128);

    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;

        app.Tick();

        if (IsIconic(hwnd)) {
            Sleep(16);  // keep events, sounds and the overlay going while minimized
            continue;
        }
        if (g_resizeW) {
            CleanupRenderTarget();
            g_swapChain->ResizeBuffers(0, g_resizeW, g_resizeH, DXGI_FORMAT_UNKNOWN, 0);
            g_resizeW = g_resizeH = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        DrawUi(app);
        ImGui::Render();
        const float clear[4] = {0.047f, 0.051f, 0.071f, 1.0f};
        g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_context->ClearRenderTargetView(g_rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swapChain->Present(1, 0);
    }

    app.Shutdown();
    if (app.logo) reinterpret_cast<ID3D11ShaderResourceView*>(app.logo)->Release();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDevice();
    DestroyWindow(hwnd);
    CoUninitialize();
    CloseHandle(single);
    return 0;
}
