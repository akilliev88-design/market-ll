# Güncel durum

Son güncelleme: 27.09.2026 — Codex

## Kısaca

v0.2 geliştirme. Stüdyo v1.5 derlendi ve otomasyon doğrulaması geçti: hazır ambalaj kütüphanesi (`Config/ambalajlar.json`, 56 kayıt), stüdyo şişe/teneke/kavanoz/kase şekillerini kendisi üretir (3B kapaklı), parça renkleri, karton kutuda kapak etiketin üst yüzüne çizilir; açılım panelleri otomatik bulunur; 97 ürüne ambalaj atandı. Müşterisiz gün artık pazar payını değiştirmez; kasa kuyruğu dizi sırası yerine gerçek varış sırasını kullanır.

## Çalışan / var olan

- Oynanabilir prototip; katalog `Config/products.json` şema v2 (1–24 ürün, yuva bazlı `materials`).
- Ürün Stüdyosu: Tools > Ürün Stüdyosu veya `STUDYO.cmd`.
- Dış üretim: `Docs/Uretim/00_BASLA_BURADAN.md` → promptlar A1, A2, B1, B2, C1, C2, D, E; `MARKA_VE_URUN_LISTESI.md`.

## Doğrulama durumu

| Kontrol | Sonuç |
|---|---|
| `DERLE.cmd` (v1) | GEÇTİ — 27.09.2026 |
| `DERLE.cmd` (v1.1) | GEÇTİ — 27.09.2026 |
| `TEST.cmd` (8 test) | GEÇTİ 8/8 — 27.09.2026 |
| `DERLE.cmd` (v1.2: acilim.json, %15 oturtma, önizleme ışığı) | GEÇTİ (v1.5 toplam derlemesi) — 27.09.2026 |
| `DERLE.cmd` + `TEST.cmd` (v1.3: hazırlık listesi, prompt üretimi) | GEÇTİ 8/8 (güncel toplam doğrulama) — 27.09.2026 |
| `DERLE.cmd` (v1.4: açılım paneli otomatik bulma, yeni A1) | GEÇTİ (v1.5 toplam derlemesi) — 27.09.2026 |
| `DERLE.cmd` + `TEST.cmd` (v1.5: hazır ambalaj kütüphanesi, stüdyo şekilleri, parça renkleri) | GEÇTİ; derleme uyarısız, test 8/8 — 27.09.2026 |
| `SmokeTest.ps1` | GEÇTİ; 5 müşteri satışı, gün kapama ve disk kayıt/yükleme — 27.09.2026 |
| Stüdyoda elle deneme: kutu (açılım) → Oyuna ekle → OYNA | GEÇTİ: milk_1l oyunda; ön/yan/üst yüzler doğru, ayna yok, raf oturması doğru — 27.09.2026 |
| Stüdyoda elle deneme: cam şişe modeli + malzeme.json | Bekliyor (henüz model yok) |

## Bilinen riskler

- Stüdyo şekillerinde (shape_*) etiket yönü ve kapak UV'si ilk kez görülecek; ters/ayna ise `CreateShapePackage` → `Ring` işareti.

- İçe alınan FBX'te ön yüz yönü (Blender -Y → Unreal +X) ilk gerçek modelde doğrulanmalı; yanlışsa stüdyoya yön düzeltme eklenecek (G-007).
- `M_ProductGlass` saydam materyali ilk kez oluşturulacak; UE 5.8'de `BlendMode` erişimi uyarı verebilir.
- Oyun şimdilik tek etiket gösterir (dönem etiketleri G-011).

## Devam notu

Yok.

## Sıradaki adımlar

1. Mustafa: PET/teneke/kavanoz/kase türlerinden birer ürün üretip Oyuna ekle; etiket yönü, kapak ve malzeme yuvalarını gözle doğrula (G-017).
2. Ajan görsellerindeki bozuk küçük yazılar (ör. "Süteadürcanıya") içerik kalitesi sorunu: E promptunun 6b maddesi yakalar.
3. G-011 dönem etiketleri; G-004 git.
