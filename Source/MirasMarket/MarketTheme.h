#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"

// The design language of the menu and the HUD (G-086d, tasarim tuvali "Miras Market Ana Ekran", cizimler 4 ve 5):
// IBM Plex Sans for text, IBM Plex Mono for numbers, Bricolage Grotesque for titles; line icons; a soft shadow under
// light surfaces. Files: Content/Slate/Fonts (SIL Open Font License, the licences sit next to the fonts),
// Content/Slate/Icons (white SVG, tinted where drawn), Content/Slate/Shadow.png. A missing file falls back to the
// engine font / no icon, so the game still runs without them.
namespace MarketTheme
{
    enum class EFace : uint8 { Regular, Medium, Semi, Bold, Mono, MonoSemi, Display, DisplayBold };

    // Pixels of the design boards (1440 wide) -> a Slate font.
    FSlateFontInfo Font(EFace Face, float Pixels);
    // An icon by name ("store", "bell", "play"...); null when the file is missing.
    const FSlateBrush* Icon(const FString& Name);
    // The 9-slice soft shadow (null when missing).
    const FSlateBrush* Shadow();
    FLinearColor Hex(const TCHAR* Code, float Alpha = 1.f);
}
