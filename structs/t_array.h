//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_T_ARRAY_H
#define GROUNDED_MINIMAP_T_ARRAY_H

#include <cstdint>

namespace structs {

template <typename T>
class TArray {
public:
    T* data;
    int32_t count;
    int32_t max;
};

} // structs

#endif //GROUNDED_MINIMAP_T_ARRAY_H
