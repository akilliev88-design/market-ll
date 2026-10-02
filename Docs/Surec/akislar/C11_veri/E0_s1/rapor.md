# Otomatik oyuncunun denge raporu

3 koşu, her biri 3653 gün; toplam sure 592.0 saniye.

Tune: C10 defaults

| Tarz | Tohum | Son kasa | Borç | Magaza | İl | Ulusal pay | Kasa eksi gün | Sıkıntı günu | Denetim hatasi |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Dengeli | 21 | 13023840,93 TL | 27690,82 TL | 147 | 39 | 0.6343% | 0 | 0 | 0 |
| Dengeli | 22 | 15101850,47 TL | 19150,18 TL | 141 | 39 | 0.7098% | 0 | 0 | 0 |
| Dengeli | 23 | 15482520,31 TL | 28041,83 TL | 135 | 39 | 0.6091% | 0 | 0 | 0 |

## Bulgular ve neye bakmalı

- 0/3 koşuda kasa eksiye düştü.
- 3/3 koşu sonunda birden cok mağaza acik kaldi.
- Satış, siparis, stok ve sayi denetimi: 0 hata.


### Dengeli / tohum 21

- İlk şube: 306. gün.
- 5 mağaza: 443. gün.
- İlk depo: 785. gün.
- İlk il müdüru: 2689. gün.
- 5 il: 541. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 22269291,08 TL.
- Üst yönetim ücretleri: 7240059,85 TL.
- İşletme giderleri (ücret hariç): 6310478,06 TL.
- Depo ve merkez giderleri: 5000413,59 TL.
- Mal alımi (stok yatırımi): 2975582,94 TL.

Arka planin kasaya toplam net etkisi: 44661367,10 TL. Reddedilen komut: 314.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -5169652,37 TL.
- Depo ve merkez: -5000413,59 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -23889959,65 TL.
- Internet satışi: -9239,62 TL.
- Subeler: 88313796,53 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 28 | 20 | 3 | 493097 / 2649343793 |
| 2 | 27 | 20 | 8 | 1453611 / 2742359338 |
| 3 | 27 | 20 | 9 | 1825690 / 2806522293 |
| 4 | 23 | 20 | 14 | 3851463 / 2851676281 |
| 5 | 13 | 20 | 23 | 9772975 / 2934839710 |
| 6 | 10 | 20 | 34 | 20607625 / 3037157017 |
| 7 | 9 | 20 | 52 | 38281523 / 3095215116 |
| 8 | 8 | 20 | 88 | 59183691 / 3168933777 |
| 9 | 8 | 20 | 125 | 69739344 / 3289308589 |
| 10 | 8 | 23 | 147 | 74953801 / 3360673294 |

Rakipler: en çok 95 etkin zincir; 6 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 188 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8880 mağaza · senden çekildiği savaş: 44.
Markalardan toplam 1101670.93 TL; küsen farklı marka 5.

C3 kapanma nedenleri: 0 iflas/kapanma, 4 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 717, tamamlanan 290; kutlama 544. Ritim koruyucusu: 3 sakin dönem olayı, 15 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | 9205553.60 | 10743980.47 | 8840769.48 | 1851134.64 |
| durgünluk / 1 | 3280–3614 | 335 | 10752303.90 | 13010424.16 | 10328153.34 | 3744720.76 |
| büyük salgın / 1 | 3674–4145 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 1 | 4223–5075 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| toparlanma / 1 | 5135–5499 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 7753–7936 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7998–8543 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 330. gün 0 -> 1.
- İçecek: 510. gün 0 -> 1.
- Süt ürünleri: 570. gün 0 -> 1.
- Temizlik ve bakım: 750. gün 0 -> 1.
- Kuru gıda: 1350. gün 1 -> 2.
- İçecek: 1620. gün 1 -> 2.
- Süt ürünleri: 1650. gün 1 -> 2.
- Temizlik ve bakım: 1680. gün 1 -> 2.
- Kuru gıda: 2100. gün 2 -> 3.
- Süt ürünleri: 2580. gün 2 -> 3.
- Temizlik ve bakım: 2640. gün 2 -> 3.
- İçecek: 2670. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -23250.93 / -47.48 (zarar görüldü).
- Tür 2, Manav: -13.06 / 45513.75 (zarar görüldü).
- Tür 2, Kasap: -57.75 / 53777.03 (zarar görüldü).
- Tür 2, Şarküteri: -38.83 / 23373.90 (zarar görüldü).
- Tür 2, Fırın ve pastane: -186.72 / -24.96 (zarar görüldü).
- Tür 3, Manav: 31.91 / 607502.23.
- Tür 3, Kasap: 10.34 / 856821.13.
- Tür 3, Bebek: -76258.17 / -143.70 (zarar görüldü).
- Tür 3, Evcil hayvan: 64.46 / 118179.54.
- Tür 3, Bahçe ve oto: -44476.33 / 64605.98 (zarar görüldü).
- Tür 3, Mevsimlik: -10297.94 / 146639.19 (zarar görüldü).
- Tür 3, Şarküteri: -23.42 / 352716.10 (zarar görüldü).
- Tür 3, Fırın ve pastane: -11.73 / 398180.90 (zarar görüldü).
- Tür 3, Balık: -83286.03 / 89078.83 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -6884.09 / 298017.03 (zarar görüldü).
- Tür 3, Ev ve mutfak: -11881.42 / 130611.34 (zarar görüldü).
- Tür 3, Oyuncak: -18719.22 / 173494.24 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -45410.39 / 166146.12 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 50. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 63.
- RejectBrandOffer: 179.
- SetDepartment: 117.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 111.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 111.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 7.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 17; borç sınırı ihlali 0 ay; şirket faizi 56.007,48 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 1, yabancı kapı kolu 1.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 17 deneme / 17 başarı.
- CorpLoan: 1 deneme / 1 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 7 deneme / 7 başarı.

#### C5: internet

Salgın 3676..4153. İl önerileri 52, onay 52, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 1506; salgına uzaklık: -2170 gün.
- Telefon uygulaması ilk açılış: 3129; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 2234; salgına uzaklık: -1442 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 4 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 5 | 0.65 | 0.19 | 863 | -18791.07 | 0 | 0 | 0 | 0 | 0.995 |
| 6 | 0.91 | 0.12 | 1858 | -18047.16 | 0 | 0 | 0 | 0 | 0.992 |
| 7 | 1.17 | 0.07 | 5104 | -16139.54 | 0 | 0 | 0 | 0 | 0.990 |
| 8 | 1.44 | 0.06 | 7970 | -7542.48 | 0 | 0 | 0 | 0 | 0.987 |
| 9 | 1.90 | 0.07 | 7531 | 12548.90 | 0 | 0 | 0 | 0 | 0.984 |
| 10 | 2.41 | 0.11 | 11150 | 38731.73 | 0 | 0 | 0 | 0 | 0.978 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 68 (onay 53 / ret 15), kapama 2 (onay 0 / ret 2). Stok eritme: 14 ürün işlemi, 14 ayrı gün. Reklam müdürü gideri 223697.25 TL (iç birim).

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
| 4 | tr | Gazete ve broşür | 9819.38 | 43668.99 |
| 4 | tr | Sosyal medya | 4466.44 | 20338.69 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 34956.66 | 64959.47 |
| 5 | tr | Radyo | 47158.23 | 203675.08 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 30683.45 | 174032.99 |
| 5 | tr | Sosyal medya | 17105.09 | 159735.39 |
| 5 | tr | Arama ve uygulama reklamı | 8795.27 | 0.00 |
| 6 | tr | Televizyon | 301201.26 | 1640408.19 |
| 6 | tr | Radyo | 163873.48 | 1103644.70 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 64767.43 | 178752.16 |
| 6 | tr | Sosyal medya | 36752.86 | 359906.33 |
| 6 | tr | Arama ve uygulama reklamı | 22050.93 | 0.00 |
| 7 | tr | Televizyon | 792635.43 | 5014492.28 |
| 7 | tr | Radyo | 201500.39 | 1640701.45 |
| 7 | tr | Açık hava (bilbord) | 60301.69 | 14413.07 |
| 7 | tr | Gazete ve broşür | 208103.34 | 360713.35 |
| 7 | tr | Sosyal medya | 40297.95 | 523524.50 |
| 7 | tr | Arama ve uygulama reklamı | 24178.66 | 0.00 |
| 8 | tr | Televizyon | 1073602.14 | 9742994.11 |
| 8 | tr | Radyo | 223665.36 | 2256446.84 |
| 8 | tr | Açık hava (bilbord) | 621366.31 | 980897.45 |
| 8 | tr | Gazete ve broşür | 487992.60 | 624822.72 |
| 8 | tr | Sosyal medya | 44731.32 | 839464.21 |
| 8 | tr | Arama ve uygulama reklamı | 26837.81 | 0.00 |
| 9 | tr | Televizyon | 1211742.07 | 14744096.65 |
| 9 | tr | Radyo | 252444.51 | 3226354.28 |
| 9 | tr | Açık hava (bilbord) | 1575265.76 | 3356535.12 |
| 9 | tr | Gazete ve broşür | 918311.21 | 765643.44 |
| 9 | tr | Sosyal medya | 50488.06 | 1356874.97 |
| 9 | tr | Arama ve uygulama reklamı | 30292.52 | 0.00 |
| 10 | tr | Televizyon | 1397577.14 | 16845755.87 |
| 10 | tr | Radyo | 291160.59 | 3653239.44 |
| 10 | tr | Açık hava (bilbord) | 1816851.29 | 4270533.18 |
| 10 | tr | Gazete ve broşür | 1145086.14 | 723013.45 |
| 10 | tr | Sosyal medya | 58230.72 | 1723320.75 |
| 10 | tr | Arama ve uygulama reklamı | 34937.45 | 0.00 |
| 11 | tr | Televizyon | 269106.01 | 3406734.22 |
| 11 | tr | Radyo | 56063.67 | 737981.94 |
| 11 | tr | Açık hava (bilbord) | 349838.02 | 868545.65 |
| 11 | tr | Gazete ve broşür | 310636.47 | 168086.60 |
| 11 | tr | Sosyal medya | 11212.27 | 369913.35 |
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

### Dengeli / tohum 22

- İlk şube: 334. gün.
- 5 mağaza: 457. gün.
- İlk depo: 799. gün.
- İlk il müdüru: 2619. gün.
- 5 il: 625. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 24460489,71 TL.
- Üst yönetim ücretleri: 7974669,80 TL.
- İşletme giderleri (ücret hariç): 6914907,20 TL.
- Depo ve merkez giderleri: 5394935,46 TL.
- Mal alımi (stok yatırımi): 2834120,24 TL.

Arka planin kasaya toplam net etkisi: 52042816,86 TL. Reddedilen komut: 331.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -5827378,53 TL.
- Depo ve merkez: -5394935,46 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -26004763,24 TL.
- Internet satışi: 127250,78 TL.
- Subeler: 95340320,21 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 28 | 20 | 3 | 423302 / 2585369432 |
| 2 | 27 | 20 | 7 | 1503192 / 2664411879 |
| 3 | 27 | 20 | 9 | 2208995 / 2739396267 |
| 4 | 16 | 20 | 19 | 5883948 / 2818767366 |
| 5 | 10 | 20 | 27 | 13529723 / 2940529417 |
| 6 | 9 | 20 | 40 | 25650241 / 3007185519 |
| 7 | 9 | 20 | 56 | 39663187 / 3127550970 |
| 8 | 8 | 24 | 95 | 59727367 / 3215860338 |
| 9 | 8 | 30 | 118 | 64147055 / 3285197661 |
| 10 | 7 | 28 | 141 | 83322748 / 3425491994 |

Rakipler: en çok 115 etkin zincir; 12 farklı satılık zincir; 8 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 228 fiyat savaşı.
Ezeli rakip: Ezeli rakip: ŞAK (Deniz Arslan) · 3478 mağaza · senden çekildiği savaş: 44.
Markalardan toplam 1120209.47 TL; küsen farklı marka 5.

C3 kapanma nedenleri: 0 iflas/kapanma, 8 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 717, tamamlanan 278; kutlama 530. Ritim koruyucusu: 2 sakin dönem olayı, 18 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 2306–2480 | 175 | 3276420.40 | 4369821.80 | 3209373.46 | 3021482.22 |
| durgünluk / 1 | 2481–2814 | 334 | 4392220.78 | 6814125.87 | 4290005.16 | 5262221.88 |
| büyük salgın / 1 | 2874–3346 | 473 | 7738496.87 | 10350688.15 | 7644601.11 | 6672168.34 |
| yüksek enflasyon / 1 | 3424–4275 | 230 | 11758069.31 | 15101850.47 | 11758069.31 | 5592522.67 |
| toparlanma / 1 | 4335–4699 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6953–7136 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7198–7744 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 360. gün 0 -> 1.
- İçecek: 510. gün 0 -> 1.
- Süt ürünleri: 660. gün 0 -> 1.
- Temizlik ve bakım: 660. gün 0 -> 1.
- Kuru gıda: 1290. gün 1 -> 2.
- Süt ürünleri: 1500. gün 1 -> 2.
- Temizlik ve bakım: 1500. gün 1 -> 2.
- İçecek: 1530. gün 1 -> 2.
- Kuru gıda: 1920. gün 2 -> 3.
- Süt ürünleri: 2460. gün 2 -> 3.
- Temizlik ve bakım: 2520. gün 2 -> 3.
- İçecek: 2580. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -21915.75 / 1263.81 (zarar görüldü).
- Tür 2, Manav: -48.84 / 47569.77 (zarar görüldü).
- Tür 2, Kasap: -87.66 / 56832.51 (zarar görüldü).
- Tür 2, Fırın ve pastane: -115.04 / 21841.48 (zarar görüldü).
- Tür 3, Manav: 60.75 / 645714.38.
- Tür 3, Kasap: -56.65 / 960147.03 (zarar görüldü).
- Tür 3, Bebek: -62618.78 / -139.66 (zarar görüldü).
- Tür 3, Evcil hayvan: 44.16 / 135294.94.
- Tür 3, Bahçe ve oto: -21261.28 / -164.51 (zarar görüldü).
- Tür 3, Mevsimlik: -35672.01 / 99132.72 (zarar görüldü).
- Tür 3, Şarküteri: -107.38 / 391394.14 (zarar görüldü).
- Tür 3, Fırın ve pastane: -114.84 / 430618.46 (zarar görüldü).
- Tür 3, Balık: -38934.14 / 184274.85 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -5909.39 / 131850.40 (zarar görüldü).
- Tür 3, Ev ve mutfak: -240.37 / 158052.11 (zarar görüldü).
- Tür 3, Oyuncak: -13585.56 / 174830.74 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -38703.24 / -145.79 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 48. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 62.
- RejectBrandOffer: 180.
- SetDepartment: 121.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 115.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 115.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 6.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 34; borç sınırı ihlali 0 ay; şirket faizi 33.449,14 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 6, yabancı kapı kolu 4.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 34 deneme / 34 başarı.
- CorpLoan: 1 deneme / 1 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 7 deneme / 7 başarı.

#### C5: internet

Salgın 2887..3324. İl önerileri 49, onay 49, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 715; salgına uzaklık: -2172 gün.
- Telefon uygulaması ilk açılış: 2340; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1443; salgına uzaklık: -1444 gün.
- Hızlı teslimat ilk açılış: 3081; salgına uzaklık: 194 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.42 | 0.02 | 5 | 58.50 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.69 | 0.38 | 267 | 2730.81 | 0 | 0 | 0 | 0 | 0.994 |
| 4 | 0.95 | 0.31 | 441 | -20185.76 | 0 | 0 | 0 | 0 | 0.992 |
| 5 | 1.22 | 0.28 | 2286 | -10329.26 | 0 | 0 | 0 | 0 | 0.989 |
| 6 | 1.48 | 0.17 | 4865 | -4522.46 | 0 | 0 | 0 | 0 | 0.987 |
| 7 | 2.01 | 0.18 | 8821 | 14339.60 | 0 | 0 | 0 | 0 | 0.983 |
| 8 | 8.51 | 0.23 | 11465 | 75981.53 | 0 | 36 | 1737 | 924887 | 0.967 |
| 9 | 6.08 | 0.48 | 19802 | 151686.07 | 19 | 366 | 19802 | 9994918 | 0.756 |
| 10 | 8.00 | 0.30 | 16940 | -82508.25 | 20 | 35 | 2431 | 937838 | 0.927 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 68 (onay 57 / ret 11), kapama 0 (onay 0 / ret 0). Stok eritme: 8 ürün işlemi, 8 ayrı gün. Reklam müdürü gideri 204568.93 TL (iç birim).

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
| 4 | tr | Gazete ve broşür | 15457.58 | 77602.98 |
| 4 | tr | Sosyal medya | 6232.42 | 61013.37 |
| 4 | tr | Arama ve uygulama reklamı | 3738.75 | 0.00 |
| 5 | tr | Televizyon | 158945.73 | 671154.54 |
| 5 | tr | Radyo | 83666.18 | 427482.46 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 20775.26 | 102928.10 |
| 5 | tr | Sosyal medya | 27661.42 | 344583.50 |
| 5 | tr | Arama ve uygulama reklamı | 16595.77 | 0.00 |
| 6 | tr | Televizyon | 412992.05 | 2494337.64 |
| 6 | tr | Radyo | 183772.56 | 1231501.40 |
| 6 | tr | Açık hava (bilbord) | 22014.91 | 6149.73 |
| 6 | tr | Gazete ve broşür | 132997.98 | 229326.76 |
| 6 | tr | Sosyal medya | 36752.86 | 491898.03 |
| 6 | tr | Arama ve uygulama reklamı | 22050.93 | 0.00 |
| 7 | tr | Televizyon | 919207.08 | 6271166.77 |
| 7 | tr | Radyo | 205730.55 | 1719711.17 |
| 7 | tr | Açık hava (bilbord) | 227295.11 | 209293.59 |
| 7 | tr | Gazete ve broşür | 250116.72 | 330296.71 |
| 7 | tr | Sosyal medya | 41144.28 | 743234.91 |
| 7 | tr | Arama ve uygulama reklamı | 24685.78 | 0.00 |
| 8 | tr | Televizyon | 1131427.76 | 9921158.29 |
| 8 | tr | Radyo | 235712.67 | 2231869.33 |
| 8 | tr | Açık hava (bilbord) | 821531.02 | 1366320.25 |
| 8 | tr | Gazete ve broşür | 383700.15 | 371971.26 |
| 8 | tr | Sosyal medya | 47140.78 | 1077656.63 |
| 8 | tr | Arama ve uygulama reklamı | 28283.68 | 0.00 |
| 9 | tr | Televizyon | 1262445.17 | 13851313.97 |
| 9 | tr | Radyo | 263008.30 | 3017084.81 |
| 9 | tr | Açık hava (bilbord) | 1641178.78 | 3242779.57 |
| 9 | tr | Gazete ve broşür | 709106.70 | 447013.61 |
| 9 | tr | Sosyal medya | 52599.97 | 1554615.68 |
| 9 | tr | Arama ve uygulama reklamı | 31559.18 | 0.00 |
| 10 | tr | Televizyon | 1451290.04 | 18355556.29 |
| 10 | tr | Radyo | 302350.73 | 3977762.26 |
| 10 | tr | Açık hava (bilbord) | 1886677.30 | 4661829.47 |
| 10 | tr | Gazete ve broşür | 1317497.78 | 655533.51 |
| 10 | tr | Sosyal medya | 60468.42 | 2057432.89 |
| 10 | tr | Arama ve uygulama reklamı | 36280.69 | 0.00 |
| 11 | tr | Televizyon | 288120.63 | 4198797.53 |
| 11 | tr | Radyo | 60024.99 | 909390.55 |
| 11 | tr | Açık hava (bilbord) | 374556.75 | 1070662.94 |
| 11 | tr | Gazete ve broşür | 336706.06 | 161274.77 |
| 11 | tr | Sosyal medya | 12004.58 | 470369.54 |
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

### Dengeli / tohum 23

- İlk şube: 334. gün.
- 5 mağaza: 457. gün.
- İlk depo: 785. gün.
- İlk il müdüru: 2745. gün.
- 5 il: 485. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 22816600,94 TL.
- Üst yönetim ücretleri: 7926023,63 TL.
- İşletme giderleri (ücret hariç): 6461983,05 TL.
- Depo ve merkez giderleri: 5133768,02 TL.
- Mal alımi (stok yatırımi): 2998244,34 TL.

Arka planin kasaya toplam net etkisi: 47903025,40 TL. Reddedilen komut: 321.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -5341628,79 TL.
- Depo ve merkez: -5133768,02 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -25250937,76 TL.
- Internet satışi: 23530,26 TL.
- Subeler: 88943738,84 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 3 | 419009 / 2616019729 |
| 2 | 27 | 20 | 8 | 1517115 / 2664104721 |
| 3 | 27 | 20 | 9 | 2123718 / 2766262382 |
| 4 | 20 | 20 | 16 | 4714845 / 2881109386 |
| 5 | 12 | 20 | 24 | 10919707 / 2996229747 |
| 6 | 10 | 20 | 33 | 18792556 / 3089990654 |
| 7 | 9 | 20 | 50 | 35194292 / 3158913001 |
| 8 | 8 | 27 | 77 | 49236474 / 3214436872 |
| 9 | 8 | 27 | 118 | 64645497 / 3297649008 |
| 10 | 8 | 27 | 135 | 71949469 / 3417996779 |

Rakipler: en çok 112 etkin zincir; 5 farklı satılık zincir; 5 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 211 fiyat savaşı.
Ezeli rakip: Ezeli rakip: Migron (Selin Tuna) · 1905 mağaza · senden çekildiği savaş: 32.
Markalardan toplam 1061275.84 TL; küsen farklı marka 6.

C3 kapanma nedenleri: 0 iflas/kapanma, 5 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 711, tamamlanan 253; kutlama 506. Ritim koruyucusu: 1 sakin dönem olayı, 18 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 1967–2141 | 175 | 1380138.68 | 1853127.62 | 1324895.63 | 1097764.39 |
| durgünluk / 1 | 2142–2475 | 334 | 1865737.26 | 3594338.88 | 1829739.05 | 3752088.49 |
| büyük salgın / 1 | 2535–3006 | 472 | 4096697.62 | 7265379.57 | 3851163.85 | 7708015.79 |
| yüksek enflasyon / 1 | 3084–3936 | 570 | 8324331.25 | 15482520.31 | 8324331.25 | 11527669.40 |
| toparlanma / 1 | 3996–4360 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6614–6797 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 6859–7404 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 360. gün 0 -> 1.
- İçecek: 510. gün 0 -> 1.
- Süt ürünleri: 510. gün 0 -> 1.
- Temizlik ve bakım: 570. gün 0 -> 1.
- Kuru gıda: 1320. gün 1 -> 2.
- İçecek: 1560. gün 1 -> 2.
- Süt ürünleri: 1590. gün 1 -> 2.
- Temizlik ve bakım: 1650. gün 1 -> 2.
- Kuru gıda: 2070. gün 2 -> 3.
- İçecek: 2670. gün 2 -> 3.
- Süt ürünleri: 2670. gün 2 -> 3.
- Temizlik ve bakım: 2760. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -28325.36 / 1172.96 (zarar görüldü).
- Tür 2, Manav: -15.42 / 54773.88 (zarar görüldü).
- Tür 2, Kasap: -61.97 / 65933.79 (zarar görüldü).
- Tür 2, Şarküteri: -40.24 / 28417.66 (zarar görüldü).
- Tür 2, Fırın ve pastane: -156.23 / -15.39 (zarar görüldü).
- Tür 3, Manav: 51.25 / 701232.13.
- Tür 3, Kasap: -15.28 / 1029361.43 (zarar görüldü).
- Tür 3, Bebek: -34774.19 / -159.12 (zarar görüldü).
- Tür 3, Evcil hayvan: 68.81 / 145038.89.
- Tür 3, Bahçe ve oto: -14000.25 / -120.90 (zarar görüldü).
- Tür 3, Mevsimlik: -10507.79 / 129307.79 (zarar görüldü).
- Tür 3, Şarküteri: -25.17 / 416256.98 (zarar görüldü).
- Tür 3, Fırın ve pastane: -103.72 / 467154.77 (zarar görüldü).
- Tür 3, Balık: -48963.07 / 126518.89 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -23809.16 / 88390.57 (zarar görüldü).
- Tür 3, Ev ve mutfak: -309.60 / 168423.60 (zarar görüldü).
- Tür 3, Oyuncak: -7372.33 / -112.54 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -16895.45 / 83287.35 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 48. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 61.
- RejectBrandOffer: 181.
- SetDepartment: 114.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 108.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 108.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 6.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 15; borç sınırı ihlali 2 ay; şirket faizi 58.326,83 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 3, yabancı kapı kolu 2.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 15 deneme / 15 başarı.
- CorpLoan: 1 deneme / 1 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 8 deneme / 8 başarı.

#### C5: internet

Salgın 2540..3010. İl önerileri 67, onay 67, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 365; salgına uzaklık: -2175 gün.
- Telefon uygulaması ilk açılış: 1993; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1100; salgına uzaklık: -1440 gün.
- Hızlı teslimat ilk açılış: 2738; salgına uzaklık: 198 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.41 | 0.00 | 0 | -7.24 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.67 | 0.44 | 183 | 1280.11 | 0 | 0 | 0 | 0 | 0.995 |
| 3 | 0.94 | 0.28 | 216 | 1551.88 | 0 | 0 | 0 | 0 | 0.992 |
| 4 | 1.20 | 0.32 | 592 | -19785.66 | 0 | 0 | 0 | 0 | 0.989 |
| 5 | 1.47 | 0.35 | 2095 | -11415.31 | 0 | 0 | 0 | 0 | 0.987 |
| 6 | 1.98 | 0.35 | 5570 | 2092.52 | 0 | 0 | 0 | 0 | 0.983 |
| 7 | 8.51 | 0.35 | 10398 | 40239.89 | 0 | 18 | 780 | 284413 | 0.979 |
| 8 | 6.08 | 0.59 | 17089 | 200831.05 | 5 | 365 | 17089 | 6577650 | 0.757 |
| 9 | 7.70 | 0.34 | 16193 | 2953.19 | 21 | 87 | 4724 | 1929581 | 0.894 |
| 10 | 9.84 | 0.26 | 15068 | -194210.17 | 21 | 0 | 0 | 0 | 0.960 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 61 (onay 43 / ret 18), kapama 0 (onay 0 / ret 0). Stok eritme: 12 ürün işlemi, 12 ayrı gün. Reklam müdürü gideri 204421.61 TL (iç birim).

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
| 4 | tr | Gazete ve broşür | 12091.41 | 54876.29 |
| 4 | tr | Sosyal medya | 5353.64 | 51895.06 |
| 4 | tr | Arama ve uygulama reklamı | 3211.51 | 0.00 |
| 5 | tr | Televizyon | 69440.13 | 210956.77 |
| 5 | tr | Radyo | 63092.10 | 318212.19 |
| 5 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 5 | tr | Gazete ve broşür | 33779.54 | 134297.46 |
| 5 | tr | Sosyal medya | 21426.22 | 331321.03 |
| 5 | tr | Arama ve uygulama reklamı | 12854.53 | 0.00 |
| 6 | tr | Televizyon | 332994.64 | 1740371.95 |
| 6 | tr | Radyo | 187699.24 | 1143415.84 |
| 6 | tr | Açık hava (bilbord) | 0.00 | 0.00 |
| 6 | tr | Gazete ve broşür | 58591.14 | 132022.73 |
| 6 | tr | Sosyal medya | 37538.92 | 534424.83 |
| 6 | tr | Arama ve uygulama reklamı | 22522.86 | 0.00 |
| 7 | tr | Televizyon | 720865.42 | 4462631.11 |
| 7 | tr | Radyo | 212547.82 | 1552950.60 |
| 7 | tr | Açık hava (bilbord) | 114107.99 | 103928.00 |
| 7 | tr | Gazete ve broşür | 170552.13 | 200981.56 |
| 7 | tr | Sosyal medya | 42508.59 | 746795.49 |
| 7 | tr | Arama ve uygulama reklamı | 25503.77 | 0.00 |
| 8 | tr | Televizyon | 1143221.78 | 8680437.40 |
| 8 | tr | Radyo | 238169.31 | 2069682.76 |
| 8 | tr | Açık hava (bilbord) | 622765.51 | 879698.95 |
| 8 | tr | Gazete ve broşür | 343413.19 | 314649.47 |
| 8 | tr | Sosyal medya | 47632.41 | 1072425.04 |
| 8 | tr | Arama ve uygulama reklamı | 28579.14 | 0.00 |
| 9 | tr | Televizyon | 1300893.98 | 13338743.77 |
| 9 | tr | Radyo | 271017.99 | 2928001.58 |
| 9 | tr | Açık hava (bilbord) | 1390732.70 | 2590161.61 |
| 9 | tr | Gazete ve broşür | 592992.57 | 380474.09 |
| 9 | tr | Sosyal medya | 54202.32 | 1514984.25 |
| 9 | tr | Arama ve uygulama reklamı | 32520.86 | 0.00 |
| 10 | tr | Televizyon | 1611650.03 | 19342120.85 |
| 10 | tr | Radyo | 335758.86 | 4196227.53 |
| 10 | tr | Açık hava (bilbord) | 2095146.02 | 4829034.91 |
| 10 | tr | Gazete ve broşür | 1260671.00 | 550419.56 |
| 10 | tr | Sosyal medya | 67150.62 | 2170463.58 |
| 10 | tr | Arama ve uygulama reklamı | 40289.45 | 0.00 |
| 11 | tr | Televizyon | 334649.26 | 4187488.17 |
| 11 | tr | Radyo | 69718.17 | 907221.01 |
| 11 | tr | Açık hava (bilbord) | 435044.04 | 1064953.77 |
| 11 | tr | Gazete ve broşür | 357192.64 | 135694.29 |
| 11 | tr | Sosyal medya | 13943.31 | 469248.89 |
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
