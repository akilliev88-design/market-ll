Miras Market, **C5 (Codex): internetten satışı derle, botu büyüt, menü listesini çıkar.** Ana klasör: `C:\Users\mtass\Desktop\market-ll`.

Durum: C4'ün üstüne Claude Cowork **M32 internetten satışı** baştan yazdı ve klasöre koydu (derlenmedi). Aynı anda Claude ana klasörde başka bir iş yapacak (sokak rakipleri, Türkiye kalıntıları). Bu yüzden ilk iki adımdan sonra **ana klasöre dokunma**, kendi worktree'inde çalış. Önce oku: `Docs/Kurgu/01_KARARLAR.md` M32, `Source/MirasMarket/MarketOnline.h` (baştaki açıklama), `Docs/Surec/akislar/C.md` (M32 satırı).

## Yapacakların

1. **Al (ana klasörde):** `git status`. Claude'un M32 dosyaları değişmiş görünüyorsa (`MarketOnline.*`, `MarketEconomy.h`, `MarketMenuPages.cpp`, `MarketDirector.*`, `MarketBranches.cpp`, testler, `ulkeler.json`, belgeler; 30 dosya civarı) tek commit: `C5: M32 internetten satış (Claude, derlenmedi)`. Başka beklenmedik değişiklik varsa dur ve yaz.
2. **Derle ve test et (ana klasörde):** `DERLE.cmd /q`, `TEST.cmd /q` (alt sınır **140**), `SmokeTest.ps1`. Yeni/değişen testler: `Online.TimelineAndShare`, `Online.FamilyShopOnThePlatform`, `Online.CompanyAndProvinces`, `Online.RivalsAndCards`, `Balance.OnlineDeals`, `Eras.*`, `Company.*`, `Ledger.*`. Derleme/test hatasında en küçük düzeltme, mantık değişmez; her düzeltme `A.md`'ye dosya:satır. `Ledger.CashAudit` farkı **0** kalmalı. Geçince commit + push.
3. **Worktree:** `main`'den `akis-a` dalını güncelle (`git worktree` ile ayrı klasör). Bundan sonraki bütün işler orada; ana klasöre yazma. Yalnız A dosyaların: `MarketAutoPlay*`, `MarketMenuCapture.cpp`, `MarketReviewAutomation.cpp`, `MarketGame*` (gerekirse), `Docs/Surec/akislar/A.md`. Başka dosyada değişiklik gerekiyorsa yapma, `A.md`'ye "C'ye istek" yaz.
4. **Bot (A):**
   - **İnternet (M32):** eski online komutları (`OnlineChannel`, `HireCourier`, `FireCourier`, `FreeDelivery`, `PandemicProfile`) bottan kalksın. Yeni: `OnlineOpen` / `OnlineClose` (0 web, 1 uygulama, 2 platform, 3 hızlı), `DarkStore` (alan indeksi), `OnlineAds`, `OnlineFee`, `OnlineHire`, `OnlineAutoPolicy`. Kartlar: `online.app` (temkinli 0, dengeli 1, atak 2; kasa yetmiyorsa 3 = şimdilik yapma), `online.pandemic` (dengeli/atak 0, temkinli 2), `online.platformmarket` (0), `online.commission` (komisyon %24'ü geçecekse 2, yoksa 0). **İl önerisi kartı `online.area:<ülke>|<il>`:** zarar eden kanalı kapatma önerisini onayla (0); yeni kanal önerisini onayla; karanlık depo öneren karta (`Arg & 8`) kasa depo bedelinin 5 katından azsa ret (1). Alanlara bot elle dokunmasın.
   - **C4'te yapılmadıysa:** finans komutları, yedek kuralı (ağın aylık sabit gideri), olgun ve iki ay zararlı şubeyi kapatma, şube başına ilk 180 gün kâr dökümü (C4 promptundaki gibi).
5. **Koşular:** kısa bot testi, sonra 30 yıl × 3 tarz × 1 tohum ve 10 yıl × 3 tarz × 3 tohum. Rapor C4 başlıklarıyla, artı **internet bölümü**:
   - yıllara göre ülke internet payı ve bizim internet ciro payımız,
   - kanal başına yıllık sipariş, ciro ve kâr; sipariş başı kâr,
   - kanalların açılış günleri (salgına göre kaç gün önce/sonra), karanlık depo sayısı,
   - rakiplerin internete çıkış günleri, kartlar ve botun seçimleri, il önerileri (kaç öneri, kaçı onay/ret),
   - salgın yılında mağaza trafiği ve internet siparişi,
   - "internete hiç çıkmayan" ayrı bir deneme: temkinli tarz internetsiz oynasın; aradaki fark kaçırılan teknolojinin ne kadar tökezlettiğini gösterir (batırmamalı).
6. **Menü listesi (A8 hazırlığı):** `-MirasMenuCapture` ile üç anda görüntü al: başlangıç (internet sayfası yalnız ödeme kartı göstermeli), salgından 1 yıl önce (web ve platform açık), salgından 3 yıl sonra (uygulama, hızlı teslimat, iller listesi, politika, istatistik dolu). Sonra **bütün menü sayfaları için** `A.md`'ye "Menü sadeleştirme listesi" yaz: her sayfada (a) taşan / kesilen / boş görünen yerler, (b) oyuncuya bir şey söylemeyen sayılar ve düğmeler, (c) yeni başlayan biri için gereksiz erken görünen şeyler, (d) aynı bilginin iki yerde tekrarı. Dosya:satır ve görüntü adıyla. Menüye dokunma; listeyi Claude uygulayacak.
7. **Teslim:** alt adım başına commit (`C5 doğrulama: ...`) + `akis-a` push; `A.md`'ye kısa "C5" bölümü.

## Kurallar
Oyun sabitlerini değiştirme; öneri olarak yaz. Eski kayıt dalı yazma (M27). DERLE + TEST + Smoke geçmeden bitti deme. Limit yaklaşırsa `[yarım]` commit ve `A.md` başına "Kaldığım yer".

## Bitince bana kısa Türkçe özet
Derleme/test düzeltmeleri, testler kaç/kaç, defter farkı, dengeli botun 10./20./30. yıl sıraları, internet payımız (10./20. yıl), internetsiz denemenin farkı, kurtarma planı sayısı, en önemli 5 denge bulgusu, menü listesinden en önemli 5 madde.
