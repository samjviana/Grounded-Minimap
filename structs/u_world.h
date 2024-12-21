//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_U_WORLD_H
#define GROUNDED_MINIMAP_U_WORLD_H

#include "a_game_state_base.h"
#include "u_game_instance.h"
#include "u_object.h"

namespace structs {

class UWorld : public UObject {
public:
    char pad_0x0028[328];               //0x0028
    AGameStateBase* gameState;          //0x0170
    char pad_0x0178[96];                //0x0178
    UGameInstance* owningGameInstance;  //0x01D8
};

} // structs

#endif //GROUNDED_MINIMAP_U_WORLD_H
