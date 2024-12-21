//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_U_OBJECT_H
#define GROUNDED_MINIMAP_U_OBJECT_H

#include "f_name.h"

#include <cstdint>

namespace structs {

class UObject {
public:
    uintptr_t vtable;
    uint32_t objectFlags;
    int32_t internalIndex;
    uintptr_t classPrivate;
    FName namePrivate;
    uintptr_t outerPrivate;
};

} // structs

#endif //GROUNDED_MINIMAP_U_OBJECT_H
