#pragma once
#include "game_adapter.h"
#include "offsets.h"

class EldenRingGame : public GameAdapter {
public:
    explicit EldenRingGame(const Offsets& off) : off_(off) {}

    bool Init(std::string& err) override;
    uintptr_t Player() const override;
    std::vector<uintptr_t> Characters() const override;
    std::optional<ChrState> ReadChr(uintptr_t chr) const override;
    bool SetHp(const ChrState& s, int hp) const override;
    bool SetPosition(const ChrState& s, const Vec3& pos) const override;
    bool SetNoDead(const ChrState& s, bool on, bool isPlayer) const override;
    std::vector<std::string> Describe(const ChrState* player) const override;

private:
    // ChrState::internal slots
    static constexpr int kData = 0;     // ChrDataModule*
    static constexpr int kPhysics = 1;  // ChrPhysicsModule*

    uintptr_t WorldChrMan() const;

    Offsets off_;
    uintptr_t worldChrManGlobal_ = 0;
    uintptr_t chrDbgFlags_ = 0;
};
