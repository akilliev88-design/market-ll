Miras Market, **C6 (Codex): komuta zinciri, reklam ve sokak rakiplerini derle, bota öğret.** Ana klasör: `C:\Users\mtass\Desktop\market-ll`. C5 bittikten sonra başla.

Durum: C5 sen `akis-a`'da çalışırken Claude ana klasöre **M33 komuta zinciri, M34 reklam, M35 Türkiye kalıntıları** yazdı (derlenmedi). Önce oku: `Docs/Kurgu/01_KARARLAR.md` M33–M35, `MarketCommand.h`, `MarketAdvertising.h`, `MarketCompetitors.h` (Bind), `Docs/Surec/akislar/C.md`.

1. **Al:** `akis-a`'yı (C5) `main`e birleştir ve push et. Sonra ana klasörde `git status`: Claude'un dosyaları (35 civarı; yeni `MarketAdvertising*`, `MarketCommand*`, `MarketStreetTests.cpp`) tek commit `C6: M33-M35 (Claude, derlenmedi)`. **`Source/MirasMarket/MarketRetail.h` ve `MarketRetail.cpp`'yi `git rm` ile sil** (artık kullanılmıyor; Claude cihazdan silemiyor). Çakışma olursa C5 bot dosyaların öncelikli, Claude'un dosyalarında Claude'un hâli.
2. **Derle ve test et:** `DERLE.cmd /q`, `TEST.cmd /q` (alt sınır **143** + C5'te eklediğin testler), `SmokeTest.ps1`. Yeni testler: `Advertising.ChannelsAndManager`, `Command.ClearanceAndProposals`, `Competitors.StreetFromProvinceChains`. En küçük düzeltme, mantık değişmez, `A.md`'ye dosya:satır. `Ledger.CashAudit` 0.
3. **Bot (akis-a, yalnız A dosyaları):**
   - Reklam: `AdLevel` (`MarketAdvertising::Encode`), `AdHire`, `AdAuto`, `AdBudget`. Kural: temkinli reklam yok; dengeli 10 mağazadan sosyal medya + broşür seviye 1, internet açıksa arama 1; atak 30 mağazadan televizyon 1 + radyo 1 + sosyal 2. 20 mağazada reklam müdürü al, karışımı ona bırak (dengeli binde 20, atak binde 30). `OnlineAds` artık arama kanalı.
   - Komuta kartları: `command.close:<şube>` → zarar 3 aydan uzunsa onayla (0); `command.open:<ülke>|<il>` → yedek kuralı tutuyorsa onayla, tutmuyorsa reddet (1).
   - Rapor: reklam harcaması ve tahmini getirisi (yıllık, kanal başına), il müdürü önerileri (açma/kapama, onay/ret), şube stok eritme sayısı, sokak rakiplerinin adları ve kapananlar.
4. **Koşular:** kısa bot testi, sonra 30 yıl × 3 tarz × 1 tohum ve 10 yıl × 3 × 3. Ayrıca **Almanya'dan başlayan** bir 10 yıllık dengeli koşu (sokak rakipleri, iklim, adlar Türkçe kalmamalı; raporda sokaktaki zincir adlarını ve ilk yılın hava günlerini yaz).
5. **Menü görüntüleri:** Şirket > Reklam kartı, Satış kanalları (internet reklamı satırı), bir şubenin haftalık satırında stok eritme. Taşan/boş yerleri "C'ye istek".
6. **Teslim:** alt adım başına commit (`C6 doğrulama: ...`) + push; `A.md`'ye kısa "C6".

Kurallar: oyun sabitlerini değiştirme (öneri yaz), eski kayıt dalı yazma (M27), DERLE + TEST + Smoke geçmeden bitti deme.

Bitince kısa Türkçe özet: düzeltmeler, testler, defter farkı, dengeli botun 10./20./30. yıl sıraları, reklamın getirisi (harcanan / getirdiği), önerilerin sayısı, Almanya koşusunda Türkçe kalan bir şey var mı, en önemli 5 denge bulgusu.
