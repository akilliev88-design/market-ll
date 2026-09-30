# Mağaza editörü

`MAGAZA_EDITORU.cmd` dosyasını aç. Unreal Editor içinden **Tools > Mağaza Editörü** de aynı pencereyi açar.

## Tek büyük çalışma alanı

Editör yalnızca kuşbakışıdır. **Ekipmanlar** ve **Özellikler** düğmeleri ilgili yan paneli açıp kapatır; **F11 / Geniş alan** ikisini birlikte gizler veya açar. Yakınlaştırılmış plan yalnızca orta çalışma alanında çizilir; yan menülere taşmaz.

- **Boş alanı sol tuşla tutup sürükle:** planı istediğin yöne kaydır. Orta veya sağ tuşla planın herhangi bir yerinden de kaydırabilirsin.
- **Tekerlek:** imlecin bulunduğu noktaya yakınlaş/uzaklaş. **Home:** bütün mağazayı sığdır. **F:** seçili ekipmana yakınlaş.
- **Parçayı sol tuşla tutup sürükle:** taşı. Sarı çerçeveli parçalar seçilidir.
- **Toplu seç:** düğmeye bas, sol tuşu basılı tutarak istediğin parçaların üzerinden çerçeve çiz. Alternatif: Shift + sol sürükle. Çerçeve bitince taşıma moduna döner.
- **Birden fazla parça seçiliyken birini tut:** tüm seçimi aralarındaki mesafeleri koruyarak taşı. **Ctrl + tık:** bir parçayı seçime ekle/çıkar. **Ctrl+A:** hepsini seç. **Delete:** seçili parçaları topluca kaldır. **Ctrl+Z:** bütün işlemi bir adımda geri al.

Editörde iki 3B mod ve ilgili önizleme kontrolleri kaldırıldı. Oyundaki mağaza gezisi ve rastgele raf doldurma `MAGAZA_GEZI.cmd` içinde devam eder.

![Tek görünümle düzenleme](../Images/Stores/Cabinets/store_editor.png)
![Yakınlaştırma sınırları](../Images/Stores/Cabinets/store_editor_full.png)

## Yeni mağaza

**Yeni mağaza** düğmesine bas, adını ve türünü seç, **Boş mağaza oluştur** de. Mahalle ve ucuzcuda normal; büyük ve hiperde yüksek tavanla boş bina açılır. Sağdaki metre alanlarından bina ve depoyu boyutlandır; ekipmanları yerleştir. **Mevcut düzeni kopyala** mevcut mağazanın türü ve düzeniyle ayrı bir taslak açar.

Kimlik otomatik ve benzersizdir; mevcut hazır mağazanın üzerine yazılmaz. Yeni mağaza hemen `Saved/StoreDrafts/` içinde saklanır ve sonraki açılışta mağaza listesinde görünür. Düzenlemeleri **Ctrl+S / Taslak kaydet** ile koru. Mağaza değiştirirken son taslak otomatik saklanır, listeden açarken varsa taslak tercih edilir.

## Yerleştirme

1. Üstten mağazayı seç. Soldaki kütüphanede ekipman ara ve adına tıkla.
2. Planda boş yere tıkla. Önizleme yeşilse sığar, kırmızıysa çakışır. Eklerken **R** ile parçayı döndür. Aynı ekipmandan başka parçalar eklemek için tekrar tıkla. Ekleme sırasında planı sağ/orta tuşla kaydırabilirsin.
3. **Esc / Seçim modu** ile eklemeyi bitir. Parçaya tıklayıp sürükleyerek taşı.
4. **R / Döndür 90°**, **Yan yana kopyala / Ctrl+D** ve **Sil / Delete** seçili parçaya uygulanır.
5. **Ctrl+Z** geri alır; **Ctrl+Y** yineler. Son 50 düzenleme tutulur.

Yeni rafın hangi ürünleri taşıyacağını sağ paneldeki kategori listesinden seç. **Kategorisiz** raflar oyun gezi modunda rastgele doldurulmaz; soğutucuda süt, dondurucuda dondurma gibi uygun kategoriler kullan.

**Duvara yasla** ve **Komşuya yasla / hizala** açık başlar. Yakındaki dolabın kenarına boşluk bırakmadan oturur; grubun dış kenarları da diğer dolaplara yaslanır. **10 cm ızgara kapalı başlar:** parçayı serbestçe konumlandırabilirsin. İstersen yaslamayı veya ızgarayı değiştir. Sola, sağa, öne ve depoya yaslama düğmeleri de vardır. Duvar/kolon/depo içinde veya başka dolabın üzerinde yerleştirme kabul edilmez; satış zemininin her uygun noktasına yerleştirme serbesttir. Kırmızı parçalar düzeltilmesi gereken yerleridir.

Yaslama ve ızgara seçenekleri yeni parça eklerken de uygulanır; kapatırsan istemediğin yuvarlama yapılmaz. Seçim, döndürme, kopyalama ve kaldırma düğmeleri çalışma alanının üstünde her zaman görünür.

## Bina, depo ve bölümler

Sağdaki **Bina ve depo** alanları metre cinsindendir: mağaza eni/boyu, depo eni/boyu ve tavan yüksekliği. Giriş ve mal kabul kapısının sağ/sol konumunu da değiştirebilirsin. Değeri yazıp Enter'a bas veya sayıyı sürükle.

Bina boyutları değişince girintili dış hat korunarak ölçeklenir. Ekipman merkezleri, kolonlar ve tabelalar yeni boyuta taşınır; ekipmanların gerçek ölçüleri değişmez. Küçültme sonrasında kırmızı ekipmanları düzelt. Depo arka taraftadır; eni ve boyu bağımsız değişir.

**Kasap**, **Şarküteri**, **Manav** ve **Teknoloji** düğmeleri boş alana ilgili ekipmanları ve bölüm tabelasını ekler. Kasapta servis ve hazırlık tezgâhı; teknolojide deneme masası ve duvar teşhiri vardır. Parçaları sonrasında ayrı ayrı taşıyabilirsin. Yeterli alan yoksa bütün bölüm ekleme işlemi geri alınır.

Sağ paneli aşağı kaydırınca **Zemin** ayarları görünür: krem, açık/koyu gri, toprak, yeşil; özel kırmızı/yeşil/mavi değerleri; seramik, beton ve parlak yüzey. Bu görünüşü oyuna kaydettikten sonra mağaza gezi modunda inceleyebilirsin.

## Kaydetme

- **Taslak kaydet**: `Saved/StoreDrafts/<mağaza>.json`. Mağaza türünün alan/reyon/kasa bantlarını henüz karşılamayan tasarımlar da taslak olabilir; geometrik çakışma kabul edilmez.
- **Taslağı aç**: seçili mağazanın son taslağını getirir.
- **Oyuna kaydet**: `Config/magazalar.json` içindeki seçili mağazayı günceller. Alan, kasa, raf, soğutma bantları ve zorunlu bölümler doğrulanır. `stats` ekipmandan yeniden hesaplanır. Hatalı düzen katalog üzerine yazılmaz.
- Dosya değiştirilmeden önce `Saved/StoreBackups/` altında tarihli yedek alınır. Yazma geçici dosya üzerinden yapılır.
- Kaydedilmemiş bir mağazadan diğerine geçerken taslak otomatik korunur; hatalı geometri varsa geçiş engellenir. Pencereyi kapatmadan **Taslak kaydet** kullan.
- Oyuna kaydettiğin mağazayı `MAGAZA_GEZI.cmd` ile gezebilirsin: F10 mağaza değiştirir, F3/F7 rafları rastgele doldurur.

Aile bakkalının elle dizilmiş düzeni bu editörün mağaza kataloğuna dahil değildir. Ekonomi, şube ataması ve oyun kayıt bağlantıları Claude'un alanıdır. Editör doğrudan stok veya para değiştirmez.

## Modeller ve üretim

15 yeni ekipman: açık/alçak/cam kapılı sütlükler, tek/iki kapılı içecek dolapları, dikey/havuz/kutu dondurucular, kasap ve şarküteri servis tezgâhları, pasta ve dondurma vitrinleri, kasap hazırlık tezgâhı ve iki teknoloji teşhiri. Önceki mağaza ekipmanları da kütüphanede bulunur.

Referans: [BUZ teşhir reyonları](http://www.buzrefrigeration.com/tr/urun/teshir-reyonlari), özellikle [dikey reyonlar](http://www.buzrefrigeration.com/tr/urunler/dikey-reyonlar), [servis reyonları](http://www.buzrefrigeration.com/tr/urunler/servis-reyonlari) ve [havuz dondurucular](http://www.buzrefrigeration.com/tr/urunler/havuz-tipi-dondurucular). Modeller fotoğraflardaki tiplerden esinlenmiştir; ölçüler oyun için tasarlanmıştır, üreticinin teknik ölçüsü olarak sunulmaz. Marka/logolar eklenmedi.

- Blender kaynakları: `AssetInbox/Environment/Stores/<ekipman>/Source/*.blend` (metre, ölçek 1, -Y ön, zemin merkezi orijin).
- Aynı klasörde FBX, `equipment.json`, 1024×1024 gerçek Blender renderı `preview.png` ve UCX çarpışmaları.
- Oyun varlıkları: `Content/Stores/Equipment/`; zemin malzemesi `Content/Stores/Materials/M_EditableSurface`.
- Görsel galeri: `Docs/Images/Stores/Cabinets/index.html` ve `cabinet_library.jpg`.
- Üretim: `Tools/Blender/create_display_cabinets.py`; kontrol: `Tools/Blender/check_display_cabinets.py`; aktarım: `Tools/import_display_cabinets.py`, `Tools/create_store_surface.py`.
- Otomatik ekran/işlem kontrolü: `powershell -File Tools/StoreEditorReview.ps1`; görüntüler `Saved/Screenshots/StoreEditor/`.
- Düzenlenebilir kabukla gerçek oyun gezisi kontrolü: `powershell -File Tools/StoreTourTest.ps1 -EditableShell`; dört mağazada zemin/yürüyüş/rastgele dolum kontrol edilir, katalog değiştirilmez.

`editableShell`, `floorColor` (doğrusal RGB) ve `floorFinish` isteğe bağlı mağaza alanlarıdır. Eski mağazalar değiştirilene kadar Blender kabuklarını kullanır. Düzenlenen mağazada dış hat, depo, kapılar ve kolonlar çalışma anında instanced geometri ile kurulur; yüksek büyük mağazalarda tesisatlı tavan devam eder.
