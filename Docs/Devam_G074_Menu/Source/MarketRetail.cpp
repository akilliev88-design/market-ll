#include "MarketRetail.h"

const TArray<MarketRetail::FChain>& MarketRetail::National()
{
    static const TArray<FChain> Chains = {
        { TEXT("Bakkal ve pazar"), TEXT("geleneksel perakende"), TEXT(""), { { 2011, 58.f }, { 2025, 32.7f } }, { { 2011, 180000.f } },
          TEXT("Mahalle bakkalları ve semt pazarları. Zincirler büyüdükçe payları her yıl azalır.") },
        { TEXT("BİM"), TEXT("indirim marketi"), TEXT("bim"), { { 2011, 6.5f }, { 2023, 14.1f }, { 2025, 18.4f } },
          { { 2011, 3500.f }, { 2015, 4972.f }, { 2017, 6765.f }, { 2019, 7438.f }, { 2021, 10330.f }, { 2025, 12571.f } },
          TEXT("1995'te kuruldu. Az çeşit, düşük fiyat. Temel gıdada fiyatla yarışmak zor.") },
        { TEXT("A101"), TEXT("indirim marketi"), TEXT("a101"), { { 2011, 1.2f }, { 2023, 10.5f }, { 2025, 7.8f } },
          { { 2011, 1900.f }, { 2013, 2694.f }, { 2017, 8000.f }, { 2020, 10001.f }, { 2025, 13550.f } },
          TEXT("2008'de kuruldu; en hızlı mağaza açan zincir. 2020'de 10.000 mağazayı geçti.") },
        { TEXT("Migros"), TEXT("süpermarket, çok format"), TEXT("migros"), { { 2011, 4.6f }, { 2023, 7.1f }, { 2025, 9.2f } },
          { { 2011, 1100.f }, { 2025, 3895.f } },
          TEXT("Migros, Tansaş, Macrocenter. 2017'de Kipa'yı, 2018'de Makro'yu aldı.") },
        { TEXT("ŞOK"), TEXT("indirim marketi"), TEXT("sok"), { { 2011, 1.2f }, { 2023, 6.0f }, { 2025, 6.0f } },
          { { 2011, 1300.f }, { 2013, 2542.f }, { 2016, 4000.f }, { 2017, 6364.f }, { 2025, 11797.f } },
          TEXT("Mahalle tipi indirim marketi. 2011'de Yıldız Holding Migros'tan aldı.") },
        { TEXT("CarrefourSA"), TEXT("hipermarket, süpermarket"), TEXT("carrefoursa"), { { 2011, 1.8f }, { 2023, 2.0f }, { 2025, 1.8f } },
          { { 2011, 250.f }, { 2013, 258.f }, { 2025, 1247.f } },
          TEXT("Sabancı ve Carrefour ortaklığı.") },
        { TEXT("Tesco Kipa"), TEXT("hipermarket"), TEXT("kipa"), { { 2011, 1.5f } }, { { 2011, 170.f }, { 2016, 189.f } },
          TEXT("Ege ve Trakya'da hipermarketler. 2017'de Migros'a satıldı."), 2017 },
        { TEXT("Metro Toptan"), TEXT("toptan"), TEXT("metro"), { { 2011, 0.9f } }, { { 2011, 30.f } },
          TEXT("İşletmelere toptan satış.") },
    };
    return Chains;
}

const TArray<MarketRetail::FGlobal>& MarketRetail::Global()
{
    static const TArray<FGlobal> Rows = {
        { TEXT("Walmart"), TEXT("ABD · hipermarket"), { { 2011, 446.9f }, { 2016, 485.9f }, { 2023, 648.1f } }, TEXT("Dünyanın açık ara en büyüğü. Türkiye'de mağazası yok.") },
        { TEXT("Carrefour"), TEXT("Fransa · hipermarket"), { { 2011, 113.2f }, { 2016, 84.1f } }, TEXT("Türkiye'de Sabancı ortaklığıyla CarrefourSA.") },
        { TEXT("Tesco"), TEXT("İngiltere · süpermarket"), { { 2011, 101.6f } }, TEXT("Türkiye'de Tesco Kipa; 2017'de çıktı.") },
        { TEXT("Metro Group"), TEXT("Almanya · toptan"), { { 2011, 92.9f } }, TEXT("Türkiye'de Metro Toptan.") },
        { TEXT("Kroger"), TEXT("ABD · süpermarket"), { { 2011, 90.4f }, { 2016, 115.3f } }, TEXT("Yalnız ABD.") },
        { TEXT("Costco"), TEXT("ABD · üyelikli depo"), { { 2011, 88.9f }, { 2016, 118.7f } }, TEXT("Üyelikli toptan mağaza.") },
        { TEXT("Schwarz Grubu"), TEXT("Almanya · Lidl, Kaufland"), { { 2011, 87.2f }, { 2016, 99.3f } }, TEXT("Lidl ile Avrupa'da hızla büyüyor.") },
        { TEXT("Aldi"), TEXT("Almanya · indirim marketi"), { { 2011, 73.0f }, { 2016, 84.9f } }, TEXT("Az ürün, düşük fiyat: BİM modelinin esin kaynağı.") },
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

const TCHAR* MarketRetail::Source()
{
    return TEXT("Kaynak: USDA GAIN (2014, 2024), Bloomberg HT (2025), Deloitte Global Powers of Retailing. 2011 payları tahmindir.");
}
