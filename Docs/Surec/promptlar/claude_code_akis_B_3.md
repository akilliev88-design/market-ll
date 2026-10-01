Miras Market, **Akış B — üçüncü tur (B7)**. Aynı dal (`akis-b`) ve aynı klasörde (`C:\Users\mtass\Desktop\market-ll-B`) devam ediyorsun. Bu, birleştirmeden (C3) önceki son işin.

Başlamadan: B6 bitmiş, `Docs/Surec/akislar/B.md` güncel, son commit push edilmiş olmalı. Değilse önce onu bitir. Sonra `git fetch` ve `main`'deki şu belgeleri oku (değişti): `Docs/Kurgu/07_AKIL_ISBOLUMU.md` §3 (eski kayıt kuralı değişti, karar M27) ve §4 "Üçüncü tur" altındaki **B7**; `Docs/Kurgu/01_KARARLAR.md` M25–M27; `Docs/Surec/akislar/C.md`. C'nin yeni başlıklarına bak (`main`'de): `MarketDepartments.h` (reyonlar), `MarketSourcing.h` (tedarik ağı), `MarketBrands.h` (markalar), `MarketChains.h` (rakip zincirleri). Bunları kendi dalına birleştirmen gerekmez; imzaları okumak yeter.

## Neden
C'nin yeni sistemleri senin dönem olaylarını ve defterini henüz tanımıyor. Kur şokunda elektronik pahalanmıyor, durgunlukta giyim satışı düşmüyor, reyon satışı deftere yazılmıyor. Bu tur, birleştirmede C'nin bunları tek satırla bağlayabilmesi için senin tarafını hazırlıyor.

## Yapacakların
1. **Dönem çarpanları** (`MarketEras`, dünyadan bağımsız, tohumlu, testli). C'nin okuyacağı işlevler:
   - Gıda dışı talep çarpanı (durgunlukta elektronik, giyim, oyuncak düşer; toparlanmada artar); taze talep çarpanı.
   - İthal mal maliyet çarpanı (kur şokunda markalı ürün ve elektronik maliyeti artar, sonra yavaş iner).
   - Rakip zincir baskısı (durgunlukta zayıf zincirler daha kolay kırmızıya düşer ve satılığa çıkar; toparlanmada açılış artar).
   - Her işlevin imzası, bir cümle açıklaması ve C'nin bağlayacağı yer (dosya ve işlev: `MarketDepartments::Day`, `MarketDirector::ApplyPrices`, `MarketChains` ay turu) `B.md` "Yeni açık işlevler" ve "C'ye istekler"e.
   - Testler: olay yokken hepsi 1; kur şokunda ithal maliyeti artar ve zamanla iner; durgunlukta gıda dışı taleple taze talep arasında fark var.
2. **Defter hesapları** (`MarketLedger`): C'nin yeni sistemlerinin hesaplarını ekle: reyon satışı, reyon malı, reyon firesi, reyon tadilatı, reyon stok satışı (kapatınca), usta değişimi, marka ödemeleri (raf payı, raf parası, ciro primi), zincir satın alma ve fazla mağaza satışı, tedarik. `B.md`'ye her para hareketi için hazır `Post` çağrısı listesi yaz (dosya, işlev, satır tarifi, hangi hesap, işaret). Böylece C3'teki bağlama tek satırlık çağrılar olur.
3. **M27 temizliği, yalnız senin dosyalarında:** `MarketStaff::Migrate` ve B dosyalarındaki diğer eski kayıt dalları ile eski kayıt testlerini sil. Silinen testlerin sayısını B.md'ye yaz (Test.ps1 alt sınırını Codex günceller). `FMarketState`'teki ölü alanları (ör. `bCashier`, `Stockers`) **silme**, ortak dosya; listesini "C'ye istek" olarak yaz. Bundan sonra yeni kodda `Migrate` ya da eski kayıt dalı yazma.

## Kurallar
07 §2–§3 aynen (M27 değişikliğiyle): yalnız kendi dosyaların; menüye, A ve C dosyalarına dokunma; metin sade Türkçe, yılsız (M24); her alt iş ayrı commit + push; kendi klasöründe DERLE + TEST geçmeden bitti deme; limit azalırsa `[yarım]` commit ve B.md başına "Kaldığım yer".

## Bitince bana kısa Türkçe özet
Ne bitti, testler kaç/kaç (kaç eski kayıt testi silindi), dönem çarpanlarının örnek değerleri (kur şokunda elektronik maliyeti, durgunlukta gıda dışı talep), C'nin bağlaması gereken çağrı sayısı.
