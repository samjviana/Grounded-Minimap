//
// Created by PC-SAMUEL on 20/12/2024.
//

#define NOMINMAX

#include "minimap.h"
#include "game_handler.h"
#include "globals.h"
#include "hook_helper.h"
#include "logger.h"
#include "resources.h"
#include "structs/u_world.h"
#include "utils.h"
#include "config.h"

#include <cmath>
#include <numbers>
#include <imgui.h>
#include <imgui_internal.h>
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

    textureCount_ = CountPngResources(Globals::gModule) + 1;

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

constexpr float DegreesToRadians(float degrees) {
    return degrees * (std::numbers::pi_v<float> / 180.0f);
}

void ShowRotatedImage(ImTextureID texture, float angle_degrees, ImVec2 size = ImVec2(200, 200), ImVec2 uv1 = ImVec2(0.0f, 0.0f), ImVec2 uv2 = ImVec2(1.0f, 1.0f)) {
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

    float angle_rad = DegreesToRadians(angle_degrees);

    float cos_theta = cosf(angle_rad);
    float sin_theta = sinf(angle_rad);

    ImVec2 top_left(-size.x * 0.5f, -size.y * 0.5f);
    ImVec2 top_right(size.x * 0.5f, -size.y * 0.5f);
    ImVec2 bottom_right(size.x * 0.5f, size.y * 0.5f);
    ImVec2 bottom_left(-size.x * 0.5f, size.y * 0.5f);

    auto rotate = [&](const ImVec2& p) -> ImVec2 {
        return ImVec2(
                p.x * cos_theta - p.y * sin_theta + center.x,
                p.x * sin_theta + p.y * cos_theta + center.y
        );
    };

    ImVec2 rotated_top_left = rotate(top_left);
    ImVec2 rotated_top_right = rotate(top_right);
    ImVec2 rotated_bottom_right = rotate(bottom_right);
    ImVec2 rotated_bottom_left = rotate(bottom_left);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2 uv_top_left = uv1;
    ImVec2 uv_top_right = ImVec2(uv2.x, uv1.y);
    ImVec2 uv_bottom_right = uv2;
    ImVec2 uv_bottom_left = ImVec2(uv1.x, uv2.y);

    ImU32 tint_col = IM_COL32(255, 255, 255, 255); // No tint

    draw_list->AddImageQuad(
            texture,
            rotated_top_left,
            rotated_top_right,
            rotated_bottom_right,
            rotated_bottom_left,
            uv_top_left,
            uv_top_right,
            uv_bottom_right,
            uv_bottom_left,
            tint_col
    );

    float min_x = rotated_top_left.x;
    float max_x = rotated_top_left.x;
    float min_y = rotated_top_left.y;
    float max_y = rotated_top_left.y;

    auto update_bounds = [&](const ImVec2& p) {
        if (p.x < min_x) min_x = p.x;
        if (p.x > max_x) max_x = p.x;
        if (p.y < min_y) min_y = p.y;
        if (p.y > max_y) max_y = p.y;
    };

    update_bounds(rotated_top_right);
    update_bounds(rotated_bottom_right);
    update_bounds(rotated_bottom_left);

    ImGui::SetCursorScreenPos(ImVec2(min_x, max_y));
}

void CircleImage(ImTextureID user_texture_id, float diameter, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col = ImVec4(1, 1, 1, 1)) {
    ImVec2 p_min = ImGui::GetCursorScreenPos();
    ImVec2 p_max = ImVec2(p_min.x + diameter, p_min.y + diameter);
    ImGui::GetWindowDrawList()->AddImageRounded(user_texture_id, p_min, p_max, uv0, uv1, ImGui::GetColorU32(tint_col), diameter * 0.5f);
    ImGui::Dummy(ImVec2(diameter, diameter));
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

    if (gameInstance->localPlayers.count == 0) {
        return;
    }

    ULocalPlayer* localPlayer = gameInstance->localPlayers.data[0];
    if (!localPlayer) {
        return;
    }

    APlayerController* playerController = localPlayer->playerController;
    if (!playerController) {
        return;
    }

    ACharacter* character = playerController->character;
    if (!character) {
        return;
    }

    APawn* pawn = playerController->acknowledgedPawn;
    if (!pawn) {
        return;
    }

    EPlayerCharacterIdentity characterIdentity = character->characterIdentity;
    std::string characterName = GetCharacterName(characterIdentity);
    if (characterName == "unknown") {
        return;
    }

    FVector location = pawn->rootComponent->relativeLocation;
    float tempX = location.x;
    location.x = location.y;
    location.y = -tempX;

    float rotation = pawn->rootComponent->relativeRotation.y;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground;
    ImVec2 windowPosition = Config::position;
    ImVec2 windowSize = Config::size;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowPos(windowPosition);
    ImGui::SetNextWindowSize(windowSize);

    if (!ImGui::Begin("Minimap", nullptr, windowFlags)) {
        ImGui::PopStyleVar();
        return;
    }

    TextureInfo mapTexture = GetTexture("map.png");
    if (!mapTexture.textureResource) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    const float worldMinX = -100000.0f;
    const float worldMinY = -100000.0f;
    const float worldMaxX = 100000.0f;
    const float worldMaxY = 100000.0f;

    float regionWidth = (worldMaxX - worldMinX) / static_cast<float>(Config::zoom);
    float regionHeight = (worldMaxY - worldMinY) / static_cast<float>(Config::zoom);

    float regionMinX = location.x - (regionWidth / 2.0f);
    float regionMinY = location.y - (regionHeight / 2.0f);

    regionMinX = std::max(worldMinX, std::min(regionMinX, worldMaxX - regionWidth));
    regionMinY = std::max(worldMinY, std::min(regionMinY, worldMaxY - regionHeight));

    float uvMinX = (regionMinX - worldMinX) / (worldMaxX - worldMinX);
    float uvMinY = (regionMinY - worldMinY) / (worldMaxY - worldMinY);
    float uvMaxX = (regionMinX + regionWidth - worldMinX) / (worldMaxX - worldMinX);
    float uvMaxY = (regionMinY + regionHeight - worldMinY) / (worldMaxY - worldMinY);

    D3D12_GPU_DESCRIPTOR_HANDLE mapHandle = GetGpuDescriptorHandle(
            srvHeap_->GetGPUDescriptorHandleForHeapStart(),
            device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
            mapTexture.index
    );
    auto mapTexID = static_cast<ImTextureID>(mapHandle.ptr);

    ImGui::SetCursorPos(ImVec2(0, 0));
    CircleImage(mapTexID, windowSize.x, ImVec2(uvMinX, uvMinY), ImVec2(uvMaxX, uvMaxY));

    TextureInfo playerTexture = GetTexture(characterName + ".png");
    if (!playerTexture.textureResource) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE playerHandle = GetGpuDescriptorHandle(
            srvHeap_->GetGPUDescriptorHandleForHeapStart(),
            device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
            playerTexture.index
    );
    auto playerTexID = static_cast<ImTextureID>(playerHandle.ptr);

    ImVec2 playerSize = ImVec2(20, 20);

    float relativeX = (location.x - regionMinX) / regionWidth;
    float relativeY = (location.y - regionMinY) / regionHeight;

    float scaledX = relativeX * windowSize.x;
    float scaledY = relativeY * windowSize.y;

    ImVec2 playerPosition = ImVec2(scaledX - (playerSize.x / 2), scaledY - (playerSize.y / 2));

    ImGui::SetCursorPos(playerPosition);
    ImGui::Image(playerTexID, playerSize);

    ImGui::End();
    ImGui::PopStyleVar();
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

TextureInfo Minimap::GetTexture(const std::string &textureName) {
    if (assets_.find(textureName) != assets_.end()) {
        return assets_[textureName];
    }

    Logger::Error("Texture not found: " + textureName);
    return {};
}

LRESULT Minimap::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    }

    return CallWindowProcW(reinterpret_cast<WNDPROC>(oWndProc_), hWnd, uMsg, wParam, lParam);
}

} // grounded_minimap