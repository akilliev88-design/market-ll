# Oturum günlüğü

En yeni giriş en üstte. Biçim: tarih — ajan — başlık, ardından **Yapılan**, **Doğrulama**, **Sıradaki**.

## 27.09.2026 — Codex — Hazır ambalaj ajan şablonları ve özel model düzeltmesi (Stüdyo v1.7)

**Yapılan**
- Hazır ambalaj seçiliyken çalışan **Şablonları oluştur** düğmesi eklendi. Kutu/poşet için `acilim_sablonu.png`; yuvarlak ambalaj için `label_sablonu.png` ve gerekiyorsa `kapak_sablonu.png` seçili dönemin teslim klasörüne yazılır.
- Kutu kılavuzu yüzlerin kesin yerini ve ön yüzü; etiket kılavuzu ön merkez ile %3 dikiş güvenlik alanlarını; kapak kılavuzu dairesel baskı sınırını gösterir.
- A1 ve B2 promptları, kullanıcının bu PNG'leri ajana yüklediğini varsayacak şekilde güncellendi. Ajan aynı piksel tuvalini korur ve kılavuz renk/işaretlerini temiz baskı dosyasına taşımaz.
- Özel modeller için ölçek, Pitch/Yaw/Roll ve pivot/raf XYZ alanları eklendi. Değerler `visual.transform` içinde geriye uyumlu saklanır; önizleme ile oyun rafı aynı dönüşümü kullanır.

**Doğrulama**
- `DERLE.cmd /q`: GEÇTİ.
- `TEST.cmd /q`: GEÇTİ 10/10; `ReadyPackageTemplates` üç PNG'nin çözülmesini, kesin boyutlarını ve prompt bağlantısını kontrol etti; katalog dönüşüm değerleri round-trip testinden geçti.
- Üretilen kutu, sarma etiket ve kapak PNG'leri gözle incelendi; panel sırası, ön merkez, dikiş alanları ve kapak dairesi doğru.
- `SmokeTest.ps1`: GEÇTİ; raf doldurma, sipariş, kasiyer, 4 satış, gün kapama ve disk kayıt/yükleme.

**Sıradaki**
- G-017 hazır PET/teneke/kavanoz/kase şekillerini gerçek ürün görselleriyle elle doğrula.
- G-011 dönem etiketlerini katalog ve oyun yılına bağla.

## 27.09.2026 — Codex — Özel model UV kılavuzu (Stüdyo v1.6)

**Yapılan**
- İçe alınmış özel modellerde görünen **UV kılavuzu oluştur** düğmesi eklendi.
- Seçili modelin `Etiket`/`Label` malzeme yuvasındaki UV0 üçgen kenarları, çeyrek ızgaralı 2048 × 2048 PNG'ye çizilir.
- Çıktı `Uretim/<ürün>/model/uv_sablon.png` yoluna yazılır ve klasör açılır; etiket hazırlayan ajana doğrudan referans verilebilir.
- `UvTemplateExport` otomasyon testi eklendi ve `Test.ps1` minimum eşiği 9 teste çıkarıldı.

**Doğrulama**
- `DERLE.cmd /q`: GEÇTİ.
- `TEST.cmd /q`: GEÇTİ 9/9.
- Testin ürettiği 2048 px PNG gözle incelendi: altı UV adası, üçgen kenarları, 0–1 sınırı ve ızgara doğru.

**Sıradaki**
- G-007: içe alınmış modeller için ölçek, yön ve pivot düzeltme alanları.
- G-017: PET, teneke, kavanoz ve kase şekillerinin oyun içinde görsel doğrulaması.

## 27.09.2026 — Codex — Pazar payı, gerçek kuyruk sırası ve ilk Git sürümü

**Yapılan**
- Ziyaretçi gelmeyen bir günde memnuniyetin sıfır sayılıp yerel pazar payını düşürmesi kaldırıldı; böyle günlerde pay korunur.
- Kasa kuyruğuna varış bileti eklendi. Yürümekte olan müşteri artık daha önce kasaya ulaşmış müşterinin önüne geçemez; ödeme daima en küçük varış biletinden alınır.
- `QueueArrivalOrder` otomasyon testi eklendi; `Test.ps1` minimum başarı eşiği 8 teste çıkarıldı.
- Unreal üretim klasörlerini dışarıda bırakan Git düzeni ve uasset/umap/görsel/model dosyaları için Git LFS kuralları hazırlandı.

**Doğrulama**
- `DERLE.cmd /q`: GEÇTİ.
- `TEST.cmd /q`: GEÇTİ 8/8; `QueueArrivalOrder` dahil, başarısız/çalışmayan test yok.
- `SmokeTest.ps1`: GEÇTİ; oyuncu, raf doldurma, sipariş, kasiyer, müşteri satışları, gün kapama ve disk kayıt/yükleme.
- `Tools/escape_unicode.py --check`: GEÇTİ.

**Sıradaki**
- G-017 görsel şekil doğrulamaları; ardından G-006 özel model UV şablonu ve G-007 ölçek/pivot araçları.

## 27.09.2026 — Codex — İlk kutu ürününün oyun içi görsel doğrulaması

**Yapılan**
- Mustafa'nın Ürün Stüdyosu ile oyuna eklediği `milk_1l` ekran görüntüsü incelendi.
- Kutu hattı için G-005 ve milk_1l yeniden yayımını bekleyen G-014 tamamlandı olarak işaretlendi.

**Doğrulama**
- Ön yüz raftan dışarı bakıyor ve yazılar aynalanmamış.
- Sol yan panel ile üst panel doğru yüzlerde; üstteki kapak işareti doğru konumda.
- Ürünler dik, raf düzlemine oturuyor; HUD'daki 24 raf stoku sahnedeki 24 birimle uyumlu.

**Sıradaki**
- G-017: PET, teneke, kavanoz ve kase şekillerini aynı şekilde oyunda gözle doğrula.

## 27.09.2026 — Codex — v1.5 derleme incelemesi ve katalog uyumlu smoke testi

**Yapılan**
- Son başarısız derlemenin günlüğü incelendi. C++ derleme adımları geçmiş; bağlantı aşaması açık Unreal Editor'ın `UnrealEditor-MirasMarket.dll` ve `UnrealEditor-MirasMarketStudio.dll` dosyalarını kilitlemesi nedeniyle `LNK1104` ile durmuştu.
- Editor kapandıktan sonra aynı kaynaklar temiz biçimde derlendi; kaynak derleme hatası çıkmadı.
- Katalog v2 ilk aktif ürünü ve koli adedini değiştirdiği için smoke testindeki sabit `12 adet / 329,60 TL / 209,60 TL` beklentileri yanlış alarm veriyordu. Sipariş adedi, ürün maliyeti ve işe alım öncesi kasa üzerinden hesaplanan dinamik beklentilerle değiştirildi.
- `DURUM.md` ve `GOREVLER.md` gerçek doğrulama sonuçlarıyla güncellendi.

**Doğrulama**
- `DERLE.cmd /q`: GEÇTİ; runtime ve Studio modülleri bağlandı.
- `TEST.cmd /q`: GEÇTİ 7/7; başarısız/çalışmayan test yok.
- `SmokeTest.ps1`: GEÇTİ; oyuncu, raf doldurma, sipariş, kasiyer, 5 satış, gün kapama ve disk kayıt/yükleme.
- `Tools/escape_unicode.py`: çalıştırıldı; C++ kaynaklarında ASCII dışı karakter yok.

**Sıradaki**
- G-005 ve G-017: Ürün Stüdyosu'nda kutu ile PET/teneke/kavanoz/kase örneklerini gözle doğrula; etiket yönü, kapak ve malzeme yuvalarına bak.

## 27.09.2026 — Claude — Hazır ambalaj kütüphanesi + stüdyonun ürettiği şişe/teneke/kavanoz şekilleri (stüdyo v1.5)

**Yapılan**
- Mustafa: "ölçü girmek yerine hazır ambalaj seçilsin; kapak ne olacak?" Kararlar (Mustafa seçti): hazır ambalaj kütüphanesi + 3B şekli stüdyo üretsin.
- `Config/ambalajlar.json` (56 kayıt). `StudioBackend`: `LoadPresets`, `ApplyPreset`, `SuggestPreset`, `EnsurePresetPackage`, `CreateShapePackage` (döndürülmüş şekil; Etiket/Cam|Govde/Kapak yuvaları), parça renkleri (`FindColor/WithColor`), yayımda ürüne özel renk MI'ları.
- Katalog: `package.preset`, `package.colors` (MarketEconomy/ProductCatalog/test). 97 ürüne hazır ambalaj ve renk atandı (ölçüler ambalajın ölçüsüne çekildi; kullanıcının products.json'undaki son değişiklikler korundu: sutas_sut_1l pasif).
- Stüdyo: AMBALAJ bölümü (tür filtresi + liste + önerilen + özel ambalaj), karton kutuda "Üstte kapak var" anahtarı, PARÇA RENKLERİ, eski 3B kart listesi kaldırıldı; kutucuklar "Etiket" / "Kapak (üstten)".
- Kapak: karton kutuda A1 promptu kapağı ÜST paneline üstten çizdirir (dönemde yoksa çizme). Şişe vb.: 3B kapak; B2 yeniden yazıldı (label.png = π·çap × bant, kapak.png 512×512 üstten). Poşet artık A1/A2 ile (düz kutu).

**Doğrulama**
- Derlenmedi. Şekil profilleri Python eşleniğiyle çizilip kontrol edildi; şablon yer tutucuları kontrol edildi.

**Sıradaki**
- DERLE + TEST; stüdyoda PET/teneke/kavanoz/kase birer ürün seçip şekle ve etiket yönüne bak (u=0,5 önde olmalı).

## 27.09.2026 — Claude — Açılım otomatik panel bulma + promptlar sadeleşti (stüdyo v1.4)

**Yapılan**
- Gemini açılımı panel adları ve sahte koordinatlarla ("x=512-1272") geldi; stüdyo oranla kestiği için yüzler kaydı. `SplitNet` artık (json yoksa) açılımı düz zeminden kendisi buluyor: köşelerden zemin rengi, en büyük bağlı şekil (dışarıdaki yazılar yok sayılır), orta sıra = en geniş satırlar, ÖN sütunu = ÜST/ALT panelinin yeri, SAĞ/ARKA sınırı koyu çizgiye yapıştırılır. Gemini görselinde Python eşdeğeriyle doğrulandı.
- Yüz oranı esnetme sınırı %15 → %35 (üreticiler oranı tutturamıyor; bant yerine esnetme).
- Önizlemedeki "birden fazla yön ışığı" uyarısı: dolgu ışığına `ForwardShadingPriority = -1`.
- Mustafa: "ölçüyü ajana tahmin ettirme, görsele ölçü yazmasın." A1 yeniden yazıldı (koordinat listesi yerine panel piksel boyutları, 4×3 düzen, "görsele KESİNLİKLE yazma" bölümü, kanat/kapak işareti yasak, ince panel çizgisi istenir, acilim.json adımı kaldırıldı). A2/B1/C1/D/_genel/E: ölçü sabit, araştırma/rapor yok. Yeni yer tutucular: ACILIM_ON_PX, ACILIM_YAN_PX, ACILIM_UST_PX.

**Doğrulama**
- Derlenmedi. `DERLE.cmd` + `TEST.cmd` bekliyor.

**Sıradaki**
- Mustafa derleyip Sütaş açılımını yeniden yüklesin; yüzler doğru mu bakılsın.

## 27.09.2026 — Claude — Ürün kataloğu + ürüne göre prompt üretimi (stüdyo v1.3)

**Yapılan**
- Mustafa'nın isteğiyle eski katalog (Sütaş denemesi dahil) atıldı; `Tools/katalog_olustur.py` ile 97 ürünlük katalog kuruldu. 6 ürün oyunda (sutas_sut_1l, coca_cola_1l, ulker_potibor, barilla_spagetti_500g, caykur_rize_turist_500g, ariel_toz_4kg), gerisi hazırlıkta. Ölçüler tahmini (`estimated`).
- Katalog: `active`, `brand`, `package{type, widthMm, depthMm, heightMm, diameterMm, labelHeightMm, parts, notes, estimated}`. 24 sınırı yalnız aktif ürünlere uygulanır; oyun pasifleri yüklemez.
- Stüdyo: Oyunda/Hazırlık/Ambalaj listeleri (kategoriye göre gruplu), ambalaj bilgisi alanları, DIŞ ÜRETİM bölümü (dönem + prompt seçimi, önizleme, Kopyala, Teslim klasörünü aç), Kaydet / Oyuna ekle / Oyundan çıkar; kutu için ürün ölçüsüyle tek tıkla şablon; model içe almada ürünün model klasörü.
- Prompt şablonları `Docs/Uretim/Sablonlar/` (A1, A2, A3, B1, B2, C1, C2, D, E + ortak parçalar); eski md promptlar stüdyoya yönlendiren nota çevrildi.
- Oyun: raf üstü 3B yazılarda Türkçe karakterler ASCII'ye çevrilir (mesafe alanı fontunda glif yok).

**Doğrulama**
- Şablonlardaki tüm yer tutucuların BuildPrompt'ta karşılığı var (betikle kontrol). Derlenmedi.

**Sıradaki**
- Derle + test; ilk ürünle uçtan uca akış.

## 27.09.2026 — Claude — İlk gerçek ürün denemesi, stüdyo v1.2

**Yapılan**
- Test çökmesi düzeltildi (testte diziye kendi elemanını ekleme). Sonuç: 7/7 GEÇTİ.
- Mustafa Gemini'den gelen Sütaş açılımını yükledi. Görsel ölçü çizgili/yazılı/boşluklu sunum çizimiydi; oranla kesim kaydı. Çözüm: `acilim.json` / `<görsel>.json` panel dikdörtgenleri (`SplitNet`), inbox'a `net.json` olarak kopyalanır. milk_1l için sınırlar koyu çerçeve çizgilerinden ölçülüp yazıldı ve temiz açılım olarak doğrulandı.
- Yüz oranı ≤%15 farklıysa esnetip oturtma (boş bant yerine).
- Önizleme ışığı kameranın tarafına alındı + karşı dolgu ışığı (ön/sol yüzler siyahtı).
- Promptlar: A1'e yasaklar listesi ve isteğe bağlı acilim.json adımı; yeni A3 (panel sınırı çıkarma, Codex/Claude için); E'ye yazı kalitesi maddesi.

**Doğrulama**
- v1.2 derlenmedi.

**Sıradaki**
- Derle, milk_1l'i yeniden yayımla, oyunda bak (G-005).

## 27.09.2026 — Claude — Dış üretim prompt paketi, stüdyo v1.1

**Yapılan**
- Stüdyo v1 derlemesi iki denemede düzeldi: (1) oyun modülü başlıkları dışa açıldı (`PublicIncludePaths`), (2) köprü aktarımı iki dosyayı eski hâliyle yazmıştı; yeniden yazılıp geri okunarak doğrulandı. Sonra derleme GEÇTİ.
- `Docs/Uretim/`: kısa, adımları eksiksiz, kopyala-yapıştır promptlar — A1 kutu açılımı (tek görsel), A2 6 ayrı yüz, B1/B2 şişe-kavanoz-teneke model + etiket, C1/C2 poşet model + baskı, D dönem araştırması, E teslim kontrolü. Dönemler 2011/2018/2025 (gerçek) ve 2033 (kurgu gelecek).
- `Docs/Uretim/MARKA_VE_URUN_LISTESI.md`: 2011–2026 önemli sahiplik/logo olayları (kaynaklı) ve ~100 örnek ürün, ambalaj tipiyle.
- Stüdyo v1.1: kutuda "Tek görsel (açılım)" kutucuğu, `SplitNet` ile otomatik bölme; modellerde yuva tanıma (Etiket/Cam/Kapak/Govde), `malzeme.json` okuma, `M_ProductSolid` ve saydam `M_ProductGlass`; ürün başına kapak/gövde dokusu.
- Katalog: `visual.material` → `visual.materials` (yuva dizisi, eski alan okunur). Oyun raf görünümü yuva bazlı materyal uygular. Testler güncellendi.
- `TEST.cmd`; `Planlama/07` arşiv notu.

**Doğrulama**
- v1.1 derlenmedi (bkz. DURUM).

**Sıradaki**
- Derle + test; ilk gerçek ürünle uçtan uca deneme; G-011 dönem etiketleri.

## 27.09.2026 — Claude — Ürün Stüdyosu v1 ve devir sistemi

**Yapılan**
- Ajanlar arası devir düzeni: `AGENTS.md`, `CLAUDE.md`, `Docs/Surec/` (DURUM, GOREVLER, GUNLUK).
- Katalog v2 (`ProductCatalog.*`): isteğe bağlı `category`, `caseUnits`, `visual.package`, `visual.material`; en fazla 24 ürün; atomik yazma + `.bak`.
- Oyun: sabit 6 ürün kaldırıldı; mağaza derinliği satır sayısına göre büyüyor; rafta her birim ayrı görünür ve satışla azalır; stüdyo ürünleri gerçek kutu + etiketle görünür; koli adedi ürüne göre; HUD stok listesi 6 satırda kayar.
- Kayıt: yükleme artık katalogla id üzerinden uzlaşıyor (yeni ürün boş rafla gelir, kaldırılan ürün düşer). `IsValidFor` eski davranışıyla duruyor.
- Yeni editör modülü `MirasMarketStudio`: Tools > Ürün Stüdyosu ve araç çubuğu düğmesi. Tesla esintili koyu arayüz, turntable önizleme, kutu şablonu (ölçüden mesh + UV), model içe alma (FBX/OBJ/GLB), yüz görselleri → atlas, tek UV etiketi, doğrulama listesi, Oyuna ekle / Katalogdan çıkar.
- `DERLE.cmd`, `STUDYO.cmd`, `Tools/escape_unicode.py`; `Test.ps1` eşiği 7 test.
- 3 yeni test: `Catalog.ParseAndSerialize`, `Catalog.BoxLayout`, `Save.ReconcileCatalog`.

**Doğrulama**
- Bkz. `DURUM.md` > Doğrulama durumu (bu oturumun derleme sonucu orada).

**Sıradaki**
- G-005 gözle kontrol, G-004 git.

## 27.09.2026 — (önceki oturum) — v0.1 prototip ve Planlama v0.2

- Unreal 5.8.3 C++ prototip, 4 otomasyon testi, smoke test, görsel kontrol. Ayrıntı: `Docs/DOGRULAMA.md`.
- `Docs/Planlama/` v0.2 tasarım paketi (kod değişmedi).
