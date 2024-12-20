//
// Created by PC-SAMUEL on 20/12/2024.
//

#ifndef GROUNDED_MINIMAP_UTILS_H
#define GROUNDED_MINIMAP_UTILS_H

#include <string>
#include <windows.h>

namespace grounded_minimap {

extern std::string GetDllPath(HMODULE hModule);
extern std::string GetDllDirectory(HMODULE hModule);

} // grounded_minimap

#endif //GROUNDED_MINIMAP_UTILS_H
