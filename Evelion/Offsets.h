#pragma once

#include <cstdint>

namespace Offsets {

// GoldSrc/hl.exe is a 32-bit process even when the overlay is built as x64.
using RemotePtr = std::uint32_t;

// Counter-Strike 1.6 Steam, protocol 48
// Exe 1.1.2.7/Stdio, build 10210 (Oct 7 2024).
namespace Build10210 {
    constexpr std::uintptr_t EntityListRoot = 0x00F779D8; // hw.dll -> pointer
    constexpr std::uintptr_t LocalPlayerSlot = 0x7C;
    constexpr std::uintptr_t EntityStride = 0x324;

    constexpr std::uintptr_t EntityObjectData = 0x4;      // pointer
    constexpr std::uintptr_t EntityHealth = 0x160;        // object data
    constexpr std::uintptr_t EntityTeam = 0x1C8;          // entity
    constexpr std::uintptr_t EntityPosition = 0x7DC;      // entity
    constexpr std::uintptr_t ObjectPosition = 0x8;        // validated fallback

    constexpr std::uintptr_t ViewMatrixHw = 0x00EC9780;
    constexpr std::uintptr_t ViewMatrixClient = 0x0012EB10;
    constexpr std::uintptr_t ViewAngles = 0x01282B44;

    // Optional HUD player-info table. Used only for names; gameplay data comes
    // from EntityListRoot above and remains valid when this table is absent.
    constexpr std::uintptr_t PlayerInfoList = 0x012043CC;
    constexpr std::uintptr_t PlayerInfoStride = 0x250;
    constexpr std::uintptr_t PlayerInfoName = 0x100;
}

constexpr int MaxPlayers = 64;

} // namespace Offsets
