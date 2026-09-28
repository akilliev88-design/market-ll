# Raf Planı Editörü

`RAF_PLANI.cmd`, Unreal Editor'ı doğrudan Raf Planı Editörü ile açar. Aynı pencereye Unreal içinde **Tools > Miras Market > Raf Planı Editörü** yoluyla da ulaşılır.

## Kavramlar

- **Gondol:** Fiziksel raf ekipmanı. Tek bir ürüne ait değildir.
- **Seviye:** Alttan üste 1–4 arasındaki raf tablası.
- **Önde:** Müşterinin aynı anda gördüğü yan yana ürün adedi (facing).
- **Derinlik:** Öndeki her ürünün arkasındaki sıra adedi. Önde 3, derinlik 4 seçimi en fazla 12 görünür paket yuvası üretir.
- **Yüz:** Çift taraflı gondolun ön veya arka koridor tarafı.
- **Sıra:** Aynı seviye ve yüzde farklı markaların soldan sağa dizilme düzeni.
- **Konum:** Ürün bloğunun otomatik yerinden sola veya sağa santimetre cinsinden ince ayarı.
- **Yön:** Ambalajın önden, çeyrek tur çevrilmiş veya uygunsa yan yatırılmış duruşu.
- **İstif:** Bir raf gözünde üst üste duran ürün adedi. Yalnızca dengeli ambalajlarda ve raf yüksekliği elverdiğinde artırılabilir.

## Kullanım

1. Soldan bir gondol seç.
2. Bir ürün başka gondoldaysa **Bu gondola taşı** düğmesine bas.
3. **Seviye**, **Ön** ve **Derinlik** düğmeleriyle ürün bloğunu ayarla.
4. Raf şemasında bloğun gerçek genişliğini ve komşu markalara göre yerini gör. Ürün kartındaki küçük görsel, oyunda kullanılacak gerçek 3B ambalajın önizlemesidir.
5. **Sola 5 cm** / **Sağa 5 cm** ile bloğu rafta elle kaydır. Raf kenarına taşan veya başka marka bloğuyla çakışan hareket reddedilir.
6. **Yönü değiştir** ile önden, çeyrek tur veya ambalaj uygunsa yan yatırılmış duruşlar arasında geç. **İstif + / -** ile ürünleri üst üste koy.
7. İnce ayar yapıldığında otomatik dolum kapanır; böylece seçtiğin konum, yön ve istif oyun açılırken değiştirilmez. **İnce ayarı sıfırla** varsayılan dizilişe döndürür.
8. İki içecek aynı gondolda ve aynı seviyedeyse yan yana marka blokları oluşturur.
9. Çift taraflı gondolda diğer koridora ürün koymak için **Yüzü çevir** kullan.
10. Değişiklikler her düğmede `Config/planograms.json` dosyasına atomik kaydedilir; oyunu yeniden başlatınca görünür.

Bir seviyedeki ürün blokları 110 cm kullanılabilir genişliği aşamaz: seviye, önde adet, yüz, taşıma ve stratejiler sığmayan değişikliği reddeder; editör her seviyenin doluluğunu (cm) ve taşmaları gösterir.

**Raf kapasitesi = önde × derinlik × istif.** Oyundaki raf stoğu bu sayı kadar ürün alır (sabit 24 sınırı yok). İstif sınırı ürün yüksekliği ile rafın kullanılabilir yüksekliğinden hesaplanır.

Fiyat rayı, raftaki ürünü tutan yüksek bir bariyer değildir. Blender raflarında ve kodla üretilen yedek geometride raf tablasının ön altına asılan yaklaşık 4 cm yüksekliğinde ince bir etiket profili olarak modellenir.

**Ekipmanlar:** `gondola_double_1200` (çift yüz, 4 seviye, 110 cm) ve `wall_shelf_2400` (duvar reyonu, tek yüz, 5 kullanılabilir seviye, 230 cm). Ölçüler `Planogram.cpp` → `MarketPlanogram::Equipment`.

**Otomatik dolum** (`planograms.json` → `autoFill`, varsayılan açık): oyun açılırken önce her ekipmandaki boş seviye/yüzlere o ekipmana ait ürünlerden (orada yerleştirilmiş ürünler, sonra aynı kategorideki ürünler) ek blok koyar; sonra her bloğu raf derinliğince diziler ve seviyedeki boş genişliği markalara eşit paylaştırarak önde adedi artırır. Ek bloklar dosyaya yazılmaz. Böylece elde kaç ürün varsa raf o kadarıyla dolar; boş kalan seviyeler yalnızca o seviyeye ürün atanmadığı içindir. Editördeki önde adedi bu durumda en az değerdir; derinlik ürünün kataloğdaki derinliğinden hesaplanır. Düğmeyle kapatılırsa oyun editördeki sayıları aynen kullanır.

## Strateji düğmeleri

- **Dengeli:** Ürünleri seviyelere dağıtır; iki facing ve üç derinlik verir.
- **Kâr odaklı:** Brüt birim marjı yüksek ürünleri önce dizer; ilk bloklara daha çok facing ve dört derinlik verir.
- **Marka bloğu:** Ürünleri marka adına göre sıralar; aynı markanın ürünlerini görsel blok hâline getirir.

Bu düğmeler başlangıç düzeni üretir. Sonrasında her ürünü elle değiştirebilirsin. İleride müşteri davranışı ve satış verisi bu aynı planogram verisini kullanarak otomatik öneri üretecek.

## Dosya biçimi

`Config/planograms.json` şema v2'dir. Eski v1 dosyaları okunur; eksik alanlar `offsetCm: 0`, `orientation: 0`, `stack: 1` kabul edilir. Yeni alanlar yalnızca varsayılandan farklıysa kaydedilir. `orientation` değerleri: `0` önden, `1` çeyrek tur, `2` yan yatırılmış.
