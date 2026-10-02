# Otomatik oyuncunun denge raporu

9 koşu, her biri 3653 gün; toplam sure 372.7 saniye.

| Tarz | Tohum | Son kasa | Borç | Magaza | İl | Ulusal pay | Kasa eksi gün | Sıkıntı günu | Denetim hatasi |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Temkinli | 21 | 46000,83 TL | 229032,33 TL | 1 | 1 | 0.0020% | 72 | 72 | 0 |
| Temkinli | 22 | 47561,25 TL | 200949,46 TL | 1 | 1 | 0.0019% | 62 | 62 | 0 |
| Temkinli | 23 | 75997,88 TL | 190585,03 TL | 1 | 1 | 0.0020% | 60 | 60 | 0 |
| Dengeli | 21 | 84234,33 TL | 39945,33 TL | 1 | 1 | 0.0022% | 62 | 62 | 0 |
| Dengeli | 22 | -77617,62 TL | 138502,18 TL | 1 | 1 | 0.0002% | 610 | 610 | 0 |
| Dengeli | 23 | 46131,25 TL | 165429,45 TL | 1 | 1 | 0.0020% | 67 | 67 | 0 |
| Atak | 21 | 12113201,03 TL | 400503,66 TL | 139 | 28 | 0.5678% | 0 | 0 | 0 |
| Atak | 22 | 8807065,45 TL | 567848,35 TL | 72 | 26 | 0.4001% | 0 | 0 | 0 |
| Atak | 23 | 11319691,68 TL | 452157,04 TL | 83 | 27 | 0.4677% | 0 | 0 | 0 |

## Bulgular ve neye bakmalı

- 6/9 koşuda kasa eksiye düştü.
- 3/9 koşu sonunda birden cok mağaza acik kaldi.
- Satış, siparis, stok ve sayi denetimi: 0 hata.

- İstismar şüphesi: son kasalar arasinda en az 4 kat fark var; asagidaki giderleri, mağaza sayisini ve arka plan netini karsilastir. Bu fark tek basina hile kaniti degildir.

### Temkinli / tohum 21

- İlk şube: 399. gün.
- 5 mağaza: 729. gün.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 849. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Mal alımi (stok yatırımi): 2908603,30 TL.
- Üst yönetim ücretleri: 1227288,68 TL.
- Vergi ödemeleri: 496172,82 TL.
- Aile dükkânınin ücretleri: 382062,00 TL.
- İşletme giderleri (ücret hariç): 249682,02 TL.

Arka planin kasaya toplam net etkisi: -2100561,16 TL. Reddedilen komut: 8.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: 826099,26 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -1959422,10 TL.
- Internet satışi: 9828,51 TL.
- Subeler: 1354347,47 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 1 | 253912 / 2649343793 |
| 2 | 28 | 20 | 5 | 692048 / 2742359338 |
| 3 | 27 | 20 | 7 | 1224759 / 2806522293 |
| 4 | 27 | 20 | 7 | 1106490 / 2851676281 |
| 5 | 27 | 20 | 7 | 1102593 / 2934839710 |
| 6 | 27 | 20 | 7 | 1057658 / 3037157017 |
| 7 | 27 | 20 | 7 | 1087427 / 3095215116 |
| 8 | 28 | 20 | 7 | 1042776 / 3168933777 |
| 9 | 28 | 20 | 7 | 1027581 / 3289308589 |
| 10 | 35 | 25 | 1 | 252835 / 3360673294 |

Rakipler: en çok 43 etkin zincir; 1 farklı satılık zincir; 0 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 5 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 94991.30 TL; küsen farklı marka 6.

C3 kapanma nedenleri: 0 iflas/kapanma, 0 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 719, tamamlanan 257; kutlama 338. Ritim koruyucusu: 4 sakin dönem olayı, 16 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | 73563.97 | 12637.82 | 12056.19 | -59693.18 |
| durgünluk / 1 | 3280–3614 | 335 | 11533.81 | 39668.49 | -166103.05 | -200157.92 |
| büyük salgın / 1 | 3674–4145 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 1 | 4223–5075 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| toparlanma / 1 | 5135–5499 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 7753–7936 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7998–8543 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 665. gün 0 -> 1.
- Kuru gıda: 3365. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -1083.81 / 1554.81 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 1. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 69.
- RejectBrandOffer: 173.
- SetDepartment: 10.
- SetDeptStance: 1.
- SetSourcing: 2.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 146.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 146.
- Önce yerel pay %35.: 8.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 3.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 1; kapanan şube 6; borç sınırı ihlali 0 ay; şirket faizi 0,00 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 1, yabancı kapı kolu 1.
Kurtarma günleri: 3386

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.

#### C5: internet

Salgın 3676..4153. İl önerileri 0, onay 0, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 1576; salgına uzaklık: -2100 gün.
- Telefon uygulaması ilk açılış: 0; salgına uzaklık: 0 gün.
- HızlıSepet ilk açılış: 3466; salgına uzaklık: -210 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 4 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 5 | 0.65 | 0.38 | 126 | 1761.21 | 0 | 0 | 0 | 0 | 0.995 |
| 6 | 0.91 | 0.45 | 139 | 1765.15 | 0 | 0 | 0 | 0 | 0.992 |
| 7 | 1.17 | 0.46 | 137 | 1816.51 | 0 | 0 | 0 | 0 | 0.990 |
| 8 | 1.44 | 0.55 | 175 | 2746.56 | 0 | 0 | 0 | 0 | 0.987 |
| 9 | 1.90 | 0.62 | 182 | 3387.19 | 0 | 0 | 0 | 0 | 0.984 |
| 10 | 2.41 | 0.67 | 65 | -1648.11 | 0 | 0 | 0 | 0 | 0.978 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 0 (onay 0 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 0 ürün işlemi, 0 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 0.00 | 0.00 |
| 3 | tr | Radyo | 0.00 | 0.00 |
| 3 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 3 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 0.00 | 0.00 |
| 4 | tr | Radyo | 0.00 | 0.00 |
| 4 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 4 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 4 | tr | Sosyal medya | 0.00 | 0.00 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 0.00 | 0.00 |
| 5 | tr | Radyo | 0.00 | 0.00 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 5 | tr | Sosyal medya | 0.00 | 0.00 |
| 5 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 6 | tr | Televizyon | 0.00 | 0.00 |
| 6 | tr | Radyo | 0.00 | 0.00 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 6 | tr | Sosyal medya | 0.00 | 0.00 |
| 6 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 7 | tr | Televizyon | 0.00 | 0.00 |
| 7 | tr | Radyo | 0.00 | 0.00 |
| 7 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 7 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 7 | tr | Sosyal medya | 0.00 | 0.00 |
| 7 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 8 | tr | Televizyon | 0.00 | 0.00 |
| 8 | tr | Radyo | 0.00 | 0.00 |
| 8 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 8 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 8 | tr | Sosyal medya | 0.00 | 0.00 |
| 8 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 9 | tr | Televizyon | 0.00 | 0.00 |
| 9 | tr | Radyo | 0.00 | 0.00 |
| 9 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 9 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 9 | tr | Sosyal medya | 0.00 | 0.00 |
| 9 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 10 | tr | Televizyon | 0.00 | 0.00 |
| 10 | tr | Radyo | 0.00 | 0.00 |
| 10 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 10 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 10 | tr | Sosyal medya | 0.00 | 0.00 |
| 10 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 11 | tr | Televizyon | 0.00 | 0.00 |
| 11 | tr | Radyo | 0.00 | 0.00 |
| 11 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 11 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 11 | tr | Sosyal medya | 0.00 | 0.00 |
| 11 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Demir Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 1, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 267. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Temkinli / tohum 22

- İlk şube: 459. gün.
- 5 mağaza: 759. gün.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 909. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Mal alımi (stok yatırımi): 3006977,00 TL.
- Üst yönetim ücretleri: 1076238,28 TL.
- Vergi ödemeleri: 503323,10 TL.
- Aile dükkânınin ücretleri: 392648,50 TL.
- İşletme giderleri (ücret hariç): 290614,31 TL.

Arka planin kasaya toplam net etkisi: -1679131,75 TL. Reddedilen komut: 50.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: 748800,12 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -1804566,28 TL.
- Internet satışi: 61719,15 TL.
- Subeler: 1265488,35 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 28 | 20 | 1 | 238972 / 2585369432 |
| 2 | 28 | 20 | 4 | 631444 / 2664411879 |
| 3 | 27 | 20 | 7 | 1233566 / 2739396267 |
| 4 | 27 | 20 | 8 | 1347386 / 2818767366 |
| 5 | 27 | 20 | 8 | 1257413 / 2940529417 |
| 6 | 27 | 20 | 8 | 1191861 / 3007185519 |
| 7 | 27 | 20 | 8 | 1180172 / 3127550970 |
| 8 | 30 | 24 | 8 | 1185941 / 3215860338 |
| 9 | 36 | 32 | 1 | 145165 / 3285197661 |
| 10 | 35 | 32 | 1 | 212556 / 3425491994 |

Rakipler: en çok 61 etkin zincir; 6 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 3 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 97268.90 TL; küsen farklı marka 5.

C3 kapanma nedenleri: 0 iflas/kapanma, 4 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 732, tamamlanan 260; kutlama 342. Ritim koruyucusu: 3 sakin dönem olayı, 19 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 2306–2480 | 175 | 158773.49 | 98106.62 | 97214.49 | -66990.04 |
| durgünluk / 1 | 2481–2814 | 334 | 97769.57 | 36544.95 | 29599.65 | -63137.48 |
| büyük salgın / 1 | 2874–3346 | 473 | 56968.27 | 32181.76 | -144411.07 | -225155.11 |
| yüksek enflasyon / 1 | 3424–4275 | 230 | 36571.72 | 47561.25 | 31053.42 | 13352.02 |
| toparlanma / 1 | 4335–4699 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6953–7136 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7198–7744 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 665. gün 0 -> 1.
- Kuru gıda: 3125. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -1636.84 / 1469.79 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 0. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 68.
- RejectBrandOffer: 174.
- SetDepartment: 12.
- SetDeptStance: 1.
- SetSourcing: 2.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 126.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 126.
- Önce yerel pay %35.: 9.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 2.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 1; kapanan şube 7; borç sınırı ihlali 0 ay; şirket faizi 0,00 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 6, yabancı kapı kolu 4.
Kurtarma günleri: 3140

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.

#### C5: internet

Salgın 2887..3324. İl önerileri 0, onay 0, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 792; salgına uzaklık: -2095 gün.
- Telefon uygulaması ilk açılış: 2340; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1520; salgına uzaklık: -1367 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.42 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.69 | 0.55 | 167 | 2275.03 | 0 | 0 | 0 | 0 | 0.994 |
| 4 | 0.95 | 0.43 | 147 | 1914.85 | 0 | 0 | 0 | 0 | 0.992 |
| 5 | 1.22 | 0.55 | 253 | 2653.90 | 0 | 0 | 0 | 0 | 0.989 |
| 6 | 1.48 | 0.69 | 313 | 4043.58 | 0 | 0 | 0 | 0 | 0.987 |
| 7 | 2.01 | 1.21 | 491 | 4881.08 | 0 | 0 | 0 | 0 | 0.983 |
| 8 | 8.51 | 2.42 | 925 | 16016.31 | 0 | 36 | 277 | 18655 | 0.967 |
| 9 | 6.08 | 7.11 | 1302 | 30756.69 | 0 | 366 | 1302 | 95767 | 0.756 |
| 10 | 8.00 | 1.77 | 150 | -822.29 | 0 | 35 | 10 | 1268 | 0.927 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 0 (onay 0 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 1 ürün işlemi, 1 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 0.00 | 0.00 |
| 3 | tr | Radyo | 0.00 | 0.00 |
| 3 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 3 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 0.00 | 0.00 |
| 4 | tr | Radyo | 0.00 | 0.00 |
| 4 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 4 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 4 | tr | Sosyal medya | 0.00 | 0.00 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 0.00 | 0.00 |
| 5 | tr | Radyo | 0.00 | 0.00 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 5 | tr | Sosyal medya | 0.00 | 0.00 |
| 5 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 6 | tr | Televizyon | 0.00 | 0.00 |
| 6 | tr | Radyo | 0.00 | 0.00 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 6 | tr | Sosyal medya | 0.00 | 0.00 |
| 6 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 7 | tr | Televizyon | 0.00 | 0.00 |
| 7 | tr | Radyo | 0.00 | 0.00 |
| 7 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 7 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 7 | tr | Sosyal medya | 0.00 | 0.00 |
| 7 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 8 | tr | Televizyon | 0.00 | 0.00 |
| 8 | tr | Radyo | 0.00 | 0.00 |
| 8 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 8 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 8 | tr | Sosyal medya | 0.00 | 0.00 |
| 8 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 9 | tr | Televizyon | 0.00 | 0.00 |
| 9 | tr | Radyo | 0.00 | 0.00 |
| 9 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 9 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 9 | tr | Sosyal medya | 0.00 | 0.00 |
| 9 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 10 | tr | Televizyon | 0.00 | 0.00 |
| 10 | tr | Radyo | 0.00 | 0.00 |
| 10 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 10 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 10 | tr | Sosyal medya | 0.00 | 0.00 |
| 10 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 11 | tr | Televizyon | 0.00 | 0.00 |
| 11 | tr | Radyo | 0.00 | 0.00 |
| 11 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 11 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 11 | tr | Sosyal medya | 0.00 | 0.00 |
| 11 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Yılmaz Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 1, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 513. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Temkinli / tohum 23

- İlk şube: 489. gün.
- 5 mağaza: 789. gün.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 909. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Mal alımi (stok yatırımi): 3281470,16 TL.
- Üst yönetim ücretleri: 879322,22 TL.
- Vergi ödemeleri: 456413,70 TL.
- Aile dükkânınin ücretleri: 417400,50 TL.
- İşletme giderleri (ücret hariç): 299828,24 TL.

Arka planin kasaya toplam net etkisi: -2292978,89 TL. Reddedilen komut: 28.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: 771854,02 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -1605241,24 TL.
- Internet satışi: 78477,85 TL.
- Subeler: 1017555,54 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 1 | 222391 / 2616019729 |
| 2 | 27 | 20 | 4 | 596507 / 2664104721 |
| 3 | 27 | 20 | 7 | 1221706 / 2766262382 |
| 4 | 27 | 20 | 8 | 1148670 / 2881109386 |
| 5 | 27 | 20 | 8 | 1233815 / 2996229747 |
| 6 | 27 | 20 | 8 | 1206546 / 3089990654 |
| 7 | 27 | 20 | 8 | 1239723 / 3158913001 |
| 8 | 28 | 27 | 8 | 999926 / 3214436872 |
| 9 | 37 | 27 | 1 | 235245 / 3297649008 |
| 10 | 37 | 27 | 1 | 241628 / 3417996779 |

Rakipler: en çok 53 etkin zincir; 3 farklı satılık zincir; 3 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 6 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 97213.20 TL; küsen farklı marka 5.

C3 kapanma nedenleri: 0 iflas/kapanma, 3 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 727, tamamlanan 263; kutlama 338. Ritim koruyucusu: 6 sakin dönem olayı, 17 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 1967–2141 | 175 | 161964.12 | 119568.24 | 117955.96 | -54946.70 |
| durgünluk / 1 | 2142–2475 | 334 | 119680.24 | 92983.42 | 78919.84 | -24157.70 |
| büyük salgın / 1 | 2535–3006 | 472 | 112011.13 | 12812.33 | -130315.94 | -263138.70 |
| yüksek enflasyon / 1 | 3084–3936 | 570 | 34059.97 | 75997.88 | 33353.69 | 39766.52 |
| toparlanma / 1 | 3996–4360 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6614–6797 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 6859–7404 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 665. gün 0 -> 1.
- Süt ürünleri: 2225. gün 0 -> 1.
- Süt ürünleri: 2945. gün 1 -> 0.
- Kuru gıda: 3001. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -1280.24 / 1719.96 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 2. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 77.
- RejectBrandOffer: 165.
- SetDepartment: 10.
- SetDeptStance: 1.
- SetSourcing: 3.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 104.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 104.
- Önce yerel pay %35.: 8.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 2.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 1; kapanan şube 7; borç sınırı ihlali 0 ay; şirket faizi 0,00 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 3, yabancı kapı kolu 2.
Kurtarma günleri: 2986

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 3 deneme / 3 başarı.

#### C5: internet

Salgın 2540..3010. İl önerileri 0, onay 0, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 491; salgına uzaklık: -2049 gün.
- Telefon uygulaması ilk açılış: 1993; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1170; salgına uzaklık: -1370 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.41 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.67 | 0.33 | 49 | -254.69 | 0 | 0 | 0 | 0 | 0.995 |
| 3 | 0.94 | 0.37 | 120 | 768.86 | 0 | 0 | 0 | 0 | 0.992 |
| 4 | 1.20 | 0.51 | 196 | 1877.56 | 0 | 0 | 0 | 0 | 0.989 |
| 5 | 1.47 | 0.62 | 291 | 3168.50 | 0 | 0 | 0 | 0 | 0.987 |
| 6 | 1.98 | 1.05 | 472 | 3503.44 | 0 | 0 | 0 | 0 | 0.983 |
| 7 | 8.51 | 2.05 | 851 | 11364.24 | 0 | 18 | 130 | 9944 | 0.979 |
| 8 | 6.08 | 7.14 | 2285 | 58022.70 | 0 | 365 | 2285 | 168129 | 0.757 |
| 9 | 7.70 | 2.50 | 282 | -30.14 | 0 | 87 | 185 | 13566 | 0.894 |
| 10 | 9.84 | 2.13 | 183 | 57.38 | 0 | 0 | 0 | 0 | 0.960 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 0 (onay 0 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 1 ürün işlemi, 1 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 0.00 | 0.00 |
| 3 | tr | Radyo | 0.00 | 0.00 |
| 3 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 3 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 0.00 | 0.00 |
| 4 | tr | Radyo | 0.00 | 0.00 |
| 4 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 4 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 4 | tr | Sosyal medya | 0.00 | 0.00 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 0.00 | 0.00 |
| 5 | tr | Radyo | 0.00 | 0.00 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 5 | tr | Sosyal medya | 0.00 | 0.00 |
| 5 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 6 | tr | Televizyon | 0.00 | 0.00 |
| 6 | tr | Radyo | 0.00 | 0.00 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 6 | tr | Sosyal medya | 0.00 | 0.00 |
| 6 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 7 | tr | Televizyon | 0.00 | 0.00 |
| 7 | tr | Radyo | 0.00 | 0.00 |
| 7 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 7 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 7 | tr | Sosyal medya | 0.00 | 0.00 |
| 7 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 8 | tr | Televizyon | 0.00 | 0.00 |
| 8 | tr | Radyo | 0.00 | 0.00 |
| 8 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 8 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 8 | tr | Sosyal medya | 0.00 | 0.00 |
| 8 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 9 | tr | Televizyon | 0.00 | 0.00 |
| 9 | tr | Radyo | 0.00 | 0.00 |
| 9 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 9 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 9 | tr | Sosyal medya | 0.00 | 0.00 |
| 9 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 10 | tr | Televizyon | 0.00 | 0.00 |
| 10 | tr | Radyo | 0.00 | 0.00 |
| 10 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 10 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 10 | tr | Sosyal medya | 0.00 | 0.00 |
| 10 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 11 | tr | Televizyon | 0.00 | 0.00 |
| 11 | tr | Radyo | 0.00 | 0.00 |
| 11 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 11 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 11 | tr | Sosyal medya | 0.00 | 0.00 |
| 11 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Kaya Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 1, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 667. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Dengeli / tohum 21

- İlk şube: 320. gün.
- 5 mağaza: 667. gün.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 695. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Mal alımi (stok yatırımi): 3792804,74 TL.
- Üst yönetim ücretleri: 357364,34 TL.
- Aile dükkânınin ücretleri: 344759,00 TL.
- İşletme giderleri (ücret hariç): 302507,00 TL.
- Vergi ödemeleri: 261668,38 TL.

Arka planin kasaya toplam net etkisi: -4203884,90 TL. Reddedilen komut: 15.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: 734441,22 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -882875,82 TL.
- Internet satışi: -3093,56 TL.
- Subeler: 322779,86 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 28 | 20 | 3 | 445441 / 2649343793 |
| 2 | 27 | 20 | 7 | 1087386 / 2742359338 |
| 3 | 27 | 20 | 7 | 1202743 / 2806522293 |
| 4 | 27 | 20 | 7 | 1167956 / 2851676281 |
| 5 | 33 | 20 | 1 | 288921 / 2934839710 |
| 6 | 33 | 20 | 1 | 294716 / 3037157017 |
| 7 | 28 | 20 | 3 | 519387 / 3095215116 |
| 8 | 30 | 20 | 3 | 463781 / 3168933777 |
| 9 | 34 | 20 | 2 | 285716 / 3289308589 |
| 10 | 35 | 25 | 1 | 271332 / 3360673294 |

Rakipler: en çok 43 etkin zincir; 1 farklı satılık zincir; 0 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 2 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 90029.96 TL; küsen farklı marka 5.

C3 kapanma nedenleri: 0 iflas/kapanma, 0 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 744, tamamlanan 280; kutlama 337. Ritim koruyucusu: 8 sakin dönem olayı, 17 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | 126144.76 | 80900.78 | 80900.78 | -35426.72 |
| durgünluk / 1 | 3280–3614 | 335 | 80997.57 | 80336.74 | 59521.85 | -152.39 |
| büyük salgın / 1 | 3674–4145 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 1 | 4223–5075 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| toparlanma / 1 | 5135–5499 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 7753–7936 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7998–8543 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 360. gün 0 -> 1.
- Süt ürünleri: 750. gün 0 -> 1.
- Temizlik ve bakım: 750. gün 0 -> 1.
- İçecek: 840. gün 0 -> 1.
- İçecek: 1620. gün 1 -> 0.
- Süt ürünleri: 1620. gün 1 -> 0.
- Kuru gıda: 1620. gün 1 -> 0.
- Temizlik ve bakım: 1620. gün 1 -> 0.
- Kuru gıda: 2460. gün 0 -> 1.
- Kuru gıda: 3420. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -1174.30 / -21.51 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 0. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 88.
- RejectBrandOffer: 154.
- SetDepartment: 25.
- SetSourcing: 10.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 134.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 134.
- Önce yerel pay %35.: 18.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 77.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 1; kapanan şube 13; borç sınırı ihlali 45 ay; şirket faizi 54.199,17 TL; en yüksek limit kullanımı 99.358,25 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 1, yabancı kapı kolu 1.
Kurtarma günleri: 1655

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 13 deneme / 13 başarı.
- CorpLoan: 2 deneme / 2 başarı.
- LineAuto: 2 deneme / 2 başarı.
- OpenLine: 2 deneme / 2 başarı.
- Restructure: 6 deneme / 6 başarı.

#### C5: internet

Salgın 3676..4153. İl önerileri 0, onay 0, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 2409; salgına uzaklık: -1267 gün.
- Telefon uygulaması ilk açılış: 0; salgına uzaklık: 0 gün.
- HızlıSepet ilk açılış: 2234; salgına uzaklık: -1442 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 4 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 5 | 0.65 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 0.995 |
| 6 | 0.91 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 0.992 |
| 7 | 1.17 | 0.52 | 57 | 200.77 | 0 | 0 | 0 | 0 | 0.990 |
| 8 | 1.44 | 0.63 | 105 | -218.55 | 0 | 0 | 0 | 0 | 0.987 |
| 9 | 1.90 | 0.61 | 99 | -827.52 | 0 | 0 | 0 | 0 | 0.984 |
| 10 | 2.41 | 0.52 | 51 | -2248.26 | 0 | 0 | 0 | 0 | 0.978 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 0 (onay 0 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 6 ürün işlemi, 6 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 0.00 | 0.00 |
| 3 | tr | Radyo | 0.00 | 0.00 |
| 3 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 3 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 0.00 | 0.00 |
| 4 | tr | Radyo | 0.00 | 0.00 |
| 4 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 4 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 4 | tr | Sosyal medya | 0.00 | 0.00 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 0.00 | 0.00 |
| 5 | tr | Radyo | 0.00 | 0.00 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 5 | tr | Sosyal medya | 0.00 | 0.00 |
| 5 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 6 | tr | Televizyon | 0.00 | 0.00 |
| 6 | tr | Radyo | 0.00 | 0.00 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 6 | tr | Sosyal medya | 0.00 | 0.00 |
| 6 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 7 | tr | Televizyon | 0.00 | 0.00 |
| 7 | tr | Radyo | 0.00 | 0.00 |
| 7 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 7 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 7 | tr | Sosyal medya | 0.00 | 0.00 |
| 7 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 8 | tr | Televizyon | 0.00 | 0.00 |
| 8 | tr | Radyo | 0.00 | 0.00 |
| 8 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 8 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 8 | tr | Sosyal medya | 0.00 | 0.00 |
| 8 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 9 | tr | Televizyon | 0.00 | 0.00 |
| 9 | tr | Radyo | 0.00 | 0.00 |
| 9 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 9 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 9 | tr | Sosyal medya | 0.00 | 0.00 |
| 9 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 10 | tr | Televizyon | 0.00 | 0.00 |
| 10 | tr | Radyo | 0.00 | 0.00 |
| 10 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 10 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 10 | tr | Sosyal medya | 0.00 | 0.00 |
| 10 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 11 | tr | Televizyon | 0.00 | 0.00 |
| 11 | tr | Radyo | 0.00 | 0.00 |
| 11 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 11 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 11 | tr | Sosyal medya | 0.00 | 0.00 |
| 11 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Demir Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 1, tamamen odenen 1, yeni planla degisen 0; silinen kredi borcu 13022880 kurus. Yeni kredi/şube denenmeyen plan günu 730. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Dengeli / tohum 22

- İlk şube: 306. gün.
- 5 mağaza: 471. gün.
- İlk depo: 1331. gün.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 569. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Mal alımi (stok yatırımi): 2451919,96 TL.
- Üst yönetim ücretleri: 1437894,50 TL.
- Depo ve merkez giderleri: 1011015,33 TL.
- Vergi ödemeleri: 571326,00 TL.
- Aile dükkânınin ücretleri: 379738,50 TL.

Arka planin kasaya toplam net etkisi: -1928955,46 TL. Reddedilen komut: 345.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: 388097,20 TL.
- Depo ve merkez: -1011015,33 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -2150684,78 TL.
- Internet satışi: 7977,60 TL.
- Subeler: 2168451,56 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 28 | 20 | 3 | 524594 / 2585369432 |
| 2 | 27 | 20 | 7 | 1373349 / 2664411879 |
| 3 | 27 | 20 | 7 | 1451347 / 2739396267 |
| 4 | 27 | 20 | 8 | 1815738 / 2818767366 |
| 5 | 27 | 20 | 8 | 1806290 / 2940529417 |
| 6 | 27 | 20 | 8 | 1734654 / 3007185519 |
| 7 | 27 | 20 | 8 | 1735272 / 3127550970 |
| 8 | 35 | 24 | 1 | 153826 / 3215860338 |
| 9 | 36 | 32 | 1 | 120111 / 3285197661 |
| 10 | 35 | 32 | 1 | 72 / 3425491994 |

Rakipler: en çok 61 etkin zincir; 6 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 4 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 81507.77 TL; küsen farklı marka 11.

C3 kapanma nedenleri: 0 iflas/kapanma, 4 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 727, tamamlanan 243; kutlama 327. Ritim koruyucusu: 6 sakin dönem olayı, 20 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 2306–2480 | 175 | 169797.10 | 61283.99 | 61283.99 | -107699.75 |
| durgünluk / 1 | 2481–2814 | 334 | 61497.42 | -31491.13 | -222272.89 | -364182.37 |
| büyük salgın / 1 | 2874–3346 | 473 | 7789.92 | -49764.50 | -62498.91 | -488762.70 |
| yüksek enflasyon / 1 | 3424–4275 | 230 | -43617.54 | -77617.62 | -77617.62 | -287539.95 |
| toparlanma / 1 | 4335–4699 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6953–7136 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7198–7744 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 330. gün 0 -> 1.
- Süt ürünleri: 600. gün 0 -> 1.
- Temizlik ve bakım: 630. gün 0 -> 1.
- İçecek: 810. gün 0 -> 1.
- İçecek: 2700. gün 1 -> 0.
- Süt ürünleri: 2700. gün 1 -> 0.
- Temizlik ve bakım: 2700. gün 1 -> 0.
- Kuru gıda: 2730. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -1129.34 / -40.58 (zarar görüldü).
- Tür 2, Manav: -406.26 / 5152.00 (zarar görüldü).
- Tür 2, Kasap: -1185.95 / 6025.35 (zarar görüldü).
- Tür 2, Fırın ve pastane: -142.70 / 2061.93 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 0. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 79.
- RejectBrandOffer: 159.
- SetDepartment: 19.
- SetSourcing: 8.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 38.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 38.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 22.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 9; kapanan şube 7; borç sınırı ihlali 38 ay; şirket faizi 55.610,48 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 6, yabancı kapı kolu 4.
Kurtarma günleri: 2742 2839 2969 3064 3168 3260 3352 3447 3561

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 5 deneme / 5 başarı.
- CorpLoan: 1 deneme / 1 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 11 deneme / 11 başarı.

#### C5: internet

Salgın 2887..3324. İl önerileri 0, onay 0, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 897; salgına uzaklık: -1990 gün.
- Telefon uygulaması ilk açılış: 2340; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 2888; salgına uzaklık: 1 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.42 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.69 | 0.27 | 109 | 1379.13 | 0 | 0 | 0 | 0 | 0.994 |
| 4 | 0.95 | 0.34 | 196 | 2096.03 | 0 | 0 | 0 | 0 | 0.992 |
| 5 | 1.22 | 0.39 | 232 | 2956.20 | 0 | 0 | 0 | 0 | 0.989 |
| 6 | 1.48 | 0.43 | 265 | 3547.70 | 0 | 0 | 0 | 0 | 0.987 |
| 7 | 2.01 | 1.01 | 581 | 3696.84 | 0 | 0 | 0 | 0 | 0.983 |
| 8 | 8.51 | 1.37 | 372 | -1000.39 | 0 | 36 | 14 | 1695 | 0.967 |
| 9 | 6.08 | 1.66 | 54 | -2292.21 | 0 | 366 | 54 | 11140 | 0.756 |
| 10 | 8.00 | 2.19 | 74 | -2405.70 | 0 | 35 | 7 | 965 | 0.927 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 0 (onay 0 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 1 ürün işlemi, 1 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 0.00 | 0.00 |
| 3 | tr | Radyo | 0.00 | 0.00 |
| 3 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 3 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 0.00 | 0.00 |
| 4 | tr | Radyo | 0.00 | 0.00 |
| 4 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 4 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 4 | tr | Sosyal medya | 0.00 | 0.00 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 0.00 | 0.00 |
| 5 | tr | Radyo | 0.00 | 0.00 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 5 | tr | Sosyal medya | 0.00 | 0.00 |
| 5 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 6 | tr | Televizyon | 0.00 | 0.00 |
| 6 | tr | Radyo | 0.00 | 0.00 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 6 | tr | Sosyal medya | 0.00 | 0.00 |
| 6 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 7 | tr | Televizyon | 0.00 | 0.00 |
| 7 | tr | Radyo | 0.00 | 0.00 |
| 7 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 7 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 7 | tr | Sosyal medya | 0.00 | 0.00 |
| 7 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 8 | tr | Televizyon | 0.00 | 0.00 |
| 8 | tr | Radyo | 0.00 | 0.00 |
| 8 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 8 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 8 | tr | Sosyal medya | 0.00 | 0.00 |
| 8 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 9 | tr | Televizyon | 0.00 | 0.00 |
| 9 | tr | Radyo | 0.00 | 0.00 |
| 9 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 9 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 9 | tr | Sosyal medya | 0.00 | 0.00 |
| 9 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 10 | tr | Televizyon | 0.00 | 0.00 |
| 10 | tr | Radyo | 0.00 | 0.00 |
| 10 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 10 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 10 | tr | Sosyal medya | 0.00 | 0.00 |
| 10 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 11 | tr | Televizyon | 0.00 | 0.00 |
| 11 | tr | Radyo | 0.00 | 0.00 |
| 11 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 11 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 11 | tr | Sosyal medya | 0.00 | 0.00 |
| 11 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Yılmaz Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 9, tamamen odenen 0, yeni planla degisen 8; silinen kredi borcu 102301554 kurus. Yeni kredi/şube denenmeyen plan günu 911. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Dengeli / tohum 23

- İlk şube: 334. gün.
- 5 mağaza: 471. gün.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 569. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Mal alımi (stok yatırımi): 3844196,46 TL.
- Üst yönetim ücretleri: 813355,23 TL.
- Vergi ödemeleri: 388378,33 TL.
- Aile dükkânınin ücretleri: 366769,00 TL.
- İşletme giderleri (ücret hariç): 265108,33 TL.

Arka planin kasaya toplam net etkisi: -4124381,30 TL. Reddedilen komut: 15.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: 786583,88 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -1533994,70 TL.
- Internet satışi: 4487,93 TL.
- Subeler: 981591,78 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 3 | 409089 / 2616019729 |
| 2 | 27 | 20 | 7 | 1359811 / 2664104721 |
| 3 | 27 | 20 | 7 | 1347412 / 2766262382 |
| 4 | 27 | 20 | 7 | 1326095 / 2881109386 |
| 5 | 27 | 20 | 7 | 1311938 / 2996229747 |
| 6 | 27 | 20 | 7 | 1261365 / 3089990654 |
| 7 | 34 | 20 | 1 | 280988 / 3158913001 |
| 8 | 35 | 27 | 1 | 199117 / 3214436872 |
| 9 | 35 | 27 | 1 | 223015 / 3297649008 |
| 10 | 35 | 27 | 1 | 241717 / 3417996779 |

Rakipler: en çok 51 etkin zincir; 3 farklı satılık zincir; 3 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 4 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 90342.61 TL; küsen farklı marka 5.

C3 kapanma nedenleri: 0 iflas/kapanma, 3 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 740, tamamlanan 274; kutlama 341. Ritim koruyucusu: 6 sakin dönem olayı, 15 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 1967–2141 | 175 | 136138.24 | 86513.99 | 85587.77 | -48965.02 |
| durgünluk / 1 | 2142–2475 | 334 | 87002.87 | 11576.87 | -129585.21 | -212239.16 |
| büyük salgın / 1 | 2535–3006 | 472 | 40822.65 | 27743.58 | 13735.22 | -4718.50 |
| yüksek enflasyon / 1 | 3084–3936 | 570 | 25212.30 | 46131.25 | 22909.75 | 31978.38 |
| toparlanma / 1 | 3996–4360 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6614–6797 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 6859–7404 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 360. gün 0 -> 1.
- Süt ürünleri: 600. gün 0 -> 1.
- İçecek: 630. gün 0 -> 1.
- Temizlik ve bakım: 630. gün 0 -> 1.
- İçecek: 2400. gün 1 -> 0.
- Süt ürünleri: 2430. gün 1 -> 0.
- Kuru gıda: 2430. gün 1 -> 0.
- Temizlik ve bakım: 2430. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -662.41 / -45.49 (zarar görüldü).
- Tür 2, Manav: -458.13 / 1816.51 (zarar görüldü).
- Tür 2, Kasap: -475.99 / 2090.66 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 0. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 77.
- RejectBrandOffer: 165.
- SetDepartment: 7.
- SetSourcing: 8.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 27.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 27.
- Önce yerel pay %35.: 14.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 9.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 1; kapanan şube 6; borç sınırı ihlali 48 ay; şirket faizi 54.280,71 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 3, yabancı kapı kolu 2.
Kurtarma günleri: 2448

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 2 deneme / 2 başarı.
- CorpLoan: 1 deneme / 1 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 11 deneme / 11 başarı.

#### C5: internet

Salgın 2540..3010. İl önerileri 0, onay 0, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 365; salgına uzaklık: -2175 gün.
- Telefon uygulaması ilk açılış: 0; salgına uzaklık: 0 gün.
- HızlıSepet ilk açılış: 1450; salgına uzaklık: -1090 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.41 | 0.00 | 0 | -7.24 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.67 | 0.45 | 168 | 962.23 | 0 | 0 | 0 | 0 | 0.995 |
| 3 | 0.94 | 0.29 | 164 | 714.21 | 0 | 0 | 0 | 0 | 0.992 |
| 4 | 1.20 | 0.30 | 167 | 796.86 | 0 | 0 | 0 | 0 | 0.989 |
| 5 | 1.47 | 0.48 | 301 | 1914.84 | 0 | 0 | 0 | 0 | 0.987 |
| 6 | 1.98 | 0.54 | 333 | 1966.02 | 0 | 0 | 0 | 0 | 0.983 |
| 7 | 8.51 | 0.68 | 291 | 912.16 | 0 | 18 | 10 | 1074 | 0.979 |
| 8 | 6.08 | 1.85 | 146 | -800.46 | 0 | 365 | 146 | 16690 | 0.757 |
| 9 | 7.70 | 1.44 | 129 | -1350.19 | 0 | 87 | 25 | 3777 | 0.894 |
| 10 | 9.84 | 1.98 | 175 | -620.50 | 0 | 0 | 0 | 0 | 0.960 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 0 (onay 0 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 0 ürün işlemi, 0 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 0.00 | 0.00 |
| 3 | tr | Radyo | 0.00 | 0.00 |
| 3 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 3 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 0.00 | 0.00 |
| 4 | tr | Radyo | 0.00 | 0.00 |
| 4 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 4 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 4 | tr | Sosyal medya | 0.00 | 0.00 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 0.00 | 0.00 |
| 5 | tr | Radyo | 0.00 | 0.00 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 5 | tr | Sosyal medya | 0.00 | 0.00 |
| 5 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 6 | tr | Televizyon | 0.00 | 0.00 |
| 6 | tr | Radyo | 0.00 | 0.00 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 6 | tr | Sosyal medya | 0.00 | 0.00 |
| 6 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 7 | tr | Televizyon | 0.00 | 0.00 |
| 7 | tr | Radyo | 0.00 | 0.00 |
| 7 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 7 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 7 | tr | Sosyal medya | 0.00 | 0.00 |
| 7 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 8 | tr | Televizyon | 0.00 | 0.00 |
| 8 | tr | Radyo | 0.00 | 0.00 |
| 8 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 8 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 8 | tr | Sosyal medya | 0.00 | 0.00 |
| 8 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 9 | tr | Televizyon | 0.00 | 0.00 |
| 9 | tr | Radyo | 0.00 | 0.00 |
| 9 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 9 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 9 | tr | Sosyal medya | 0.00 | 0.00 |
| 9 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 10 | tr | Televizyon | 0.00 | 0.00 |
| 10 | tr | Radyo | 0.00 | 0.00 |
| 10 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 10 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 10 | tr | Sosyal medya | 0.00 | 0.00 |
| 10 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 11 | tr | Televizyon | 0.00 | 0.00 |
| 11 | tr | Radyo | 0.00 | 0.00 |
| 11 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 11 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 11 | tr | Sosyal medya | 0.00 | 0.00 |
| 11 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Kaya Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 1, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 3057902 kurus. Yeni kredi/şube denenmeyen plan günu 730. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Atak / tohum 21

- İlk şube: 187. gün.
- 5 mağaza: 296. gün.
- İlk depo: 295. gün.
- İlk il müdüru: 1940. gün.
- 5 il: 527. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 30080671,42 TL.
- Üst yönetim ücretleri: 9691936,58 TL.
- Depo ve merkez giderleri: 6248060,02 TL.
- İşletme giderleri (ücret hariç): 6035189,10 TL.
- Mal alımi (stok yatırımi): 3084346,56 TL.

Arka planin kasaya toplam net etkisi: 43419549,90 TL. Reddedilen komut: 364.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -4600266,36 TL.
- Depo ve merkez: -6248060,02 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -32486028,39 TL.
- Internet satışi: 76250,66 TL.
- Subeler: 113561144,42 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 5 | 1041500 / 2649343793 |
| 2 | 20 | 20 | 18 | 5305160 / 2742359338 |
| 3 | 10 | 20 | 21 | 12967783 / 2806522293 |
| 4 | 10 | 20 | 28 | 22636807 / 2851676281 |
| 5 | 9 | 20 | 40 | 36113337 / 2934839710 |
| 6 | 8 | 20 | 57 | 49706484 / 3037157017 |
| 7 | 8 | 20 | 79 | 68303270 / 3095215116 |
| 8 | 8 | 20 | 115 | 82202275 / 3168933777 |
| 9 | 8 | 20 | 127 | 75185566 / 3289308589 |
| 10 | 8 | 24 | 139 | 66745318 / 3360673294 |

Rakipler: en çok 81 etkin zincir; 3 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 2 satın almamız; 255 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8819 mağaza · senden çekildiği savaş: 48.
Markalardan toplam 1404517.23 TL; küsen farklı marka 7.

C3 kapanma nedenleri: 0 iflas/kapanma, 2 rakip tarafından alınma, 2 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 707, tamamlanan 260; kutlama 441. Ritim koruyucusu: 2 sakin dönem olayı, 14 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | 10730981.14 | 12109126.49 | 9417022.18 | -3050124.08 |
| durgünluk / 1 | 3280–3614 | 335 | 12116912.65 | 11889560.27 | 10268982.85 | 2151.31 |
| büyük salgın / 1 | 3674–4145 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 1 | 4223–5075 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| toparlanma / 1 | 5135–5499 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 7753–7936 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7998–8543 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 210. gün 0 -> 1.
- Süt ürünleri: 315. gün 0 -> 1.
- Temizlik ve bakım: 375. gün 0 -> 1.
- İçecek: 450. gün 0 -> 1.
- Kuru gıda: 705. gün 1 -> 2.
- Süt ürünleri: 735. gün 1 -> 2.
- Temizlik ve bakım: 735. gün 1 -> 2.
- İçecek: 750. gün 1 -> 2.
- Kuru gıda: 1560. gün 2 -> 3.
- Süt ürünleri: 2070. gün 2 -> 3.
- Temizlik ve bakım: 2175. gün 2 -> 3.
- İçecek: 2205. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -49778.73 / -38.36 (zarar görüldü).
- Tür 2, Manav: -17.80 / 24499.73 (zarar görüldü).
- Tür 2, Kasap: -524.77 / 26970.25 (zarar görüldü).
- Tür 2, Evcil hayvan: 28.08 / 5419.32.
- Tür 2, Şarküteri: -594.94 / 10206.26 (zarar görüldü).
- Tür 2, Fırın ve pastane: -1133.20 / -78.83 (zarar görüldü).
- Tür 2, Balık: -3544.41 / -196.64 (zarar görüldü).
- Tür 3, Manav: 45.64 / 388880.65.
- Tür 3, Kasap: -35.68 / 656584.42 (zarar görüldü).
- Tür 3, Bebek: -28424.40 / -168.55 (zarar görüldü).
- Tür 3, Evcil hayvan: 58.53 / 106529.42.
- Tür 3, Bahçe ve oto: -45072.31 / -3247.42 (zarar görüldü).
- Tür 3, Mevsimlik: -25209.65 / 124226.97 (zarar görüldü).
- Tür 3, Şarküteri: -79.39 / 242338.63 (zarar görüldü).
- Tür 3, Fırın ve pastane: -246.35 / 229621.51 (zarar görüldü).
- Tür 3, Balık: -29167.06 / 133659.61 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -22485.30 / 709036.53 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -42705.56 / 402044.56 (zarar görüldü).
- Tür 3, Ev ve mutfak: -5521.52 / 139524.47 (zarar görüldü).
- Tür 3, Oyuncak: -16899.46 / 221061.50 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -43096.15 / -80.56 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 74. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 65.
- RejectBrandOffer: 177.
- ReplaceMasters: 1.
- SetDepartment: 119.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 163.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 163.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 13.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 26; borç sınırı ihlali 4 ay; şirket faizi 846.603,71 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 4, kabul 2, ret 2, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 1, yabancı kapı kolu 1.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- BidChainFinanced: 4 deneme / 4 başarı.
- CloseBranch: 26 deneme / 26 başarı.
- CorpLoan: 3 deneme / 3 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 18 deneme / 18 başarı.

#### C5: internet

Salgın 3676..4153. İl önerileri 42, onay 42, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 1527; salgına uzaklık: -2149 gün.
- Telefon uygulaması ilk açılış: 3129; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 2290; salgına uzaklık: -1386 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 4 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 5 | 0.65 | 0.05 | 1993 | -16541.93 | 0 | 0 | 0 | 0 | 0.995 |
| 6 | 0.91 | 0.05 | 2975 | -16571.67 | 0 | 0 | 0 | 0 | 0.992 |
| 7 | 1.17 | 0.04 | 4996 | -13938.67 | 0 | 0 | 0 | 0 | 0.990 |
| 8 | 1.44 | 0.05 | 6410 | 4845.73 | 0 | 0 | 0 | 0 | 0.987 |
| 9 | 1.90 | 0.09 | 9399 | 30884.51 | 0 | 0 | 0 | 0 | 0.984 |
| 10 | 2.41 | 0.17 | 11882 | 87572.69 | 0 | 0 | 0 | 0 | 0.978 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 100 (onay 57 / ret 43), kapama 0 (onay 0 / ret 0). Stok eritme: 16 ürün işlemi, 16 ayrı gün. Reklam müdürü gideri 225880.20 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 0.00 | 0.00 |
| 3 | tr | Radyo | 0.00 | 0.00 |
| 3 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 3 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 456771.55 | 1591176.75 |
| 4 | tr | Radyo | 118420.40 | 627588.48 |
| 4 | tr | Açık hava (bilbord) | 95710.27 | 178632.60 |
| 4 | tr | Gazete ve broşür | 91142.60 | 273369.22 |
| 4 | tr | Sosyal medya | 23682.98 | 133233.04 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 808954.32 | 4217378.48 |
| 5 | tr | Radyo | 168530.36 | 1027308.29 |
| 5 | tr | Açık hava (bilbord) | 409397.61 | 660238.61 |
| 5 | tr | Gazete ve broşür | 190366.96 | 327217.55 |
| 5 | tr | Sosyal medya | 33705.03 | 223875.42 |
| 5 | tr | Arama ve uygulama reklamı | 15387.70 | 0.00 |
| 6 | tr | Televizyon | 882115.05 | 6876319.37 |
| 6 | tr | Radyo | 183772.56 | 1514939.77 |
| 6 | tr | Açık hava (bilbord) | 801727.80 | 1669264.11 |
| 6 | tr | Gazete ve broşür | 319938.87 | 502970.46 |
| 6 | tr | Sosyal medya | 36752.86 | 403019.31 |
| 6 | tr | Arama ve uygulama reklamı | 22050.93 | 0.00 |
| 7 | tr | Televizyon | 967208.92 | 10015207.60 |
| 7 | tr | Radyo | 201500.39 | 2173919.80 |
| 7 | tr | Açık hava (bilbord) | 902728.66 | 2550823.72 |
| 7 | tr | Gazete ve broşür | 505877.57 | 706470.13 |
| 7 | tr | Sosyal medya | 40297.95 | 690719.45 |
| 7 | tr | Arama ve uygulama reklamı | 24178.66 | 0.00 |
| 8 | tr | Televizyon | 1073602.14 | 14713156.94 |
| 8 | tr | Radyo | 223665.36 | 3186986.00 |
| 8 | tr | Açık hava (bilbord) | 1002028.16 | 3754534.64 |
| 8 | tr | Gazete ve broşür | 816660.61 | 957165.47 |
| 8 | tr | Sosyal medya | 44731.32 | 1176141.04 |
| 8 | tr | Arama ve uygulama reklamı | 26837.81 | 0.00 |
| 9 | tr | Televizyon | 1211742.07 | 17346280.91 |
| 9 | tr | Radyo | 252444.51 | 3756235.26 |
| 9 | tr | Açık hava (bilbord) | 1130958.76 | 4426351.00 |
| 9 | tr | Gazete ve broşür | 1198189.44 | 1032758.84 |
| 9 | tr | Sosyal medya | 50488.06 | 1575455.33 |
| 9 | tr | Arama ve uygulama reklamı | 30292.52 | 0.00 |
| 10 | tr | Televizyon | 1397577.14 | 16972520.50 |
| 10 | tr | Radyo | 291160.59 | 3675145.92 |
| 10 | tr | Açık hava (bilbord) | 1304405.00 | 4330866.86 |
| 10 | tr | Gazete ve broşür | 1547805.21 | 915465.48 |
| 10 | tr | Sosyal medya | 58230.72 | 1729938.15 |
| 10 | tr | Arama ve uygulama reklamı | 34937.45 | 0.00 |
| 11 | tr | Televizyon | 269106.01 | 3094838.98 |
| 11 | tr | Radyo | 56063.67 | 670139.54 |
| 11 | tr | Açık hava (bilbord) | 251165.52 | 789706.03 |
| 11 | tr | Gazete ve broşür | 308718.44 | 156646.65 |
| 11 | tr | Sosyal medya | 11212.27 | 335845.80 |
| 11 | tr | Arama ve uygulama reklamı | 6727.27 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Demir Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 0, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 0. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Atak / tohum 22

- İlk şube: 180. gün.
- 5 mağaza: 303. gün.
- İlk depo: 302. gün.
- İlk il müdüru: 547. gün.
- 5 il: 317. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 18984586,38 TL.
- Üst yönetim ücretleri: 5993173,29 TL.
- Depo ve merkez giderleri: 4217524,29 TL.
- İşletme giderleri (ücret hariç): 3452832,72 TL.
- Mal alımi (stok yatırımi): 2588473,54 TL.

Arka planin kasaya toplam net etkisi: 28639720,25 TL. Reddedilen komut: 471.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -2278962,13 TL.
- Depo ve merkez: -4217524,29 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -25551540,48 TL.
- Internet satışi: 82205,05 TL.
- Subeler: 73888160,39 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 8 | 2427572 / 2585369432 |
| 2 | 14 | 20 | 20 | 8202267 / 2664411879 |
| 3 | 10 | 20 | 21 | 11595291 / 2739396267 |
| 4 | 10 | 20 | 25 | 16941432 / 2818767366 |
| 5 | 10 | 20 | 31 | 23930453 / 2940529417 |
| 6 | 9 | 20 | 39 | 31674180 / 3007185519 |
| 7 | 9 | 20 | 46 | 33537209 / 3127550970 |
| 8 | 9 | 24 | 63 | 42299967 / 3215860338 |
| 9 | 9 | 32 | 71 | 44022659 / 3285197661 |
| 10 | 8 | 32 | 72 | 47265498 / 3425491994 |

Rakipler: en çok 93 etkin zincir; 8 farklı satılık zincir; 7 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 1 satın almamız; 230 fiyat savaşı.
Ezeli rakip: Ezeli rakip: Migron (Selin Tuna) · 1947 mağaza · senden çekildiği savaş: 33.
Markalardan toplam 841068.49 TL; küsen farklı marka 7.

C3 kapanma nedenleri: 0 iflas/kapanma, 6 rakip tarafından alınma, 1 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 716, tamamlanan 287; kutlama 479. Ritim koruyucusu: 0 sakin dönem olayı, 18 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 2306–2480 | 175 | 3485761.51 | 3629523.76 | 3433078.60 | 391961.01 |
| durgünluk / 1 | 2481–2814 | 334 | 3639897.39 | 5075905.83 | 3639897.39 | 2616143.24 |
| büyük salgın / 1 | 2874–3346 | 473 | 5359026.08 | 6447029.13 | 5359026.08 | 2331007.62 |
| yüksek enflasyon / 1 | 3424–4275 | 230 | 7280528.66 | 8807065.45 | 7280528.66 | 2299665.71 |
| toparlanma / 1 | 4335–4699 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6953–7136 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7198–7744 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 195. gün 0 -> 1.
- Süt ürünleri: 315. gün 0 -> 1.
- İçecek: 330. gün 0 -> 1.
- Temizlik ve bakım: 330. gün 0 -> 1.
- Kuru gıda: 570. gün 1 -> 2.
- Süt ürünleri: 705. gün 1 -> 2.
- Temizlik ve bakım: 720. gün 1 -> 2.
- İçecek: 735. gün 1 -> 2.
- Kuru gıda: 1800. gün 2 -> 3.
- Süt ürünleri: 2565. gün 2 -> 3.
- İçecek: 2685. gün 2 -> 3.
- Temizlik ve bakım: 2805. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -11854.98 / -36.41 (zarar görüldü).
- Tür 2, Manav: -162.36 / 21899.08 (zarar görüldü).
- Tür 2, Kasap: -288.48 / 25862.54 (zarar görüldü).
- Tür 2, Bebek: -7145.39 / -518.72 (zarar görüldü).
- Tür 2, Evcil hayvan: 16.59 / 4931.41.
- Tür 2, Şarküteri: -2358.76 / 7979.03 (zarar görüldü).
- Tür 2, Fırın ve pastane: -1270.21 / 10326.91 (zarar görüldü).
- Tür 2, Balık: -2234.54 / -82.51 (zarar görüldü).
- Tür 3, Manav: -294.90 / 323511.46 (zarar görüldü).
- Tür 3, Kasap: -46.71 / 558243.62 (zarar görüldü).
- Tür 3, Bebek: -9734.04 / -271.11 (zarar görüldü).
- Tür 3, Evcil hayvan: 91.61 / 88368.49.
- Tür 3, Bahçe ve oto: -5147.16 / -136.13 (zarar görüldü).
- Tür 3, Mevsimlik: -3955.58 / 75075.76 (zarar görüldü).
- Tür 3, Şarküteri: -552.72 / 220636.67 (zarar görüldü).
- Tür 3, Fırın ve pastane: -823.77 / 223650.66 (zarar görüldü).
- Tür 3, Balık: -6812.22 / 41857.26 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -28268.33 / 223597.68 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -13272.67 / 217981.40 (zarar görüldü).
- Tür 3, Ev ve mutfak: -1558.64 / 140970.19 (zarar görüldü).
- Tür 3, Oyuncak: -10992.62 / 72870.88 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -3663.73 / 62524.74 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 46. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 60.
- RejectBrandOffer: 182.
- ReplaceMasters: 2.
- SetDepartment: 132.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 233.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 233.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 14.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 15; borç sınırı ihlali 5 ay; şirket faizi 1.333.285,24 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 1, kabul 1, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 6, yabancı kapı kolu 4.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- BidChainFinanced: 1 deneme / 1 başarı.
- CloseBranch: 15 deneme / 15 başarı.
- CorpLoan: 4 deneme / 4 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 17 deneme / 17 başarı.

#### C5: internet

Salgın 2887..3324. İl önerileri 37, onay 37, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 701; salgına uzaklık: -2186 gün.
- Telefon uygulaması ilk açılış: 2339; salgına uzaklık: -548 gün.
- HızlıSepet ilk açılış: 1429; salgına uzaklık: -1458 gün.
- Hızlı teslimat ilk açılış: 3095; salgına uzaklık: 208 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.42 | 0.01 | 35 | -2283.34 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.69 | 0.14 | 1325 | -19043.18 | 0 | 0 | 0 | 0 | 0.994 |
| 4 | 0.95 | 0.09 | 1372 | -19543.29 | 0 | 0 | 0 | 0 | 0.992 |
| 5 | 1.22 | 0.09 | 3154 | -15554.17 | 0 | 0 | 0 | 0 | 0.989 |
| 6 | 1.48 | 0.08 | 4774 | -10607.65 | 0 | 0 | 0 | 0 | 0.987 |
| 7 | 2.01 | 0.13 | 7234 | -4807.96 | 0 | 0 | 0 | 0 | 0.983 |
| 8 | 8.51 | 0.22 | 9938 | 48137.07 | 0 | 36 | 1513 | 548145 | 0.967 |
| 9 | 6.08 | 0.49 | 16653 | 142622.44 | 10 | 366 | 16653 | 5768780 | 0.756 |
| 10 | 8.00 | 0.34 | 15901 | -36714.87 | 10 | 35 | 1891 | 518156 | 0.927 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 38 (onay 21 / ret 17), kapama 0 (onay 0 / ret 0). Stok eritme: 9 ürün işlemi, 9 ayrı gün. Reklam müdürü gideri 253912.80 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 142884.22 | 406129.09 |
| 3 | tr | Radyo | 74165.18 | 306622.58 |
| 3 | tr | Açık hava (bilbord) | 38790.67 | 89690.46 |
| 3 | tr | Gazete ve broşür | 40060.19 | 123110.14 |
| 3 | tr | Sosyal medya | 14832.24 | 98008.41 |
| 3 | tr | Arama ve uygulama reklamı | 8899.10 | 0.00 |
| 4 | tr | Televizyon | 405338.92 | 1611574.20 |
| 4 | tr | Radyo | 155710.24 | 700793.53 |
| 4 | tr | Açık hava (bilbord) | 79933.36 | 191729.55 |
| 4 | tr | Gazete ve broşür | 98177.81 | 189827.06 |
| 4 | tr | Sosyal medya | 31140.41 | 199417.18 |
| 4 | tr | Arama ve uygulama reklamı | 18684.05 | 0.00 |
| 5 | tr | Televizyon | 751703.86 | 3024790.84 |
| 5 | tr | Radyo | 168530.36 | 818358.76 |
| 5 | tr | Açık hava (bilbord) | 149279.47 | 303417.89 |
| 5 | tr | Gazete ve broşür | 127304.44 | 205081.05 |
| 5 | tr | Sosyal medya | 33705.03 | 266726.88 |
| 5 | tr | Arama ve uygulama reklamı | 20221.76 | 0.00 |
| 6 | tr | Televizyon | 882115.05 | 4451362.46 |
| 6 | tr | Radyo | 183772.56 | 994975.01 |
| 6 | tr | Açık hava (bilbord) | 365220.92 | 647880.44 |
| 6 | tr | Gazete ve broşür | 240961.61 | 294418.00 |
| 6 | tr | Sosyal medya | 36752.86 | 375569.73 |
| 6 | tr | Arama ve uygulama reklamı | 22050.93 | 0.00 |
| 7 | tr | Televizyon | 987512.75 | 5916989.31 |
| 7 | tr | Radyo | 205730.55 | 1287054.77 |
| 7 | tr | Açık hava (bilbord) | 762786.85 | 1283273.22 |
| 7 | tr | Gazete ve broşür | 324377.06 | 348344.19 |
| 7 | tr | Sosyal medya | 41144.28 | 550809.13 |
| 7 | tr | Arama ve uygulama reklamı | 24685.78 | 0.00 |
| 8 | tr | Televizyon | 1131427.76 | 7426078.96 |
| 8 | tr | Radyo | 235712.67 | 1608968.83 |
| 8 | tr | Açık hava (bilbord) | 978422.87 | 1874108.47 |
| 8 | tr | Gazete ve broşür | 455251.05 | 381233.01 |
| 8 | tr | Sosyal medya | 47140.78 | 773464.72 |
| 8 | tr | Arama ve uygulama reklamı | 28283.68 | 0.00 |
| 9 | tr | Televizyon | 1262445.17 | 9584321.51 |
| 9 | tr | Radyo | 263008.30 | 2075500.08 |
| 9 | tr | Açık hava (bilbord) | 1094118.97 | 2443849.06 |
| 9 | tr | Gazete ve broşür | 702663.59 | 453589.98 |
| 9 | tr | Sosyal medya | 52599.97 | 1069315.78 |
| 9 | tr | Arama ve uygulama reklamı | 31559.18 | 0.00 |
| 10 | tr | Televizyon | 1451290.04 | 11461607.35 |
| 10 | tr | Radyo | 302350.73 | 2481846.22 |
| 10 | tr | Açık hava (bilbord) | 1257783.85 | 2924517.38 |
| 10 | tr | Gazete ve broşür | 838256.92 | 476758.67 |
| 10 | tr | Sosyal medya | 60468.42 | 1283693.90 |
| 10 | tr | Arama ve uygulama reklamı | 36280.69 | 0.00 |
| 11 | tr | Televizyon | 288120.63 | 2366204.23 |
| 11 | tr | Radyo | 60024.99 | 512365.62 |
| 11 | tr | Açık hava (bilbord) | 249704.36 | 603779.53 |
| 11 | tr | Gazete ve broşür | 172872.22 | 90878.75 |
| 11 | tr | Sosyal medya | 12004.58 | 265014.32 |
| 11 | tr | Arama ve uygulama reklamı | 7202.59 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Yılmaz Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 0, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 0. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Atak / tohum 23

- İlk şube: 180. gün.
- 5 mağaza: 289. gün.
- İlk depo: 288. gün.
- İlk il müdüru: 1779. gün.
- 5 il: 471. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 26602087,25 TL.
- Üst yönetim ücretleri: 11707041,76 TL.
- Depo ve merkez giderleri: 5848745,13 TL.
- İşletme giderleri (ücret hariç): 3568845,52 TL.
- Mal alımi (stok yatırımi): 3392663,24 TL.

Arka planin kasaya toplam net etkisi: 37568297,35 TL. Reddedilen komut: 312.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -2229777,66 TL.
- Depo ve merkez: -5848745,13 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -36037655,24 TL.
- Internet satışi: -530155,40 TL.
- Subeler: 97619593,02 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 5 | 1250275 / 2616019729 |
| 2 | 13 | 20 | 18 | 9137083 / 2664104721 |
| 3 | 10 | 20 | 22 | 17550307 / 2766262382 |
| 4 | 10 | 20 | 32 | 27628034 / 2881109386 |
| 5 | 9 | 20 | 46 | 41443589 / 2996229747 |
| 6 | 8 | 20 | 63 | 46457061 / 3089990654 |
| 7 | 8 | 20 | 74 | 57327149 / 3158913001 |
| 8 | 8 | 27 | 85 | 52793379 / 3214436872 |
| 9 | 8 | 27 | 83 | 54776905 / 3297649008 |
| 10 | 8 | 27 | 83 | 55258824 / 3417996779 |

Rakipler: en çok 89 etkin zincir; 5 farklı satılık zincir; 5 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 252 fiyat savaşı.
Ezeli rakip: Ezeli rakip: ŞAK (Deniz Arslan) · 3433 mağaza · senden çekildiği savaş: 41.
Markalardan toplam 1220481.02 TL; küsen farklı marka 7.

C3 kapanma nedenleri: 0 iflas/kapanma, 5 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 712, tamamlanan 272; kutlama 470. Ritim koruyucusu: 1 sakin dönem olayı, 14 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 1967–2141 | 175 | 4484668.35 | 4748356.26 | 4036980.01 | 507767.88 |
| durgünluk / 1 | 2142–2475 | 334 | 4770756.76 | 6201811.02 | 4549275.99 | 3189565.32 |
| büyük salgın / 1 | 2535–3006 | 472 | 6256763.79 | 6553172.46 | 6256763.79 | 1486169.47 |
| yüksek enflasyon / 1 | 3084–3936 | 570 | 7556718.99 | 11319691.68 | 7556718.99 | 5148559.53 |
| toparlanma / 1 | 3996–4360 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6614–6797 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 6859–7404 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 195. gün 0 -> 1.
- Süt ürünleri: 315. gün 0 -> 1.
- Temizlik ve bakım: 315. gün 0 -> 1.
- İçecek: 375. gün 0 -> 1.
- Kuru gıda: 525. gün 1 -> 2.
- Süt ürünleri: 705. gün 1 -> 2.
- Temizlik ve bakım: 720. gün 1 -> 2.
- İçecek: 735. gün 1 -> 2.
- Kuru gıda: 1440. gün 2 -> 3.
- Süt ürünleri: 1815. gün 2 -> 3.
- İçecek: 1995. gün 2 -> 3.
- Temizlik ve bakım: 1995. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -9859.00 / -31.22 (zarar görüldü).
- Tür 2, Manav: -265.85 / 27325.29 (zarar görüldü).
- Tür 2, Kasap: -743.19 / 32977.51 (zarar görüldü).
- Tür 2, Bebek: -1806.01 / -73.42 (zarar görüldü).
- Tür 2, Evcil hayvan: 20.61 / 6451.62.
- Tür 2, Şarküteri: -419.92 / 12877.87 (zarar görüldü).
- Tür 2, Fırın ve pastane: -599.17 / -32.27 (zarar görüldü).
- Tür 2, Balık: -3336.45 / -122.35 (zarar görüldü).
- Tür 3, Manav: -58.16 / 417959.55 (zarar görüldü).
- Tür 3, Kasap: -186.02 / 753624.52 (zarar görüldü).
- Tür 3, Bebek: -4009.71 / -207.96 (zarar görüldü).
- Tür 3, Evcil hayvan: 66.77 / 118919.44.
- Tür 3, Bahçe ve oto: -4727.11 / -179.32 (zarar görüldü).
- Tür 3, Mevsimlik: -2808.60 / 84683.58 (zarar görüldü).
- Tür 3, Şarküteri: -262.58 / 273359.89 (zarar görüldü).
- Tür 3, Fırın ve pastane: -452.18 / 282555.30 (zarar görüldü).
- Tür 3, Balık: -18133.27 / 50467.39 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -32129.97 / 278161.32 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -37740.55 / 213784.48 (zarar görüldü).
- Tür 3, Ev ve mutfak: -3743.57 / 165183.73 (zarar görüldü).
- Tür 3, Oyuncak: -14563.43 / 92246.44 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -3142.04 / 18026.97 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 70. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 63.
- RejectBrandOffer: 179.
- ReplaceMasters: 2.
- SetDepartment: 96.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 170.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 170.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 12.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 10; borç sınırı ihlali 4 ay; şirket faizi 1.043.067,77 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 3, yabancı kapı kolu 2.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 10 deneme / 10 başarı.
- CorpLoan: 3 deneme / 3 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 17 deneme / 17 başarı.

#### C5: internet

Salgın 2540..3010. İl önerileri 100, onay 100, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 456; salgına uzaklık: -2084 gün.
- Telefon uygulaması ilk açılış: 1993; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1086; salgına uzaklık: -1454 gün.
- Hızlı teslimat ilk açılış: 2731; salgına uzaklık: 191 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.41 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.67 | 0.17 | 391 | -13341.17 | 0 | 0 | 0 | 0 | 0.995 |
| 3 | 0.94 | 0.07 | 1154 | -21104.17 | 0 | 0 | 0 | 0 | 0.992 |
| 4 | 1.20 | 0.07 | 3208 | -17037.31 | 0 | 0 | 0 | 0 | 0.989 |
| 5 | 1.47 | 0.05 | 5497 | -14046.98 | 0 | 0 | 0 | 0 | 0.987 |
| 6 | 1.98 | 0.08 | 7229 | -7449.46 | 0 | 0 | 0 | 0 | 0.983 |
| 7 | 8.51 | 0.13 | 9079 | 19276.15 | 0 | 18 | 659 | 362490 | 0.979 |
| 8 | 6.08 | 0.36 | 15965 | 21373.26 | 16 | 365 | 15965 | 6855469 | 0.757 |
| 9 | 7.70 | 0.27 | 14448 | -216753.76 | 16 | 87 | 4403 | 1611449 | 0.894 |
| 10 | 9.84 | 0.24 | 12962 | -281071.96 | 16 | 0 | 0 | 0 | 0.960 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 64 (onay 18 / ret 46), kapama 2 (onay 0 / ret 2). Stok eritme: 12 ürün işlemi, 12 ayrı gün. Reklam müdürü gideri 249976.26 TL (iç birim).

| Yıl | Ülke | Kanal | Harcama TL | Tahmini ek ciro TL |
|---|---|---|---:|---:|
| 1 | tr | Televizyon | 0.00 | 0.00 |
| 1 | tr | Radyo | 0.00 | 0.00 |
| 1 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 1 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 1 | tr | Sosyal medya | 0.00 | 0.00 |
| 1 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 2 | tr | Televizyon | 0.00 | 0.00 |
| 2 | tr | Radyo | 0.00 | 0.00 |
| 2 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 2 | tr | Gazete ve broşür | 0.00 | 0.00 |
| 2 | tr | Sosyal medya | 0.00 | 0.00 |
| 2 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 3 | tr | Televizyon | 93104.55 | 209830.30 |
| 3 | tr | Radyo | 24888.05 | 104569.92 |
| 3 | tr | Açık hava (bilbord) | 45319.06 | 48915.90 |
| 3 | tr | Gazete ve broşür | 19910.44 | 81480.65 |
| 3 | tr | Sosyal medya | 4977.31 | 49202.03 |
| 3 | tr | Arama ve uygulama reklamı | 2986.38 | 0.00 |
| 4 | tr | Televizyon | 694437.53 | 2834398.91 |
| 4 | tr | Radyo | 155710.24 | 843475.03 |
| 4 | tr | Açık hava (bilbord) | 150375.86 | 315048.42 |
| 4 | tr | Gazete ve broşür | 134562.65 | 294660.06 |
| 4 | tr | Sosyal medya | 31140.41 | 305396.04 |
| 4 | tr | Arama ve uygulama reklamı | 18684.05 | 0.00 |
| 5 | tr | Televizyon | 808954.32 | 4837686.93 |
| 5 | tr | Radyo | 168530.36 | 1102749.69 |
| 5 | tr | Açık hava (bilbord) | 612745.97 | 1007442.61 |
| 5 | tr | Gazete ve broşür | 233210.80 | 329696.81 |
| 5 | tr | Sosyal medya | 33705.03 | 415467.56 |
| 5 | tr | Arama ve uygulama reklamı | 20221.76 | 0.00 |
| 6 | tr | Televizyon | 900962.15 | 7102950.16 |
| 6 | tr | Radyo | 187699.24 | 1549846.84 |
| 6 | tr | Açık hava (bilbord) | 808394.30 | 1793183.24 |
| 6 | tr | Gazete ve broşür | 373042.22 | 421180.79 |
| 6 | tr | Sosyal medya | 37538.92 | 660099.62 |
| 6 | tr | Arama ve uygulama reklamı | 22522.86 | 0.00 |
| 7 | tr | Televizyon | 1020235.42 | 9068361.28 |
| 7 | tr | Radyo | 212547.82 | 1965690.00 |
| 7 | tr | Açık hava (bilbord) | 918211.27 | 2313613.30 |
| 7 | tr | Gazete ve broşür | 567804.56 | 482663.75 |
| 7 | tr | Sosyal medya | 42508.59 | 939141.12 |
| 7 | tr | Arama ve uygulama reklamı | 25503.77 | 0.00 |
| 8 | tr | Televizyon | 1143221.78 | 10859881.68 |
| 8 | tr | Radyo | 238169.31 | 2351867.08 |
| 8 | tr | Açık hava (bilbord) | 1028900.06 | 2771248.62 |
| 8 | tr | Gazete ve broşür | 746120.81 | 517510.88 |
| 8 | tr | Sosyal medya | 47632.41 | 1210007.53 |
| 8 | tr | Arama ve uygulama reklamı | 28579.14 | 0.00 |
| 9 | tr | Televizyon | 1300893.98 | 12284688.10 |
| 9 | tr | Radyo | 271017.99 | 2660075.29 |
| 9 | tr | Açık hava (bilbord) | 1170803.60 | 3134653.72 |
| 9 | tr | Gazete ve broşür | 887299.05 | 514945.13 |
| 9 | tr | Sosyal medya | 54202.32 | 1375883.62 |
| 9 | tr | Arama ve uygulama reklamı | 32520.86 | 0.00 |
| 10 | tr | Televizyon | 1611650.03 | 15899801.93 |
| 10 | tr | Radyo | 335758.86 | 3442838.67 |
| 10 | tr | Açık hava (bilbord) | 1450485.21 | 4057073.02 |
| 10 | tr | Gazete ve broşür | 1114595.34 | 575963.86 |
| 10 | tr | Sosyal medya | 67150.62 | 1780772.67 |
| 10 | tr | Arama ve uygulama reklamı | 40289.45 | 0.00 |
| 11 | tr | Televizyon | 334649.26 | 3220956.42 |
| 11 | tr | Radyo | 69718.17 | 697444.75 |
| 11 | tr | Açık hava (bilbord) | 301184.15 | 821875.32 |
| 11 | tr | Gazete ve broşür | 231465.63 | 106450.68 |
| 11 | tr | Sosyal medya | 13943.31 | 360746.07 |
| 11 | tr | Arama ve uygulama reklamı | 8365.86 | 0.00 |

Tahmin kâr değildir: C'nin mağaza ciro ek tahmini (internet cirosu dahil) beş marka kanalının akılda kalan payıyla dağıtılır (yuvarlama korunur). Arama trafik çarpanına etki etmez; internet ek cirosu C tarafından ayrı tahmin edilmediği için aramanın getirisi burada 0/ölçülmedi, etkisiz demek değil. Takvim yılları, ilk/son yıl kısmi; müdür ücreti kanal harcamasına dağıtılmaz. CSV para iç kuruş; Almanya gösterim ölçeği ayrıdır. Sokak adları/kapanışlar komuta_olaylar.csv; ilk 365 gün hava ilk_yil_hava.csv (0 güneş, 1 bulut, 2 yağmur, 3 kar, 4 sıcak).
- Sokak: Kaya Market
- Sokak: BİN
- Sokak: Migron
- Sokak: A110
- Sokak: ŞAK
- Sokak: Mahalle bakkalları
- Sokak: Salı Pazarı

#### C7: kurtarmadan sonra

Plan 0, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 0. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

## Denetimin kapsamı

Satış fisi, siparis bedeli, gün kapanisi ve mal kabul aktarimi bagimsiz hesapla kontrol edilir. Negatif stok, gecersiz sayilar ve pay sinirlari her gün denetlenir. Bagli muhasebe defterinin kasa farkı hem isaretli hem mutlak toplamla C bolumunde verilir. Kasa eksisi oyun sonu degildir. Ligler yillik, şubeler ilk 180 günun gercek defter satirlariyla olculur.

Fiyatlar normal oyuncunun kullandigi adimlarla degisir. Kredi, şube, depo, yonetici ve kararlar normal komutlardan gecer. Aile dükkânı PlayDay ile oynar; test modu, bedava mal veya para kullanilmaz. CSV tutarlari kurustur.
