#include <windows.h>
#include <filesystem>

#include "config.h"
#include "game_handler.h"
#include "globals.h"
#include "logger.h"
#include "utils.h"
#include "hook_helper.h"
#include "minimap.h"

using namespace grounded_minimap;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
DWORD WINAPI OnProcessAttach(LPVOID lpvThreadParameter);
void MainLoop();
void Cleanup();

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    UNREFERENCED_PARAMETER(lpvReserved);

    if (fdwReason == DLL_PROCESS_ATTACH) {
        Globals::gModule = hinstDLL;
        Globals::gDllPath = GetDllDirectory(hinstDLL);
        Globals::gLogFilePath = Globals::gDllPath + "\\grounded_minimap.log";
        Globals::gConfigFilePath = Globals::gDllPath + "\\gm_config.toml";

        Logger::Initialize(Globals::gLogFilePath);
        Logger::Info("Starting Grounded Minimap...");

        DisableThreadLibraryCalls(hinstDLL);
        HANDLE hThread = CreateThread(nullptr, 0, OnProcessAttach, nullptr, 0, nullptr);
        if (hThread) {
            Logger::Info("Thread created successfully");
            CloseHandle(hThread);
        } else {
            Logger::Error("Failed to create thread");
        }
    }

    return TRUE;
}

DWORD WINAPI OnProcessAttach(LPVOID lpvThreadParameter) {
    UNREFERENCED_PARAMETER(lpvThreadParameter);

    Logger::Info("Thread started successfully");

    while (!Globals::gGameWindow) {
        Globals::gGameWindow = FindWindowW(L"UnrealWindow", L"Grounded");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Globals::gGameExe = GetGameExe();
    if (Globals::gGameExe.empty()) {
        Logger::Error("Failed to get game executable name");
    }
    Logger::Info("Game executable: " + Globals::gGameExe);

    Globals::gGameWindowSize = GetWindowSize(Globals::gGameWindow);

    Config::LoadConfig(Globals::gConfigFilePath);
    Config::SaveConfig(Globals::gConfigFilePath);
    // Debug mode needs to be initialized after loading the configuration
    if (Config::debug) {
        Logger::InitializeDebug();
    }

    Logger::Info("Grounded Minimap initialized successfully");

    GameHandler::Initialize();
    HookHelper::Hook();
    Minimap::Initialize();

    MainLoop();

    Cleanup();
    FreeLibraryAndExitThread(Globals::gModule, 0);
}

void MainLoop() {
    auto lastWriteTime = std::filesystem::last_write_time(Globals::gConfigFilePath);
    bool updateConfig = false;

    while (true) {
        if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000 || GetAsyncKeyState(VK_ADD) & 0x8000) {
            Config::zoom += 2;
            updateConfig = true;
        }

        if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000 || GetAsyncKeyState(VK_SUBTRACT) & 0x8000) {
            Config::zoom--;
            if (Config::zoom < 1) {
                Config::zoom = 1;
            }
            updateConfig = true;
        }

        if (updateConfig) {
            Config::SaveConfig(Globals::gConfigFilePath);
            updateConfig = false;
        }

        try {
            auto currentWriteTime = std::filesystem::last_write_time(Globals::gConfigFilePath);
            if (currentWriteTime != lastWriteTime) {
                lastWriteTime = currentWriteTime;

                Logger::Info("Config file updated. Reloading...");
                Config::LoadConfig(Globals::gConfigFilePath);
            }
        } catch (const std::exception& e) {
            Logger::Error("Failed to reload config file: " + std::string(e.what()));
        }

        std::this_thread::yield();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Cleanup() {
    Logger::Info("Shutting down SoulsVision...");
    Minimap::Uninitialize();
    HookHelper::Unhook();

    Logger::Info("SoulsVision shutdown complete");
    Logger::Shutdown();
}