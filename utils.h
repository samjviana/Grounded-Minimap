//
// Created by PC-SAMUEL on 20/12/2024.
//

#ifndef GROUNDED_MINIMAP_UTILS_H
#define GROUNDED_MINIMAP_UTILS_H

#include "shared_types.h"
#include "structs/e_player_character_identity.h"
#include "imgui.h"

#include <d3d12.h>
#include <string>
#include <windows.h>

namespace grounded_minimap {

extern std::string GetDllPath(HMODULE hModule);
extern std::string GetDllDirectory(HMODULE hModule);
extern std::string GetGameExe();
extern ImVec2 GetWindowSize(HWND hwnd);
extern int CountPngResources(HMODULE hModule);
extern bool LoadTextureFromResource(int resourceID, HMODULE module, ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, TextureInfo* textureInfo);
extern bool LoadTextureFromMemory(const void* data, size_t data_size, ID3D12Device* d3d_device, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, TextureInfo* textureInfo);
extern D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHeapStart, SIZE_T descriptorIncrementSize, int index);
extern std::string GetCharacterName(structs::EPlayerCharacterIdentity playerCharacterIdentity);

} // grounded_minimap

#endif //GROUNDED_MINIMAP_UTILS_H
