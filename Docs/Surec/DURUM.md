# Güncel durum

Son güncelleme: 28.09.2026 — Codex

## Kısaca

v0.2 geliştirme. Raf sistemi veri güdümlü planograma geçti: bir gondolda birden çok marka, seviye, facing, arka derinlik ve ön/arka yüz kullanılabiliyor. `RAF_PLANI.cmd` ile açılan editör planı kod yazmadan düzenliyor. Blender 5.2 hattı için kullanıcı rehberi, metadata şablonu ve Claude/harici ajan sözleşmesi hazır.

## Çalışan / var olan

- Oynanabilir prototip; katalog `Config/products.json` şema v2 (1–24 ürün, yuva bazlı `materials`).
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
| Stüdyoda elle deneme: kutu (açılım) → Oyuna ekle → OYNA | GEÇTİ: milk_1l oyunda; ön/yan/üst yüzler doğru, ayna yok, raf oturması doğru — 27.09.2026 |
| Stüdyoda elle deneme: cam şişe modeli + malzeme.json | Bekliyor (henüz model yok) |

## Bilinen riskler

- Stüdyo şekillerinde (shape_*) etiket yönü ve kapak UV'si ilk kez görülecek; ters/ayna ise `CreateShapePackage` → `Ring` işareti.

- İçe alınan FBX'te ön yüz yönü ilk gerçek modelde doğrulanmalı; gerekirse Stüdyo'daki Pitch/Yaw/Roll alanlarıyla düzeltilir.
- `M_ProductGlass` saydam materyali ilk kez oluşturulacak; UE 5.8'de `BlendMode` erişimi uyarı verebilir.
- Oyun şimdilik tek etiket gösterir (dönem etiketleri G-011).
- Prosedürel mağaza artık daha okunaklıdır; fotogerçekçi hedef için sonraki turda raf/zemin/duvar PBR doku setleri ve ayrıntılı prop modelleri gerekir (G-021).

## Devam notu

Yok.

## Sıradaki adımlar

1. G-024: Aynı standardı kullanan 1200 mm tek yüz duvar rafı üret; yan duvarlardaki boşluğu kategori raflarıyla doldur.
2. G-021: Terrazzo zemin ve raf için gerçek PBR doku setlerini ana materyallere bağla.
3. Mustafa: PET/teneke/kavanoz/kase türlerinden birer ürün üretip Oyuna ekle; etiket yönü, kapak ve malzeme yuvalarını gözle doğrula (G-017).
