# A6 — 10 yıl, üç tarz, üç tohum

Kaynak d190bfe; TR/Kırklareli; tohum 21–23. Son rapor: Saved/AutoPlay/20261001-090707. Yorum ve öneriler A6_30_yil_rapor.md'de. İlk, hatalı yer seçimi koşusu kullanılmadı. Bu son koşu 9 × 3.653 gün; denetim hatası 0, işlem çıkışı 0.

# Otomatik oyuncunun denge raporu

9 koşu, her biri 3653 gün; toplam sure 279.6 saniye.

| Tarz | Tohum | Son kasa | Borç | Magaza | İl | Ulusal pay | Kasa eksi gün | Sıkıntı günu | Denetim hatasi |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Temkinli | 21 | 58815956,92 TL | 0,00 TL | 107 | 48 | 4.8703% | 0 | 0 | 0 |
| Temkinli | 22 | 67072087,15 TL | 0,00 TL | 108 | 48 | 4.9158% | 0 | 0 | 0 |
| Temkinli | 23 | 59046036,39 TL | 0,00 TL | 108 | 48 | 4.9158% | 0 | 0 | 0 |
| Dengeli | 21 | -195977687,17 TL | 868952,75 TL | 151 | 40 | 6.8730% | 1013 | 1013 | 0 |
| Dengeli | 22 | -197527728,97 TL | 543178,75 TL | 141 | 40 | 6.4178% | 1119 | 1119 | 0 |
| Dengeli | 23 | -51758604,21 TL | 1265312,37 TL | 31 | 30 | 1.4110% | 2782 | 2782 | 0 |
| Atak | 21 | -23866651,26 TL | 696459,46 TL | 15 | 13 | 0.6827% | 3219 | 3219 | 0 |
| Atak | 22 | -3613487,04 TL | 45699,26 TL | 5 | 3 | 0.2276% | 3467 | 3467 | 0 |
| Atak | 23 | -40897903,92 TL | 1138685,27 TL | 22 | 20 | 1.0014% | 3016 | 3016 | 0 |

## Bulgular ve neye bakmalı

- 6/9 koşuda kasa eksiye düştü.
- 9/9 koşu birden cok mağazaya ulaştı.
- Satış, siparis, stok ve sayi denetimi: 0 hata.


### Temkinli / tohum 21

- İlk şube: 399. gün.
- 5 mağaza: 489. gün.
- İlk depo: 721. gün.
- İlk il müdüru: 2444. gün.
- 5 il: 519. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 32534370,74 TL.
- Üst yönetim ücretleri: 8993595,22 TL.
- İşletme giderleri (ücret hariç): 7252874,94 TL.
- Depo ve merkez giderleri: 5105360,93 TL.
- Mal alımi (stok yatırımi): 1815403,46 TL.

Arka planin kasaya toplam net etkisi: 100692436,75 TL. Reddedilen komut: 203.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -6265858,16 TL.
- Depo ve merkez: -5105360,93 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -8994452,22 TL.
- Internet satışi: 0,00 TL.
- Subeler: 120251983,10 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 1 | 168820 / 2649343793 |
| 2 | 25 | 20 | 12 | 2293671 / 2742359338 |
| 3 | 18 | 20 | 23 | 6287650 / 2806522293 |
| 4 | 11 | 20 | 35 | 11150641 / 2851676281 |
| 5 | 10 | 20 | 47 | 22752798 / 2934839710 |
| 6 | 9 | 20 | 59 | 32930348 / 3037157017 |
| 7 | 9 | 20 | 71 | 42356768 / 3095215116 |
| 8 | 8 | 20 | 84 | 51421963 / 3168933777 |
| 9 | 8 | 20 | 96 | 61726876 / 3289308589 |
| 10 | 8 | 20 | 107 | 68813522 / 3360673294 |

Rakipler: en çok 105 etkin zincir; 4 farklı satılık zincir; 2 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 178 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8962 mağaza · senden çekildiği savaş: 43.
Markalardan toplam 1780281.33 TL; küsen farklı marka 7.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 545. gün 0 -> 1.
- Süt ürünleri: 665. gün 0 -> 1.
- Temizlik ve bakım: 725. gün 0 -> 1.
- İçecek: 785. gün 0 -> 1.
- Kuru gıda: 965. gün 1 -> 2.
- İçecek: 1205. gün 1 -> 2.
- Süt ürünleri: 1265. gün 1 -> 2.
- Temizlik ve bakım: 1325. gün 1 -> 2.
- Kuru gıda: 1445. gün 2 -> 3.
- Süt ürünleri: 2465. gün 2 -> 3.
- Temizlik ve bakım: 2645. gün 2 -> 3.
- İçecek: 2705. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -226.31 / 6580.13 (zarar görüldü).
- Tür 2, Manav: -10.38 / 71544.11 (zarar görüldü).
- Tür 2, Kasap: -77.17 / 81315.67 (zarar görüldü).
- Tür 2, Evcil hayvan: 19.47 / 11978.53.
- Tür 2, Balık: -4558.60 / -136.23 (zarar görüldü).
- Tür 3, Manav: 92.87 / 659305.84.
- Tür 3, Kasap: 81.58 / 915355.35.
- Tür 3, Bebek: -2461.52 / 48239.32 (zarar görüldü).
- Tür 3, Mevsimlik: -3907.16 / 153723.99 (zarar görüldü).
- Tür 3, Şarküteri: 52.18 / 401654.56.
- Tür 3, Fırın ve pastane: -18.83 / 439837.28 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -2397.03 / -369.26 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -914.44 / 487990.29 (zarar görüldü).
- Tür 3, Ev ve mutfak: -1097.35 / 171735.02 (zarar görüldü).
- Tür 3, Oyuncak: -3017.34 / 300721.92 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -5677.22 / 60753.16 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 14 / 49. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 75.
- RejectBrandOffer: 283.
- ReplaceMasters: 37.
- SetDepartment: 19.
- SetDeptStance: 13.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 106.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 106.
- Önce yerel pay %35.: 11.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Temkinli / tohum 22

- İlk şube: 399. gün.
- 5 mağaza: 489. gün.
- İlk depo: 721. gün.
- İlk il müdüru: 2444. gün.
- 5 il: 519. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 34713563,80 TL.
- Üst yönetim ücretleri: 8851348,24 TL.
- İşletme giderleri (ücret hariç): 7462406,53 TL.
- Depo ve merkez giderleri: 5105360,93 TL.
- Mal alımi (stok yatırımi): 1812575,46 TL.

Arka planin kasaya toplam net etkisi: 111579310,67 TL. Reddedilen komut: 203.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -6490078,25 TL.
- Depo ve merkez: -5105360,93 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -8852549,94 TL.
- Internet satışi: 0,00 TL.
- Subeler: 131771169,40 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 28 | 20 | 1 | 172720 / 2585369432 |
| 2 | 27 | 20 | 12 | 2319797 / 2664411879 |
| 3 | 16 | 20 | 23 | 6521950 / 2739396267 |
| 4 | 10 | 20 | 35 | 11741116 / 2818767366 |
| 5 | 10 | 20 | 47 | 24358563 / 2940529417 |
| 6 | 9 | 20 | 59 | 35472651 / 3007185519 |
| 7 | 9 | 20 | 71 | 45705168 / 3127550970 |
| 8 | 8 | 20 | 84 | 54322639 / 3215860338 |
| 9 | 8 | 20 | 96 | 63387026 / 3285197661 |
| 10 | 8 | 20 | 108 | 71272252 / 3425491994 |

Rakipler: en çok 104 etkin zincir; 1 farklı satılık zincir; 1 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 176 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8996 mağaza · senden çekildiği savaş: 47.
Markalardan toplam 1610820.51 TL; küsen farklı marka 8.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 545. gün 0 -> 1.
- Süt ürünleri: 665. gün 0 -> 1.
- İçecek: 725. gün 0 -> 1.
- Temizlik ve bakım: 725. gün 0 -> 1.
- Kuru gıda: 965. gün 1 -> 2.
- İçecek: 1205. gün 1 -> 2.
- Süt ürünleri: 1205. gün 1 -> 2.
- Temizlik ve bakım: 1325. gün 1 -> 2.
- Kuru gıda: 1445. gün 2 -> 3.
- Süt ürünleri: 2405. gün 2 -> 3.
- Temizlik ve bakım: 2585. gün 2 -> 3.
- İçecek: 2645. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -344.21 / 6788.29 (zarar görüldü).
- Tür 2, Manav: -26.16 / 73655.60 (zarar görüldü).
- Tür 2, Kasap: -111.61 / 83817.28 (zarar görüldü).
- Tür 2, Evcil hayvan: 20.84 / 12297.21.
- Tür 2, Balık: -4541.63 / -142.36 (zarar görüldü).
- Tür 3, Manav: 190.13 / 665926.72.
- Tür 3, Kasap: 161.56 / 926364.06.
- Tür 3, Bebek: -942.70 / 49841.22 (zarar görüldü).
- Tür 3, Mevsimlik: 155.69 / 160054.15.
- Tür 3, Şarküteri: 74.29 / 412084.93.
- Tür 3, Fırın ve pastane: -22.36 / 450234.60 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -7510.32 / 96783.40 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -648.82 / 484262.59 (zarar görüldü).
- Tür 3, Ev ve mutfak: -896.70 / 179649.65 (zarar görüldü).
- Tür 3, Oyuncak: -5747.10 / 310618.63 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -3330.02 / -270.75 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 14 / 40. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 68.
- RejectBrandOffer: 295.
- ReplaceMasters: 41.
- SetDepartment: 19.
- SetDeptStance: 13.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 102.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 102.
- Önce yerel pay %35.: 11.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Temkinli / tohum 23

- İlk şube: 399. gün.
- 5 mağaza: 489. gün.
- İlk depo: 751. gün.
- İlk il müdüru: 2444. gün.
- 5 il: 519. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 32617942,08 TL.
- Üst yönetim ücretleri: 8717882,35 TL.
- İşletme giderleri (ücret hariç): 7378462,07 TL.
- Depo ve merkez giderleri: 5094979,67 TL.
- Mal alımi (stok yatırımi): 1837392,80 TL.

Arka planin kasaya toplam net etkisi: 101201865,06 TL. Reddedilen komut: 203.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -6389750,59 TL.
- Depo ve merkez: -5094979,67 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -8718681,38 TL.
- Internet satışi: 0,00 TL.
- Subeler: 120578401,24 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 1 | 161675 / 2616019729 |
| 2 | 27 | 20 | 12 | 1969755 / 2664104721 |
| 3 | 16 | 20 | 23 | 6318382 / 2766262382 |
| 4 | 12 | 20 | 35 | 11041019 / 2881109386 |
| 5 | 10 | 20 | 47 | 22690832 / 2996229747 |
| 6 | 9 | 20 | 59 | 32965948 / 3089990654 |
| 7 | 9 | 20 | 71 | 42907657 / 3158913001 |
| 8 | 8 | 20 | 84 | 52414782 / 3214436872 |
| 9 | 8 | 20 | 96 | 61548724 / 3297649008 |
| 10 | 8 | 20 | 108 | 69637749 / 3417996779 |

Rakipler: en çok 114 etkin zincir; 0 farklı satılık zincir; 0 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 178 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8951 mağaza · senden çekildiği savaş: 45.
Markalardan toplam 1812274.02 TL; küsen farklı marka 8.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 605. gün 0 -> 1.
- Süt ürünleri: 725. gün 0 -> 1.
- Temizlik ve bakım: 725. gün 0 -> 1.
- İçecek: 785. gün 0 -> 1.
- Kuru gıda: 965. gün 1 -> 2.
- İçecek: 1205. gün 1 -> 2.
- Süt ürünleri: 1265. gün 1 -> 2.
- Temizlik ve bakım: 1325. gün 1 -> 2.
- Kuru gıda: 1445. gün 2 -> 3.
- Süt ürünleri: 2465. gün 2 -> 3.
- Temizlik ve bakım: 2645. gün 2 -> 3.
- İçecek: 2705. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -374.56 / 6313.03 (zarar görüldü).
- Tür 2, Manav: -12.59 / 72357.09 (zarar görüldü).
- Tür 2, Kasap: -345.73 / 81370.23 (zarar görüldü).
- Tür 2, Evcil hayvan: 20.55 / 12222.94.
- Tür 2, Balık: -4663.78 / -143.89 (zarar görüldü).
- Tür 3, Manav: 77.11 / 661818.19.
- Tür 3, Kasap: 85.66 / 920560.93.
- Tür 3, Bebek: -2357.24 / 48354.73 (zarar görüldü).
- Tür 3, Mevsimlik: -1870.29 / 156401.15 (zarar görüldü).
- Tür 3, Şarküteri: 22.43 / 405763.60.
- Tür 3, Fırın ve pastane: -155.15 / 443678.85 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -2604.42 / -428.56 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -1367.90 / 488783.55 (zarar görüldü).
- Tür 3, Ev ve mutfak: -1336.59 / 174415.91 (zarar görüldü).
- Tür 3, Oyuncak: -2934.49 / 304816.44 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -5551.56 / 63524.29 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 16 / 54. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 70.
- RejectBrandOffer: 289.
- ReplaceMasters: 42.
- SetDepartment: 19.
- SetDeptStance: 13.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 106.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 106.
- Önce yerel pay %35.: 11.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Dengeli / tohum 21

- İlk şube: 107. gün.
- 5 mağaza: 233. gün.
- İlk depo: 449. gün.
- İlk il müdüru: 1373. gün.
- 5 il: 261. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 186060430,45 TL.
- Vergi ödemeleri: 22078676,15 TL.
- İşletme giderleri (ücret hariç): 16231126,42 TL.
- Üst yönetim ücretleri: 16138599,03 TL.
- Depo ve merkez giderleri: 9166015,50 TL.

Arka planin kasaya toplam net etkisi: -141025467,35 TL. Reddedilen komut: 296.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -15530795,99 TL.
- Depo ve merkez: -9166015,50 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -16464836,60 TL.
- Internet satışi: 0,00 TL.
- Subeler: -104823314,87 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 7 | 1135624 / 2649343793 |
| 2 | 18 | 20 | 20 | 5775723 / 2742359338 |
| 3 | 10 | 20 | 40 | 23563697 / 2806522293 |
| 4 | 9 | 20 | 66 | 47283439 / 2851676281 |
| 5 | 8 | 20 | 93 | 68763750 / 2934839710 |
| 6 | 8 | 20 | 119 | 85014325 / 3037157017 |
| 7 | 7 | 19 | 145 | 100780015 / 3095215116 |
| 8 | 80 | 20 | 151 | 130085 / 3168933777 |
| 9 | 77 | 20 | 151 | 0 / 3289308589 |
| 10 | 74 | 20 | 151 | 0 / 3360673294 |

Rakipler: en çok 92 etkin zincir; 4 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 198 fiyat savaşı.
Ezeli rakip: Ezeli rakip: Migron (Selin Tuna) · 1956 mağaza · senden çekildiği savaş: 29.
Markalardan toplam 1282208.58 TL; küsen farklı marka 9.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 210. gün 0 -> 1.
- Süt ürünleri: 420. gün 0 -> 1.
- Temizlik ve bakım: 420. gün 0 -> 1.
- İçecek: 450. gün 0 -> 1.
- Kuru gıda: 600. gün 1 -> 2.
- Temizlik ve bakım: 750. gün 1 -> 2.
- İçecek: 780. gün 1 -> 2.
- Süt ürünleri: 780. gün 1 -> 2.
- Kuru gıda: 930. gün 2 -> 3.
- Süt ürünleri: 1290. gün 2 -> 3.
- Temizlik ve bakım: 1380. gün 2 -> 3.
- İçecek: 1440. gün 2 -> 3.
- İçecek: 2670. gün 3 -> 2.
- Süt ürünleri: 2670. gün 3 -> 2.
- Kuru gıda: 2670. gün 3 -> 2.
- Temizlik ve bakım: 2670. gün 3 -> 2.
- İçecek: 2700. gün 2 -> 1.
- Süt ürünleri: 2700. gün 2 -> 1.
- Kuru gıda: 2700. gün 2 -> 1.
- Temizlik ve bakım: 2700. gün 2 -> 1.
- İçecek: 2730. gün 1 -> 0.
- Süt ürünleri: 2730. gün 1 -> 0.
- Kuru gıda: 2730. gün 1 -> 0.
- Temizlik ve bakım: 2730. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -1486.22 / 2872.04 (zarar görüldü).
- Tür 2, Manav: -3227.91 / 34904.25 (zarar görüldü).
- Tür 2, Kasap: -8826.14 / 42467.50 (zarar görüldü).
- Tür 2, Fırın ve pastane: -3866.75 / 17815.84 (zarar görüldü).
- Tür 3, Manav: -13549.84 / 574675.15 (zarar görüldü).
- Tür 3, Kasap: -10005.08 / 798703.83 (zarar görüldü).
- Tür 3, Bebek: -42114.40 / 35735.97 (zarar görüldü).
- Tür 3, Evcil hayvan: -56221.73 / -231.34 (zarar görüldü).
- Tür 3, Bahçe ve oto: -49324.24 / 90133.71 (zarar görüldü).
- Tür 3, Mevsimlik: -35110.51 / 131052.41 (zarar görüldü).
- Tür 3, Şarküteri: -23216.84 / 327420.23 (zarar görüldü).
- Tür 3, Fırın ve pastane: -40745.93 / 396301.40 (zarar görüldü).
- Tür 3, Balık: -155026.49 / -474.18 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -96506.63 / -2843.46 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -192279.49 / 430607.03 (zarar görüldü).
- Tür 3, Ev ve mutfak: -32256.50 / 135447.29 (zarar görüldü).
- Tür 3, Oyuncak: -8823.62 / 268973.46 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -48937.70 / 123916.31 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 14 / 52. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 74.
- RejectBrandOffer: 282.
- ReplaceMasters: 81.
- SetDepartment: 118.
- SetSourcing: 24.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 207.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Açılış için 11.170,68 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.243,02 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.642,41 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.699,60 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.712,35 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.756,69 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.768,46 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.877,34 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 12.017,87 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 41.940,77 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 42.625,95 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 43.228,49 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Tedarik asgarisi: 207.
- Önce yerel pay %35.: 6.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Dengeli / tohum 22

- İlk şube: 107. gün.
- 5 mağaza: 233. gün.
- İlk depo: 421. gün.
- İlk il müdüru: 1401. gün.
- 5 il: 261. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 186289661,11 TL.
- Vergi ödemeleri: 20046176,23 TL.
- Üst yönetim ücretleri: 16047028,94 TL.
- İşletme giderleri (ücret hariç): 15479723,07 TL.
- Depo ve merkez giderleri: 8655494,26 TL.

Arka planin kasaya toplam net etkisi: -144657480,00 TL. Reddedilen komut: 286.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -14824241,09 TL.
- Depo ve merkez: -8655494,26 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -16250070,11 TL.
- Internet satışi: 0,00 TL.
- Subeler: -109307539,99 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 7 | 1231748 / 2585369432 |
| 2 | 18 | 20 | 18 | 4754079 / 2664411879 |
| 3 | 10 | 20 | 38 | 22658308 / 2739396267 |
| 4 | 9 | 20 | 64 | 47010735 / 2818767366 |
| 5 | 8 | 20 | 90 | 67070511 / 2940529417 |
| 6 | 8 | 20 | 117 | 84936538 / 3007185519 |
| 7 | 8 | 20 | 141 | 65095722 / 3127550970 |
| 8 | 72 | 20 | 141 | 9329 / 3215860338 |
| 9 | 72 | 20 | 141 | 0 / 3285197661 |
| 10 | 69 | 20 | 141 | 0 / 3425491994 |

Rakipler: en çok 94 etkin zincir; 4 farklı satılık zincir; 3 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 194 fiyat savaşı.
Ezeli rakip: Ezeli rakip: ŞAK (Deniz Arslan) · 3614 mağaza · senden çekildiği savaş: 24.
Markalardan toplam 907069.65 TL; küsen farklı marka 8.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 210. gün 0 -> 1.
- Süt ürünleri: 390. gün 0 -> 1.
- Temizlik ve bakım: 390. gün 0 -> 1.
- İçecek: 420. gün 0 -> 1.
- Kuru gıda: 660. gün 1 -> 2.
- Süt ürünleri: 780. gün 1 -> 2.
- Temizlik ve bakım: 780. gün 1 -> 2.
- İçecek: 810. gün 1 -> 2.
- Kuru gıda: 990. gün 2 -> 3.
- Süt ürünleri: 1290. gün 2 -> 3.
- Temizlik ve bakım: 1380. gün 2 -> 3.
- İçecek: 1440. gün 2 -> 3.
- İçecek: 2580. gün 3 -> 2.
- Süt ürünleri: 2580. gün 3 -> 2.
- Kuru gıda: 2580. gün 3 -> 2.
- Temizlik ve bakım: 2580. gün 3 -> 2.
- İçecek: 2610. gün 2 -> 1.
- Süt ürünleri: 2610. gün 2 -> 1.
- Kuru gıda: 2610. gün 2 -> 1.
- Temizlik ve bakım: 2610. gün 2 -> 1.
- İçecek: 2640. gün 1 -> 0.
- Süt ürünleri: 2640. gün 1 -> 0.
- Kuru gıda: 2640. gün 1 -> 0.
- Temizlik ve bakım: 2640. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -696.93 / 3288.09 (zarar görüldü).
- Tür 2, Manav: -1493.40 / 33304.00 (zarar görüldü).
- Tür 2, Kasap: -5228.33 / 40119.41 (zarar görüldü).
- Tür 2, Fırın ve pastane: -457.95 / 15903.52 (zarar görüldü).
- Tür 3, Manav: -45588.14 / 575904.75 (zarar görüldü).
- Tür 3, Kasap: -34708.53 / 801201.03 (zarar görüldü).
- Tür 3, Bebek: -10932.40 / 34676.40 (zarar görüldü).
- Tür 3, Evcil hayvan: -81305.27 / -125.46 (zarar görüldü).
- Tür 3, Bahçe ve oto: -15947.59 / 87654.00 (zarar görüldü).
- Tür 3, Mevsimlik: -30759.69 / 136872.01 (zarar görüldü).
- Tür 3, Şarküteri: -30286.71 / 339626.22 (zarar görüldü).
- Tür 3, Fırın ve pastane: -50057.90 / 391546.81 (zarar görüldü).
- Tür 3, Balık: -184114.92 / -388.03 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -145812.44 / -2586.86 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -12441.20 / 443501.09 (zarar görüldü).
- Tür 3, Ev ve mutfak: -8731.06 / 141667.81 (zarar görüldü).
- Tür 3, Oyuncak: -10285.73 / 276642.60 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -62452.37 / 70185.62 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 14 / 54. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 66.
- RejectBrandOffer: 297.
- ReplaceMasters: 74.
- SetDepartment: 110.
- SetSourcing: 24.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 206.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Açılış için 11.160,59 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.562,73 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.565,71 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.634,16 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.708,33 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.770,52 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.817,80 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 41.978,56 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 41.994,84 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 43.490,37 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 43.890,81 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 44.424,84 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 67.162,32 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Tedarik asgarisi: 206.
- Önce yerel pay %35.: 6.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Dengeli / tohum 23

- İlk şube: 107. gün.
- 5 mağaza: 233. gün.
- İlk depo: 407. gün.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 261. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 47204230,20 TL.
- Depo ve merkez giderleri: 2774840,41 TL.
- Üst yönetim ücretleri: 2353264,26 TL.
- İşletme giderleri (ücret hariç): 974353,21 TL.
- Vergi ödemeleri: 547547,22 TL.

Arka planin kasaya toplam net etkisi: -50232939,71 TL. Reddedilen komut: 50.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -842514,31 TL.
- Depo ve merkez: -2774840,41 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -2773180,24 TL.
- Internet satışi: 0,00 TL.
- Subeler: -44415358,70 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 27 | 20 | 7 | 1321448 / 2616019729 |
| 2 | 14 | 20 | 23 | 7546158 / 2664104721 |
| 3 | 26 | 20 | 31 | 2456053 / 2766262382 |
| 4 | 26 | 20 | 31 | 2401530 / 2881109386 |
| 5 | 26 | 20 | 31 | 2428453 / 2996229747 |
| 6 | 27 | 20 | 31 | 1846424 / 3089990654 |
| 7 | 27 | 20 | 31 | 1778309 / 3158913001 |
| 8 | 28 | 20 | 31 | 1819098 / 3214436872 |
| 9 | 28 | 20 | 31 | 1805767 / 3297649008 |
| 10 | 29 | 20 | 31 | 1853684 / 3417996779 |

Rakipler: en çok 80 etkin zincir; 0 farklı satılık zincir; 0 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 100 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 9025 mağaza · senden çekildiği savaş: 4.
Markalardan toplam 127082.02 TL; küsen farklı marka 7.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 210. gün 0 -> 1.
- Süt ürünleri: 360. gün 0 -> 1.
- İçecek: 390. gün 0 -> 1.
- Temizlik ve bakım: 390. gün 0 -> 1.
- Kuru gıda: 570. gün 1 -> 2.
- Süt ürünleri: 720. gün 1 -> 2.
- Temizlik ve bakım: 720. gün 1 -> 2.
- İçecek: 750. gün 1 -> 2.
- İçecek: 900. gün 2 -> 1.
- Süt ürünleri: 900. gün 2 -> 1.
- Kuru gıda: 900. gün 2 -> 1.
- Temizlik ve bakım: 900. gün 2 -> 1.
- İçecek: 930. gün 1 -> 0.
- Süt ürünleri: 930. gün 1 -> 0.
- Kuru gıda: 930. gün 1 -> 0.
- Temizlik ve bakım: 930. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -1266.46 / 2696.69 (zarar görüldü).
- Tür 2, Manav: -283.33 / 27524.40 (zarar görüldü).
- Tür 2, Kasap: -438.93 / 24482.19 (zarar görüldü).
- Tür 2, Fırın ve pastane: -5579.56 / 13581.57 (zarar görüldü).
- Tür 3, Manav: 50.73 / 44878.83.
- Tür 3, Kasap: 45.13 / 46576.95.
- Tür 3, Bebek: -1600.71 / -100.37 (zarar görüldü).
- Tür 3, Mevsimlik: -203.74 / 9141.24 (zarar görüldü).
- Tür 3, Şarküteri: -68.22 / 21991.65 (zarar görüldü).
- Tür 3, Fırın ve pastane: -26.65 / 28758.33 (zarar görüldü).
- Tür 3, Balık: -10626.62 / -588.35 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -2236.07 / 19034.68 (zarar görüldü).
- Tür 3, Ev ve mutfak: -1399.31 / 8738.48 (zarar görüldü).
- Tür 3, Oyuncak: -333.66 / 1436.05 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -1572.85 / -160.22 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 16 / 38. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 78.
- RejectBrandOffer: 275.
- ReplaceMasters: 14.
- SetDepartment: 27.
- SetSourcing: 16.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 401.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Açılış için 11.166,36 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.515,00 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.569,77 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.620,62 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.655,08 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.707,06 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 41.424,41 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 42.687,95 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 42.835,00 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Tedarik asgarisi: 401.
- Önce yerel pay %35.: 6.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Atak / tohum 21

- İlk şube: 79. gün.
- 5 mağaza: 163. gün.
- İlk depo: 302. gün.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 233. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 19871255,37 TL.
- Üst yönetim ücretleri: 2536474,31 TL.
- Depo ve merkez giderleri: 1622482,61 TL.
- İşletme giderleri (ücret hariç): 397416,02 TL.
- Mal alımi (stok yatırımi): 277758,32 TL.

Arka planin kasaya toplam net etkisi: -23458754,58 TL. Reddedilen komut: 31.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -321105,15 TL.
- Depo ve merkez: -1622482,61 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -2978993,70 TL.
- Internet satışi: 0,00 TL.
- Subeler: -19100456,12 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 21 | 20 | 14 | 4193953 / 2649343793 |
| 2 | 28 | 20 | 15 | 748015 / 2742359338 |
| 3 | 28 | 20 | 15 | 703969 / 2806522293 |
| 4 | 28 | 20 | 15 | 686586 / 2851676281 |
| 5 | 28 | 20 | 15 | 685094 / 2934839710 |
| 6 | 30 | 20 | 15 | 512043 / 3037157017 |
| 7 | 30 | 20 | 15 | 502839 / 3095215116 |
| 8 | 31 | 20 | 15 | 512117 / 3168933777 |
| 9 | 31 | 20 | 15 | 519961 / 3289308589 |
| 10 | 32 | 20 | 15 | 497293 / 3360673294 |

Rakipler: en çok 48 etkin zincir; 0 farklı satılık zincir; 0 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 47 fiyat savaşı.
Ezeli rakip: Ezeli rakip: Migron (Selin Tuna) · 2047 mağaza · senden çekildiği savaş: 3.
Markalardan toplam 109422.00 TL; küsen farklı marka 6.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 120. gün 0 -> 1.
- İçecek: 225. gün 0 -> 1.
- Süt ürünleri: 225. gün 0 -> 1.
- Temizlik ve bakım: 225. gün 0 -> 1.
- Kuru gıda: 315. gün 1 -> 2.
- İçecek: 465. gün 1 -> 0.
- Süt ürünleri: 465. gün 1 -> 0.
- Kuru gıda: 465. gün 2 -> 1.
- Temizlik ve bakım: 465. gün 1 -> 0.
- Kuru gıda: 480. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -379.46 / 597.21 (zarar görüldü).
- Tür 2, Manav: -356.53 / 10502.03 (zarar görüldü).
- Tür 2, Kasap: -928.97 / 12069.55 (zarar görüldü).
- Tür 2, Evcil hayvan: 5.79 / 1991.58.
- Tür 2, Fırın ve pastane: -1503.00 / 6712.94 (zarar görüldü).
- Tür 2, Balık: -636.90 / -31.96 (zarar görüldü).
- Tür 3, Manav: -72.46 / 6503.73 (zarar görüldü).
- Tür 3, Kasap: -14.22 / 8780.55 (zarar görüldü).
- Tür 3, Bebek: -281.94 / -17.29 (zarar görüldü).
- Tür 3, Evcil hayvan: -513.47 / -30.32 (zarar görüldü).
- Tür 3, Bahçe ve oto: -42.81 / 748.97 (zarar görüldü).
- Tür 3, Mevsimlik: -25.34 / 1043.00 (zarar görüldü).
- Tür 3, Şarküteri: -20.50 / 3100.45 (zarar görüldü).
- Tür 3, Fırın ve pastane: -75.65 / 5271.94 (zarar görüldü).
- Tür 3, Balık: -1426.62 / -91.31 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -456.95 / 4368.77 (zarar görüldü).
- Tür 3, Ev ve mutfak: -90.18 / 885.94 (zarar görüldü).
- Tür 3, Oyuncak: -71.32 / -11.88 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -559.95 / -32.33 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 14 / 22. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 84.
- RejectBrandOffer: 256.
- ReplaceMasters: 9.
- SetDepartment: 36.
- SetDeptStance: 13.
- SetSourcing: 10.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 906.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Açılış için 10.520,64 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.523,52 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.579,90 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.589,16 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.600,45 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.602,41 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.609,67 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.888,58 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.967,31 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.999,47 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.002,51 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.003,57 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.003,92 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.225,15 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.344,73 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.352,02 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.355,82 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.356,78 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.431,02 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.633,13 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 37.169,49 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 37.861,93 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 38.066,05 TL gerekiyor (depozito, tadilat, açılış stoğu).: 3.
- Açılış için 38.271,89 TL gerekiyor (depozito, tadilat, açılış stoğu).: 5.
- Açılış için 38.504,06 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 38.965,08 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 5.298,96 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Tedarik asgarisi: 906.
- Önce işletmenin borcunu kapat.: 1.
- Önce yerel pay %35.: 8.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Atak / tohum 22

- İlk şube: 79. gün.
- 5 mağaza: 163. gün.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: bu koşuda ulaşılmadı.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 3305626,74 TL.
- Üst yönetim ücretleri: 262937,33 TL.
- İşletme giderleri (ücret hariç): 159166,21 TL.
- Mal alımi (stok yatırımi): 114364,40 TL.
- Aile dükkânınin ücretleri: 46973,50 TL.

Arka planin kasaya toplam net etkisi: -3551691,78 TL. Reddedilen komut: 1.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -152758,94 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -300921,74 TL.
- Internet satışi: 0,00 TL.
- Subeler: -3287938,36 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 30 | 20 | 5 | 4691 / 2585369432 |
| 2 | 30 | 20 | 5 | 3172 / 2664411879 |
| 3 | 30 | 20 | 5 | 3198 / 2739396267 |
| 4 | 30 | 20 | 5 | 3315 / 2818767366 |
| 5 | 30 | 20 | 5 | 2922 / 2940529417 |
| 6 | 30 | 20 | 5 | 2823 / 3007185519 |
| 7 | 30 | 20 | 5 | 2793 / 3127550970 |
| 8 | 33 | 20 | 5 | 3028 / 3215860338 |
| 9 | 34 | 20 | 5 | 3023 / 3285197661 |
| 10 | 34 | 20 | 5 | 3033 / 3425491994 |

Rakipler: en çok 35 etkin zincir; 0 farklı satılık zincir; 0 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 2 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 105898.20 TL; küsen farklı marka 9.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 120. gün 0 -> 1.
- Kuru gıda: 195. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -386.40 / -24.57 (zarar görüldü).
- Tür 2, Manav: -82.10 / 137.92 (zarar görüldü).
- Tür 2, Kasap: -140.20 / -17.30 (zarar görüldü).
- Tür 2, Bebek: 3.08 / 136.84.

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 14 / 14. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 88.
- RejectBrandOffer: 239.
- SetDepartment: 9.
- SetDeptStance: 3.
- SetSourcing: 2.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 946.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Açılış için 10.524,69 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.527,12 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.530,06 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.531,63 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.599,77 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.602,06 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.609,46 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.661,41 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.001,77 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 5.296,93 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Tedarik asgarisi: 946.
- Önce işletmenin borcunu kapat.: 1.
- Önce yerel pay %35.: 8.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Atak / tohum 23

- İlk şube: 79. gün.
- 5 mağaza: 163. gün.
- İlk depo: 260. gün.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: 289. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 36922716,41 TL.
- Üst yönetim ücretleri: 2496704,36 TL.
- Depo ve merkez giderleri: 1939572,27 TL.
- İşletme giderleri (ücret hariç): 767467,15 TL.
- Vergi ödemeleri: 408750,67 TL.

Arka planin kasaya toplam net etkisi: -39970246,42 TL. Reddedilen komut: 57.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -660874,73 TL.
- Depo ve merkez: -1939572,27 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -3040389,18 TL.
- Internet satışi: 0,00 TL.
- Subeler: -34863880,85 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 26 | 20 | 10 | 2709808 / 2616019729 |
| 2 | 22 | 20 | 22 | 4115039 / 2664104721 |
| 3 | 27 | 20 | 22 | 2218836 / 2766262382 |
| 4 | 26 | 20 | 22 | 2273462 / 2881109386 |
| 5 | 27 | 20 | 22 | 1754646 / 2996229747 |
| 6 | 27 | 20 | 22 | 1713710 / 3089990654 |
| 7 | 27 | 20 | 22 | 1346818 / 3158913001 |
| 8 | 28 | 20 | 22 | 1133280 / 3214436872 |
| 9 | 28 | 20 | 22 | 1154743 / 3297649008 |
| 10 | 29 | 20 | 22 | 1157533 / 3417996779 |

Rakipler: en çok 62 etkin zincir; 0 farklı satılık zincir; 0 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 142 fiyat savaşı.
Ezeli rakip: Ezeli rakip: ŞAK (Deniz Arslan) · 3811 mağaza · senden çekildiği savaş: 5.
Markalardan toplam 121337.65 TL; küsen farklı marka 8.

İflas nedeni için ayrı durum bayrağı yok; haber sayısı haber tavanı nedeniyle alt sınırdır, bütün kapanmalar iflas sayılmaz.

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 105. gün 0 -> 1.
- Süt ürünleri: 225. gün 0 -> 1.
- İçecek: 255. gün 0 -> 1.
- Temizlik ve bakım: 255. gün 0 -> 1.
- Kuru gıda: 390. gün 1 -> 2.
- İçecek: 465. gün 1 -> 2.
- Süt ürünleri: 525. gün 1 -> 2.
- Temizlik ve bakım: 585. gün 1 -> 2.
- İçecek: 660. gün 2 -> 1.
- Süt ürünleri: 660. gün 2 -> 1.
- Temizlik ve bakım: 660. gün 2 -> 1.
- İçecek: 675. gün 1 -> 0.
- Süt ürünleri: 675. gün 1 -> 0.
- Kuru gıda: 675. gün 2 -> 1.
- Temizlik ve bakım: 675. gün 1 -> 0.
- Kuru gıda: 690. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -359.16 / 1125.57 (zarar görüldü).
- Tür 2, Manav: -448.84 / 16650.61 (zarar görüldü).
- Tür 2, Kasap: -672.55 / 19144.71 (zarar görüldü).
- Tür 2, Evcil hayvan: 5.48 / 2428.28.
- Tür 2, Fırın ve pastane: -875.35 / 9580.31 (zarar görüldü).
- Tür 2, Balık: -593.57 / -28.59 (zarar görüldü).
- Tür 3, Manav: -157.27 / 22462.48 (zarar görüldü).
- Tür 3, Kasap: -11.12 / 34069.79 (zarar görüldü).
- Tür 3, Bebek: -99.98 / 2596.38 (zarar görüldü).
- Tür 3, Evcil hayvan: -1108.35 / -43.15 (zarar görüldü).
- Tür 3, Bahçe ve oto: -159.02 / 1273.75 (zarar görüldü).
- Tür 3, Mevsimlik: -201.28 / 6128.26 (zarar görüldü).
- Tür 3, Şarküteri: -38.78 / 17222.39 (zarar görüldü).
- Tür 3, Fırın ve pastane: -6.68 / 22362.24 (zarar görüldü).
- Tür 3, Balık: -5046.91 / -159.70 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -1866.56 / 20276.95 (zarar görüldü).
- Tür 3, Ev ve mutfak: -425.27 / 8191.44 (zarar görüldü).
- Tür 3, Oyuncak: -476.35 / 11427.49 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -416.65 / 557.41 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 16 / 51. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 80.
- RejectBrandOffer: 268.
- ReplaceMasters: 14.
- SetDepartment: 49.
- SetDeptStance: 13.
- SetSourcing: 16.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 850.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Açılış için 10.533,25 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.536,11 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.545,38 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.548,24 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.596,10 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.597,55 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.660,65 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.662,21 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.730,25 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.932,75 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 10.969,31 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.016,55 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.019,57 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.039,56 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.044,52 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.084,43 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.086,20 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.108,83 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.177,19 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 11.364,59 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 11.600,04 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 37.603,90 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 37.810,86 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 38.279,10 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 38.488,33 TL gerekiyor (depozito, tadilat, açılış stoğu).: 3.
- Açılış için 38.716,59 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 38.718,57 TL gerekiyor (depozito, tadilat, açılış stoğu).: 2.
- Açılış için 39.392,52 TL gerekiyor (depozito, tadilat, açılış stoğu).: 5.
- Açılış için 39.793,02 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 40.032,52 TL gerekiyor (depozito, tadilat, açılış stoğu).: 3.
- Açılış için 40.202,80 TL gerekiyor (depozito, tadilat, açılış stoğu).: 3.
- Açılış için 40.858,63 TL gerekiyor (depozito, tadilat, açılış stoğu).: 3.
- Açılış için 40.860,79 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 41.152,07 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Tedarik asgarisi: 850.
- Önce işletmenin borcunu kapat.: 1.
- Önce yerel pay %35.: 9.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

## Denetimin kapsamı

Satış fisindeki para, siparis bedeli, ana gün kapanisi ve mal kabul aktarimi bagimsiz hesapla kontrol edilir. Negatif stok, gecersiz sayilar ve pay sinirlari her gün denetlenir. Arka planin net kasa hareketi ayrica olculur; tek tek kalemlerin tam korunum denetimi B'nin muhasebe defteri C tarafindan baglandiginda tamamlanacak. Kasa eksisi oyun sonu degildir; sikinti günleri ayri sayilir. Ligler C bolumunde yillik olculur; B/C3 entegrasyonu oncesi bu koşu tam oyun dengesi degildir.

Fiyatlar normal oyuncunun kullandigi adimlarla degisir. Kredi, şube, depo, yonetici ve kararlar normal komutlardan gecer. Aile dükkânı PlayDay ile oynar; test modu, bedava mal veya para kullanilmaz. CSV tutarlari kurustur.
