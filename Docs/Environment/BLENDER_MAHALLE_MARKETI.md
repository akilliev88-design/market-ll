# Sıfırdan Blender mahalle marketi

G-116 ile bu prototipin rafları da kısa günlük alışveriş düzenine uyarlandı; manav, ekmek ve bakım rafı eklendi.
Kampanya mahalle planı ve diğer üç format için [güncel yerleşim rehberine](DUNYA_MARKET_YERLESIMLERI.md) bakın.

Mustafa'nın 05.10.2026 isteği: Image-blaster çıktısı görsel olarak reddedildi. Yeni bina Blender geometrisiyle sıfırdan hazırlandı; önceki dünya modelinden geometri/doku alınmadı.

8 × 10 m bina, 3,1 m iç yükseklik, geniş cam cephe ve cam köşe, 1,8 m açık giriş, basamaksız kaldırım, sundurma/tabela, tavan ışıkları ve küçük arka depo. Raf, dolap, kasa ve etiketli ürünler mevcut oyun varlıklarıdır; ayrı ekipman olarak yerleştirilir. Sokak şimdilik küçük bir çevre denemesidir.

## Açma

1. Proje kökündeki **MAHALLE_MARKET_GEZI.cmd** dosyasına çift tıkla.
2. WASD ile yürü, fareyle bak. Girişten kaldırıma çıkabilirsin.
3. **1 / 2 / 3** dış cephe, içerisi ve içeriden giriş kameraları. **0** yeniden yürüyüş. **Esc** çıkış.

Bu ayrı Unreal oyun sahnesidir. Kampanya kaydına ve ana mağazanın ticari düzenine henüz bağlı değildir. Ana oyun mağazasını değiştirmek ayrıca yapılmalıdır.

## Blender'da düzenleme

Kaynak: `AssetInbox/Environment/Stores/HandmadeNeighborhood/Source/HandmadeNeighborhood.blend`.

Dosya metre birimindedir. `EditableArchitecture` koleksiyonunda adlandırılmış özgün parçalar saklıdır. Bunlar varsayılan olarak gizlidir. Düzenlemek için koleksiyonu aç, parçaları göster ve birleşik `SM_HandmadeNeighborhood` önizlemesini gizle. FBX için aynı dönüşümle render ve UCX çarpışma parçaları birlikte dışa aktarılmalıdır; kapı boşluğunu tek büyük çarpışma gövdesiyle kapatma.

Üretim betiği: `Tools/Blender/create_neighborhood_store.py`. Ölçü ve parçalar burada okunabilir şekilde tanımlanır. Betiği yeniden çalıştırmak elle yapılan `.blend` değişikliklerini değiştirir; elle düzenlediğin dosyayı ayrı adla kaydet.

## Betikten yeniden üretme / doğrulama

Unreal kapalıyken, proje kökünde PowerShell:

```powershell
powershell -File Tools/HandmadeNeighborhood.ps1 -RebuildAssets
```

Blender modeli ve FBX yeniden oluşturulur; Unreal yalnız bu yeni mağazayı içe alır. Giriş, cam çarpışması ve zemin kontrol edilir; üç oyun görüntüsü `Saved/Screenshots/HandmadeNeighborhood` altına yazılır. `.blend` doğrudan elle düzenlenmişse bu komutu kullanma: betik özgün model tanımından yeniden üretir.

Kod değiştirildiğinde Unreal kapalıyken `DERLE.cmd /q`, ardından `TEST.cmd /q` ve `powershell -File SmokeTest.ps1` çalıştırılır. Gezinti modu mevcut sanat denemesinin `-HandmadeNeighborhood` seçeneğidir.


05.10.2026 (G-115): Eski açık soğutucu çıkarıldı; süt/soğuk ürün reyonunda masaüstünden aktarılan yeni üç kapılı `drink_cooler_3door_2100` kullanılır. Ürün dizilimi modelin raf yüksekliği, genişliği ve derinliğinden yeniden hesaplanır. Yeni görünüm için geziyi kapatıp yeniden aç.
