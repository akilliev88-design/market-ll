# Güncel durum

Son güncelleme: 28.09.2026 — Codex

## Kısaca

v0.2 geliştirme. Raf sistemi veri güdümlü planogram v2'ye geçti. Ürün bloğu rafta 5 cm adımlarla taşınabilir, çeyrek tur döndürülebilir, uygun kutu/poşet yan yatırılabilir ve raf yüksekliği elverdiğinde üst üste dizilebilir. Editör gerçek ambalaj küçük görseli ile kat/seviye raf şeması gösterir. Blender gondol ve duvar reyonlarında fiyat etiketi profili, raf önündeki yüksek set yerine raf altına asılan ince raydır.

## Çalışan / var olan

- Oynanabilir prototip; katalog `Config/products.json` şema v2 (aktif ürün sınırı yok, yuva bazlı `materials`). Raf kapasitesi = raf planında önde × derinlik; otomatik dolum açık. Test modu varsayılan açık (F2/F3).
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
- MetaHuman smoke ve yükleme günlüğüyle doğrulandı; yüz, saç ve kıyafet kalitesi normal oyun kamerasında ayrı bir yakın plan turunda incelenmeli.
- Raf Planı Editörü derlendi ve davranış kuralları otomasyonla doğrulandı; bu oturumdaki Windows otomasyon yüzeyi Unreal penceresini sunmadığı için panelin son piksel düzeni elle açılarak ayrıca görülebilir.

## Devam notu

G-044 tamamlandı. `Config/planograms.json` şema v2 oldu; v1 dosyaları geriye uyumlu okunuyor. Raf Planı Editörü konum/yön/istif kontrolleri, gerçek ambalaj küçük görseli ve raf şeması kazandı. Oyunda yön ve istif gerçek ürün dönüşümüne/kapasitesine bağlandı. Blender gondol ve duvar rafı ince fiyat rayıyla yeniden üretildi ve Unreal'a aktarıldı. Derleme, 15/15 test, smoke ve beş açılı oyun kontrolü geçti; yarım kod yoktur.

## Sıradaki adımlar

00. Mustafa: `Docs/Environment/FAB_PAKET_LISTESI.md` paketlerini indir (G-042); Claude bağlayacak.
1. Raf tekrarını azalt: her kategoriye daha fazla gerçek ürün/ambalaj ekle ve planogram editöründe aynı seviyede marka bloklarını karıştır. Editörü `RAF_PLANI.cmd` ile açıp yeni raf şemasının son piksel düzenini Mustafa ile birlikte kontrol et.
2. G-021'i sürdür: kasa, soğutucu, manav, fırın ve servis reyonları için Blender modülleri üret.
3. `MH_Teyze`yi normal oyun kamerasında yakın planda kontrol et; ardından `MH_Amca`, `MH_Anne`, `MH_Genc` ve döneme uygun kıyafetleri ekle (G-042).
4. Mustafa: PET/teneke/kavanoz/kase türlerinden birer ürün üretip Oyuna ekle; etiket yönü, kapak ve malzeme yuvalarını gözle doğrula (G-017).
