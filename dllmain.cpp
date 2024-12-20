#include <windows.h>

#include "config.h"
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

    Config::LoadConfig(Globals::gConfigFilePath);
    Config::SaveConfig(Globals::gConfigFilePath);
    // Debug mode needs to be initialized after loading the configuration
    if (Config::debug) {
        Logger::InitializeDebug();
    }

    Logger::Info("Grounded Minimap initialized successfully");

    while (!Globals::gGameWindow) {
        Globals::gGameWindow = FindWindowW(L"UnrealWindow", L"Grounded");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    HookHelper::Hook();
    Minimap::Initialize();

    MainLoop();

    Cleanup();
    FreeLibraryAndExitThread(Globals::gModule, 0);

    return 0;
}

void MainLoop() {
    while (true) {
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