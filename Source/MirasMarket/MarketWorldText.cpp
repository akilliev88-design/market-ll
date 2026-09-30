#include "MarketWorldText.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Font.h"
#include "Materials/MaterialInstanceDynamic.h"
void MarketWorldText::Apply(UTextRenderComponent* Text)
{
    UFont* Font = LoadObject<UFont>(nullptr, TEXT("/Game/Stores/Fonts/F_PlexTurkish.F_PlexTurkish"));
    if (!Font) { UE_LOG(LogTemp, Error, TEXT("Turkish world-text font missing; run Tools/create_turkish_font.py")); return; }
    Text->SetFont(Font);
    auto* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Stores/Fonts/M_PlexText.M_PlexText"));
    UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base ? Base : Text->GetMaterial(0), Text);
    Material->SetFontParameterValue(TEXT("Font"), Font, 0);
    Text->SetTextMaterial(Material);
}
