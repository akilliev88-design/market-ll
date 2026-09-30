# Akış B — Claude Code

Sözleşme: `Docs/Kurgu/07_AKIL_ISBOLUMU.md` §4 Akış B. Dal: `claude/miras-market-akis-b-evw111` (bulut oturumu; Mustafa'nın promptundaki `akis-b` yerine oturumun zorunlu dalı. Birleştirmede bu dal `akis-b` sayılır).

## Kaldığım yer

B1, B2, B3 bitti ve commit edildi. Sıradaki: B4 (dönem olayları).

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

### B2 · Muhasebe defteri (#37, #42)

Yeni `MarketLedger.h/.cpp` (`namespace MarketLedger`), durum `FMarketState::Ledger` (`FMarketLedger`).

- **Kayıt:** gün, mağaza (aile dükkânı −1, merkez −2, şube indeksi 0..), hesap (35 hesap: satış, online satış, SMM, fire, eksik/kırık/kasa farkı, maaş, sigorta primi, tazminat, işe alım, kira, enerji, reklam ve diğer, banka/kart ücretleri, online giderleri, nakliye, merkez, faiz, ceza, vergi, batan veresiye, şube sonucu; bilanço hareketleri: mal alımı, vadeli alım, kredi girişi/anapara, yatırım, yatırım dönüşü, kart tahsilatı, veresiye, vergi ödemesi, eve giden para, babadan kalan borç, sermaye, açıklanamayan fark), tutar (+ giriş/gelir, − çıkış/gider), nakit mi (kasa oynadı mı).
- **Kim yazar:** B dosyalarındaki her para hareketi yerinde `MarketLedger::Post` ile (Campaign, Credit, Finance, Online, Payments, Staff, Company, Freshness). `MarketEconomy.cpp`'nin hareketleri (kasa satışı, SMM, mal alımı, maaş, enerji, reklam/diğer, eski ikinci şube, teslimat eksik/kırık) gün kapanışının başında sayaçlardan okunur (`BeginClose`). Kapanış sırasında yazılan kayıtlar kapanan güne düşer.
- **Denetim (`EndClose`):** kasa değişimi = nakit kayıtların toplamı. Fark "açıklanamayan fark" hesabına yazılır, `Ledger.LastGap` / `GapDays` / `TotalGap` tutulur; `AuditOk`, `AuditText` ("Defter kasayla tutuyor." ya da farkın tutarı).
- **Raporlar:** `DayStatement`, `WeekStatement`, `Statement(Gün1, Gün2, Mağaza)` (son 120 günün kayıtlarından), `MonthStatement`, `YearStatement` (ay toplamları bütün kampanya boyunca saklanır): ciro, brüt kâr, giderler, net kâr, nakit değişimi, hesap hesap tutarlar. `Balance`: kasa, stok (maliyetle, yoldaki dahil), şube stoğu, kart alacağı, veresiye, depozitolar; toptancı borcu, banka, vergi, babadan kalan borç; özkaynak.
- **Eski kayıt:** defter boş başlar; ilk gün kapanışı farksız açar.
- Aynı gün/mağaza/hesap/tür kayıtları tek satırda toplanır (kart ödemeleri sepet sepet gelir); 200 günlük oyunda ~1.700 satır, saklama 120 gün.
- Testler: `Ledger.PostAndStatements`, `Ledger.CashAudit` (aile dükkânı 90 gün PlayDay: kart, veresiye, telefon siparişi, kredi, müşavir, vergi — tek fark toptancı vadesi, o da birebir), `Ledger.BalanceSheet`, `Ledger.OlderSaves`.
- Doğrulama denemesi: aşağıdaki "C'ye istekler 7" satırları yalnız derleme kopyasına uygulanınca, iki şube açıp birini kapatan, vadeli alım yapan 260 günlük oyunda **açıklanamayan fark 0** (tek fark denemenin kendisinin bilerek kasaya koyduğu para).

### B3 · Ücret ve sigorta (#39)

- **Asgari ücret tabanı:** `MarketStaff::MinimumDailyWage(Gün)` = net aylık asgari ücret (`MarketPrices::MinimumWage`) / 30 × ülkenin `wageFactor`'ü, 50 kuruşa yukarı yuvarlı (başlangıçta 22,00 TL/gün). `FairWage`, aday ücretleri ve İK pazarlığı bunun altına inmez; ocak/temmuzda asgari ücret artınca altında kalan ücretler kapanışta yükseltilir ("Asgari ücret arttı: N çalışanın günlük ücreti X oldu."). Mali müşavir ücret değil hizmet bedeli, tabana ve sigortaya girmez.
- **İşveren sigorta payı:** ülke paketinden (`ulkeler.json` → `economy.employerSocialRate`: TR 0,225, DE 0,21, GB 0,14, US 0,10; oyun değerleri). Her kapanışta ödenen ücretlerin payı kasadan çıkar, günün gideri olur, deftere "Sigorta primi" yazılır (`DailySocialSecurity`, `EmployerShare`, `EmployerCost`).
- **Kıdem tazminatı:** işten çıkarmada ihbar (3 günlük ücret, eskisi gibi) + ilk tam yıldan sonra yıl başına `severanceDaysPerYear` günlük ücret (TR 30, DE 15, GB 7, US 0), kesirli yıl oranlı (`SeniorityPay`, `SeverancePay`). İstifada ödenmez. Kasada yoksa çıkarılamaz. Deftere "Tazminat".
- Müdür ve şube çalışanları için aynı kurallar C'ye istek (aşağıda 9).
- Test: `Staff.WagesAndSocialSecurity`; `Staff.PeopleAndMorale` ve `Staff.TillAndHr` yeni tabana göre güncellendi (asgari ücretin altında "10 TL'lik düşük ücretli" artık yasal değil; en düşük ücretle çok becerili bir görevli aynı şekilde küser; pazarlık testi 30 TL'lik adayla).

## Doğrulama

Bu oturum Linux bulut kapsayıcısında; Unreal yok, `DERLE.cmd` / `TEST.cmd` çalıştırılamadı. **Derlenmedi (UE).** Yerine:

- Saf modüller ve testleri, Unreal'in kullanılan kısmını taklit eden küçük bir katmanla (sahte `CoreMinimal.h`: FString, TArray, TMap, FMath, FRandomStream UE algoritmasıyla, JSON, otomasyon testi makroları) clang ile `-Wshadow-all -Werror=shadow` derlenip çalıştırıldı. Dünyaya bağlı dosyalar (MarketGame, menü, mağaza kiti) bu katmanda derlenmez.
- 30.09.2026, B1 sonrası: **77/77 test geçti** (başlangıçta 70/70; +7 `Balance.*`). UE'deki toplam 84 testin dünyaya bağlı 14'ü bu sayıya dahil değil.
- B2 sonrası: **81/81** (+4 `Ledger.*`). B3 sonrası: **82/82** (+1 `Staff.WagesAndSocialSecurity`).
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
- `MarketLedger::Post(State, EAccount, Amount, bCash = true, Store = FamilyShop)` — her para hareketi.
- `MarketLedger::BeginClose(State, Products)` / `EndClose(State)` — Director B blokları.
- `MarketLedger::Statement / DayStatement / WeekStatement / MonthStatement / YearStatement` → `FStatement` (Revenue, GrossProfit, Expenses, NetProfit, CashChange, `At(EAccount)`).
- `MarketLedger::Balance(State, Products)` → `FBalance` (Assets, Liabilities, Equity).
- `MarketLedger::AuditOk / AuditText / StatementText / AccountName / IsIncomeStatement`.
- `MarketStaff::MinimumDailyWage(Gün)`, `EmployerSocialRate()`, `EmployerShare(Ücret)`, `EmployerCost(Ücret)`, `SeniorityPay(Ücret, İşeGirişGünü, Gün)`, `SeverancePay(State, Çalışan)`, `DailySocialSecurity(State)`.
- `MarketCountry::FProfile::EmployerSocialRate`, `SeveranceDaysPerYear` (`ulkeler.json` → `economy.employerSocialRate`, `economy.severanceDaysPerYear`).

## C'ye istekler

**Director bağlaması:** B blokları `MarketDirector::CloseDay`'in başında (`MarketLedger::BeginClose`, `State.DayNews.Reset()`'ten hemen sonra, bütün sistemlerden önce olmalı) ve sonunda (`MarketCompany::TrackNationalRevenue`, `MarketLedger::EndClose`, en son olmalı). Kalıcı yerleşimde bu sıra korunmalı.

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

7. **Defter bağlama satırları (B2).** Her satır ilgili `State.Cash` değişikliğinin hemen ardına; `P` = `MarketLedger::Post(State, MarketLedger::EAccount::`. Bağlanınca `Ledger.CashAudit` testindeki "toptancı vadesi" istisnası kaldırılıp fark 0 beklenmeli.
   - `MarketSuppliers.cpp` `OnOrder` (`State.Cash += Bill;`): `P SupplierCredit, Bill);` · `PayBills` ve `CloseDay` vadesi gelen ödeme (`State.Cash -= Bill.Amount;`): `P SupplierCredit, -Bill.Amount);` · gecikme farkı (`Bill.Amount += Fee;`): `P Penalties, -Fee, false);`
   - `MarketBranches.cpp` `Open` (`State.Cash -= 2 * Branch.Rent;`): `P Investment, -2 * Branch.Rent, true, State.Branches.Num());` · `Close` depozito: `P Divestment, 2 * B.Rent, true, BranchIndex);` · yarı fiyata satılan mal (`State.PendingLoss += Value;`): `P Shrinkage, -Value, false, BranchIndex);` ve `State.Cash += SoldValue;` ardına `P Divestment, SoldValue, true, BranchIndex);` · `Migrate` iadesi: `P Divestment, Refund, true, MarketLedger::HeadOfficeStore);` · açılış stoğu ve müdür siparişi (`State.Cash -= Bill; State.Purchases += Bill;`): `P Purchases, -Bill, true, Index);` (defter kapanış sırasındaki alımları aile dükkânından düşer; çift sayılmaz) · şubenin günü (`State.Cash += Revenue - Skim - Logistics - Opex;` ardına):
     ```cpp
     const int64 RentDay = FMath::RoundToInt64(B.Rent * MarketPrices::ListLevel(State.Day) / MarketPrices::ListLevel(FMath::Max(1, B.OpenedDay)) / 30.0);
     const int64 WagesDay = B.Workers * MarketStaff::FairWage(MarketStaff::ERole::Cashier, 50, State.Day) + B.ManagerWage;
     P Sales, Revenue, true, Index);          P Shrinkage, -Skim, true, Index);
     P Logistics, -Logistics, true, Index);   P Rent, -RentDay, true, Index);
     P Wages, -WagesDay, true, Index);        P Utilities, -(Opex - RentDay - WagesDay), true, Index);
     P CostOfGoods, -Cogs, false, Index);     P Waste, -WasteCost, false, Index);
     P Shrinkage, -DepotLoss, false, Index);
     ```
     (B3 sonrası şube ücretleri de sigorta primiyle ödenmeli: `WagesDay` için `MarketStaff::EmployerCost` kullanılır, aşağıda.)
   - `MarketManagers.cpp` `CloseDay` (`State.Cash -= Wages;`): `P Wages, -Wages, true, MarketLedger::HeadOfficeStore);`
   - `MarketDepots.cpp` `Build` (`State.Cash -= Cost;`): `P Investment, -Cost, true, MarketLedger::HeadOfficeStore);`
   - `MarketCompetitors.cpp` Bereket satın alma (`State.Cash -= Price;`): `P Investment, -Price, true, MarketLedger::HeadOfficeStore);`
   - `MarketGame.cpp` (Codex) raf küçülünce toptancıya yarı fiyata iade (`State.Cash += Refund; State.PendingLoss += Refund;`): `P Divestment, Refund); P Shrinkage, -Refund, false);`
   - Açılışta `MarketStart::Setup` bir şey yazmak zorunda değil: defter ilk kapanışta kasayı başlangıç kabul eder.
   - Şube tadilatı ve işe alım bedeli `State.OtherCosts` yoluyla ödeniyor; defterde aile dükkânının "reklam ve diğer" satırında görünür. Şubeye yazılsın istenirse `OtherCosts`'a eklemek yerine doğrudan `State.Cash -=` + `P Investment, -X, true, Index);` yapılmalı (ikisi birden değil).
8. **Menü (defter):** Finans sayfasına "Gelir tablosu" (dün / bu hafta / bu ay / bu yıl: `DayStatement`, `WeekStatement`, `MonthStatement`, `YearStatement`; kartta `StatementText`, ipucunda hesap hesap `ByAccount` + `AccountName`) ve "Bilanço" (`Balance`: varlıklar, borçlar, özkaynak) kartları; altta tek satır `AuditText`. Mağaza seçilince `Statement(State, Gün1, Gün2, ŞubeIndeksi)`.

9. **Ücret kuralları müdür ve şubelerde (B3):**
   - `MarketBranches.cpp` şube günü: çalışan ücreti `B.Workers * MarketStaff::FairWage(...)` zaten tabanı izliyor; üstüne sigorta: `Opex`'e `MarketStaff::EmployerShare(B.Workers * FairWage + B.ManagerWage)` eklenmeli (ve defterde `P SocialSecurity, -Pay, true, Index);`).
   - `MarketManagers.cpp` `CloseDay`: `const int64 Wages = DailyWages(State);` ardına `const int64 Social = MarketStaff::EmployerShare(Wages);` → kasadan, `LastBranchProfit`/`LastProfit`'ten düş, `P SocialSecurity, -Social, true, MarketLedger::HeadOfficeStore);`. Müdür görevden alınırken (`Dismiss`/`Replace`): `MarketStaff::SeniorityPay(DailyWage, AppointedDay, State.Day) + DailyWage * MarketStaff::SeveranceDays` ödenmeli (`P Severance, ...`).
   - Menü Personel sayfası: çalışan satırında "işverene maliyeti" (`EmployerCost(DailyWage)`), işten çıkar düğmesinin onayında `SeverancePay(State, E)` ("İhbar ve kıdem: X").

## Kararlar ve varsayımlar

- **#24 sayıları:** rakip fiyatında alma %90 (eski eğri %97,3); duyarlılık K = 3; kayıptan kaçınma ×1,4; `kvi` ×(1 + kvi); esneklik 0,5–6 aralığına kırpılır. Sonuç: süt (1,5; kvi 1) rakibin %10 üstünde ~%72, kola (4) %15 üstünde ~%40 alır. 400 günlük denemede aile dükkânının cirosu ~%9 düştü (rakip fiyatında %97 → %90). Otomatik oyuncu raporunda izlenmeli; gerekirse `ParityChance` 0,92–0,93'e çekilir.
- **#27:** "Toptancı desteği" = teklif süresince teslim alınan birim × (indirimli maliyet × 15/85). Brüt kâr = kampanyalı satışların tahmini geliri − defter maliyeti − (broşür gibi) kampanya maliyeti. Maliyet altı satış satırında son gün indirimi sayılmaz (amacı zaten eski malı ucuza satmak).
- **#30:** Online siparişler kampanya raporunun "sırasında" adedine girmez (raporlar önce kapanıyor); "öncesi" de artık online saymıyor, böylece ikisi aynı ölçü.
- **#43:** Zarar devrinin süre sınırı yok (gerçekte 5 yıl; oyunda sadelik). İpotek faiz primi +6 puan (acil kredi +12), masraf %2 (ekspertiz, tapu). İpoteğin ödenmemesi hâlâ sonuçsuz (dükkân bankaya geçmiyor) — ayrı iş.
- **#45:** Kişi başı günlük gıda harcaması oyun ölçeğinde 0,65 TL (başlangıç fiyat düzeyi; `ulkeler.json` → `economy.groceryPerPersonDay` ile ülke başına verilebilir; yoksa 0,65 × `wageFactor`). Ayar: oyunun ucuzcu şubesi (~900 TL/gün) gerçekteki en büyük zincirin bir mağazası kadar pay tutsun (BİM ≈ 3.500 mağazayla %6,5). Böylece 50 ucuzcu ≈ %0,1. **5. bölüm hedefi %2'den %0,1'e indi** (%2 bu ölçüde ~1.000 mağaza demek). Mustafa'ya soru olarak aşağıda.

- **B2:** Defter "olay anında" yazar; oyunun `LastProfit`'i bazı kalemleri bir gün sonra gösterir (teslimat eksiği, erken ödeme ücreti) — günlük net kârlar ±bir gün kayabilir, toplam aynı. Defterin net kârı işe alım bedeli, ihbar tazminatı ve vergiyi de gider sayar (oyunun `LastProfit`'i saymıyordu: #37'nin "görünmeyen kayıplar" kısmı). Vergi beyan edilince gider (nakitsiz), ödenince bilanço hareketi. Kart komisyonu satış anında gider yazılır. Şube stoğu bugünkü alış maliyetiyle değerlenir (şube satırı ortalama maliyet tutmuyor).

- **B3:** Sigorta oranları ve kıdem günleri oyun değeri (TR işveren payı ~%22,5; kıdem 30 gün/yıl gerçek kurala yakın; DE/GB/US kaba). İhbar 3 gün kaldı (gerçekte 2–8 hafta); kıdem ilk yıldan sonra devreye girdiği için "moral düşeni çıkar, yenisini al" döngüsü ilk yıl ucuz kalır — bilerek (oyunun ilk yılı zaten zor). Asgari ücret tabanı ülkenin `wageFactor`'ü ile çarpılır (Almanya'da günlük taban ~57 TL karşılığı); `FairWage` ise ülke çarpanını bilmiyor, bu yüzden yurt dışında herkes tabandan çalışır — dünya aşamasında (B5/A7) ücret eğrisi ülkeye göre ayrılmalı.

## Bilinen sorunlar

- Oyun cirosu gerçeğin ~1/10'u (#49, C). Ulusal pay bu ölçeğe göre ayarlandı; #49 düzelirse `groceryPerPersonDay` de ~10 kat büyümeli.

## Mustafa'ya sorular

1. 5. bölümün "ulusal pay" hedefi: %0,1 (≈50 ucuzcu kadar ciro) uygun mu, yoksa daha büyük bir hedef mi (ör. %0,25)?
2. Raf fiyatına tepki: rakip fiyatında %90 alma (eskisi %97) dükkânın cirosunu ~%9 düşürdü. Bot raporuna göre 0,92–0,93 yapılabilir.
