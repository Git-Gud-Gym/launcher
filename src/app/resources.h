// Files embedded in GitGudGym.exe (see app.rc), so the app is a single download.
#pragma once
#include <windows.h>

#include <span>

#define IDR_LOGO 101    // assets/logo_mark.png
#define IDR_NOTIFY 102  // assets/mixkit-gaming-lock-2848.wav (resource type WAVE, for PlaySound)

#ifndef RC_INVOKED
// Bytes of an embedded resource; valid for the life of the process. Empty if missing.
inline std::span<const BYTE> EmbeddedResource(int id, const wchar_t* type) {
    HMODULE self = GetModuleHandleW(nullptr);
    HRSRC info = FindResourceW(self, MAKEINTRESOURCEW(id), type);
    HGLOBAL data = info ? LoadResource(self, info) : nullptr;
    const void* bytes = data ? LockResource(data) : nullptr;
    if (!bytes) return {};
    return {static_cast<const BYTE*>(bytes), SizeofResource(self, info)};
}
#endif
