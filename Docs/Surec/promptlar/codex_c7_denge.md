Miras Market, **C7 (Codex): denge düzeltmelerini ve menü sadeleştirmeyi derle, kurtarma sarmalını ölç.** Ana klasör: `C:\Users\mtass\Desktop\market-ll`. C6 bittikten sonra başla.

Durum: C6 sen `akis-a`'da çalışırken Claude ana klasöre **C7** yazdı (16 dosya, derlenmedi). Önce oku: `Docs/Surec/akislar/C.md` (C7 satırları), `MarketFinance.h` (baştaki M31/C7 açıklaması).

1. **Al:** `akis-a`'yı (C6) `main`e birleştir ve push et. Ana klasörde `git status`: Claude'un C7 dosyaları tek commit `C7: denge ve menü sadeleştirme (Claude, derlenmedi)`. Çakışmada A dosyaları senin, C dosyaları Claude'un.
2. **Derle ve test et:** `DERLE.cmd /q`, `TEST.cmd /q` (alt sınır **146** + C6'da eklediklerin), `SmokeTest.ps1`. Yeni/değişen: `Finance.RescuePlan`, `Finance.RescueOnePlan`, `Online.CompanyAndProvinces`, `Brands.*`, `SaveVersion.*` (sürüm 4). En küçük düzeltme, `A.md`'ye dosya:satır. `Ledger.CashAudit` 0. Sonra `akis-a`'ya geç.
3. **Bot (A):** kurtarma planı sürerken (`State.RescueUntil`) bot şube açmaya, kredi çekmeye çalışmasın (reddedilen komut sayısı şişmesin). Plan bitince yedek kuralıyla yeniden büyüyebilir.
4. **Koşular:** 30 yıl × 3 tarz × 1 tohum ve 10 yıl × 3 × 3 (C5 ile aynı tohumlar). Rapor C5 başlıklarıyla, öne:
   - **kurtarma sayısı, günleri, silinen borç, plan kredisinin bitip bitmediği; 10/20/30. yıl toplam borç ve aile dükkânı yıllık cirosu** (C5: 321 plan, 461–529 milyon borç, ciro ~0),
   - kurtarmadan sonra ilk şubeye kaç günde dönüldüğü,
   - il önerileri (aynı ilde tekrar sayısı; C5'te 33),
   - hiper balık/ev/kırtasiye/bebek 30 günlük şube başı kâr,
   - dengeli botun 10./20./30. yıl sırası.
5. **Menü görüntüleri:** C5'in üç dönemi yeniden (`-MirasMenuPhase=start/before/after`, tohum 22). Sadeleştirme listesinden neyin düzeldiğini, neyin kaldığını `A.md`'ye yaz (madde başına düzeldi/kaldı + görüntü adı).
6. **Teslim:** alt adım başına commit (`C7 doğrulama: ...`) + push; `A.md`'ye kısa "C7".

Kurallar: oyun sabitlerini değiştirme (öneri yaz), eski kayıt dalı yazma (M27), DERLE + TEST + Smoke geçmeden bitti deme.

Bitince kısa Türkçe özet: düzeltmeler, testler, defter farkı, kurtarma sayısı (C5'e göre), 30. yıl borç, dengeli sıralar, menü listesinden kalan en önemli 5 madde, en önemli 5 denge bulgusu.
