//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_U_WIDGET_MANAGER_H
#define GROUNDED_MINIMAP_U_WIDGET_MANAGER_H

#include "t_array.h"
#include "u_object.h"

namespace structs {

class UWidgetManager : public UObject {
public:
    char pad_0x0028[40];            //0x0028
    TArray<UObject*> windowStack;   //0x0050
};

} // structs

#endif //GROUNDED_MINIMAP_U_WIDGET_MANAGER_H
