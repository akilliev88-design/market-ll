#pragma once
#include "MarketStoreKit.h"
namespace MarketStoreEditing
{
    MARKETSIM_API FVector2D HalfSize(const FPlanogramFixture& Fixture);
    // Moves are rejected when they intersect fixtures, structures, depot, or floor recesses.
    MARKETSIM_API bool CanPlace(const FStoreTemplate& Store,const FPlanogramFixture& Fixture,int32 Ignore=INDEX_NONE);
    MARKETSIM_API FVector Snap(const FStoreTemplate& Store,const FPlanogramFixture& Fixture,bool Walls,bool Neighbours,float GridCm,float GapCm=2);
    MARKETSIM_API bool MoveGroup(FStoreTemplate& Store,const TSet<int32>& Selection,FVector Delta,bool Walls,bool Neighbours,float GridCm,float SnapRangeCm=35);
    MARKETSIM_API bool MoveDoor(FStoreTemplate& Store,bool Receiving,FVector Desired);
    MARKETSIM_API bool MoveDepotDoor(FStoreTemplate& Store,FVector Desired);
    MARKETSIM_API FStorePoint DepotDoor(const FStoreTemplate& Store);
    MARKETSIM_API bool GeometryValid(const FStoreTemplate& Store,FString& Error);
    MARKETSIM_API bool MoveWall(FStoreTemplate& Store,bool Depot,int32 Edge,FVector Desired,FString& Error);
    MARKETSIM_API bool EditObstacle(FStoreTemplate& Store,int32 Index,FVector At,FVector Size,FString Shape,FString& Error);
    MARKETSIM_API bool SaveTour(const FStoreTemplate& Store,FString& Error);
    MARKETSIM_API TArray<FString> TourIds();
    MARKETSIM_API bool LoadTour(const FString& Id,FStoreTemplate& Store,FString& Error);
    MARKETSIM_API bool Place(FStoreTemplate& Store,const FString& Equipment,const FString& Category,FVector At,int32& Selected,bool Walls=true,bool Neighbours=true,float GridCm=10,float Yaw=0,float GapCm=2,float SnapRangeCm=35);
    MARKETSIM_API int32 Pick(const FStoreTemplate& Store,FVector Origin,FVector Direction);
    MARKETSIM_API bool Create(const FString& Format,const FString& Id,const FString& Name,FStoreTemplate& Store);
    MARKETSIM_API bool Duplicate(FStoreTemplate& Store,int32 Selected,float GapCm,int32& NewSelection);
    MARKETSIM_API bool Resize(FStoreTemplate& Store,double WidthCm,double DepthCm,double DepotWidthCm,double DepotDepthCm,double HeightCm,FString& Error);
    MARKETSIM_API bool Save(const FStoreTemplate& Store,const FString& Path,FString& Error);
    MARKETSIM_API bool LoadDraft(const FString& Path,FStoreTemplate& Store,FString& Error);
    MARKETSIM_API void BuildArchitecture(UWorld* World,const FStoreTemplate& Store);
}
