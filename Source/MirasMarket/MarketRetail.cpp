#include "MarketRetail.h"

const TArray<MarketRetail::FChain>& MarketRetail::National()
{
    static const TArray<FChain> Chains = {
        { TEXT("Bakkal ve pazar"), TEXT("geleneksel perakende"), TEXT(""), { { 2011, 58.f }, { 2025, 32.7f } }, { { 2011, 180000.f } },
          TEXT("Mahalle bakkallar\u0131 ve semt pazarlar\u0131. Zincirler b\u00fcy\u00fcd\u00fck\u00e7e paylar\u0131 her y\u0131l azal\u0131r.") },
        { TEXT("B\u0130M"), TEXT("indirim marketi"), TEXT("bim"), { { 2011, 6.5f }, { 2023, 14.1f }, { 2025, 18.4f } },
          { { 2011, 3500.f }, { 2015, 4972.f }, { 2017, 6765.f }, { 2019, 7438.f }, { 2021, 10330.f }, { 2025, 12571.f } },
          TEXT("Az \u00e7e\u015fit, d\u00fc\u015f\u00fck fiyat. Temel g\u0131dada fiyatla yar\u0131\u015fmak zor.") },
        { TEXT("A101"), TEXT("indirim marketi"), TEXT("a101"), { { 2011, 1.2f }, { 2023, 10.5f }, { 2025, 7.8f } },
          { { 2011, 1900.f }, { 2013, 2694.f }, { 2017, 8000.f }, { 2020, 10001.f }, { 2025, 13550.f } },
          TEXT("En h\u0131zl\u0131 ma\u011faza a\u00e7an zincir: her mahallede bir tane.") },
        { TEXT("Migros"), TEXT("s\u00fcpermarket, \u00e7ok format"), TEXT("migros"), { { 2011, 4.6f }, { 2023, 7.1f }, { 2025, 9.2f } },
          { { 2011, 1100.f }, { 2025, 3895.f } },
          TEXT("Birka\u00e7 formatla \u00e7al\u0131\u015f\u0131r; b\u00fcy\u00fcd\u00fck\u00e7e k\u00fc\u00e7\u00fck zincirleri sat\u0131n al\u0131r.") },
        { TEXT("\u015eOK"), TEXT("indirim marketi"), TEXT("sok"), { { 2011, 1.2f }, { 2023, 6.0f }, { 2025, 6.0f } },
          { { 2011, 1300.f }, { 2013, 2542.f }, { 2016, 4000.f }, { 2017, 6364.f }, { 2025, 11797.f } },
          TEXT("Mahalle tipi indirim marketi.") },
        { TEXT("CarrefourSA"), TEXT("hipermarket, s\u00fcpermarket"), TEXT("carrefoursa"), { { 2011, 1.8f }, { 2023, 2.0f }, { 2025, 1.8f } },
          { { 2011, 250.f }, { 2013, 258.f }, { 2025, 1247.f } },
          TEXT("B\u00fcy\u00fck ma\u011fazalar: hipermarket ve s\u00fcpermarket.") },
        { TEXT("Tesco Kipa"), TEXT("hipermarket"), TEXT("kipa"), { { 2011, 1.5f } }, { { 2011, 170.f }, { 2016, 189.f } },
          TEXT("Bat\u0131da hipermarketler. Bir s\u00fcre sonra daha b\u00fcy\u00fck bir zincire sat\u0131l\u0131r."), 2017 },
        { TEXT("Metro Toptan"), TEXT("toptan"), TEXT("metro"), { { 2011, 0.9f } }, { { 2011, 30.f } },
          TEXT("\u0130\u015fletmelere toptan sat\u0131\u015f.") },
    };
    return Chains;
}

const TArray<MarketRetail::FGlobal>& MarketRetail::Global()
{
    static const TArray<FGlobal> Rows = {
        { TEXT("Walmart"), TEXT("ABD \u00b7 hipermarket"), { { 2011, 446.9f }, { 2016, 485.9f }, { 2023, 648.1f } }, TEXT("D\u00fcnyan\u0131n a\u00e7\u0131k ara en b\u00fcy\u00fc\u011f\u00fc.") },
        { TEXT("Carrefour"), TEXT("Fransa \u00b7 hipermarket"), { { 2011, 113.2f }, { 2016, 84.1f } }, TEXT("Hipermarketin \u00f6nc\u00fcs\u00fc; yerel ortaklarla \u00e7al\u0131\u015f\u0131r.") },
        { TEXT("Tesco"), TEXT("\u0130ngiltere \u00b7 s\u00fcpermarket"), { { 2011, 101.6f } }, TEXT("S\u00fcpermarket devi; yurt d\u0131\u015f\u0131 ma\u011fazalar\u0131n\u0131 zamanla k\u00fc\u00e7\u00fclt\u00fcr.") },
        { TEXT("Metro Group"), TEXT("Almanya \u00b7 toptan"), { { 2011, 92.9f } }, TEXT("\u0130\u015fletmelere toptan sat\u0131\u015f.") },
        { TEXT("Kroger"), TEXT("ABD \u00b7 s\u00fcpermarket"), { { 2011, 90.4f }, { 2016, 115.3f } }, TEXT("Yaln\u0131z ABD.") },
        { TEXT("Costco"), TEXT("ABD \u00b7 \u00fcyelikli depo"), { { 2011, 88.9f }, { 2016, 118.7f } }, TEXT("\u00dcyelikli toptan ma\u011faza.") },
        { TEXT("Schwarz Grubu"), TEXT("Almanya \u00b7 Lidl, Kaufland"), { { 2011, 87.2f }, { 2016, 99.3f } }, TEXT("Lidl ile Avrupa'da h\u0131zla b\u00fcy\u00fcyor.") },
        { TEXT("Aldi"), TEXT("Almanya \u00b7 indirim marketi"), { { 2011, 73.0f }, { 2016, 84.9f } }, TEXT("Az \u00fcr\u00fcn, d\u00fc\u015f\u00fck fiyat: B\u0130M modelinin esin kayna\u011f\u0131.") },
    };
    return Rows;
}

float MarketRetail::At(const TArray<FPoint>& Points, float Year)
{
    if (Points.Num() == 0) return 0.f;
    if (Year <= Points[0].Year) return Points[0].Value;
    for (int32 I = 1; I < Points.Num(); ++I)
    {
        if (Year > Points[I].Year) continue;
        const float Span = static_cast<float>(Points[I].Year - Points[I - 1].Year);
        const float T = Span > 0.f ? (Year - Points[I - 1].Year) / Span : 1.f;
        return FMath::Lerp(Points[I - 1].Value, Points[I].Value, T);
    }
    return Points.Last().Value;
}

float MarketRetail::OthersShare(float Year, float PlayerShare)
{
    float Listed = PlayerShare;
    for (const FChain& Chain : National())
        if (Chain.ClosedYear == 0 || Year < Chain.ClosedYear) Listed += At(Chain.Share, Year);
    return FMath::Max(0.f, 100.f - Listed);
}

