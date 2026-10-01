Miras Market, **C3 doğrulaması (Codex)**. Bu iş `main` üzerinde, ana klasörde (`C:\Users\mtass\Desktop\market-ll`).

Durum: A, B ve C dalları `main`e birleşti (170b1c2). Claude Cowork üstüne **C3 bağlamayı** yazdı ama **derlemedi**. Claude Code'un B işleri de Unreal'de hiç derlenmedi (yalnız Linux taklit katmanında test edildi). Yani bu ilk tam derleme. Önce oku: `Docs/Surec/akislar/C.md` (C3 bölümü), `Docs/Surec/akislar/B.md` ("Doğrulama" ve "C'ye istekler"), `Docs/Kurgu/07_AKIL_ISBOLUMU.md` §3, `AGENTS.md` (M27 kuralı).

## Yapacakların

1. **Al:** `git status` temiz olmalı; değilse dur ve yaz. `git pull`.
2. **Derle ve test et:** `DERLE.cmd /q`, `TEST.cmd /q` (alt sınır 125), `SmokeTest.ps1`.
3. **Hata düzelt:** derleme ya da test hatası hangi dosyada olursa olsun **en küçük düzeltmeyi** yap, oyun mantığını değiştirme. Her düzeltmeyi `Docs/Surec/akislar/A.md`'ye dosya:satır ve bir cümleyle yaz. Mantık hatası görürsen düzeltme, yaz.
   - Özellikle dikkat: `Ledger.CashAudit` artık kasa farkı **0** bekliyor (toptancı vadesi bağlandı). Fark çıkarsa farkın hangi sistemden geldiğini bul ve yaz; ilgili sistemde eksik `MarketLedger::Post` varsa ekle (nakit hareketinin hemen ardına, `B.md` "C'ye istekler 7/13" biçiminde).
   - `MarketStart::Setup` artık `MarketEras::Setup` çağırıyor (dönemler tohumla kayıyor). Fiyat eğrisine bağlı eski bir test beklentisi kırılırsa beklentiyi yeni eğriye uyarla, kuralı değiştirme.
   - `BranchVisitReview`: ziyaret artık şubeyi "görüldü" işaretliyor (`VisitBranch`); karşılaştırma ziyaret açıldıktan sonra alınıyor.
4. **Bot:** kısa bot testi, sonra uzun koşular (30 yıl × 3 tarz × 1 tohum; 10 yıl × 3 tarz × 3 tohum). Rapor A6'daki C bölümüyle aynı başlıkları taşısın, artı:
   - defter denetimi (açıklanamayan fark günleri ve toplamı),
   - dönem olaylarının zamanları ve o dönemlerde kasa/kâr,
   - hedef ve kutlama sayıları, sıkıcı dönem ve felaket yığılması (B'nin ritim koruyucusu artık içeride),
   - zincir kapanma nedenleri (`State.Rivals.Closures / Takeovers / OurBuys`),
   - dengeli bot için ulusal ve dünya sırası 10./20./30. yılda.
   Botun yeni komutlara ihtiyacı yok; gün/hafta/ay turu A3'teki gibi.
5. **Menü görüntüleri:** `-MirasMenuCapture` ile yeni menü (Finans gelir tablosu/bilanço, ana ekran hedefler kartı, Raporlar › Rekorlar, üst hapta +1 gün/hafta/ay, Mağazalar'da Gez). Bozuk, taşan, boş görünen yerleri A.md'de "C'ye istek" olarak listele.
6. **Teslim:** her alt adım ayrı commit (`C3 doğrulama: ...`) + push. `Docs/Surec/akislar/A.md`'ye kısa "C3 doğrulaması" bölümü.

## Kurallar
Oyun sabitlerini (denge sayıları) değiştirme; öneri olarak yaz. Yeni `Migrate` ya da eski kayıt dalı yazma (M27). DERLE + TEST + Smoke geçmeden bitti deme. Limit yaklaşırsa `[yarım]` commit ve A.md başına "Kaldığım yer".

## Bitince bana kısa Türkçe özet
Kaç derleme/test düzeltmesi gerekti (dosya başına), testler kaç/kaç, defter farkı, dengeli botun 10./20./30. yıl sıraları, en önemli 5 denge bulgusu ve menü görüntülerindeki en önemli 3 sorun.
