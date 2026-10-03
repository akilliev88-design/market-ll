# D3 + D5 teslim notu — Claude Code (bulut), dal `akis-cc`

Tarih: 03.10.2026. Durum: **derlenmedi, test edilmedi** (bulutta Unreal yok). Derleme ve test Cowork tarafından Mustafa'nın bilgisayarında yapılacak.

> Not: Görev belgesi ve `10_ULKE_STANDARDI.md` ilk teslimde GitHub'da yoktu; iş Mustafa'nın özetiyle yapıldı. Belgeler gelince (akis-cc2, birleşim temizliği) şu eksikler kapatıldı: her pakete `continent`; yeni ülkelerde 42 ad / 42 soyad, DE/GB/US müdür havuzları 40+; Brezilya "yüksek enflasyon" (0,08 ± 0,04), İspanya "oynak" (0,022 ± 0,02); yeni ülkelere `groceryPerPersonDay`; PackStandard'a ≥8 il, ≥40 ad/soyad, kıta ve yeni ülkelerin beklenen il/bölge/tatil/zincir sayıları; `Test.ps1` alt sınırı 164; `FxRate` yabancı ülkede E1 enflasyonu. Görev belgesindeki "dokunma" listesinden `MarketEras.cpp`'ye (varsayılan ülke, 1 satır) dokunuldu.

## D3 — Türkiye dalları pakete

| Eski kod dalı | Şimdi |
|---|---|
| `MarketCalendar::Info` — `bTurkey`: 6 resmî gün, Ramazan/Kurban bayramı, Ramazan ayı, okul açılışı/karne, bayram adı boş | Paket tatilleri (yeni: `table`, `fastDays`), `calendar.school`, `calendar.showHolidayNames`. TR paketine 1 Mayıs, 19 Mayıs, 30 Ağustos eklendi. Yeni `MarketCalendar::HolidayStart` |
| `MarketDepartments` — kasap zirvesi "tr ise Kurban öncesi" + yazısı | Tatilde `butcherPeak: true` (TR: Kurban Bayramı); yazı tatilin adından |
| `MarketPayments::CardShare` — "tr ise yıllık eğri" | `habits.cardShareByYear`; yoksa `cardShare` + dünya eğilimi |
| `MarketCountry::ApplyEconomy` — "tr ise yerleşik fiyat eğrisi" | `economy.curve: "builtin"` |
| `MarketCompetitors::DisplayName` — "tr ise gerçek ad" | `realChainNames: true` |
| `MarketChains::NationalRoster` — 28 satır C++ (TR/DE/GB/US) | Her paketin `roster` dizisi; `GiantRoster` kök `giants` dizisi |
| `MarketChains` — bölge zincir eki " Market/Gross/Süper/Gıda" (her ülkede Türkçe) | `regionalSuffixes` (DE/GB/US'ye yerel sözcükler verildi) |
| `MarketStaff` — "tr ise yerleşik isim listesi" | `names.staff` (TR'de eski liste birebir) |
| `MarketManagers::NamePool` — TR/DE/GB+US yerleşik listeler | `names.managers` (her ülkede eski birleşik liste, aynı sırayla) |
| `MarketCast::FirmWord` — de / gb,us / Türkçe | `firmWords.wholesale`, `firmWords.cashCarry` |
| `MarketMenuPages` — harita yalnız "tr" (21 yer) | `MarketCountry::MapCountry()` (il dosyası `iller.json` olan paket), `HasMap()` |
| `MarketStart`, `MarketEras`, menü — boş ülke → "tr" | `MarketCountry::DefaultId()` / `Default()` / `FindOrDefault()`; kök `defaultCountry` |

Kalan ülke kodları (izinli, "varsayılan"): `FMarketState::CountryId = "tr"`, `FProfile::Id = "tr"`, `DefaultIdRef` ilk değeri, `MarketAutoPlay.h` bot varsayılanı (`tr`/`kirklareli`), `MarketReviewAutomation.cpp` ve `MarketMenuCapture.cpp` (Codex otomasyon sahneleri; dokunulmadı).

Davranış: Türkiye'de aynı kalmalı. Etkin ülke artık oyun başlamadan önce de TR paketi (eskiden çıplak varsayılan yapı); testlerdeki "Türkiye'ye dön" satırları `SetActive(DefaultId())` oldu. Ülke paketi dosyası hiç yoksa yerleşik eğrili çıplak yapı kullanılır (tatil yok).

## D5 — altı yeni ülke

`fr` Fransa (EUR, 13 bölge), `es` İspanya (EUR, 17), `pl` Polonya (PLN, 16), `br` Brezilya (BRL, 27), `mx` Meksika (MXN, 32), `jp` Japonya (JPY, 47). Her pakette: para, ekonomi karakteri, alışkanlık, geleneksel ticaret ve pazar günü, sokak zinciri adları, 8–9 kurgu ulusal zincir (kurgu patronlar), tatiller (Paskalya'ya bağlı günler, Carnaval, Día de Muertos, Obon/Shogatsu…), okul günleri, 4 kurgu banka, teslimat platformu, iklim, isim ve müdür havuzları, akrabalar, bölge/alt bölge/il.

### Mağaza türleri (akis-cc2 adım 2 ile kalıcı)

İlk teslimde geçici eşleme vardı (kombini → `regional`, atacarejo → `wholesale`). Adım 2'de iki yeni arketip eklendi ve paketler onlara geçti:

| Gerçek tür | Örnek | Arketip | Mağaza biçimi |
|---|---|---|---|
| Kombini / yakın market | JP Seven & Me, FamilyMarto, Lawsun; MX OXO; PL Żabik; FR Petit Casinot | `convenience` (yeni; ciro 25.000/gün, ağırlık 0,5, brüt %28) | `kucuk` |
| Atacarejo (toptan perakende) | BR Atacadeo, Asaí | `cashCarry` (yeni; ciro 700.000/gün, ağırlık 6, brüt %12, en az 200 bin nüfus) | `hiper` |
| Bodega (MX indirim) | Bodegón Aurora | `discount` | `kucuk` |
| Gyomu süper | JP Gyomu Supa | `discount` | `kucuk` |

Dünya devi Seven & Me de `convenience`. Oyuncunun açabileceği mağaza türleri (kucuk/mahalle/buyuk/hiper) değişmedi; yakın market ve toptan perakendeyi oyuncu türü olarak eklemek ayrı bir karar.

### Bilinen etkiler

- Dünya devleri: CarreFive'ın evi `fr`, Aeonn ve Seven & Me'nin evi `jp` oldu (kendi ülkelerinde ulusal kolları var).
- Yeni ülkeler `MarketCountry::All()`'a girdiği için uzun koşularda (8. yıldan sonra) devlerin "yeni ülkeye kapı" seçimi ve bot yönetici atama döngüsü daha çok ülke görür; Türkiye kampanyasının 8. yıl sonrası lig sonuçları D5 öncesinden farklı olabilir. Bu D3 değil D5 etkisidir.
- `Test.ps1` alt sınırı 164 (E1 ile 161 + 3).

## Yeni testler

- `MirasMarket.Country.PackStandard` — dosya hatasız ayrışır; her pakette zorunlu alanlar (ad, para, ekonomi, bakkal/pazar, 4 sokak zinciri, ≥3 ulusal zincir ve geçerli arketip, dünya çapında tekil zincir kimliği, bölge sözcükleri, firma sözcükleri, 4 banka, platform, ≥6 isim, iklim 12/12, ≥3 tatil ve hepsinin tarihi var, ≥3 il, her il bir alt bölgede, her alt bölge bir ana bölgede); devlerin ev paketi; 6 yeni paketin varlığı.
- `MirasMarket.Country.SmokeEveryPack` — her pakette aile dükkânı kurulur, 30 gün `PlayDay`; denetim hatası 0, defter farkı her gün 0, negatif stok yok, para kendi biriminde.
- `MirasMarket.Country.TurkeyFromPack` — TR'nin eski kod dallarının paketten aynı sonucu verdiği: 6 resmî gün, Ramazan, arife, bayram uzunlukları, bayram adı boş, okul günleri, kart eğrisi, kasap zirvesi ×1,8, kadro sırası, harita ülkesi.
- Değişen: `Country.PacksCurrencyEconomy` (TR satırına `curve: builtin`), geri dönüşler `SetActive(DefaultId())` (`Country.*`, `Eras.*`, `Staff.WagesAndSocialSecurity`).

## Paket şemasına eklenen alanlar (10_ULKE_STANDARDI.md §3'e eklenecek)

Kök:
- `defaultCountry` (metin) — varsayılan ülke.
- `giants` (dizi) — `id`, `name`, `home` (menü adı), `pack` (ev paketi, boş olabilir), `archetype`, `revenueB`, `growth`.

Ülke:
- `economy.curve` — `"builtin"` = oyunun el yapımı fiyat eğrisi; yoksa üretilen eğri.
- `habits.cardShareByYear` — başlangıç yılından itibaren yıllık kartlı ödeme payı.
- `calendar.showHolidayNames` (varsayılan true), `calendar.school.start|end` — `{month, weekday (0 = Pzt), n (5 = son)}`.
- `holidays[].table` — ay takvimi tablosu (`ramazan` | `kurban`), `rule: lunar` için zorunlu.
- `holidays[].fastDays` — bayramdan önceki oruç günleri (Ramazan etiketi).
- `holidays[].butcherPeak` — kasabın büyük haftası bu bayramdan önceki hafta.
- `realChainNames` — marka anahtarı kurgu demedikçe gerçek zincir adları (TR).
- `roster[]` — `id`, `name`, `boss`, `archetype` (discount, fastDiscount, super, hyper, premium, regional, family, wholesale, club), `stores`, `price`, `service`, `aggression`, `ambition`, `region` (isteğe bağlı ana bölge).
- `regionalSuffixes` — bölge zinciri adının sözcükleri.
- `firmWords.wholesale`, `firmWords.cashCarry` — toptancı/cash & carry firma sözcüğü.
- `names.staff.first|last`, `names.managers.first|last` — isteğe bağlı ayrı havuzlar (yoksa `names.first|last`).

## Dokunulan dosyalar

`Config/ulkeler.json`; `MarketCountry.h/.cpp`, `MarketCalendar.h/.cpp`, `MarketChains.h/.cpp`, `MarketStaff.h/.cpp`, `MarketCast.cpp`, `MarketCompetitors.cpp`, `MarketDepartments.cpp`, `MarketEras.cpp`, `MarketManagers.cpp`, `MarketMenuPages.cpp` (yalnız harita satırlarında `"tr"` → `MapCountry()`/`DefaultId()`), `MarketPayments.cpp`, `MarketStart.cpp`; testler `MarketCountryTests.cpp`, `MarketErasTests.cpp`, `MarketStaffTests.cpp`, yeni `MarketCountryPackTests.cpp`; belgeler `01_KARARLAR.md` (işaretli blok D3D5), `DURUM.md`, `GUNLUK.md`, `GOREVLER.md`, bu not.

Cowork'un dosyalarıyla çakışma olabilecek yerler: `MarketMenuPages.cpp` (harita satırları) ve belgeler. Görev belgesindeki "dokunulmayacak dosyalar" listesi bu oturumda okunamadı.

## Derlemede dikkat

- `MarketCalendar.h` ve `MarketStaff.h` `MarketCountry` yapılarını ileri bildirimle tanır.
- `FRosterChain`/`FRosterGiant` alanları `const TCHAR*` yerine `FString` oldu.
- Etkin ülke ilk çağrıda `MarketCountry::Default()` ile yüklenir (`ulkeler.json` okunur).

## Yeni ülkelerin özeti (benzediği gerçek yapı; pakette gerçek ad yok)

| Ülke | İl | Ekonomi | Zincir kadrosu (kurgu) | Benzediği gerçek yapı |
|---|---|---|---|---|
| Fransa | 13 bölge (2016 sonrası metropol bölgeleri; departman 96 fazla) | istikrarlı | CarreFive, Leclair, Auchamp (hiper); Mousquetaires Marché, Système Ü (süper); Lidel, Alda; Petit Casinot (yakın market); Monoprice (premium) | Carrefour, E.Leclerc, Auchan, Intermarché, Système U, Lidl, Aldi, Petit Casino, Monoprix |
| İspanya | 17 özerk bölge | oynak | Mercadeo (lider süper), Díaz (indirim), CarreFive, Alcampa (hiper), Lidel, Eroskin ve Consumo (bölgesel), El Cortejo (premium) | Mercadona, Dia, Carrefour, Alcampo, Lidl, Eroski, Consum, El Corte Inglés |
| Polonya | 16 voyvodalık | oynak | Biedronia (indirim lideri), Żabik (yakın market), Lidel, Dinoz (bölgesel), Kauffeld/Auchamp (hiper), Stokrotek, Lewiatanek (aile marketleri birliği) | Biedronka, Żabka, Lidl, Dino, Kaufland, Auchan, Stokrotka, Lewiatan |
| Brezilya | 27 eyalet | yüksek enflasyon | Atacadeo, Asaí (atacarejo), CarreFive (hiper), Extrá, Pão de Mel (premium), Mateux, Zaffira (bölgesel), Díaz | Atacadão, Assaí, Carrefour, Extra, Pão de Açúcar, Mateus, Zaffari, Dia |
| Meksika | 32 eyalet | oynak | Wallmark, Soriano (hiper), Bodegón Aurora, Tiendas 3E (indirim), OXO (yakın market), Chedrahui, HEV (bölgesel), La Comerciante (premium), Costko | Walmart/Bodega Aurrera, Soriana, Tiendas 3B, OXXO, Chedraui, H-E-B, La Comer, Costco |
| Japonya | 47 prefektör | istikrarlı, deflasyona yakın | Aeonn (süper), Seven & Me, FamilyMarto, Lawsun (kombini), Ito-Yokodo (hiper), Lifu, Gyomu Supa, Triall | Aeon, 7-Eleven, FamilyMart, Lawson, Ito-Yokado, Life, Gyōmu Super, Trial |
