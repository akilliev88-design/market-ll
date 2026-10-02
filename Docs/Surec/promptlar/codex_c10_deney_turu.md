Miras Market, **C10 (Codex): deney turu — tek değişkenli denge denemeleri, gider dağılımı ve kurtarma düzeltmesi.** Ana klasör: `C:\Users\mtass\Desktop\market-ll`.

Önce oku: `Docs/Kurgu/06_GIDIS_YOLU.md` §5 (onaylı hedef eğriler, M39), kendi `C9_cekirdek_denge_rapor.md` (R1–R5), `Docs/Surec/akislar/C.md` (C10 satırı), `Source/MirasMarket/MarketTuning.h`.

Claude'un C10 oyun tarafı (derlenmedi):
- **R1 gider dağılımı:** şube tadilatı ve işe alımı o şubenin, müdür primi/tazminatı, internet kurulumu/uygulama ve reklam/e-ticaret müdürü tazminatı merkezin defterine (`MarketLedger::AddStoreCost`, `PendingStoreCosts`). İlk dükkânın defteri artık yalnız kendi gideri. Nakit akışı değişmedi.
- **R4 kurtarma:** şube kalmayınca depolar kapanır, kamyonlar bugünkü fiyatın %40'ına satılır, depo müdürü de ayrılır; plan kasaya **3 aylık** kalan gider (ilk dükkân + merkez + asgari patron maaşı) ve dolu raf parası koyar.
- **Deney düğmeleri** (`MarketTuning`): `BranchCompetition` (şube çekiminde rekabet katsayısı, kodda 3,0), `RealWageGrowth` (asgari ücretin yıllık reel artışı, 0,015), `RealSpend` (şube müşterisinin sepetinin reel ücretle büyüme esnekliği, kodda 0 = kapalı). Oyun bunları hiç değiştirmez; yalnız bot/komutlet. Yeni test `Tuning.Knobs`.

1. **Al:** `akis-a`'yı (C9 bot) `main`e birleştir, push dene. Claude'un dosyaları tek commit `C10: gider dağılımı, kurtarma 3 ay, deney düğmeleri (Claude, derlenmedi)`. Derle, test (alt sınır **154**), Smoke. Sonra `akis-a`.
2. **Komutlet (A):** `-Tune=Anahtar=Değer,...` → `MarketTuning::Apply` (koşu başında; rapora `MarketTuning::Describe()`). Bot sabitleri için de aynı biçimde A tarafında düğme: `OpenBuffer.Balanced`, `OpenBuffer.Careful`, `LossMonthsToClose`.
3. **Deneyler:** her biri 10 yıl × 3 tarz × tohum 21/22/23; **yalnız bir düğme** değişir, geri kalan C10 varsayılanı:
   - D0 taban (C10 kaynağı, düğmesiz)
   - D1 `BranchCompetition=2.5`
   - D2 `RealWageGrowth=0.005`
   - D3 `RealSpend=0.7`
   - D4 bot `OpenBuffer.Balanced=1.0` (ve ayrı D4b `OpenBuffer.Careful=2.0`)
   - D5 bot `LossMonthsToClose=4`
   Her deney için §5 tablosu (✓/✗) ve tek satır özet: ilk şube günü ortalaması, 3./10. yıl mağaza, 10. yıl ulusal sıra, kurtarma, ilk dükkânın (artık temiz) yıllık faaliyet kârının enflasyondan arındırılmış 10. yıl / 1. yıl oranı, şube başına 30 günlük kâr (olgun).
4. **Birleşim:** D0'a göre hedefe en çok yaklaştıran 2–3 düğmeyi birlikte koş (10 yıl × 3 × 3, sonra 30 yıl × 3 × 21). Öneri: hangi düğme hangi değerle koda yazılmalı (Claude yazar).
5. **Menü:** yok (yalnız C9'un kalan iki görüntüsü değişmediyse not).
6. **Teslim:** commit + push; `A.md`'ye "C10"; rapor `C10_deney_raporu.md` (önce sonuç tablosu: deney × hedef satırları).

Kurallar: oyun sabitlerini kodda değiştirme (düğme kullan), eski kayıt dalı yazma (M27), DERLE + TEST + Smoke geçmeden bitti deme.

Bitince kısa Türkçe özet: düzeltmeler, testler, deney tablosu (D0–D5, birleşim), önerilen düğme değerleri, ilk dükkân kârı (temiz defterle), kurtarma.
