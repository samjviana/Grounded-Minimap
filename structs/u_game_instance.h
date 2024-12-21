//
// Created by PC-SAMUEL on 21/12/2024.
//

#ifndef GROUNDED_MINIMAP_U_GAME_INSTANCE_H
#define GROUNDED_MINIMAP_U_GAME_INSTANCE_H

#include "t_array.h"
#include "u_object.h"
#include "u_widget_manager.h"

namespace structs {

class UGameInstance : public UObject {
public:
    char pad_0x0028[16];            //0x0028
    TArray<UObject*> localPlayers;  //0x0038
    char pad_0x0048[800];           //0x0048
    UWidgetManager* widgetManager;  //0x0368
};

} // structs

#endif //GROUNDED_MINIMAP_U_GAME_INSTANCE_H
