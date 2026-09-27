#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Fonts/SlateFontInfo.h"

// Visual language of the Product Studio: dark graphite surfaces, generous spacing,
// one mint accent, rounded cards (see Docs/Planlama/02_MENU_VE_DENEYIM.md section 1).
struct FStudioStyle
{
    static const FStudioStyle& Get();

    FLinearColor Bg, Surface, Raised, Line, Text, Muted, Faint, Accent, AccentText, Danger, Warn;

    FSlateBrush BgBrush;
    FSlateBrush SurfaceBrush;
    FSlateBrush RaisedBrush;
    FSlateBrush ViewportFrame;
    FSlateBrush PillBrush;
    FSlateBrush SwatchBrush;
    FSlateBrush DividerBrush;

    FButtonStyle Primary;
    FButtonStyle Secondary;
    FButtonStyle Ghost;
    FButtonStyle DangerGhost;
    FButtonStyle Card;
    FButtonStyle CardSelected;
    FButtonStyle Segment;
    FButtonStyle SegmentActive;
    FButtonStyle Tile;
    FButtonStyle Chip;
    FButtonStyle ChipActive;

    FEditableTextBoxStyle Input;

    FSlateFontInfo Font(int32 Size, const TCHAR* Weight = TEXT("Regular")) const;

private:
    FStudioStyle();
};
