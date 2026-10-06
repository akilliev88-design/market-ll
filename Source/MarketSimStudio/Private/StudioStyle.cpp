#include "StudioStyle.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

namespace
{
    FLinearColor Hex(const TCHAR* Value) { return FLinearColor(FColor::FromHex(Value)); }

    FSlateBrush Rounded(const FLinearColor& Color, float Radius, const FLinearColor& Outline = FLinearColor::Transparent, float OutlineWidth = 0.f)
    {
        return FSlateRoundedBoxBrush(Color, Radius, Outline, OutlineWidth);
    }

    FButtonStyle Button(const FSlateBrush& Normal, const FSlateBrush& Hovered, const FSlateBrush& Pressed, const FSlateBrush& Disabled)
    {
        return FButtonStyle()
            .SetNormal(Normal).SetHovered(Hovered).SetPressed(Pressed).SetDisabled(Disabled)
            .SetNormalPadding(FMargin(0)).SetPressedPadding(FMargin(0));
    }
}

const FStudioStyle& FStudioStyle::Get()
{
    static const FStudioStyle Instance;
    return Instance;
}

FSlateFontInfo FStudioStyle::Font(int32 Size, const TCHAR* Weight) const
{
    return FCoreStyle::GetDefaultFontStyle(FName(Weight), Size);
}

FStudioStyle::FStudioStyle()
{
    Bg = Hex(TEXT("0E0F11"));
    Surface = Hex(TEXT("16181B"));
    Raised = Hex(TEXT("1F2227"));
    Line = Hex(TEXT("2B2F35"));
    Text = Hex(TEXT("F4F5F6"));
    Muted = Hex(TEXT("9AA3AD"));
    Faint = Hex(TEXT("5E6670"));
    Accent = Hex(TEXT("71C6AC"));
    AccentText = Hex(TEXT("0B1F19"));
    Danger = Hex(TEXT("E5484D"));
    Warn = Hex(TEXT("F2B84B"));

    BgBrush = FSlateColorBrush(Bg);
    SurfaceBrush = Rounded(Surface, 18.f);
    RaisedBrush = Rounded(Raised, 14.f);
    ViewportFrame = Rounded(Surface, 22.f, Line, 1.f);
    PillBrush = Rounded(Raised, 999.f);
    SwatchBrush = Rounded(FLinearColor::White, 10.f);
    DividerBrush = FSlateColorBrush(Line);

    const FLinearColor AccentHover = Hex(TEXT("86D4BD"));
    const FLinearColor AccentPressed = Hex(TEXT("5BAE95"));
    Primary = Button(Rounded(Accent, 12.f), Rounded(AccentHover, 12.f), Rounded(AccentPressed, 12.f), Rounded(Hex(TEXT("2A3A35")), 12.f));
    Secondary = Button(Rounded(Raised, 12.f, Line, 1.f), Rounded(Hex(TEXT("272B31")), 12.f, Line, 1.f), Rounded(Hex(TEXT("1A1D21")), 12.f, Line, 1.f), Rounded(Surface, 12.f));
    Ghost = Button(Rounded(FLinearColor::Transparent, 10.f), Rounded(Raised, 10.f), Rounded(Line, 10.f), Rounded(FLinearColor::Transparent, 10.f));
    DangerGhost = Button(Rounded(FLinearColor::Transparent, 12.f, Hex(TEXT("4A2527")), 1.f), Rounded(Hex(TEXT("2A1719")), 12.f, Danger, 1.f),
                         Rounded(Hex(TEXT("3A1C1F")), 12.f, Danger, 1.f), Rounded(FLinearColor::Transparent, 12.f));
    Card = Button(Rounded(FLinearColor::Transparent, 14.f), Rounded(Raised, 14.f), Rounded(Line, 14.f), Rounded(FLinearColor::Transparent, 14.f));
    CardSelected = Button(Rounded(Raised, 14.f, Accent, 1.f), Rounded(Raised, 14.f, Accent, 1.f), Rounded(Line, 14.f, Accent, 1.f), Rounded(Raised, 14.f));
    Segment = Button(Rounded(FLinearColor::Transparent, 10.f), Rounded(Hex(TEXT("24282D")), 10.f), Rounded(Line, 10.f), Rounded(FLinearColor::Transparent, 10.f));
    SegmentActive = Button(Rounded(Hex(TEXT("2C3137")), 10.f), Rounded(Hex(TEXT("31363D")), 10.f), Rounded(Line, 10.f), Rounded(Hex(TEXT("2C3137")), 10.f));
    Tile = Button(Rounded(Raised, 14.f, Line, 1.f), Rounded(Hex(TEXT("262A30")), 14.f, Accent, 1.f), Rounded(Line, 14.f, Accent, 1.f), Rounded(Surface, 14.f));
    Chip = Button(Rounded(Raised, 999.f, Line, 1.f), Rounded(Hex(TEXT("262A30")), 999.f, Line, 1.f), Rounded(Line, 999.f), Rounded(Surface, 999.f));
    ChipActive = Button(Rounded(Hex(TEXT("1D302A")), 999.f, Accent, 1.f), Rounded(Hex(TEXT("223A33")), 999.f, Accent, 1.f), Rounded(Line, 999.f, Accent, 1.f), Rounded(Surface, 999.f));

    Input = FAppStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("NormalEditableTextBox"));
    Input.SetBackgroundImageNormal(Rounded(Hex(TEXT("121417")), 10.f, Line, 1.f))
         .SetBackgroundImageHovered(Rounded(Hex(TEXT("121417")), 10.f, Hex(TEXT("3A4048")), 1.f))
         .SetBackgroundImageFocused(Rounded(Hex(TEXT("121417")), 10.f, Accent, 1.f))
         .SetBackgroundImageReadOnly(Rounded(Surface, 10.f, Line, 1.f))
         .SetForegroundColor(FSlateColor(Text))
         .SetPadding(FMargin(12.f, 9.f));
}
