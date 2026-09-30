#include "MarketMenuWidget.h"
#include "MarketMenuInternal.h"

#include "MarketGame.h"
#include "ProductCatalog.h"
#include "MarketSuppliers.h"
#include "MarketPromotions.h"
#include "MarketCompetitors.h"
#include "MarketEvents.h"
#include "MarketStory.h"
#include "MarketFinance.h"
#include "MarketOnline.h"
#include "MarketPayments.h"
#include "MarketSimulation.h"
#include "MarketCredit.h"
#include "MarketFreshness.h"
#include "MarketBranches.h"
#include "MarketCompany.h"
#include "MarketCampaign.h"
#include "MarketCalendar.h"
#include "MarketStaff.h"
#include "MarketRivals.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "InputCoreTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

// Named namespace (not anonymous): the module is built as a unity build and the HUD has its own helpers.
namespace MarketMenuUi
{
    FSlateFontInfo MenuFont(bool bBold, int32 Size)
    {
        return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
    }

    // 1234.5 TL -> "1.234,50 TL"
    FString Tl(int64 Kurus)
    {
        const bool bNegative = Kurus < 0;
        const int64 Abs = FMath::Abs(Kurus);
        FString Whole = FString::Printf(TEXT("%lld"), Abs / 100);
        for (int32 I = Whole.Len() - 3; I > 0; I -= 3) Whole.InsertAt(I, TEXT('.'));
        return FString::Printf(TEXT("%s%s,%02lld TL"), bNegative ? TEXT("-") : TEXT(""), *Whole, Abs % 100);
    }

    FLinearColor Hex(const TCHAR* Code, float Alpha)
    {
        FLinearColor Result(FColor::FromHex(Code));
        Result.A = Alpha;
        return Result;
    }

    // "s\u00fct" -> "S\u00fct", "i\u00e7ecek" -> "\u0130\u00e7ecek" (Turkish dotted capital I).
    FString Title(const FString& Text)
    {
        if (Text.IsEmpty()) return Text;
        FString Result = Text;
        const TCHAR First = Result[0];
        Result[0] = First == TEXT('i') ? TCHAR(0x0130) : First == TCHAR(0x0131) ? TEXT('I') : FChar::ToUpper(First);
        return Result;
    }

    FString Initials(const FString& Name)
    {
        TArray<FString> Words;
        Name.ParseIntoArrayWS(Words);
        FString Result;
        for (const FString& Word : Words)
        {
            if (Word.IsEmpty() || !FChar::IsAlnum(Word[0])) continue;
            Result.AppendChar(FChar::ToUpper(Word[0]));
            if (Result.Len() >= 2) break;
        }
        return Result.IsEmpty() ? FString(TEXT("?")) : Result;
    }

    FLinearColor RivalColor(int32 Rival)
    {
        switch (Rival)
        {
        case 0: return Hex(TEXT("C8102E"));
        case 1: return Hex(TEXT("F28C00"));
        default: return Hex(TEXT("0068A8"));
        }
    }

    // Price of one rival for one product today; false = the rival's shelf is empty.
    bool RivalShelfPrice(const AMarketGameMode& G, int32 Product, int32 Rival, int64& OutPrice)
    {
        bool bEmpty = false;
        const float Factor = MarketRivals::RivalFactor(G.State.Day, G.State.RivalSeed, G.RivalAisles, G.Products[Product].Category, Rival, &bEmpty)
            * MarketCompetitors::NewsRivalIndex(G.State, Rival); // the chain's everyday price level (G-065)
        OutPrice = MarketDemand::RivalPrice(G.Products[Product], Factor);
        return !bEmpty;
    }

    // Cheapest open rival with the product on its shelf (INDEX_NONE = nobody has it today).
    int32 CheapestRival(const AMarketGameMode& G, int32 Product, int64& OutPrice)
    {
        int32 Best = INDEX_NONE;
        OutPrice = 0;
        for (int32 Rival = 0; Rival < MarketRivals::RivalCount(G.State.Day); ++Rival)
        {
            int64 Price = 0;
            if (RivalShelfPrice(G, Product, Rival, Price) && (Best == INDEX_NONE || Price < OutPrice)) { Best = Rival; OutPrice = Price; }
        }
        return Best;
    }

    double BuyChanceOf(const AMarketGameMode& G, int32 Product)
    {
        const int64 Theirs = MarketDemand::RivalPrice(G.Products[Product], G.RivalPriceFactor(Product));
        return MarketDemand::BuyChance(MarketDemand::PriceRatio(G.State.Stock[Product].Price, Theirs), G.State.MarketShare);
    }

    FString ProblemText(const AMarketGameMode& G, const MarketDemand::FProblem& Problem)
    {
        const FString Shown = G.Products.IsValidIndex(Problem.Product) ? G.ProductName(Problem.Product) : FString();
        switch (Problem.Kind)
        {
        case MarketDemand::EProblem::Waiting: return FString::Printf(TEXT("%d m\u00fc\u015fteri i\u00e7eride beklemekten vazge\u00e7ti."), Problem.Count);
        case MarketDemand::EProblem::NotCarried: return FString::Printf(TEXT("%d m\u00fc\u015fteri %s sordu ama rafta yok (R ile reyona koy)."), Problem.Count, *Shown);
        case MarketDemand::EProblem::Empty: return FString::Printf(TEXT("%s rafta bitti: %d m\u00fc\u015fteri eli bo\u015f d\u00f6nd\u00fc."), *Shown, Problem.Count);
        case MarketDemand::EProblem::Expensive: return FString::Printf(TEXT("%d m\u00fc\u015fteri %s fiyat\u0131n\u0131 pahal\u0131 buldu."), Problem.Count, *Shown);
        }
        return FString();
    }

    FString RivalsToday(const AMarketGameMode& G, int32 OnlyRival)
    {
        TArray<FString> Lines;
        for (const MarketRivals::FEvent& Event : MarketRivals::ActiveOn(G.State.Day, G.State.RivalSeed, G.RivalAisles))
            if (OnlyRival == INDEX_NONE || Event.Rival == OnlyRival) Lines.Add(MarketRivals::Describe(Event));
        return Lines.Num() > 0 ? FString::Join(Lines, TEXT("\n")) : FString(TEXT("Sakin: \u00f6zel bir kampanya yok."));
    }
}
