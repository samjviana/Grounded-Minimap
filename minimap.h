//
// Created by PC-SAMUEL on 20/12/2024.
//

#ifndef GROUNDED_MINIMAP_MINIMAP_H
#define GROUNDED_MINIMAP_MINIMAP_H


#include "shared_types.h"

#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace grounded_minimap {

class Minimap {
public:
    static void Initialize();
    static void Uninitialize();

private:
    static void Render(IDXGISwapChain3* pSwapChain);
    static void CleanupRenderTargets();
    static void RenderTargets(IDXGISwapChain3 *pSwapChain);
    static void InitializeDXResources(IDXGISwapChain3* pSwapChain);
    static void InitializeFileResources(ID3D12Device* device);
    static void InitializeRenderTargets(IDXGISwapChain3* pSwapChain);
    static void DrawMinimap(ID3D12Device* device);
    static inline int GetCorrectDXGIFormat(int eCurrentFormat);
    static void LoadAllTexturesResources(ID3D12Device* device);

    static ID3D12Resource **renderTargets_;
    static ID3D12CommandQueue* commandQueue_;
    static uint64_t buffersCounts_;
    static ID3D12CommandAllocator **commandAllocators_;
    static ID3D12GraphicsCommandList *commandList_;
    static std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpuDescriptorHandles_;
    static ID3D12DescriptorHeap *srvHeap_;
    static ID3D12DescriptorHeap *rtvHeap_;
    static int textureCount_;
    static uint64_t oWndProc_;
    static std::unordered_map<std::string, TextureInfo> assets_;

    static LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

} // grounded_minimap

#endif //GROUNDED_MINIMAP_MINIMAP_H
