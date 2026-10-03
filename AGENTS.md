# AGENTS.md — Miras Market ortak çalışma kuralları

Bu projeyi üç taraf birlikte geliştirir: **Mustafa** (ürün sahibi, son karar), **Claude** ve **Codex**. Bir ajanın limiti bitince diğeri kaldığı yerden devam eder. Bu dosya ikisinin de ilk okuduğu yerdir; kuralları burada tut, ayrıntıyı bağlantılı belgelere koy.

## 1. Oturum başında (zorunlu, 2 dakika)

1. `Docs/Surec/DURUM.md` oku: şu an ne çalışıyor, ne yarım, sıradaki adım.
2. `Docs/Surec/GOREVLER.md` içinde **Devam ediyor** satırlarına bak. Başka ajanın üstündeki işi, DURUM'daki devam notu izin vermiyorsa ele alma.
3. `Docs/Surec/GUNLUK.md` son iki girişini oku.
4. Git kuruluysa `git status` ve `git log -5 --oneline`. Beklenmeyen değişiklik varsa silme; önce Mustafa'ya sor.

## 2. Oturum sonunda veya limit yaklaşınca (zorunlu)

1. `Docs/Surec/DURUM.md` dosyasını güncelle. Yarım iş varsa **Devam notu** bölümüne dosya adı, adım ve derleme/test durumunu yaz.
2. `Docs/Surec/GUNLUK.md` dosyasının **en üstüne** bir giriş ekle: tarih, ajan, yapılanlar, doğrulama, sıradaki adım.
3. `Docs/Surec/GOREVLER.md` içindeki durumları güncelle.
4. Git varsa anlamlı bir commit at. Mesaj Türkçe olabilir, ilk satır kısa olsun.

Limit aniden biterse sonraki ajan GUNLUK/DURUM ile dosyaların son hâlini karşılaştırarak eksiği tamamlar.

## 3. Proje haritası

| Yol | İçerik |
|---|---|
| `Source/MirasMarket/MarketEconomy.*` | Para, stok, sipariş, satış, gün sonu, kayıt uzlaştırma. Dünyadan bağımsız ve test edilebilir. |
| `Source/MirasMarket/ProductCatalog.*` | `Config/products.json` okuma/yazma, kutu atlas yerleşimi (`FBoxPackageLayout`). Oyun ve stüdyo ortak kullanır. |
| `Source/MirasMarket/MarketGame.*` | Prototip GameMode, karakter, sahne, müşteri, HUD. Tek dosyayı daha fazla büyütme; yeni sistemleri ayrı sınıflara ayır. |
| `Source/MirasMarket/MarketAutomation.cpp` | `-MirasSmoke` ve `-MirasCapture` otomatik oynanış/görüntü çalıştırmaları (`AMarketGameMode::TickAutomation`). |
| `Source/MirasMarket/MarketArrange.cpp` | Oyunda raf dizme modu (R): nişan, hayalet önizleme, RAF DÜZENİ paneli verisi. |
| `Source/MirasMarket/Planogram.*` | Raf planı verisi (`Config/planograms.json`, şema v3), ekipman ölçüleri, genişlik/derinlik/istif hesapları. |
| `Source/MirasMarket/StaffPlanner.*` | Reyon görevlisinin kararları (dünyadan bağımsız, test edilir): raf doldur, rafta olmayan ürünü kategorisinin reyonuna koy, bir koli almayan bloğu genişlet. |
| `Source/MirasMarket/MarketDemand.*` | Müşteri kararları (dünyadan bağımsız, test edilir): hangi ürünü ister, rakip fiyatına göre alır mı, kayıp nedeni sayımı, gün raporunun en büyük 3 sorunu. |
| `Source/MirasMarket/MarketBasket.*` | 1–4 ürünlü alışveriş listesi, kategori içi ikame ve kayıtlı tekrar müşteri memnuniyeti. |
| `Source/MirasMarket/MarketCampaign.*` | Babadan kalan borç, taksit, ikinci şube kilidi ve yedi günlük hafta toplamları. |
| `Source/MirasMarket/MarketStoreDemand.*` | E2: **tek müşteri formülü** (dünyadan bağımsız, test edilir): her mağazanın (ilk dükkân dahil) günlük müşterisi = ilin gidişi × pay (çekim / çekim + rekabet × zincir baskısı) × alışkanlık × yamyamlık; ilk dükkânın yerel payı. Eski MarketRivals ve MarketCompetitors kaldırıldı (E2, M57). |
| `Source/MirasMarket/MarketProductDemand.*` | E3b: **ortak ürün isteği** (dünyadan bağımsız, test edilir): segmentin bir ürünü ne kadar istediği (zevk × takvim × salgın alımı) ve raf fiyatının rakibe göre bir isteği satışa çevirme olasılığı. İlk dükkânın müşterileri listelerini ve fiyat kararlarını tek tek buradan çeker; şubeler aynı sayıları günlük beklenen değer olarak kullanır. |
| `Source/MirasMarket/MarketSubsidiaries.*` | M65: marka (kısa ad) ve her ülkede şirket: ana şirket ve alt şirketler, tescilli ad (ülke paketindeki şirket türleri, oyuncu değiştirebilir), kuruluş masrafı, aylık kâr aktarımı ve stopaj; rakiplerin tescilli adı. |
| `Source/MirasMarket/MarketResearch.*` | M58: yeni ülkeye girmeden önce ücretli pazar araştırması (30–45 gün, bir yıl geçerli, rapor: en iyi iller, zincir payları, ülkeye özgü mağaza türleri). |
| `Source/MirasMarket/MarketMenu*` | M ile açılan tıklanabilir yönetim menüsü (10 sayfa). `MarketMenuWidget.cpp` çerçeve, yapı taşları, Özet/Sipariş/Raporlar; `MarketMenuPages.cpp` Fiyat, Kampanyalar, Rakipler, Personel, Finans, Satış kanalları, Şubeler; `MarketMenuInternal.h` ortak yardımcılar; `MarketMenu.cpp` aç/kapat, komutlar ve `Todos()`. Ayrıntı `Docs/MENU.md`. |
| `Source/MirasMarket/MarketHudWidget.*` | Sade oyun ekranı (A1): gün/saat, kasa, borç, en çok 3 bildirim (`Todos()`), ipucu. Geri kalan her şey menüde. |
| `Source/MirasMarket/MarketMap.*`, `MarketRetail.*` | Şubeler sayfasının Türkiye il haritası (`Config/iller.json`) ve Rakipler sayfasının ulusal/uluslararası pazar verisi. |
| `Source/MirasMarket/MarketWorkers.cpp` | Oyundaki reyon görevlileri: yürüme, koli taşıma, rafa tek tek dizme (`AMarketGameMode::TickWorkers`). |
| `Source/MirasMarket/MarketDirector.*` | **Oyuna tek bağlantı noktası**: anlık çarpanlar (trafik, talep, sipariş öngörüsü) ve gün kapanışında bütün arka plan sistemlerini sırayla çağırır. Yeni sistem MarketGame.cpp'ye değil buraya bağlanır. Kurgu: `Docs/Kurgu/`. |
| `Source/MirasMarket/MarketCalendar.*`, `MarketGoods.*` | Takvim (gün 1 = 7 Mart 2011 Pzt), mevsim, deterministik hava, gerçek bayram/tatiller, maaş günü; kategori → talep grubu sınıflandırması. |
| `Source/MirasMarket/MarketCustomers.*` | Müşteri segmentleri (emekli, aile, iş çıkışı, öğrenci, esnaf, çocuk): saat, liste zevki, adet, bütçe, fiyat toleransı, sabır, yürüme hızı, raf önünde bakma süresi. Mahalle müşterisinin segmenti sabittir; Nermin teyze (id 0) emeklidir. |
| `Source/MirasMarket/MarketPrices.*`, `MarketSuppliers.*` | Enflasyon (oyunun kendi eğrisi, birebir tarih değil), aylık liste düzeyi, asgari ücret endeksi (fiyatın biraz önünde), kredi faizi; toptancılar (Trakya Gıda/Selim, Özdemir Toptan), güven, vade, hacim iskontosu, geç ödeme, toptancı fiyatı = katalog maliyeti × 1,10, kampanyaya göre aylık ±%2 liste oynaması, ay başı zam listesi, zammı rafa yansıtma. Oyun `CatalogBase` (2011 değerleri) → `Products` (bugünün fiyatları). |
| `Source/MirasMarket/MarketPromotions.*` | Oyuncu kampanyaları: reyon indirimi, 3 al 2 öde, broşür, gondol başı, toptancı destekli teklif; ödenen fiyat, listeye girme ilgisi, trafik, alış maliyeti; bitince sonuç raporu. |
| `Source/MirasMarket/MarketEvents.*`, `MarketStory.*` | Karar altyapısı (bekleyen seçim, varsayılan, son gün), süreli etkiler (trafik, ilgi, fiyat hoşgörüsü, maliyet), mahalle olayları (dolap, elektrik, zabıta, düğün, şikâyet, kaldırım, derbi, taziye, kamyon, kar); hikâye bölümleri ve hedefleri, karakter sahneleri (Nermin teyze, Cem, Selim, Kadir Bey), sat/devam, dükkân kimliği, hatıralar. |
| `Source/MirasMarket/MarketFreshness.*`, `MarketCredit.*`, `MarketFinance.*` | Tazelik (parti, FEFO, son gün %30 indirim ya da bağış, fire); veresiye defteri (limit, maaş günü tahsilat, batık); banka kredisi, taksit, nakit sıkıntısı merdiveni (oyun bitmez), ay sonu raporu. |
| `Source/MirasMarket/MarketLayout.*` | Yeni şubenin otomatik raf planı: müşteri yolu sırası (içecek girişte, süt arkada, temizlik gıdadan ayrı), talebe/marja göre yüz, göz hizası/ağır alt raf, marka blokları; oyuncunun elle dizme kurallarıyla (`MarketPlanogramEdit`). Ana dükkân elle dizilmeye devam eder. |
| `Source/MirasMarket/MarketBranches.*` | Şubeler: 7 kurgu semt (nüfus, gelir, kira, müşteri karışımı, rakip yoğunluğu, komşuluk), açılış süreci, uzak şube günlük simülasyonu (pay, talep, stok, müdür siparişi, fiyat), olgunlaşma, yamyamlık, müdür terfisi, kapatma, eski "ikinci şube" kayıtlarının dönüşümü. |
| `Source/MirasMarket/MarketOnline.*`, `MarketPayments.*` | İnternet mağazacılığı: telefon (2011+), web (2014+), platform "Getirsin" (2016+); kurye kapasitesi, depodan/raftan toplama, ikame kuralı, itibar/yıldız, ilçe online payının dükkândan müşteri çekmesi, 2020-21 profili. Ödeme: nakit/kart/yemek kartı, POS kirası ve komisyonu, kart parasının ertesi gün gelmesi. |
| `Source/MirasMarket/MarketMotion.*` | İnsan hareketi zekâsı (dünyadan bağımsız, test edilir): kişiye sabit yürüme tarzı ve hızı, alışveriş listesinin yürüme sırası, raf önü süresi, kuyruktan vazgeçme, kalabalıkta yol verme (sağdan geçme, arkada yavaşlama), tanıdık sohbeti. `MarketPeople` animasyonu bununla sürer. |
| `Source/MirasMarket/MarketSimulation.*` | Stratejik ilerletme ve zorluk: aile dükkânının gününü yürüyen insan olmadan aynı kurallarla oynatır (müşteri, raf kararı, ikame, ödeme, kasa, gün kapanışı); ailenin rutini (zammı rafa yansıt, vergi, borç taksiti, raf doldurma, önerilen sipariş); karar bekleyince/kasa eksiye düşünce/hafta bitince durur. Rahat/Normal/Zor. |
| `Source/MirasMarket/MarketCompany.*` | Şirket büyümesi (dünyadan bağımsız, test edilir): Lüleburgaz dışı şehir mağazaları toplu modelle (Trakya, Türkiye, Kırcaali/Filibe/Köstence), bölge deposu, kamyon, merkezi satın alma, "Miras" özel markası, karanlık mağaza, ulusal pay, 4–7. bölüm hedefleri, liderlik yılı ve "Miras" sonu. |
| `Source/MirasMarket/MarketChains.*`, `MarketBrands.*`, `MarketSourcing.*`, `MarketDepartments.*`, `MarketStoreViews.*` | Akış C: rakip zincirleri ve dünya ligi (il/bölge/ülke/dünya kadrosu, aylık kararlar, fiyat savaşı, ezeli rakip, satın alma), markaların reyon yarışı (M25), tedarik ağı (G-083: hat kademeleri, alım gücü), reyonlar (M26: taze ve gıda dışı, mağaza türü başına), şube mağaza görünümü ölçüleri. |
| `Source/MirasMarket/MarketLedger.*`, `MarketEras.*`, `MarketGoals.*` | Akış B: muhasebe defteri (her para hareketi `Post`, gelir tablosu, bilanço, kasa denetimi), dönem olayları ve çarpanları (kur şoku, durgunluk, salgın, enflasyon, toparlanma), hedefler/ilkler/rekorlar/kutlamalar ve ritim koruyucusu. |
| `Source/MirasMarket/MarketAutoPlay*.*`, `MirasAutoPlayCommandlet.*`, `MarketBranchVisit.*`, `MarketMenuCapture.cpp` | Akış A: otomatik oyuncu ve denge raporu (`AUTOPLAY.cmd`), şube ziyareti, menü ekran görüntüsü otomasyonu. |
| `Docs/Kurgu/` | Claude'un kurgu kitabı (`00_KURGU_KITABI.md`) ve bütün konuların karar tablosu (`01_KARARLAR.md`). Yeni konu önce kararlar tablosuna yazılır. |
| `Source/MirasMarket/MarketManagers.*` | G-086b yönetim kademeleri (dünyadan bağımsız, test edilir): mağaza müdürü tarzı/morali/kararları, il/bölge/direktör/ülke müdürleri, doğrudan bağlı sayımı ve 5 kişi sınırı, denetim etkileri, ücretler. Menüde Mağazalar › Yönetim. |
| `Source/MirasMarket/MarketStoreAssign.*` | G-088 Claude tarafı: (ülke, il, tür) → gezilebilir mağaza seçimi, raf kategorisi yardımcıları, mağaza ölçülerinden oyun çarpanları (`FStoreMeasures`). Görünüm/kurulum Codex'in `MarketStoreKit`'inde. |
| `Source/MirasMarket/MarketStaff.*` | Personel ve muhasebe (dünyadan bağımsız, test edilir): kişi olarak çalışanlar ve aday havuzu, kasiyer hızı ve kasa farkı, görevli hızı/taşıma/yorgunluk, moral ve istifa, İK müdürü, mali müşavir, haftalık vergi. Ayrıntı `Docs/PERSONEL_VE_MUHASEBE.md`. |
| `Source/MirasMarket/MarketOrderAdvice.*` | Sipariş yardımı (dünyadan bağımsız, test edilir): önerilen koli sayısı, L ile listeyi öneriye yükseltme, 50 TL asgari sipariş. |
| `Source/MirasMarket/MarketDelivery.cpp` | Çok ürünlü sipariş taslağı, arka kapıdaki fiziksel koliler ve oyuncunun mal kabul taşıması. |
| `Source/MirasMarket/PlanogramEdit.*` | Blok blok elle dizme kuralları (`PlanBlock`, `AddBlock`, `AddToRowEnd`, taşı/kaldır/önde/yön/istif/aralık). Oyun ve editör ortak kullanır; reyon görevlileri de (`StaffPlanner`) bunu kullanır. |
| `Source/MirasMarket/MarketTests.cpp` | Unreal otomasyon testleri (`MirasMarket.*`). |
| `Source/MirasMarketStudio/` | Yalnızca editörde çalışan **Ürün Stüdyosu** ve **Raf Planı** modülü. Arayüz: `SProductStudio`, `SPlanogramStudio`, `SStudioViewport`, `StudioStyle`. Arka uç (`StudioBackend.h`) konulara bölünmüştür: `StudioBackend.cpp` (dosya/görsel/açılım), `StudioMeshes.cpp` (malzeme, kutu/şekil/model 3B), `StudioPresets.cpp` (hazır ambalajlar, parça renkleri), `StudioProducts.cpp` (kontrol, yayımla), `StudioPrompts.cpp` (promptlar, teslim klasörü). Ortak iç yardımcılar `StudioBackendInternal.h`. |
| `Config/products.json` | Ürün kataloğu, şema v2. Stüdyo yazar; elle de düzenlenebilir. |
| `Content/Products/` | Stüdyonun ürettiği varlıklar: `Packages/<ambalaj>/SM_*`, `Items/<ürün>/T_*_Label`, `MI_*`, `Materials/M_ProductLabel`. Elle düzenleme. |
| `AssetInbox/` | Kullanıcının getirdiği ham dosyaların kopyası ve manifestler. Stüdyo yazar; kaynak arşivi olarak saklanır. |
| `Docs/Planlama/` | v0.2 tasarım paketi (hedef oyun). |
| `Docs/Surec/` | Devir belgeleri: DURUM, GOREVLER, GUNLUK. |
| `Docs/URUN_STUDYOSU.md` | Stüdyonun kullanım kılavuzu ve dosya standardı. |
| `Docs/METAHUMAN_REHBERI.md` | İnsan görünüşü, ortak animasyon dönüştürme ve kalabalık standardı. |
| `Docs/MAL_KABUL.md` | Çok ürünlü sipariş listesi, arka kapı teslimatı ve depoya taşıma kontrolleri. |
| `Docs/Uretim/` | Mustafa'nın dış ajanlara vereceği **kısa, kopyala-yapıştır promptlar** (A1/A2 kutu, B1/B2 şişe, C1/C2 poşet, D araştırma, E kontrol) ve marka/ürün listesi. Stüdyo davranışı değişirse bu promptları da güncelle. |
| `Uretim/` | Dış ajanların teslim klasörü: `Uretim/<ürün>/model/`, `Uretim/<ürün>/<yıl>/`. |
| `Docs/Uretim/Sablonlar/` | Stüdyonun ürün verisiyle doldurduğu prompt şablonları (`{{YER_TUTUCU}}`). Yeni yer tutucu eklersen `StudioPrompts.cpp` → `BuildPrompt` içine de ekle. |
| `Config/ambalajlar.json` | Hazır ambalaj kütüphanesi (stüdyo 3B şekli kendisi üretir). Ölçü değişirse id de değişir. |
| `Tools/MetaHuman/hazirla_animasyon.py` | Ortak IK/retarget varlıklarını ve MetaHuman yürüyüş/bekleme kopyalarını üretir. |
| `Tools/Arsiv/katalog_olustur.py` | Arşiv. Katalogu ürün listesinden sıfırdan üretirdi; stüdyodaki değişiklikleri ve ambalaj atamalarını SİLER. Kullanma. |

## 4. Komutlar (Windows, proje kökü)

| Komut | Ne yapar |
|---|---|
| `TEST.cmd /q` | `Test.ps1`'i çalıştırır; çıktı `Saved/Logs/TEST_son.log`. |
| `DERLE.cmd /q` | Editor hedefini derler. Çıktı: `Saved/Logs/DERLE_son.log`. **Unreal Editor açıkken derleme yapma.** |
| `powershell -File Test.ps1` | Otomasyon testleri; rapor `Saved/TestReports/index.json`. En az 7 test başarılı olmalı. |
| `powershell -File SmokeTest.ps1` | Gerçek oyun dünyasında otomatik oynanış kontrolü. |
| `OYNA.cmd` | Oyunu pencerede açar. |
| `STUDYO.cmd` | Editörü açar ve Ürün Stüdyosu'nu getirir (Tools > Ürün Stüdyosu ile de açılır). |
| `python Tools/escape_unicode.py` | C++ kaynaklarındaki Türkçe karakterleri `\uXXXX` kaçışına çevirir. |

Motor yolu: `C:\Program Files\Epic Games\UE_5.8` (UE 5.8.3). Farklıysa betiklere `-EngineRoot` ver.

## 5. Kod kuralları

- **Para** her yerde `int64` kuruş. JSON'da TL ve iki ondalık.
- **Ürün kimliği (`id`) yayımlandıktan sonra değiştirilmez.** Kayıtlar stoğu id ile eşleştirir; id değişirse o ürünün stoğu sıfırlanır.
- `products.json` şema değişikliği geriye uyumlu olmalı (yeni alanlar isteğe bağlı). Şemayı değiştiren, `MarketCatalog::Parse/Serialize`, testleri ve `Docs/URUN_STUDYOSU.md` dosyasını birlikte günceller.
- C++ kaynakları ASCII kalır. Türkçe metni yaz, sonra `python Tools/escape_unicode.py` çalıştır. Oyun HUD metinleri şimdilik ASCII Türkçedir.
- `.uasset` dosyalarını metin gibi düzenleme. Varlıkları stüdyo veya editör Python betikleri üretir.
- Yeni oyun kuralı = yeni test. Para/stok korunumu ve iki kez uygulama testleri öncelikli.
- **Karar M52 — motor ülke bilmez:** kodda ülke kodu (`"tr"` vb.) ya da ülkeye özel dal yazılmaz; ülkeye özgü her sayı, ad, tarih ve alışkanlık ülke paketindedir (`Config/ulkeler.json`). Yeni ülke yalnız paketle eklenir. Ayrıntı `Docs/Kurgu/10_ULKE_STANDARDI.md`.
- **Karar M27:** yayına kadar eski kayıt uyumu yok. `Migrate` ya da eski kayıt dalı yazılmaz; kayıt biçimi değişince `FMarketState::CurrentVersion` artar, eski kayıt yüklenmez.
- Kutu ambalajında yüz ↔ UV eşlemesi yalnızca `FBoxPackageLayout` üzerinden yapılır; başka yerde koordinat kopyalama.

## 6. "Bitti" demenin şartı

Değişiklik ancak şunlardan sonra bitmiş sayılır: `DERLE.cmd` başarılı, `Test.ps1` başarılı. Oyun akışına dokunduysan `SmokeTest.ps1` de başarılı olmalı. Derleyemediysen DURUM'a **"derlenmedi"** yaz; derlenmemiş işi tamamlanmış gibi gösterme.

## 7. Ajanlara özel notlar

- **Gidiş yolu (Mustafa, 30.09.2026):** sıra `Docs/Kurgu/06_GIDIS_YOLU.md`'de. Önce oyunun aklı, sonra dükkân içi simülasyon. (02.10.2026, Mustafa: "aynı anda en fazla bir derlenmemiş Claude işi" kuralı kaldırıldı. Derlenmemiş iş yine DURUM'da "derlenmedi" diye işaretlenir.)

- **Codex:** Aynı klasörde yerel çalışır. Kabuk erişimin varsa derleme ve testleri sen çalıştır, sonuçları GUNLUK'e yaz.
- **Claude (Cowork):** Klasöre köprü üzerinden erişir. Kabuk yoksa adımları `Saved/Claude/is.cmd` dosyasına yazar; **Mustafa** kökteki `CLAUDE_KOS.cmd`'ye çift tıklar (02.10.2026: derleme, test ve bot koşusu Mustafa'da), bitince Claude'a yazar; Claude çıktıyı `Saved/Claude/son.log`'dan (bitiş satırı `CLAUDE_KOS_BITTI`) ve bot klasörlerinden okuyup kontrol eder. Çalışan bir `is.cmd`'nin üstüne yazma. **Dosya yazdıktan sonra geri okuyup karşılaştır:** 27.09.2026'da bir aktarım iki dosyayı eski hâliyle yazdı ve derleme bu yüzden kırıldı. 29.09.2026'da da aynısı oldu: aynı çıktı yolundan ikinci kez yazılan dosya ilk hâliyle gitti. Bir dosyayı ikinci kez yazarken her seferinde **yeni bir çıktı yolu** kullan (ör. `outputs/r3/...`) ve geri okuyup karşılaştır.
- Mustafa'ya teknik terimi az, sonucu net anlat. Bir karar onun tercihine bağlıysa sor. Varsayım yaptıysan GUNLUK'e yaz.

## 8. İş bölümü ve dosya sahipliği (Mustafa, 30.09.2026)

Aynı dosyaya iki ajan aynı anda dokunmaz. Sahibi olmayan dosyada değişiklik gerekiyorsa yapma; `GOREVLER.md` ilgili satırına ve `GUNLUK.md`'ye not yaz.

| Alan | Sahip | Dosyalar |
|---|---|---|
| Oyunun aklı, kurgu, ekonomi, menü | Claude | `MarketEconomy.*`, `MarketBranches.*`, `MarketCompany.*`, `MarketStoreDemand.*`, `MarketProductDemand.*`, `MarketSubsidiaries.*`, `MarketResearch.*`, `MarketChains.*`, `MarketStaff.*`, `MarketSuppliers.*`, `MarketPrices.*`, `MarketCountry.*`, `MarketStart.*`, `MarketStory.*`, `MarketEvents.*`, `MarketDirector.*`, `MarketSimulation.*`, `MarketLayout.*`, `MarketMenu*`, `MarketHudWidget.*`, `MarketMap.*`, `MarketTheme.*`, `Config/*.json` (magazalar.json hariç), `Docs/Kurgu/` |
| Mağaza görünümleri, 3B, dünya | Codex | `MarketStoreKit.*`, `Planogram.*` (ekipman tanımları), `MarketGame.*`, `MarketWorkers.cpp`, `MarketVisuals.*`, `MarketArrange.cpp`, `MarketPeople.*`, `MarketAutomation.cpp`, `Source/MirasMarketStudio/`, `Tools/Blender/`, `Tools/*.py`, `Config/magazalar.json`, `AssetInbox/`, `Content/Stores/` |
| Derleme, test, smoke, bot | Mustafa (Claude hazırlar ve kontrol eder); Codex varsa kabuktan | Claude adımları `Saved/Claude/is.cmd`'ye yazar, Mustafa `CLAUDE_KOS.cmd` ile çalıştırır, Claude `son.log` ve raporları okur. Sonuç GUNLUK'e yazılır |
| Karar, oyun testi, ambalaj | Mustafa | — |
