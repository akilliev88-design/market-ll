#pragma once
#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PreviewScene.h"
#include "SEditorViewport.h"

class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

class FStudioViewportClient : public FEditorViewportClient
{
public:
    FStudioViewportClient(const TSharedRef<FPreviewScene>& InScene, const TSharedRef<SEditorViewport>& InWidget);
    virtual void Tick(float DeltaSeconds) override;
    virtual FLinearColor GetBackgroundColor() const override;
    TFunction<void(float)> OnTickScene;

private:
    TSharedRef<FPreviewScene> SceneRef; // keeps the scene alive as long as the client
};

// Turntable preview of one packaged product (mesh + label material).
class SStudioViewport : public SEditorViewport
{
public:
    SLATE_BEGIN_ARGS(SStudioViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SStudioViewport() override;

    // Materials: per slot override, nullptr = mesh default.
    void ShowItem(UStaticMesh* Mesh, const TArray<UMaterialInterface*>& Materials);
    void SetSpinning(bool bInSpin) { bSpin = bInSpin; }
    bool IsSpinning() const { return bSpin; }
    void ResetView();

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:
    void TickScene(float DeltaSeconds);

    TSharedPtr<FPreviewScene> Scene;
    TSharedPtr<FStudioViewportClient> StudioClient;
    USceneComponent* Turntable = nullptr;
    UStaticMeshComponent* Item = nullptr;
    UStaticMeshComponent* Floor = nullptr;
    FVector LookAt = FVector(0, 0, 10);
    float Distance = 60.f;
    float Yaw = 0.f;
    TWeakObjectPtr<UStaticMesh> LastMesh;
    bool bSpin = true;
};
