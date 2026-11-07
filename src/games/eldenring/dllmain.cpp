// Entry point of the Elden Ring module: wires the Elden Ring adapter into the shared trainer core.
#include <windows.h>

#include <filesystem>

#include "eldenring_game.h"
#include "trainer.h"

namespace {

HMODULE g_self = nullptr;

DWORD WINAPI MainThread(LPVOID) {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(g_self, path, MAX_PATH);
    const std::filesystem::path dir = std::filesystem::path(path).parent_path();

    EldenRingGame game(LoadOffsets(dir / L"gitgudgym-eldenring.ini"));
    RunTrainer(g_self, game, L"eldenring");
    FreeLibraryAndExitThread(g_self, 0);
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = module;
        DisableThreadLibraryCalls(module);
        if (HANDLE t = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr)) CloseHandle(t);
    }
    return TRUE;
}
