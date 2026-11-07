#include <windows.h>

#include "trainer.h"

namespace {

HMODULE g_self = nullptr;

DWORD WINAPI MainThread(LPVOID) {
    RunTrainer(g_self);
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
