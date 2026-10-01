# C3: büyüme erken duruyor; para hesabı tutuyor

01.10.2026 · main · tohum 21 · 30 yıl × temkinli/dengeli/atak. Tüm 32.874 gün oynandı; satış/stok denetimi ve muhasebe farkı sıfır. 10 yıl × üç tarz × üç tohum ile toplam 12 kampanya / 65.751 gün: bütün kampanyalarda kasa eksiye düştü, defter farkı yine sıfır. İlk birleşim testleri 125/125; rapor ölçümleriyle 126/126 ve Smoke geçti. Derleme/test düzeltmesi gerekmedi.

| Tarz | İlk kasa eksisi (gün) | 30. yıl mağaza | Son kasa (TL) |
|---|---:|---:|---:|
| Temkinli | 850 | 2 | -38.422.998,13 |
| Dengeli | 141 | 3 | -54.244.068,32 |
| Atak | 93 | 4 | -73.175.466,34 |

Dengeli botun 10./20./30. yıl ulusal sırası **32 / 32 / 33**, dünya sırası **20 / 20 / 20**. Önceki A6'da aynı tohumla 151 mağazaya ulaşmıştı; şimdi üç mağazada kalıyor. Önceki ulusal sıralar 74/63/60 olduğundan yalnız sıra sayısına bakmak yanıltır: bu koşuda rakip kadrosu da küçük kalıyor, bizim ortak ciromuz bu üç yılda sıfır. B ve C birlikte eklendi; bu karşılaştırma tek bir ayarın etkisini ayırmaz.

## En önemli beş bulgu ve C'ye ayar önerileri

1. **İlk şubeler kasayı tüketiyor.** Dengeli 89. gün kasa 13.334,18 TL, günlük net +188,54 TL; 120. gün 1.895,34 TL / -101,19 TL; 141. gün -30,55 TL / -299,97 TL. 180. günde aile dükkânının cirosu sıfır; mal alacak para kalmıyor. Öncelik: ilk şubenin ciro/kira/ücret dengesini ve açılış sonrası işletme sermayesini incelemek. Deneme adayı: ilk mahalle şubesinde 14 günlük **bütün ağın** gideri için rezerv ve aylık kira/ücret tahminini açılış önizlemesine koymak. Botun mevcut rezervi aile dükkânına dayanıyor; zarar eden şubeleri kapatmıyor. Sonuç, bütün insan stratejilerinin aynı sona gittiğinin kanıtı değildir.
2. **Reyon ve tedarik ayarları uzun koşuda yeterince sınanamadı.** Süper/hiper yok; tek ölçülen reyon temkinlinin mahalle manavı: 8 aylık gözlem, 2 zararlı, toplam 30 günlük kâr -125,53 ile +243,81 TL arasında. 14 reyonun en iyi/en kötü üçlüsü için veri yok. Kademe değişimi, zincir alımı, depo ve dış ülke yok. Balık/elektronik/hiper ayarlarını başarılı saymayın; nakit akışı düzeldikten sonra aynı 12 koşuyu yeniden yapın. Tedarik asgarilerini şimdilik değiştirmeyin: bu koşu üst kademeye ulaşamıyor.
3. **Marka parası boş raflarla da sürüyor — istismar şüphesi.** 30 yılda temkinli/dengeli/atak 2.373.721,17 / 2.399.663,22 / 1.910.132,72 TL marka geliri alıyor; satışlar çok önce duruyor. MarketBrands.cpp:67,75 kapasiteyi sayıyor; :334 ve :365 kapasite/raf payıyla ödeme yapıyor. C'ye öneri: raf tahsisinin yanında ürünün gerçekten mevcut olmasını, örneğin ayın en az 21 gününde rafta bulunmasını ödeme koşulu olarak denemek. Bu yeni oyun kararıdır; kodu değiştirmedim. Marka güveninin boş rafı da görüp görmediği incelenmeli (9–10 küsen marka).
4. **Ritim koruyucusu sayısal olarak çalışıyor; şirket hayatı yine durgun.** 30 yıllık üç koşuda eşik aşan sessizlik 0, en uzun sessizlik 19 gün; koruyucu 16/13/20 sakin olay getiriyor ve 62/58/60 kötü olayı erteliyor. Felaket kümeleri mahalle / C dahil her üçünde 0/0 (A6 56/141, 56/149, 56/64). Buna karşılık hedef tamamlamaları yalnız 75/14/17; 1.981/1.967/1.970 gözlenen hedefin çoğu boşa geçiyor, mağazalar 2/3/4'te kalıyor. Kutlama 89/24/28. Öneri: sıkıntı aşamasında büyüme hedefinden stok ve toparlanma hedefine dönmek; 30 yıllık yinelenen karar akışını 'zevkli' diye kabul etmemek. Olay ölçüleri oyunun zevkini tek başına kanıtlamaz.
5. **Lig ölçeği hâlâ uzakta; ayarı önce nakit düzeldikten sonra ölçün.** 30. yıl liderin ortak cirosu 5.980.908.128, bizimki sıfır. A6'nın LeagueCompression 0,04 → 0,004 ve ulusal başlangıç mağazaları ×0,1 deneme önerisi korunuyor; şimdi uygularsak erken nakit çöküşünü gizleyebilir. Rakip kapanma nedenleri artık kesin: üç koşuda da 0 iflas/kapanma, 4 rakip satın alması, 0 bizim alımımız; 2/2/1 savaş, ezeli rakip yok. Düşük savaş sayısı, yeni 21 gün / 60 gün dinlenme ayarının tek başına başarısını göstermez; ağ büyümüyor.

## Dönemler ve ölçümün sınırı

Tohum 21'de ilk kur şoku 3.105–3.279, durgunluk 3.280–3.614, salgın 3.674–4.145, yüksek enflasyon 4.223–5.075, toparlanma 5.135–5.499. Kasa üç tarzda da bunlardan **önce** eksiye düşüyor; ilk çöküş dönem şokuna bağlanamaz. Aşağıdaki tablolar her dönemde ilk/son/en az kasa ile defterin günlük net kâr toplamını verir. Kasa değerleri gün sonudur; dönemler gerçek tarihle değil kampanya günüyle gösterilir. Reyon Last30Profit yaklaşık yumuşatılmış toplamdır, literal son 30 günün defteri değildir. Hedef sayısı kapanışta görülen yeni hedeflerdir; kısa süreli ara durumları saymaz. Bot nakit sıkıntısında normal oyuncu komutları ve PlayDay ile gözleme devam eder; durması gereken UI turunu otomatik geçmez, para/mal verilmez.

Kaynaklar: Saved/AutoPlay/20261001-101756 (30 yıl), 20261001-101807 (10 yıl). Lig, reyon, tedarik, defter/ritim ve dönem CSV'leri bu raporun yanında C3 önekiyle saklandı. Günlük bütün gözlemler Saved içinde.

---
# Otomatik oyuncunun denge raporu

3 koşu, her biri 10958 gün; toplam sure 44.5 saniye.

| Tarz | Tohum | Son kasa | Borç | Magaza | İl | Ulusal pay | Kasa eksi gün | Sıkıntı günu | Denetim hatasi |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Temkinli | 21 | -38422998,13 TL | 320,16 TL | 2 | 1 | 0.0000% | 10109 | 10109 | 0 |
| Dengeli | 21 | -54244068,32 TL | 5599,58 TL | 3 | 2 | 0.0000% | 10818 | 10818 | 0 |
| Atak | 21 | -73175466,34 TL | 157230,07 TL | 4 | 2 | 0.0000% | 10848 | 10848 | 0 |

## Bulgular ve neye bakmalı

- 3/3 koşuda kasa eksiye düştü.
- 3/3 koşu birden cok mağazaya ulaştı.
- Satış, siparis, stok ve sayi denetimi: 0 hata.


### Temkinli / tohum 21

- İlk şube: 399. gün.
- 5 mağaza: bu koşuda ulaşılmadı.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: bu koşuda ulaşılmadı.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 17205353,54 TL.
- Aile dükkânınin ücretleri: 10842278,74 TL.
- Üst yönetim ücretleri: 6401212,26 TL.
- İşletme giderleri (ücret hariç): 5062185,77 TL.
- Mal alımi (stok yatırımi): 330572,90 TL.

Arka planin kasaya toplam net etkisi: -25103515,23 TL. Reddedilen komut: 12.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -13295900,06 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -10272005,57 TL.
- Internet satışi: 0,00 TL.
- Subeler: -17177124,30 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 1 | 151145 / 2649343793 |
| 2 | 29 | 20 | 2 | 291018 / 2742359338 |
| 3 | 29 | 20 | 2 | 35 / 2806522293 |
| 4 | 29 | 20 | 2 | 0 / 2851676281 |
| 5 | 29 | 20 | 2 | 0 / 2934839710 |
| 6 | 29 | 20 | 2 | 0 / 3037157017 |
| 7 | 29 | 20 | 2 | 0 / 3095215116 |
| 8 | 30 | 20 | 2 | 0 / 3168933777 |
| 9 | 30 | 20 | 2 | 0 / 3289308589 |
| 10 | 31 | 20 | 2 | 0 / 3360673294 |
| 11 | 29 | 20 | 2 | 0 / 3439091198 |
| 12 | 28 | 20 | 2 | 0 / 3551332904 |
| 13 | 28 | 20 | 2 | 0 / 3608917786 |
| 14 | 28 | 20 | 2 | 0 / 3764119345 |
| 15 | 29 | 20 | 2 | 0 / 3906787046 |
| 16 | 31 | 20 | 2 | 0 / 4022490350 |
| 17 | 31 | 20 | 2 | 0 / 4106515797 |
| 18 | 31 | 20 | 2 | 0 / 4247660882 |
| 19 | 31 | 20 | 2 | 0 / 4438563514 |
| 20 | 31 | 20 | 2 | 0 / 4531107228 |
| 21 | 31 | 20 | 2 | 0 / 4703714880 |
| 22 | 31 | 20 | 2 | 0 / 4818212431 |
| 23 | 31 | 20 | 2 | 0 / 4911777117 |
| 24 | 33 | 20 | 2 | 0 / 5017046055 |
| 25 | 32 | 20 | 2 | 0 / 5174495485 |
| 26 | 32 | 20 | 2 | 0 / 5294445267 |
| 27 | 32 | 20 | 2 | 0 / 5384598971 |
| 28 | 32 | 20 | 2 | 0 / 5614968312 |
| 29 | 32 | 20 | 2 | 0 / 5786449255 |
| 30 | 32 | 20 | 2 | 0 / 5980908128 |

Rakipler: en çok 32 etkin zincir; 4 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 2 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 2373721.17 TL; küsen farklı marka 10.

C3 kapanma nedenleri: 0 iflas/kapanma, 4 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 1981, tamamlanan 75; kutlama 89. Ritim koruyucusu: 16 sakin dönem olayı, 62 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | -1175559.47 | -1296476.35 | -1296476.35 | -121618.67 |
| durgünluk / 1 | 3280–3614 | 335 | -1297233.11 | -1546243.39 | -1546243.39 | -249767.04 |
| büyük salgın / 1 | 3674–4145 | 472 | -1595545.37 | -2010247.24 | -2010247.24 | -415559.60 |
| yüksek enflasyon / 1 | 4223–5075 | 853 | -2090462.68 | -3298455.45 | -3298455.45 | -1209075.33 |
| toparlanma / 1 | 5135–5499 | 365 | -3415093.71 | -4202689.85 | -4202689.85 | -789648.78 |
| kur şoku / 2 | 7753–7936 | 184 | -11785755.81 | -12636601.93 | -12636601.93 | -855536.90 |
| yüksek enflasyon / 2 | 7998–8543 | 546 | -12938723.43 | -15891846.80 | -15891846.80 | -2958428.93 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kademe değişmedi.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -143.15 / 260.79 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 0. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 271.
- RejectBrandOffer: 380.
- SetDepartment: 2.
- SetDeptStance: 1.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 0.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Önce yerel pay %35.: 9.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Dengeli / tohum 21

- İlk şube: 107. gün.
- 5 mağaza: bu koşuda ulaşılmadı.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: bu koşuda ulaşılmadı.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 33671971,23 TL.
- Aile dükkânınin ücretleri: 10786964,50 TL.
- Üst yönetim ücretleri: 5845316,31 TL.
- İşletme giderleri (ücret hariç): 5054021,39 TL.
- Mal alımi (stok yatırımi): 81166,96 TL.

Arka planin kasaya toplam net etkisi: -40923287,82 TL. Reddedilen komut: 0.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -13390900,35 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -9579313,99 TL.
- Internet satışi: 0,00 TL.
- Subeler: -33671910,89 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 30 | 20 | 3 | 62 / 2649343793 |
| 2 | 30 | 20 | 3 | 0 / 2742359338 |
| 3 | 30 | 20 | 3 | 0 / 2806522293 |
| 4 | 30 | 20 | 3 | 0 / 2851676281 |
| 5 | 30 | 20 | 3 | 0 / 2934839710 |
| 6 | 30 | 20 | 3 | 0 / 3037157017 |
| 7 | 30 | 20 | 3 | 0 / 3095215116 |
| 8 | 31 | 20 | 3 | 0 / 3168933777 |
| 9 | 31 | 20 | 3 | 0 / 3289308589 |
| 10 | 32 | 20 | 3 | 0 / 3360673294 |
| 11 | 30 | 20 | 3 | 0 / 3439091198 |
| 12 | 29 | 20 | 3 | 0 / 3551332904 |
| 13 | 29 | 20 | 3 | 0 / 3608917786 |
| 14 | 29 | 20 | 3 | 0 / 3764119345 |
| 15 | 30 | 20 | 3 | 0 / 3906787046 |
| 16 | 32 | 20 | 3 | 0 / 4022490350 |
| 17 | 32 | 20 | 3 | 0 / 4106515797 |
| 18 | 32 | 20 | 3 | 0 / 4247660882 |
| 19 | 32 | 20 | 3 | 0 / 4438563514 |
| 20 | 32 | 20 | 3 | 0 / 4531107228 |
| 21 | 32 | 20 | 3 | 0 / 4703714880 |
| 22 | 32 | 20 | 3 | 0 / 4818212431 |
| 23 | 32 | 20 | 3 | 0 / 4911777117 |
| 24 | 34 | 20 | 3 | 0 / 5017046055 |
| 25 | 34 | 20 | 3 | 0 / 5174495485 |
| 26 | 33 | 20 | 3 | 0 / 5294445267 |
| 27 | 33 | 20 | 3 | 0 / 5384598971 |
| 28 | 33 | 20 | 3 | 0 / 5614968312 |
| 29 | 33 | 20 | 3 | 0 / 5786449255 |
| 30 | 33 | 20 | 3 | 0 / 5980908128 |

Rakipler: en çok 33 etkin zincir; 4 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 2 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 2399663.22 TL; küsen farklı marka 9.

C3 kapanma nedenleri: 0 iflas/kapanma, 4 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 1967, tamamlanan 14; kutlama 24. Ritim koruyucusu: 13 sakin dönem olayı, 58 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | -1980218.83 | -2148086.66 | -2148086.66 | -168860.00 |
| durgünluk / 1 | 3280–3614 | 335 | -2149152.62 | -2504428.60 | -2504428.60 | -356341.94 |
| büyük salgın / 1 | 3674–4145 | 472 | -2575087.03 | -3167524.75 | -3167524.75 | -593644.72 |
| yüksek enflasyon / 1 | 4223–5075 | 853 | -3281508.94 | -4986593.04 | -4986593.04 | -1706600.61 |
| toparlanma / 1 | 5135–5499 | 365 | -5149408.32 | -6262889.43 | -6262889.43 | -1116333.33 |
| kur şoku / 2 | 7753–7936 | 184 | -16940479.89 | -18134658.22 | -18134658.22 | -1200662.48 |
| yüksek enflasyon / 2 | 7998–8543 | 546 | -18557761.11 | -22707247.14 | -22707247.14 | -4156802.01 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kademe değişmedi.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Reyon işletilmedi; kâr sıralaması için veri yok.

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 0. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 278.
- RejectBrandOffer: 366.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 1440.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 1440.
- Önce yerel pay %35.: 6.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

### Atak / tohum 21

- İlk şube: 79. gün.
- 5 mağaza: bu koşuda ulaşılmadı.
- İlk depo: bu koşuda ulaşılmadı.
- İlk il müdüru: bu koşuda ulaşılmadı.
- 5 il: bu koşuda ulaşılmadı.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 50938506,86 TL.
- Aile dükkânınin ücretleri: 10772398,50 TL.
- Üst yönetim ücretleri: 6820710,06 TL.
- İşletme giderleri (ücret hariç): 5054588,19 TL.
- Mal alımi (stok yatırımi): 68487,02 TL.

Arka planin kasaya toplam net etkisi: -59853121,30 TL. Reddedilen komut: 1.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -13383457,08 TL.
- Depo ve merkez: 0,00 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -10914052,88 TL.
- Internet satışi: 0,00 TL.
- Subeler: -50937975,56 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 30 | 20 | 4 | 32 / 2649343793 |
| 2 | 30 | 20 | 4 | 0 / 2742359338 |
| 3 | 30 | 20 | 4 | 0 / 2806522293 |
| 4 | 30 | 20 | 4 | 0 / 2851676281 |
| 5 | 30 | 20 | 4 | 0 / 2934839710 |
| 6 | 30 | 20 | 4 | 0 / 3037157017 |
| 7 | 30 | 20 | 4 | 0 / 3095215116 |
| 8 | 31 | 20 | 4 | 0 / 3168933777 |
| 9 | 31 | 20 | 4 | 0 / 3289308589 |
| 10 | 32 | 20 | 4 | 0 / 3360673294 |
| 11 | 30 | 20 | 4 | 0 / 3439091198 |
| 12 | 29 | 20 | 4 | 0 / 3551332904 |
| 13 | 29 | 20 | 4 | 0 / 3608917786 |
| 14 | 29 | 20 | 4 | 0 / 3764119345 |
| 15 | 30 | 20 | 4 | 0 / 3906787046 |
| 16 | 32 | 20 | 4 | 0 / 4022490350 |
| 17 | 32 | 20 | 4 | 0 / 4106515797 |
| 18 | 32 | 20 | 4 | 0 / 4247660882 |
| 19 | 32 | 20 | 4 | 0 / 4438563514 |
| 20 | 32 | 20 | 4 | 0 / 4531107228 |
| 21 | 32 | 20 | 4 | 0 / 4703714880 |
| 22 | 32 | 20 | 4 | 0 / 4818212431 |
| 23 | 32 | 20 | 4 | 0 / 4911777117 |
| 24 | 34 | 20 | 4 | 0 / 5017046055 |
| 25 | 34 | 20 | 4 | 0 / 5174495485 |
| 26 | 33 | 20 | 4 | 0 / 5294445267 |
| 27 | 33 | 20 | 4 | 0 / 5384598971 |
| 28 | 33 | 20 | 4 | 0 / 5614968312 |
| 29 | 33 | 20 | 4 | 0 / 5786449255 |
| 30 | 33 | 20 | 4 | 0 / 5980908128 |

Rakipler: en çok 33 etkin zincir; 4 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 1 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 1910132.72 TL; küsen farklı marka 10.

C3 kapanma nedenleri: 0 iflas/kapanma, 4 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 1970, tamamlanan 17; kutlama 28. Ritim koruyucusu: 20 sakin dönem olayı, 60 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | -2655899.64 | -2888087.93 | -2888087.93 | -235829.85 |
| durgünluk / 1 | 3280–3614 | 335 | -2889510.33 | -3372015.41 | -3372015.41 | -488359.53 |
| büyük salgın / 1 | 3674–4145 | 472 | -3466489.35 | -4276858.68 | -4276858.68 | -818225.28 |
| yüksek enflasyon / 1 | 4223–5075 | 853 | -4433845.49 | -6735969.92 | -6735969.92 | -2315431.04 |
| toparlanma / 1 | 5135–5499 | 365 | -6956747.02 | -8448842.41 | -8448842.41 | -1500716.19 |
| kur şoku / 2 | 7753–7936 | 184 | -22843303.84 | -24455436.15 | -24455436.15 | -1623171.20 |
| yüksek enflasyon / 2 | 7998–8543 | 546 | -25029593.78 | -30633511.93 | -30633511.93 | -5620832.74 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kademe değişmedi.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Reyon işletilmedi; kâr sıralaması için veri yok.

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 0. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 230.
- RejectBrandOffer: 470.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 2900.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Açılış için 10.626,26 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Açılış için 5.295,40 TL gerekiyor (depozito, tadilat, açılış stoğu).: 1.
- Tedarik asgarisi: 2900.
- Önce işletmenin borcunu kapat.: 1.
- Önce yerel pay %35.: 8.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

## Denetimin kapsamı

Satış fisindeki para, siparis bedeli, ana gün kapanisi ve mal kabul aktarimi bagimsiz hesapla kontrol edilir. Negatif stok, gecersiz sayilar ve pay sinirlari her gün denetlenir. Arka planin net kasa hareketi ayrica olculur; tek tek kalemlerin tam korunum denetimi B'nin muhasebe defteri C tarafindan baglandiginda tamamlanacak. Kasa eksisi oyun sonu degildir; sikinti günleri ayri sayilir. Ligler C bolumunde yillik olculur; B/C3 entegrasyonu oncesi bu koşu tam oyun dengesi degildir.

Fiyatlar normal oyuncunun kullandigi adimlarla degisir. Kredi, şube, depo, yonetici ve kararlar normal komutlardan gecer. Aile dükkânı PlayDay ile oynar; test modu, bedava mal veya para kullanilmaz. CSV tutarlari kurustur.
