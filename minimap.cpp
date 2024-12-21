//
// Created by PC-SAMUEL on 20/12/2024.
//

#include "minimap.h"
#include "game_handler.h"
#include "globals.h"
#include "hook_helper.h"
#include "logger.h"
#include "resources.h"
#include "structs/u_world.h"
#include "utils.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

// This needs to be defined globally
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace structs;

namespace grounded_minimap {

ID3D12Resource** Minimap::renderTargets_ = nullptr;
ID3D12CommandQueue* Minimap::commandQueue_ = nullptr;
uint64_t Minimap::buffersCounts_ = 0;
ID3D12CommandAllocator** Minimap::commandAllocators_ = nullptr;
ID3D12GraphicsCommandList* Minimap::commandList_ = nullptr;
std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Minimap::cpuDescriptorHandles_;
ID3D12DescriptorHeap* Minimap::srvHeap_ = nullptr;
ID3D12DescriptorHeap* Minimap::rtvHeap_ = nullptr;
int Minimap::textureCount_;
uint64_t Minimap::oWndProc_ = 0;
std::unordered_map<std::string, TextureInfo> Minimap::assets_;

void Minimap::Initialize() {
    HookHelper::SetRenderCallback(Render);
    HookHelper::SetCleanupCallback(CleanupRenderTargets);
    HookHelper::SetCommandQueue(&commandQueue_);

    textureCount_ = CountPngResources(Globals::gModule);

    if (ImGui::GetCurrentContext()) {
        return;
    }

    if (!ImGui::CreateContext()) {
        Logger::Error("Failed to create ImGui context");
        return;
    }
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;

    if (!ImGui_ImplWin32_Init(Globals::gGameWindow)) {
        Logger::Error("Failed to initialize ImGui for Win32");
        return;
    }

    Logger::Info("Minimap Overlay initialized successfully.");
}

void Minimap::Uninitialize() {
    if (!ImGui::GetCurrentContext()) {
        return;
    }

    if (ImGui::GetIO().BackendPlatformUserData) {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
    }
    ImGui::DestroyContext();
    Logger::Info("ImGui platforms shutdown and context destroyed.");

    CleanupRenderTargets();

    Logger::Info("Minimap Overlay uninitialized successfully.");

}

void Minimap::Render(IDXGISwapChain3 *pSwapChain) {
    if (commandQueue_ == nullptr) {
        Logger::Error("Command queue is null.");
        return;
    }

    ID3D12Device* device;
    if (pSwapChain->GetDevice(IID_PPV_ARGS(&device)) != S_OK) {
        Logger::Error("Failed to get ID3D12Device from SwapChain.");
        return;
    }

    InitializeDXResources(pSwapChain);
    InitializeFileResources(device);
    InitializeRenderTargets(pSwapChain);

    if (!ImGui::GetCurrentContext()) {
        device->Release();
        return;
    }


    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DrawMinimap(device);

    ImGui::EndFrame();

    RenderTargets(pSwapChain);
}

void Minimap::CleanupRenderTargets() {
    if (!renderTargets_) return;
    for (UINT i = 0; i < buffersCounts_; ++i) {
        if (renderTargets_[i]) {
            renderTargets_[i]->Release();
            renderTargets_[i] = nullptr;
        }
    }
    delete[] renderTargets_;
    renderTargets_ = nullptr;
}

void Minimap::RenderTargets(IDXGISwapChain3 *pSwapChain) {
    UINT bufferIndex = pSwapChain->GetCurrentBackBufferIndex();
    ID3D12CommandAllocator *commandAllocator = commandAllocators_[bufferIndex];
    commandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = renderTargets_[bufferIndex];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList_->Reset(commandAllocator, nullptr);
    commandList_->ResourceBarrier(1, &barrier);
    commandList_->OMSetRenderTargets(1, &cpuDescriptorHandles_[bufferIndex], FALSE, nullptr);
    commandList_->SetDescriptorHeaps(1, &srvHeap_);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList_->ResourceBarrier(1, &barrier);
    commandList_->Close();

    commandQueue_->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList **>(&commandList_));

}

void Minimap::InitializeDXResources(IDXGISwapChain3 *pSwapChain) {
    DXGI_SWAP_CHAIN_DESC sd;
    pSwapChain->GetDesc(&sd);

    if (ImGui::GetIO().BackendRendererUserData) {
        return;
    }

    ID3D12Device* device;
    if (pSwapChain->GetDevice(IID_PPV_ARGS(&device)) != S_OK)
        return;

    buffersCounts_ = sd.BufferCount;

    cpuDescriptorHandles_.clear();

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = textureCount_ + 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_)) != S_OK) {
            device->Release();
            Logger::Error("Failed to create SRV heap.");
            return;
        }
    }
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = static_cast<UINT>(buffersCounts_);
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap_)) != S_OK) {
            srvHeap_->Release();
            device->Release();
            return;
        }

        const SIZE_T rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
        commandAllocators_ = new ID3D12CommandAllocator *[buffersCounts_];
        for (int i = 0; i < buffersCounts_; ++i) {
            cpuDescriptorHandles_.push_back(rtvHandle);
            rtvHandle.ptr += rtvDescriptorSize;
        }
        device->Release();
    }

    for (UINT i = 0; i < sd.BufferCount; ++i) {
        if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                           IID_PPV_ARGS(&commandAllocators_[i])) != S_OK) {
            srvHeap_->Release();
            for (UINT j = 0; j < i; ++j) {
                commandAllocators_[j]->Release();
            }
            rtvHeap_->Release();
            delete[] commandAllocators_;
            device->Release();
            return;
        }
    }

    if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators_[0], nullptr,
                                  IID_PPV_ARGS(&commandList_)) != S_OK ||
        commandList_->Close() != S_OK) {
        srvHeap_->Release();
        for (UINT i = 0; i < buffersCounts_; ++i)
            commandAllocators_[i]->Release();
        rtvHeap_->Release();
        delete[] commandAllocators_;
        device->Release();
        return;
    }

    ImGui_ImplDX12_Init(device, static_cast<int>(buffersCounts_),
                        DXGI_FORMAT_R8G8B8A8_UNORM, srvHeap_,
                        srvHeap_->GetCPUDescriptorHandleForHeapStart(),
                        srvHeap_->GetGPUDescriptorHandleForHeapStart());
    ImGui::GetMainViewport()->PlatformHandleRaw = Globals::gGameWindow;
    oWndProc_ = SetWindowLongPtrW(Globals::gGameWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

void Minimap::InitializeFileResources(ID3D12Device *device) {
    if (assets_.empty()) {
        LoadAllTexturesResources(device);
    }
}

void Minimap::InitializeRenderTargets(IDXGISwapChain3 *pSwapChain) {
    DXGI_SWAP_CHAIN_DESC sd;
    pSwapChain->GetDesc(&sd);

    if (renderTargets_) {
        return;
    }

    ID3D12Device* device;
    if (pSwapChain->GetDevice(IID_PPV_ARGS(&device)) != S_OK)
        return;
    renderTargets_ = new ID3D12Resource *[buffersCounts_];
    for (UINT i = 0; i < buffersCounts_; i++) {
        ID3D12Resource *buffer;
        if (pSwapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)) != S_OK) {
            continue;
        }
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        device->CreateRenderTargetView(buffer, &desc, cpuDescriptorHandles_[i]);
        renderTargets_[i] = buffer;
    }
    if (renderTargets_[0] == nullptr) {
        for (UINT i = 0; i < buffersCounts_; ++i) {
            if (renderTargets_[i])
                renderTargets_[i]->Release();
        }
        delete[] renderTargets_;
        renderTargets_ = nullptr;
    }
    device->Release();
}

void Minimap::DrawMinimap(ID3D12Device *device) {
    UWorld* world = GameHandler::GetWorld();
    if (!world) {
        return;
    }

    std::string name = GameHandler::FNameToString(world->namePrivate);
    if (name == "MainMenu" || name == "None" || name == "Null" || name == "CompanyIntro") {
        return;
    }

    UGameInstance* gameInstance = world->owningGameInstance;
    if (!gameInstance) {
        return;
    }
    UWidgetManager* widgetManager = gameInstance->widgetManager;
    if (!widgetManager) {
        return;
    }

    if (widgetManager->windowStack.count != 0) {
        return;
    }

    AGameStateBase* gameState = world->gameState;
    if (!gameState) {
        return;
    }

    if (!gameState->replicatedHasBegunPlay || gameState->replicatedWorldTimeSeconds <= 0.0f) {
        return;
    }

    // Hello, world!
    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::End();
}

int Minimap::GetCorrectDXGIFormat(int eCurrentFormat) {
    if (eCurrentFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    return eCurrentFormat;
}

void Minimap::LoadAllTexturesResources(ID3D12Device *device) {
    std::unordered_map<std::string, int> resources = {
            {"map.png", IDR_MAP_PNG},
            {"hoops.png", IDR_HOOPS_PNG},
            {"max.png", IDR_MAX_PNG},
            {"pete.png", IDR_PETE_PNG},
            {"willow.png", IDR_WILLOW_PNG}
    };

    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = srvHeap_->GetCPUDescriptorHandleForHeapStart();
    SIZE_T descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    srvCpuHandle.ptr += descriptorSize;

    int textureIndex = 1;
    for (const auto& [fileName, resourceID] : resources) {
        TextureInfo textureInfo = {};

        if (LoadTextureFromResource(resourceID, Globals::gModule, device, srvCpuHandle, &textureInfo)) {
            textureInfo.index = textureIndex++;
            assets_[fileName] = textureInfo;
            Logger::Info(std::format("Loaded texture: {}", fileName));

            srvCpuHandle.ptr += descriptorSize;
        } else {
            Logger::Error(std::format("Texture {} failed to load", fileName));
        }
    }
}

LRESULT Minimap::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    }

    return CallWindowProcW(reinterpret_cast<WNDPROC>(oWndProc_), hWnd, uMsg, wParam, lParam);
}

} // grounded_minimap