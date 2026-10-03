# D3 + D5 teslim notu — Claude Code (bulut), dal `akis-cc`

Tarih: 03.10.2026. Durum: **derlenmedi, test edilmedi** (bulutta Unreal yok). Derleme ve test Cowork tarafından Mustafa'nın bilgisayarında yapılacak.

> Not: Görev belgesi `Docs/Surec/akislar/D3_D5_claude_code.md`, `09_DUNYA_YENIDEN.md`, `10_ULKE_STANDARDI.md` ve AGENTS.md §5 (M52) bu oturum boyunca GitHub'da yoktu (son commit C15b, 324384d). İş, Mustafa'nın sohbetteki özetine göre yapıldı: "ülkeye özgü her şey pakete; kodda ülke kodu yalnız paket yoksa varsayılan seçiminde". Belgeler gelince karşılaştırılmalı.

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

### Mağaza türü eşlemeleri (Cowork'un ekleyeceği türler)

| Gerçek tür | Örnek | Eşlenen arketip | Yakın mağaza biçimi |
|---|---|---|---|
| Kombini / yakın market | JP Seven & Me, FamilyMarto, Lawsun; MX OXO; PL Żabik; FR Petit Casinot | `regional` (bölge kısıtı yok) | `kucuk` |
| Atacarejo (perakende toptan) | BR Atacadeo, Asaí | `wholesale` | `hiper` |
| Bodega (MX indirim) | Bodegón Aurora | `discount` | `kucuk` |
| Gyomu süper (iş süpermarketi) | JP Gyomu Supa | `discount` | `kucuk` |

### Bilinen etkiler

- Dünya devleri: CarreFive'ın evi `fr`, Aeonn ve Seven & Me'nin evi `jp` oldu (kendi ülkelerinde ulusal kolları var).
- Yeni ülkeler `MarketCountry::All()`'a girdiği için uzun koşularda (8. yıldan sonra) devlerin "yeni ülkeye kapı" seçimi ve bot yönetici atama döngüsü daha çok ülke görür; Türkiye kampanyasının 8. yıl sonrası lig sonuçları D5 öncesinden farklı olabilir. Bu D3 değil D5 etkisidir.
- `Test.ps1` alt sınırı 160'ta bırakıldı (3 yeni test var; Cowork'un testleriyle birlikte yükseltilmeli).

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
