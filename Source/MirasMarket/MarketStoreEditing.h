#pragma once
#include "MarketStoreKit.h"
namespace MarketStoreEditing
{
    MIRASMARKET_API FVector2D HalfSize(const FPlanogramFixture& Fixture);
    // Moves are rejected when they intersect fixtures, structures, depot, or floor recesses.
    MIRASMARKET_API bool CanPlace(const FStoreTemplate& Store,const FPlanogramFixture& Fixture,int32 Ignore=INDEX_NONE);
    MIRASMARKET_API FVector Snap(const FStoreTemplate& Store,const FPlanogramFixture& Fixture,bool Walls,bool Neighbours,float GridCm,float GapCm=2);
    MIRASMARKET_API bool MoveGroup(FStoreTemplate& Store,const TSet<int32>& Selection,FVector Delta,bool Walls,bool Neighbours,float GridCm,float SnapRangeCm=35);
    MIRASMARKET_API bool MoveDoor(FStoreTemplate& Store,bool Receiving,FVector Desired);
    MIRASMARKET_API bool Place(FStoreTemplate& Store,const FString& Equipment,const FString& Category,FVector At,int32& Selected,bool Walls=true,bool Neighbours=true,float GridCm=10,float Yaw=0,float GapCm=2,float SnapRangeCm=35);
    MIRASMARKET_API int32 Pick(const FStoreTemplate& Store,FVector Origin,FVector Direction);
    MIRASMARKET_API bool Create(const FString& Format,const FString& Id,const FString& Name,FStoreTemplate& Store);
    MIRASMARKET_API bool Duplicate(FStoreTemplate& Store,int32 Selected,float GapCm,int32& NewSelection);
    MIRASMARKET_API bool Resize(FStoreTemplate& Store,double WidthCm,double DepthCm,double DepotWidthCm,double DepotDepthCm,double HeightCm,FString& Error);
    MIRASMARKET_API bool Save(const FStoreTemplate& Store,const FString& Path,FString& Error);
    MIRASMARKET_API bool LoadDraft(const FString& Path,FStoreTemplate& Store,FString& Error);
    MIRASMARKET_API void BuildArchitecture(UWorld* World,const FStoreTemplate& Store);
}
