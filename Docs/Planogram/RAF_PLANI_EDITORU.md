# Raf Planı Editörü

`RAF_PLANI.cmd`, Unreal Editor'ı doğrudan Raf Planı Editörü ile açar. Aynı pencereye Unreal içinde **Tools > Miras Market > Raf Planı Editörü** yoluyla da ulaşılır.

## Kavramlar

- **Gondol:** Fiziksel raf ekipmanı. Tek bir ürüne ait değildir.
- **Seviye:** Alttan üste 1–4 arasındaki raf tablası.
- **Önde:** Müşterinin aynı anda gördüğü yan yana ürün adedi (facing).
- **Derinlik:** Öndeki her ürünün arkasındaki sıra adedi. Önde 3, derinlik 4 seçimi en fazla 12 görünür paket yuvası üretir.
- **Yüz:** Çift taraflı gondolun ön veya arka koridor tarafı.
- **Sıra:** Aynı seviye ve yüzde farklı markaların soldan sağa dizilme düzeni.

## Kullanım

1. Soldan bir gondol seç.
2. Bir ürün başka gondoldaysa **Bu gondola taşı** düğmesine bas.
3. **Seviye**, **Ön** ve **Derinlik** düğmeleriyle ürün bloğunu ayarla.
4. İki içecek aynı gondolda ve aynı seviyedeyse yan yana marka blokları oluşturur.
5. Çift taraflı gondolda diğer koridora ürün koymak için **Yüzü çevir** kullan.
6. Değişiklikler her düğmede `Config/planograms.json` dosyasına atomik kaydedilir; oyunu yeniden başlatınca görünür.

Panel bir seviyedeki ürün blokları 110 cm kullanılabilir genişliği aşarsa yeni facing eklemez. Derinlik fiziksel stok kapasitesini değiştirmez; 24 birimlik oyun stoğunun raftaki görsel dağılımını belirler.

## Strateji düğmeleri

- **Dengeli:** Ürünleri seviyelere dağıtır; iki facing ve üç derinlik verir.
- **Kâr odaklı:** Brüt birim marjı yüksek ürünleri önce dizer; ilk bloklara daha çok facing ve dört derinlik verir.
- **Marka bloğu:** Ürünleri marka adına göre sıralar; aynı markanın ürünlerini görsel blok hâline getirir.

Bu düğmeler başlangıç düzeni üretir. Sonrasında her ürünü elle değiştirebilirsin. İleride müşteri davranışı ve satış verisi bu aynı planogram verisini kullanarak otomatik öneri üretecek.
