#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "config.h"

struct Vec3 {
    float x, y, z;
};

// Snapshot of one character's state, read from game memory.
struct ChrState {
    uintptr_t chr = 0;
    uintptr_t data = 0;     // ChrDataModule*
    uintptr_t physics = 0;  // ChrPhysicsModule*
    int hp = 0;
    int maxHp = 0;
    Vec3 pos{};
    bool hasPos = false;
};

float Distance(const ChrState& a, const ChrState& b);
float Distance(const Vec3& a, const Vec3& b);

class Game {
public:
    explicit Game(const Offsets& off) : off_(off) {}

    // Resolves the global pointers by pattern scan. Safe to call repeatedly until it succeeds.
    bool Init(std::string& err);

    uintptr_t WorldChrMan() const;
    uintptr_t Player() const;
    std::vector<uintptr_t> Characters() const;
    bool IsLoaded(uintptr_t chr) const;

    std::optional<ChrState> ReadChr(uintptr_t chr) const;
    bool SetHp(const ChrState& s, int hp) const;
    bool SetPosition(const ChrState& s, const Vec3& pos) const;
    bool SetNoDead(const ChrState& s, bool on) const;
    bool SetDbgPlayerNoDead(bool on) const;

    uintptr_t worldChrManGlobal() const { return worldChrManGlobal_; }
    uintptr_t chrDbgFlags() const { return chrDbgFlags_; }
    std::optional<uint8_t> FlagByte(const ChrState& s) const;

private:
    const Offsets& off_;
    uintptr_t worldChrManGlobal_ = 0;
    uintptr_t chrDbgFlags_ = 0;
};
