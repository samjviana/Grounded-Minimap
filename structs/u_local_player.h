//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_U_LOCAL_PLAYER_H
#define GROUNDED_MINIMAP_U_LOCAL_PLAYER_H

#include "a_player_controller.h"
#include "u_object.h"

namespace structs {

class ULocalPlayer : public UObject {
public:
    char pad_0x0028[8];                     //0x0028
    APlayerController* playerController;    //0x0030
};

} // structs

#endif //GROUNDED_MINIMAP_U_LOCAL_PLAYER_H
