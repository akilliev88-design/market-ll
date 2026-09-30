# Oyunun aklı: üç akışa bölünmüş iş (Aşama 1)

Tarih: 30.09.2026 · Karar: Mustafa · Yazan: Claude (Cowork)
Dayanak: `06_GIDIS_YOLU.md` (Aşama 1, A1–A8), `05_YOL_HARITASI.md` §2 (hata numaraları), `02_DERIN_INCELEME.md` (hataların ayrıntısı), `01_KARARLAR.md`, `00_KURGU_KITABI.md`, `AGENTS.md`.

Aşama 1 üç ajana bölündü. Üçü **aynı anda** çalışır, her biri kendi git dalında ve kendi klasöründe. En sonunda Claude (Cowork) hepsini inceler, oyuna ve menüye bağlar.

| Akış | Ajan | Konu | Dal ve klasör |
|---|---|---|---|
| **A** | Codex | Doğrulama (Aşama 0), otomatik oyuncu, zaman ve tur | `akis-a` · `C:\Users\mtass\Desktop\market-ll-A` |
| **B** | Claude Code | Denge hataları, muhasebe defteri, ücret ve sigorta, dönem olayları, (vakit kalırsa) dünya ve oyunun sonu | `akis-b` · `C:\Users\mtass\Desktop\market-ll-B` |
| **C** | Claude (Cowork) | Mağaza ağının aklı, il pazarı ve rakipler, tedarik ağı, markaların reyon yarışı (M25); en sonda **bağlama** (Director, menü, kayıt) | `main` · `C:\Users\mtass\Desktop\market-ll` |

## 1. Başlangıç sırası

1. **Codex, Aşama 0'ı `main` üzerinde yapar:** bugünkü hâli derler, test eder, smoke çalıştırır, derleme hatalarını düzeltir, **commit atar ve GitHub'a push eder**. Commit mesajı `Aşama 0: ...` ile başlar.
2. Aşama 0 commit'i gelmeden hiçbir akış başlamaz. B ve C, `git log` içinde `Aşama 0` commit'ini görene kadar bekler.
3. A ve B kendi klasörlerini git worktree ile açar (proje kökünden):
   ```
   git worktree add ..\market-ll-A -b akis-a      (Codex)
   git worktree add ..\market-ll-B -b akis-b      (Claude Code)
   ```
   Worktree'de `git lfs pull` çalıştır; ilk `DERLE.cmd /q` uzun sürer (Binaries/Intermediate sıfırdan kurulur). Betikler göreli yol kullanır, worktree'de aynen çalışır.

## 2. Dosya sahipliği (akış süresince)

Bir dosyayı yalnız sahibi değiştirir. Başka akışın dosyasında değişiklik gerekiyorsa **yapma**; kendi akış notuna "C'ye / B'ye istek" diye yaz.

| Akış | Sahip olduğu dosyalar |
|---|---|
| **A** | `MarketSimulation.*`, yeni `MarketAutoPlay.*` (+ testleri), yeni commandlet dosyası, `MarketAutomation.cpp`, `MarketGame.*` (Codex'in zaten), `Test.ps1`, `SmokeTest.ps1`, yeni `AUTOPLAY.cmd` |
| **B** | `MarketDemand.*`, `MarketPromotions.*`, `MarketOnline.*`, `MarketPayments.*`, `MarketFinance.*`, `MarketCredit.*`, `MarketFreshness.*`, `MarketStaff.*`, `MarketCompany.*`, `MarketCountry.*`, `MarketPrices.*`, `MarketCalendar.*`, `MarketStory.*`, `MarketEvents.*`, `MarketCampaign.*`, yeni `MarketLedger.*`, `MarketEras.*`, `Config/ulkeler.json`, bunların testleri |
| **C** | `MarketBranches.*`, `MarketDepots.*`, `MarketManagers.*`, `MarketStoreAssign.*`, `MarketCompetitors.*`, `MarketRivals.*`, `MarketSuppliers.*`, `MarketLayout.*`, `MarketStart.*`, `MarketCustomers.*`, `MarketBasket.*`, yeni `MarketBrands.*`, yeni `MarketLeague.*`, `Config/markalar.json`, `Config/zincirler.json`; **bağlama dosyaları:** `MarketDirector.*`, `MarketMenu*`, `MarketHudWidget.*`, `MarketMap.*`, `MarketTheme.*`, `MarketRetail.*` |

**Ortak iki dosya, işaretli bloklarla:**
- `MarketEconomy.h`: `FMarketState`'in **sonuna** her akış kendi bloğunu açar (`// ===== Akış A =====` … `// ===== Akış A son =====`). Yeni durum mümkünse akışın kendi başlığındaki bir `USTRUCT`'tadır ve `FMarketState`'e **tek alan** olarak eklenir. Mevcut alanlar ve yapılar değiştirilmez; gerekiyorsa C'ye istek yazılır. Kendi yapınızın başlığı `MarketEconomy.h`'yi include etmemeli (döngü olmasın).
- `MarketDirector.cpp`: A ve B yalnız kendi işaretli bloklarına **satır ekler** (`CloseDay` sırasına kendi sisteminin günlük kapanışı, `Command` içine kendi komutları). Mevcut satırlar değişmez. Kalıcı yerleşimi C bağlamada yapar.

**Menüye kimse dokunmaz, yalnız C.** A ve B menüde görünmesi gerekenleri akış notuna yazar (sayfa, kart, düğme, hangi işlev, hangi komut).

**Süreç belgeleri:** A ve B akış boyunca `DURUM.md`, `GOREVLER.md`, `GUNLUK.md`, `AGENTS.md`'yi değiştirmez. Her biri `Docs/Surec/akislar/A.md` ve `B.md` dosyasını tutar (§5). C birleştirmede bunları süreç belgelerine işler. İstisna: Codex Aşama 0'ı `main`'de normal kurallarla (GUNLUK, DURUM) kapatır.

## 3. Ortak kalite kuralları (hepsi zorunlu)

- `AGENTS.md` §5 kod kuralları: para `int64` kuruş; C++ kaynakları ASCII, Türkçe metin `\uXXXX` (`python Tools/escape_unicode.py`); `products.json` şeması geriye uyumlu.
- **Dünyadan bağımsız ve test edilir mantık:** yeni sistem kendi `namespace`'inde, `FMarketState` üzerinde çalışır, `UWorld`'e ihtiyaç duymaz. Her yeni kural = yeni otomasyon testi (`MirasMarket.<Modül>.<Konu>`). Para ve stok korunumu, iki kez uygulama ve eski kayıt uyumluluğu testleri öncelikli.
- **Belirlenimci:** rastgelelik yalnız kampanya tohumu + gün + sabit tuzla (`FRandomStream` ya da mevcut `Roll` yardımcıları). `FMath::Rand` ve saat kullanılmaz; aynı tohum aynı sonucu verir.
- **Eski kayıtlar:** yeni alanlar varsayılanla açılır; gerekiyorsa `Migrate` (tek sefer, iki kez uygulanınca değişmez).
- **Derleme tuzakları:** yerel ad gölgelemesi (C4456–C4459) derlemeyi kırar; unity build yüzünden anonim namespace yerine adlı namespace; başlıkta gereksiz include yok.
- **M24:** oyuncuya takvim yılı ve gerçek dünya tarihi gösterilmez ("N. yıl"). Koddaki `Kurus2011`, `StartYear` iç çapadır, adları değişmez.
- **Zevk ve akış** (`06_GIDIS_YOLU.md` §2b, Mustafa): her sistem oyuncuya bir sonraki dakikayı oynatacak bir şey vermeli: yakında bitecek hedef, görünür sonuç, kutlama, anlamlı takas, isimli rakip/karakter, ritim. Sistemin ürettiği hedef, haber ve kutlama cümlelerini işlev olarak sun (C menüye ve "Şimdi ne yapmalı"ya bağlar).
- **Tycoon ilkeleri** (`06_GIDIS_YOLU.md` §2): oyuncuya giden her metin bir cümle + bir sayı; ayrıntı ipucunda. Oyuncuya dönük metin sade Türkçe, teknik terim yok.
- **Bitti şartı:** kendi klasöründe `DERLE.cmd /q` geçti, `TEST.cmd /q` geçti (hepsi, eski testler dahil). Akış A oyun akışına dokunduysa `SmokeTest.ps1` de geçti. Derlenmeyen iş bitmiş sayılmaz.
- **Küçük adımlar:** her alt iş ayrı commit (Türkçe kısa ilk satır: `B2: muhasebe defteri ...`). Dal düzenli `git push -u origin akis-x` ile GitHub'a gider.
- **Limit yaklaşınca:** yarım işi `[yarım]` ile commit + push et, akış notunun başına "Kaldığım yer" yaz (dosya, adım, derleme durumu). Sonraki oturum oradan devam eder.

## 4. Akışların işi

### Akış A — Codex: doğrulama, otomatik oyuncu, zaman

**A0 · Aşama 0 (main üzerinde, ilk iş).** `DERLE.cmd /q`, `TEST.cmd /q`, `SmokeTest.ps1`. Derlenmemiş Claude işleri: G-086f, G-089 (M19–M23: `MarketManagers`, `MarketDepots`, menü), M24, `MarketStoreAssign`. **Bu adımda derleme ve test hatalarını hangi dosyada olursa olsun düzeltme izni var** (Mustafa); en küçük düzeltme, mantık değiştirmeden; her düzeltme GUNLUK'te dosya:satır ile. Mantık hatası görürsen düzeltme, not et. `Test.ps1`'deki alt sınırı güncel test sayısına çek. Smoke il seçme ekranında takılırsa smoke'u düzelt (akış A dosyası). Commit + push, sonra `akis-a` worktree'sini aç.

**A1 · Otomatik oyuncu ve denge raporu** (G-090). Yeni `MarketAutoPlay.h/.cpp` (`namespace MarketAutoPlay`), dünyasız:
- Yeni kampanyayı kodla kurar (`MarketStart`, ülke + il parametre), sonra her gün `MarketSimulation::PlayDay` ile oynar. Kararları yalnız oyuncunun da kullandığı yollarla verir: `MarketDirector::Command`, sipariş önerisi, mevcut menü komutlarının karşılıkları. Bekleyen kararlarda (hikâye, olay) tarza göre seçer.
- Üç tarz (`FProfile`): **temkinli** (borç almaz, kasa tamponu büyük, geç büyür), **dengeli**, **atak** (kredi alır, erken şube açar, fiyat kırar). Tarz parametreleri tabloda: kasa tamponu, şube açma eşiği, kredi isteği, fiyat stratejisi, müdür/depo kararları.
- Ölçer ve yazar: günlük/haftalık kasa, borç, net kâr, mağaza sayısı, il sayısı, ulusal pay, çalışan, iflas/nakit sıkıntısı günleri; eşiklere ulaşma günü (ilk şube, 5 mağaza, ilk depo, ilk il müdürü, 5 il, ikinci ülke); en çok para kaybettiren 5 kalem; **tutarlılık denetimleri** (para korunumu, negatif stok, NaN, sınır dışı değer); **istismar şüphesi** (bir tarzın kasası diğerlerinden katlarca hızlı büyüyorsa hangi kalemden geldiği).
- **Akış ölçümü (06 §2b):** "sıkıcı dönem" (30 günden uzun ne karar ne olay ne eşik) ve "felaket yığılması" (7 günde 3'ten çok kötü olay) sayılır ve raporda gösterilir.
- Çıktı: `Saved/AutoPlay/<tarih-saat>/rapor.md` (Türkçe, Mustafa okuyacak: özet tablo + bulgular + "neye bakmalı") ve `gunluk.csv`, `haftalik.csv`.
- Çalıştırma: **kısa** sürüm otomasyon testi `MirasMarket.AutoPlay.Short` (3 tarz × 1 tohum × 120 gün; çökme yok, korunum tutuyor; 60 sn altında) ve **uzun** sürüm commandlet (`-run=MirasAutoPlay -Years=10 -Seeds=3 -Country=tr -Province=kirklareli`) + `AUTOPLAY.cmd`. Hedef: 10 yıl × 3 tarz × 3 tohum 15 dakikayı geçmesin.
- Bot yalnız bugün var olan sistemleri kullanır; B ve C'nin yeni sistemleri birleştirmeden sonra otomatik olarak devreye girer. Yeni komutlar geldikçe tarz tablosuna eklenecek yerleri akış notuna yaz.

**A3 · Zaman ve tur.** `MarketSimulation`:
- #12: "1 hafta" tam 7 gün ilerletir (durmadıkça); yeni **1 ay** (takvim ayının sonuna kadar).
- **Durma nedenleri** tek bir yapıda (`EStop`: karar bekliyor, kasa eksi, hafta/ay raporu, hikâye bölümü, önemli olay — il müdürü kasadan çalıyor, rakip fiyat savaşı vb.) ve oyuncuya giden bir cümle.
- #8: ilerletilen günlerde aile dükkânını kimin yürüttüğü sonucu etkiler: müdür varsa onun becerisi (`MarketManagers::FamilyRule`), yoksa ailenin rutini, **kusursuz değil** (unutulan sipariş satırı, geç yansıtılan zam; beceri düştükçe artar). Oyuncu dükkândayken bu ceza yok.
- Hafta ve ay sonu raporu için veri (`FMarketState::History` üstünden haftalık/aylık özet işlevleri).
- Menüdeki "1 gün / 1 hafta / 1 ay" düğmeleri C'nin işi; A işlevleri ve `AMarketGameMode` tarafını hazırlar (Game Codex'in dosyası) ve akış notuna "C: menüye bağla" yazar.

**A · Teslim:** bot raporunun ilk tam koşusu (10 yıl) `Docs/Surec/akislar/A_ilk_rapor.md` olarak da kopyalanır.

### Akış B — Claude Code: denge ve şirket derinliği

Önce `02_DERIN_INCELEME.md`'de ilgili hata numaralarının tarifini, `05_YOL_HARITASI.md` §2'de güncel durumunu oku. Her maddeye test yaz.

**B1 · Açık denge hataları:**
- **#24** raftaki alma kararı fiyat esnekliğine bağlı olsun (`MarketDemand` `BuyChance`): ürünün `Elasticity` alanı (katalogda var) kullanılır; temel ürün az, keyif ürünü çok tepki verir.
- **#27** maliyet altı satış: fiyat maliyetin altına düşerken uyarı metni ve günlük raporda zarar satırı; toptancı destekli kampanya raporu gerçek desteği ve gerçek kârı göstersin (`MarketPromotions`).
- **#30** online siparişler raftaki kampanya fiyatını ve indirimi kullansın (`MarketOnline`).
- **#43** vergide zarar devri (zarar eden hafta sonraki kârlardan düşülür); son gün indirimi ve ipoteğin "ödül gibi" etkileri (`02`'deki tarif) (`MarketStaff`, `MarketFreshness`, `MarketFinance`).
- **#45** ulusal pay mağaza sayısıyla değil **ciroyla** (bizim ülkedeki toplam ciromuz / ülkenin gıda perakende büyüklüğü) (`MarketCompany`).
- **Kontrol et, rapor et (düzeltme C'nin dosyasındaysa yalnız not):** #21 menü ile simülasyonun rakip fiyatı aynı mı, #25 kampanya başka reyondan çalıyor mu, #31 "3 al 2 öde" müşteriyi 3'e zorluyor mu, #39 ücret/kıdem.

**B2 · Muhasebe defteri** (#37, #42): yeni `MarketLedger.*`. Her para hareketi bir kayıt: gün, mağaza (aile dükkânı = -1, şube indeksi, şirket merkezi = -2), hesap (satış, satılan malın maliyeti, fire, maaş, sigorta, kira, enerji, vergi, KDV, faiz, kredi anaparası, yatırım, depo, kira geliri…), tutar. `Post()`; gün/hafta/ay/yıl **gelir tablosu** ve **bilanço** (kasa, stok değeri, alacak, borç, özkaynak); **tutarlılık denetimi**: kasa değişimi = kayıtların toplamı. Kendi dosyalarındaki para hareketlerini deftere bağla. C'nin dosyalarındaki (şube, depo, tedarik) ve A'nın (simülasyon) hareketler için hangi satıra hangi `Post` çağrısının gideceğini akış notuna liste olarak yaz; bağlamayı C yapar. Eski kayıtta defter boş başlar.

**B3 · Ücret ve sigorta** (#39): ücret asgari ücretin altına düşmez; işveren sigorta payı ülke paketinden (`ulkeler.json` → `employerSocialRate`, TR ~%22,5 gibi oyun değeri); kıdem tazminatı (çalışma süresine göre, işten çıkarmada); müdür ücretleri de aynı kurallarla. Deftere yazılır.

**B4 · Dönem olayları** (karar 06 §5: sıra sabit, zamanı her kampanyada ±2 yıl kayar, sıklık ülkenin ekonomi karakterine bağlı): yeni `MarketEras.*`. Olaylar: kur şoku (ithal ürün maliyeti ve enflasyon sıçraması), yüksek enflasyon dönemi, durgunluk (sepet küçülür, ucuzcuya kayış), salgın (mevcut `MarketOnline` salgın profilini buradan tetikle, eskisini bozma), toparlanma. `ulkeler.json`'a `economy.character` (`stable` / `volatile` / `highInflation`). Olay adları gerçek değil ("kur şoku", "büyük salgın"), yıl gösterilmez. Haber cümleleri ve etkiler `Director`'daki kendi bloğundan.

**B5 · (Vakit kalırsa) Dünya ve son** (A7): kurgu kurlar (L08 önerisi: para birimi adı gerçek, kur kurgu, ülkeye göre istikrarlı/oynak; lig tek ortak birimde), yeni ülkeler için `ulkeler.json` iskeleti, oyun sonu (liderlik ölçütü: `MarketLeague` sıralamasını C yazar; B yalnız ortak birim çevirisini ve sonu bağlar) (J02 önerisi: "30. yılın sonu" ya da lig birinciliği; `MarketStory`). Vakit yoksa B5'e hiç başlama; akış notuna yaz.

### Akış C — Claude (Cowork): mağaza ağı, il pazarı, tedarik, markalar; en sonda bağlama

- **C1 (A4):** G-088 Aşama C ekonomisi: şube görünüm ataması ve kaydı (`MarketStoreAssign`), ölçülerden çarpanlar şube hesabına; G-089 sonrası düzeltmeler; #47 şube geri dönüş süresi.
- **C2 (A5):** yaşayan il pazarı (ilde rakip havuzu ülke paketinden, #19 kampanya adedi istismarı, #18 zincir bilançosu), tedarik ağı (G-083 katmanlı toptancılar, #41 gecikme faizi tavanı), **M25 markaların reyon yarışı** (`MarketBrands.*`: raf parası, ciro primi, ortak kampanya, reyondaki marka payları, özel marka).
- **C2b · Rakip kadrosu ve rakip aklı (Mustafa, 30.09.2026: "iyi bir rakip listesi, akılları iyi olsun; dünya ligi ne çok zor ne çok kolay"):** üç katman, hepsi kurgu adlı (L12), ülke paketinden:
  - **İl:** bakkallar, semt pazarı, fırın/kasap, yerel aile marketi (1–15 mağaza), bölgesel zincir (20–150 mağaza). İlin rekabet değerinden kurulur; büyüyen, zorlanan, satılığa çıkan (satın alınabilir) olur.
  - **Ülke:** her ülkede 6–8 ulusal zincir, her biri bir kişilik: indirim devi, hızlı açan ucuzcu, çok formatlı süpermarket, hipermarket, premium, hızlı teslimat, toptan. Kasası, borcu, büyüme hedefi var; yerel zincir satın alır, iflas edebilir, geç oyunda satın alınabilir.
  - **Dünya:** 10–12 uluslararası dev; ülkelere girer ve çıkar; oyuncu yurt dışına gidince oranın yerleşik oyuncularıdır. Dünya perakende ligi bunların cirosuyla kurulur (lig sıralaması C'de; B5 yalnız kur ve oyun sonu).
  - **Rakip aklı (hile yok):** her rakip bir hedef ve kişilikle, oyuncunun da gördüğü bilgiyle karar verir: boş ve kârlı illeri bulur, zayıf olduğun ilde saldırır, güçlü olduğun yerden kaçar ya da fiyat savaşı açar, gecikmeli ve kusurlu tepki verir, pahalı hatalar yapabilir, iyi giden hamlesini tekrarlar. Zorluk ayarı rakiplerin **karar kalitesini ve bilgisini** değiştirir, onlara bedava para vermez.
  - **Denge hedefi (A1 botuyla ölçülür):** dengeli bot ulusal ilk 3'e 10–15. yıl civarı, dünya ilk 10'a 20–25. yıl, birinciliğe 30. yıla yakın ulaşır (40–60 saat). Temkinli bot birinci olamaz; atak bot olabilir ama iflas riski taşır. Tutmazsa rakip büyüme ve karar parametreleri ayarlanır.
- **C3 · Bağlama:** A ve B dallarının birleştirilmesi (git işlemini Mustafa'nın isteğiyle Claude Code ya da Codex yapar), `FMarketState` blokları, `Director` sırası, defter çağrıları, menü sayfaları (zaman düğmeleri, defter/gelir tablosu, dönem olayları, markalar, bot raporu), belgeler; sonra Codex tam derleme + test + smoke + uzun bot koşusu.

### Sonraki işler (ilk işler bitince, aynı dal ve klasörde)

Kurallar aynı (§2–§3). İlk işler (A0–A3, B1–B5) bitip akış notu yazıldıktan sonra başlanır.

**A4 · Şube ziyareti (Codex, Aşama 2'nin çekirdeği).** Oyuncu haritadan bir şubesine girip birinci şahıs gezer; gördüğü her şey şubenin sayılarından gelir (06 simülasyon ilkesi: "aklın penceresi").
- `AMarketGameMode::StartBranchVisit(int32 BranchIndex)` / `EndBranchVisit()`: **test modu açılmaz**, aile dükkânının kaydı ve ekonomisi değişmez; ziyaret sırasında ekonomi saati durur (v1). Çıkınca oyuncu tam bıraktığı yere ve hâle döner.
- Mağaza: şubenin görünümü `FMarketBranch::StoreView` (`MarketStoreViews::Find` → `MarketStoreKit::Find`), yoksa türün ilk şablonu. Raf kategorileri `StoreCategoryOverrides` mantığıyla.
- Raflar şubenin gerçek doluluğuyla: her ürün `Items[i].Units / Capacity` oranında dolu (boş raf boş görünür). İçerideki müşteri sayısı `LastShoppers`'a, kasa kuyruğu `LastQueueLost`'a, çalışan sayısı `Workers`'a göre; kötü karnede biraz dağınıklık (kutular, boş palet) — yalnız görsel.
- Üstte küçük bilgi şeridi: şube adı, karne, müdür, "Esc: çık".
- Ziyaret başlarken `MarketDirector::Command(State, "VisitBranch", BranchIndex, Message)` çağrılır (ziyaretin kazandırdıklarını — gizli beceri, dürüstlük ipucu, moral — C yazacak; komut yoksa sessizce geçer).
- Menüdeki "Gez" düğmesini C bağlar; A4 `AMarketGameMode` işlevini ve bir otomasyon/görüntü kontrolünü (`BranchVisitReview`: ziyaret aç, 3 açıdan PNG, çık, kayıt aynı) hazırlar.

**A5 · Menü ekran görüntüsü otomasyonu (Codex).** `-MirasMenuCapture`: otomatik oyuncuyla 3. yıla kadar oynanmış bir kampanya (şubeler, müdürler, depo olsun) yüklenir; menünün her sayfası ve sekmesi, açık ve koyu temada, 1920×1080 ve 1280×720'de PNG olarak `Saved/Screenshots/Menu/<tarih>/` altına yazılır; bir `index.html` hepsini yan yana gösterir. Amaç: C ve Mustafa menüyü oynamadan görebilsin (A8 sadeleştirme turu için).

**B6 · Hedefler, kilometre taşları ve kutlamalar (Claude Code; 06 §2b "zevk ve akış").** Yeni `MarketGoals.*` (dünyadan bağımsız, testli):
- **Üç ölçekte hedef**, oyuncunun aşamasına göre üretilir ve hep en az biri yakında bitecek durumdadır: kısa (bu hafta: ciro rekoru, rafları %90 dolu tut, borcun taksiti), orta (bu ay: ikinci şube, il müdürü, ilk depo), uzun (bu yıl / bölüm: ülkede ilk 5, ilk yurt dışı). Her hedefte ilerleme yüzdesi, kalan süre ve tek cümle "neden önemli".
- **Kilometre taşları ve rekorlar:** ilkler (ilk şube, 10. mağaza, ilk il birinciliği…), rekorlar (en iyi gün/hafta/ay, en çok mağaza). Her biri bir **kutlama** kaydı üretir (başlık, bir cümle, önem); menü bunları kısa kutlama kartı olarak gösterecek (C bağlar). Küçük ve anlamlı ödüller: hatıra (`MarketStory::AddMemory`), ekibe moral, toptancı güveni; para ödülü yok ya da çok küçük.
- **Ritim koruyucusu:** 20 günden uzun süre ne olay ne karar ne kilometre taşı varsa `MarketEvents`'e olumlu ya da ilginç bir olay önerir; 7 günde 3'ten çok kötü olay yığılırsa yeni kötü olayı erteler. Ölçütler sabit değil, zorluğa bağlı.
- Metinler sade Türkçe, yılsız (M24). C'ye istekler: hedef şeridi (üst haplar), kutlama kartı, Raporlar'da rekorlar sekmesi, "Şimdi ne yapmalı"ya hedef satırı.

## 5. Akış notu biçimi (`Docs/Surec/akislar/A.md`, `B.md`)

```
# Akış B — Claude Code
## Kaldığım yer        (varsa; en üstte)
## Yapılanlar          (alt iş, commit, dosyalar)
## Doğrulama           (DERLE / TEST sonucu, test sayısı, tarih)
## Yeni açık işlevler  (imza + bir cümle; C'nin bağlayacakları)
## C'ye istekler       (Director / FMarketState / menü / C'nin dosyalarında değişiklik; mümkünse hazır kod parçası)
## Kararlar ve varsayımlar (Mustafa'ya sorulmadan verilen, sayılar dahil)
## Bilinen sorunlar
```

## 6. Bitiş

Akış bitince son commit + push, akış notu tam, `DERLE` ve `TEST` geçmiş. Mustafa, Claude'a (Cowork) "A/B bitti" der; C birleştirmeye başlar.
