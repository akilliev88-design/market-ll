# G-088 — mağaza kitinin kullanımı

Mağaza/depo boyutu, yaslama, hazır bölümler ve zemin düzenleme için `MAGAZA_EDITORU.cmd` açılır. Kullanım ve kaynaklar: [Mağaza editörü](MAGAZA_EDITORU.md). 15 yeni Blender dolabı ve teşhir modeli: `Docs/Images/Stores/Cabinets/index.html`.

Aşama A dört örnek içerir: `mahalle_01`, `kucuk_01`, `buyuk_01`, `hiper_01`. Kalan 16 mağaza Mustafa'nın görsel onayından sonra hazırlanır. Aile dükkânının ekipman yerleşimi değişmedi.

Mustafa'nın ilk görseli reddetmesinden sonra yerleşimler yeniden tasarlandı. Mahalle dükkânı L biçiminde, iki kolon ve duvar dönüşü içerir; ucuzcuda köşe girintisi, kolonlar ve farklı uzunlukta raf grupları vardır. Süpermarketin manav avlusu ve arka servis hattı, hipermarketin üç raf bölgesi, geniş kesişen koridorları ve ayrı gıda dışı bölümü bulunur. Süpermarket tavanı 600 cm, hipermarket tavanı 800 cm; mevcut `CeilingBay_6000` boru/taşıyıcı modülü yalnız bu iki türde instanced kullanılır. Mahalle/ucuzcu 310/360 cm. Aile bakkalında aynı kabuk ve ekipman konumları korunarak açık renk düz tavan ve yüzeye yakın ışık panelleri kullanılır.

`Tools/create_store_design.py` dört yerleşimin kaynağıdır; `create_store_templates.py` bunu çağırır. İsteğe bağlı `architecture` alanı `outlineCm` zemin çokgenini, `obstacles` kolon/duvar dönüşlerini, `zones` zemin kaplamalarını ve `sections` bölüm tabelalarını taşır. Kabuklar `Tools/Blender/store_architecture.py` ile üretilir. Alan çokgenden hesaplanır; doğrulayıcılar ekipmanların girinti/kolonlara taşmasını reddeder. `roof` ayrı varlıktır: önizlemenin genel ve tepeden açılarında çatı ile tesisat gizlenir; üç iç açı gerçek tavanı gösterir. Manav kasaları açık kenarlı, ürünler ayrı meyve geometrisidir.

## Kaynaklar ve üretim

- `Tools/store_specs.py`: ekipman ölçüleri, dört kabuğun üretim parametreleri.
- `Tools/create_store_templates.py`: `Config/magazalar.json` yerleşimlerini üretir. Yeniden üretimden sonra doğrulayıcıyı çalıştır.
- `Tools/Blender/create_store_phase_a.py`: metre, -Y ön, zemin merkezinde orijin, uygulanmış dönüş/ölçek, UCX, PBR malzemeler, `.blend`, FBX, önizleme ve `equipment.json`.
- `Tools/Blender/measure_store_equipment.py`: mesh ve UCX birlikte ölçülür; zemin altına taşma, orijin, dönüş/ölçek ve çarpışma adedi denetlenir. Nominal ölçüler yerine fiziksel zarf metadata'ya yazılır.
- `Tools/Blender/export_store_fbx.py`: kaynak `.blend` metre kalır; FBX için geçici sahnede mesh ve UCX koordinatları birlikte santimetreye çevrilir. UE'nin eski FBX yolunda yalnız birim ayarı render geometrisini büyütüp UCX'i 100 kat küçük bırakabildiği için bu adım zorunludur. Önizleme oyuncunun zeminde kalmasını ayrıca denetler.
- `Tools/import_store_phase_a.py`: Unreal editör Python aktarımı. Her varlığın UCX sayısı denetlenir. UE 5.8'de `FbxImportUI` ayarları için işlem süresince eski FBX yolu seçilir; proje ayarı değiştirilmez. [Epic aktarım belgesi](https://dev.epicgames.com/documentation/unreal-engine/importing-assets-using-interchange-in-unreal-engine).
- `Tools/create_turkish_font.py`: Plex SemiBold'u yalnız editör işlemine kaydeder, Türkçe offline `UFont` ve ona bağlı yazı materyalini `/Game/Stores/Fonts` altında üretir; işletim sistemine kalıcı font kurulmaz.

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background --python Tools/Blender/create_store_phase_a.py -- .
python Tools/validate_stores.py --write
powershell -File Tools/StorePreview.ps1
powershell -File Tools/StorePreview.ps1 -Benchmark
```

`python` PATH'te yoksa yerel Codex bağımlılık paketindeki Python'u kullan. Aktarım ve font betikleri `UnrealEditor-Cmd.exe <mutlak .uproject yolu> -run=pythonscript -script=<mutlak betik yolu> -unattended -NullRHI -nosound` ile çalışır. Editör açıkken derleme yapılmaz.

## Oyun bağlantısı (Claude, Aşama C)

1. `MarketStoreKit::Load(Errors)` sözleşmeyi atomik yükler; bozuk yükleme önceki geçerli listeyi değiştirmez. `TemplatesFor` sıralı kimlikleri, `Find` bir mağazayı verir.
2. `ToPlanogram(Store, Overrides)` ürünsüz planı oluşturur. Overrides anahtarı `fikstür_id` ise iki yüz, `fikstür_id/front` veya `fikstür_id/back` ise tek yüz değişir. Boş değer **Kategorisiz** demektir.
3. `Fill(Plan, Products)` atanmış reyonları koruyarak `MarketLayout::Plan` kullanır. Büyük mağazalarda aynı üründen birden çok blok oluşturabilir. Bu yalnız görünüm planıdır; ekonomik stok eklemez.
4. `Build(World, Store, Filled)` kabuk, instanced ekipman/ürünler, tabela, dekor ve tema ışığını kurar. `Clear(World)` yalnız kitin etiketlediği aktörleri kaldırır.
5. `AMarketGameMode::StoreCategoryOverrides` son seçimleri taşır. `OnStoreCategoriesChanged` geri çağrısını il+tür kaydına bağla. Kit aile dükkânının `Config/planograms.json` dosyasına şube seçimini yazmaz.
6. `Clear` sonrası aile dükkânına dönüşte aile planını/dünyasını geri getirmek, stok ve müşteri bağlantısı, atama, gez düğmesi ve `Stats` sayılarının ekonomik hesaba aktarılması Claude'dadır. Bu işte ilgili Claude dosyaları değiştirilmedi.

**MarketLayout notu:** mevcut `MarketLayout::Plan` kategori atamalarını yeniden yapar ve rapor için ASCII kategori kullanır. Bu yüzden `Fill`, atanmış kategorileri bölüm bölüm planlar; soğutucuya temizlik ve çift yüzlü rafa yanlış kategori gelmesini önler. `MarketLayout.cpp` değiştirilmedi. Claude ileride yüz kategorilerini doğrudan kabul eden bir API ekleyebilir.

**Personel notu:** mevcut `StaffPlanner` fikstürün tek kategorisini okur. `MarketWorkers.cpp` yüz kategorisini denetler ve arka yüz için uygun yer seçimini uyarlayarak çalışır. Eski kategori blokları korunur; paylaşılan stok nedeniyle bir ürünün herhangi bir bloğu yanlış kategorideyse görevliler o ürünü doldurmaz. `StaffPlanner` dosyası değiştirilmedi.

## Raf kategorisi ve Türkçe yazı

`MAGAZA_GEZI.cmd` bağımsız test gezisini açar; kampanya yüklenmez/kaydedilmez, mağazalar oyuncunun yürüdüğü gerçek dünyada kurulur. Başlangıç mahalledir; isteğe bağlı ilk argüman mağaza id'sidir. **F10** sonraki, **Shift+F10** önceki mağaza; **F3 / F7** her basışta yeni rastgele raf dolumu; **WASD/fare** hareket/bakış, **T** kategori, **Esc** çıkış. Ekranda mağaza adı ve tuşlar sürekli görünür. Gezi sırasında ekonomi/menü/kayıt komutları çalışmaz. Normal aile dükkânında F2 test modu açıkken F7 rastgele dolum yapar; rafların fiziksel yerleri değişmez. Rastgele dolum katalog kopyasında geçici sıralama anahtarlarıyla `MarketLayout::Plan` kullanır; gerçek marka/fiyat/ölçü ve kategori değişmez. `Tools/StoreTourTest.ps1` dört mağazada oyuncunun zeminde kalması, dolum değişmesi ve paranın değişmemesini denetler.

Tabelaya nişan alıp T: Türkçe alfabetik kategori listesi, mevcut seçim önce, Kategorisiz en sonda. Oklar veya tekerlek, E veya tıkla onay, Esc ile kapat. İki gondol yüzü bağımsızdır. Eski bloklar taşınmaz; tabelada uyumsuz ürün sayısı çıkar. Aile dükkânında seçim planograma yazılır.

Eski planogramlar `Category` değerini iki yüzde miras alır. Yeni isteğe bağlı `faceCategories` alanı boş kategoriyi de saklar; şema v3 geriye uyumlu kalır. `UpperTurkish` yalnız i/ı değil, ç/ğ/ö/ş/ü harflerini de açıkça dönüştürür. Ortak `MarketWorldText::Apply` tabela, fiyat ve diğer dünya yazısında aynı atlas/material çiftini kullanır.

## Önizleme ve doğrulama

- `-SimStorePreview=<id>`: oyuncu başlangıcı ve zemin çarpışması kontrolü, bağımsız sabit kamera, shader/ışık bekleme, beş 1280×720 PNG (çatı kaldırılmış genel görünüm, giriş/kasa, manav/teşhir, kolon/servis, tepeden plan). Yol: `Saved/Screenshots/Stores/<id>/01.png` … `05.png`.
- Sanat önizlemesi katalogdaki hazırlık ürünlerini de geçici olarak kullanır; bunların bir kısmı renkli prototip kutudur. Ürün kataloğu ve kampanya stoğu değişmez. Normal `Fill` yalnız aktif ürünleri kullanır.
- `-SimStoreBenchmark`: aynı beş açı 1920×1080, FPS logu, ekran görüntüsü yazmadan. Ölçüm offscreen editör oyununda yapılır; yürüme/müşteri simülasyonuyla ayrı performans testi değildir.
- `validate_stores.py --write`: fiziksel ekipmandan stats hesaplar; bant, bölüm, kategori/id, noktalar, ekipman çakışması ve oyuncu başlangıcı denetlenir. Yazmasız çalıştırma eskimiş stats'ı reddeder.
- `MarketStoreKitTests.cpp`: dört tür, kategoriye bağlı ve genel otomatik plan, benzersiz id, eksik bölüm/bant, bozuk JSON, Türkçe dönüşüm, offline atlas glifleri ve kayıt uyumu.

Üretim kaynakları `AssetInbox/Environment/Stores`, Unreal varlıkları `Content/Stores` altındadır. Paketlenmiş oyunda metadata dağıtımına ilişkin son staging düzeni Aşama C/release hazırlığında ele alınmalıdır; burada editör/yerel oyun doğrulandı.
