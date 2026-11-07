#include "offsets.h"

#include "ini.h"

Offsets LoadOffsets(const std::filesystem::path& file) {
    Offsets o;
    ini::ReadNum(file, L"offsets", L"player", o.player);
    ini::ReadNum(file, L"offsets", L"chr_list_begin", o.chrListBegin);
    ini::ReadNum(file, L"offsets", L"chr_list_end", o.chrListEnd);
    ini::ReadNum(file, L"offsets", L"chr_modules", o.chrModules);
    ini::ReadNum(file, L"offsets", L"module_data", o.moduleData);
    ini::ReadNum(file, L"offsets", L"module_physics", o.modulePhysics);
    ini::ReadNum(file, L"offsets", L"physics_pos", o.physicsPos);
    ini::ReadNum(file, L"offsets", L"data_hp", o.dataHp);
    ini::ReadNum(file, L"offsets", L"data_max_hp", o.dataMaxHp);
    ini::ReadNum(file, L"offsets", L"data_flags", o.dataFlags);
    ini::ReadNum(file, L"offsets", L"no_dead_bit", o.noDeadBit);
    ini::ReadNum(file, L"offsets", L"use_dbg_player_no_dead", o.useDbgPlayerNoDead);
    ini::ReadNum(file, L"offsets", L"dbg_player_no_dead", o.dbgPlayerNoDead);
    if (o.noDeadBit > 7) o.noDeadBit = 0;
    return o;
}
