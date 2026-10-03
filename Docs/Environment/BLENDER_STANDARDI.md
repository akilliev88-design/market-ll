# Miras Market Blender varlık standardı

Uygulamalı kullanıcı akışı: [BLENDER_KULLANICI_REHBERI.md](BLENDER_KULLANICI_REHBERI.md). Claude veya başka bir ajana verilecek hazır sözleşme: [CLAUDE_BLENDER_PROMPT.md](CLAUDE_BLENDER_PROMPT.md). Yeni ekipman metadata başlangıcı: [equipment_template.json](equipment_template.json).

## Temel sözleşme

- Blender birimi metre, Unit Scale `1.0`; 1 metre Unreal'da 100 cm'dir.
- Z yukarıdır. Müşteriye bakan ön yüz Blender'da `-Y` yönüdür.
- Origin ekipmanın zemin merkezindedir. Model zemin altına taşmaz.
- Ölçek ve dönüşler dışa aktarmadan önce uygulanır.
- Statik mesh adı `SM_`; çarpışma parçaları `UCX_<mesh>_NN`; materyal yuvaları `MI_` ile başlar.
- FBX dışa aktarımı `Forward -Y`, `Up Z`, animasyonsuz ve yalnız seçili nesnelerle yapılır.

## İlk ana modül

`SM_Gondola_1200`, 1200 × 900 × 1600 mm çift yüzlü gondol rafıdır. Dört raf seviyesi, iki müşteri yüzü, fiyat rayları, sıcak ahşap altlık/başlık taşıyıcı ve üç basit UCX çarpışma kutusu içerir.

Kaynak ve çıktılar:

```text
Tools/Blender/create_gondola_shelf.py
AssetInbox/Environment/Shelves/Gondola_1200/
├── Source/Gondola_1200.blend
├── SM_Gondola_1200.fbx
└── equipment.json
```

`equipment.json` görünür modelden tahmin edilmeyecek oyun verilerini taşır: gerçek ölçü, müşteri cepheleri, sekiz raf yerleşim bölgesi, bağlantı noktaları, materyal yuvaları ve çarpışma politikası.

`planogram.showCategorySign` ekipmanın üzerindeki kategori tabelasını açar/kapatır (alan yoksa açık). `false` olduğunda tabela levhası ve yazısı üretilmez; ekipmanın kategorisi, ürün yerleşimi ve fiyat etiketleri korunur. BUZ referanslı soğutucu, dondurucu ve soğuk servis vitrinleri ile önceki soğuk tezgâhlar tabelasızdır. Blender kaynaklarında ayrıca tabela veya tabela taşıyıcısı bulunmadığından bu değişiklik için model/FBX yeniden üretilmez. [Oyun içi kontrol görüntüsü](../Images/Stores/Cabinets/no_category_signs_store.png).

## Kalite kapısı

1. Ölçü ve origin doğrulanır.
2. Ön/arka müşteri yönleri kontrol edilir.
3. Raf bölgeleri ürün ölçüleriyle kapasite testinden geçer.
4. Fiyat rayları ayrı materyal yuvası olarak kalır.
5. UCX adları ve engelleme hacmi Unreal'da kontrol edilir.
6. Yakın görünümde bevel, uzak görünümde siluet incelenir.
7. LOD ve Nanite kararı Unreal içe aktarımında verilir.

## Üretim ve Unreal'a aktarım

Blender üretimi:

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background --python Tools/Blender/create_gondola_shelf.py -- .
```

Blender kalite kapısı:

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background AssetInbox/Environment/Shelves/Gondola_1200/Source/Gondola_1200.blend --python Tools/Blender/validate_gondola_shelf.py -- .
```

Unreal aktarımı ve doğrulaması editör kapalıyken tek komutla yapılır:

```text
IMPORT_ENVIRONMENT.cmd
```
