#pragma once
#include <cstdint>
#include <filesystem>

// Memory layout for Elden Ring, verified against eldenring.exe 2.2.0.0 (patch 1.12.0).
// Overridable from the [offsets] section of gitgudgym-eldenring.ini.
struct Offsets {
    uint32_t player = 0x1E508;       // WorldChrMan -> PlayerIns*
    uint32_t chrListBegin = 0x1F1B8;  // WorldChrMan -> ChrIns** begin
    uint32_t chrListEnd = 0x1F1C0;    // WorldChrMan -> ChrIns** end
    uint32_t chrModules = 0x190;      // ChrIns -> ChrModuleBag*
    uint32_t moduleData = 0x0;        // ChrModuleBag -> ChrDataModule*
    uint32_t modulePhysics = 0x68;    // ChrModuleBag -> ChrPhysicsModule*
    uint32_t physicsPos = 0x70;       // ChrPhysicsModule -> float[3]
    uint32_t dataHp = 0x138;
    uint32_t dataMaxHp = 0x13C;
    uint32_t dataFlags = 0x19B;
    uint8_t noDeadBit = 0;
    bool useDbgPlayerNoDead = false;
    uint32_t dbgPlayerNoDead = 0x0;
};

Offsets LoadOffsets(const std::filesystem::path& iniPath);
