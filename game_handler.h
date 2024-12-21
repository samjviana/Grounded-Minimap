//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_GAME_HANDLER_H
#define GROUNDED_MINIMAP_GAME_HANDLER_H

#include "shared_types.h"
#include "structs/f_name.h"
#include "structs/u_world.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

using namespace structs;

namespace grounded_minimap {

class GameHandler {
public:
    static void Initialize();

    static std::string FNameToString(FName fName);

    static structs::UWorld* GetWorld();
private:

    static std::unordered_map<uint32_t, std::string> fNameCache_;

    static std::string GWORLD_SIGNATURE;
    static std::string GNAMES_SIGNATURE;

    static uint64_t baseAddress_;
    static uint64_t gWorldOffset_;
    static uint64_t gNamesOffset_;
};

} // grounded_minimap

#endif //GROUNDED_MINIMAP_GAME_HANDLER_H
