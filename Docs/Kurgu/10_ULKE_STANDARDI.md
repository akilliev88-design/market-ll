# Ülke standardı: motor ülke bilmez

03.10.2026 · Claude (Cowork), Mustafa'nın isteğiyle: "Yaptığımız şeyleri standart yap ki sonradan ülke eklemek istediğimizde sorun olmasın; ülkelere özgü şeyleri değil, oyunun arkasındaki aklı standartlaştır." Karar M52.

## 1. İlke

1. **Motor ülke bilmez.** Oyunun aklı (ekonomi, mağaza günü, rakipler, yönetim, banka, tedarik, hikâye akışı) her ülkede aynı kurallarla çalışır. Kodda ülke kodu (`"tr"`, `"de"`…) ya da ülkeye özel dal yoktur; tek istisna, paket bulunamazsa hangi paketin varsayılan sayılacağıdır.
2. **Ülkeye özgü her şey pakettedir** (`Config/ulkeler.json` + il dosyası). Bir sayı, bir ad, bir tarih, bir alışkanlık bir ülkeye aitse pakette durur. Motor yalnız paketin alanlarını okur.
3. **Yeni ülke = yeni paket.** Kod değişmez. Paket doğrulama testi ve duman testi geçince ülke oyundadır.
4. **Türkiye ülkelerden biridir.** Bugüne kadar "yerleşik" olan Türkiye eğrileri ve listeleri de pakete taşınır; Türkiye paketi diğerleriyle aynı şemayı kullanır.
5. **Ülkeden bağımsız kavramlar standarttır:** il, alt bölge, bölge, ülke, kıta; iç para birimi ve gösterim; zincir arketipleri; mağaza türleri; yönetim kademeleri; dönem olayı türleri. Bunlar motorda tanımlanır, paket yalnız değer verir.

## 2. Motorun standart kavramları

| Kavram | Motor | Paketin verdiği |
|---|---|---|
| Yer | il (birinci düzey idari birim) → alt bölge → bölge → ülke → kıta | il listesi (id, ad, nüfus, gelir, kira, rekabet, harita), alt bölge ve bölge üyeliği, ülkenin kıtası |
| Para | tek iç birim (kuruş ölçeği); şirket defteri ana ülkenin parasında; ülkeler arası kur | para birimi kodu/simgesi/ondalık işareti, gösterim ölçeği, başlangıç kuru |
| Ekonomi | ülke başına fiyat düzeyi, ücret endeksi, faiz; dönem olayları (kur şoku, durgunluk, salgın, yüksek enflasyon, toparlanma) ülke başına | ekonomi karakteri, yıllık enflasyon (ortalama, oynaklık ya da yıl yıl eğri), faiz farkı, ücret düzeyi, SGK/kıdem, vergi |
| Takvim | gün, hafta, ay; maaş günü; bayram öncesi/arifesi; okul açılışı | tatiller (sabit, Paskalya'ya göre, n. hafta günü, ay takvimi), okul açılış/kapanış, pazar kapalı mı |
| Müşteri | segmentler, sepet, fiyat duyarlılığı | alışkanlıklar (haftalık alışveriş payı, kart payı, yemek kartı vb.), iklim, talep grubu çarpanları |
| Rakip | zincir arketipleri: indirim, hızlı indirim, süpermarket, hipermarket, bölgesel, toptan; eklenecek: **yakın market (kombini)**, **toptan perakende (atacarejo/cash&carry)**; geleneksel ticaret | ulusal zincir kadrosu (kurgu ad, arketip, büyüklük, patron, bölge), geleneksel ticaret adları, ülkede kolu olan dünya devleri |
| Yönetim | mağaza → il → alt bölge → bölge → ülke → kıta → genel müdür; 5 kişi kuralı | isim havuzları, ücret düzeyi |
| Banka | not, kredi türleri, başvuru | 4 banka adı, faiz farkları |
| Online | dönemler (telefon, web, platform), kurye | platform adı, online payının tavanı, dönemlerin yılı |
| Hikâye | evrensel bölümler ve sahneler | akraba, komşu, toptancı, mahalle adları; bayram adı |

## 3. Paket şeması (özet)

Zorunlu: `id`, `name`, `nameEn`, `continent`, `currency` (code, symbol, decimal, symbolBefore), `displayScale`, `fxPerWorld`, `economy`, `habits`, `holidays`, `chains`, `banks`, `names`, `relatives`, `regions`, `subregions`, `provinces` (liste ya da dosya adı). (M61b: `referencePopK` ve `referenceProvince` kalktı; illerin gelir, kira ve rekabeti ülkenin ortanca iline göre hesaplanır, hiçbir il ayrıcalıklı değil.)
İsteğe bağlı (varsayılanı motorda): `traditional`, `online`, `climate`, `school`, `payments`, `feasts`, `kmPerMapUnit`.

D3 ile eklenen alanlar (Claude Code, 03.10.2026; hepsi isteğe bağlı, yoksa motor varsayılanı):

| Alan | Anlamı | Yoksa |
|---|---|---|
| kök `defaultCountry` | paket bulunamazsa ya da kayıtta ülke yoksa kullanılan ülke | `tr` |
| kök `giants[]` | dünya devleri: `id`, `name`, `home` (menüdeki ülke adı), `pack` (ev paketi, boş olabilir), `archetype`, `revenueB`, `growth` | dev yok |
| `continent` | kıta kimliği (`avrupa`, `amerika`, `asya`…); kıta direktörü (D4) | zorunlu |
| `economy.curve` | `"builtin"` = oyunun el yapımı fiyat eğrisi (Türkiye); yoksa ortalama ± oynaklıktan üretilen eğri | üretilen eğri |
| `economy.groceryPerPersonDay` | kişi başı günlük market harcaması (başlangıç düzeyinde, para birimi) | 0,65 × `wageFactor` |
| `habits.cardShareByYear[]` | başlangıç yılından itibaren yıllık kartlı ödeme payı | `cardShare` + dünya eğilimi |
| `calendar.showHolidayNames` | gün yazısında tatilin kendi adı ("Weihnachten") | `true` |
| `calendar.school.start` / `.end` | okul açılışı / karne günü: `{month, weekday (0 = Pzt), n (5 = son)}` | yok |
| `holidays[].table` | ay takvimi tablosu (`ramazan` \| `kurban`); `rule: lunar` için zorunlu | — |
| `holidays[].fastDays` | bayramdan önceki oruç günleri (Ramazan etiketi) | 0 |
| `holidays[].butcherPeak` | kasabın büyük haftası bu bayramdan önceki hafta | Noel haftası |
| `realChainNames` | marka anahtarı kurgu demedikçe menüde gerçek zincir adları | `false` |
| `roster[]` | ulusal zincirler: `id`, `name`, `boss`, `archetype`, `stores`, `price`, `service`, `aggression`, `ambition`, `region` (isteğe bağlı ana bölge) | zincir yok |
| `regionalSuffixes[]` | bölge zinciri adının sözcükleri ("Market", "Gross"…) | varsayılan ülkeninki |
| `localSuffixes[]` | yerel aile zinciri adının sözcükleri ("Gıda", "Kardeşler") | varsayılan ülkeninki |
| `firmWords.wholesale` / `.cashCarry` | toptancı firma sözcükleri ("Gıda Dağıtım", "Toptan") | varsayılan ülkeninki |
| `names.staff.first/last`, `names.managers.first/last` | personel ve müdür için ayrı isim havuzları | `names.first/last` |
| `playerFormats[]` | M54: oyuncunun dört temel türe ek açabileceği türler (`yakin` yakın market, `toptan` toptan perakende) | yok (yalnız dört tür) |
| `company.legalForms[]`, `company.setupCost`, `company.dividendWithholding` | M65: şirket türleri (tescilli adın sonu, ilki varsayılan), şirket kuruluş masrafı (para birimi, başlangıç düzeyi), yurt dışına kâr aktarımı stopajı (0–0,5) | varsayılan ülkeninki |

Zincir arketipleri (`roster[].archetype`, `giants[].archetype`): `discount`, `fastDiscount`, `super`, `hyper`, `premium`, `regional`, `family`, `wholesale` (toptan market), `club`, `convenience` (yakın market, kombini; mağaza biçimi küçük), `cashCarry` (toptan perakende, atacarejo; biçimi hiper).

Alanların tam listesi ve varsayılanları `MarketCountry::FProfile` başlık dosyasındaki yorumlardır; şema değişirse `ulkeler.json`'un `note` alanı ve bu belge birlikte güncellenir.

## 4. Doğrulama standardı (her pakete otomatik)

- **Paket doğrulama testi** (`MirasMarket.Country.PackStandard`): her paket için zorunlu alanlar dolu; en az 8 il ve hepsinin nüfusu > 0; her il bir alt bölgede, her alt bölge bir bölgede; tatiller çözülüyor; zincirler bilinen arketiplerde; en az 40 ad ve 40 soyad; 4 banka; para gösterimi çalışıyor.
- **Duman testi** (`MirasMarket.Country.SmokeEveryPack`): her ülkede kampanya başlar, 30 gün ilerler; çökme yok, para/stok/defter farkı 0.
- Bot koşusu (dengeleme döneminde): her ülkede başlangıç için kısa koşu; büyük ülke farkları raporlanır.

## 5. Yeni ülke ekleme adımları

1. `ulkeler.json`'a paketi ekle (şablon: mevcut bir paket), illeri ayrı dosyaya koy.
2. Kurgu zincir ve banka adları (F8: gerçek marka yok).
3. `TEST.cmd` → paket doğrulama ve duman testleri geçmeli.
4. Kısa bot koşusu ile ekonomi sayılarını kontrol et.
5. GUNLUK'e yaz. Kod değişikliği gerekiyorsa bu standarda bir eksik var demektir: önce standardı genişlet, sonra paketi.

## 6. Kodda kalmaması gerekenler (denetim listesi)

- `== TEXT("tr")`, `bTurkey`, `bHome` (ülke anlamında) — D3 ile kalkar.
- `MarketPrices`'taki yerleşik Türkiye eğrisi (`Rates[]`, `StartWage`) — Türkiye paketine taşınır (D1).
- Türkiye zincir kadrosu (`MarketChains::NationalRoster`) — pakete (D3).
- Ülkeye özel metinler (bayram, kasaplık, yemek kartı) — pakete ya da evrensel metne.
