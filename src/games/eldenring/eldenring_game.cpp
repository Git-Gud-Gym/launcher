#include "eldenring_game.h"

#include <cmath>
#include <cstdio>

#include "memory.h"

namespace {

// Both patterns match exactly once in eldenring.exe 2.2.0.0 (1.12.0).
constexpr const char* kWorldChrManPattern = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0F 48 39 88";
constexpr const char* kChrDbgFlagsPattern = "80 3D ?? ?? ?? ?? 00 0F 85 ?? ?? ?? ?? 32 C0 48";

constexpr size_t kMaxCharacters = 8192;

}  // namespace

bool EldenRingGame::Init(std::string& err) {
    const mem::Module exe = mem::MainModule();
    if (!worldChrManGlobal_) {
        if (uintptr_t hit = mem::FindPattern(exe, kWorldChrManPattern)) {
            worldChrManGlobal_ = mem::ResolveRip(hit, 3, 7);
        }
    }
    if (!chrDbgFlags_) {
        if (uintptr_t hit = mem::FindPattern(exe, kChrDbgFlagsPattern)) {
            chrDbgFlags_ = mem::ResolveRip(hit, 2, 7);
        }
    }
    if (!worldChrManGlobal_) {
        err = "WorldChrMan pattern not found (unsupported game version?)";
        return false;
    }
    return true;
}

uintptr_t EldenRingGame::WorldChrMan() const { return mem::ReadPtr(worldChrManGlobal_); }

uintptr_t EldenRingGame::Player() const {
    uintptr_t wcm = WorldChrMan();
    return wcm ? mem::ReadPtr(wcm + off_.player) : 0;
}

std::vector<uintptr_t> EldenRingGame::Characters() const {
    std::vector<uintptr_t> out;
    uintptr_t wcm = WorldChrMan();
    if (!wcm) return out;
    uintptr_t begin = mem::ReadPtr(wcm + off_.chrListBegin);
    uintptr_t end = mem::ReadPtr(wcm + off_.chrListEnd);
    if (!begin || end < begin) return out;
    size_t count = (end - begin) / sizeof(uintptr_t);
    if (count == 0 || count > kMaxCharacters) return out;
    out.resize(count);
    if (!mem::SafeCopy(out.data(), reinterpret_cast<const void*>(begin), count * sizeof(uintptr_t))) out.clear();
    return out;
}

std::optional<ChrState> EldenRingGame::ReadChr(uintptr_t chr) const {
    if (!mem::IsPlausiblePtr(chr)) return std::nullopt;
    uintptr_t modules = mem::ReadPtr(chr + off_.chrModules);
    if (!modules) return std::nullopt;
    uintptr_t data = mem::ReadPtr(modules + off_.moduleData);
    if (!data) return std::nullopt;
    auto hp = mem::Read<int32_t>(data + off_.dataHp);
    auto maxHp = mem::Read<int32_t>(data + off_.dataMaxHp);
    // Reject garbage so a stale or wrong pointer never gets written to.
    if (!hp || !maxHp || *maxHp <= 0 || *maxHp > 10'000'000 || *hp < 0 || *hp > *maxHp * 2) return std::nullopt;

    ChrState s;
    s.chr = chr;
    s.hp = *hp;
    s.maxHp = *maxHp;
    s.internal[kData] = data;
    if (uintptr_t phys = mem::ReadPtr(modules + off_.modulePhysics)) {
        if (auto pos = mem::Read<Vec3>(phys + off_.physicsPos); pos && std::isfinite(pos->x)) {
            s.internal[kPhysics] = phys;
            s.pos = *pos;
            s.hasPos = true;
        }
    }
    return s;
}

bool EldenRingGame::SetHp(const ChrState& s, int hp) const {
    return mem::Write<int32_t>(s.internal[kData] + off_.dataHp, hp);
}

bool EldenRingGame::SetPosition(const ChrState& s, const Vec3& pos) const {
    return s.internal[kPhysics] && mem::Write<Vec3>(s.internal[kPhysics] + off_.physicsPos, pos);
}

bool EldenRingGame::SetNoDead(const ChrState& s, bool on, bool isPlayer) const {
    if (isPlayer && off_.useDbgPlayerNoDead && chrDbgFlags_) {
        mem::Write<uint8_t>(chrDbgFlags_ + off_.dbgPlayerNoDead, on ? 1 : 0);
    }
    const uintptr_t addr = s.internal[kData] + off_.dataFlags;
    auto flags = mem::Read<uint8_t>(addr);
    if (!flags) return false;
    const uint8_t bit = static_cast<uint8_t>(1u << off_.noDeadBit);
    const uint8_t want = on ? (*flags | bit) : (*flags & ~bit);
    return want == *flags || mem::Write<uint8_t>(addr, want);
}

std::vector<std::string> EldenRingGame::Describe(const ChrState* player) const {
    char buf[160];
    std::vector<std::string> lines;
    std::snprintf(buf, sizeof(buf), "WorldChrMan global %p -> %p, ChrDbgFlags %p", (void*)worldChrManGlobal_,
                  (void*)WorldChrMan(), (void*)chrDbgFlags_);
    lines.push_back(buf);
    if (player) {
        auto flags = mem::Read<uint8_t>(player->internal[kData] + off_.dataFlags);
        std::snprintf(buf, sizeof(buf), "  player data %p flags 0x%02X", (void*)player->internal[kData],
                      flags ? *flags : 0);
        lines.push_back(buf);
    }
    return lines;
}
