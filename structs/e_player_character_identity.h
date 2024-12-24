//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_E_PLAYER_CHARACTER_IDENTITY_H
#define GROUNDED_MINIMAP_E_PLAYER_CHARACTER_IDENTITY_H

#include <cstdint>

namespace structs {

enum EPlayerCharacterIdentity : uint8_t {
    Max = 0,
    Hoops = 1,
    Pete = 2,
    Willow = 3,
    MrBones = 4,
    Count = 5,
    None = 254,
    Random = 255
};

} // structs

#endif //GROUNDED_MINIMAP_E_PLAYER_CHARACTER_IDENTITY_H
