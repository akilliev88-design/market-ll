# Geliştirme planı ve Unreal yaklaşımı

> Bu dosya ilk prototipin tarihsel geliştirme planıdır. Şu anki çalışma yalnızca planlamadır; yeni kapsam ve uygulama kapıları [Planlama v0.2](Planlama/00_OKUMA_REHBERI.md) paketindedir.

## Teknik başlangıç

Bu çalışma alanında Unreal Engine 5.8.3 kurulu olduğu doğrulandı. C++ oyun projesi bu sürümün hedef ayarlarıyla oluşturuldu. Sahne, oyuna girildiğinde temel geometrilerden üretilir; editörde haritayı açmak tek başına marketi göstermez, Play gerekir. Başlangıç haritası motorun Entry haritasıdır.

Şu anki yapı:

- `MarketEconomy`: para, stok, sipariş, satış ve gün sonu hesabı; dünyadan bağımsız test edilebilir kurallar.
- `MarketGame`: birinci şahıs karakter, prototip sahne, müşteri hareketi, etkileşim, yönetim komutları, kayıt ve HUD.
- `MarketTests`: stok korunumu, nakit hareketleri, satış maliyeti, personel/şube katkısı ve kayıt serileştirmesi için otomasyon.
- `Config/products.json`: ürün kimliği, gerçek/kurgu adlar, maliyet, fiyat ve prototip rengi.
- `Content/Materials/M_Market`: renk parametresi olan ortak prototip materyali; üretim betiği `Tools/create_material.py`.

Bu küçük sürümde GameMode sahne ve akışı birlikte yönetir. Yeni sistemler eklenirken aşağıdaki sorumluluklara ayrılması gerekir; mevcut kodun tüm oyun için nihai mimari olduğu varsayılmamalı.

| Sistem | Hedef sorumluluk |
|---|---|
| Campaign subsystem | Takvim, şirket, şubeler, kalıcı kimlikler |
| Economy subsystem | İşlem defteri, nakit, alacak/borç, gün sonu |
| Inventory component | Parti, lokasyon, rezervasyon ve stok hareketi |
| Store simulation | Yerel ve uzak şubeler için ortak ticari kurallar |
| Interaction interface | Bakılan nesne, uygun eylem ve oyuncu geri bildirimi |
| Product/brand definitions | Sunumdan bağımsız ürün kimlikleri ve içerik |
| Customer controller | NavMesh, alışveriş listesi, karar ve kuyruk |
| Staff task system | Öncelik, görev atama, vardiya, mola |
| Rival simulation | Bütçeli ve açıklanabilir rekabet kararları |
| Save service | Sürüm, doğrulama, geçiş, atomik kayıt, yedek |
| UMG interface | Yönetim bilgisayarı, raporlar, ayarlar ve öğretici |

Prototipte ürün tanımları JSON'dan okunur. İçerik büyüdüğünde Data Assets/Data Tables ile tanımların görsellerden ve değişen şube durumundan ayrılması hedeflenir. Epic'in [veriye dayalı oyun sistemi belgesi](https://dev.epicgames.com/documentation/unreal-engine/data-driven-gameplay-elements-in-unreal-engine) ve [Data Assets belgesi](https://dev.epicgames.com/documentation/unreal-engine/data-assets-in-unreal-engine) bu ayrımın motor tarafındaki referanslarıdır. Şu an bu Data Table yapısı uygulanmış değildir.

## Aşamalar ve bitiş koşulları

### 0 — Teknik ve ekonomik prototip

Küçük gezilebilir mağaza, altı ürün, raf doldurma, ödeme alma, günlük sipariş, fiyat, basit rakip indirimi, kasiyer, gün raporu ve kayıt. İkinci şube yalnızca günlük net katkıyla temsil edilir. Ayrıntılı müşteri animasyonu ve dekor yoktur.

Bitiş: proje derlenir; ekonomi ve kayıt testleri geçer; sahne motor içinde açılır; kullanıcı ilk günü oynayabilir.

### 1 — İlk markette iyi bir oyun günü

Gerçek koli ve elde taşıma, raftaki ürün miktarına göre değişen görseller, mal kabul noktası, barkod/kasa etkileşimi, fareyle yönetim ekranı, sesler, küçük hikâye girişi. Müşteri rotaları NavMesh ve kuyruk noktalarıyla düzeltilir. Raf/duvar içinden geçen karakterler kaldırılır.

Bitiş: yeni oyuncu 15–20 dakikada yardım almadan satış ve sipariş yapar; tüm müşteri kayıpları nedenleriyle görünür; kayıt sonrası stok ve kasa eşleşir.

### 2 — Mahalle rekabeti

20–40 ürün, kategori farkları, parti/bozulma, iki farklı rakip stratejisi, tedarikçi koşulları, bütçeli kampanya, yerel müşteri segmentleri ve aile hikâyesi görevleri. Nakit krizi/toparlanma, iadeler ve veresiye eklenir.

Bitiş: en az üç farklı fiyat/ürün stratejisi oynanabilir; rakip indirimleri oyuncuya anlaşılır nedenler ve karşı hamleler sunar; tek baskın ürünle ekonomiyi sonsuz kazanca çeviren açıklar giderilir.

### 3 — Yetki devri ve gerçek ikinci şube

Reyon görevlisi, vardiya, müdür, şube seçimi, ayrı stok/kasa, konum ve kiralar. İkinci şube kendi taleplerine göre ürün alır ve satar; mevcut sabit katkı modeli kaldırılır.

Bitiş: oyuncu bir şubeyi ziyaret ederken diğeri çalışır; ziyaret değişiminde stok/para çift sayılmaz; yeterli personelle ilk mağazadaki rutin işler oyuncusuz yürür.

### 4 — Trakya operasyonu

Depo, araç kapasitesi, sevkiyat, soğuk zincir, merkezi satın alma, şube şablonları ve bölgesel rakipler.

Bitiş: merkezi depo avantaj ve maliyet yaratır; gecikme rafta ve raporda görünür; uzak şube sayısı arttığında performans ölçümleri kabul edilebilir kalır.

### 5 — Ulusal şirket

Türkiye bölgeleri, bölge müdürlükleri, kurumsal finans, özel marka, reklam ve tedarik sözleşmeleri. Satın almalar ve franchise isteğe bağlı sonraki sistemlerdir.

Bitiş: genişleme, marj, nakit ve hizmet arasında gerçek kararlar vardır; yönetim ekranı çok sayıda şubeyi anlaşılır biçimde filtreler ve önceliklendirir.

### 6 — Uluslararası büyüme ve kampanya sonu

Ülke bazlı ürün talebi, yerel tedarik, para birimi, teslimat süresi ve genişleme modelleri. Uluslararası liderlik için birden fazla sürdürülebilir başarı ölçütü.

Bitiş: ülke seçimi farklı oynanış yaratır; kampanya kazanılabilir ve sonrasında serbest devam edilebilir.

## Kalite ve üretim sınırları

Süre tahmini ekip, görsel hedef ve içerik bütçesi netleşmeden verilmez. Bu kapsam uzun süreli ve aşamalı geliştirme gerektirir. Her aşama oynanabilir bir sürüm bırakmalı.

Öncelikli riskler: manuel işlerin sıkıcılaşması, tek ekonomik stratejinin baskın olması, çok şubeli simülasyonda çift işlem, dönem varlıklarının üretim maliyeti ve yapay zekâ yol bulma sorunları.

Testler yalnızca fonksiyonları tekrar etmemeli: para/stok korunumu, negatif veya iki kez satış, kayıt uyumluluğu, teslimatın iki kez uygulanması, kuyrukta fiyat değişimi ve aktif/uzak şube geçişi gibi oyuncuya yansıyan hataları yakalamalı.

Görsel varlıklar ilk aşamada geçici modellerdir. Sonraki aşamada ölçülü raflar, kasa, ürün ambalajları, bina cephesi, personel ve müşteri modelleri üretilir veya uygun içerik paketlerinden seçilir. Henüz dış varlık paketi satın alınmadı.
