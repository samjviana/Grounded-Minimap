//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_SHARED_TYPES_H
#define GROUNDED_MINIMAP_SHARED_TYPES_H

#include <cstdint>
#include <d3d12.h>
#include <string>

namespace grounded_minimap {

struct TextureInfo {
    ID3D12Resource* textureResource = nullptr;
    int index = -1;
    int width = 0;
    int height = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = {};
};

struct StringLocation {
    std::string stringFound;
    uint64_t position;
    std::string sectionName;
};

} // grounded_minimap

#endif //GROUNDED_MINIMAP_SHARED_TYPES_H
