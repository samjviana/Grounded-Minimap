//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_A_CHARACTER_H
#define GROUNDED_MINIMAP_A_CHARACTER_H

#include "e_player_character_identity.h"
#include "u_object.h"

namespace structs {

class ACharacter : public UObject {
public:
    char pad_0x0028[5984];                      //0x0028
    EPlayerCharacterIdentity characterIdentity; //0x1788
};

} // structs

#endif //GROUNDED_MINIMAP_A_CHARACTER_H
