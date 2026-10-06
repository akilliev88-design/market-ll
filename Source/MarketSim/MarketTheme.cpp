#include "MarketTheme.h"

#include "Brushes/SlateBoxBrush.h"
#include "Brushes/SlateImageBrush.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"

namespace MarketTheme
{
    FString SlateDir()
    {
        return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Slate"));
    }

    FSlateFontInfo Font(EFace Face, float Pixels)
    {
        const TCHAR* File = TEXT("IBMPlexSans-Regular");
        const char* Fallback = "Regular";
        switch (Face)
        {
        case EFace::Regular: break;
        case EFace::Medium: File = TEXT("IBMPlexSans-Medium"); break;
        case EFace::Semi: File = TEXT("IBMPlexSans-SemiBold"); Fallback = "Bold"; break;
        case EFace::Bold: File = TEXT("IBMPlexSans-Bold"); Fallback = "Bold"; break;
        case EFace::Mono: File = TEXT("IBMPlexMono-Medium"); break;
        case EFace::MonoSemi: File = TEXT("IBMPlexMono-SemiBold"); Fallback = "Bold"; break;
        case EFace::Display: File = TEXT("BricolageGrotesque-SemiBold"); Fallback = "Bold"; break;
        case EFace::DisplayBold: File = TEXT("BricolageGrotesque-Bold"); Fallback = "Bold"; break;
        }
        // Slate sizes are points at 96 DPI: a design pixel is 0.75 point.
        const float Points = FMath::Max(6.f, Pixels * 0.75f);
        static TMap<FString, bool> Exists;
        const FString Path = FPaths::Combine(SlateDir(), TEXT("Fonts"), FString(File) + TEXT(".ttf"));
        bool* Known = Exists.Find(Path);
        if (!Known) Known = &Exists.Add(Path, IFileManager::Get().FileExists(*Path));
        if (*Known) return FSlateFontInfo(Path, Points);
        return FCoreStyle::GetDefaultFontStyle(Fallback, FMath::RoundToInt32(Points));
    }

    const FSlateBrush* Icon(const FString& Name)
    {
        static TMap<FString, TSharedPtr<FSlateBrush>> Cache;
        if (const TSharedPtr<FSlateBrush>* Found = Cache.Find(Name)) return Found->Get();
        const FString Path = FPaths::Combine(SlateDir(), TEXT("Icons"), Name + TEXT(".svg"));
        TSharedPtr<FSlateBrush> Brush;
        if (IFileManager::Get().FileExists(*Path)) Brush = MakeShared<FSlateVectorImageBrush>(Path, FVector2f(24.f, 24.f));
        Cache.Add(Name, Brush);
        return Brush.Get();
    }

    const FSlateBrush* Shadow()
    {
        static TSharedPtr<FSlateBrush> Brush;
        static bool bTried = false;
        if (!bTried)
        {
            bTried = true;
            const FString Path = FPaths::Combine(SlateDir(), TEXT("Shadow.png"));
            if (IFileManager::Get().FileExists(*Path)) Brush = MakeShared<FSlateBoxBrush>(Path, FMargin(0.45f));
        }
        return Brush.Get();
    }

    FLinearColor Hex(const TCHAR* Code, float Alpha)
    {
        FLinearColor Result(FColor::FromHex(Code));
        Result.A = Alpha;
        return Result;
    }
}
