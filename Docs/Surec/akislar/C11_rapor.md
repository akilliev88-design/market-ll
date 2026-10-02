# C11 raporu — merkez giderleri, ülke müdürü tuzağı, reel ücret

02.10.2026 · Claude (Cowork). Codex limiti doldu; derleme, test, smoke ve bot koşusu Claude tarafından bilgisayar üzerinden (`CLAUDE_KOS.cmd`) yapıldı.

## Sonuç tablosu (§5, 10 yıl × 3 tarz × tohum 21/22/23)

| Ölçü | Hedef | C10 D0 (önce) | C11 E0 (şimdi) |
|---|---|---|---|
| Dengeli ilk şube | 120–244. gün | 320 (0/3) | 306 / 334 / 334 (0/3) ✗ |
| Dengeli 3. yıl mağaza | 10 | 7 (0/3) | 9 / 9 / 9 (0/3) ✗ (yakın) |
| Dengeli 10. yıl mağaza | 60–120 | 1 (0/3) | 147 / 141 / 135 (0/3) ✗ (üstünde) |
| Dengeli 10. yıl ulusal sıra | ilk 10 | 34 (0/3) | 8 / 7 / 8 (3/3) ✓ |
| Temkinli ilk şube | 240–426. gün | 449 (1/3) | 399 / 459 / 459 (1/3) |
| Temkinli 10. yıl mağaza | 15–40 | 1 (0/3) | 119 / 107 / 107 (0/3) ✗ (çok üstünde) |
| Temkinli 10. yıl ulusal sıra | ilk 20 | 35 (0/3) | 8 / 8 / 8 (3/3) ✓ |
| Kurtarma (10 yıl, 9 koşu) | 0–1 | 7 | **0** ✓ |
| İlk dükkân reel kârı 10/1 (T/D/A ort.) | ≥ 1 | 0,49 / 0,41 / 0,46 | 0,62 / 0,91 / 0,64 (1/9 ✓) |
| Sıkıcı dönem | 0 | 0 | 0 ✓ |
| Para/stok/defter farkı | 0 | 0 | 0 ✓ |
| Atak 10. yıl mağaza / sıra | — | 98 / 8 | 176 / 156 / 134, sıra 7 |

Ham veri: `Saved/AutoPlay/C10/C11_E0_s0..s2` (bilgisayarda); küçük kanıt `C11_veri/E0_s*` (aile_yillik, kurtarma, c3, bot raporu). Özet aracı `Tools/c11_ozet.py`.

## Kök neden: ülke müdürü tuzağı

C10 verisinde temkinli ve dengeli oyuncu 3. yılda 7 mağazada takılıp 9–10. yılda tek dükkâna düşüyordu. Sebep: 5 ile yayılan 6 şubede bot (ve menüdeki "Sınırdasın: il müdürü ya da ülke müdürü düşün" ipucu) **ülke müdürü** atıyordu. Ülke müdürünün bandı günde 250 TL (2011 parası, asgari ücretin ~11 katı); işverenle yılda ~130 bin TL. Aynı dönemde ilk dükkânın yıllık kârı 40–60 bin, altı şubenin toplamı ~120 bin TL. Şirket FAVÖK'ü ilk dükkân kadar kalıyor, ücretler fiyatlardan hızlı arttıkça eksiye dönüyor, şubeler kapanıyordu. Atak ise hızla 20+ mağazaya çıkıp ölçek ekonomisine (depo, merkezi alım, büyük format) geçtiği için bu çukuru görmüyordu. "Ya hep ya hiç" görüntüsü bundandı.

## Yapılanlar (C11)

1. **M40 — ülke müdürünün maaşı ağın büyüklüğüne göre:** band × (ülkedeki mağaza / 30), en az %30, en çok %100 (`MarketManagers::CountryWageScale`, `ShopsInCountry`). Aday havuzu ve şubeden terfi bu ölçekle; şirket büyüdükçe maaş haftalık olarak banda yükselir, hiç düşmez. Menü yardım metni güncellendi.
2. **İK müdürü ve mali müşavir merkezin defterinde** (`MarketLedger::BeginClose`, `MarketStaff` SGK payı). Kasa değişmedi; yalnız defter ayrıldı. İlk dükkânın defteri artık yalnız kendi çalışanları.
3. **Kurtarma ve "bir aylık sabit gider" bütçesine merkezin kalan giderleri** (`MarketFinance::HeadOfficeDailyCost`: web, uygulama, karanlık depo, e-ticaret/reklam müdürü, reklam kanalları, depo kirası, kamyon, POS ve yemek kartı). Yeni yardımcılar: `MarketOnline::DailyFixedCost`, `MarketAdvertising::DailySpend`, `MarketCompany::DailyOfficeCost`, `MarketPayments::DailyFees`; kendi `CloseDay`'leri aynı hesabı kullanır.
4. **Asgari ücretin reel artışı yılda %0,5** (C10 D2; önce %1,5).
5. akis-a (C10 botu) main'e birleşti. Derlenmemiş tek iş kuralı kaldırıldı (Mustafa).

Testler: `Ledger.CashAudit` (müşavir merkezde, ücret toplamı korunur), `Finance.RescuePlan` (merkez giderleri bütçede), `Managers.CountryAfterFiveProvinces` (az mağazada %30, büyüyünce artış), `Managers.CountryManager` ve `Tuning.Knobs` güncellendi.

## Doğrulama

DERLE geçti (ilk denemede). TEST **157/157** (156 temiz + 1 motor HTTP uyarısı). Smoke PASSED (C11 ilk derlemesiyle). Bot: 9 kampanya × 3653 gün, 808 sn, üç tarz paralel, çıkış 0, denetim hatası 0.

## Kalan denge işleri (sıradaki)

- **Orta oyun fazla kolay:** 15+ mağazadan sonra şirket FAVÖK'ü milyonlara çıkıyor; temkinli bot 10. yılda 28–31 milyon TL kasa, sıfır borçla yine 107–119 mağazaya ulaşıyor. Büyüme artık paradan değil botun açılış sıklığından (temkinli 30, dengeli 14, atak 7 günde bir) sınırlanıyor. Bu yüzden temkinli ile dengeli arasında fark kalmadı. Seçenekler Mustafa'ya soruldu: büyük format/ölçek kârını kısmak (oyun) ya da temkinli botun açılış sıklığını yavaşlatmak (ölçüm aracı).
- **Dengeli ilk şube geç (306–334. gün):** hedef 4–8. ay. C10 D4 (açılış tamponu 1,0) yalnız 287'ye indirmişti; ilk şubenin maliyeti/izin süresi ya da ilk yılın kârı ayrı bakılmalı.
- **İlk dükkân reel kârı:** 1.–8. yıl yatay (dengeli ~60–70 bin TL, 2011 parası); 9–10. yılda dönem şokları (kur/durgunluk) ve ağ büyürken iki puanlık pay kaybıyla düşüyor. Ölçü 10. yılı şok yılına denk getiriyor; ayrıca bot ikinci şubeden sonra ilk dükkânın fiyatını yükseltiyor (marj %24 → %35, pay %35 → %25), bu da hacmi düşürüyor.
- `Docs/Surec/bekleyen/` (eski yamalar) C11 commit'ine izlenmeyen klasör olarak girdi; içerik değişmedi.
