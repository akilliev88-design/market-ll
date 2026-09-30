# Akış B — Claude Code

Sözleşme: `Docs/Kurgu/07_AKIL_ISBOLUMU.md` §4 Akış B. Dal: `claude/miras-market-akis-b-evw111` (bulut oturumu; Mustafa'nın promptundaki `akis-b` yerine oturumun zorunlu dalı. Birleştirmede bu dal `akis-b` sayılır).

## Kaldığım yer

B1 bitti ve commit edildi. Sıradaki: B2 (muhasebe defteri).

## Yapılanlar

### B1 · Açık denge hataları

| # | Ne yapıldı | Dosyalar | Test |
|---|---|---|---|
| 24 | Raftaki "alır mı" kararı ürünün esnekliğine (katalog `elasticity`, yoksa 2,5) ve fiyatının bilinirliğine (`kvi`) bağlı. Rakip fiyatında herkes için %90 alır; ucuzlukta keyif ürünü çok, temel ürün az kazanır; pahalılıkta kayıptan kaçınma ×1,4. Eski `BuyChance` eğrisi eski çağıranlar için duruyor. | `MarketDemand.*` | `Balance.ElasticShelf` |
| 27 | Maliyet altı satış: `MarketDemand::PriceWarning` (fiyat değişince gösterilecek tek cümle) ve gün raporunda "Maliyetin altında satış: N adet, X zarar (en çok …)". Kampanya bitince rapora "satışların brüt kârı" ve toptancı destekli teklifte "toptancı desteği" (teklif süresince gelen koliler × maliyet indirimi) eklendi. | `MarketDemand.*`, `MarketPromotions.*`, `MarketLedger.h` | `Balance.BelowCostAndFundedDeal` |
| 30 | Online siparişler o günün raf kampanya fiyatını öder (`MarketPromotions::DealPrice`); rafta planı olmayan ürün sipariş edilmez; yeni kampanyanın "öncesi" online adetleri saymaz (`Ledger.OnlineSold`). | `MarketOnline.cpp`, `MarketPromotions.*` | `Balance.OnlineDeals` |
| 43 | Vergide zarar devri (zarar eden hafta sonraki haftaların matrahından düşülür, süre sınırı yok). Son gün indirimi yalnız eski partinin rafta kalan birimlerine (FEFO; sepet karışıksa oranlı). İpotek artık ödül değil: ihtiyaç kadar (açığın 1,5 katı; 500–3.000 TL × fiyat düzeyi), faiz +6 puan, %2 masraf. | `MarketStaff.cpp`, `MarketFreshness.*`, `MarketFinance.*` | `Balance.TaxLossCarry`, `Balance.LastDayMarkdown`, `Balance.Mortgage` |
| 45 | Ulusal pay ciroya bağlı: ülkedeki ciromuz (aile dükkânı + online + o ülkedeki şubeler, ~30 günlük yumuşatma) / ülkenin gıda perakendesi (nüfus × kişi başı günlük harcama × fiyat düzeyi). 5. bölüm hedefi %2 → %0,1. | `MarketCompany.*`, `MarketCountry.*`, `MarketStory.*`, `MarketDirector.cpp` (B bloğu) | `Balance.NationalShareByRevenue` |

Kontrol edilenler (düzeltme gerekirse C'ye):

- **#21 menü/simülasyon rakip fiyatı:** Kapandı. Menü (`MarketMenu.cpp`, `MarketMenuPages.cpp`, `MarketMenuWidget.cpp`), oyun (`AMarketGameMode::RivalPriceFactor`) ve simülasyon hepsi `MarketDirector::RivalPriceFactor` → `MarketCompetitors::RivalPriceFactor` kullanıyor. Kalan tek farklılık: `MarketBranches.cpp:684` boş kategoriyle çağırıyor (reyon indirimleri şubeye girmiyor, #14'ün şube hâli) → C'ye not.
- **#25 kampanya başka reyondan çalıyor:** Kısmen açık. Mağaza geneli indirim ve broşür müşteri getiriyor (`TrafficFactor`), ama tek ürün/reyon kampanyasında müşterinin liste uzunluğu hâlâ sabit (`MarketCustomers::BuildList`, C'nin dosyası). C'ye öneri aşağıda.
- **#31 "3 al 2 öde" 2 isteyeni 3'e çıkarıyor:** Tasarım gereği bırakıldı (G-078: tek ürün kapsamında bedava birime tamamlama; `Pantry` sonrası düşüşü veriyor). Verilen indirim sepet başına değil günlük satış/3 üzerinden tahmin ediliyor; sapma küçük. Artık rapor "brüt kâr"ı da gösteriyor.
- **#39 ücret/kıdem:** B3'te ele alınıyor (kıdem tazminatı `MarketStaff::Fire` içinde zaten vardı).

## Doğrulama

Bu oturum Linux bulut kapsayıcısında; Unreal yok, `DERLE.cmd` / `TEST.cmd` çalıştırılamadı. **Derlenmedi (UE).** Yerine:

- Saf modüller ve testleri, Unreal'in kullanılan kısmını taklit eden küçük bir katmanla (sahte `CoreMinimal.h`: FString, TArray, TMap, FMath, FRandomStream UE algoritmasıyla, JSON, otomasyon testi makroları) clang ile `-Wshadow-all -Werror=shadow` derlenip çalıştırıldı. Dünyaya bağlı dosyalar (MarketGame, menü, mağaza kiti) bu katmanda derlenmez.
- 30.09.2026, B1 sonrası: **78/78 test geçti** (başlangıçta 70/70; +7 `Balance.*` + dosya sayımı). UE'deki toplam 84 testin dünyaya bağlı 14'ü bu sayıya dahil değil.
- Codex'in `DERLE.cmd /q` + `TEST.cmd /q` koşusu bekleniyor.

## Yeni açık işlevler

- `float MarketDemand::ElasticityOf(const FMarketProduct&)` — katalog esnekliği ya da 2,5.
- `double MarketDemand::BuyChanceFor(double Ratio, float Share, double Tolerance, float Elasticity, float Kvi = 0)` — ürüne göre alma olasılığı.
- `FString MarketDemand::PriceWarning(State, Products, Index)` — fiyat maliyetin altındaysa tek cümle, değilse boş.
- `int64 MarketPromotions::DealPrice(State, Products, Index, Quantity, Day)` — o günün kampanyalı raf fiyatı (son gün indirimi hariç).
- `double MarketFreshness::PriceFactor(State, Index, Quantity)`, `int32 MarketFreshness::MarkdownUnitsLeft(State, Index)`.
- `int64 MarketFinance::MortgageAmount(State)`.
- `int64 MarketCompany::CountryMarketDay(State)`, `int64 MarketCompany::CountryRevenueToday(State)`, `void MarketCompany::TrackNationalRevenue(State)`.
- `FMarketState::Ledger` (`MarketLedger.h`, `FMarketLedger`): B1 alanları `TaxLossCarry`, `CountryRevenueDay`, `PromoTallies`, `OnlineSold`, `LastBelowCostUnits/Loss`.

## C'ye istekler

1. **Fiyat değişince uyarı (#27):** Menünün Fiyat sayfası ve `AMarketGameMode` PriceUp/PriceDown bildirimi, fiyat değiştikten sonra `MarketDemand::PriceWarning(State, Products, Index)` boş değilse onu ikinci satır olarak göstersin:
   ```cpp
   const FString Warn = MarketDemand::PriceWarning(State, Products, Selected);
   Notify(FString::Printf(TEXT("%s: %s"), *ProductName(Selected), *PriceSummary(Selected)) + (Warn.IsEmpty() ? FString() : TEXT("\n") + Warn));
   ```
   (`MarketGame.cpp` Codex'in dosyası; Codex ya da bağlamada C.)
2. **Fiyat özeti yeni eğriyi göstersin (#24):** `AMarketGameMode::PriceSummary` (`MarketGame.cpp:819`) ve menüde "alan müşteri ~%" gösteren her yer `BuyChance(...)` yerine `BuyChanceFor(Ratio, State.MarketShare, 0.0, MarketDemand::ElasticityOf(Products[Index]), Products[Index].Kvi)` kullansın; yoksa ekran simülasyondan farklı sayı söyler (#21'in tekrarı).
3. **Şube reyon fiyatı (#14/#21):** `MarketBranches.cpp:684` `MarketCompetitors::RivalPriceFactor(State, FString(), ...)` boş kategoriyle çağrılıyor; ürünün kategorisiyle çağrılmalı.
4. **Kapanan şubenin malı (#43):** `MarketBranches.cpp:347` kapanan şubenin mallarını ana dükkâna `Received` olarak ekliyor; tazelik modülü bunları taze parti sayıyor. Bozulan ürünlerde `Received` yerine doğrudan eski bir parti eklenmeli:
   ```cpp
   const int32 Life = MarketGoods::ShelfLifeDays(Products[Row]);
   if (Life > 0) { FMarketBatch Old; Old.ProductId = Stock->Id; Old.Units = Take; Old.ExpiresDay = State.Day + FMath::Max(1, Life / 3); State.Batches.Add(Old); }
   else Stock->Received += Take;
   ```
5. **#25 kampanya ek alışveriş getirsin:** `MarketCustomers::BuildList` liste uzunluğunu kampanya ilgisiyle biraz uzatsın (öneri: listedeki kampanyalı ürün başına %35 olasılıkla +1 kalem, en çok +1), böylece indirim başka reyondan çalmak yerine sepeti büyütür.
6. **Menü:** Rakipler/Şirket sayfasındaki "Ulusal pay" artık ciro payı; açıklama ipucu: "Ülkedeki cironuzun ülkenin gıda perakendesine oranı (son ~30 gün)." 5. bölüm hedefi metni kodda (`MarketStory::NationalShareGoal`).

## Kararlar ve varsayımlar

- **#24 sayıları:** rakip fiyatında alma %90 (eski eğri %97,3); duyarlılık K = 3; kayıptan kaçınma ×1,4; `kvi` ×(1 + kvi); esneklik 0,5–6 aralığına kırpılır. Sonuç: süt (1,5; kvi 1) rakibin %10 üstünde ~%72, kola (4) %15 üstünde ~%40 alır. 400 günlük denemede aile dükkânının cirosu ~%9 düştü (rakip fiyatında %97 → %90). Otomatik oyuncu raporunda izlenmeli; gerekirse `ParityChance` 0,92–0,93'e çekilir.
- **#27:** "Toptancı desteği" = teklif süresince teslim alınan birim × (indirimli maliyet × 15/85). Brüt kâr = kampanyalı satışların tahmini geliri − defter maliyeti − (broşür gibi) kampanya maliyeti. Maliyet altı satış satırında son gün indirimi sayılmaz (amacı zaten eski malı ucuza satmak).
- **#30:** Online siparişler kampanya raporunun "sırasında" adedine girmez (raporlar önce kapanıyor); "öncesi" de artık online saymıyor, böylece ikisi aynı ölçü.
- **#43:** Zarar devrinin süre sınırı yok (gerçekte 5 yıl; oyunda sadelik). İpotek faiz primi +6 puan (acil kredi +12), masraf %2 (ekspertiz, tapu). İpoteğin ödenmemesi hâlâ sonuçsuz (dükkân bankaya geçmiyor) — ayrı iş.
- **#45:** Kişi başı günlük gıda harcaması oyun ölçeğinde 0,65 TL (başlangıç fiyat düzeyi; `ulkeler.json` → `economy.groceryPerPersonDay` ile ülke başına verilebilir; yoksa 0,65 × `wageFactor`). Ayar: oyunun ucuzcu şubesi (~900 TL/gün) gerçekteki en büyük zincirin bir mağazası kadar pay tutsun (BİM ≈ 3.500 mağazayla %6,5). Böylece 50 ucuzcu ≈ %0,1. **5. bölüm hedefi %2'den %0,1'e indi** (%2 bu ölçüde ~1.000 mağaza demek). Mustafa'ya soru olarak aşağıda.

## Bilinen sorunlar

- Oyun cirosu gerçeğin ~1/10'u (#49, C). Ulusal pay bu ölçeğe göre ayarlandı; #49 düzelirse `groceryPerPersonDay` de ~10 kat büyümeli.

## Mustafa'ya sorular

1. 5. bölümün "ulusal pay" hedefi: %0,1 (≈50 ucuzcu kadar ciro) uygun mu, yoksa daha büyük bir hedef mi (ör. %0,25)?
2. Raf fiyatına tepki: rakip fiyatında %90 alma (eskisi %97) dükkânın cirosunu ~%9 düşürdü. Bot raporuna göre 0,92–0,93 yapılabilir.
