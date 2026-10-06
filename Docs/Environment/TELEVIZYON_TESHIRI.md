# Televizyon teşhir kiti

01.10.2026 — Codex. Referanslardan esinlenen oyun modelleri; marka kopyası veya üretici teknik modeli değildir.

## Ayrı varlıklar

| Ekipman | Ölçü | Blender kaynağı |
|---|---|---|
| `tv_wall_4800` | 480 × 65 × 255 cm, iki seviye | `AssetInbox/Environment/Stores/tv_wall_4800/Source/tv_wall_4800.blend` |
| `tv_plinth_2400` | 240 × 75 × 60 cm, tek yüz | `AssetInbox/Environment/Stores/tv_plinth_2400/Source/tv_plinth_2400.blend` |
| `tv_island_3000` | 300 × 140 × 65 cm + 12 cm kablo omurgası, iki yüz | `AssetInbox/Environment/Stores/tv_island_3000/Source/tv_island_3000.blend` |

Stand FBX dosyaları boş mobilyadır. Blender önizlemesindeki `PreviewOnly_*` TV objeleri ayrı kalır ve dışa aktarılmaz. LED şeritleri ayrı materyal yuvasındadır; Unreal aktarımı emissive materyali kurar. Parlama mevcut oyun ışığı/Lumen/bloom ayarlarıyla görünür.

32, 43, 55, 65 ve 75 inç TV'lerin her biri `AssetInbox/Products/Televisions/tv_<boyut>/` altında ayrı `.fbx`, `product.json`, `preview.png` ve `Source/tv_<boyut>.blend` dosyasına sahiptir. İnç değeri 16:9 ekran köşegenidir; dış ölçülere çerçeve ve ayak dahildir. Pivot alt merkez; ürün eksenleri +X ön, +Y genişlik, +Z yukarı. Ayaklar, ince çerçeve, arka gövde, havalandırma ve bağlantı girintisi bulunur. Ekrandaki manzara özgün, sabit demo dokusudur; video oynatılmaz.

## Oyuna yerleştirme

Mağaza Editörü ekipman kütüphanesinde üç teşhir vardır. Teknoloji bölümü ekleme de bunları kullanır. Standı yerleştirdikten sonra mevcut kategori seçimi ile reyon kategorisini belirle; adanın yüzleri ayrı seçilir. Başlıktaki yazı seçilen kategoriye bağlıdır.

TV'ler henüz satış kataloğuna otomatik eklenmez. Ürün Stüdyosu'nda ayrı ürün olarak oluşturulup yayımlanırlar. Hazır Unreal mesh yolu `/Game/Stores/Televisions/SM_Television55.SM_Television55` biçimindedir; dosyadan çalışılacaksa ilgili FBX kullanılabilir. Ürün kimliğini `tv_55` gibi tutmak aşağıdaki eşleşmeyi doğrudan kullanır. Gerçek ürünün kendi kimliği tercih edilirse profil satırındaki `id` de o kimlik olmalıdır. Ölçüleri `product.json` içinden al; görsel ölçek 1, yön düzeltmesi 0. Raf dizme ile elle yerleştir. Yerleşim adedi/kapasitesi mevcut planogram kurallarıyla hesaplanır.

## Ürüne bağlı özellikler

`AssetInbox/Products/Televisions/display.json`, katalog ürün kimliğine bağlı görsel özellikleri saklar. Katalog şeması ve ekonomi değişmez. Başlangıç profillerinde yalnız ölçülen inç vardır; teknoloji tahmin edilmez. Gerçek ürünün özellikleri belli olduğunda ilgili satıra ekle:

```json
{
  "id": "tv_55",
  "inches": 55,
  "technology": "OLED",
  "resolution": "4K",
  "refreshHz": 120,
  "rearLight": true,
  "rearColor": "3C82FF"
}
```

Bu yalnız örnektir; 55 inç olmak OLED veya 120 Hz olmak anlamına gelmez. `rearLight` ürüne özel sabit arka aydınlatmadır; ekran görüntüsünü izleyen Ambilight simülasyonu değildir. Kapalı/eksikse renkli ışık oluşmaz. Markalar ve ürün adları `ProductName`/katalog üzerinden gerçek-kurgu seçimini izler; özellikler ID üzerinden alınır. Profil yoksa yalnız ürün adı görünür. Standlara sabit Philips/LG/OLED/QLED yazısı gömülmez. Raf yeniden dizilince eski yazı ve ışıklar kaldırılır, yeni ürün için yeniden kurulur. Mağaza önizlemesi/gezi de aynı eşleştirmeyi kullanır.

## Yeniden üretim

```powershell
& 'C:/Program Files/Blender Foundation/Blender 5.2/blender.exe' --background --python Tools/Blender/create_tv_displays.py -- 'C:/Users/mtass/Desktop/market-ll'
& 'C:/Program Files/Blender Foundation/Blender 5.2/blender.exe' --background --python Tools/Blender/create_televisions.py -- 'C:/Users/mtass/Desktop/market-ll'
```

Unreal Python: `Tools/import_tv_displays.py` yalnız bu sekiz modeli aktarır; cm ölçülerini ve stand UCX sayılarını doğrular. TV üreticisi var olan `display.json` dosyasının üzerine yazmaz. C++: `MarketTelevisionDisplay.*`; testler `MarketSim.Visuals.TelevisionDisplay` ve `TelevisionLabels`.

## Yüzey çakışması kontrolü

Standların gövde/tabla ve duvar/kolon yüzleri aynı düzlemde üst üste binmez. Podyum tablasının altında 4 mm açıklık vardır; etiket rayı gövdeden 4 mm dışarıdadır. `railFrontY` görünür ray yüzüne göre hesaplanır. LED/parlama materyali korunur; bu düzeltme oyun genelindeki Lumen ayarlarını değiştirmez.

```powershell
& 'C:/Program Files/Blender Foundation/Blender 5.2/blender.exe' --background --python Tools/Blender/check_tv_surfaces.py -- 'C:/Users/mtass/Desktop/market-ll' --strict
```

Kontrol üç Blender kaynak meshindeki aynı yöne bakan, eksene paralel yüzleri tarar; zemine temas eden alt yüzler hariç, çakışma varsa başarısız olur. Genel ışık titremesi testi değildir. Unreal aktarımına `-TVFixturesOnly` verilirse yalnız üç stand yeniden aktarılır; ayrı TV ürünlerine dokunulmaz. `Tools/review_tv_displays.py` için `-TVTemporalReview` aynı kameradan dört saniye arayla üç kare ister; kaydedilmeyen kontrol sahnesi kullanır.
