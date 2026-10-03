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