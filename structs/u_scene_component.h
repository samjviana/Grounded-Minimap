//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_U_SCENE_COMPONENT_H
#define GROUNDED_MINIMAP_U_SCENE_COMPONENT_H

#include "f_vector.h"
#include "u_object.h"

namespace structs {

class USceneComponent : public UObject {
public:
    char pad_0x0028[252];       //0x0028
    FVector relativeLocation;   //0x0124
    FVector relativeRotation;   //0x0130
};

} // structs

#endif //GROUNDED_MINIMAP_U_SCENE_COMPONENT_H
