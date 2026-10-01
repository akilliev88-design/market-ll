Miras Market, **C4 doğrulaması (Codex)**: şirket finansı ve kurtarma planı. Bu iş `main` üzerinde, ana klasörde (`C:\Users\mtass\Desktop\market-ll`).

Durum: senin C3 doğrulamanın (1fa3806) üstüne Claude Cowork şunları yazdı, **derlenmedi**: M28 şirket finansı (`MarketBanking`), M29 rakibe teklif + satın alma kredisi, M30 bağlı şirket + devlerin çıkışı + ülkeye göre adlar (`MarketCast`), M31 bankanın kurtarma planı (`MarketFinance::Rescue`), C3 raporundaki istekler (marka parası yalnız dolu rafa, finans kısa tutar, harita katman tepsisi, müdür satırı, kutlama satırı, şube açarken aylık sabit gider). Dosyalar klasörde, commit edilmemiş olabilir. Önce oku: `Docs/Kurgu/01_KARARLAR.md` M28–M31, `Docs/Surec/akislar/C.md` (Kaldığım yer + "C3 doğrulamasından sonra").

## Yapacakların

1. **Al:** `git status`. Claude'un dosyaları değişmiş görünüyorsa (46 dosya civarı) tek commit yap: `C4: M28-M31 ve C3 istekleri (Claude, derlenmedi)`. Başka beklenmedik değişiklik varsa dur ve yaz. `Docs/Surec/bekleyen/` içindeki eski yamaları **uygulama** (hepsi zaten içeride); klasörü silebilirsin.
2. **Derle ve test et:** `DERLE.cmd /q`, `TEST.cmd /q` (alt sınır **133**), `SmokeTest.ps1`. Yeni testler: `Banking.*`, `Chains.TakeoverBid`, `Chains.SubsidiaryAndExit`, `Cast.NamesFromTheCountry`, `Finance.RescuePlan`, `Brands.EmptyShelfEarnsNothing`.
3. **Hata düzelt:** en küçük düzeltme, mantığı değiştirme; her düzeltme `A.md`'ye dosya:satır + bir cümle. `Ledger.CashAudit` farkı yine **0** olmalı; fark çıkarsa hangi sistemden geldiğini yaz (M28 bankacılık, M30 `OwnedTurn`, M31 kurtarma kredisi en olası yerler).
4. **Bot (A):**
   - Yeni komutları kullan: `CorpLoan` (arg: `MarketBanking::EncodeLoan`), `OpenLine`, `LineAuto`, `RepayLine`, `Restructure`, `BuyChainFinanced`, `BidChainFinanced`, `ConvertStores` (arg: `MarketChains::EncodeConvert`), `SellSubsidiary`. Tarz başına basit kurallar yeter: dengeli/atak not B olunca limit açar ve `LineAuto` yapar; yatırım kredisini yalnız ağın aylık sabit giderinin 3 katı yedek kalacaksa çeker; satılık/çıkış satışındaki zinciri finansmanla alır; bağlı şirketi ayda bir çevirir.
   - **Yedek kuralı:** yeni şube yalnız açılıştan sonra kasada **bütün ağın** bir aylık sabit gideri kalacaksa (`MarketBranches::MonthlyFixedCost` toplamı + aile dükkânı) açılır.
   - **Zararlı şubeyi kapat:** olgun (90 günden eski) ve `Last30Profit` iki ay üst üste eksi olan şube kapanır (`CloseBranch`).
   - Kurtarma planını (M31) bot tetiklememeye çalışsın ama geldiyse oyuna devam etsin.
5. **Koşular:** kısa bot testi, sonra 30 yıl × 3 tarz × 1 tohum ve 10 yıl × 3 tarz × 3 tohum. Rapor C3 başlıklarıyla, artı:
   - **şube başına ilk 180 günün kâr dökümü** (ciro, brüt kâr, kira, ücret, SGK, işletme, lojistik, fire, net; mahalle/süper ayrı) — denge kararı bunun üstüne verilecek,
   - kredi notu yıllık seyri, şirket borcu / FAVÖK, ödenen faiz, limit kullanımı, borç sınırı ihlali ay sayısı,
   - kurtarma planı sayısı ve günleri, kapanan şube sayısı,
   - teklif / kabul / ret, finansmanla alım, bağlı şirket sayısı ve mağazası, çevrilen mağaza, devlerin çıkışı ve "kapı" kolları,
   - marka geliri (artık boş raf ödenmiyor) ve dengeli botun 10./20./30. yıl ulusal/dünya sırası.
6. **Menü görüntüleri:** `-MirasMenuCapture` ile Finans (banka kartı, gelir tablosu kısa tutarlar), Şirket (SATIN ALMA VE BAĞLI ŞİRKETLER), harita katman tepsisi, Mağazalar müdür satırı, ana ekran hedefler/kutlama. Taşan/boş yerleri `A.md`'de "C'ye istek" olarak listele.
7. **Teslim:** alt adım başına commit (`C4 doğrulama: ...`) + push; `A.md`'de kısa "C4 doğrulaması" bölümü.

## Kurallar
Oyun sabitlerini değiştirme; öneri olarak yaz. Eski kayıt dalı yazma (M27). DERLE + TEST + Smoke geçmeden bitti deme. Limit yaklaşırsa `[yarım]` commit ve `A.md` başına "Kaldığım yer".

## Bitince bana kısa Türkçe özet
Derleme/test düzeltmeleri (dosya başına), testler kaç/kaç, defter farkı, dengeli botun 10./20./30. yıl sıraları, kurtarma planı sayısı, mahalle şubesinin ilk 180 gün net kârı (ortalama), en önemli 5 denge bulgusu, menüde en önemli 3 sorun.
