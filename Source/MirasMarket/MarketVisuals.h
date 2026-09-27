#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UObject;
class UWorld;

namespace MarketVisuals
{
    UMaterialInterface* CreatePackageSurface(UObject* Outer, UMaterialInterface* Source, const FString& SlotName);
    void BuildStoreLighting(UWorld* World, float BackWallY);
}
