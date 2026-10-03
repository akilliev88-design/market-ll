# BUZ dolap tipi karşılaştırması — 03.10.2026

İstek: Mustafa'nın BUZ teşhir reyonları sayfasındaki tiplerden oyuna eklenmeyenleri kontrol et.

## Kapsam ve kanıt

Ana sayfa web okuyucusunda açılamadı. Erişilebilen resmi alt sayfalar, resmi ürün arama sonuçları ve önceki araştırmada saklanmış Saved/BuzReference/reference_sheet.jpg karşılaştırıldı. Tüm üretici ürünlerinin birebir envanteri değildir; doğrulanmış tip/varyant açıklarıdır. Üretici ölçüleri oyun modellerinin ölçüsü sayılmaz.

Projedeki 12 BUZ referanslı dolabın Blender kaynağı ve Unreal uasset'i mevcut: açık sütlük, alçak sütlük, cam kapılı sütlük, üç kapılı dikey dondurucu, tek ve iki kapılı içecek dolabı, kasap ve şarküteri tezgâhı, havuz ve kutu dondurucu, pasta ve dondurma vitrini. Diğer üç yeni ekipman teknoloji/hazırlık ekipmanıdır.

## Eksikler

| Tip / varyant | Projedeki durum | Kaynak |
|---|---|---|
| METEOR: üstte yarım boy cam kapılı dikey bölüm + altta yatay dondurucu | Birleşik model yok; dikey ve havuz ayrı | https://www.buzrefrigeration.com/tr/urunler/dikey-dondurucular/meteor?lang=ar |
| SENNA ada baş modülü | Tek dikdörtgen havuz var; ada başını kapatan ayrı modül yok | https://www.buzrefrigeration.com/tr/urunler/havuz-tipi-dondurucular/senna?lang=ar |
| Üç kapılı içecek soğutucusu | Tek/iki kapılı var; üç kapılı dondurucu farklı ekipman | https://www.buzrefrigeration.com/tr/urun/dikey-sogutucular |
| Dikey dondurucunun 2/4/5 kapılı boyları | NEFAS ailesinin kapı seçeneklerine karşılık yalnız üç kapılı model var | https://www.buzrefrigeration.com/tr/urunler/dikey-dondurucular/nefas-lm?lang=ar |
| Köşeli/dönüşlü servis ve pasta tezgâhları | Kaydedilmiş referans görsellerinde dönüşlü hatlar var; oyun modelleri düz modüller | Saved/BuzReference/reference_sheet.jpg; https://www.buzrefrigeration.com/uploadedfiles/download/tuvan-compressed.pdf |
| İçten motorlu servis reyonu ailesine özgü gövdeler | Genel kasap/şarküteri vitrinleri mevcut; bu aileye özel model/metadata referansı yok | https://buzrefrigeration.com/tr/urunler/icten-motorlu-servis-reyonlari |

Önerilen ilk üretim sırası: METEOR birleşik dondurucu, ada baş modülü, köşe servis modülü. Bunlar yerleşim ve siluete yeni seçenek verir. Kapı sayıları ve renk varyantları sonraki tur olabilir. Bu yalnız inceleme; yeni model üretilmedi.
## Mağaza düzenleme karşılaştırması

03.10.2026 ek inceleme: Web okuyucusunda açılmayan sayfalar PowerShell Invoke-WebRequest ile doğrudan okunabildi. Ana sayfa Raf Sistemleri, Süpermarket Kasaları ve Süpermarket Aksesuarları olarak üç aile sunuyor. Aksesuar alt sayfasındaki ürün adları doğrudan doğrulandı.

Mevcut: gondol ve duvar rafları, temel ekmek rafı, küçük/büyük manav, tek/çift kasa ve detaylı kasa hattı, el sepeti ve metal alışveriş arabası alanı, palet teşhiri, genel ev/tekstil/teknoloji rafları.

Eksik veya ayrı modele dönüştürülmemiş:
- Kasa önü küçük ürün teşhir ünitesi: kasa hattı var, bağımsız kasa önü rafı yok.
- Depo raf ünitesi: depo hacmi/kapısı ve koliler var, özel raf kütüphanesi yok.
- Tel teşhir aksesuarları ve tel askı sepetleri.
- Bariyer ve geçiş sistemleri.
- Tekerlekli alışveriş sepeti, çocuk arabası ve plastik alışveriş arabası çeşitleri: metal araba ve el sepeti mevcut.
- Eczane, hırdavat/kütüphane, yapı market için özel üniteler: genel raflar var, bu amaçlara özgü ayrı modeller yok.

Ekmek, manav ve kasaların temel karşılığı mevcut; üreticinin her görsel/ölçü varyantı modellenmedi. Bunları tümüyle eksik diye saymamak gerekir. Depo kategorisinin sayfasında ürün adları okunamadığından model bazında sayı verilmedi.

Kaynaklar:
- http://www.buzrefrigeration.com/tr/urun/magaza-duzenleme
- http://www.buzrefrigeration.com/tr/urunler/kasa-onu-uniteleri
- http://www.buzrefrigeration.com/tr/urunler/depo-uniteleri
- http://www.buzrefrigeration.com/tr/urunler/supermarket-aksesuarlari
- http://www.buzrefrigeration.com/tr/urunler/ekmek-raflari
- http://www.buzrefrigeration.com/tr/urunler/meyve-sebze-uniteleri
- http://www.buzrefrigeration.com/tr/urunler/supermarket-kasalari

Önerilen sıra: kasa önü teşhiri, depo rafı, tel askı sepeti ve giriş bariyeri. Bunlar mevcut market türlerinde doğrudan kullanılabilir. İnceleme sırasında yeni model/kod üretilmedi.