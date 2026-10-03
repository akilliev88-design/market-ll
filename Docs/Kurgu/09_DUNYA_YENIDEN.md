# Dünya yeniden tasarımı: bütün sistemin gözden geçirilmesi

03.10.2026 · Claude (Cowork). Taslak; Mustafa'nın onayını bekleyen kararlar §8'de.

## 1. Yön (Mustafa, 03.10.2026)

1. **Oyun dünya çapındadır.** Oyuncu oyundaki ülkelerin herhangi birinde başlayabilir ve hepsinde oynayabilir. Hiçbir sistem "Türkiye ve yurt dışı" diye düşünülmez; her kural her ülkede çalışır, Türkiye yalnız ülkelerden biridir.
2. **Dengeleme burada durur.** Büyük bir değişiklik yapıyoruz; bot hedefleri (06_GIDIS_YOLU §5) yeni yapı oturunca yeniden yazılır. C15b ve C16 (M48–M50) kodu bekletilir, yeni yapıya göre yeniden ele alınır.
3. **Tek ekonomi modeli.** İlk dükkân ile şubeler aynı günlük modeli kullanır (06_GIDIS_YOLU Aşama 1 "C10"). Yürünen dükkân bu modeli gösterir; oyuncunun elle işi küçük bir sapma ekler.
4. **Ülke sayısı yaklaşık 10** (25 fazla).
5. **Yönetim kademesi ülkenin üstüne çıkar:** ülke müdürleri bir üst kademeye bağlanır.

## 2. Bugün ne var (kod)

- **Ülke paketleri** (`MarketCountry`, `Config/ulkeler.json`): para birimi ve gösterim ölçeği, ekonomi karakteri (istikrarlı / oynak / yüksek enflasyon), tatiller, isim havuzları, iller ve bölgeler, bankalar, online platform, iklim, kurgu zincir adları. 4 ülke: Türkiye (81 il, harita), Almanya (16), Birleşik Krallık (12), ABD (50).
- **Başlangıç ülkesi seçilebilir** (`MarketStart`, karar L02): kampanya bir ülkeyle açılır, o ülke "etkin ülke" olur.
- **Kur ve dünya ligi:** her ülkenin dünya birimine kuru var (`FxRate`), dünya ligi ciroları dünya biriminde.
- **Yurt dışı şube:** 6. bölümde ("Sınır Ötesi") açılır; şube başka ülkenin ilinde açılabilir.

## 3. Asıl sorunlar

| # | Sorun | Nerede | Sonucu |
|---|---|---|---|
| S1 | **Tek fiyat eğrisi.** Fiyat düzeyi, ücret endeksi ve faiz yalnız etkin ülkeninki (`MarketPrices::ListLevel/WageIndex/LoanRate`, günü alır, ülkeyi almaz). | MarketPrices, MarketBranches, MarketStaff, MarketBanking | Almanya'daki şube Türkiye enflasyonuyla yaşar; kur riski gerçekte yok |
| S2 | **İki ekonomi.** İlk dükkân ayrıntılı müşteri modeliyle, şubeler toplu modelle hesaplanıyor. | MarketEconomy/MarketSimulation ↔ MarketBranches | Aynı koşulda farklı sonuç; dengeleme iki yerden |
| S3 | **Türkiye'ye özel kod dalları.** `Id == "tr"` 40'a yakın yerde: tatiller, okul takvimi, zincir kadrosu (BİM, A101, Migros…), isimler, ödeme (yemek kartı), kasaplık ve bayram, dönem olayları. | Calendar, Chains, Competitors, Departments, Eras, Managers, Payments, Staff, Start, menü | Başka ülkede başlayan oyuncu yarım bir oyun görür |
| S4 | **Hikâye ve bölümler Türkiye kurgusu.** Bölüm adları (Trakya, Türkiye, Sınır Ötesi), Nermin teyze, babadan kalan borç, bayram sahneleri. Akraba adları ülkeye göre değişiyor, gerisi değişmiyor. | MarketStory, MarketCampaign | Başlangıç ülkesi değişince hikâye uymuyor |
| S5 | **Yurt dışı çok geç ve tek kapıdan:** 6. bölüm, yalnız kendi mağazanı açarak (ve satılık bir kolu alarak). | MarketCompany, MarketBranches, MarketChains | 10 yılda hiçbir bot ikinci ülkeye geçmedi |
| S6 | **Yönetim ülkede bitiyor.** Kademeler: mağaza → il → alt bölge → bölge → ülke; oyuncu en çok 5 kişiyi doğrudan yönetir (`SpanLimit`). | MarketManagers | 10 ülkede 10 ülke müdürü oyuncuya bağlanamaz |
| S7 | **Hedefler, rapor, menü ülke odaklı:** "ulusal sıra", "ulusal pay", il haritası ana ekran; dünya ligi arka planda. | MarketGoals, MarketMenu*, MarketMap, bot raporu | Oyuncu oyunu ülke içi bir yarış sanıyor |

## 4. Sistem sistem gözden geçirme

| Sistem | Bugün | Dünya için ne değişmeli | Büyüklük |
|---|---|---|---|
| **Ekonomi çekirdeği** (fiyat, ücret, faiz, enflasyon) | Etkin ülkenin tek eğrisi | Her ülkenin kendi fiyat düzeyi, ücret endeksi, faizi ve dönem olayları; fonksiyonlar `(Ülke, Gün)` alır. Şirketin defteri **ana para biriminde** (başlangıç ülkesi); yabancı şube kendi parasıyla yaşar, deftere kurla girer, kur farkı ayrı satır | Büyük |
| **Mağaza günü** | İki model | Tek model (§5) | Büyük |
| **Ülke paketleri** | 4 ülke, Türkiye dalı kodda | 10 ülke; Türkiye'ye özel ne varsa pakete taşınır (tatil, okul takvimi, zincir kadrosu, ödeme alışkanlıkları, kasaplık/bayram). Kodda `"tr"` kalmaz | Orta + veri işi |
| **Rakip zincirler** | Türkiye kadrosu kodda; diğerlerinde arketip adı değişiyor; dünya devleri | Her ülkenin kadrosu pakette (yerel liderler + o ülkedeki dünya devleri kolları); dünya devleri birden çok ülkede | Orta |
| **Yönetim** | Ülkede biter | Ülke müdürü → **kıta direktörü** → **genel müdür** (CEO, isteğe bağlı) → oyuncu (§6). Kıta direktörü kıtanın ülke müdürlerini denetler; genel müdür kıta direktörlerini | Orta |
| **Yurt dışına çıkış** | 6. bölüm, kendi mağaza | Bölüm kilidi kalkar; ülkeye giriş yolları: kendi mağaza, zincir satın alma, ortaklık/franchise, ihracat/online. Her yolun maliyeti ve riski farklı. Girişin şartı: ana ülkede bir ölçek (ör. 25 mağaza) ve bir ülke müdürü adayı | Büyük |
| **Kur riski** | Ligde var, şubede yok | Yabancı şubenin kârı kurla eriyebilir/büyüyebilir; döviz kredisi seçeneği; kur şoku o ülkenin dönem olayı | Orta |
| **Banka** | Bankalar ülke paketinde, kredi şirkete | Kredi hangi ülkenin bankasından alınırsa o paranın faizi ve kur riski; not şirketin tamamına | Orta |
| **Tedarik, depo, kamyon** | Ülke içi; depo 600 km | Depo ülke içinde kalır (sınır ötesi tedarik yok, ya da gümrükle pahalı); merkezi alım ülke başına; dünya ölçeğinde "küresel alım" bir yatırım olabilir | Küçük–orta |
| **Markalar ve ürünler** | Tek katalog, Türkiye fiyatları | Tek katalog kalır (iç birim); ülke fiyat düzeyi ve tat farkı (talep grubu çarpanları) pakette; marka adları ülkeye göre | Orta |
| **Online, reklam, ödeme** | Ülke paketinden kısmen | Dönemleri (telefon 2011, web 2014, platform 2016) ülkeye göre; yemek kartı yalnız olan ülkelerde | Küçük |
| **Personel ve vergi** | SGK oranı, kıdem tazminatı pakette | Asgari ücret ve vergi takvimi ülkeye göre (zaten kısmen) | Küçük |
| **Takvim ve hava** | Türkiye tatilleri kodda | Hepsi pakette; okul takvimi pakette | Küçük |
| **Hikâye ve bölümler** | Türkiye kurgusu | Bölümler ülkeden bağımsız: Defter → Karşı Dükkân → İkinci Tabela → İller → Ülke Çapında → **Dünyaya Açılış** → Miras. Sahneler ülkeye göre metin (akraba, komşu, toptancı adı pakette); bayram yerine ülkenin bayramı | Orta (metin işi büyük) |
| **Hedefler, rapor** | Ulusal sıra/pay | Dünya ligi baştan görünür; hedefler ülke içi + dünya; bot raporu ülke sayısı ve dünya sırası | Orta |
| **Menü ve harita** | Türkiye il haritası | Önce **dünya haritası** (10 ülke, bizim olduklarımız işaretli), tıklayınca ülkenin il haritası; Rakipler sayfası ülke seçmeli | Büyük (Codex/3B tarafıyla) |
| **Bot** | Tek ülke | Yurt dışına çıkma kararı; hedefler: X. yılda ilk ülke, 30. yılda Y ülke, dünya sırası | Orta |
| **Kayıt** | Sürüm 8–9 | Sürüm artar, eski kayıt yüklenmez (M27) | — |

> D3/D5 (Claude Code, 03.10.2026): Türkiye dalları ülke paketine taşındı, 6 yeni ülke eklendi (10 ülke). Yeni paket alanları `10_ULKE_STANDARDI.md` §3; teslim `Docs/Surec/akislar/D3_D5_cc_teslim.md`.

## 5. Tek ekonomi modeli (S2)

- **Bir mağazanın günü tek fonksiyon:** `MarketStoreDay::Run(State, Store, Products, Day)`. Bütün mağazalar için aynı hesap: il müşteri potansiyeli × pay (çekim / çekim + rekabet) × olgunlaşma × yamyamlık × raf doluluğu × hizmet × fiyat → satış, stok, fire, ücret, kira, giderler.
- **İlk dükkân "mağaza 0" olur;** ayrı hikâye bayrakları (aile dükkânı, ilk müdür) kalır ama ekonomisi şubeyle aynı.
- **Yürünen dükkân:** oyuncu içerideyken müşteri sayısı ve sepetler modelin o günkü sonucundan dağıtılır. Oyuncunun yaptığı (raf dizme, fiyat etiketi, kasada durmak) modele küçük bir çarpan olarak girer (doluluk, hizmet, fiyat). Gün sonu sayıları modelle aynı.
- **Stratejik ilerletme** (`MarketSimulation`) ayrı bir oyun değil, yalnız oyuncunun yürümediği günleri hızlı geçirir; zaten aynı modeli çağırır.
- **Doğrulama:** aynı koşulda ilk dükkân ile bir şube aynı ay sonucunu verir (test); para/stok/defter denetimi 0.

## 6. Yönetim kademesi (S6)

```
Oyuncu (patron)
 └─ Genel müdür (CEO) — isteğe bağlı; tutulursa kıta direktörleri ona bağlanır
     └─ Kıta direktörü (Avrupa, Amerika, Asya-Pasifik …)
         └─ Ülke müdürü
             └─ Bölge direktörü → bölge müdürü → il müdürü → mağaza müdürü
```

- Her kişi en çok 5 kişiyi doğrudan yönetir (`SpanLimit` aynı).
- Kıta direktörü ikinci ülkeden itibaren gerekir ya da 3 ülkeden sonra (karar §8).
- Ücretler ağın büyüklüğüne göre ölçeklenir (M40'taki ülke müdürü kuralı gibi).
- Genel müdür oyuncunun yerine rutin kararları verir (otomatik mod).

## 7. Önerilen 10 ülke

Çeşitlilik ölçütleri: farklı kıtalar, farklı ekonomi karakteri (istikrarlı / oynak / yüksek enflasyon), farklı pazar yapısı (indirim marketi ağırlıklı / hipermarket ağırlıklı / geleneksel ticaret), oyuncu kitlesi.

| Ülke | Kıta | Ekonomi | Pazarın karakteri | Durum |
|---|---|---|---|---|
| Türkiye | Avrupa/Asya | yüksek enflasyon | indirim marketleri, bakkal | var |
| Almanya | Avrupa | istikrarlı | indirim marketi devleri | var |
| Birleşik Krallık | Avrupa | istikrarlı | büyük süpermarket zincirleri | var |
| ABD | Amerika | istikrarlı | hipermarket ve dev zincirler | var |
| Fransa | Avrupa | istikrarlı | hipermarketin doğduğu yer | yeni |
| İspanya | Avrupa | oynak | bölgesel zincirler | yeni |
| Polonya | Avrupa | oynak, hızlı büyüyen | indirim marketleri büyüyor | yeni |
| Brezilya | Amerika | yüksek enflasyon | atacarejo (toptan perakende) | yeni |
| Meksika | Amerika | oynak | geleneksel ticaret güçlü | yeni |
| Japonya | Asya | istikrarlı, deflasyon | kombini ve küçük format | yeni |

Kıtalar: Avrupa 6, Amerika 3, Asya 1. Alternatif olarak Japonya yerine Hindistan ya da Güney Kore konabilir.

## 8. Kararlar (Mustafa, 03.10.2026)

1. **Ülkeler:** §7'deki 10 ülke (Türkiye, Almanya, Birleşik Krallık, ABD, Fransa, İspanya, Polonya, Brezilya, Meksika, Japonya).
2. **Yurt dışı kapısı:** bölüm kilidi kalkar; ölçek şartı (ana ülkede ~25 mağaza ve bir ülke müdürü adayı). Dengeli oyuncu ~5–7. yılda çıkar.
3. **Kıta direktörü:** üç ülkeden sonra zorunlu; ilk iki ülkenin ülke müdürleri doğrudan oyuncuya bağlanır. Genel müdür her zaman isteğe bağlı.
4. **Hikâye:** evrensel hikâye; bölümler ülkeden bağımsız, sahneler tek kurgu, isimler/bayramlar/akraba/komşu/toptancı ülke paketinden.
5. **Hedefler:** açık (D8'de; öneri: ilk yurt dışı 5–7. yıl, 30. yılda 6–8 ülke, dünyada ilk 10).

## 8b. Önceki soru listesi

1. **Ülke listesi:** §7'deki 10 ülke uygun mu, değişen olsun mu?
2. **Yurt dışı ne zaman açılsın:** bölüm kilidi tamamen kalksın mı, yoksa bir ölçek şartı mı olsun (ör. ana ülkede 25 mağaza)?
3. **Kıta direktörü:** ikinci ülkeden itibaren mi, üç ülkeden sonra mı gereksin?
4. **Hikâye:** bölümler ülkeden bağımsız hale gelsin, sahne metinleri her ülke için ayrı mı yazılsın, yoksa tek bir "evrensel" hikâye ve ülke adları mı değişsin?
5. **Hedefler (dengeli oyuncu):** ilk yurt dışı adımı kaçıncı yılda, 30. yılda kaç ülke, dünyada kaçıncı?

## 9. Sıra ve iş bölümü (öneri)

| Faz | İş | Kim |
|---|---|---|
| D1 | Ülke başına ekonomi (S1): fiyat/ücret/faiz `(Ülke, Gün)`, ana para birimi defteri, kur farkı | Claude |
| D2 | Tek mağaza ekonomisi (S2, §5) ve prototip kalıntılarının temizliği (§10, K1–K6) | Claude |
| D3 | Türkiye dallarını pakete taşıma (S3), ülke paketi şemasını genişletme | Claude Code (yerel arama ve taşıma işi, testlerle) |
| D4 | Yönetim kademesi: kıta direktörü, genel müdür (S6) — **bitti, M66 (60f3129)** | Claude |
| D5 | 6 yeni ülke paketi verisi (iller, bölgeler, tatiller, isimler, zincir kadroları, ekonomi) | Claude Code (veri) + Claude (kontrol) |
| D6 | Yurt dışına giriş yolları ve bölüm yapısı (S4, S5) — **bitti, M67 (60f3129)**: ölçek kapısı, ortaklık, 6. bölüm "Dünyaya Açılış" | Claude |
| D7 | Dünya haritası ve menü (S7); dünya ve ülke tablolarına liste dışı başlangıç (M53: listedekileri geçince gireriz) — **ilk parça bitti (60f3129)**: ana ekranda "Dünya" çipi ve kartı. **Kalanı yazıldı (derlenmedi)**: kartın üstünde şematik dünya haritası (kıta alanları, ülke noktaları, renk = durumumuz) ve M53 | Codex / Claude Code (arayüz), Claude (metin) |
| D8 | Bot ve hedefler yeniden; dengeleme yeniden başlar — **ilk tur yazıldı (derlenmedi)**: M56 son yok, bot ortaklık ve yurt dışı kilometre taşları, M55 süre tablosu; 30 yıllık bot koşusu is.cmd içinde | Claude |
| D9 | Orta oyun (C16, Mustafa 03.10.2026): M46 mağaza portföyü (karne, yenile, tür değiştir, taşı, eskime) ve M47 krizlere ve rakip hamlelerine cevap — Claude Code'a verilecek; M48–M50 (il pazarı ve il atağı, yol ayrımları, yollar) — Claude'da yazılı, bekliyor. D1 (ülke başına dönem olayları) ve D2 (tek mağaza modeli) bitince yeni modele bağlanır; yol ayrımlarına "yurt dışı yolu", yollara "dünya markası" eklenir | Claude Code + Claude |

D1 ve D2 birbirine bağlı ve çekirdekte; D3 ile D5 bağımsız yürüyebilir. Bekleyen C16 kodu (M48–M50) D2'den sonra yeni modele bağlanır.

## 10. Prototipten kalanlar (Mustafa 03.10.2026: "oyunun ilk kalıntıları duruyorsa kaldıralım, hâlâ bir yerlerde bir şeyleri etkiliyor olabilir")

Kod taramasında bulunanlar. Hepsi D2 (tek ekonomi modeli) içinde kaldırılır ya da yeni modele katılır; her biri için "önce/sonra" testi.

| # | Kalıntı | Bugün ne yapıyor | Ne olacak |
|---|---|---|---|
| K1 | **`MarketRivals`** (prototip BİM, Migros, A101 günlük haberleri) | İlk dükkânın müşteri trafiğini ve reyon fiyatlarını hâlâ çarpıyor (`MarketDirector::TrafficFactor`, `RivalPriceFactor`); oyun ekranında haber satırları | Kalkar. Rakip etkisi yalnız zincir modelinden (`MarketChains`) gelir |
| K2 | **`MarketCompetitors`** ilçe pay modeli (Bereket Market, BİM, Migros, A101, Şok; ilk dükkânın yerel payı) | İlk dükkânın payı ve trafiği bu modelden, şubelerinki `MarketChains`'ten: iki rekabet modeli | Tek rekabet modeli: mağazanın payı ildeki zincirlerden ve geleneksel ticaretten. Bereket Market (karşı dükkân) hikâyenin yerel rakibi olarak zincir modelinde küçük bir yerel zincir olur; öfkesi, fiyat savaşı ve personel ayartma oraya taşınır |
| K3 | **İlk dükkânın ayrı ekonomisi** (`MarketEconomy` + `MarketSimulation` müşteri modeli) ↔ şube günü (`MarketBranches::CloseDay`) | Aynı koşulda farklı sonuç | Tek mağaza günü (§5) |
| K4 | **Eski kayıt uyumu**: `MarketManagers::Migrate` (her gün çağrılıyor), `MarketStaff` "Staff boş = v0.1 bayrakları", depoların alt bölge göçü, "older save" dalları, `bSecondStore` | M27 eski kayıt uyumunu kaldırdı ama kod duruyor; `Migrate` her gün çalışıp gizli değerleri tohumluyor | Kalkar; değerler kişi/şube yaratılırken verilir. `bSecondStore` yerine şube sayısı |
| K5 | **v0.1 "ikinci şube" kuralları** (`MarketCampaign::ExpandCash/ExpandProfitableDays/ExpandShare`, `EExpandBlock`) | İlk şubenin şartı hâlâ bunlardan | Evrensel hikâyenin 2. bölüm hedefleri olarak kalır ama paraya bağlı şart (ExpandCash) kalkar; açılış maliyetini zaten `CanExpand` ölçüyor |
| K6 | **`BranchResult` defter hesabı** ("v0.1 second store") | Tek bir yerde kullanılıyor | Kalkar |
| K7 | **(Y1: eğri pakete taşındı)** **Yerleşik Türkiye ekonomi eğrisi** (`MarketPrices::Rates[]`, `StartWage`) ve Türkiye dalları (~45) | Türkiye paketin dışında özel | Pakete (D1, D3) |
| K8 | **(Y1: adlar "Start" oldu)** **"2011" adları** (`Kurus2011`, `CatalogBase`) | Yalnız ad (M24) | Ad olarak kalabilir; yeni kodda "başlangıç düzeyi" denir |
