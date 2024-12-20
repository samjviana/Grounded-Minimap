cmake_minimum_required(VERSION 3.27)
project(grounded_minimap)

set(CMAKE_CXX_STANDARD 23)


set(GAME_DIR "C:/XboxGames/Grounded")
SET(MODS_DIR "${GAME_DIR}/Content/Maine/Binaries/WinGDK/mods")

add_library(grounded_minimap SHARED
        dllmain.cpp
        globals.cpp
        globals.h
        logger.cpp
        logger.h
        utils.cpp
        utils.h
        config.cpp
        config.h
        hook_helper.cpp
        hook_helper.h
        hook_helper.cpp
        hook_helper.h
        minimap.cpp
        minimap.h)

target_link_libraries(grounded_minimap PRIVATE d3d12 dxgi)

add_subdirectory(third_party)

add_custom_command(TARGET grounded_minimap POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:grounded_minimap>" "${MODS_DIR}/")