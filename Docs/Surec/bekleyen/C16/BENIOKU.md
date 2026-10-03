# C16 (M48–M50) bekleyen kod

03.10.2026 · Claude (Cowork). Derlenmedi. Ana koda **bilerek konmadı**: dünya yeniden tasarımı (M51) sırasında bekletiliyor (09_DUNYA_YENIDEN.md §9, D9).

- Dosyalar `.txt` uzantılı; derlemeye girmesin diye. Her biri o dosyanın **C15b + C16** hâli (C15b ana kodda zaten var).
- Yeni dosyalar: `MarketStrategy.h`, `MarketStrategy.cpp`, `MarketStrategyTests.cpp`.
- Değişen dosyalar ve C16 farkı:
  - `MarketBranches.cpp`: çekim × `MarketStrategy::PullFactor`, hizmet × `ServiceFactor`, raf fiyatı × `ShelfPriceFactor`, depo kaybı × `DepotLossFactor`, tadilat × `FitOutFactor`, kira × `RentFactor` (Open ve OpeningCost), kötü yer × `HastyFactor`, `GrowthCapacity` × `CapacityFactor` + `CapacityBonus`.
  - `MarketBanking.cpp`: `YearRate` − `RateDiscount`.
  - `MarketChains.h/.cpp`: yeni `Withdraw`.
  - `MarketDirector.cpp`: `ProvincePush` komutu, `MarketStrategy::CloseDay`.
  - `MarketEvents.cpp`: `strategy.*` kararları.
  - `MarketEconomy.h`: `FMarketPush`, `FMarketStrategyState`, `FMarketState::Strategy`, kayıt sürümü 9.
  - `MarketAutoPlay.cpp`, `MarketAutoPlayC.*`: bot seçimleri, il atağı, rapor satırı.
  - `MarketMenuPages.cpp`: Şirket sekmesi "Strateji, yollar ve iller" kartı.
  - `Test.ps1`: alt sınır 162.
- Uygulamak için ilgili dosyaların güncel hâliyle karşılaştırıp yalnız `C16` işaretli satırları taşı (C15b sonrası ana kod değiştiyse dosyayı olduğu gibi kopyalama).
- Kararlar: 01_KARARLAR.md M48, M49, M50.
