# Otomatik oyuncunun denge raporu

3 koşu, her biri 3653 gün; toplam sure 491.5 saniye.

Tune: C10 defaults

| Tarz | Tohum | Son kasa | Borç | Magaza | İl | Ulusal pay | Kasa eksi gün | Sıkıntı günu | Denetim hatasi |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Temkinli | 21 | 31375979,60 TL | 0,00 TL | 119 | 48 | 0.5182% | 0 | 0 | 0 |
| Temkinli | 22 | 28552326,43 TL | 0,00 TL | 107 | 48 | 0.4842% | 0 | 0 | 0 |
| Temkinli | 23 | 28209192,49 TL | 0,00 TL | 107 | 48 | 0.5008% | 0 | 0 | 0 |

## Bulgular ve neye bakmalı

- 0/3 koşuda kasa eksiye düştü.
- 3/3 koşu sonunda birden cok mağaza acik kaldi.
- Satış, siparis, stok ve sayi denetimi: 0 hata.


### Temkinli / tohum 21

- İlk şube: 399. gün.
- 5 mağaza: 699. gün.
- İlk depo: 1231. gün.
- İlk il müdüru: 3046. gün.
- 5 il: 819. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 22283915,48 TL.
- İşletme giderleri (ücret hariç): 4414966,81 TL.
- Depo ve merkez giderleri: 3750951,88 TL.
- Üst yönetim ücretleri: 3360305,72 TL.
- Mal alımi (stok yatırımi): 2657549,52 TL.

Arka planin kasaya toplam net etkisi: 69831948,30 TL. Reddedilen komut: 157.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -3324924,64 TL.
- Depo ve merkez: -3750951,88 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -4580584,57 TL.
- Internet satışi: 123086,83 TL.
- Subeler: 86292892,24 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 1 | 248169 / 2649343793 |
| 2 | 28 | 20 | 5 | 766683 / 2742359338 |
| 3 | 27 | 20 | 9 | 1721100 / 2806522293 |
| 4 | 23 | 20 | 16 | 3841978 / 2851676281 |
| 5 | 14 | 20 | 28 | 8751062 / 2934839710 |
| 6 | 10 | 20 | 40 | 16887961 / 3037157017 |
| 7 | 9 | 20 | 52 | 32433534 / 3095215116 |
| 8 | 9 | 20 | 65 | 41145358 / 3168933777 |
| 9 | 8 | 20 | 83 | 47882316 / 3289308589 |
| 10 | 8 | 24 | 119 | 61236800 / 3360673294 |

Rakipler: en çok 111 etkin zincir; 6 farklı satılık zincir; 4 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 204 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8845 mağaza · senden çekildiği savaş: 54.
Markalardan toplam 945284.28 TL; küsen farklı marka 6.

C3 kapanma nedenleri: 0 iflas/kapanma, 4 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 705, tamamlanan 275; kutlama 551. Ritim koruyucusu: 1 sakin dönem olayı, 13 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 3105–3279 | 175 | 23978123.51 | 25337795.83 | 22521347.23 | 5127292.02 |
| durgünluk / 1 | 3280–3614 | 335 | 25266715.41 | 30153194.88 | 24646662.54 | 12473313.84 |
| büyük salgın / 1 | 3674–4145 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 1 | 4223–5075 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| toparlanma / 1 | 5135–5499 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 7753–7936 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7998–8543 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 665. gün 0 -> 1.
- Süt ürünleri: 1085. gün 0 -> 1.
- İçecek: 1145. gün 0 -> 1.
- Temizlik ve bakım: 1145. gün 0 -> 1.
- Kuru gıda: 1385. gün 1 -> 2.
- Süt ürünleri: 1745. gün 1 -> 2.
- Temizlik ve bakım: 1805. gün 1 -> 2.
- İçecek: 1865. gün 1 -> 2.
- Kuru gıda: 2105. gün 2 -> 3.
- Süt ürünleri: 2945. gün 2 -> 3.
- İçecek: 3065. gün 2 -> 3.
- Temizlik ve bakım: 3125. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -7042.10 / 2829.92 (zarar görüldü).
- Tür 2, Manav: -21.34 / 66272.47 (zarar görüldü).
- Tür 2, Kasap: -119.08 / 76239.09 (zarar görüldü).
- Tür 2, Evcil hayvan: 13.75 / 12685.78.
- Tür 2, Balık: -2973.47 / -128.83 (zarar görüldü).
- Tür 3, Manav: 61.94 / 446953.47.
- Tür 3, Kasap: 81.68 / 649955.42.
- Tür 3, Bebek: -72991.18 / -1700.58 (zarar görüldü).
- Tür 3, Evcil hayvan: 505.35 / 82598.19.
- Tür 3, Bahçe ve oto: -12239.05 / 12815.98 (zarar görüldü).
- Tür 3, Mevsimlik: -16280.74 / 106977.90 (zarar görüldü).
- Tür 3, Şarküteri: -150.75 / 271069.11 (zarar görüldü).
- Tür 3, Fırın ve pastane: -783.09 / 291754.21 (zarar görüldü).
- Tür 3, Balık: -12528.18 / 70717.12 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -51572.31 / 374124.15 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -24983.13 / 194282.69 (zarar görüldü).
- Tür 3, Ev ve mutfak: -2842.84 / 84439.92 (zarar görüldü).
- Tür 3, Oyuncak: -36163.81 / -2311.22 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -23569.36 / 191641.28 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 50. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 61.
- RejectBrandOffer: 181.
- ReplaceMasters: 30.
- SetDepartment: 44.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 119.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 119.
- Önce yerel pay %35.: 8.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 3.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 4; borç sınırı ihlali 0 ay; şirket faizi 0,00 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 1, yabancı kapı kolu 1.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 4 deneme / 4 başarı.

#### C5: internet

Salgın 3676..4153. İl önerileri 20, onay 20, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 1576; salgına uzaklık: -2100 gün.
- Telefon uygulaması ilk açılış: 3129; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 2311; salgına uzaklık: -1365 gün.
- Hızlı teslimat ilk açılış: 0; salgına uzaklık: 0 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 4 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 5 | 0.65 | 0.24 | 587 | -10257.99 | 0 | 0 | 0 | 0 | 0.995 |
| 6 | 0.91 | 0.21 | 1368 | -9938.81 | 0 | 0 | 0 | 0 | 0.992 |
| 7 | 1.17 | 0.12 | 3598 | -5233.24 | 0 | 0 | 0 | 0 | 0.990 |
| 8 | 1.44 | 0.11 | 6487 | 8717.79 | 0 | 0 | 0 | 0 | 0.987 |
| 9 | 1.90 | 0.13 | 8748 | 30996.25 | 0 | 0 | 0 | 0 | 0.984 |
| 10 | 2.41 | 0.20 | 10667 | 108802.83 | 0 | 0 | 0 | 0 | 0.978 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 37 (onay 37 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 6 ürün işlemi, 6 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

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

Plan 0, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 0. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Temkinli / tohum 22

- İlk şube: 459. gün.
- 5 mağaza: 759. gün.
- İlk depo: 1291. gün.
- İlk il müdüru: 3102. gün.
- 5 il: 879. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 20688530,63 TL.
- İşletme giderleri (ücret hariç): 4291756,43 TL.
- Depo ve merkez giderleri: 3675003,51 TL.
- Üst yönetim ücretleri: 3135389,63 TL.
- Mal alımi (stok yatırımi): 2573870,66 TL.

Arka planin kasaya toplam net etkisi: 61907105,25 TL. Reddedilen komut: 154.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -3253998,85 TL.
- Depo ve merkez: -3675003,51 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -4315151,74 TL.
- Internet satışi: 590839,46 TL.
- Subeler: 77977833,45 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 28 | 20 | 1 | 242831 / 2585369432 |
| 2 | 28 | 20 | 4 | 638062 / 2664411879 |
| 3 | 27 | 20 | 9 | 1575701 / 2739396267 |
| 4 | 23 | 20 | 14 | 3193047 / 2818767366 |
| 5 | 16 | 20 | 26 | 7696041 / 2940529417 |
| 6 | 10 | 20 | 38 | 14480916 / 3007185519 |
| 7 | 9 | 20 | 50 | 26048984 / 3127550970 |
| 8 | 9 | 24 | 63 | 36333279 / 3215860338 |
| 9 | 9 | 32 | 77 | 45101049 / 3285197661 |
| 10 | 8 | 31 | 107 | 57196021 / 3425491994 |

Rakipler: en çok 127 etkin zincir; 8 farklı satılık zincir; 6 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 198 fiyat savaşı.
Ezeli rakip: Ezeli rakip: ŞAK (Deniz Arslan) · 3493 mağaza · senden çekildiği savaş: 39.
Markalardan toplam 818359.44 TL; küsen farklı marka 5.

C3 kapanma nedenleri: 0 iflas/kapanma, 6 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 713, tamamlanan 299; kutlama 551. Ritim koruyucusu: 1 sakin dönem olayı, 18 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 2306–2480 | 175 | 3703955.20 | 5191606.48 | 3526417.80 | 2345392.68 |
| durgünluk / 1 | 2481–2814 | 334 | 5221120.74 | 9832106.36 | 5221120.74 | 6253515.48 |
| büyük salgın / 1 | 2874–3346 | 473 | 11137059.56 | 19471424.77 | 8751859.69 | 16391911.98 |
| yüksek enflasyon / 1 | 3424–4275 | 230 | 21980695.14 | 28552326.43 | 21664715.92 | 10316359.03 |
| toparlanma / 1 | 4335–4699 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6953–7136 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 7198–7744 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 665. gün 0 -> 1.
- Süt ürünleri: 1025. gün 0 -> 1.
- İçecek: 1145. gün 0 -> 1.
- Temizlik ve bakım: 1145. gün 0 -> 1.
- Kuru gıda: 1445. gün 1 -> 2.
- Süt ürünleri: 1805. gün 1 -> 2.
- İçecek: 1865. gün 1 -> 2.
- Temizlik ve bakım: 1865. gün 1 -> 2.
- Kuru gıda: 2105. gün 2 -> 3.
- Süt ürünleri: 3125. gün 2 -> 3.
- İçecek: 3365. gün 2 -> 3.
- Temizlik ve bakım: 3365. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -22521.29 / 2614.45 (zarar görüldü).
- Tür 2, Manav: -27.42 / 70849.19 (zarar görüldü).
- Tür 2, Kasap: -56.64 / 79594.98 (zarar görüldü).
- Tür 2, Evcil hayvan: 11.57 / 13621.10.
- Tür 2, Balık: -2260.20 / -86.46 (zarar görüldü).
- Tür 3, Manav: -12.66 / 461115.04 (zarar görüldü).
- Tür 3, Kasap: 55.70 / 647946.14.
- Tür 3, Bebek: -49611.88 / -1053.88 (zarar görüldü).
- Tür 3, Evcil hayvan: 403.19 / 85751.43.
- Tür 3, Bahçe ve oto: -11432.22 / 79345.10 (zarar görüldü).
- Tür 3, Mevsimlik: -43218.21 / 55317.07 (zarar görüldü).
- Tür 3, Şarküteri: -26.94 / 269160.66 (zarar görüldü).
- Tür 3, Fırın ve pastane: -628.02 / 290456.08 (zarar görüldü).
- Tür 3, Balık: -18748.93 / 57152.74 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -8191.08 / 630341.72 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -8709.30 / 63077.21 (zarar görüldü).
- Tür 3, Ev ve mutfak: -4977.56 / 108461.94 (zarar görüldü).
- Tür 3, Oyuncak: -26126.75 / 151664.86 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -23962.10 / 194462.70 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 44. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 59.
- RejectBrandOffer: 183.
- ReplaceMasters: 29.
- SetDepartment: 59.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 117.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 117.
- Önce yerel pay %35.: 9.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 2.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 4; borç sınırı ihlali 0 ay; şirket faizi 0,00 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 6, yabancı kapı kolu 4.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 4 deneme / 4 başarı.

#### C5: internet

Salgın 2887..3324. İl önerileri 21, onay 21, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 792; salgına uzaklık: -2095 gün.
- Telefon uygulaması ilk açılış: 2340; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1520; salgına uzaklık: -1367 gün.
- Hızlı teslimat ilk açılış: 3158; salgına uzaklık: 271 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.00 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.42 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 3 | 0.69 | 0.51 | 177 | 2396.00 | 0 | 0 | 0 | 0 | 0.994 |
| 4 | 0.95 | 0.34 | 253 | -24091.08 | 0 | 0 | 0 | 0 | 0.992 |
| 5 | 1.22 | 0.36 | 993 | -16976.84 | 0 | 0 | 0 | 0 | 0.989 |
| 6 | 1.48 | 0.36 | 2527 | 949.17 | 0 | 0 | 0 | 0 | 0.987 |
| 7 | 2.01 | 0.30 | 6278 | 25839.99 | 0 | 0 | 0 | 0 | 0.983 |
| 8 | 8.51 | 0.37 | 10982 | 88784.34 | 0 | 36 | 1848 | 604685 | 0.967 |
| 9 | 6.08 | 0.67 | 19530 | 320211.32 | 2 | 366 | 19530 | 6557612 | 0.756 |
| 10 | 8.00 | 0.43 | 18869 | 193726.56 | 11 | 35 | 2218 | 636782 | 0.927 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 28 (onay 28 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 3 ürün işlemi, 3 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

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

Plan 0, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 0. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

### Temkinli / tohum 23

- İlk şube: 459. gün.
- 5 mağaza: 759. gün.
- İlk depo: 1291. gün.
- İlk il müdüru: 3102. gün.
- 5 il: 909. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Vergi ödemeleri: 21301103,56 TL.
- İşletme giderleri (ücret hariç): 4519507,93 TL.
- Depo ve merkez giderleri: 3896369,89 TL.
- Üst yönetim ücretleri: 3166428,28 TL.
- Mal alımi (stok yatırımi): 2657750,72 TL.

Arka planin kasaya toplam net etkisi: 66206126,95 TL. Reddedilen komut: 153.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -3464202,66 TL.
- Depo ve merkez: -3896369,89 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -4374616,29 TL.
- Internet satışi: 864544,28 TL.
- Subeler: 79009815,41 TL.

#### C: yeni sistemlerin sonucu

| Yıl | Ulusal sıra | Dünya sırası | Mağaza | Bizim / liderin ortak cirosu |
|---:|---:|---:|---:|---:|
| 1 | 29 | 20 | 1 | 223405 / 2616019729 |
| 2 | 27 | 20 | 4 | 643325 / 2664104721 |
| 3 | 27 | 20 | 9 | 1501328 / 2766262382 |
| 4 | 26 | 20 | 14 | 3220105 / 2881109386 |
| 5 | 14 | 20 | 26 | 7824503 / 2996229747 |
| 6 | 10 | 20 | 38 | 13571219 / 3089990654 |
| 7 | 9 | 20 | 50 | 25998663 / 3158913001 |
| 8 | 9 | 27 | 63 | 31490159 / 3214436872 |
| 9 | 9 | 27 | 77 | 46562630 / 3297649008 |
| 10 | 8 | 27 | 107 | 59173544 / 3417996779 |

Rakipler: en çok 128 etkin zincir; 3 farklı satılık zincir; 3 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 210 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 8717 mağaza · senden çekildiği savaş: 64.
Markalardan toplam 848648.02 TL; küsen farklı marka 6.

C3 kapanma nedenleri: 0 iflas/kapanma, 3 rakip tarafından alınma, 0 bizim alımımız. Haber sayısı alt sınırdır; nedenler doğrudan sistem sayaçlarından gelir.

Defter denetimi: açıklanamayan fark 0 gün, toplam 0.00 TL; mutlak fark toplamı 0.00 TL.
Hedefler: gözlenen 721, tamamlanan 302; kutlama 561. Ritim koruyucusu: 1 sakin dönem olayı, 16 ertelenen kötü olay; eşiği aşan 0 sıkıcı dönem, en uzun sessizlik 19 gün.

Dönemler (günler kampanya başlangıcından; kâr defterden; kasalar ilk/son gün kapanışı, TL):

| Dönem / dalga | Planlanan günler | Oynanan gün | İlk kasa | Son kasa | En az kasa | Net kâr |
|---|---|---:|---:|---:|---:|---:|
| kur şoku / 1 | 1967–2141 | 175 | 2116963.31 | 2331485.38 | 2105892.86 | 999397.44 |
| durgünluk / 1 | 2142–2475 | 334 | 2346461.96 | 5260805.47 | 2335141.39 | 3804397.21 |
| büyük salgın / 1 | 2535–3006 | 472 | 5934224.02 | 11086028.94 | 5934224.02 | 8733506.29 |
| yüksek enflasyon / 1 | 3084–3936 | 570 | 10388768.58 | 28209192.49 | 10388768.58 | 26281957.82 |
| toparlanma / 1 | 3996–4360 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| kur şoku / 2 | 6614–6797 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |
| yüksek enflasyon / 2 | 6859–7404 | 0 | 0.00 | 0.00 | 0.00 | 0.00 |

Tedarik kademe değişimleri (hat, gün, önce, sonra):
- Kuru gıda: 665. gün 0 -> 1.
- Süt ürünleri: 1085. gün 0 -> 1.
- Temizlik ve bakım: 1145. gün 0 -> 1.
- İçecek: 1205. gün 0 -> 1.
- Kuru gıda: 1385. gün 1 -> 2.
- Süt ürünleri: 1805. gün 1 -> 2.
- İçecek: 1865. gün 1 -> 2.
- Temizlik ve bakım: 1865. gün 1 -> 2.
- Kuru gıda: 2105. gün 2 -> 3.
- Süt ürünleri: 3065. gün 2 -> 3.
- İçecek: 3305. gün 2 -> 3.
- Temizlik ve bakım: 3305. gün 2 -> 3.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -28079.06 / 2744.79 (zarar görüldü).
- Tür 2, Manav: 5.70 / 76735.62.
- Tür 2, Kasap: -262.81 / 91890.44 (zarar görüldü).
- Tür 2, Evcil hayvan: 17.19 / 15671.87.
- Tür 2, Balık: -2111.04 / -128.06 (zarar görüldü).
- Tür 3, Manav: -98.37 / 496588.83 (zarar görüldü).
- Tür 3, Kasap: 55.64 / 739985.35.
- Tür 3, Bebek: -43005.11 / -1088.10 (zarar görüldü).
- Tür 3, Evcil hayvan: 332.87 / 98521.66.
- Tür 3, Bahçe ve oto: -13823.99 / 36004.52 (zarar görüldü).
- Tür 3, Mevsimlik: -30655.34 / 88344.10 (zarar görüldü).
- Tür 3, Şarküteri: -281.94 / 305860.83 (zarar görüldü).
- Tür 3, Fırın ve pastane: -625.41 / 331145.10 (zarar görüldü).
- Tür 3, Balık: -46312.65 / 63044.41 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -9126.95 / 734559.56 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -48724.73 / 10068.14 (zarar görüldü).
- Tür 3, Ev ve mutfak: -5970.74 / 120308.44 (zarar görüldü).
- Tür 3, Oyuncak: -32651.50 / -685.84 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -28044.64 / 215416.92 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 0 / 50. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 63.
- RejectBrandOffer: 179.
- ReplaceMasters: 26.
- SetDepartment: 67.
- SetDeptStance: 14.
- SetSourcing: 12.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 115.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 115.
- Önce yerel pay %35.: 8.
- Üçüncü mağaza için önce bir İK müdürü işe al.: 2.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

#### C4: banka ve ilk 180 gün

Kurtarma 0; kapanan şube 4; borç sınırı ihlali 0 ay; şirket faizi 0,00 TL; en yüksek limit kullanımı 0,00 TL.
Teklif 0, kabul 0, ret 0, krediyle alım 0, çevrilen mağaza 0, devlerin satılık kolu 3, yabancı kapı kolu 2.
Kurtarma günleri:

Şube dökümü sube180.csv; yıllık banka durumu banka.csv. Eksik 180 günler kapanma/koşu sonu nedeniyle ayrı okunmalı; yatırım ve stok alımı net faaliyet kârına dahil değil. Faiz şirket bankasının toplamıdır; aile kredisi ayrıdır.
- CloseBranch: 4 deneme / 4 başarı.

#### C5: internet

Salgın 2540..3010. İl önerileri 27, onay 27, ret 0. Kanallar normal oyuncu komutlarıyla açılır.
- Web sitesi ilk açılış: 463; salgına uzaklık: -2077 gün.
- Telefon uygulaması ilk açılış: 1993; salgına uzaklık: -547 gün.
- HızlıSepet ilk açılış: 1170; salgına uzaklık: -1370 gün.
- Hızlı teslimat ilk açılış: 2815; salgına uzaklık: 275 gün.

| Yıl | Ülke internet % | Bizim ciroda internet % | Sipariş | Net TL | Depo | Salgın gün | Salgın sipariş | Salgın müşteri | Trafik çarpanı ort. |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.41 | 0.00 | 0 | 0.00 | 0 | 0 | 0 | 0 | 1.000 |
| 2 | 0.67 | 0.42 | 59 | -115.40 | 0 | 0 | 0 | 0 | 0.995 |
| 3 | 0.94 | 0.40 | 134 | 1207.77 | 0 | 0 | 0 | 0 | 0.992 |
| 4 | 1.20 | 0.47 | 411 | -19027.83 | 0 | 0 | 0 | 0 | 0.989 |
| 5 | 1.47 | 0.43 | 1206 | -14245.99 | 0 | 0 | 0 | 0 | 0.987 |
| 6 | 1.98 | 0.52 | 3593 | 12022.73 | 0 | 0 | 0 | 0 | 0.983 |
| 7 | 8.51 | 0.50 | 8842 | 62620.98 | 0 | 18 | 855 | 227627 | 0.979 |
| 8 | 6.08 | 1.01 | 18539 | 296992.47 | 0 | 365 | 18539 | 4858641 | 0.757 |
| 9 | 7.70 | 0.58 | 17670 | 269719.22 | 2 | 87 | 4598 | 1333477 | 0.894 |
| 10 | 9.84 | 0.46 | 18845 | 255370.33 | 12 | 0 | 0 | 0 | 0.960 |

Kanal kârı mal ve sipariş masrafı sonrası katkıdır; ortak web/uygulama/depo/reklam/müdür gideri ayrı sütunda, toplam netten çıkar. Kanalı olmayan günde de ortak gider kaybolmaz. Ciro payı fiziksel aile+şubelerin yıllık cirosundandır; bağlı şirket agregası hariç. Ülke payı yıl sonu; trafik çarpanı gün ortalaması. Kurulum yatırımı faaliyet netine eklenmez. Açılış 0 = hiç açılmadı. Ayrıntı internet.csv ve internet_olaylar.csv.

#### C6: reklam ve komuta

Açma önerisi 27 (onay 27 / ret 0), kapama 0 (onay 0 / ret 0). Stok eritme: 6 ürün işlemi, 6 ayrı gün. Reklam müdürü gideri 0.00 TL (iç birim).

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

Plan 0, tamamen odenen 0, yeni planla degisen 0; silinen kredi borcu 0 kurus. Yeni kredi/şube denenmeyen plan günu 0. kurtarma.csv gün, ilk yeni şube, kalan plan ve borc silme; kurtarma_yillar.csv toplam borc ve aile dükkânınin tam oyun yili cirosu. Silinen borc kasaya gelir degildir; plan degismesi ödeme sayilmaz.

## Denetimin kapsamı

Satış fisi, siparis bedeli, gün kapanisi ve mal kabul aktarimi bagimsiz hesapla kontrol edilir. Negatif stok, gecersiz sayilar ve pay sinirlari her gün denetlenir. Bagli muhasebe defterinin kasa farkı hem isaretli hem mutlak toplamla C bolumunde verilir. Kasa eksisi oyun sonu degildir. Ligler yillik, şubeler ilk 180 günun gercek defter satirlariyla olculur.

Fiyatlar normal oyuncunun kullandigi adimlarla degisir. Kredi, şube, depo, yonetici ve kararlar normal komutlardan gecer. Aile dükkânı PlayDay ile oynar; test modu, bedava mal veya para kullanilmaz. CSV tutarlari kurustur.
