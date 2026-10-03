# Tek ekonomi ve ülke ekonomisi: teknik tasarım (D1 + D2)

03.10.2026 · Claude (Cowork). Bağlı belgeler: 09_DUNYA_YENIDEN.md (§3 S1, S2; §5; §10 kalıntılar), 10_ULKE_STANDARDI.md. Kararlar M51, M52. **Durum: tasarım; kod yazılmadı.** Bu belge, Claude'un limiti biterse Claude Code'un ya da Codex'in aynı yerden devam edebilmesi için adım adım yazıldı.

## 0. Bugünkü durum (kod okuması, 03.10.2026)

**İlk dükkân (aile dükkânı):**
- Gün `MarketSimulation::PlayDay` (stratejik ilerletme) ya da `MarketGame` (yürünen dünya) içinde geçiyor.
- Müşteri sayısı `ShoppersPerDay = 55` × `MarketDirector::TrafficFactor`. Bu, dünyanın görsel ölçeği (240 sn'lik gün); gerçek müşteri sayısı değil.
- `TrafficFactor` şu çarpımlardan oluşuyor: takvim × **MarketRivals** (prototip BİM/Migros/A101 haberleri, 0,8–1,0) × kampanyalar × **MarketCompetitors** (sokağın pay modeli, `sqrt(pay/25)` 0,6–1,5) × olaylar × `MainShopFactor` (ev ilindeki şubelerin yamyamlığı) × online × ödeme × zorluk × reklam.
- Her müşteri ayrı ayrı oynanıyor: segment, sepet, fiyat toleransı. Tolerans `MarketCompetitors::RivalPriceFactor` ile ölçülüyor (sokaktaki rakiplerin pay ağırlıklı fiyatı).
- Stok ayrıntılı: `State.Stock` (raf, depo, kapı, yolda), partiler/tazelik, toptancı siparişi.
- Personel kişi kişi; kasa, raf.

**Şube:**
- Gün `MarketBranches::CloseDay` içinde toplu (aggregate) modelle hesaplanıyor.
- Müşteri: `TripsOf(il, tür)` × pay × olgunlaşma × yamyamlık.
  - Pay = çekim / (çekim + 3 × ilin rekabeti × `MarketChains::PressureFactor`).
  - Çekim = fiyat × doluluk × hizmet × memnuniyet × olgunlaşma × şirket.
- Ürün talebi: ilin istek payları × müşteri. Satış, stok, fire, ücret, kira hep şubenin kendi kaydında (`FMarketBranch::Items`).

**Para:**
- Tek iç birim.
- `MarketPrices::ListLevel/WageIndex/LoanRate(Gün)` tek eğri: kampanya ülkesinin. Türkiye'de yerleşik eğri, diğer paketlerde ortalama ± oynaklık.
- Yurt dışı şube de aynı eğriyle yaşıyor.
- Kur yalnız dünya liginde (`MarketCountry::FxRate`).

**Sonuç:** iki rekabet modeli (MarketCompetitors + MarketRivals ↔ MarketChains), iki talep ölçeği (55 görsel müşteri ↔ gerçek alışveriş sayısı), iki satış modeli (müşteri müşteri ↔ toplu) ve tek ülke ekonomisi.

## 1. Hedef

1. **Bir mağazanın günü tek kuralla hesaplanır.** İlk dükkân "mağaza 0"dır; kendisine özel ekonomi kuralı yoktur (M: "dükkânlar hepsi aynı sistem").
2. **Rekabet tek modeldir.** İl, zincirler (`MarketChains`) ve geleneksel ticaret. Sokağın yerel rakibi (karşı dükkân) hikâyenin yerel küçük zinciridir.
3. **Talep gerçek ölçektedir.** Yürünen dünya görsel bir örneklemdir: gördüğün her müşteri modelin N gerçek müşterisini temsil eder; gün sonu sayıları modelinkiyle aynıdır.
4. **Her ülkenin kendi ekonomisi vardır.** Fiyat düzeyi, ücret endeksi ve faiz ülke başına hesaplanır. Şirketin defteri ana ülkenin parasındadır. Yabancı mağazanın günü kendi ülkesinin fiyatlarıyla hesaplanır, deftere günün kuruyla girer; kur farkı ayrı bir satırdır.

## 2. Fazlar (her biri ayrı derleme turu; her turda TEST + Smoke + kısa bot)

### Faz E1 — Ülke başına fiyat (D1, çekirdek) — yazıldı 03.10.2026, derlenmedi

1. `MarketPrices`'a ülke parametresi:
   - `PriceLevel(Country, Day)`, `ListLevel(Country, Day)`, `WageIndex(Country, Day)`, `LoanRate(Country, Day)`, `Scaled(Country, ...)`, `WageScaled(Country, ...)`.
   - Ülke başına ekonomi kaydı: `TMap<FString, FEconomy>`, paketten (`InflationMean/Vol`, `LoanSpread`, `Character`), tohum = kampanya tohumu + ülke.
   - Eski tek parametreli fonksiyonlar "kampanya ülkesi" anlamında kalır; yeni kodda kullanılmaz. Taşınma bitince silinir.
2. **Türkiye'nin yerleşik eğrisi** (`Rates[]`, `StartWage`) bu fazda olduğu yerde, "kampanya ülkesi ve paket eğri vermiyorsa" dalında kalır. D3 birleşince paketin `economy.curve` alanına taşınır (K7).
3. **Kur:** `MarketCountry::FxRate` var. Yeni `MarketCountry::ToHome(State, Country, Day)` = (yerel para / dünya birimi) oranının ana ülkeye göre değişimi; başlangıçta 1. Bunun ülke paketine dokunması gerekiyorsa D3 birleşimine kadar `MarketPrices` içinde yardımcı olarak yazılır.
4. Testler:
   - İki ülke farklı enflasyonla ayrışır.
   - `ToHome` başlangıçta 1'dir, enflasyon farkını büyük ölçüde götürür, kalan kur oynaklığıdır.
   - Türkiye'de sonuç önce/sonra aynıdır.

### Faz E2 — Tek talep ve tek rekabet (D2-1; kalıntı K1, K2)

> **Durum (Claude Code, 03.10.2026):** iki derleme turuna bölündü. **E2a derlendi, TEST 165 + 1 uyarı, Smoke geçti; ölçüm: yeni formül ilk dükkâna eskisinin 1,40 katı müşteri verdi → `FamilySiteTrips` 0,72.** **E2b derlendi, TEST 163/163, Smoke geçti; 90 gün ciro eskinin 1,083 katı. E2 bitti (03.10.2026).** Eski not: `MarketStoreDemand` (ortak formül), şube onu çağırır (aynı sonuç), ilk dükkânın yeni müşteri sayısı `FamilyShoppers` hazır ama `MarketTuning` `UnifiedDemand` anahtarıyla kapalı; ölçüm testi `StoreDemand.FamilyBeforeAfter` aynı 90 günü iki yolla oynatıp ciro oranını `OLCUM` satırıyla raporlar. **E2b (ölçümden sonra):** anahtar kalkar (yeni yol tek), `FamilyTrips` ayarı ölçüme göre konur, MarketRivals ve MarketCompetitors pay modeli silinir, karşı dükkân (Bereket) ve hikâye karakterleri silinir (M57), yürünen dünya (MarketGame) aynı sayıyı kullanır. Not: ortak formülde zorluk şubedeki gibi yarım güçle girer (eskiden ilk dükkânda tam güç); yamyamlık katsayısı ilk dükkânda 0,25 yerine şubelerdeki 0,6.

1. Yeni `MarketStoreDemand.h/.cpp`, şubenin bugünkü talep kodundan taşınır:
   - `FStoreDay` girdisi: ülke, il, tür, fiyat endeksi, doluluk, hizmet, memnuniyet, olgunlaşma, açılış günü.
   - `Shoppers(State, Input, Day)` = `TripsOf` × pay × olgunlaşma × yamyamlık × takvim × online × reklam × zorluk.
   - Pay = çekim / (çekim + rekabet × il baskısı).
2. **Şube** `MarketBranches::CloseDay` bu fonksiyonu çağırır (davranış aynı; önce/sonra testi).
3. **İlk dükkân:**
   - Gerçek müşteri sayısı aynı fonksiyondan gelir (`Format = mahalle`, ev ili, ilk dükkânın raf fiyatı, doluluğu, kasası, memnuniyeti).
   - Stratejik ilerletme bu sayıyı **görsel ölçeğe** çevirir: `Agents = Real × WorldScale` (WorldScale ≈ 55 / bugünkü gerçek sayı, sabit). Her ajan `Real/Agents` müşteri sayılır: sepet, ciro ve stok çıkışı bu ağırlıkla.
   - `MarketDirector::TrafficFactor` yalnız bu oranı döndürür. MarketRivals ve MarketCompetitors çarpanları kalkar.
4. **Tolerans:** "Rakibin fiyatı" ildeki zincirlerin mağaza ağırlıklı fiyat endeksinden (`MarketChains`, fiyat savaşı dahil) gelir. `MarketCompetitors::RivalPriceFactor` kalkar.
5. **Kaldırılır:**
   - `MarketRivals` (bütün dosya; menü ve oyun ekranındaki haber satırları da).
   - `MarketCompetitors`'ın pay modeli (`TargetShare`, `OurAttraction`, `TrafficFactor`, `RivalPriceFactor`, BİM/Migros/A101/Şok profilleri).
6. **Kalır, taşınır:** karşı dükkân (Bereket) hikâyesi `MarketChains`'te ev ilinin tek mağazalı yerel zinciri olur:
   - öfke, fiyat savaşı (`WarProvince`), personel ayartma, satılığa çıkma ve "rival.*" kararı;
   - geleneksel ticaret (bakkal, pazar) ilin `Competition` değerinin parçası.
   - Hikâye metinleri ve kararları aynı kalır.
7. `State.MarketShare` (yerel pay): ilk dükkânın ildeki payı = modelin payı. Hikâyenin %35 hedefi buna bakar.
8. **Ayar:** yeni formülde ilk dükkânın gerçek günlük müşterisi, bugünkü görsel 55 müşterinin cirosuna denk gelecek şekilde `TripsOf` ve sepet bir kez ayarlanır. Ölçü:
   - ilk 90 günün cirosu önce/sonra ±%15;
   - ilk şubenin günü ±%20.
9. Testler:
   - Aynı koşulda ilk dükkân ve bir şube aynı müşteri sayısı (±%5).
   - MarketRivals kalktı: trafik takvim dışında haberle değişmiyor.
   - Karşı dükkânın fiyat savaşı payı düşürüyor.
   - Hikâyenin pay hedefi çalışıyor.

### Faz E3 — Tek satış ve tek gün (D2-2; kalıntı K3, K4, K5, K6)

> **Durum (Claude Code, 03.10.2026):** üç derleme turuna bölündü, çünkü ilk dükkânın stok kaydı 55 dosyada 330 yerde, şube kaydı 463 yerde kullanılıyor; tek seferde birleştirmek derleyicisiz çok riskli.
> - **E3a — kalıntılar (yazıldı, derlenmedi):** her gün çalışan `MarketManagers::Migrate` kalktı (değerler kişi atanırken veriliyordu; isimler işe alımda kaydediliyor); `bSecondStore` kayıttan çıktı (şube sayısına bakılır); v0.1 ikinci şube nakit şartı (`ExpandCash`, `ExpandBlock`) kalktı, ilk şube şartları borç + kârlı gün + yerel pay olarak kaldı; v0.1 şube toplamı (`LastBranchProfit` 800 + pay × 35) ve onun `BranchResult` kaydı kalktı (hesap adı "Bağlı şirket sonucu", yalnız satın aldığımız zincirler); kayıt sürümü 10.
> - **E3b — tek satış:** ilk dükkânın hızlı ilerletme günü şubenin ürün ürün satış formülünü kullanır; yürünen dünya aynı sayıları örnekler.
> - **E3c — tek mağaza kaydı:** `FMarketBranch` ürün kaydı ilk dükkânın `FMarketStock`'u olur; tazelik, toptancı siparişi, personel şubeye açılır. Kalan K4 (personelsiz eski "v0.1 bayrakları": `bCashier/Stockers`) burada kalkar.

1. Şubenin ürün ürün satışı (istek payı × müşteri × raftaki stok × fiyat çekimi) **ilk dükkânın stratejik gününe de uygulanır.**
   - Ajan oynatma yalnız yürünen dünyada kalır ve aynı sayıları örnekler: ajanın sepeti o günün model satışından çekilir.
   - Böylece "aynı koşulda aynı ay sonucu" kesinleşir.
2. İlk dükkânın ayrıntılı sistemleri (tazelik partileri, toptancı siparişi, kişi kişi personel) **şubeye de açılır**, ayrı kalmaz. Tek tip mağaza kaydı: `FMarketStore` (bugünkü `FMarketBranch` + `State.Stock`'un alanları). İlk dükkân `Stores[0]`.
   - Bu adım büyük olduğu için kendi içinde ikiye bölünür:
     - (a) kayıt birleşimi, davranış aynı;
     - (b) şubeye eksik sistemlerin açılması.
3. **Kalkar:**
   - `MarketManagers::Migrate` her gün çağrısı ve diğer "older save" dalları (K4);
   - `bSecondStore`;
   - v0.1 ikinci şube şartları (`ExpandCash`) (K5);
   - `BranchResult` (K6).
4. Testler:
   - İki mağaza aynı ay sonucu.
   - Para/stok/defter farkı 0.
   - Kayıt sürümü artar.

### Faz E4 — Yabancı mağazanın parası

1. Mağaza günü kendi ülkesinin `ListLevel/WageIndex`'iyle hesaplanır (ürün fiyatı = katalog başlangıç fiyatı × ülkenin liste düzeyi × ülkenin fiyat/gelir çarpanı).
2. Günün bütün para hareketleri (satış, mal, ücret, kira, gider) `ToHome` ile ana paraya çevrilip kasaya ve deftere yazılır.
3. Kur farkı: mağazanın stok değeri ve depozitosu yerel parada tutulur; ay sonunda ana paraya yeniden değerlenir, fark `MarketLedger::EAccount::FxDifference` (yeni) satırına gider.
4. Banka: kredi hangi ülkenin bankasındansa o ülkenin faizi; geri ödeme o paradan, kurla.
5. Testler:
   - Kur sabitken yabancı mağaza ana ülkedeki eşiyle aynı kârı verir.
   - Kur %20 kayınca kâr ana parada %20 değişir, fark satırı doğru.
   - Defter denetimi 0.

## 3. Sıra ve bağımlılık

E1 → E2 → E3 → E4. E1 ve E2 birbirinden bağımsız derlenebilir (önce E1 daha küçük). D3/D5 (Claude Code) birleşimi E2'den önce ya da sonra olabilir; `MarketCompetitors` ve `MarketChains` iki tarafın da dokunduğu dosyalardır, birleşimde Claude Code'un D3 bloğu korunur, pay modeli silinir.

## 4. Devam notu (limit biterse)

- Bu belgenin hangi fazı bitti: DURUM.md "Kısaca" en üst satırı ve GUNLUK.md'nin en üst girişi söyler.
- Her faz bir commit; mesajı "E1: …", "E2: …".
- Faz yarımsa: DURUM'da dosya adı ve adım; derlenmediyse "derlenmedi".
- Bekleyen C16 kodu: `Docs/Surec/bekleyen/C16/BENIOKU.md`.
