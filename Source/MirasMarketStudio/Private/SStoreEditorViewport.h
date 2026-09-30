#pragma once
#include "SStudioViewport.h"
class SStoreStudio;
struct FStoreTemplate;
class FStoreEditorViewportClient;
class SStoreEditorViewport : public SEditorViewport
{
public:
    SLATE_BEGIN_ARGS(SStoreEditorViewport){} SLATE_ARGUMENT(SStoreStudio*,Owner) SLATE_END_ARGS()
    void Construct(const FArguments&);
    virtual ~SStoreEditorViewport() override;
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    void Show(const FStoreTemplate&,bool Fill);
    void SetWalk(bool Walk);
    void Focus(bool Selection=false);
    void SetLighting(bool Lit);
    void SyncSelected();
    bool ReviewInteraction(FString& Error);
    TSharedPtr<FPreviewScene> Scene;
    TSharedPtr<FStoreEditorViewportClient> Client;
    SStoreStudio* Owner=nullptr;
    bool bWalk=false;
    bool bFilled=false;
    int32 FillSeed=1;
    FString LastId;
};
