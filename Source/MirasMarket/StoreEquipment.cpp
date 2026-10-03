#include "StoreEquipment.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

const TMap<FString, FPlanogramEquipment>& StoreEquipment::Registry()
{
    static const TMap<FString, FPlanogramEquipment> Data = []()
    {
        TMap<FString, FPlanogramEquipment> Result;
        TArray<FString> Files;
        IFileManager::Get().FindFilesRecursive(Files, *(FPaths::ProjectDir() / TEXT("AssetInbox/Environment/Stores")), TEXT("equipment.json"), true, false);
        Files.Sort();
        for (const FString& Path : Files)
        {
            FString Text; TSharedPtr<FJsonObject> Root;
            if (!FFileHelper::LoadFileToString(Text, *Path) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root)) continue;
            const TSharedPtr<FJsonObject>* Plan = nullptr;
            if (!Root->TryGetObjectField(TEXT("planogram"), Plan)) continue;
            FPlanogramEquipment E;
            E.Id = Root->GetStringField(TEXT("id"));
            E.MeshPath = Root->GetStringField(TEXT("unrealMesh"));
            E.Family = Root->GetStringField(TEXT("family"));
            const auto D = Root->GetObjectField(TEXT("dimensionsMm"));
            E.DimensionsCm = FVector(D->GetNumberField(TEXT("width")), D->GetNumberField(TEXT("depth")), D->GetNumberField(TEXT("height"))) / 10;
            E.Levels = 0;
            E.bDoubleSided = false;
            (*Plan)->TryGetBoolField(TEXT("doubleSided"), E.bDoubleSided);
            E.UsableWidthCm = (*Plan)->GetNumberField(TEXT("widthCm"));
            E.UsableDepthCm = (*Plan)->GetNumberField(TEXT("depthCm"));
            E.FrontY = (*Plan)->GetNumberField(TEXT("frontY"));
            E.MeshYaw = (*Plan)->GetNumberField(TEXT("meshYaw"));
            E.SignZ = E.DimensionsCm.Z + 12;
            E.SignWidthCm = E.DimensionsCm.X - 10;
            (*Plan)->TryGetBoolField(TEXT("signOnTop"), E.bSignOnTop);
            (*Plan)->TryGetBoolField(TEXT("showCategorySign"), E.bShowCategorySign);
            (*Plan)->TryGetNumberField(TEXT("signZ"), E.SignZ);
            (*Plan)->TryGetNumberField(TEXT("signY"), E.SignY);
            (*Plan)->TryGetNumberField(TEXT("signWidthCm"), E.SignWidthCm);
            (*Plan)->TryGetNumberField(TEXT("railAboveTopZ"), E.RailAboveTopZ);
            const auto& Zones = Root->GetArrayField(TEXT("zones"));
            for (const auto& Value : Zones)
            {
                const auto Zone = Value->AsObject();
                if (Zone->GetStringField(TEXT("face")) != TEXT("front")) continue;
                const int32 Level = Zone->GetIntegerField(TEXT("level"));
                if (Level < 0 || Level >= MarketPlanogram::MaxLevels) continue;
                E.Levels = FMath::Max(E.Levels, Level + 1);
                E.LevelTopZ[Level] = Zone->GetArrayField(TEXT("centerCm"))[2]->AsNumber();
                E.LevelClearanceCm[Level] = Zone->GetNumberField(TEXT("clearanceHeightCm"));
                E.RailFrontY[Level] = FMath::Abs(E.FrontY) + 2;
                (*Plan)->TryGetNumberField(TEXT("railFrontY"), E.RailFrontY[Level]);
            }
            Root->TryGetNumberField(TEXT("checkouts"), E.CheckoutCount);
            if (Result.Contains(E.Id)) { UE_LOG(LogTemp, Error, TEXT("Duplicate store equipment: %s"), *E.Id); }
            else Result.Add(E.Id, E);
        }
        return Result;
    }();
    return Data;
}
