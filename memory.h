//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_MEMORY_H
#define GROUNDED_MINIMAP_MEMORY_H

#include <cstdint>
#include <string>

namespace grounded_minimap {

class Memory {
public:
    static uintptr_t SignatureScan(const std::string& signature, const std::string& moduleName);
    static uintptr_t DereferenceAddress(uintptr_t address);
    static uintptr_t GetModuleBaseAddress(const std::string& moduleName);
};

} // grounded_minimap

#endif //GROUNDED_MINIMAP_MEMORY_H
