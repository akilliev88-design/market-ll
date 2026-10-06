# Raf Planı Editörü

`RAF_PLANI.cmd`, Unreal Editor'ı doğrudan Raf Planı Editörü ile açar. Aynı pencereye Unreal içinde **Tools > MarketSim > Raf Planı Editörü** yoluyla da ulaşılır.

## Kavramlar

- **Gondol:** Fiziksel raf ekipmanı. Tek bir ürüne ait değildir.
- **Seviye:** Alttan üste 1–4 arasındaki raf tablası.
- **Önde:** Müşterinin aynı anda gördüğü yan yana ürün adedi (facing).
- **Derinlik:** Öndeki her ürünün arkasındaki sıra adedi. Elle seçilmez; raf derinliği ve ambalaj derinliğinden hesaplanır. Önde 3, derinlik 4 en fazla 12 görünür paket yuvası üretir.
- **Yüz:** Çift taraflı gondolun ön veya arka koridor tarafı.
- **Blok:** Bir rafta yan yana duran aynı ürün grubu (önde × derinlik × kat). Bir ürünün birden çok bloğu olabilir.
- **Konum:** Bloğun merkezinin raf ortasına göre yeri (cm). Blok konduğu yerde kalır.
- **Yön:** Ambalajın önden, çeyrek tur çevrilmiş veya uygunsa yan yatırılmış duruşu.
- **İstif:** Bir raf gözünde üst üste duran ürün adedi. Yalnızca dengeli ambalajlarda ve raf yüksekliği elverdiğinde artırılabilir.

## Temel kural

Raflarda **yalnızca senin koyduğun ürün blokları** durur. Her blok konduğu yerde (raf ortasına göre cm) sabit kalır; oyun hiçbir şeyi kendiliğinden koymaz, kaydırmaz, genişletmez. **Aynı ürünü istediğin kadar** koyabilirsin (aynı rafa yan yana, başka rafa, başka reyona). Katalogdaki yeni ürün **Rafta değil** olarak gelir; rafa koyana kadar satılmaz (stoğu depoda bekler). Derinlik tek otomatik değerdir: blok, raf derinliğine ambalaj derinliği kadar sığan sırayla dizilir. Bloklar **dip dibe** konabilir (0,3 cm tolerans); istenirse her blok için **aralık** (Z / X, editörde Aralık − / +) verilir, blok komşularıyla en az o kadar mesafe bırakır. Bir bloğun önündeki ürünler arasında 0,5 cm, derinlikteki sıralar arasında 2 cm vardır.

## Oyun içinde dizme (R) — önizlemeli

Market **kapalıyken** R'ye bas. Ekranın ortasındaki noktayı herhangi bir rafın bir seviyesine getir:

- **Yeşil şerit + ürünlerin hayaleti:** elindeki ürün tıklarsan tam orada duracak. Nişan dolu bir yere denk gelirse blok en yakın boşluğa yapışır (en fazla yarım blok + 15 cm kayar); yer darsa önde adet azaltılır ve panel bunu yazar.
- **Kırmızı şerit:** buraya sığmıyor; panel nedenini ve en geniş boşluğu yazar.
- **Turuncu şerit:** nişandaki mevcut blok. **Mavi şerit:** F ile alınıp taşınan blok. **Gri şeritler:** nişan alınan raftaki bütün bloklar — stoğu olmadığı için boş görünen ama yeri ayrılmış bloklar da böylece görünür.
- Sağdaki **RAF DÜZENİ** paneli: reyon / yüz / raf, rafın doluluğu ve en geniş boşluğu, elindeki ürünün ölçüsü-yönü-önde adet-kat-derinlik-kapasitesi, ürünün raf stoğu ve depo stoğu, nişandaki blok, tıklarsan ne olacağı ve o an çalışan tuşlar.

| Tuş | İş |
|---|---|
| Sol tık / E | Elindekini buraya koy — tekrar tekrar (taşırken: bırak) |
| Tekerlek / TAB / Q | Elindeki ürünü değiştir |
| Sağ tık / DEL | Nişandaki bloğu kaldır |
| F | Nişandaki bloğu al ve taşı (tekrar F: iptal) |
| C | Nişandaki bloğu eline al (aynısını başka yere koymak için) |
| + / − | Nişan bir bloktaysa onun, değilse elindekinin önde adedi |
| Y / U | Aynı kuralla yön / kat (üst üste) |
| Z / X | Aynı kuralla aralık − / + (0 = dip dibe) |
| Sol / Sağ ok | Nişandaki bloğu 5 cm kaydır (komşuya veya kenara dayanınca durur) |
| Yukarı / Aşağı ok | Nişandaki bloğu üst / alt rafa al |
| R | Bitir |

Her değişiklik anında `Config/planograms.json` dosyasına yazılır ve raf ekranda yeniden kurulur. Test modunda değişen ürünün rafı bedava dolar. Küçülen veya kaldırılan bloğun fazlası depoya döner.

## Editörde kullanım (`RAF_PLANI.cmd`)

1. Soldan bir reyon seç. **Bu reyondaki ürün blokları** seviye seviye, soldan sağa listelenir (konum cm, önde, derinlik, kat, adet).
2. Her blokta: **← 5 cm / 5 cm →**, **Önde − / +**, **Yönü değiştir**, **Kat − / +**, **Seviyeye taşı S1…**, **Yüzü çevir**, **Aynısından ekle**, **Kaldır**.
3. **Ürün ekle** listesinde her ürün için Ön/Arka **S1…** düğmeleri, ürünün yeni bir bloğunu o seviyenin sağ ucundaki boşluğa koyar. Rafta olmayan ürünler en üstte, turuncu kartla durur.
4. Son işlemin sonucu (veya neden yapılmadığı) başlığın altında sarı yazıyla görünür.
5. Oyun ve editör aynı kuralları kullanır (`Source/MarketSim/PlanogramEdit.*`).

Bir seviyedeki ürün blokları rafın kullanılabilir genişliğini (gondol 116 cm, duvar reyonu 235 cm) aşamaz: seviye, önde adet, yüz ve taşıma sığmayan değişikliği reddeder; editör her seviyenin doluluğunu (cm) ve taşmaları gösterir.

**Raf kapasitesi = önde × derinlik × istif.** Oyundaki raf stoğu bu sayı kadar ürün alır (sabit 24 sınırı yok). İstif sınırı ürün yüksekliği ile rafın kullanılabilir yüksekliğinden hesaplanır.

Fiyat rayı, raftaki ürünü tutan yüksek bir bariyer değildir. Blender raflarında ve kodla üretilen yedek geometride raf tablasının ön altına asılan yaklaşık 4 cm yüksekliğinde ince bir etiket profili olarak modellenir.

**Ekipmanlar:** `gondola_double_1200` (çift yüz, 4 seviye, 116 cm — rafın tamamı; dikmeler ürünlerin arkasında) ve `wall_shelf_2400` (duvar reyonu, tek yüz, 5 kullanılabilir seviye, 235 cm). Ölçüler `Planogram.cpp` → `MarketPlanogram::Equipment`.

**Otomatik dolum kaldırıldı** (28.09.2026, Mustafa kararı). Eski dosyalardaki `autoFill` alanı okunur ama etkisi yoktur ve bir sonraki kayıtta dosyadan çıkar.

## Reyon görevlisi (çalışanların dizmesi)

Yönetim masasında **J** reyon görevlisi alır (en fazla 3; 120 TL + günlük 20 TL), **K** birini çıkarır. Görevliler market açıkken de kapalıyken de çalışır; depoya yürür, koliyi alır, reyona taşır ve ürünleri rafa tek tek koyar. Sırayla:

1. **Yarısı veya daha azı dolu raf:** en boş olandan başlayarak depodan doldurur.
2. **Depoda olup rafta yeri olmayan ürün:** ürünün kategorisindeki reyonlarda (reyonun `category` alanı ürünün kategorisiyle aynı) boş bir yer bulur. Aynı markanın yanını, bir koliyi alacak kadar önde adedi (2–6), göz hizasını ve boşluk bırakmayan yeri tercih eder. Yer yoksa bir kez haber verir ("... reyonlarında yer kalmadı" veya "... için reyon yok").
3. **Beşte biri eksik raf:** tamamlar.
4. **Bir koliyi almayan blok:** yanında yer varsa önde adedini 1 artırır (en fazla bir ürün genişliği kadar kayabilir).

Kurallar oyuncunun dizme kurallarıyla aynıdır (`PlanogramEdit`): görevli yalnız boş yere blok ekler veya bloğu boşluğa doğru genişletir; hiçbir bloğu taşımaz, daraltmaz, silmez. Oyuncu R ile dizerken görevliler raf planına dokunmaz, yalnız doldurur. Ürünler depodan rafa birer birer geçer; iş yarıda kalırsa stok kaybolmaz. Kararlar `StaffPlanner.*`, oyundaki hareket `MarketWorkers.cpp`.

Eski **Dengeli / Kâr odaklı / Marka bloğu** düğmeleri 28.09.2026'da kaldırıldı (G-048). `strategy` alanı eski dosyalarda kalsa da okunmaz ve bir sonraki kayıtta çıkar.

## Dosya biçimi

`Config/planograms.json` şema v3'tür: her blok `x` (raf ortasına göre blok merkezi, cm) taşır ve aynı `productId` birden çok blokta geçebilir. Eski v1/v2 dosyaları okunur; `order`/`offsetCm` ile eski dizilişin gösterdiği yer bir kez hesaplanıp `x` olur ve sonraki kayıtta v3 yazılır. `orientation` değerleri: `0` önden, `1` çeyrek tur, `2` yan yatırılmış.
