#include "MarketCast.h"
#include "MarketCountry.h"

namespace MarketCastLocal
{
    uint32 Mix(uint32 A, uint32 B)
    {
        uint32 H = A * 0x9E3779B1u ^ B * 0x85EBCA77u;
        H ^= H >> 15; H *= 0x2C1B3C6Du; H ^= H >> 12; H *= 0x297A2D39u; H ^= H >> 15;
        return H;
    }

    // Last names of the roles must differ (the shop next door is not the wholesaler's family).
    int32 LastIndex(MarketCast::ERole Role, int32 Count)
    {
        if (Count <= 0) return 0;
        const uint32 Seed = static_cast<uint32>(MarketCountry::ActiveSeed());
        const int32 Start = static_cast<int32>(Mix(Seed, 0xCA57u) % static_cast<uint32>(Count));
        // Each role steps through the list from the same start, so two roles never share a last name while the
        // list is long enough.
        return (Start + static_cast<int32>(Role)) % Count;
    }

    int32 FirstIndex(MarketCast::ERole Role, int32 Count)
    {
        if (Count <= 0) return 0;
        const uint32 Seed = static_cast<uint32>(MarketCountry::ActiveSeed());
        return static_cast<int32>(Mix(Seed ^ 0xF125u, static_cast<uint32>(Role) + 1u) % static_cast<uint32>(Count));
    }
}

FString MarketCast::FirstName(ERole Role)
{
    const TArray<FString>& Names = MarketCountry::Active().FirstNames;
    return Names.Num() > 0 ? Names[MarketCastLocal::FirstIndex(Role, Names.Num())] : FString(TEXT("Ali"));
}

FString MarketCast::LastName(ERole Role)
{
    const TArray<FString>& Names = MarketCountry::Active().LastNames;
    return Names.Num() > 0 ? Names[MarketCastLocal::LastIndex(Role, Names.Num())] : FString(TEXT("Y\u0131lmaz"));
}

FString MarketCast::Person(ERole Role) { return FirstName(Role) + TEXT(" ") + LastName(Role); }

FString MarketCast::Salesman() { return FirstName(ERole::Salesman); }
FString MarketCast::Wholesaler() { return LastName(ERole::Wholesaler) + TEXT(" G\u0131da Da\u011f\u0131t\u0131m"); }
FString MarketCast::CashCarry() { return LastName(ERole::CashCarry) + TEXT(" Toptan"); }
FString MarketCast::CashCarryOwner() { return FirstName(ERole::CashCarryOwner) + TEXT(" ") + LastName(ERole::CashCarry); }
FString MarketCast::RivalShop() { return LastName(ERole::RivalShop) + TEXT(" Market"); }
FString MarketCast::RivalOwner() { return FirstName(ERole::RivalOwner) + TEXT(" ") + LastName(ERole::RivalShop); }

FString MarketCast::Bank(int32 Index)
{
    const TArray<FString>& Banks = MarketCountry::Active().Banks;
    if (Banks.IsValidIndex(Index)) return Banks[Index];
    static const TCHAR* Generic[4] = { TEXT("Yerel banka"), TEXT("Ticaret bankas\u0131"), TEXT("Yat\u0131r\u0131m bankas\u0131"), TEXT("Kalk\u0131nma bankas\u0131") };
    return Generic[FMath::Clamp(Index, 0, 3)];
}

FString MarketCast::Platform()
{
    const FString& Name = MarketCountry::Active().PlatformName;
    return Name.IsEmpty() ? FString(TEXT("H\u0131zl\u0131Sepet")) : Name;
}

FString MarketCast::Accountant()
{
    return Person(ERole::Accountant);
}
