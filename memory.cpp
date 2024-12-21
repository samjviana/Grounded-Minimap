//
// Created by PC-SAMUEL on 21/12/2024.
//

#include "memory.h"

#include <sstream>
#include <vector>

#include <windows.h>
#include <psapi.h>

namespace grounded_minimap {

uintptr_t Memory::SignatureScan(const std::string &signature, const std::string &moduleName) {
    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    if (!hModule) {
        return 0;
    }

    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO))) {
        return 0;
    }

    auto startAddress = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    DWORD moduleSize = modInfo.SizeOfImage;

    std::vector<uint8_t> patternBytes;
    std::vector<bool> patternMask;
    std::istringstream iss(signature);
    std::string token;

    while (iss >> token) {
        if (token == "?" || token == "??") {
            patternBytes.push_back(0);
            patternMask.push_back(false);
        } else {
            patternBytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
            patternMask.push_back(true);
        }
    }

    size_t patternLength = patternBytes.size();

    for (uintptr_t i = startAddress; i <= startAddress + moduleSize - patternLength; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternLength; ++j) {
            if (patternMask[j] && *(uint8_t*)(i + j) != patternBytes[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return i;
        }
    }
    return 0;
}

uintptr_t Memory::DereferenceAddress(uintptr_t address) {
    int offset = *reinterpret_cast<int*>(address + 0x3);
    address += 0x7 + offset;

    return address;
}

uintptr_t Memory::GetModuleBaseAddress(const std::string &moduleName) {
    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    if (!hModule) {
        return 0;
    }

    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(MODULEINFO))) {
        return 0;
    }

    return reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
}

} // grounded_minimap