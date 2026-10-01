Miras Market, **Akış A — üçüncü tur (A6)**. Aynı dal (`akis-a`) ve aynı klasörde (`C:\Users\mtass\Desktop\market-ll-A`) devam ediyorsun. Bu, birleştirmeden (C3) önceki son işin.

Başlamadan: A4 ve A5 bitmiş, `Docs/Surec/akislar/A.md` güncel, son commit push edilmiş olmalı. Değilse önce onları bitir. Sonra şunları yeniden oku (değişti): `Docs/Kurgu/07_AKIL_ISBOLUMU.md` §3 (eski kayıt kuralı değişti, karar M27) ve §4 "Üçüncü tur" altındaki **A6**; `Docs/Kurgu/01_KARARLAR.md` M25–M27; `Docs/Surec/akislar/C.md` (C'nin yazdığı her şey, derlenmemiş).

## Neden
C'nin yeni sistemleri (rakip zincirleri ve dünya ligi, tedarik ağı, markalar, reyonlar) hiç derlenmedi ve hiç oynanmadı. Senin botun bunları oynayıp ölçmezse birleştirmede denge körlemesine kalır. Bu tur C'nin kodunu gerçek hâle getiriyor.

## Yapacakların
1. **C'nin işini al ve derle.** `main` klasöründe (`C:\Users\mtass\Desktop\market-ll`) `git status`; yalnız `Source/`, `Docs/`, `Config/` altındaki değişiklikleri `Akış C: C2b, C2c, M25, M26 ve sonrası (derlenmedi)` mesajıyla commit + push et (dosya değiştirme, yalnız commit). Kendi klasöründe `git merge main`, `DERLE.cmd /q` + `TEST.cmd /q`. C dosyalarında hata çıkarsa en küçük düzeltme, mantığı değiştirme; her düzeltme A.md'ye dosya:satır ile ve `main`'e ayrı commit (`C düzeltme: ...`).
2. **Bota C'nin komutlarını öğret** (`MarketDirector::Command` üzerinden, oyuncunun yolu):
   - Reyonlar: `SetDepartment` (Arg = reyon×100 + mağaza türü×10 + açık), `SetDeptStance` (reyon×10 + 0/1/2), `ReplaceMasters` (reyon). Süpermarkette alan %22, hiperde %70; hangi reyonun açılacağını tarz ve son 30 günün reyon kârı (`MarketDepartments::Results`, `FMarketBranchDept::Last30Profit`) belirlesin; zarar eden reyonu kapatsın.
   - Tedarik: `SetSourcing` (hat×10 + kademe), `MarketSourcing::CanSet` izin verince; aylık asgariyi tutturamayacaksa çıkmasın.
   - Markalar: `AcceptBrandOffer` / `RejectBrandOffer` (teklif id'si); kabul ölçütü tarz tablosunda.
   - Rakipler: `BuyChain` (satılık zincir, `State.Rivals.Chains` indeksi); yalnız atak ve dengeli, kasa yetiyorsa.
   - Temkinli az ve geç, atak çok ve erken, dengeli arada. Parametreler tarz tablosunda.
3. **Uzun koşu:** 30 yıl × 3 tarz × 1 tohum ve 10 yıl × 3 tarz × 3 tohum. Rapora **C bölümü** ekle:
   - Ulusal sıra ve dünya ligi sırası yıllara göre. Hedef: dengeli bot ulusal ilk 3'e 10–15. yılda, dünya ilk 10'a 20–25. yılda, birinciliğe 30. yıla yakın ulaşır; temkinli birinci olamaz; atak olabilir ama iflas riski taşır.
   - Reyonların türe ve mağaza türüne göre 30 günlük kârı (zarar edenler ayrıca).
   - Tedarik kademesine çıkış günleri ve düşüşler; marka gelirleri ve küsen markalar.
   - Rakip zincir sayısı, iflaslar, satılığa çıkanlar, satın almalar, fiyat savaşları, ezeli rakip.
   - Sıkıcı dönem ve felaket yığılması sayıları (C sistemleri eklenince değişti mi?).
4. **"C'ye ayar önerileri"** (A.md): sayıyla ve gerekçeyle, ör. `MarketChains::LeagueCompression` 0,04 → ?, `MarketDepartments` tablosunda hangi reyonun `Ratio`/`Margin`'i, tedarik asgari alımları, marka teklif sıklığı. C'nin dosyasındaki sabitleri **sen değiştirme**, C uygular.
5. **Kayıt sürümü (karar M27: yayına kadar eski kayıt uyumu yok):** bu tur için `MarketEconomy.cpp` sana verildi. Kayıt sürümünü tek bir sabitten oku; yüklenen kaydın sürümü güncel sürümden farklıysa yükleme reddedilsin ve `MarketGame` yükleme yolu oyuncuya bir cümle döndürsün ("Bu kayıt oyunun eski bir sürümünden; yeni oyun başlat."). Sürümü şimdi artırma, C3'te bir kez artacak. Menü metni gerekiyorsa C'ye istek. Test: eski sürüm reddedilir, güncel sürüm yüklenir. Kendi dosyalarında yeni `Migrate` ya da eski kayıt dalı yazma.

## Kurallar
07 §2–§3 aynen (M27 değişikliğiyle). Kendi dosyaların + bu tur `MarketEconomy.cpp`; menüye ve B dosyalarına dokunma; C dosyalarında yalnız derleme/test düzeltmesi. Her alt iş ayrı commit + push; DERLE + TEST (+ Smoke) geçmeden bitti deme; limit yaklaşırsa `[yarım]` commit ve A.md başına "Kaldığım yer".

## Bitince bana kısa Türkçe özet
C'nin kodunda kaç düzeltme gerekti, testler kaç/kaç, dengeli botun ulusal ve dünya sırası 10./20./30. yılda, en çok zarar ettiren 3 reyon ve en kârlı 3 reyon, C'ye en önemli 5 ayar önerisi.
