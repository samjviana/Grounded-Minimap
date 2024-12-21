//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_A_GAME_STATE_BASE_H
#define GROUNDED_MINIMAP_A_GAME_STATE_BASE_H

#include "u_object.h"

namespace structs {

class AGameStateBase : public UObject {
public:
    char pad_0x0028[568];               //0x0028
    bool replicatedHasBegunPlay;        //0x0260
    char pad_0x0261[3];                 //0x0261
    float replicatedWorldTimeSeconds;   //0x0264
};

} // structs

#endif //GROUNDED_MINIMAP_A_GAME_STATE_BASE_H
