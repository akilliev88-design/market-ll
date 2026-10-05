#pragma once
#include "CoreMinimal.h"
class UWorld;
class AActor;
struct FStoreTemplate;
// Physical sales-face clearance check for the independent store review worlds.
bool AuditStoreSalesFaces(UWorld* World, const FStoreTemplate& Store, AActor* Walker);
