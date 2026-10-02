# Otomatik oyuncunun denge raporu

3 koşu, her biri 3653 gün; toplam sure 779.7 saniye.

Tune: C10 defaults

| Tarz | Tohum | Son kasa | Borç | Magaza | İl | Ulusal pay | Kasa eksi gün | Sıkıntı günu | Denetim hatasi |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Atak | 21 | 15477152,28 TL | 429492,86 TL | 176 | 28 | 0.7369% | 0 | 0 | 0 |
| Atak | 22 | 16862460,77 TL | 304688,18 TL | 156 | 28 | 0.7615% | 0 | 0 | 0 |
| Atak | 23 | 17310132,93 TL | 454453,09 TL | 134 | 27 | 0.6916% | 0 | 0 | 0 |

## Bulgular ve neye bakmalı

- 0/3 koşuda kasa eksiye düştü.
- 3/3 koşu sonunda birden cok mağaza acik kaldi.
- Satış, siparis, stok ve sayi denetimi: 0 hata.


### Atak / tohum 21

- İlk şube: 187. gün.
- 5 mağaza: 296. gün.
- İlk depo: 295. gün.
- İlk il müdüru: 1800. gün.
- 5 il: 527. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 36784034,60 TL.
- Üst yönetim ücretleri: 9941204,84 TL.
- İşletme giderleri (ücret hariç): 8511201,42 TL.
- Depo ve merkez giderleri: 7737250,76 TL.
- Mal alımi (stok yatırımi): 3242016,66 TL.

Arka planin kasaya toplam net etkisi: 59606118,73 TL. Reddedilen komut: 438.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -7070746,54 TL.
- Depo ve merkez: -7737250,76 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -34713163,58 TL.
- Internet satışi: 115353,77 TL.
- Subeler: 139226433,97 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 5 | 1077286 / 2649343793 |
| 2 | 18 | 20 | 19 | 6214386 / 2742359338 |
| 3 | 10 | 20 | 22 | 15145277 / 2806522293 |
| 4 | 10 | 20 | 31 | 24988373 / 2851676281 |
| 5 | 9 | 20 | 45 | 41040354 / 2934839710 |
| 6 | 8 | 20 | 73 | 60658142 / 3037157017 |
| 7 | 8 | 20 | 101 | 84633705 / 3095215116 |
| 8 | 7 | 19 | 143 | 97116999 / 3168933777 |
| 9 | 7 | 19 | 152 | 88494314 / 3289308589 |
| 10 | 7 | 22 | 176 | 86974853 / 3360673294 |

Rakipler: en çok 81 etkin zincir; 3 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 2 satın almamız; 274 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8782 mağaza · senden çekildiği savaş: 52.
Markalardan toplam 1665002.38 TL; küsen farklı marka 8.

C3 kapanma nedenleri: 0 iflas/kapanma, 2 rakip tarafından alınma, 2 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 715, tamamlanan 269; kutlama 457. Ritim koruyucusu: 1 sakin dönem olayı, 13 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | 13043050.55 | 13558827.05 | 11658675.64 | -2724411.06 |
| durgünluk / 1 | 3280–3614 | 335 | 13578105.88 | 15029139.03 | 12857968.71 | 2516424.35 |
| büyük salgın / 1 | 3674–4145 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 1 | 4223–5075 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| toparlanma / 1 | 5135–5499 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 7753–7936 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7998–8543 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 195. gün 0 -> 1.
- Süt ürünleri: 315. gün 0 -> 1.
- Temizlik ve bakım: 345. gün 0 -> 1.
- İçecek: 375. gün 0 -> 1.
- Kuru gıda: 630. gün 1 -> 2.
- İçecek: 735. gün 1 -> 2.
- Süt ürünleri: 735. gün 1 -> 2.
- Temizlik ve bakım: 735. gün 1 -> 2.
- Kuru gıda: 1455. gün 2 -> 3.
- Süt ürünleri: 1845. gün 2 -> 3.
- İçecek: 1965. gün 2 -> 3.
- Temizlik ve bakım: 1995. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -40344.90 / -30.99 (zarar görüldü).
- Tür 2, Manav: -7.99 / 27339.61 (zarar görüldü).
- Tür 2, Kasap: -434.38 / 31604.01 (zarar görüldü).
- Tür 2, Evcil hayvan: 28.74 / 5520.89.
- Tür 2, Fırın ve pastane: -1013.57 / 13925.13 (zarar görüldü).
- Tür 2, Balık: -3329.29 / -191.66 (zarar görüldü).
- Tür 3, Manav: -121.76 / 496314.84 (zarar görüldü).
- Tür 3, Kasap: -137.24 / 818094.68 (zarar görüldü).
- Tür 3, Bebek: -52433.46 / -238.12 (zarar görüldü).
- Tür 3, Evcil hayvan: 71.23 / 136843.78.
- Tür 3, Bahçe ve oto: -59294.42 / -124.36 (zarar görüldü).
- Tür 3, Mevsimlik: -19634.10 / 150853.45 (zarar görüldü).
- Tür 3, Şarküteri: -267.19 / 295689.31 (zarar görüldü).
- Tür 3, Fırın ve pastane: -486.77 / 283138.25 (zarar görüldü).
- Tür 3, Balık: -136977.56 / 99681.38 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -30995.22 / 888721.24 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -59798.15 / 504950.31 (zarar görüldü).
- Tür 3, Ev ve mutfak: -67280.35 / -6370.47 (zarar görüldü).
- Tür 3, Oyuncak: -23122.34 / 275464.55 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -50346.18 / 51431.33 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 79. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 62.
- RejectBrandOffer: 180.
- ReplaceMasters: 3.
- SetDepartment: 146.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 147.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 147.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 13.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 45; borç sınırı ihlali 3 ay; şirket faizi 945.511,89 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 4, kabul 2, ret 2, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 1, yabancı kapı kolu 1.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- BidChainFinanced: 4 deneme / 4 başarı.
- CloseBranch: 45 deneme / 45 başarı.
- CorpLoan: 3 deneme / 3 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 17 deneme / 17 başarı.

#### C5: internet

Salgın 3676..4153. İl önerileri 30, onay 30, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 1506; salgına uzaklık: -2170 gün.
- Telefon uygulaması ilk açılış: 3129; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 2283; salgına uzaklık: -1393 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 4 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 5 | 0.65 | 0.05 | 2479 | -13829.55 | 0 | 0 | 0 | 0 | 0.995 |
| 6 | 0.91 | 0.05 | 3567 | -9829.45 | 0 | 0 | 0 | 0 | 0.992 |
| 7 | 1.17 | 0.04 | 5853 | -6235.62 | 0 | 0 | 0 | 0 | 0.990 |
| 8 | 1.44 | 0.05 | 8632 | 10725.73 | 0 | 0 | 0 | 0 | 0.987 |
| 9 | 1.90 | 0.08 | 11438 | 35543.01 | 0 | 0 | 0 | 0 | 0.984 |
| 10 | 2.41 | 0.14 | 12880 | 98979.65 | 0 | 0 | 0 | 0 | 0.978 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 119 (onay 70 / ret 49), kapama 0 (onay 0 / ret 0). Stok eritme: 17 ürün işlemi, 17 ayrı gün. Reklam müdürü gideri 269344.24 TL (iç birim).

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
| 3 | tr | Televizyon | 190708.29 | 566851.22 |
| 3 | tr | Radyo | 61859.14 | 282971.11 |
| 3 | tr | Açık hava (bilbord) | 35796.59 | 37324.32 |
| 3 | tr | Gazete ve broşür | 44975.24 | 154296.19 |
| 3 | tr | Sosyal medya | 0.00 | 0.00 |
| 3 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 4 | tr | Televizyon | 666868.37 | 2712179.79 |
| 4 | tr | Radyo | 155710.24 | 816768.35 |
| 4 | tr | Açık hava (bilbord) | 122003.15 | 235784.12 |
| 4 | tr | Gazete ve broşür | 109037.21 | 225270.19 |
| 4 | tr | Sosyal medya | 23682.98 | 90818.72 |
| 4 | tr | Arama ve uygulama reklamı | 0.00 | 0.00 |
| 5 | tr | Televizyon | 808954.32 | 4801014.42 |
| 5 | tr | Radyo | 168530.36 | 1093158.06 |
| 5 | tr | Açık hava (bilbord) | 476859.50 | 783862.33 |
| 5 | tr | Gazete ve broşür | 225571.42 | 364260.91 |
| 5 | tr | Sosyal medya | 33705.03 | 235190.11 |
| 5 | tr | Arama ve uygulama reklamı | 15387.70 | 0.00 |
| 6 | tr | Televizyon | 882115.05 | 7906096.55 |
| 6 | tr | Radyo | 183772.56 | 1724559.10 |
| 6 | tr | Açık hava (bilbord) | 817814.11 | 1944575.59 |
| 6 | tr | Gazete ve broşür | 385705.25 | 572264.72 |
| 6 | tr | Sosyal medya | 36752.86 | 459039.57 |
| 6 | tr | Arama ve uygulama reklamı | 22050.93 | 0.00 |
| 7 | tr | Televizyon | 967208.92 | 12113317.45 |
| 7 | tr | Radyo | 201500.39 | 2625671.66 |
| 7 | tr | Açık hava (bilbord) | 902728.66 | 3085238.14 |
| 7 | tr | Gazete ve broşür | 640760.69 | 853340.32 |
| 7 | tr | Sosyal medya | 40297.95 | 834090.59 |
| 7 | tr | Arama ve uygulama reklamı | 24178.66 | 0.00 |
| 8 | tr | Televizyon | 1073602.14 | 17879297.61 |
| 8 | tr | Radyo | 223665.36 | 3872041.47 |
| 8 | tr | Açık hava (bilbord) | 1002028.16 | 4562005.31 |
| 8 | tr | Gazete ve broşür | 1036305.10 | 1162961.85 |
| 8 | tr | Sosyal medya | 44731.32 | 1428862.83 |
| 8 | tr | Arama ve uygulama reklamı | 26837.81 | 0.00 |
| 9 | tr | Televizyon | 1211742.07 | 20652323.24 |
| 9 | tr | Radyo | 252444.51 | 4472028.29 |
| 9 | tr | Açık hava (bilbord) | 1130958.76 | 5269850.81 |
| 9 | tr | Gazete ve broşür | 1494040.04 | 1229971.93 |
| 9 | tr | Sosyal medya | 50488.06 | 1874868.55 |
| 9 | tr | Arama ve uygulama reklamı | 30292.52 | 0.00 |
| 10 | tr | Televizyon | 1397577.14 | 21309258.71 |
| 10 | tr | Radyo | 291160.59 | 4614225.49 |
| 10 | tr | Açık hava (bilbord) | 1304405.00 | 5437458.35 |
| 10 | tr | Gazete ve broşür | 1903210.86 | 1148365.12 |
| 10 | tr | Sosyal medya | 58230.72 | 2174006.89 |
| 10 | tr | Arama ve uygulama reklamı | 34937.45 | 0.00 |
| 11 | tr | Televizyon | 269106.01 | 3980435.33 |
| 11 | tr | Radyo | 56063.67 | 861908.29 |
| 11 | tr | Açık hava (bilbord) | 251165.52 | 1015682.98 |
| 11 | tr | Gazete ve broşür | 388674.30 | 201453.08 |
| 11 | tr | Sosyal medya | 11212.27 | 431992.84 |
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
- İlk il müdüru: 1457. gün.
- 5 il: 317. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 37568040,02 TL.
- Üst yönetim ücretleri: 11567206,52 TL.
- İşletme giderleri (ücret hariç): 9624921,94 TL.
- Depo ve merkez giderleri: 9409334,49 TL.
- Zararli şubeler (net zarar): 4074180,57 TL.

Arka planin kasaya toplam net etkisi: 64481265,21 TL. Reddedilen komut: 572.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -8220154,39 TL.
- Depo ve merkez: -9409334,49 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -39047661,51 TL.
- Internet satışi: 68182,85 TL.
- Subeler: 134433280,79 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 26 | 20 | 9 | 2690079 / 2585369432 |
| 2 | 10 | 20 | 19 | 11451228 / 2664411879 |
| 3 | 10 | 20 | 26 | 21229005 / 2739396267 |
| 4 | 9 | 20 | 41 | 36219413 / 2818767366 |
| 5 | 8 | 20 | 71 | 57739650 / 2940529417 |
| 6 | 8 | 20 | 113 | 79711788 / 3007185519 |
| 7 | 8 | 20 | 130 | 71293570 / 3127550970 |
| 8 | 8 | 24 | 157 | 80657823 / 3215860338 |
| 9 | 8 | 29 | 142 | 76887531 / 3285197661 |
| 10 | 7 | 28 | 156 | 89696112 / 3425491994 |

Rakipler: en çok 96 etkin zincir; 11 farklı satılık zincir; 9 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 1 satın almamız; 336 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8579 mağaza · senden çekildiği savaş: 66.
Markalardan toplam 2081092.47 TL; küsen farklı marka 7.

C3 kapanma nedenleri: 0 iflas/kapanma, 8 rakip tarafından alınma, 1 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 704, tamamlanan 244; kutlama 447. Ritim koruyucusu: 1 sakin dönem olayı, 18 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 2306–2480 | 175 | 8686923.69 | 9279975.51 | 8305005.89 | -518921.02 |
| durgünluk / 1 | 2481–2814 | 334 | 9307021.92 | 10563638.61 | 9040309.30 | 634285.35 |
| büyük salgın / 1 | 2874–3346 | 473 | 11237093.91 | 12774403.70 | 11113031.38 | 2976053.30 |
| yüksek enflasyon / 1 | 3424–4275 | 230 | 13788939.62 | 16862460.77 | 13788939.62 | 4201636.56 |
| toparlanma / 1 | 4335–4699 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6953–7136 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7198–7744 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 195. gün 0 -> 1.
- Süt ürünleri: 315. gün 0 -> 1.
- İçecek: 330. gün 0 -> 1.
- Temizlik ve bakım: 330. gün 0 -> 1.
- Kuru gıda: 510. gün 1 -> 2.
- İçecek: 555. gün 1 -> 2.
- Süt ürünleri: 555. gün 1 -> 2.
- Temizlik ve bakım: 555. gün 1 -> 2.
- Kuru gıda: 1215. gün 2 -> 3.
- Süt ürünleri: 1575. gün 2 -> 3.
- İçecek: 1635. gün 2 -> 3.
- Temizlik ve bakım: 1635. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -29998.34 / -35.04 (zarar görüldü).
- Tür 2, Manav: -150.28 / 29671.67 (zarar görüldü).
- Tür 2, Kasap: -268.52 / 36115.87 (zarar görüldü).
- Tür 2, Evcil hayvan: 19.26 / 6171.98.
- Tür 2, Fırın ve pastane: -588.49 / 16636.70 (zarar görüldü).
- Tür 2, Balık: -1524.26 / -80.64 (zarar görüldü).
- Tür 3, Manav: -9.40 / 559722.70 (zarar görüldü).
- Tür 3, Kasap: -96.55 / 985484.54 (zarar görüldü).
- Tür 3, Bebek: -75373.73 / -203.54 (zarar görüldü).
- Tür 3, Evcil hayvan: 55.51 / 167786.67.
- Tür 3, Bahçe ve oto: -16326.56 / -212.70 (zarar görüldü).
- Tür 3, Mevsimlik: -18342.23 / 129149.45 (zarar görüldü).
- Tür 3, Şarküteri: -340.94 / 364433.45 (zarar görüldü).
- Tür 3, Fırın ve pastane: -718.09 / 340857.32 (zarar görüldü).
- Tür 3, Balık: -75187.45 / 68033.26 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -9742.23 / 563609.00 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -17910.19 / 333386.39 (zarar görüldü).
- Tür 3, Ev ve mutfak: -36105.49 / -2831.89 (zarar görüldü).
- Tür 3, Oyuncak: -19435.16 / 1680.59 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -39846.55 / 39296.16 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 83. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 62.
- RejectBrandOffer: 180.
- ReplaceMasters: 3.
- SetDepartment: 141.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 115.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 115.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 14.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 73; borç sınırı ihlali 4 ay; şirket faizi 726.228,73 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 1, kabul 1, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 6, yabancı kapı kolu 4.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- BidChainFinanced: 1 deneme / 1 başarı.
- CloseBranch: 73 deneme / 73 başarı.
- CorpLoan: 3 deneme / 3 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 17 deneme / 17 başarı.

#### C5: internet

Salgın 2887..3324. İl önerileri 72, onay 72, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 708; salgına uzaklık: -2179 gün.
- Telefon uygulaması ilk açılış: 2340; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1429; salgına uzaklık: -1458 gün.
- Hızlı teslimat ilk açılış: 3067; salgına uzaklık: 180 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.42 | 0.00 | 28 | -1821.74 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.69 | 0.08 | 1749 | -18807.24 | 0 | 0 | 0 | 0 | 0.994 |
| 4 | 0.95 | 0.05 | 2360 | -18270.50 | 0 | 0 | 0 | 0 | 0.992 |
| 5 | 1.22 | 0.04 | 5198 | -12805.32 | 0 | 0 | 0 | 0 | 0.989 |
| 6 | 1.48 | 0.06 | 6259 | 5411.76 | 0 | 0 | 0 | 0 | 0.987 |
| 7 | 2.01 | 0.09 | 10279 | 19915.92 | 0 | 0 | 0 | 0 | 0.983 |
| 8 | 8.51 | 0.16 | 12651 | 83521.88 | 0 | 36 | 1808 | 1103108 | 0.967 |
| 9 | 6.08 | 0.40 | 23384 | 159497.15 | 15 | 366 | 23384 | 10729117 | 0.756 |
| 10 | 8.00 | 0.23 | 18491 | -148459.06 | 15 | 35 | 2881 | 910928 | 0.927 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 81 (onay 53 / ret 28), kapama 1 (onay 0 / ret 1). Stok eritme: 16 ürün işlemi, 16 ayrı gün. Reklam müdürü gideri 266886.08 TL (iç birim).

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
| 3 | tr | Televizyon | 307880.65 | 1036323.87 |
| 3 | tr | Radyo | 86003.73 | 456506.92 |
| 3 | tr | Açık hava (bilbord) | 70487.49 | 111306.66 |
| 3 | tr | Gazete ve broşür | 70156.29 | 228621.91 |
| 3 | tr | Sosyal medya | 17199.71 | 140513.57 |
| 3 | tr | Arama ve uygulama reklamı | 10319.58 | 0.00 |
| 4 | tr | Televizyon | 747415.68 | 3729939.47 |
| 4 | tr | Radyo | 155710.24 | 949093.12 |
| 4 | tr | Açık hava (bilbord) | 316312.19 | 475550.93 |
| 4 | tr | Gazete ve broşür | 183800.99 | 326834.34 |
| 4 | tr | Sosyal medya | 31140.41 | 268402.97 |
| 4 | tr | Arama ve uygulama reklamı | 18684.05 | 0.00 |
| 5 | tr | Televizyon | 808954.32 | 6499310.37 |
| 5 | tr | Radyo | 168530.36 | 1442504.63 |
| 5 | tr | Açık hava (bilbord) | 742736.42 | 1568723.89 |
| 5 | tr | Gazete ve broşür | 318275.38 | 464689.87 |
| 5 | tr | Sosyal medya | 33705.03 | 471198.32 |
| 5 | tr | Arama ve uygulama reklamı | 20221.76 | 0.00 |
| 6 | tr | Televizyon | 882115.05 | 10658226.98 |
| 6 | tr | Radyo | 183772.56 | 2316167.82 |
| 6 | tr | Açık hava (bilbord) | 823306.70 | 2715190.98 |
| 6 | tr | Gazete ve broşür | 632061.21 | 685648.83 |
| 6 | tr | Sosyal medya | 36752.86 | 874685.26 |
| 6 | tr | Arama ve uygulama reklamı | 22050.93 | 0.00 |
| 7 | tr | Televizyon | 987512.75 | 13942731.57 |
| 7 | tr | Radyo | 205730.55 | 3020668.84 |
| 7 | tr | Açık hava (bilbord) | 921678.12 | 3558288.70 |
| 7 | tr | Gazete ve broşür | 1006813.13 | 817612.39 |
| 7 | tr | Sosyal medya | 41144.28 | 1292557.95 |
| 7 | tr | Arama ve uygulama reklamı | 24685.78 | 0.00 |
| 8 | tr | Televizyon | 1131427.76 | 14946969.89 |
| 8 | tr | Radyo | 235712.67 | 3236761.46 |
| 8 | tr | Açık hava (bilbord) | 1055998.75 | 3814161.76 |
| 8 | tr | Gazete ve broşür | 1350385.19 | 791205.67 |
| 8 | tr | Sosyal medya | 47140.78 | 1553475.10 |
| 8 | tr | Arama ve uygulama reklamı | 28283.68 | 0.00 |
| 9 | tr | Televizyon | 1262445.17 | 17902261.61 |
| 9 | tr | Radyo | 263008.30 | 3876495.35 |
| 9 | tr | Açık hava (bilbord) | 1178281.76 | 4568097.66 |
| 9 | tr | Gazete ve broşür | 1633797.56 | 847528.97 |
| 9 | tr | Sosyal medya | 52599.97 | 1997081.36 |
| 9 | tr | Arama ve uygulama reklamı | 31559.18 | 0.00 |
| 10 | tr | Televizyon | 1451290.04 | 21139693.92 |
| 10 | tr | Radyo | 302350.73 | 4577505.96 |
| 10 | tr | Açık hava (bilbord) | 1354536.34 | 5394174.89 |
| 10 | tr | Gazete ve broşür | 1765562.37 | 878694.50 |
| 10 | tr | Sosyal medya | 60468.42 | 2367650.78 |
| 10 | tr | Arama ve uygulama reklamı | 36280.69 | 0.00 |
| 11 | tr | Televizyon | 288120.63 | 4519058.11 |
| 11 | tr | Radyo | 60024.99 | 978538.99 |
| 11 | tr | Açık hava (bilbord) | 268912.53 | 1153119.29 |
| 11 | tr | Gazete ve broşür | 375986.70 | 173568.81 |
| 11 | tr | Sosyal medya | 12004.58 | 506138.87 |
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
- İlk il müdüru: 1555. gün.
- 5 il: 436. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 37072850,69 TL.
- Üst yönetim ücretleri: 12180141,31 TL.
- Depo ve merkez giderleri: 8348362,24 TL.
- İşletme giderleri (ücret hariç): 7046892,19 TL.
- Zararli şubeler (net zarar): 3130106,31 TL.

Arka planin kasaya toplam net etkisi: 58955634,06 TL. Reddedilen komut: 445.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -5659437,27 TL.
- Depo ve merkez: -8348362,24 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -39191874,42 TL.
- Internet satışi: -385005,48 TL.
- Subeler: 131762108,64 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 5 | 1189205 / 2616019729 |
| 2 | 10 | 20 | 19 | 13107819 / 2664104721 |
| 3 | 10 | 20 | 26 | 20802634 / 2766262382 |
| 4 | 9 | 20 | 38 | 34713469 / 2881109386 |
| 5 | 8 | 20 | 57 | 55407668 / 2996229747 |
| 6 | 8 | 20 | 90 | 65159516 / 3089990654 |
| 7 | 8 | 20 | 112 | 74824131 / 3158913001 |
| 8 | 8 | 26 | 121 | 71185389 / 3214436872 |
| 9 | 8 | 27 | 129 | 78181342 / 3297649008 |
| 10 | 7 | 26 | 134 | 81713088 / 3417996779 |

Rakipler: en çok 89 etkin zincir; 8 farklı satılık zincir; 7 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 298 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8582 mağaza · senden çekildiği savaş: 58.
Markalardan toplam 1721471.20 TL; küsen farklı marka 8.

C3 kapanma nedenleri: 0 iflas/kapanma, 7 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 703, tamamlanan 238; kutlama 446. Ritim koruyucusu: 1 sakin dönem olayı, 15 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 1967–2141 | 175 | 5859988.47 | 6496757.16 | 5791306.94 | 1316614.09 |
| durgünluk / 1 | 2142–2475 | 334 | 6534358.20 | 8263829.40 | 6496491.08 | 4257972.18 |
| büyük salgın / 1 | 2535–3006 | 472 | 8795893.17 | 9757035.23 | 8400384.09 | 2894592.03 |
| yüksek enflasyon / 1 | 3084–3936 | 570 | 11159766.00 | 17310132.93 | 11159766.00 | 9375274.04 |
| toparlanma / 1 | 3996–4360 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6614–6797 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 6859–7404 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 195. gün 0 -> 1.
- Süt ürünleri: 315. gün 0 -> 1.
- İçecek: 375. gün 0 -> 1.
- Temizlik ve bakım: 375. gün 0 -> 1.
- Kuru gıda: 480. gün 1 -> 2.
- Süt ürünleri: 645. gün 1 -> 2.
- Temizlik ve bakım: 660. gün 1 -> 2.
- İçecek: 675. gün 1 -> 2.
- Kuru gıda: 1290. gün 2 -> 3.
- Süt ürünleri: 1650. gün 2 -> 3.
- Temizlik ve bakım: 1785. gün 2 -> 3.
- İçecek: 1815. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -21245.50 / -34.33 (zarar görüldü).
- Tür 2, Manav: -279.97 / 30738.31 (zarar görüldü).
- Tür 2, Kasap: -660.16 / 39178.16 (zarar görüldü).
- Tür 2, Bebek: -1884.39 / -69.57 (zarar görüldü).
- Tür 2, Evcil hayvan: 20.55 / 6578.20.
- Tür 2, Şarküteri: -516.96 / 16323.27 (zarar görüldü).
- Tür 2, Fırın ve pastane: -613.11 / -32.50 (zarar görüldü).
- Tür 2, Balık: -3407.74 / -109.69 (zarar görüldü).
- Tür 3, Manav: -66.84 / 569946.78 (zarar görüldü).
- Tür 3, Kasap: 17.96 / 1046815.28.
- Tür 3, Bebek: -36014.35 / -207.44 (zarar görüldü).
- Tür 3, Evcil hayvan: 78.50 / 170777.04.
- Tür 3, Bahçe ve oto: -16475.57 / -1408.75 (zarar görüldü).
- Tür 3, Mevsimlik: -16104.68 / 113752.44 (zarar görüldü).
- Tür 3, Şarküteri: -201.08 / 382489.97 (zarar görüldü).
- Tür 3, Fırın ve pastane: -676.15 / 385447.44 (zarar görüldü).
- Tür 3, Balık: -15926.77 / 127786.98 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -35034.30 / 347720.17 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -3507.74 / 281261.86 (zarar görüldü).
- Tür 3, Ev ve mutfak: -2014.69 / 227224.39 (zarar görüldü).
- Tür 3, Oyuncak: -11078.68 / 93749.62 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -26436.62 / 70686.01 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 78. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 63.
- RejectBrandOffer: 179.
- ReplaceMasters: 2.
- SetDepartment: 116.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 160.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 160.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 12.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 33; borç sınırı ihlali 3 ay; şirket faizi 1.065.435,62 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 2, kabul 0, ret 2, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 3, yabancı kapı kolu 2.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- BidChainFinanced: 2 deneme / 2 başarı.
- CloseBranch: 33 deneme / 33 başarı.
- CorpLoan: 3 deneme / 3 başarı.
- LineAuto: 1 deneme / 1 başarı.
- OpenLine: 1 deneme / 1 başarı.
- Restructure: 18 deneme / 18 başarı.

#### C5: internet

Salgın 2540..3010. İl önerileri 101, onay 101, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 421; salgına uzaklık: -2119 gün.
- Telefon uygulaması ilk açılış: 1993; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1163; salgına uzaklık: -1377 gün.
- Hızlı teslimat ilk açılış: 2724; salgına uzaklık: 184 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.41 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.67 | 0.19 | 619 | -11526.52 | 0 | 0 | 0 | 0 | 0.995 |
| 3 | 0.94 | 0.09 | 1465 | -15548.02 | 0 | 0 | 0 | 0 | 0.992 |
| 4 | 1.20 | 0.07 | 3582 | -12396.55 | 0 | 0 | 0 | 0 | 0.989 |
| 5 | 1.47 | 0.05 | 6144 | -7055.59 | 0 | 0 | 0 | 0 | 0.987 |
| 6 | 1.98 | 0.08 | 7035 | 7812.13 | 0 | 0 | 0 | 0 | 0.983 |
| 7 | 8.51 | 0.13 | 10461 | 43584.41 | 0 | 18 | 755 | 481078 | 0.979 |
| 8 | 6.08 | 0.34 | 18250 | 57857.90 | 17 | 365 | 18250 | 9117753 | 0.757 |
| 9 | 7.70 | 0.26 | 17044 | -191999.29 | 17 | 87 | 5197 | 2115819 | 0.894 |
| 10 | 9.84 | 0.22 | 15643 | -255733.95 | 17 | 0 | 0 | 0 | 0.960 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 78 (onay 36 / ret 42), kapama 1 (onay 0 / ret 1). Stok eritme: 14 ürün işlemi, 13 ayrı gün. Reklam müdürü gideri 238346.40 TL (iç birim).

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
| 3 | tr | Televizyon | 249555.59 | 810546.50 |
| 3 | tr | Radyo | 74165.18 | 380890.62 |
| 3 | tr | Açık hava (bilbord) | 75629.36 | 133199.37 |
| 3 | tr | Gazete ve broşür | 57091.58 | 186793.01 |
| 3 | tr | Sosyal medya | 14832.24 | 147222.02 |
| 3 | tr | Arama ve uygulama reklamı | 8899.10 | 0.00 |
| 4 | tr | Televizyon | 747415.68 | 3536024.23 |
| 4 | tr | Radyo | 155710.24 | 927052.84 |
| 4 | tr | Açık hava (bilbord) | 245086.58 | 417224.18 |
| 4 | tr | Gazete ve broşür | 165627.46 | 305382.97 |
| 4 | tr | Sosyal medya | 31140.41 | 311478.54 |
| 4 | tr | Arama ve uygulama reklamı | 18684.05 | 0.00 |
| 5 | tr | Televizyon | 808954.32 | 6277325.30 |
| 5 | tr | Radyo | 168530.36 | 1398663.84 |
| 5 | tr | Açık hava (bilbord) | 710664.47 | 1488072.59 |
| 5 | tr | Gazete ve broşür | 277120.15 | 416438.84 |
| 5 | tr | Sosyal medya | 33705.03 | 526139.99 |
| 5 | tr | Arama ve uygulama reklamı | 20221.76 | 0.00 |
| 6 | tr | Televizyon | 900962.15 | 9942997.29 |
| 6 | tr | Radyo | 187699.24 | 2162183.70 |
| 6 | tr | Açık hava (bilbord) | 810865.78 | 2530744.57 |
| 6 | tr | Gazete ve broşür | 524457.79 | 587159.16 |
| 6 | tr | Sosyal medya | 37538.92 | 921531.72 |
| 6 | tr | Arama ve uygulama reklamı | 22522.86 | 0.00 |
| 7 | tr | Televizyon | 1020235.42 | 12142819.71 |
| 7 | tr | Radyo | 212547.82 | 2630917.10 |
| 7 | tr | Açık hava (bilbord) | 918211.27 | 3098904.94 |
| 7 | tr | Gazete ve broşür | 819569.13 | 646196.54 |
| 7 | tr | Sosyal medya | 42508.59 | 1256573.13 |
| 7 | tr | Arama ve uygulama reklamı | 25503.77 | 0.00 |
| 8 | tr | Televizyon | 1143221.78 | 14734638.83 |
| 8 | tr | Radyo | 238169.31 | 3190810.74 |
| 8 | tr | Açık hava (bilbord) | 1028900.06 | 3760007.97 |
| 8 | tr | Gazete ve broşür | 1155316.88 | 701516.42 |
| 8 | tr | Sosyal medya | 47632.41 | 1641818.82 |
| 8 | tr | Arama ve uygulama reklamı | 28579.14 | 0.00 |
| 9 | tr | Televizyon | 1300893.98 | 16973545.51 |
| 9 | tr | Radyo | 271017.99 | 3675379.07 |
| 9 | tr | Açık hava (bilbord) | 1170803.60 | 4331119.89 |
| 9 | tr | Gazete ve broşür | 1329094.53 | 710937.31 |
| 9 | tr | Sosyal medya | 54202.32 | 1901028.50 |
| 9 | tr | Arama ve uygulama reklamı | 32520.86 | 0.00 |
| 10 | tr | Televizyon | 1611650.03 | 22802359.71 |
| 10 | tr | Radyo | 335758.86 | 4937493.55 |
| 10 | tr | Açık hava (bilbord) | 1450485.21 | 5818413.95 |
| 10 | tr | Gazete ve broşür | 1737666.21 | 825632.90 |
| 10 | tr | Sosyal medya | 67150.62 | 2553859.67 |
| 10 | tr | Arama ve uygulama reklamı | 40289.45 | 0.00 |
| 11 | tr | Televizyon | 334649.26 | 4751492.42 |
| 11 | tr | Radyo | 69718.17 | 1028861.20 |
| 11 | tr | Açık hava (bilbord) | 301184.15 | 1212424.92 |
| 11 | tr | Gazete ve broşür | 370849.32 | 157027.54 |
| 11 | tr | Sosyal medya | 13943.31 | 532166.17 |
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
