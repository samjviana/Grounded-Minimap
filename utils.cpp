//
// Created by PC-SAMUEL on 20/12/2024.
//

#include "utils.h"

namespace grounded_minimap {

std::string GetDllPath(HMODULE hModule) {
    char path[MAX_PATH];
    if (GetModuleFileNameA(hModule, path, MAX_PATH) == 0) {
        return "";
    }

    return {path};
}

std::string GetDllDirectory(HMODULE hModule) {
    std::string path = GetDllPath(hModule);
    size_t lastSlashIndex = path.find_last_of("\\/");
    if (lastSlashIndex == std::string::npos) {
        return "";
    }

    return path.substr(0, lastSlashIndex);
}

} // grounded_minimap