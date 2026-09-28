# Blender ile Miras Market varlığı üretme

Bu rehber, Codex limiti olmasa da aynı kalitede raf, dolap, kasa veya teşhir ünitesi üretebilmen için hazırlanmıştır. Başlangıç örneği `AssetInbox/Environment/Shelves/Gondola_1200/Source/Gondola_1200.blend` dosyasıdır.

## Bir kez yapılacak Blender ayarı

1. Scene Properties > Units: Unit System `Metric`, Unit Scale `1.0`, Length `Meters`.
2. Ön görünüş müşteriye bakan `-Y`, yukarı `+Z`, sağ `+X` olacak.
3. Varlığın origin noktası zeminin tam ortasında kalacak: `X=0, Y=0, Z=0`.
4. Düzenli koleksiyonlar aç: `VISIBLE`, `COLLISION`, `SOCKETS`.
5. Mevcut gondol `.blend` dosyasını farklı kaydet; malzeme ve ad düzenini koru.

## Her modelde uygulanacak sıra

1. Önce gerçek dış ölçüyü milimetre cinsinden yaz. Örnek: genişlik 1200, derinlik 450, yükseklik 2100 mm.
2. Ana silueti metreyle kur: 1200 mm = `1.2 m`.
3. Raf tablalarını ayrı nesne tut; adları `Shelf_L00`, `Shelf_L01` biçiminde olsun.
4. Ürün konabilecek her düzlem için `equipment.json` içine bir bölge yaz. Görünür yardımcı düzlem ekleme.
5. Keskin kenarlara küçük bevel ekle. Gerçek rafta olmayan dekoratif girinti üretme.
6. Materyal yuvalarını malzemeye göre ayır: metal, boyalı metal, ahşap/laminat, fiyat rayı, plastik/cam.
7. Üçgen yüz, ters normal, üst üste yüz ve uygulanmamış ölçek kontrolü yap.
8. Ana nesneyi seçip `Ctrl+A > Rotation & Scale` uygula.
9. Basit kutulardan UCX çarpışma üret; raf tablalarının her birini çarpışma yapma.
10. Önce `.blend`, sonra yalnız teslim nesnelerini seçerek FBX kaydet.

## FBX dışa aktarma

- Selected Objects: açık
- Object Types: Mesh
- Apply Transform: açık
- Forward: `-Y Forward`
- Up: `Z Up`
- Apply Unit: açık
- Add Leaf Bones / Animation: kapalı
- Path Mode: Auto

Dosyalar şu yapıda teslim edilir:

```text
AssetInbox/Environment/<Kategori>/<VarlikAdi>/
├── Source/<VarlikAdi>.blend
├── SM_<VarlikAdi>.fbx
├── equipment.json
├── preview.png
└── Textures/
    ├── T_<VarlikAdi>_BaseColor.png
    ├── T_<VarlikAdi>_Normal.png
    └── T_<VarlikAdi>_ORM.png
```

`ORM` dosyasının R kanalı ambient occlusion, G kanalı roughness, B kanalı metallic değeridir. Renk dokusu sRGB; Normal ve ORM sRGB kapalıdır. 1K küçük aksesuar, 2K standart raf, 4K yalnız büyük ve yakından görülen benzersiz ünite için kullanılır.

## Teslimden önce kısa kontrol

- [ ] Gerçek dış ölçü doğru ve `equipment.json` ile aynı.
- [ ] Origin zeminde ve merkezde; model zeminin altına inmiyor.
- [ ] Müşteri yüzü `-Y`; yazılar bu yönden aynasız okunuyor.
- [ ] Scale `1,1,1`, rotation `0,0,0`.
- [ ] Mesh `SM_`, çarpışma `UCX_<SM adı>_00` düzeninde.
- [ ] Ürün bölgeleri raf kenarından taşmıyor ve santimetre cinsinden yazılmış.
- [ ] Materyal sayısı gereksiz çoğaltılmamış; PBR doku kanalları doğru.
- [ ] `.blend`, FBX, metadata ve önizleme birlikte teslim edilmiş.

## Mevcut otomatik örneği yeniden üretme

Blender arayüzünde Scripting çalışma alanını açıp `Tools/Blender/create_gondola_shelf.py` dosyasını çalıştırabilir veya PowerShell'den:

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background --python Tools/Blender/create_gondola_shelf.py -- .
```

Bu betik öğrenme örneğidir: yeni varlıkta ölçüleri ve parçaları değiştir, fakat eksen, origin, ad ve teslim kurallarını değiştirme.

## Mağaza kitini yeniden üretme

`BLENDER_MAGAZA_KITI.cmd` yedi varlığı Blender'da baştan üretir, FBX'leri Unreal'a aktarır ve ölçü/materyal/çarpışma kalite kapısını çalıştırır:

- `SM_WallShelf_2400`: altı seviyeli duvar reyonu.
- `SM_BulkIsland_1600`: şeffaf hazneli kuru yemiş/lokum adası.
- `SM_CeilingBay_6000`: açık tavan kirişi, galvaniz kanal ve altı lineer armatür.
- `SM_CheckoutLane_2500`: konveyör, tarayıcı, tartı, kasa çekmecesi, ekranlar ve paketleme alanı olan market kasası.
- `SM_OfficeDesk_1800`: çekmeceli yönetim masası, monitör, klavye, fare ve evraklar.
- `SM_RefrigeratedWall_3000`: üç cam kapılı, iç raflı ve ürün dolgulu duvar soğutucusu.
- `SM_ProduceIsland_2400`: iki yüzlü, eğimli ahşap kasalı ve ürün dolgulu manav adası.

Tavan modülü ayrıca kablo tavası, elektrik boruları, askı çubukları ve kırmızı sprinkler hattını içerir.

Kaynak betik `Tools/Blender/create_store_kit.py` dosyasıdır. Renk, ölçü ve parça biçimini burada değiştirip aynı komutu tekrar çalıştırabilirsin.

