# Güncel durum

Son güncelleme: 28.09.2026 — Codex (G-052, G-057)

## Kısaca

v0.2 geliştirme. **Yeni oyun rafları boş açılır**; başlangıç stoğu depodadır ve oyuncu R modu/E ile yerleştirmeyi doğrudan deneyebilir. Raflar elle ve önizlemeyle dizilir; **reyon görevlileri** (G-049 geçti) boş rafları depodan doldurur, rafta olmayan ürünü kendi reyonuna dizer ve dar bloğu genişletir. Blender mağaza kiti yedi parçaya çıktı: gondol/duvar rafı, dökme ada, servisli açık tavan, ayrıntılı kasa, yönetim masası, cam kapılı soğutucu ve manav adası. Otomatik yerleştirme yok; yeni ürün "Rafta değil" durumunda başlar.

## Çalışan / var olan

- Oynanabilir prototip; katalog `Config/products.json` şema v2 (aktif ürün sınırı yok, yuva bazlı `materials`). Raf kapasitesi = önde × derinlik × istif (istif raf yüksekliğine sığan kadar sayılır); rafta olmayan ürünün kapasitesi 0. Test modu varsayılan açık (F2/F3).
- Ürün Stüdyosu: Tools > Ürün Stüdyosu veya `STUDYO.cmd`.
- Dış üretim: `Docs/Uretim/00_BASLA_BURADAN.md` → promptlar A1, A2, B1, B2, C1, C2, D, E; `MARKA_VE_URUN_LISTESI.md`.

## Doğrulama durumu

| Kontrol | Sonuç |
|---|---|
| `DERLE.cmd` (v1) | GEÇTİ — 27.09.2026 |
| `DERLE.cmd` (v1.1) | GEÇTİ — 27.09.2026 |
| `TEST.cmd` (9 test) | GEÇTİ 9/9 — 27.09.2026 |
| `DERLE.cmd` (v1.2: acilim.json, %15 oturtma, önizleme ışığı) | GEÇTİ (v1.5 toplam derlemesi) — 27.09.2026 |
| `DERLE.cmd` + `TEST.cmd` (v1.3: hazırlık listesi, prompt üretimi) | GEÇTİ 9/9 (güncel toplam doğrulama) — 27.09.2026 |
| `DERLE.cmd` (v1.4: açılım paneli otomatik bulma, yeni A1) | GEÇTİ (v1.5 toplam derlemesi) — 27.09.2026 |
| `DERLE.cmd` + `TEST.cmd` (v1.5: hazır ambalaj kütüphanesi, stüdyo şekilleri, parça renkleri) | GEÇTİ; derleme uyarısız, test 9/9 — 27.09.2026 |
| `DERLE.cmd` + `TEST.cmd` (v1.6: özel model UV kılavuzu) | GEÇTİ; 2048 px PNG görsel olarak incelendi — 27.09.2026 |
| `DERLE.cmd` + `TEST.cmd` (v1.7: hazır ambalaj ajan şablonları + özel model dönüşümü) | GEÇTİ; test 10/10, kutu/etiket/kapak PNG'leri görsel incelendi — 27.09.2026 |
| `SmokeTest.ps1` | GEÇTİ; 5 müşteri satışı, gün kapama ve disk kayıt/yükleme — 27.09.2026 |
| Görsel temel v1 (`DERLE.cmd`, `TEST.cmd`, `SmokeTest.ps1`, 1280×720 sahne yakalama) | GEÇTİ; 10/10 test, ışık/raf/ürün sahnesi gözle incelendi — 28.09.2026 |
| Canlılık geçişi v2 (referans market karşılaştırması) | GEÇTİ; derleme, 10/10 test, smoke ve 1280×720 görüntü kontrolü — 28.09.2026 |
| Blender gondol v1 (`create` + Blender validate + Unreal import/validate + oyun) | GEÇTİ; 120×90×160 cm, 5 materyal, 3 UCX, 8 raf bölgesi; derleme, 10/10 test, smoke ve ekran görüntüsü — 28.09.2026 |
| Planogram v1: çok marka + facing + derinlik + editör | GEÇTİ; derleme, 11/11 test, smoke (4 satış) ve 1280×720 sahne kontrolü — 28.09.2026 |
| Blender mağaza kiti v1 | GEÇTİ; 3 kaynak `.blend`, FBX, metadata ve önizleme; Unreal ölçü/materyal/çarpışma doğrulaması ve 1280×720 oyun görüntüsü — 28.09.2026 |
| Raf genişliği sınırı (G-028: otomatik yerleşim, editör kontrolleri, taşma uyarısı, WidthLimit testi) | GEÇTİ — güncel 14/14 test içinde — 28.09.2026 |
| G-029…G-034 canlılık + sınırsız raf + test modu (HUD, Lumen, yüzey kütüphanesi, dokular, tabela/fiyat etiketi, `SON_KONTROL.cmd`) | GEÇTİ — derleme, 14/14 test, smoke ve görsel kontrol — 28.09.2026 |
| `SON_KONTROL.cmd` (G-028 + G-029…G-034) | GEÇTİ — Mustafa çalıştırdı; derleme, malzemeler, 14 test, smoke ve ekran görüntüsü — 28.09.2026 |
| Opus sonrası birleşik sürüm (G-028…G-043) | **GEÇTİ** — `DERLE.cmd`, 14/14 otomasyon, smoke, PBR malzeme aktarımı ve beş açılı görsel kontrol — 28.09.2026 |
| Nötr-sıcak market ışığı ayarı | **GEÇTİ** — beyaz raf/etiket dengesi ve ürün renkleri beş adet 1280x720 görüntüde incelendi — 28.09.2026 |
| Planogram v2: serbest konum, yön, istif, raf önizlemesi ve ince fiyat rayı | **GEÇTİ** — derleme, 15/15 test, smoke, Blender/Unreal varlık doğrulaması ve beş açılı oyun görüntüsü — 28.09.2026 |
| G-045: elle dizme (otomatik dolum/yerleştirme kaldırıldı, oyunda R modu, editör seviye/sıra düğmeleri) + inceleme hataları | GEÇTİ — derleme, 16/16 test, smoke (3 satış) — 28.09.2026 |
| G-046: önizlemeli nişanla dizme, serbest konum (şema v3), aynı üründen çok blok, RAF DÜZENİ paneli, blok tabanlı editör | GEÇTİ — derleme (iki gölgeleme hatası düzeltildikten sonra), 16/16 test, smoke — 28.09.2026 |
| G-047: dip dibe dizme (0,3 cm tolerans, önde arası 0,5 cm), blok aralığı ayarı (Z/X, `gap`), raftaki blokları gösteren gri şeritler, 3B etikette Türkçe harf düzeltmesi | GEÇTİ (Mustafa oyunda denedi) |
| G-047 ek: raf genişliği tam tabla (gondol 116, duvar 235 cm), düzen modunda bloklar dolu çizilir, çift sayıda önde eksik sütun | GEÇTİ — derleme, 16/16 test; Mustafa oyunda denedi ("çok güzel oldu") — 28.09.2026 |
| G-048: kod temizliği (eski raf planı kodu ve strateji düğmeleri silindi, ortak yardımcılar, `MarketAutomation.cpp`, Stüdyo arka ucu 5 dosyaya bölündü, `RebuildRight` parçalandı) | GEÇTİ — derleme, 16/16 test, smoke (2 satış) — 28.09.2026 |
| G-049: reyon görevlisi (J/K, yürüyerek raf doldurma, rafta olmayan ürünü kategorisinin reyonuna dizme, dar bloğu genişletme, `Staff.Planner` testi) | **GEÇTİ** — derleme, 17/17 test ve smoke — 28.09.2026 |
| G-050: Blender kasa/masa/soğutucu/manav/tavan ayrıntıları + boş raf başlangıcı | **GEÇTİ** — 7 varlıkta 0 hata/0 uyarı, derleme, 17/17 test, smoke ve 5 açılı görüntü — 28.09.2026 |
| G-051: fiyat ve müşteri talebi (`MarketDemand`, kayıp nedenleri, gün raporunda 3 sorun, masada rakip fiyatı, `Customers.PriceAndDemand` testi) | **GEÇTİ** — birleşik derleme, 19/19 test ve smoke — 28.09.2026 |
| G-056: MetaHuman ortak retarget + doğal hareket + `BP_MH_*` otomatik keşif | **GEÇTİ** — iki animasyon üretildi; 1 MetaHuman ve dönüştürülmüş yürüyüş/bekleme smoke günlüğünde yüklendi — 28.09.2026 |
| G-052: çok ürünlü sipariş listesi + arka kapıda fiziksel mal kabul + oyuncu/görevli taşıması + eksik/hasarlı olay | **GEÇTİ** — derleme, 20/20 test ve smoke — 28.09.2026 |
| G-057: ölçülmüş yürüyüş klibi hızına göre ayak kayması düzeltmesi | **GEÇTİ** — 376,276 cm / 1,5 sn kök hareketi ölçüldü; 140 cm/sn oyun hızında oynatma 0,558 — 28.09.2026 |
| G-058: sipariş önerisi + 50 TL toptancı asgarisi | **GEÇTİ** — derleme, 21/21 test ve smoke — 28.09.2026 |
| Stüdyoda elle deneme: kutu (açılım) → Oyuna ekle → OYNA | GEÇTİ: milk_1l oyunda; ön/yan/üst yüzler doğru, ayna yok, raf oturması doğru — 27.09.2026 |
| Stüdyoda elle deneme: cam şişe modeli + malzeme.json | Bekliyor (henüz model yok) |

## Bilinen riskler

- Stüdyo şekillerinde (shape_*) etiket yönü ve kapak UV'si ilk kez görülecek; ters/ayna ise `CreateShapePackage` → `Ring` işareti.

- İçe alınan FBX'te ön yüz yönü ilk gerçek modelde doğrulanmalı; gerekirse Stüdyo'daki Pitch/Yaw/Roll alanlarıyla düzeltilir.
- `M_ProductGlass` saydam materyali ilk kez oluşturulacak; UE 5.8'de `BlendMode` erişimi uyarı verebilir.
- Oyun şimdilik tek etiket gösterir (dönem etiketleri G-011).
- Prosedürel mağaza artık daha okunaklıdır; fotogerçekçi hedef için sonraki turda raf/zemin/duvar PBR doku setleri ve ayrıntılı prop modelleri gerekir (G-021).
- Lumen + mesafe alanları açık (`DefaultEngine.ini`); ilk açılışta shader derlemesi uzun sürebilir.
- `gorsel_malzemeler.py` Unreal Python malzeme grafiği kuruyor; pin adı uyuşmazsa `GORSEL_son.log` içinde `MIRAS_MATERIALS_ERROR` yazar ve oyun düz renklere düşer (çökmez).
- MetaHuman ortak yürüyüş/bekleme animasyonları doğru iskelete dönüştürüldü ve smoke günlüğünde yüklendi. Yüz, saç, kıyafet ve ayak basışı normal oyun kamerasında elle incelenmeli; raf alma/ödeme klipleri henüz yok.
- Raf Planı Editörü derlendi ve davranış kuralları otomasyonla doğrulandı; bu oturumdaki Windows otomasyon yüzeyi Unreal penceresini sunmadığı için panelin son piksel düzeni elle açılarak ayrıca görülebilir.

## Yön kararı (28.09.2026, Mustafa onayladı)

Görsel ve raf dizme işleri yeterli seviyede; bir süre **donduruldu**. Öncelik **"İlk Hafta" oynanabilir dilimi**: bir oyuncu 7 günü baştan sona oynayabilmeli (sipariş → mal kabul → raf → fiyat → satış → gün raporu → hafta hedefi). Görevler G-051…G-055. Başarı ölçütü: yeni biri ilk 30 dakikada döngüyü anlıyor ve ilk kararının sonucunu görüyor.

## Devam notu

G-052 ve G-057 tamamlandı. Sipariş masasında B/V ile çok ürünlü liste hazırlanır ve N ile onaylanır; ertesi sabah koliler KABUL stoğunda ve arka kapıda görünür. Oyuncu E ile depoya taşır, reyon görevlileri bunu otomatik önceliklendirir. MetaHuman yürüyüş oynatma oranı klibin ölçülen 250,85 cm/sn kök hızına bağlandı; 12 cm/sn altında beklemeye geçer.

G-058 doğrulandı: masada L önerilen siparişi taslağa yazar, seçili ürünün stok ve talep özeti görünür; N ile onaylanan siparişte 50 TL toptancı asgarisi uygulanır.

## Sıradaki adımlar

0. Mustafa: oyunda MetaHuman ayak basışını yeniden dene; masada birkaç ürünü B ile listeye ekle, N ile onayla, günü kapat ve arka kapıdaki koliyi E ile depoya taşı.
1. G-053 alışveriş listesi → G-054 hafta hedefi (borç tutarı ve G-008 rakip kararı Mustafa'da) → G-055 oyun testi.
2. Paralel (Mustafa): İlk Hafta için 20–40 ürünü gerçek ambalajıyla hazırla; 97 ürünün hepsi gerekmiyor.
3. Dondurulanlar (İlk Hafta bitince): G-021 servis reyonları, G-042 MetaHuman çeşitliliği, FAB paketleri, G-017 ambalaj yönü denemeleri.
