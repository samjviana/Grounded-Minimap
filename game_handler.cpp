//
// Created by PC-SAMUEL on 21/12/2024.
//

#include "game_handler.h"
#include "globals.h"
#include "logger.h"
#include "memory.h"

namespace grounded_minimap {

std::unordered_map<uint32_t, std::string> GameHandler::fNameCache_;

std::string GameHandler::GWORLD_SIGNATURE = "48 8B CF E8 ?? ?? ?? ?? 48 C7 05 ?? ?? ?? ?? 00 00 00 00 33 DB";
std::string GameHandler::GNAMES_SIGNATURE = "80 3D ?? ?? ?? ?? 00 74 09 48 8D 05 ?? ?? ?? ?? EB 13";

uint64_t GameHandler::baseAddress_ = 0;
uint64_t GameHandler::gWorldOffset_ = 0;
uint64_t GameHandler::gNamesOffset_ = 0;

void GameHandler::Initialize() {
    baseAddress_ = Memory::GetModuleBaseAddress(Globals::gGameExe);
    Logger::Info(std::format("BaseAddress: 0x{:X}", baseAddress_));

    gWorldOffset_ = Memory::SignatureScan(GWORLD_SIGNATURE, Globals::gGameExe);
    gWorldOffset_ = Memory::DereferenceAddress(gWorldOffset_ + 0x8);
    gWorldOffset_ -= baseAddress_;
    gWorldOffset_ += 0x4;
    Logger::Info(std::format("GWORLD Offset: 0x{:X}", gWorldOffset_));

    gNamesOffset_ = Memory::SignatureScan(GNAMES_SIGNATURE, Globals::gGameExe);
    gNamesOffset_ = Memory::DereferenceAddress(gNamesOffset_ + 0x9);
    gNamesOffset_ -= baseAddress_;
    Logger::Info(std::format("GNAMES Offset: 0x{:X}", gNamesOffset_));
}

structs::UWorld *GameHandler::GetWorld() {
    uintptr_t ptr = baseAddress_ + gWorldOffset_;
    if (!ptr) {
        return nullptr;
    }

    uintptr_t worldAddr = *reinterpret_cast<uintptr_t*>(ptr);
    if (!worldAddr) {
        return nullptr;
    }

    return reinterpret_cast<structs::UWorld*>(worldAddr);
}

std::string GameHandler::FNameToString(FName fName) {
    if (fNameCache_.contains(fName.comparisonIndex)) {
        return fNameCache_[fName.comparisonIndex];
    }

    enum { NAME_SIZE = 1024 };
    char name[NAME_SIZE] = { 0 };

    const unsigned int chunkOffset = fName.comparisonIndex >> 16;
    const unsigned short nameOffset = fName.comparisonIndex;

    uint64_t gNames = baseAddress_ + gNamesOffset_;
    uint64_t namePoolChunk = *reinterpret_cast<uint64_t*>(gNames + 0x8 * (chunkOffset + 0x2)) + 0x2 * nameOffset;
    const uint16_t nameLength = *reinterpret_cast<uint16_t*>(namePoolChunk) >> 6;

    std::memcpy(name, reinterpret_cast<void*>(namePoolChunk + 0x2), nameLength);
    std::string finalName = std::string(name);
    if (finalName.empty()) {
        finalName = "Null";
    }

    fNameCache_.insert(std::make_pair(fName.comparisonIndex, finalName));
    return finalName;
}

} // grounded_minimap