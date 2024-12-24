//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_A_PAWN_H
#define GROUNDED_MINIMAP_A_PAWN_H

#include "u_object.h"
#include "u_scene_component.h"

namespace structs {

class APawn : public UObject {
public:
    char pad_0x0028[280];           //0x0028
    USceneComponent* rootComponent; //0x0140
};

} // structs

#endif //GROUNDED_MINIMAP_A_PAWN_H
