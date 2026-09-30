Miras Market, **Akış A — ikinci tur**. Aynı dal (`akis-a`) ve aynı klasörde (`C:\Users\mtass\Desktop\market-ll-A`) devam ediyorsun.

Başlamadan: A1 ve A3 bitmiş, `Docs/Surec/akislar/A.md` güncel, son commit push edilmiş olmalı. Değilse önce onları bitir. Sonra `Docs/Kurgu/07_AKIL_ISBOLUMU.md`'yi yeniden oku (değişti): §4 "Sonraki işler" altındaki **A4** ve **A5** senin işin; kurallar §2–§3 aynen geçerli. `Docs/Kurgu/06_GIDIS_YOLU.md` §2b (zevk ve akış) ile simülasyon ilkesini ("dükkân aklın penceresi: içeride görülen her şey sayılardan gelir") da oku. A4 için `main`'deki Akış C işi (`MarketStoreViews.*`, `FMarketBranch::StoreView`) gerekiyor; bu iş `main` klasöründe **commit edilmemiş** duruyor:
1. `main` klasöründe (`C:\Users\mtass\Desktop\market-ll`) `git status`; yalnız `Source/`, `Docs/`, `Config/` altındaki değişiklikleri `C1: mağaza görünümü şube hesabına (Akış C, derlenmedi)` mesajıyla commit + push et. Dosya değiştirme, yalnız commit.
2. Kendi klasöründe `git merge main`; `DERLE.cmd /q` + `TEST.cmd /q`.
3. C'nin dosyalarında derleme/test hatası çıkarsa (C derlenmeden teslim etti) en küçük düzeltmeyi yap, mantığı değiştirme, her düzeltmeyi A.md'ye dosya:satır ile yaz; bu düzeltmeleri ayrı commit olarak `main`'e de uygula (`C1 düzeltme: ...`) ki C aynı hatayı görmesin.

## Yapacakların
**A4 · Şube ziyareti:** `AMarketGameMode::StartBranchVisit(int32 BranchIndex)` / `EndBranchVisit()`. Test modu açılmaz, aile dükkânının ekonomisi ve kaydı değişmez, ziyarette ekonomi saati durur, çıkınca oyuncu aynı yere ve hâle döner. Mağaza şubenin `StoreView`'ından; raflar `Units/Capacity` oranında dolu, müşteri sayısı `LastShoppers`'a, kuyruk `LastQueueLost`'a, çalışan `Workers`'a göre; kötü karnede biraz dağınıklık (yalnız görsel). Üstte küçük bilgi şeridi. Başlarken `MarketDirector::Command(State, "VisitBranch", BranchIndex, Message)` çağır (komut yoksa sessizce geç). `BranchVisitReview` kontrolü: ziyaret aç → 3 açıdan PNG → çık → kayıt değişmedi. Ayrıntı 07 §4 A4.

**A5 · Menü ekran görüntüsü otomasyonu:** `-MirasMenuCapture`: otomatik oyuncuyla 3. yıla kadar oynanmış dolu bir kampanya; menünün her sayfası/sekmesi açık ve koyu temada, 1920×1080 ve 1280×720 PNG → `Saved/Screenshots/Menu/<tarih>/` + yan yana gösteren `index.html`. PNG'leri kendin gözle kontrol et; bozuk, taşan ya da boş görünen sayfaları A.md'de "C'ye istek" olarak listele (menü C'nin, sen düzeltme).

## Kurallar
07 §2–§3 aynen: yalnız kendi dosyaların (`MarketGame.*`, `MarketStoreTour.cpp`, `MarketStoreBuild.cpp`, `MarketAutomation.cpp`, yeni dosyaların), menüye ve C/B dosyalarına dokunma; her alt iş ayrı commit + push; DERLE + TEST (+ Smoke) geçmeden bitti deme; limit yaklaşırsa `[yarım]` commit ve A.md başına "Kaldığım yer".

## Bitince bana kısa Türkçe özet
Ne bitti, testler kaç/kaç, ziyaret ve menü görüntülerinden gördüğün en önemli 5 sorun, C'nin bağlaması gerekenler.
