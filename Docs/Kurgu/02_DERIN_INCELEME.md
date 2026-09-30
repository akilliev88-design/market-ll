# Miras Market — Derin İnceleme ve Yeniden Tasarım Önerisi

Tarih: 29.09.2026 · Hazırlayan: Claude (5 ayrı inceleme ajanı + çapraz kontrol)
Kapsam: `Source/MirasMarket` (oyun kodu), `Config/*.json`, `Docs/Kurgu`, `Docs/Planlama`, `Docs/Surec`.
Kodda hiçbir değişiklik yapılmadı. Satır numaraları 29.09.2026 tarihli dosyalara göredir.

Bu belge dört bölümden oluşur:

1. **Özet teşhis:** oyun neden "basit kalmış" hissi veriyor.
2. **Mantık hataları:** önem sırasına göre, dosya:satır referanslı.
3. **Yeniden tasarım:** dünya, rekabet, fiyat/kampanya, ekonomi, oyun döngüsü ve hikâye.
4. **Yol haritası:** hangi sırayla yapılmalı.

---

## 1. Özet teşhis

Oyunun iskeleti geniş: takvim, enflasyon, toptancı, veresiye, banka, personel, olaylar, hikâye bölümleri, şubeler, şehirler hepsi var. Sorun **genişlikte değil, derinlikte ve bağlantıda**. Sistemlerin çoğu birbirini gerçekten etkilemiyor:

| Belirti | Kök neden |
|---|---|
| Rakipler "cansız" | Zincirlerin parası, kararı, hafızası yok. Bereket'in fiyat savaşı koddaki bir hata yüzünden pazar payına hiç işlemiyor. Bakkal, semt pazarı, hipermarket yok. |
| Kampanyalar anlamsız | Fiyat esnekliği neredeyse yok: %20 indirim satın alma olasılığını yalnızca 2–3 puan artırıyor. Kampanyalar yalnızca reyona ya da tek ürüne uygulanıyor, mekanikler sabit. |
| Harita süs | 15 şehrin hepsi ekonomik olarak aynı; İstanbul en kötü şehir. Harita üzerinde karar yok. |
| "Dünya 1 numarası" hedefi yok | Son bölümün kazanma şartı aslında **İstasyon semtinde %40 pay + 60 mağaza**. Oyuncu hiçbir küresel devle karşılaştırılmıyor. |
| Büyüme bir para makinesi | Şehir mağazası 60–110 günde yatırımını çıkarıyor, riski ve rakip tepkisi yok. |
| Elle oynamak cezalı | "İlerlet" düğmesi 1. günden açık ve kusursuz; elle oynamaktan hep daha kârlı. |
| Başarısızlık baskısı zayıf | Vade kaldıracı çalışmıyor, KDV kâr sayılıyor, müşavir istismar edilebiliyor. |

**Tek cümle:** Oyunun "zekâsı" şu an çarpanlardan oluşuyor; karar veren aktörlerden oluşmuyor. Önerinin özü, her rakibe, müşteri segmentine ve ülkeye kendi bütçesi, hafızası ve kararı olan bir model vermek. Oyuncu da bu dünyada katman katman (tezgâh → holding) yükselsin.

---

## 2. Mantık hataları (önem sırasına göre)

İşaretler: 🔴 kritik (oyunu bozuyor ya da istismar edilebilir) · 🟠 yüksek (dengeyi bozuyor) · 🟡 orta.
✔ = bu raporu yazarken kodda ayrıca doğrulandı.

### 2.1 Kayıt, test modu, hikâye akışı

| # | Önem | Hata | Yer |
|---|---|---|---|
| 1 | 🔴 ✔ | **Test modu varsayılan açık.** `bTestModeAtStart=True`. F2/F3 rafları, "+" düğmesi siparişi bedava yapıyor. Test modu kayda yazılmıyor. | `Config/DefaultGame.ini:15`, `MarketGame.cpp:151, 968-976`, `MarketMenuWidget.cpp:1231` |
| 2 | 🔴 | **Kayıt üzerine yazılıyor.** Açılış her zaman yeni oyun; tek kayıt yuvası var ve gün sonunda otomatik kayıt yapılıyor. F9'a basmayı unutan oyuncunun uzun kampanyası siliniyor. | `MarketGame.cpp:33-36, 131-169, 1456` |
| 3 | 🔴 ✔ | **"Burada bitsin" (bölüm 99) oyunu kırıyor.** `ChapterOpen` = `Chapter >= N`, bu yüzden 99 her bölümü açık sayıyor. Oyuncu dükkânı satmış sayılıyor ama hem dükkânı hem parayı tutuyor. "Miras" sonu yalnızca `Chapter == 7` iken geldiği için bir daha hiç gelmiyor. | `MarketCompany.cpp:104-108, 301`, `MarketStory.cpp:357` |
| 4 | 🔴 | **"Rüyaymış" para açığı.** Satış parası 3 gün kasada kalıyor, sonra geri alınıyor. O sürede harcanırsa kasa derin eksiye düşüyor ve yatırımlar kalıyor. | `MarketStory.cpp:344, 351` |
| 5 | 🟠 | **Raf düzeni kayda değil `Config/planograms.json`'a yazılıyor.** Bütün kampanyalar aynı düzeni paylaşıyor. Yüklemede sığmayan mal sessizce yarı fiyatına satılıyor. | `MarketWorkers.cpp:84-90`, `MarketGame.cpp:211-235` |
| 6 | 🟠 | **Kayıt sürümü sabit 1.** Yüklenen veri doğrulanmıyor. Yeni alan eklendikçe eski kayıtlar bozulur. | `MarketEconomy.cpp:237` |
| 7 | 🟡 | **Kayıt-yükle ile geleceği bilmek.** Bütün sonuçlar tohum + gün ile belirleniyor (zabıta, veresiye, olaylar). Önerilen çözüm: sonuç **durum + tohum** ile hesaplansın, oyuncunun hamlesi sonucu değiştirsin. | genel |

### 2.2 Zaman ve oyun döngüsü

| # | Önem | Hata | Yer |
|---|---|---|---|
| 8 | 🟠 | **İlerletme kusursuz ve her zaman açık.** Kasiyer olmadan anında kasa, otomatik vergi/borç/zam, önerilen sipariş. Birinci şahıs oynamak ceza gibi kalıyor. Plan 01 §3 "yalnızca işler devredildiyse" diyor. | `MarketMenu.cpp:240-255`, `MarketSimulation.cpp` `PlayDay` |
| 9 | 🟠 | **Dükkân kapalıyken zaman donuyor.** Sabah hazırlığının bedeli yok. O tuşu her an günü bitiriyor; "ilk hafta" hedefleri saniyeler içinde geçilebiliyor. | `MarketGame.cpp:913-915, 1296-1297` |
| 10 | 🟠 | **Menü açıkken zaman akıyor, oyuncu kasaya gidemiyor** (G-075). KARARLAR A02 ise "menüde zaman durur" diyor: belge ile kod çelişiyor. | `MarketMenu.cpp:107` |
| 11 | 🟡 | **Zorluk oyun sırasında serbestçe değişiyor** ve kayda işlenmiyor ("1 hafta" ilerletmeden önce Rahat'a geçilebiliyor). | `MarketMenuWidget.cpp:692-696` |
| 12 | 🟡 | **"1 hafta" düğmesi 7 gün ilerletmiyor**, hafta sonunda duruyor. %22 olay ihtimali yüzünden pratikte 4–5 günde bir kesiliyor. | `MarketMenuWidget.cpp:1061-1062` |
| 13 | 🟡 | 1x hızla 2011→2030 yaklaşık **460 saat**. Aylık/çeyreklik tur olmadan hikâyenin 2018, 2020 ve 2023 dönemlerine ulaşmak fiilen imkânsız. | tasarım |

### 2.3 Rekabet ve pazar payı

| # | Önem | Hata | Yer |
|---|---|---|---|
| 14 | 🔴 ✔ | **Bereket'in fiyat savaşı paya hiç işlemiyor.** Pay hesabı `PriceIndex`'i boş kategori (`FString()`) ile çağırıyor. Savaş indirimi ise yalnızca kategori adı eşleşirse uygulanıyor. `MarketRivals`'ın reyon indirimleri, zamları ve stok-yok haberleri de aynı nedenle paya girmiyor. | `MarketCompetitors.cpp:82, 160, 182` |
| 15 | 🔴 | **Paylar %100 etmiyor.** Rakip payları o günün anlık değeri, bizim payımız yavaş hareketli ortalama ve [5, 65] ile sınırlı. Menüde toplam %85–115 arasında dolaşıyor. | `MarketCompetitors.cpp:187, 192` |
| 16 | 🔴 | **Payımız arttıkça ilçe küçülüyor.** Müşteri sayısı √pay ile artıyor; online modülü toplam pazarı `ziyaretçi/pay` ile hesaplıyor, sonuç 1/√pay. | `MarketCompetitors.cpp:113`, `MarketOnline.cpp:61` |
| 17 | 🟠 | **A101 çift sayılıyor.** Hem pay modelinde rakip, hem de trafiğimizi kalıcı olarak ×0,95 yapıyor. "Uzun saat" haberi de rakibin çekiciliğine değil doğrudan bizim trafiğimize dokunuyor. | `MarketRivals.cpp:57`, `MarketDirector.cpp:84-87` |
| 18 | 🟠 | **Zincirlerin ekonomisi yok.** 500.000 TL nakit hiç kullanılmıyor. Fiyat, hizmet ve mağaza sayısı hiç değişmiyor. Zorluk ayarı rakipleri etkilemiyor. | `MarketCompetitors.cpp:43-47`, `MarketSimulation.cpp:398-406` |
| 19 | 🟠 | **Kampanya sayısı istismarı.** Çekiciliği indirim derinliği değil **kampanya adedi** belirliyor (adet başına +%8). Ucuz ürünlere üç "3 al 2 öde" konursa kalıcı +%24 çekicilik geliyor. | `MarketCompetitors.cpp:146` |
| 20 | 🟠 | **Bereket öngörülebilir.** Hedefi "dün en çok satan reyon", eşitlikte alfabetik sıra. Oyuncu savaşı istediği reyona çekebiliyor. Bereket hiç kapanmıyor, satılığa çıkmıyor; kurgu E03 (Bereket'i satın alma) uygulanmamış. | `MarketCompetitors.cpp:195-240` |
| 21 | 🟡 | **Menü ile simülasyon farklı rakip fiyatı gösteriyor.** Menü "en ucuz zincir"i gösteriyor, Bereket ve Şok yok; simülasyon ise ağırlıklı ortalama kullanıyor. Oyuncu yanlış bilgiye göre karar veriyor. | `MarketMenuWidget.cpp:130-140` |
| 22 | 🟡 | **Personel ayartma hatalı.** A101 ilçeye gelmeden ayartma yapabiliyor; Migros ve Şok hiç yapmıyor. | `MarketCompetitors.cpp:262-280` |
| 23 | 🟡 | **Tarih hataları.** Harita 2011'de Kırklareli'de Şok mağazaları gösteriyor, rakip modülü ise Şok'u 131. günde "getiriyor". Şok 2011'de zaten vardı, yalnızca sahibi değişti. | `MarketCompetitors.cpp:47`, `iller.json` |

### 2.4 Fiyat, kampanya, katalog

| # | Önem | Hata | Yer |
|---|---|---|---|
| 24 | 🔴 | **Fiyat esnekliği yok denecek kadar az.** P(alır) oran 0,8'de 0,998, 1,0'da 0,973. Rakibin altına inmek kazandırmıyor, üstüne çıkmak sert cezalandırılıyor. Optimum fiyat her üründe listenin %6–18 üstü. | `MarketDemand.cpp:13-17` |
| 25 | 🔴 | **Kampanya satışı başka reyondan çalıyor.** Müşteri listesinin uzunluğu sabit; kampanya yalnızca hangi kalemin seçileceğini değiştiriyor. Süte indirim yapınca deterjan satışı düşüyor. Ne ek müşteri ne kategori içi ikame var. | `MarketCustomers.cpp:132, 143` |
| 26 | 🔴 | **Satılan malın maliyeti bugünkü fiyattan.** Zam, iskonto ve toptancı destekli kampanya eldeki eski stoğun maliyetini geriye dönük değiştiriyor. | `MarketEconomy.cpp:110`, `MarketFreshness.cpp:118` |
| 27 | 🟠 | **Maliyetin altında satış serbest** ve uyarısız. Toptancı destekli kampanya raporu %15 alış kazancını göstermiyor; kârlı kampanya zararlı görünüyor. | `MarketGame.cpp:1064`, `MarketPromotions.cpp:278-280` |
| 28 | 🟠 | **Gondol başı bedava, süresiz ve eskimiyor** (×1,6 öne çıkarma, üstüne +%8 çekicilik). Oyuncunun her zaman kullanacağı en iyi seçenek. | `MarketPromotions.cpp` |
| 29 | 🟠 | **Referans fiyat hafızası yok.** Sürekli indirimin ve kampanya sonrası satış düşüşünün bedeli yok; müşteri kampanyada stok yapmıyor. | tasarım |
| 30 | 🟠 | **Online satış kampanyaları yok sayıyor**, dükkânda olmayan ürünü sipariş ettiriyor ve sonraki kampanyanın karşılaştırma tabanını şişiriyor. | `MarketOnline.cpp:151, 429, 474-476` |
| 31 | 🟡 | "3 al 2 öde" 2 isteyen müşteriyi zorla 3'e çıkarıyor; kaybedilen marj gün toplamından hesaplanıyor. | `MarketPromotions.cpp:80-85` |
| 32 | 🟡 | **`products.json` veri sorunları.** Aktif 16 üründe marj tek tip %22–37; oysa gerçekte süt, çay, yağ ve şeker %8–15, bisküvi ve bakım %28–40. Raf ömrü alanı yok (UHT süt 7 gün sayılıyor, gerçekte 90–120 gün). Kategoriler kaba ("içecek" = su + kola + Red Bull). KDV oranı, gramaj ve "fiyatı bilinen ürün" alanları yok. `fictionalName` = `realName`. 2011 için bazı fiyatlar yüksek (Coca-Cola 1 L 2,75 TL). | `Config/products.json`, `MarketGoods.cpp:80` |

### 2.5 Ekonomi, finans, personel

| # | Önem | Hata | Yer |
|---|---|---|---|
| 33 | 🔴 ✔ | **Vade işletme sermayesi sağlamıyor.** Toptancı vade verse bile `Cash < Bill` ise sipariş reddediliyor. Oyunun asıl ticari kaldıracı (vadeli al, peşin sat) çalışmıyor. Otomatik sipariş de aynı. | `MarketEconomy.cpp:44`, `MarketSimulation.cpp:174` |
| 34 | 🔴 ✔ | **KDV kâr gibi sayılıyor, üstüne gelir vergisi alınıyor.** Ciro KDV dahil, ödenen KDV gider olarak düşülmüyor. Formül `(satış−alış)×0,08`; doğrusu `×oran/(1+oran)`. | `MarketStaff.cpp:603-606` |
| 35 | 🔴 | **Müşavir istismarı.** İşe alma ve çıkarma bedeli yok; vergi yalnızca 7. günün kapanışında kontrol ediliyor. 7. gün al, 8. gün çıkar: haftada 4 TL'ye %10 indirim ve sıfır denetim. | `MarketStaff.cpp:316` |
| 36 | 🔴 | **Enflasyona bağlanmamış sabitler.** İşe alma bedeli 120/200 TL, müşavir 4 TL/gün, en küçük sipariş 50 TL, ceza tabanları, `ExpandCash` 950 TL sabit kalıyor. 2025'te fiyat düzeyi ≈7,4 olduğu için hepsi 7 kat ucuzluyor. | çeşitli |
| 37 | 🔴 | **Görünmeyen kayıplar.** Eksik veya kırık gelen mal ödeniyor ama gidere yazılmıyor; raf küçülünce sığmayan mal sessizce yok oluyor. Kâr ile nakit birbirinden ayrışıyor. | `MarketEconomy.cpp:167-175, 202` |
| 38 | 🟠 ✔ | **Depozito enflasyon istismarı.** Şehir mağazası kapanınca depozito **bugünün** fiyat düzeyiyle geri ödeniyor: 2011'de verilen 10.000 TL, 2024'te ~59.000 TL olarak dönüyor. | `MarketCompany.cpp:187` |
| 39 | 🟠 | **Ücret asgari ücretin altına düşüyor.** Düşük becerili çalışan ≈13,5 TL/gün ≈ 400 TL/ay; 2011 net asgari ücret ≈630–659 TL. SGK işveren payı (≈×1,49), haftalık izin ve kıdem tazminatı yok. Moral düşen çalışanı çıkarıp yenisini almak bedava. | `MarketStaff.cpp:147-152`, `MarketPrices.cpp:24` |
| 40 | 🟠 | **Toptancı güveni şişiriliyor.** Erken ödenen her fatura +5 güven. 50 TL'lik parçalara bölünmüş siparişlerle güven birkaç günde 80'e çıkıyor. | `MarketSuppliers.cpp:156, 216` |
| 41 | 🟠 | **Gecikme faizi abartılı.** Toptancı faturası günde %2 bileşik ve tavansız (yıllık >%1.000); banka taksitine her gün yeniden %3 ekleniyor. | `MarketSuppliers.cpp:234-239`, `MarketFinance.cpp:154-158` |
| 42 | 🟠 | **Muhasebe modülden modüle farklı.** Şube tadilatı gider, şehir mağazası yatırım sayılıyor. Şube cirosu KDV'ye giriyor, şehir cirosu girmiyor. Yurt dışı kâr TL, kur yok, Bulgaristan'a Türkiye enflasyonu uygulanıyor. | `MarketBranches.cpp:194, 406`, `MarketCompany.cpp:169, 297-298` |
| 43 | 🟡 | Haftalık vergide zarar devri yok. Son gün indirimi taze partiler de dahil bütün birimlere uygulanıyor. Kapanan şubenin eski sütü ana dükkâna "taze parti" olarak geliyor. 30 gün eksi kalana en büyük kredi normal faizle veriliyor (ipotek bir ödül gibi çalışıyor). | `MarketFreshness.cpp:34-38`, `MarketBranches.cpp:215`, `MarketFinance.cpp:210` |

### 2.6 Dünya, şube, şirket

| # | Önem | Hata | Yer |
|---|---|---|---|
| 44 | 🔴 | **"Dünya liderliği" aslında mahalle liderliği.** Son bölümün şartı İstasyon semtinde %40 pay, 60 mağaza ve memnuniyet. Ciro sıralaması yok; `MarketRetail`'deki dünya devleri son veri yılından sonra donuyor. Oyuncu tabloda hiç yer almıyor. | `MarketCompany.cpp:244-250`, `MarketRetail.cpp:57` |
| 45 | 🔴 | **Ulusal pay formülü 20–60 kat şişik.** Mağaza başına %0,04 sayılıyor; BİM ise 3.500 mağazayla %6,5 alıyor. "50 mağaza" ile "%2 pay" hedefleri aynı şeyi ölçüyor. | `MarketCompany.cpp:101-104`, `MarketRetail.cpp:8-9` |
| 46 | 🔴 | **Şehirler ekonomik olarak aynı.** Gelir/rekabet oranı birbirini götürüyor; olgun mağaza İstanbul Avrupa'da −7 TL/gün (en kötü), Babaeski'de +238 TL/gün. Haritada verilecek bir karar yok. | `MarketCompany.cpp:133-142` |
| 47 | 🟠 | **Yatırım 60–110 günde geri dönüyor** (gerçekte 2–4 yıl). Kapasite, doygunluk ve rakip tepkisi yok. | `MarketCompany.cpp:144-148` |
| 48 | 🟠 | **Hava ve takvim tek.** Lüleburgaz'a kar yağınca İzmir'in cirosu düşüyor; Filibe'ye Ramazan ve 23 Nisan etkisi uygulanıyor. | `MarketCompany.cpp:138` |
| 49 | 🟠 | **Ölçek tutarsız.** Lüleburgaz semt alışveriş sayıları (120–320/gün) ~10 kat düşük: ilçe ~140 bin kişi. Aynı format Kocasinan'da 600 TL, Babaeski'de 2.640 TL ciro yapıyor. | `MarketBranches.cpp:116-122` |
| 50 | 🟡 | **Harita verisi sentetik ve 2011'de donmuş.** 60'tan fazla ilde BİM:A101:Şok:Migros oranı aynı (≈49:21:19:15). Nüfus artmıyor. Onur, CarrefourSA ve Kipa listeler arasında tutarsız. | `Config/iller.json` |
| 51 | 🟡 | "İkinci şube" hedefi kira sözleşmesi imzalanınca tamamlanıyor (tadilat bitmeden). Açılış stoğu kasa kontrolü olmadan düşüyor. README "950 TL" diyor, gerçek bedel 6–10 bin TL. | `MarketBranches.cpp:196, 349-350` |

---

## 3. Yeniden tasarım

### 3.1 Dünya yapısı ve "Dünya 1 numarası" nasıl olunur

**İlke:** Dünya devlerinin hiçbiri her ülkede değil. Costco ~890 mağaza ve 14 ülkeyle ~254 milyar $ ciro yapıyor; Kroger tek ülkede ~147 milyar $; BİM 3 ülkede. Birincilik **ciro** ile ölçülür. Oraya giden yol ise doğru formatlar, ölçek, toptan satış ve **büyüyen pazarları erken yakalamak**tan geçer.

#### Dört katmanlı dünya

**Dünya → Bölge → Ülke → Pazar hücresi**

- Türkiye'de pazar hücresi = il; büyük ilçeler ileride ayrı hücre olabilir. Başka ülkelerde 3–8 metropol ya da bölge hücresi.
- Başlangıçta 25–35 ülke yeter. Oyuncu yalnızca aile dükkânında yürür; diğer mağazalar hücre düzeyinde toplu simüle edilir.

#### Ülke profili (öneri: `Config/world.json`)

```json
{"id":"uz","name":"Özbekistan","region":"orta_asya","currency":"UZS",
 "series":{"pop":[[2011,29.3],[2024,37]],"gdppc_usd":[[2011,1600],[2024,3000]],
           "inflation":[[2011,12],[2018,17],[2024,10]],"fx_shock":[{"year":2017,"pct":-50}]},
 "modern":{"floor":0.08,"cap":0.65,"k":0.25,"t0":2030},
 "online":{"start":2019,"cap":0.08},
 "formats":{"discount":1.2,"super":0.9,"hyper":0.7,"club":0.2,"wholesale":1.3},
 "regulation":{"fdi_cap":1.0,"opens":2017,"permit_days":[30,90],"hyper_zoning":0.5},
 "costs":{"rent_idx":0.35,"wage_idx":0.2,"tax":0.15},
 "risk":{"political":0.35,"fx_vol":0.25},
 "culture":{"distance":0.3,"halal":true,"turkic":true},
 "cells":[{"id":"tashkent","pop_share":0.09,"income":1.6,"sites":[40,120,300]}],
 "rivals":[{"id":"yerel_super_1","archetype":"local_super","stores":[[2011,20],[2024,150]]}]}
```

`sites` = hücredeki A/B/C kaliteli dükkân yeri sayısı. İyi yerler sınırlı olduğu için erken giren kazanır, rakibin önü kesilir.

#### Büyüyen pazar dinamiği (fırsat penceresi)

- `Pazar = Nüfus × a × KişiBaşıGelir^0,6 × FiyatDüzeyi`
- `ModernPay(t) = taban + (tavan − taban) / (1 + e^(−k·(t − t0)))`. Kalan pay bakkal ve pazara ait.
- Gelir büyüdükçe ve zincirler mağaza açtıkça `t0` öne kayar. S eğrisinin **dik kısmına erken giren**, ucuz yer ve zayıf rakiple büyür. Oyunun "yakala" anı budur.
- Örnek pencereler: Özbekistan 2017 reformu; Mısır ve Fas (BİM'in gerçek rotası); Endonezya (satın almayla); Hindistan (çok markalı perakendede yabancı sermaye sınırı, giriş toptan ya da ortaklıkla); Nijerya (dev ama kur ve güvenlik riski yüksek); Almanya (doygun, ama ~3 milyon Türk kökenliye "diaspora formatı").

#### Ülkeye giriş yolları (her biri ayrı bir oyun kararı)

| Yol | Sermaye | Kontrol | Risk | Not |
|---|---|---|---|---|
| Kendi mağazası | Yüksek | Tam | Yüksek | Yavaş ama tam kâr |
| Franchise | ~%20 | Düşük | Marka riski | Telif %3–5; franchise alana mal satışı toptan ciroya yazılır |
| Ortaklık (JV) | %49–51 | Paylaşılan | Orta | İzin süresi ×0,5; zor ülkelerin kapısı |
| Satın alma | Çok yüksek | Tam | Sürprizler | `Bedel = FAVÖK × Çarpan × 1,25 + borç`; sinerji 24 ayda oturur; inceleme kalitesine göre gizli sorunlar çıkar |
| Toptan (cash & carry) | Orta | Tam | Düşük | Bakkala satar; Hindistan gibi kapalı pazarlarda tek kapı |
| Online / hızlı teslimat | Düşük | Tam | Düşük marj | 2016 sonrası |

#### Küresel Perakende Ligi (kazanma koşulu)

- Her yıl yayımlanan kurgu "Küresel 50" tablosu. Sıralama konsolide perakende ve toptan cirosunun dolar karşılığıyla yapılır; franchise alanların satışı sayılmaz.
- **Dünya 1 numarası:** 2 mali yıl üst üste ciroda 1. olmak. Aynı dönemde faaliyet nakdi pozitif ve borç/FAVÖK < 3 olmalı; borçla şişirilmiş liderlik sayılmaz.
- **Yan ligler:** marka değeri, kârlılık, "yükselen pazar şampiyonu", ülke birinciliği sayısı.
- **Ara sonlar:** Trakya 1. → Türkiye ilk 5 → Türkiye 1. → Bölge 1. → Dünya ilk 50 / ilk 10 / 1.
- Kurgu dev rakipler gerçek şirketlerden esinlenir, kendi ciro eğrisi ve strateji kişiliği olur (örnek: "Stateline" = Walmart esinli; hipermarket, satın almalar, ülke çıkışları). Rakipler de yanlış yapar: bir ülkede N yıl sermayenin maliyetinden az kazanırsa çekilir ve varlıklarını satar (Tesco/Kipa 2016, Carrefour'un Çin çıkışı gibi). Bunlar oyuncu için **satın alma fırsatı** olayıdır.
- Ayar: **"dünya ölçeği"** (devlerin ciro eğrisi ×0,5 – ×1,0). Böylece A06 kararına ("eğlence ve tahmin edilemezlik, birebir tarih değil") uygun olarak oyuncu 40–60 saatte zirveye oynayabilir.

**Referans (yaklaşık, ~2024; tasarım başlangıç değerleri, doğrulanmalı):**

| Esin | Ciro | Mağaza / ülke | Oyundaki dersi |
|---|---|---|---|
| Walmart | ~681 milyar $ | ~10.700 / 19 | Hipermarket + satın alma, ülke çıkışları |
| Costco | ~254 milyar $ | ~890 / 14 | Az mağaza, dev ciro |
| Schwarz (Lidl + Kaufland) | ~175 milyar € | ~14.000 / 30+ | Organik indirim zinciri yayılması |
| Aldi | 120 milyar $+ | ~13.000 / ~20 | Dar çeşit (BİM modeli) |
| Kroger | ~147 milyar $ | ~2.700 / 1 | Tek ülkede dev |
| Carrefour | ~90 milyar € | 14.000+ / 40+ | Yoğun franchise |
| BİM | ~15 milyar $ | ~13.000 / 3 | Yükselen pazar rotası |

#### İlerleme

1. **Lüleburgaz (2011).** Semt alışveriş sayıları ~10 katına çıkar. Rakipler bakkal, pazar, Bereket ve zincirler. Şube yeri A/B/C mülk havuzundan seçilir.
2. **Trakya.** Kırklareli, Edirne, Tekirdağ. Yerel zincir satın alma olayı, ilk dağıtım merkezi, Kipa esinli hipermarket rakibi.
3. **Türkiye.** 81 il hücresi, rakip sayıları yıl yıl güncellenir, bölge depoları, bakkala toptan satış, 2016 Kipa çıkışı satın alma fırsatı.
4. **Yakın yurt dışı.** Bulgaristan ve Romanya (AB kuralları; Bulgaristan 2026'da avroya geçti), Kafkasya, Orta Asya, Almanya diaspora formatı.
5. **Yükselen pazarlar.** Mısır, Fas, Özbekistan, Pakistan, Nijerya; Hindistan'a JV veya toptan; Endonezya'ya satın alma.
6. **Küresel.** Çekilen bir Batılı zinciri satın alma, online platform, özel marka ihracatı.

Her aşamanın kapısı mağaza sayısı değil **ciro + pay eşiği + gerekli kurum** olur (dağıtım merkezi, hazine birimi, ülke müdürü).

---

### 3.2 Yerel rekabet ekosistemi (bakkal, pazar, zincirler)

#### Aktörler

| Arketip | Örnek (kurgu) | Güçlü yanı | Zayıf yanı | Tipik hamlesi |
|---|---|---|---|---|
| Bakkal / tekel | "Ali'nin Yeri" | Yakınlık, veresiye, gece açık, sigara/ekmek | Fiyat ×1,08–1,15, dar çeşit | Veresiye limitini artırır, eve servis |
| Aile marketi | Bereket, "Yıldız Gıda" | Mahalle sadakati | Az nakit | Fiyat savaşı, dedikodu, personel ayartma, satılığa çıkma |
| **Semt pazarı** | "Salı Pazarı" | Meyve-sebze ×0,70–0,85, tazelik ×1,2 | Yalnızca taze ürün, haftada 1–2 gün | O gün taze talebimizi keser; akşamüstü fiyat kırar; yağmurda ×0,6 |
| Kasap / fırın / manav | "Halk Ekmek" büfesi | Tek kategoride güçlü | Tek kategori | Ekmek fiyatı kıyası (fiyatı herkesin bildiği ürün) |
| Hard discount | BİM/A101/Şok esinli | Fiyat ×0,90–0,94, çok mağaza | Dar çeşit, aktüel çabuk biter | Haftalık aktüel, köşe başı yer yarışı |
| Ulusal süpermarket | Migros esinli | Çeşit, kart kampanyası | Fiyat ×1,03–1,08 | Kart indirimi, tadilat |
| Hipermarket (2011–16) | Kipa esinli | Çeşit ×2, haftalık sepet | Uzak (arabayla) | Hafta sonu dev kampanya |
| Online / hızlı teslimat | 2016+ | Kolaylık | Teslimat ücreti | Sepet eşiği, kupon |

#### Müşteri seçimi: segment × alışveriş amacı üzerinden logit

```
U(s,j) = −βp·ln(AlgılananFiyat_j) − βd·Mesafe_j + βa·ln(Çeşit_j) + βf·Tazelik_j
         − βq·Kuyruk_j + βl·Sadakat(s,j) + βk·Kampanya_j + Açık_j(t) + ε
P(s,j) = e^U(s,j) / Σ_k e^U(s,k)
```

- **Toplam korunur:** ilçenin günlük alışveriş havuzu sabit; paylar ciroya dayanır, toplamları %100'dür. Menüdeki bütün rakamlar bu tek kaynaktan gelir.
- **Segmentler:** fiyat hassas · kolaylık · kalite · sadık yaşlı (veresiye, bakkala bağlı) · haftalık aile (arabalı, broşür okur).
- **Alışveriş amacı:** acil (1–2 kalem, yakın olanı seçer) · günlük tamamlama · haftalık büyük alışveriş (çeşit ve fiyat önemli) · taze (pazar günü pazara gider) · gece (yalnızca açık olan).
- **Fiyat imajı:** `AlgılananFiyat = 0,6·İmaj + 0,4·GerçekFiyat`. İmaj yalnızca fiyatı herkesin bildiği ürünlerden (ekmek, süt, yağ, şeker, çay) ve yavaş oluşur: `İmaj ← 0,9·İmaj + 0,1·KilitÜrünFiyatı`. Pahalı bir dükkân tek kampanyayla ucuz görünmeye başlamaz.
- **Sadakat:** iyi deneyimde yavaş artar, boş raf ve uzun kuyrukta 3 kat hızlı düşer.

#### Rakip yapay zekâsı

- **Her rakibin kendi bilançosu var:** `Kâr = Ciro·Marj − Sabit − Kampanya − Personel`. 60 gün zarar eden kapanır ya da satılığa çıkar; oyuncu satın alabilir.
- **Aylık strateji:** savun / saldır / hasat et (fiyat artır) / genişle / çekil.
  `Skor(a) = Beklenen Δkâr(90 gün) + Kişilik uyumu + Tehdit − Risk·(1 − risk iştahı)`
- **Haftalık taktik:** reyon fiyatı ±%5–15 · oyuncunun kampanyasını 3–7 gün gecikmeyle kopyalamak · aktüel/kart kampanyası · personel ayartma (teklif = ücret × 1,2) · tadilat · **yer yarışı** (oyuncunun baktığı semtte dükkân kiralama; önce imzalayan alır) · bakkal için veresiye ve eve servis.
- **Hafıza:** her oyuncu hamlesi `{tür, gün, reyon, şiddet}` olarak kaydedilir ve `e^(−gün/90)` hızıyla söner. Bereket "satmıyorum" cevabını unutmaz.
- **Bilgi sınırı:** rakip fiyatlarımızı yalnızca kilit ürünlerde ve 3–7 gün gecikmeyle görür. Tepkiler 2–3 gün gecikir; bazen blöf yapılır.
- **Kişilik:** saldırganlık, gurur, sabır, taklit, risk iştahı (0–1). Oyuncunun seçtiği kimlik rakiplerin tepkisini değiştirir. "Hızlı İndirim" kimliği indirim zincirlerini daha çok kızdırır.
- **Zorluk hileyle değil, bilgi ve karar kalitesiyle:**

| Seviye | Bilgi gecikmesi | Karar kalitesi | Tepki süresi |
|---|---|---|---|
| Rahat | 7 gün | Daha fazla hata | 5 gün |
| Normal | 4 gün | — | 3 gün |
| Zor | 2 gün | 2 adım ileri bakar | 1–2 gün |

---

### 3.3 Fiyat ve kampanya sistemi (tek ürüne de uygulanabilen)

#### Kampanya = kapsam × mekanik × teşhir × fon

```
Kapsam  : Tek ürün (SKU) | Marka | Alt kategori | Kategori | Raf/Reyon | Tüm mağaza | Kart sahipleri | Kanal (online/mağaza)
Mekanik : %X indirim | X TL indirim | 3 al 2 öde | 2 al 1 öde | 2. ürün %50 | 3 adet 10 TL | Paket (çay+şeker)
          | Sepette 50 TL'ye 5 TL | Kart puanı ×N | Kupon | Hediyeli ürün | Müşteri başı adet sınırı
Teşhir  : Raf etiketi | Gondol başı (kiralık) | Kasa önü | Yer standı | Broşür/aktüel
Fon     : Adet başı toptancı desteği | Sabit ödeme (gondol kirası, broşür katılımı) | Hedef primi | Uyum şartı
```

Kampanya penceresinde ürün, marka, raf ya da reyon seçilir; yüzde, süre ve kapsam ayarlanır. Önizleme tahmini **artımlı brüt kârı** gösterir.

#### Talep formülü

- **Efektif fiyat.** 3 al 2 öde için `P·(1 − ⌊q/3⌋/q)`, 2. ürün %50 için `P·(1 − 0,25·⌊q/2⌋·2/q)`.
- **Algılanan kazanç.** `g = (Ref − P_eff) / Ref`, burada `Ref = 0,6·ReferansFiyat + 0,4·RakipFiyatı`. Etiket için +0,03, broşür için +0,05 eklenir.
- **Kalem talebi.** `λ = λ₀ · e^(β·g) · Görünürlük · Takvim`
  - β kategoriye göre değişir: temel gıda 1,5 · çay ve deterjan 3 · cips ve içecek 4.
  - Zam yönünde β × 1,4 (kayıptan kaçınma).
- **Kanibalizasyon.** İç içe logit: önce alt kategori, sonra ürün. İndirimli ürün pay alır; alt kategorinin toplamı ise en fazla `×(1 + 0,3·β·g)` büyür.
- **Evde stok yapma.** `q = q₀·(1 + s·g)` (deterjan s = 1,5, süt 0,2). Fazla alınan miktar müşterinin evdeki stoğuna yazılır, sonraki alımı azaltır. Kampanya sonrası düşüş böylece kendiliğinden oluşur.
- **Referans fiyat hafızası.** `Ref ← Ref + 0,08·(GörülenFiyat − Ref)`. Son 90 günün %30'undan fazlasında kampanyadaki ürünün referans fiyatı düşer ve "normal fiyat" kalıcı olarak aşınır.
- **Zarar lideri ve müşteri çekme.** Broşürde, fiyatı herkesin bildiği ürünlerdeki indirim ek müşteri getirir (tavan ×1,3). Gelen müşteri tam fiyatlı ürün de alır.
- **Kampanyada stok tükenmesi.** Gelen müşteri başına memnuniyet −6 (normalde −2). Broşürdeki ürün tükenirse ceza 2 kat ve "Broşürde vardı, rafta yok" haberi çıkar.

#### Takvim şablonları

| Dönem | Ürünler / mekanik |
|---|---|
| Ramazan | Erzak kolisi, hurma, çay-şeker, yağ |
| Bayram | Çikolata, kolonya, lokum |
| Okul açılışı | Atıştırmalık, süt |
| Maaş günü | "Aktüel günü" |
| Kış | Çay, çorba, deterjan |
| Yaz | Su, ayran, dondurma |

Broşür 3 gün önceden planlanır, stok önceden sipariş edilir. Kampanya ve sipariş sistemi bu yolla birbirine bağlanır.

#### Diğer parçalar

- **Sadakat kartı ve kupon:** puan borcu bilançoya yazılır; kart verisi "kim ne alıyor" raporunu açar.
- **Yasal çerçeve:** maliyet altı satış yalnızca "zarar lideri" etiketiyle ve adet sınırıyla yapılabilir, aksi halde denetim olayı. Etikette yazan "eski fiyat" son 30 günde gerçekten uygulanmış olmalı.
- **Kampanya raporu:** artımlı adet, artımlı brüt kâr (fon dahil), başka üründen çalınan satış, tamamlayıcı ürünlerden gelen kâr, ek müşteri, sonraki 14 gündeki düşüş, boş raf kaybı, referans fiyattaki değişim.
  Örnek: *"Kampanya 38 TL kazandırdı ama Pınar, Sütaş'tan 12 adet çaldı; Sütaş'ın normal fiyatı artık 2,35 TL sanılıyor."*
- **Katalog alanları (`products.json`):** alt kategori, KVI (fiyatı bilinirlik 0–1), KDV oranı (2011: gıda %8, deterjan/bakım %18), raf ömrü, gramaj/birim fiyat, özel marka, esneklik, stoklanabilirlik, müşteri çekme gücü, marka sadakati. Kategoriye göre gerçekçi marj bantları.

---

### 3.4 Ekonomi, tedarik, personel, finans

- **Muhasebe çekirdeği (önce bu):** küçük bir çift taraflı defter (`Post(borç, alacak, tutar, sebep)`). Hesaplar: Kasa, Banka, Kart alacağı, Veresiye, Stok (ağırlıklı ortalama maliyet), Toptancı borcu, KDV, Vergi, Özkaynak. 34, 37, 26, 42 ve 43 numaralı hatalar bununla birlikte kapanır. Ay sonunda gelir tablosu, bilanço ve nakit akışı çıkar.
- **Makroekonomi:** aylık `{enflasyon, kur, politika faizi, tüketici güveni, asgari ücret, işveren maliyeti}`. Kur, ithal ağırlıklı kategorilere daha çok geçer (kahve β 0,7, süt 0,1). Kurgu dönemlerin her biri 2–3 seçenekli ulusal karar olayı olur:
  - **2018 civarı "kur fırtınası":** stok yap / zammı yansıt / marjı erit.
  - **2020 salgını:** trafik ×0,6, sepet ×1,5, online ×3; tavan fiyat ikilemi.
  - **2021–23 yüksek enflasyon:** haftalık etiket yenileme işçiliği, yılda iki asgari ücret artışı, vadelerin kısalması, **negatif reel faiz** (kredili stok stratejisi ve riski).
- **Toptancı pazarlığı ve işletme sermayesi:**
  - Sözleşme: vade 0–90 gün, raf giriş ücreti, hedef primi, erken ödeme iskontosu, pazarlama fonu, münhasırlık.
  - `Güç = 0,4·log(Ciro) + 0,2·√Mağaza + 0,2·Güven + 0,2·Alternatif`; `Vade = 7 + 60·Güç`.
  - Ekranda nakit dönüşüm süresi `CCC = DIO + DSO − DPO`. Hedef BİM tipi **negatif CCC**: mal satılır, para vadeden önce gelir, büyümeyi finanse eder.
  - Sipariş kuralı: `Kasa + kullanılmamış vade limiti ≥ tutar`.
  - Özel marka: maliyet ×0,75, raf fiyatı ×0,85, kalite olayı riski.
  - Üreticiden doğrudan alım: −%6, karşılığında palet minimumu ve kendi lojistiğin.
- **Dağıtım merkezi:** teslim süresi ve birim lojistik maliyeti hacimle düşer; bulunurluk %94 → %98; mağaza başına emniyet stoğu azalır. Soğuk zincir süt ürünlerinin raf ömründen 1 gün alır.
- **Personel:** beş beceri (kasa, reyon, taze, depo, yönetim), kıdem, bağlılık, eğitim, hırsızlık eğilimi.
  - Ücret en az asgari brüt; işveren maliyeti SGK ile ≈ brüt × 1,2.
  - Kıdem tazminatı ve ihbar süresi var; işten çıkarmanın gerçek bir bedeli olur.
  - `p(ayrılma/ay) = σ(−3 + 0,05·(60 − Moral) + 0,8·(PiyasaÜcreti/Ücret − 1) − 0,3·ln(1 + Kıdem))`.
  - Fire ve kaçak cironun %1–2'si civarında; kamera, alarm ve sayım gibi önlemlerle düşer.
- **Finansman:** limit = f(FAVÖK, teminat, kredi notu); leasing; kart alacağının erken tahsili (faktoring); yatırımcı (değerleme = FAVÖK × 6, yönetim kurulu hedefleri); halka arz (100+ mağaza, 3 yıl denetimli mali tablo, çeyrek beklentisi baskısı). İflas yerine **konkordato**: borcun bir kısmı silinir ve ödeme planı yapılır, bedeli kredi notu ve vadenin sıfırlanması. Bu B06 kararına ("oyun bitmez") uygun.

---

### 3.5 Oyun döngüsü: katmanlı rol

| Rol | Tur | Oyuncunun işi | Devredilen |
|---|---|---|---|
| Tezgâh | Gün (4–6 dk) | Kasa, raf, mal kabul, fiyat | — |
| Mağaza müdürü | Gün → hafta | Vardiya, sipariş politikası, kampanya | Kasa ve raf (çalışanın becerisi ve hatasıyla) |
| Bölge | Hafta | Şube hedefleri, müdür atama, depo ve rota | Mağaza siparişi |
| Genel müdür | Ay | Format, toptancı anlaşması, özel marka, kredi | Bölge işleri |
| Holding CEO | Çeyrek | Satın alma, halka arz, ülke girişi, yönetim kurulu | Genel müdürlük |

- **İlerletme devre bağlanır.** Sonuç, işi yapan kişinin becerisine göre hesaplanır (kasiyer yoksa kuyruk kaybı, zayıf müdürde sipariş hatası).
- **Kapalı dükkânda da saat işler.** Sabah hazırlığı (06:30–08:00) sınırlı bir süredir.
- **Elle oynamanın bir karşılığı olur.** Kriz günlerinde (bayram arifesi, denetim, kamyon arızası) oyuncu dükkâna inerse bonus verim alır.
- **Göstergeler paneli:** nakit akışı, bulunurluk, kuyruk kaybı, pay, müdavim, marj, borç/özkaynak. Her biri 4 haftalık eğilimle ve renk rozetiyle gösterilir. "Rakibi geçtin" gibi kutlama anları olur.
- **Zorluk başlangıçta ayrı ayrı seçilir ve kilitlenir:** ekonomi, rakip saldırganlığı, olay yoğunluğu, mikro iş yükü. Ayrıca **Demir mod**: tek kayıt ve gerçek iflas.
- **Tekrar oynanabilirlik:** görünür kampanya tohumu; farklı başlangıç şehri (Lüleburgaz, Çorlu, Edirne, bir Anadolu ilçesi); farklı mirasçı profili (tapulu ama borçlu / nakitli ama kiracı / kötü yerde iyi ekip); rastgele rakip kişilik destesi.

### 3.6 Hikâye ve karakter hafızası

| Yıllar | Dönem | Gerilim |
|---|---|---|
| 2011–13 | Defter | Babanın borcu, Nermin teyze, Kadir Bey'in teklifi |
| 2014–17 | Zincir baskısı | İndirim zinciri köşeye açılır; kardeş mirastan payını ister (sat / ortak al / kredi çek) |
| 2018 | Kur sarsıntısı | Toptancı vadeyi keser, banka teminat ister |
| 2020–21 | Salgın | Mevcut profil korunur; online patlaması |
| 2022–23 | Etiket savaşı | Haftalık fiyat güncelleme, Selim'in şirketi el değiştirir |
| 2024–30 | Konsolidasyon | Bereket satılığa çıkar, yabancı fon teklif verir, ülke girişleri, halef seçimi |

- **İlişki kaydı:** her karakter için güven, kin, borç/iyilik bayrakları ve son karşılaşma tutulur.
- **Kararların sonucu gecikmeli gelir.** Nermin'e veresiye verilmediyse 90 gün sonra mahallenin tavrı değişir. Cem işe alınmadıysa 2016'da Bereket'in müdürü olarak karşına çıkar.

### 3.7 Kilometre taşları (örnek)

| Oyun süresi | Takvim | Olay | Açılan |
|---|---|---|---|
| 1 sa | Mart 2011 | İlk hafta | Kasiyer, haftalık rapor |
| 2 sa | Nisan 2011 | Borç kapandı, kimlik seçildi | Haftalık tur |
| 5 sa | 2012 | 2–3 şube | Müdür, İK |
| 10 sa | 2014 | 8 mağaza, depo | Aylık tur, bölge rolü |
| 18 sa | 2018 | Kur sarsıntısı | Kredi yapılandırma |
| 25 sa | 2020 | 50 mağaza, özel marka | Genel müdürlük |
| 35 sa | 2023 | Yurt dışı pilot | Çeyrek tur, CEO |
| 45+ sa | 2026–30 | Küresel ilk 10 → 1 | Satın almalar, halef sonu |

---

## 4. Yol haritası

**Aşama 0: Acil düzeltmeler (1–2 oturum).** Oyun bozuk kalmasın.

1. Test modu varsayılan kapalı; açıksa kayda yazılsın (#1).
2. Otomatik yükleme ve 3 kayıt yuvası; kayıt sürümü artırılsın (#2, #6).
3. Bölüm 99 ve "Rüyaymış" hataları (#3, #4).
4. Bereket savaşı ve rakip haberleri paya işlesin; paylar ciroya dayansın ve toplamı %100 olsun (#14–17).
5. Vade ile sipariş; KDV ayrı hesap; müşavir istismarı (#33–35).
6. Sabitler enflasyona bağlansın; depozito açılıştaki değerle geri dönsün (#36, #38).

**Aşama 1: Temel model (3–5 oturum).**

- Muhasebe defteri ve stokta ağırlıklı ortalama maliyet.
- `products.json` yeni alanları ve gerçekçi marj bantları.
- Kampanya kapsam/mekanik yapısı (tek ürün dahil) ve yeni talep formülü (esneklik, referans fiyat, kanibalizasyon, evde stok).
- Raf düzeni kayda taşınsın.

**Aşama 2: Yaşayan mahalle (3–4 oturum).**

- Semt verisi: haneler, segmentler, satış noktaları.
- Bakkal, semt pazarı, fırın.
- Logit müşteri seçimi.
- Bereket'e bilanço, hafıza, kişilik; satılığa çıkma ve satın alma.
- Zincirlerin haftalık ve aylık karar döngüsü.

**Aşama 3: Rol katmanları ve zaman (2–3 oturum).**

- İlerletme devre bağlansın, kapalı dükkânda saat işlesin.
- Haftalık ve aylık tur, göstergeler paneli.
- Karakter ilişki kaydı ve gecikmeli sonuçlar.

**Aşama 4: Türkiye (3–4 oturum).**

- 81 il hücresi, zaman serili rakip mağaza sayıları.
- Mülk (A/B/C yer) havuzu ve yer yarışı.
- Dağıtım merkezi, toptancı sözleşmeleri, özel marka.
- Makro dönem olayları.

**Aşama 5: Dünya (4–6 oturum).**

- `world.json` (25–35 ülke), kur, giriş yolları, satın alma.
- Küresel 50 ligi ve kurgu dev rakipler.
- Konkordato, yatırımcı, halka arz.

**Mustafa'nın kararını gerektiren açık sorular:**

- **Zaman ile gerçekçilik dengesi.** Dünya 1 numarası kaç saatlik oyunda ulaşılabilir olsun (öneri: 40–60 saat, "dünya ölçeği" ayarıyla)?
- **Menü açıkken zaman.** Aksın mı (G-075), dursun mu (A02)? Öneri: tezgâh rolünde dursun, müdür rolünden itibaren aksın.
- **Gerçek zincir adları.** A12 gerçek adlara yalnızca olağan ticaret yaptırıyor. Fiyat savaşı, kapanma, satın alınma gibi olumsuz olaylar yalnızca kurgu şirketlerle mi olsun, yoksa her zinciri kurgu adla mı değiştirelim?
