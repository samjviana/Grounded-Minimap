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
        minimap.h
        resources.h
        resources.rc
        shared_types.cpp
        shared_types.h
        game_handler.cpp
        game_handler.h
        memory.cpp
        memory.h
        structs/f_name.cpp
        structs/f_name.h
        structs/u_world.cpp
        structs/u_world.h
        structs/u_object.cpp
        structs/u_object.h
        structs/u_game_instance.cpp
        structs/u_game_instance.h
        structs/t_array.cpp
        structs/t_array.h
        structs/u_widget_manager.cpp
        structs/u_widget_manager.h
        structs/a_game_state_base.cpp
        structs/a_game_state_base.h
        structs/u_local_player.cpp
        structs/u_local_player.h
        structs/a_player_controller.cpp
        structs/a_player_controller.h
        structs/a_character.cpp
        structs/a_character.h
        structs/a_pawn.cpp
        structs/a_pawn.h
        structs/e_player_character_identity.cpp
        structs/e_player_character_identity.h
        structs/u_scene_component.cpp
        structs/u_scene_component.h
        structs/f_vector.cpp
        structs/f_vector.h)

target_link_libraries(grounded_minimap PRIVATE d3d12 dxgi)

add_subdirectory(third_party)

set_source_files_properties(resources.rc PROPERTIES LANGUAGE RC)

add_custom_command(TARGET grounded_minimap POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:grounded_minimap>" "${MODS_DIR}/")