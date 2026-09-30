Miras Market projesindesin (Unreal 5.8, C++, Türkçe market yönetim oyunu). Proje kökü: C:\Users\mtass\Desktop\market-ll. Oyunun aklı (Aşama 1) üç ajana bölündü; sen **Akış B**'sin: denge hataları ve şirket derinliği. Aynı anda Codex (Akış A) ve Claude Cowork (Akış C) başka dosyalarda çalışıyor. İşin sonunda Cowork senin dalını inceleyip oyuna ve menüye bağlayacak; ona temiz, belgelenmiş, test edilmiş bir dal bırak.

## Başlamadan
- `git log --oneline -10` içinde `Aşama 0` ile başlayan bir commit yoksa **başlama**; Codex'in Aşama 0'ı bitirmesini bekle ve bana söyle.
- Sonra kendi worktree'ni aç: `git worktree add ..\market-ll-B -b akis-b`, orada `git lfs pull`, ilk `DERLE.cmd /q` (uzun sürer) ve `TEST.cmd /q` — başlangıçta her şey yeşil olmalı. Bundan sonra **yalnız** `..\market-ll-B` klasöründe çalış; `main` klasörüne dokunma.

## Önce oku (sırayla)
1. `AGENTS.md` (kurallar; §5 kod kuralları, §6 "bitti" şartı)
2. `Docs/Kurgu/07_AKIL_ISBOLUMU.md` — **bu işin sözleşmesi.** Dosya sahipliği, ortak dosyalardaki işaretli bloklar, kalite kuralları ve senin işinin tam tarifi (§4 Akış B). Bu prompt ile çelişirse o belge geçerlidir.
3. `Docs/Kurgu/06_GIDIS_YOLU.md` (tycoon ilkeleri §2), `00_KURGU_KITABI.md` (§11 Finans, §12 Büyüme), `01_KARARLAR.md`
4. `Docs/Kurgu/02_DERIN_INCELEME.md` — hataların (#24, #27, #30, #37, #39, #42, #43, #45…) tarifi; `05_YOL_HARITASI.md` §2 güncel durumları
5. Dokunacağın modüllerin başlıkları ve testleri, ayrıca `MarketEconomy.h` (`FMarketState`) ve `MarketDirector.cpp` (`CloseDay` sırası)

## Yapacakların (sırayla; her biri ayrı commit, her biri testli)
- **B1 · Açık denge hataları:** #24 raftaki alma kararı ürün esnekliğine bağlı, #27 maliyet altı satış uyarısı + destekli kampanya raporu, #30 online kampanya fiyatı, #43 zarar devri / son gün indirimi / ipotek, #45 ulusal pay ciroya bağlı; #21, #25, #31, #39'u kontrol et ve raporla.
- **B2 · Muhasebe defteri** (`MarketLedger.*`): tek kaynak defter, gelir tablosu + bilanço, "kasa değişimi = kayıtlar toplamı" denetimi; kendi dosyalarındaki para hareketlerini bağla, diğerleri için C'ye satır satır liste.
- **B3 · Ücret ve sigorta:** asgari ücret tabanı, işveren sigorta payı (ülke paketinden), kıdem tazminatı, deftere yazım.
- **B4 · Dönem olayları** (`MarketEras.*`): kur şoku, yüksek enflasyon, durgunluk, salgın (mevcut `MarketOnline` salgınını buradan tetikle), toparlanma; sıra sabit, zaman kampanyaya göre ±2 yıl, sıklık ülke ekonomi karakterine bağlı (`ulkeler.json`); adsız, yılsız.
- **B5 · (vakit kalırsa)** dünya ve son: kurgu kurlar, `MarketLeague.*`, oyun sonu. Vakit yoksa başlama, notuna yaz.
Ayrıntılı tarif 07 §4 Akış B'de.

## Kurallar (kısaca; ayrıntısı 07 §2–§3)
- Yalnız kendi dosyaların (07 §2 Akış B satırı). `MarketEconomy.h` ve `MarketDirector.cpp`'de yalnız `// ===== Akış B =====` bloğuna **ekleme**; mevcut satırı değiştirme. Menüye (`MarketMenu*`, HUD, Map) ve C/A dosyalarına dokunma; gerekeni akış notuna "C'ye istek" olarak yaz (mümkünse hazır kod parçasıyla).
- Dünyadan bağımsız, tohumlu ve belirlenimci mantık; para `int64` kuruş; her yeni kurala otomasyon testi (`MirasMarket.<Modül>.<Konu>`), özellikle para korunumu, iki kez uygulama, eski kayıt uyumu.
- C++ ASCII, Türkçe metin `\uXXXX` (`python Tools/escape_unicode.py`); C4456–C4459 gölgelemesi derlemeyi kırar; unity build için adlı namespace.
- Oyuncuya giden metin: sade Türkçe, bir cümle + bir sayı; takvim yılı ve gerçek dünya tarihi yok (M24).
- Her alt işten sonra kendi klasöründe `DERLE.cmd /q` + `TEST.cmd /q`; ikisi geçmeden sonraki işe geçme. Hata çıkarsa logu (`Saved/Logs/DERLE_son.log`, `Saved/TestReports`) okuyup düzelt.
- Her alt iş ayrı commit (`B2: muhasebe defteri ...`), düzenli `git push -u origin akis-b`.
- Akış boyunca DURUM/GOREVLER/GUNLUK/AGENTS'a dokunma; günlüğün `Docs/Surec/akislar/B.md` (biçim 07 §5). Sayılarda ve tasarımda verdiğin her kararı oraya "Kararlar ve varsayımlar" altına yaz.
- Limitin azalırsa: `[yarım]` commit + push, B.md'nin başına "Kaldığım yer" (dosya, adım, derleme durumu).

## Bitince bana (Mustafa'ya) kısa Türkçe özet ver
Ne bitti, testler kaç/kaç geçti, hangi hata numaraları kapandı, Cowork'ün bağlaması gerekenler, bana sorulması gereken kararlar.
