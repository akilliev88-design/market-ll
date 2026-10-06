# Oyun içi sanat denemesi

05.10.2026 · Codex · G-106 · Mustafa'nın gerçek oyun sahnesi isteği.

## Açma ve gezinme

Proje kökündeki `SANAT_DENEME.cmd` dosyasına çift tıkla.

- `1`: genel mağaza görüntüsü.
- `2`: raf, ürün ve fiyat etiketi yakını.
- `3`: insan ve soğutucu yakını.
- `0`: oyuncunun gözünden gezinmeye geç; WASD ve fare.
- `Esc`: çık.

Bu ayrı bir GameMode ile kurulan 8 × 10 m deneme alanıdır. Kampanya, stok, para ve oyun kaydı oluşturmaz. Yeni oyun ekranı veya menü tasarımı değildir. Kaynak: `Source/MarketSim/MarketArtTrial.h/.cpp`.

## Denenen görünüm

Mevcut raf/soğutucu/kasa modelleri ve Ürün Stüdyosu'nda hazırlanmış 22 ürün kullanıldı. Yalnız etiketli ürünler seçildi; boş temsili ürün kutuları kullanılmadı. Her raf katı ürün ölçüsüne göre bloklarla doldurulur. Bu işlem yalnız görsel örnek içindir; ekonomik stok oluşturmaz. Kutu ambalajlarının mevcut stüdyo malzemeleri ve ortak UV yerleşimi korunur.

Nötr boyalı metal, antrasit taşıyıcılar, mavi fiyat şeritleri, fiyat yazıları, terrazzo zemin ve gölge veren alan ışıkları denendi. Pozlama sabit tutulur. Kıyafetlerde mevcut MetaHuman malzemesinin renk parametreleri geçici olarak değiştirilir; kaynak varlık değiştirilmez.

## Sınırlar

Bu sahne sanat dilinin ilk motor denemesidir; daha önceki Imagegen konseptlerinin birebir karşılığı veya nihai kalite değildir. Mevcut insanın kısa kıyafeti, çıplak ayakları ve bekleme animasyonu geçicidir. Bu tur yeni karakter, kıyafet, ayakkabı veya ambalaj grafiği üretilmedi. Katalogda hazır bulunan bazı etiketler gerçek markaları taşır; nihai özgün ambalaj dilinin seçildiği anlamına gelmez.

Sol duvardaki çerçeveli cam panel ve yan alan ışığı yalnız ışık denemesidir; tamamlanmış dış dünya/pencere manzarası değildir. Ana oyunun müşteri davranışı veya mağaza ekonomisi bu sahnede çalıştırılmaz.

## Yeniden görüntü alma

`powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ArtTrialReview.ps1`

Üç görüntü `Saved/Screenshots/ArtTrial/01.png`–`03.png`, çalışma kaydı `Saved/Logs/ArtTrial.log`. Script yeni dosya zamanını ve başarı satırını kontrol eder. Shader derleme bitince her kamera için yerleşme süresi bırakılır. En fazla 300 saniyede tamamlanamayan yakalama hata verir.

Kare süresi gerçek saatten ölçülür; GPU süresi değildir. Projenin 90 FPS sınırı ve küçük sahne kapsamı dikkate alınmalıdır. Bütün mağazalar/kalabalık/alt donanımlar için performans garantisi değildir.


## Son doğrulama — 05.10.2026

- DERLE: başarılı.
- TEST: 173 başarılı + 1 uyarılı, 0 başarısız (174 test).
- Smoke: başarılı; gerçek ana oyun açılışı, stok, sipariş, mal kabul, çalışan, satış, gün kapanışı ve kayıt kontrolü.
- Sanat yakalama: üç yeni 1600×900 PNG, ART_TRIAL_PASSED; görsel olarak incelendi.
- Sahne: 7 ekipman, 22 ürün çeşidi, 244 blok, 2417 fiziksel ürün örneği, 1 MetaHuman.
- RTX 4070 Laptop GPU; üç kamerada medyan 11,11 ms, p95 11,18–11,22 ms. 90 FPS sınırında, gerçek saat ölçümü; GPU ölçümü veya bütün oyun için sonuç değildir.
- Son kıyafet sürümünde skeletal material kullanım uyarısı yok.

Kalıcı görsel kopyalar: Docs/Images/ArtDirection/20261005/Unreal/01.png–03.png. Üçüncü açı insanı arkadan gösterir; yüz genel görüntüde görünür. İnsan/ürün grafikleri mevcut varlıklardır, nihai sanat üretimi değildir.
