> **ARŞİV (06.10.2026):** Bu belge M69 öncesi kurguyu ve eski planı anlatır (2011, Lüleburgaz, "Miras Market", babadan kalan dükkân, hikâye bölümleri). **Güncel değildir; ajanlar buna göre kod ya da metin yazmasın.** Güncel kurgu `Docs/Kurgu/00_KURGU_KITABI.md`, kararlar `Docs/Kurgu/01_KARARLAR.md`, durum `Docs/Surec/DURUM.md`.

# Ürün Stüdyosu ve varlık teslim standardı

Durum: gelecekte geliştirilecek üretim aracı ve veri sözleşmesi. Aşağıdaki dosya ağacı örnektir; bugün çalışan importer veya hazırlanmış UV şablonu yoktur. Model/etiket görselleri bu görevde üretilmedi.

## 1. Temel ayrım

**Ürün:** Satılan şeyin kimliği ve ticari kuralları.
**Ambalaj modeli:** Şekil, fiziksel ölçü, malzeme bölgeleri, tutuş ve yerleşim.
**Marka paketi:** Görseller, ad, logo, dönem ve ülke sürümü.
**Stok partisi:** Gerçek alış maliyeti, miktar, tarih ve bulunduğu yer.

Bir süt kutusunun farklı marka baskıları aynı uygun şekil şablonunu kullanabilir. Aynı markanın farklı boyları, biçimleri veya içerikleri ise farklı ürün/ambalaj tanımları olabilir. Ürün kimliğine marka adını değişmez mantık olarak gömmeyiz; marka paketi değiştirmek mevcut stokları silmez.

## 2. Üç içerik hazırlama yolu

### A. Şablon ambalaj — tercih edilen başlangıç

Kullanıcı dikdörtgen kutu, silindir kutu, şişe, kavanoz, poşet, tepsi veya blister ailesinden seçer. Gerçek ölçüyü yazar. Editör, o geometri sürümünün yüz şablonlarını çıkarır. Kullanıcı bu şablona uygun dış görselleri yükler. Önizleme ve kabul kontrolünden sonra oyun paketi hazırlanır.

Şablon olması her şeklin aynı UV'yi kullanması demek değildir. Gable-top süt kartonu, düz dikdörtgen kutudan farklı kat izleri ve kapatma yüzeyleri gerektirir. Şişe omuzları ile düz silindir sargısı aynı projeksiyon değildir.

### B. Özel model + modelin kendi UV baskısı

Kullanıcı Meshy/Blender vb. kaynaktan modeli getirir. Ölçü, pivot, malzeme ve UV kontrol edilir. **O modelden** UV şablonu dışarı verilir; etiket/ambalaj görseli ona göre hazırlanır. Hazır dokulu modelde de marka yazısı, seam, ölçü ve performans kontrolü gerekir.

### C. Özel model + fotoğrafı belirli bölgeye yerleştirme

Editör yüzey bölgesini seçtirir; fotoğrafı kırpma, çevirme, ölçekleme ve dört köşe perspektif düzeltmesi sağlar. Düz bir kutu yüzü için kullanışlıdır. Kıvrımlı şişe veya buruşuk poşette tek fotoğraf görünmeyen tarafı doğru üretemez. Eksik yüzler kullanıcıdan istenir veya açıkça geçici görselle gösterilir. "Fotoğraf yükle, her kutu kusursuz kaplansın" garantisi verilmez.

## 3. Ürün Stüdyosu ekran akışı

1. **Kimlik:** ürün, marka, kategori, ülke/dönem, ticari satış birimi ve varyant ilişkisi.
2. **Şekil:** şablon seç veya özel model yükle; cm/mm birimini seç; ön yönü işaretle.
3. **Yüzler:** görüntüleri ilgili yüzlere at; seam ve hizayı 3B görünümde kontrol et.
4. **Malzeme:** kâğıt/plastik/cam/metal aileleri; parlaklık ve saydamlık önizlemesi.
5. **Fizik ve raf:** ölçüler, destek yüzeyi, izin verilen yönelimler, istif, ağırlık ve tutuş noktaları.
6. **Ticari veri:** koli adedi, tedarikçi bağları, raf ömrü, depolama, fiyatlandırma birimi.
7. **Doğrula:** birim, eksik dosya, UV, ters normal, poligon/malzeme sayısı, içerik kaynağı.
8. **Mağaza önizlemesi:** rafta 1/12/48 adet, elde, kasada ve uzakta görünüm.
9. **Yayımla:** yeni içerik sürümü; eski sürümü koru; etkilenen ürün ve kayıtları göster.

Model ve etiket değiştirilince ticari veriler silinmez. Yanlış içe aktarma ham dosyayı değiştirmez. Taslak ve kabul edilmiş içerik ayrıdır.

## 4. Araç nerede çalışacak?

İlk hedef **Unreal Editor içinde özel editör eklentisi/paneli**. İçerik burada içe alınır, doğrulanır ve paketlenmiş oyuna hazırlanır. `.uasset` ve shader/cook işlemleri bu üretim aşamasına aittir.

Paketlenmiş bir Windows oyununa rastgele FBX sürüklemek, Unreal Editor importer'ını kendiliğinden kullanılabilir yapmaz. Çalışma zamanında model/mod yükleme ayrı altyapı, format desteği ve paketleme tasarımı gerektirir. İleride bağımsız masaüstü Ürün Stüdyosu veya onaylı içerik paketlerini yükleyen oyun menüsü düşünülebilir; başlangıç teslimatı olarak vaat edilmez.

PNG tabanlı dinamik etiket değiştirmek, çalışma zamanında yeni geometri import etmekten farklı iştir. İlk etapta ikisini tek özellik gibi sunmayız.

## 5. Gelecekteki teslim klasörü

```text
AssetInbox/
  Products/
    prd_demo_box_001/
      manifest.json
      Source/
        reference_front.png
        reference_back.png
        reference_notes.md
        source.blend                 # varsa düzenlenebilir kaynak
        raw_generation.glb           # varsa Meshy ham çıktısı
      Mesh/
        SM_prd_demo_box_001.fbx       # onaylı UE giriş biçimi
      Labels/
        front.png
        back.png
        left.png
        right.png
        top.png
        bottom.png
      Textures/                      # hazır UV dokulu model yolunda
        T_prd_demo_box_001_BC.png
        T_prd_demo_box_001_N.png
        T_prd_demo_box_001_ORM.png
      QA/
        dimensions.txt
        preview_front.png
        preview_back.png
        preview_uv.png
        provenance.md
  Fixtures/
    fix_gondola_1000_001/
      manifest.json
      Source/
      Mesh/
      Textures/
      QA/
```

`Labels` altındaki altı yüz yalnızca uyumlu altı yüzlü şablon yoludur. Şişe için `wrap.png/cap.png`, özel model için o modele özgü `UVTemplate` ve atlas dosyaları kullanılır. Hepsini her ürün için zorunlu tutmayız. Dosya adları küçük/büyük harf açısından tutarlı, ASCII ve boşluksuz olur; oyuncuya görünen isimler Türkçe olabilir.

Ham dosyalar AssetInbox'ta saklanır; motorun Content klasörüne kontrolsüz kopyalanmaz. Kabul sonrası içerik araç tarafından uygun oyun paketine aktarılır. Bu klasörler bugün oluşturulmadı; üretim başladığında kullanılacak sözleşmedir.

## 6. Ortak geometri sözleşmesi

| Konu | Önerilen standart |
|---|---|
| Manifest ölçü birimi | Milimetre; alan adında `_mm` |
| Unreal içi ölçü | Santimetre; normalleştirme adımında tek dönüşüm |
| Nihai ürün yerel ekseni | +X ön yüz, +Y genişlik yönü, +Z yukarı |
| Boyut adları | Width = Y, Depth = X, Height = Z |
| Pivot | Satıştaki destek yüzeyinin alt merkezi; elde tutuş ayrı nokta |
| Konum | Kaynak doğrulama sahnesinde taban Z=0; model pivotu orijinde |
| Ölçek/dönüş | Normalize edilmiş kabul çıktısında ölçek 1,1,1 ve tanımlı yön |
| Görünür parça | İstenmeyen zemin, kamera, ışık, arka plan, fazladan kopya yok |
| Mesh | Geçerli normal yönleri, kontrol edilen triangulation, gereksiz iç yüzey yok |
| Collision | Basit kutu/kapsül veya birkaç convex parça; satış görselinden ayrı |
| Etkileşim | Grip, scan, support ve gerekirse hang noktaları |

Blender'daki metre/santimetre görünümü ile FBX dönüşüm ayarları birbirine karıştırılmaz. Exporter ve Unreal importer sürümünün birlikte çalıştığı **tek import profili** pilotta sabitlenir. Prompttaki "+X ön" ifadesi exporter'ın bunu doğru çevirdiğini kanıtlamaz: FRONT yazılı yön test modeli ve 100 mm ölçü testi zorunludur. 100 mm nesne Unreal'da 10 cm olmalıdır; ikinci kez 100 kat ölçekleme yapılmaz.

Epic'in FBX yolu UV, çarpışma ve malzeme desteği içerir; belgede FBX 2020.2 kullanıldığı belirtilir. Bu, Blender arayüzünde mutlaka "2020.2" seçeneği bulunacağı anlamına gelmez; gerçek ihracat/içe alma çifti test edilir. Tüm malzemelerin otomatik eksiksiz bağlanacağı varsayılmaz. [Epic FBX belgesi](https://dev.epicgames.com/documentation/en-us/unreal-engine/fbx-static-mesh-pipeline-in-unreal-engine).

## 7. UV ve yüz görsellerinin eşleşmesi

UV, düz görselin model üzerinde nereye geldiğini tanımlar. Modeli ve düz görseli birbirinden bağımsız üretip en son otomatik eşleştirmeye güvenmeyiz. Şablonun `template_id`, `template_version` ve UV yerleşim sürümü, görsel paketiyle eşleşmek zorundadır.

Tercih edilen yöntem: kullanıcı yüz görsellerini ayrı üretir; editör bu görselleri modelin sabit UV alanlarına **deterministik olarak** yerleştirip bir atlas oluşturur. AI'dan hassas atlas koordinatları çizmesini istemeyiz. Aynı geometriye farklı markalar uygulanabilir.

Örnek geometri: gerçek bir marka ölçüsü olduğu iddia edilmeyen **70 mm genişlik × 50 mm derinlik × 200 mm yükseklik düz kutu**. Kaynak yüzler 8 piksel/mm yoğunlukta:

| Yüz | Görsel ölçüsü |
|---|---|
| Ön / arka | 560 × 1600 px |
| Sol / sağ | 400 × 1600 px |
| Üst / alt | 560 × 400 px |

Bu örnek için önerilen 2048 × 2048 atlas içerik dikdörtgenleri, **PNG üst-sol başlangıç koordinatıyla** şöyledir:

| Yüz | X | Y | W | H |
|---|---:|---:|---:|---:|
| Ön | 16 | 16 | 560 | 1600 |
| Arka | 592 | 16 | 560 | 1600 |
| Sağ | 1168 | 16 | 400 | 1600 |
| Sol | 1584 | 16 | 400 | 1600 |
| Üst | 16 | 1632 | 560 | 400 |
| Alt | 592 | 1632 | 560 | 400 |

Aralarda 16 px boşluk vardır; mip taşmasını azaltmak için boşluklara komşu kenar rengi yayılır. Alt mip seviyelerinde okunabilirlik ayrıca denenir. Bu yalnızca bu ölçülü kutuya ait taslak UV planıdır. UV'nin V yönü, arka/üst yüzün dönüşü ve ayna durumu şablonda açık dönüşüm bilgisiyle tutulur; tabloya bakarak tüm model yüzlerini aynı yönde yapıştırmayız.

Boyut değiştiğinde kaynak oranları değişir; etiket zorla esnetilmez. Yazı ve logoların oranı korunur. Şablon profili uygun atlası yeniden paketler veya kullanıcıdan yeni baskı ister.

Özel modelde önemli baskı bölgelerinde üst üste/aynalanmış UV kabul edilmez. Teknik parçaların bilinçli UV paylaşımı ayrı işaretlenebilir. Baskı dışı generic malzemeyi paylaşmak mümkündür.

## 8. Doku ve malzeme

- Baskı/renk: PNG veya TGA; oyun Base Color dokusu renk uzayı sRGB. Fotoğraf ışığı/gölgesi mümkün olduğunca baskıdan ayrılır.
- Normal: Unreal için doğrulanmış DirectX yönü; veri dokusu, sRGB kapalı. OpenGL gelen normalin yeşil kanalı gerektiğinde dönüştürülür ve ışık testiyle doğrulanır.
- ORM paketinde R=ambient occlusion, G=roughness, B=metallic; veri dokusu, sRGB kapalı. Bu **proje sözleşmesidir**, her aracın varsayılanı değildir.
- Roughness malzemenin kendisidir; tüm ambalajı ayna gibi yapmak yerine kâğıt/plastik/folyo farkı tanımlanır.
- Saydam cam/plastik, label ve içerik görünümü ayrı çözüm gerektirebilir. Uzak raflarda daha ucuz görünüm kullanılır.
- Fiyat ve son kullanma tarihi sabit ambalaj görseline rastgele yazdırılmaz. Fiyat raf etiketi/UI'dır; parti tarihi değişken bilgi olarak ayrı katmana aittir.
- Yapay zekâdan gerçek barkod doğruluğu beklenmez. Oyun taraması ürün kimliği/metaverisine dayanır; görünür kod gerekiyorsa kontrollü üretilir. Gerçek GTIN iddiası doğrulanmadan yapılmaz.

Genel aday bütçe: basit ambalaj 100–1.500 üçgen, şişe/karmaşık ambalaj 500–4.000, raf modülü 1.000–10.000. Yakın çekim özel varlıklar istisna olabilir. Bunlar önerilen başlangıç aralıklarıdır; performans garantisi veya tüm ürünlere tek zorunlu limit değildir.

Çok tekrarlanan ürün için tek/az malzeme hedeflenir. Raf görüntüsünde ayrı Actor ve fizik gövdesi yerine örnekleme kullanılabilir; ele alınan ürün etkileşimli nesneye dönüşür. Aynı mesh farklı materyallerle kullanıldığında hepsinin tek draw call olacağı varsayılmaz. Atlas/texture-array/instance verisi seçenekleri gerçek sahnede ölçülür.

Doku hedefi rol bazlıdır: küçük raf görünümü 512–1024, okunabilir ambalaj 1024–2048, büyük yakın çekim gerektiğinde 4096. Her SKU'ya 4K vermek başlangıç standardı değildir. Kaynak görseller daha yüksek çözünürlükte arşivlenebilir.

## 9. Taslak manifest örneği

Bu JSON bir planlama örneğidir; mevcut prototip tarafından okunmaz. Özel modellerde template yerine custom-mesh profili seçilir.

```json
{
  "schema_version": 1,
  "product_id": "prd_demo_box_001",
  "brand_pack_id": "brand_user_reference_01",
  "display_name": "Örnek kutulu ürün",
  "sale_unit": "each",
  "category_id": "grocery.dry",
  "package": {
    "template_id": "box_rect",
    "template_version": 1,
    "uv_layout_id": "box_70_50_200_v1",
    "width_mm": 70,
    "depth_mm": 50,
    "height_mm": 200,
    "mass_g": 500,
    "front_axis": "+X",
    "up_axis": "+Z",
    "pivot": "bottom_center"
  },
  "placement": {
    "support_mode": "standing",
    "allowed_yaw_degrees": [0, 180],
    "stack_limit": 2,
    "required_capabilities": ["ambient", "flat_support"]
  },
  "supply": {"case_units": 12},
  "labels": {
    "front": "Labels/front.png",
    "back": "Labels/back.png",
    "left": "Labels/left.png",
    "right": "Labels/right.png",
    "top": "Labels/top.png",
    "bottom": "Labels/bottom.png"
  },
  "availability": {"start_year": 2011, "countries": ["TR"]},
  "provenance_file": "QA/provenance.md"
}
```

Raf ömrü, sıcaklık, gerçek marka ve ülke uygunluğu bu örnekte tahmin edilmedi. İlgili kategori için bu alanlar zorunlu olduğunda kullanıcı/ürün araştırmasıyla doldurulur. Örnek, gerçek bir ürünün onaylanmış kaydı değildir.

## 10. Kabul ve hata mesajları

| Kontrol | Reddedilecek/incelemeye düşecek durum |
|---|---|
| Kimlik | Aynı ID ile fark edilmeden farklı ürünün üzerine yazılması |
| Ölçü | Sıfır/negatif ölçü, birimsiz geometri, manifest-bounds farkı |
| Yön/pivot | Ürün rafta yan yatıyor, tabanı zeminin altında |
| UV | Yanlış şablon sürümü, önemli bölgede ayna/çakışma, kesilen logo |
| Dosya | Eksik doku, bozuk dosya, taşan yol, aşırı büyük doku |
| Malzeme | Normal/ORM renk uzayı yanlış, beklenmeyen saydamlık |
| Oyun verisi | Adet/kilogram belirsiz, koli dönüşümü eksik, uyumsuz raf yeteneği |
| Yakın/uzak görünüm | Yazı oranı bozuk, seam, mip sızıntısı, parıldama |
| Performans | Temsilî dolu rafta hedefi aşan bellek/draw-call/geometri maliyeti |

Kontrol raporu açıklamalı olmalı: "Ön görsel oranı 0,50; yüz oranı 0,35. Kırp, kenar ekle veya doğru şablonla yeniden üret." Rastgele otomatik düzeltmeyle yanlış ambalajı sessizce yayımlamaz.

## 11. Üretim kaydı ve güncelleme

Kaynak dosya, kullanılan araç/model sürümü, prompt sürümü, referans görsellerin kaynağı, kullanıcı düzenlemeleri, içerik paketi ve kabul tarihi saklanır. Kişisel kullanımın her türlü kullanım hakkını otomatik çözmüş olduğu varsayılmaz; bu belge hukuki izin kararı vermez. Yayınlama kararı olursa marka paketi ve diğer içeriklerin kullanım koşulları ayrıca değerlendirilir.

Görsel güncellemeler kimliği koruyabilir; fiziksel ölçü güncellemesi raf kapasitesini etkileyebilir. Yeni sürüm, eski kampanyadaki mevcut yerleşimi kontrol etmeden raf kapasitesini sessizce azaltmaz. İçerik eksikse anlaşılır geçici temsil ve uyarı kullanılır; stok/kasa silinmez.
