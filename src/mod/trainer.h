#pragma once
#include <windows.h>

// Runs the mod until the user unloads it (F10). Called on the mod's own thread.
void RunTrainer(HMODULE self);
