# Claude / başka ajan için Blender görev promptu

Aşağıdaki metni kopyala; köşeli alanları doldur ve ajana bu depo ile birlikte ver.

```text
Miras Market Unreal Engine projesi için Blender 5.2 uyumlu, oyun hazır bir çevre varlığı üret.

VARLIK
- Ad: [VARLIK_ADI]
- Tür: [gondol / duvar rafı / soğutucu / kasa / teşhir / başka]
- Gerçek dış ölçü: [GENİŞLİK] × [DERİNLİK] × [YÜKSEKLİK] mm
- Kullanım: [ürün kategorileri ve oyuncunun göreceği mesafe]
- Referanslar: [dosya yolları; yoksa “referans yok”]
- Ürün bölgeleri: [kaç seviye, her seviyenin kullanılabilir genişlik/derinliği]

ÖNCE OKU VE UYGULA
- Docs/Environment/BLENDER_STANDARDI.md
- Docs/Environment/BLENDER_KULLANICI_REHBERI.md
- Docs/Environment/equipment_template.json
- Referans uygulama: Tools/Blender/create_gondola_shelf.py
- Referans doğrulama: Tools/Blender/validate_gondola_shelf.py

DEĞİŞMEZ TEKNİK SÖZLEŞME
1. Blender Metric, Unit Scale 1.0. Ölçüler gerçek ölçüdür; göz kararı ölçekleme yapma.
2. +Z yukarı, müşteriye bakan ana ön yüz -Y. Origin zemin merkezinde ve model Z=0 altına geçmez.
3. Görünür ana mesh `SM_[VARLIK_ADI]`; collision `UCX_SM_[VARLIK_ADI]_00` biçiminde.
4. Rotation 0/0/0 ve scale 1/1/1 uygulanmış olacak. Normal yönleri dışarı bakacak.
5. Ürün konabilecek her yüzey metadata'da santimetre cinsinden ayrı zone olacak. Zone id sabit ve açıklayıcı olacak (`front_l00`, `front_l01`, `back_l00`).
6. Materyaller fiziksel malzemeye göre ayrılacak. BaseColor'a ışık/gölge boyama. Roughness ve metallic PBR değerleriyle verilecek. Etiket veya logo uydurma.
7. FBX: selected objects, mesh, Apply Transform, Forward -Y, Up Z, animasyon yok.
8. Teslim: kaynak .blend, FBX, equipment.json, 1024×1024 preview.png ve kullanılan tüm dokular.
9. Teslim klasörü: AssetInbox/Environment/[KATEGORİ]/[VARLIK_ADI]/
10. Var olan ürün kataloğunu, Config/products.json dosyasını ve Content/Products altını değiştirme.

KALİTE HEDEFİ
- Yakın çekimde keskin bilgisayar kutusu gibi görünmeyecek: gerçek üretim kalınlıkları ve küçük bevel kullanılacak.
- Poligon bütçesi hedefi [BÜTÇE] üçgen. Siluete katkısı olmayan parçaları geometri yapma.
- UCX sade olacak; ürün yerleşim hacimlerini kapatmayacak.
- Tekrarlı modüller aynı ölçü ve bağlantı noktalarıyla yan yana boşluksuz kurulabilecek.

ÇALIŞMA BİTİNCE RAPORLA
- Üretilen dosyaların yolları
- Ölçülen bounding box
- Üçgen ve materyal sayısı
- Zone sayısı ve kapasiteleri
- Uygulanan doğrulamalar ve kalan gerçek riskler

Görevi yalnız açıklama yazarak bitirme. Dosyaları gerçekten üret, doğrulama betiğini çalıştır ve hataları düzelt.
```

Ajana görsel referans verirken “bunun aynısını yap” yerine hangi özelliklerin alınacağını yaz: raf tablası profili, ayak biçimi, malzeme, dönem, aşınma seviyesi ve modüler ölçü. Marka/logoyu çevre mesh'ine gömmek yerine ayrı tabela materyali olarak iste.

