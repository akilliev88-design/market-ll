# Ürün Stüdyosu — kullanım ve dosya standardı

Ürün Stüdyosu, dışarıda hazırladığın kutu modelini ve etiket görselini oyuna eklemeni sağlayan Unreal Editor penceresidir. Dışarıdaki ajana verilecek hazır metinler: [Docs/Uretim/00_BASLA_BURADAN.md](Uretim/00_BASLA_BURADAN.md). Dosyaları kopyalama, doku ve materyal oluşturma, kataloğu güncelleme işlerini kendisi yapar.

## Açmak

- `STUDYO.cmd` dosyasına çift tıkla. Editör açılır ve stüdyo kendiliğinden gelir.
- Editör zaten açıksa **Tools > Ürün Stüdyosu** menüsünü ya da üst araç çubuğundaki düğmeyi kullan.
- Kod değiştiyse önce `DERLE.cmd` çalıştır (editör kapalıyken).

## Ekran

| Bölüm | İşlev |
|---|---|
| Sol | **Ürünler** / **Ambalajlar** listesi. Yeşil çerçeve seçili olanı gösterir; **3B** etiketi ürünün gerçek kutusu olduğunu belirtir. |
| Orta | Dönen 3B önizleme. Görsel yükleyince anında güncellenir. Sağ fare tuşuyla bakış, tekerlekle yakınlaşma. |
| Alt | **Etiket** kutucukları. Kutuda: **Tek görsel (açılım)** + 6 yüz. Modelde: **Etiket (UV)**, varsa **Kapak** ve **Gövde**. Kutucuğa tıkla → görsel seç; × ile kaldır. |
| Sağ | Ürün bilgisi, fiyat, ambalaj seçimi, kontrol listesi ve **Oyuna ekle**. |

## Hazır ambalaj kütüphanesi (v1.5)

`Config/ambalajlar.json`: 56 hazır ambalaj (kutu, poşet, PET/cam şişe, teneke, sprey, kavanoz, kase). Ürün seçilince sağdaki **AMBALAJ** bölümünde türe göre listelenir; birine tıklamak ürünün türünü, ölçülerini, parçalarını ve varsayılan renklerini doldurur ve 3B şekli üretir.

- **Kutu / poşet** → `box_WxDxH` kutu şablonu (poşet düz kutu olarak). Açılım (A1) ile etiketlenir. `topCap: true` ya da "Üstte kapak var" → kapak, ÜST paneline üstten görünüşüyle çizilir (3B kapak yok).
- **Yuvarlak ambalajlar** → `shape_<id>` döndürülmüş şekil; yuvalar `Etiket`, `Cam` ya da `Govde`, `Kapak`.
  - Etiket UV'si 0–1 bant: yatay orta (u = 0,5) ürünün önü (+X), sağa doğru izleyicinin sağı; üst kenar etiketin üstü. `label.png` = π·çap × etiket bandı (8 px/mm, en çok 4096).
  - Kapak ve diskler üstten izdüşüm UV'si alır: kare `kapak.png` kapağın üstüne, kenar rengi yan yüzüne gelir.
  - Şekiller: `sise`, `sise_genis`, `sikma`, `teneke`, `aerosol`, `kavanoz`, `kase` (`StudioBackend.cpp` → `BuildProfile`).
- **Parça renkleri**: ürün kataloğunda `package.colors` = `"Cam=2B1A12/0.85;Kapak=E30613"` (HEX, isteğe bağlı saydamlık). Yayımlarken ürüne özel `MI_<ürün>_<parça>` (ambalajın parça materyalinin çocuğu) oluşturulur. Görsel yüklenen parçada görsel kazanır.
- Bir hazır ambalajın ölçüsünü değiştirirsen **id'sini de değiştir**; eski 3B şekil diskte kalır ve kullanan ürünler bozulmaz.

## Yeni ürün: kutu şablonu yolu (önerilen)

1. **+ Yeni ürün**.
2. Sağda **Genişlik / Derinlik / Yükseklik (mm)** gir, **Kutu şablonu oluştur**. Aynı ölçü bir kez oluşturulur ve bütün ürünler paylaşır: aynı kutu, farklı etiket.
3. Görselleri yükle. İki yol var:
   - **Tek görsel (açılım)** — 6 yüz bir arada (prompt A1). Stüdyo, kutu ölçüsüne göre görseli yüzlere böler. Kutucuğun altında önerilen piksel yazar.
   - **Yüz yüz** — 6 ayrı görsel (prompt A2). Yalnız **Ön** zorunlu.
   İkisi birlikte kullanılırsa tek tek yüklenen yüz, açılımdakinin yerine geçer.
4. Ürün adı, kurgu adı (F8 ile görünür), kategori, alış ve satış fiyatı, koli adedini yaz. Ürün kimliği addan otomatik türetilir.
5. Kontrol listesinde kırmızı madde kalmayınca **Oyuna ekle**.
6. Oyunu (yeniden) başlat: ürün kendi rafında, gerçek kutusu ve etiketiyle görünür.

### Açılım düzeni (A1)

```text
            |   ÜST    |          |          |
   SOL      |   ÖN     |   SAĞ    |   ARKA   |
            |   ALT    |          |          |
```

Sütun genişlikleri D, G, D, G; satır yükseklikleri D, Y, D (mm oranında). Boş köşeler kullanılmaz. Oran %3'ten fazla saparsa stüdyo uyarır ama yine orantılı keser.

Görsel ölçü çizgili, yazılı ya da paneller arası boşluklu geldiyse (görüntü üreticilerinde sık olur) oranla kesim kayar. Çözüm: görselin yanına her panelin piksel sınırını veren `acilim.json` (ya da `<görsel adı>.json`) koy — prompt **A3** bunu Codex/Claude'a yazdırır. Stüdyo dosyayı kendisi bulur ve panelleri oradan keser.

Yüz görselinin oranı kutu yüzünden en fazla %15 farklıysa stüdyo görseli yüze tam oturtur (hafif esnetir); daha büyük farkta oranı korur ve kalan kenarı görselin kenar rengiyle doldurur.

### Yüz görselleri

- Görsel, yüzün **dışarıdan bakınca** görünen hâli olmalı. Ayna çevirme yapma.
- Ön/arka/sol/sağ yüzlerin üst kenarı kutunun üstüdür. Üst yüzde görselin **alt kenarı ön tarafa**, alt yüzde **üst kenarı ön tarafa** bakar (standart kutu açılımı).
- Sağ/sol, kutuya önden bakan kişinin sağı ve soludur.
- Oran en fazla %15 farklıysa görsel yüze oturtulur; daha büyük farkta esnetilmez, boş kenar görselin kenar rengiyle doldurulur. Her iki durumda stüdyo uyarır.
- PNG saydamlığı desteklenir; saydam alanlar kenar rengiyle birleşir.
- Boş bırakılan yüzler ön yüzün kenar rengiyle doldurulur.

Örnek 70 × 50 × 200 mm kutu için önerilen ölçüler: ön/arka 560 × 1600, sağ/sol 400 × 1600, üst/alt 560 × 400 px (Planlama 03 §7 ile aynı).

## Yeni ürün: kendi modelin

1. **Model içe al…** ile FBX, OBJ, GLB veya glTF seç (prompt B1/C1 çıktısı: `Uretim/<ürün>/model/model.fbx`). Model tek parça ve iskeletsiz olmalı; 1 birim = 1 cm.
   - Stüdyo malzeme yuvalarını **adından** tanır: `Etiket`, `Cam`, `Kapak`, `Govde`.
   - Aynı klasörde `malzeme.json` varsa okur: cam rengi/saydamlığı, kapak ve gövde rengi. `Cam` yuvası json olmasa da otomatik saydam yapılır. Böylece etiketin kaplamadığı yerler gri kalmaz.
2. Önerilen yön: **+X ön yüz, +Z yukarı**, pivot tabanın ortası. Farklıysa stüdyo modeli önizlemede ortalar; rafta da tabanına oturtur. Ön yüzün yönü doğru değilse modeli Blender'da düzelt.
3. **Etiket (UV)** kutucuğuna, modelin UV açılımına göre hazırlanmış tek görseli yükle. Boş bırakırsan modelin kendi dokusu kullanılır. Modelde `Kapak`/`Govde` yuvası varsa o kutucuklar da çıkar; baskılı kapak/gövde görseli isteğe bağlıdır (yoksa malzeme.json rengi kullanılır).
4. Bilgileri doldur, **Oyuna ekle**.

## Mevcut ürünü değiştirmek

Soldan ürünü seç, alanları veya görselleri değiştir, **Değişiklikleri oyuna uygula**. Ürün kimliği kilitlidir, çünkü kayıtlı oyunlardaki stok bu kimliğe bağlıdır. Ambalaj değişirse eski etiket yeni şekle uymaz; yeni görsel yükle.

**Katalogdan çıkar** ürünü yalnızca katalogdan çıkarır; dosyaları silmez. Kayıtlı oyun yüklenince o ürünün stoğu düşer.

## Stüdyonun arkada yaptıkları

| Ne | Nereye |
|---|---|
| Ham görsel ve model kopyaları | `AssetInbox/Products/<ürün>/Labels/`, `AssetInbox/Packages/<ambalaj>/` |
| Ürün manifesti | `AssetInbox/Products/<ürün>/manifest.json` |
| Kutu modeli | `Content/Products/Packages/box_<G>x<D>x<Y>/SM_box_...` |
| Etiket dokusu ve materyali | `Content/Products/Items/<ürün>/T_<ürün>_Label`, `MI_<ürün>` (varsa `_Kapak`, `_Govde`) |
| Model parça materyalleri (cam, kapak, gövde rengi) | `Content/Products/Packages/<ambalaj>/MI_<ambalaj>_<yuva>` |
| Ortak ana materyaller | `Content/Products/Materials/M_ProductLabel`, `M_ProductSolid`, `M_ProductGlass` |
| Katalog satırı | `Config/products.json` (önceki sürüm `products.json.bak`) |

## Katalog şeması (v2)

v1.5 ile `package` içine isteğe bağlı `preset` (hazır ambalaj id) ve `colors` (parça renkleri) eklendi.


```json
{"id":"sutas_sut_1l","realName":"Sütaş Süt 1 L","fictionalName":"Trakya Süt 1 L","category":"süt",
 "cost":1.70,"price":2.50,"caseUnits":12,"color":"EEF1E9",
 "visual":{"package":"/Game/Products/Packages/box_70x50x200/SM_box_70x50x200.SM_box_70x50x200",
           "materials":["/Game/Products/Items/sutas_sut_1l/MI_sutas_sut_1l.MI_sutas_sut_1l"]}}
```

`visual.materials` malzeme yuvası sırasıyla ürüne özel materyallerdir; boş dize = ambalajın kendi materyali. Eski `visual.material` (tek) alanı da okunur ve yuva 0 sayılır. `visual`, `category`, `caseUnits` isteğe bağlıdır; v1 dosyaları olduğu gibi okunur. En fazla 24 ürün.

## Sınırlar (v1)

- Kutu şablonu yalnız dikdörtgen kutudur. Şişe, silindir, poşet şablonları sonraki adım.
- Özel model için UV şablonu dışa aktarma henüz yok (G-006).
- Model ölçeği/pivotu için düzeltme alanı yok; stüdyo yalnızca uyarır (G-007).
- Değişiklik oyunda görünmek için oyunun yeniden başlatılması gerekir.
