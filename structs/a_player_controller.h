//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_A_PLAYER_CONTROLLER_H
#define GROUNDED_MINIMAP_A_PLAYER_CONTROLLER_H

#include "a_character.h"
#include "a_pawn.h"
#include "u_object.h"

namespace structs {

class APlayerController : public UObject {
public:
    char pad_0x0028[616];       //0x0028
    ACharacter* character;      //0x0290
    char pad_0x0298[56];        //0x0298
    APawn* acknowledgedPawn;    //0x02D8
};

} // structs

#endif //GROUNDED_MINIMAP_A_PLAYER_CONTROLLER_H
