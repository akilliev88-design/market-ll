# Blender modelleri ve büyük mağazalar

05.10.2026, Mustafa'nın isteğiyle masaüstündeki teslimler kaynaklarına dokunulmadan projeye kopyalandı. 132 yeni ekipman/aksesuar/manav modeli `/Game/Stores/Desktop` altında. Önceden oyunda bulunan ekipmanlar tekrar alınmadı. Kopyaların kaynağı `AssetInbox/Environment/Stores/Desktop/manifest.json` içinde.

Üç mevcut mağaza türü yeni Blender binalarıyla güncellendi:

| Tür | Ölçü | Açma |
|---|---|---|
| Küçük market | 22 × 19 m | `KUCUK_MARKET_GEZI.cmd` |
| Büyük süpermarket | 40 × 30 m | `BUYUK_MARKET_GEZI.cmd` |
| Hipermarket | 80 × 50 m | `HIPERMARKET_GEZI.cmd` |

WASD/fare ile gez. **1** cephe, **2** raflar, **3** manav, **4** kasa; **0** yürüyüşe dön, **Esc** çık. Bu açıcılar satış yapmayan, kampanya kaydını değiştirmeyen görsel inceleme sahnesidir. `Config/magazalar.json` içindeki asıl mağaza tanımları da yeni binaları ve ekipmanları kullanır; normal mağaza ziyareti gerçek şube stoklarını göstermeye devam eder.

Yerel `MAGAZA_GEZI.cmd` de yeni üç büyük düzeni açar. Önceki elle düzenlenmiş gezi taslakları `Saved/StoreBackups/BeforeHandmadeLarge_20261005_135402` altında korunur; mahalle taslağı değişmedi. Bu yerel gezi taslakları Git dışında kalır. Başka bilgisayarda aynı düzenleri kurmak için `Tools/sync_large_store_tours.py` çalıştırılabilir; mevcut dosyaları tarihli ayrı klasöre yedekler.

Yeni gondol rafları, ekmek rafı, dik dondurucular ve manav kasaları; büyük türlerde kompakt kasalar ve self servis kasalar yerleştirildi. Kasa POS cihazları ve depo aksesuarları eklendi. Her raf yüzü kendi kategorisinden ürün alır; kullanılabilir genişlik, raf yüksekliği ve derinlik sınırları denetlenir. İnceleme sahnesinde raflar yayımlanmış model/malzeme sahibi katalog ürünleriyle doldurulur. Modeli bulunmayan kategoriler boş kalabilir. Manavdaki meyve/sebzeler şimdilik görseldir; yeni ürünlerin fiyat, stok ve satış sistemi ayrıca katalog çalışması gerektirir.

Mustafa'nın ek isteğiyle tavan açık gri, taşıyıcı çelik orta gri, havalandırma galvaniz gri oldu. Oyun kurulumunda tavan malzemeleri değiştirilir ve armatürlerin yukarıya yayılan ışığı tavanı aydınlatır; yalnız aşağıya ışık veren düzenin siyah görünümü kaldırılır. Blender ekipman üreticisinin sonraki tavan üretimi de bu gri renkleri kullanır. Yeni el yapımı bina kaynaklarının çatı yüzeyi gri duvar malzemesidir; oyun görünümünde düz gri tavan uygulanır.

## Kaynakları düzenleme

Üç bina: `AssetInbox/Environment/Stores/HandmadeLarge/<id>/Source/<id>.blend`. Metre birimi; `EditableArchitecture` koleksiyonunda ayrı özgün parçalar saklıdır. Düzenlerken bu koleksiyonu göster, birleşik dışa aktarım modelini gizle. Çatı ayrı modeldir. Cam cephede gerçek giriş boşluğu ve arka mal kabul açıklığı vardır. UCX çarpışma parçaları kapıları kapatmamalıdır.

Masaüstü modellerinin kopyaları `Desktop/<id>/` altında. Geometri onarılanlarda ayrı `Source/Game_<id>.blend` bulunur. Kaynak teslim `.blend` dosyaları korunur. Kaplama ve ürün modelleri bina geometrisinden ayrıdır.

## Yeniden üretim / aktarım sırası

Unreal Editor ve oyun kapalı olmalı; her adım bitmeden sonrakini başlatma.

1. `Tools/prepare_desktop_store_assets.py`: teslimleri kopyala.
2. Blender arka planda `Tools/Blender/audit_desktop_store_assets.py -- <proje kökü>`: yüzey denetimi, onarım ve FBX dışa aktarım.
3. Blender arka planda `Tools/Blender/collect_desktop_materials.py`: kaynak Principled renk/pürüzlülüklerini metadata'ya yaz.
4. Unreal Python commandlet `Tools/import_desktop_store_assets.py`, ardından `Tools/repair_desktop_materials.py`: 132 varlık, kaynak renkleri ve UV dokuları. FBX'in bulamadığı gömülü dokular ikinci adımda açıkça içe alınır; eksik TextureSample bağlantıları kaldırılarak basit Principled yüzeyler yeniden kurulur. Kaynak UV dokuları, renk/pürüzlülük/metalik değerleri, ekran emisyonu ve cam saydamlığı kullanılır; gelişmiş Blender düğüm efektleri birebir taşınmaz. Tekrar çalıştırmak güvenlidir.
5. Blender `Tools/Blender/create_large_handmade_stores.py -- <proje kökü>`: üç bina ve ayrı çatılar.
6. Unreal Python commandlet `Tools/import_large_handmade_stores.py`: altı bina/çatı varlığı.
7. `Tools/arrange_large_handmade_stores.py`: üç mağaza düzeni ve ekipman yüzlerinin ölçüleri.
   Yerel gezi taslakları yeni düzenleri gizliyorsa `Tools/sync_large_store_tours.py` çalıştır; eski taslaklar yedeklenir.
8. `DERLE.cmd /q`, `TEST.cmd /q`, `SmokeTest.ps1`, `Tools/StoreTourTest.ps1`.
9. `Tools/LargeStoreReview.ps1`: mağaza başına sekiz görüntü, giriş/zemin ve sabit model kontrolü. `Tools/measure_store_temporal.py`: ardışık üç sabit kamera görüntüsünü karşılaştırır.

## Titreşim kontrolünün kapsamı

132 kaynak modelde aynı düzlemde üst üste gelen üçgenler tarandı; 9 modelde 99 yüz 0,8 mm ayrıldı, yeniden taramada çakışma kalmadı. UV, malzeme, yüz yumuşatma ve vertex grupları korunur. Bu onarım kaynak dosya kopyalarına uygulanır.

Oyun içi kontrol, sabit ekipman dönüşümlerinin değişmediğini ve fizik simülasyonunun kapalı olduğunu denetler. Ardışık karelerde 20/255 üzeri değişen piksel alanı ölçülür; yakın kasa çekimlerinin yanında iki hareketli kamera çekimi incelenir. Bu sınırlı sahne kontrolü bütün uzaklık/açı/ekran kartlarında titreşim olmayacağını garanti etmez. Rapor ve görüntüler `Docs/Images/LargeStores/20261005` altında saklanır.
