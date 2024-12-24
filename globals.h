//
// Created by PC-SAMUEL on 20/12/2024.
//

#ifndef GROUNDED_MINIMAP_GLOBALS_H
#define GROUNDED_MINIMAP_GLOBALS_H

#include <cstdint>
#include <string>
#include <imgui.h>
#include <windows.h>
#include <unordered_map>

namespace grounded_minimap {

class Globals {
public:
    static inline HMODULE gModule = nullptr;
    static inline HWND gGameWindow = nullptr;
    static inline std::string gGameExe;
    static inline ImVec2 gGameWindowSize;
    static inline std::string gDllPath;
    static inline std::string gLogFilePath;
    static inline std::string gConfigFilePath;
};

} // grounded_minimap

#endif //GROUNDED_MINIMAP_GLOBALS_H
