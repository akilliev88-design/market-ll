# A6 — Son uzun koşuların değerlendirmesi (01.10.2026)

**Kod çalışıyor; denge hedefi karşılanmıyor.** C kaynaklarında derleme/test düzeltmesi gerekmedi (0). M27 sürüm kapısı ve C komutlarını kullanan bot hazır; DERLE, TEST **105/105** ve Smoke geçti. Kayıt sürümü **2** kaldı; artış C3'te. Kaynak: `d190bfe` (ikisinde aynı derlenmiş kaynak), başlangıç TR/Kırklareli, yalnız oyuncu komutları ve PlayDay; oyuncu kaydı/test modu/para veya stok armağanı yok. B/B7 ve C3 henüz birleşmedi.

Son 30 yıllık koşu: `Saved/AutoPlay/20261001-090217` (523,7 sn, 3 × 10.958 gün, tohum 21). Son 10 yıllık koşu: `Saved/AutoPlay/20261001-090707` (279,6 sn, 9 × 3.653 gün, tohum 21–23). Toplam **12 kampanya / 65.751 gün / 0 denetim hatası**; iki commandlet çıkışı 0. Bu, para/stok/sayı denetiminin geçmesidir; C'nin her giderinin bağımsız defter denetimi değildir (B/C3 bağlaması bekleniyor).

| Dengeli, tohum 21 | 10. yıl | 20. yıl | 30. yıl |
|---|---:|---:|---:|
| Ulusal sıra | 74 | 63 | 60 |
| Dünya sırası | 20 | 20 | 20 |
| Açık mağaza | 151 | 151 | 151 |

Dengelinin kasası ilk kez 2.640. gün eksiye düştü; son iki yıllık örnekte ortak ciro 0. Temkinli ilk şubeyi 399. günde açtı; 10. yılda üç tohumda ulusal 8., dünya 20., 107–108 mağaza ve artı kasa. 30 yıllık tohumda 6.371. günden sonra o da açık verdi. Atak ilk açık 435. gün; 30 yılda 15 mağaza. Üç tohumlu 10 yıl: dengeli 29–74., atak 29–34. ulusal; bütün tarz/tohumlarda dünya 20. Dengeli/atak 6/6 koşuda açık verdi; temkinli 3/3 koşuda ilk 10 yıl artıda kaldı. “Ulusal ilk 3 / dünya ilk 10 / birincilik” hedeflerinin hiçbiri sağlanmadı.

**Reyon sıralamasının ölçüsü:** 30 yıllık koşunun bütün günlerinde görülen, aynı mağaza türündeki şubelerin toplam `Last30Profit` uçları (nominal TL; açılış günleri dahil). En büyük zarar: hiper elektronik **−1.092.495,88**, hiper evcil **−934.376,36**, hiper manav **−748.669,14**. En yüksek kâr: hiper kasap **5.899.083,07**, hiper manav **4.117.632,66**, hiper giyim **1.987.648,33**. Bunlar farklı şube sayıları/tarihlerdeki toplam uçlardır; tek şube ya da toplam kampanya kârı diye okunmaz. C'nin `Last30Profit` hesabı 29/30 sönümlü yaklaşık toplamdır, tam kayan 30 kayıt toplamı değildir.

Daha karşılaştırılabilir yapısal kontrol: aylık örneklerde balık süperde **3/3**, hiperde **15/15**, hiper evcil **23/23**, hiper elektronik **9/9** zarar. Örnek kâr/ciro oranları sırasıyla −%143,10 / −%98,53 / −%63,93 / −%19,48. Süper evcilde **695 örnekte sıfır zarar**, toplam örnek kâr/ciro %33,35: hiperde küçük reyonun bir tam personel maliyeti dikkat çekiyor. Kasap ve manavın normal işletme örnekleri çoğunlukla artıda; son negatif uçları ağın nakitsiz kaldığı dönem de büyütüyor. Bütün mağaza türleri/reyonlar aşağıdaki ham C bölümünde; CSV özetinde şube başına tutarlar da var.

Tedarik: 30 yılda temkinli/dengeli 12 yükseliş + 12 düşüş, atak 5 + 5; son değişimler 6.545 / 2.730 / 480. gün. Tam tarihler `tedarik.csv` ve aşağıda. Markalardan 30 yıl toplam temkinli **23.291.816,31 TL**, dengeli **2.901.902,19 TL**, atak **2.210.812,64 TL**; küsen marka 11 / 11 / 10. Yeni 30 yıl koşusunda bizim zincir alımımız **0**: satışlar geldiğinde kasa yetmedi. Önceki, yer seçimi hatalı pilotta normal `BuyChain` yoluyla dengeli ve atak 6'şar alım yapmıştı; bunlar son koşuya karıştırılmadı.

30 yılda etkin zincir tepe sayısı 105 / 92 / 49, ayrı satılık zincir 35 / 29 / 5, piyasadan çekilme 35 / 27 / 5; fiyat savaşı 457 / 468 / 48. Ezeli rakip temkinli ve dengelide BİN/Haluk Sezer, atakta yok. **Gerçek iflas toplamı elde edilemiyor:** C'de kapanma nedeni bayrağı yok; görünür iflas haberi 0, haber tavanı var. “35 kapanma = 35 iflas” denmedi. C3'e istek: kapanma nedeni ve kalıcı olay sayacı ekleyin.

Sıkıcı dönem proxy sayımı bütün koşularda 0: mahalle olayları 31 gün sessizlik bırakmıyor. Bu, oyuncunun sıkılmadığını ispatlamaz. 30 yıl felaket kümeleri, mahalle olayları / C savaşları da dahil: temkinli **56/141**, dengeli **56/149**, atak **56/64**. Aynı kampanyanın iki gözlemidir, eski kaynakla ayrı yeniden koşu değildir; farklı illerdeki eşzamanlı savaşlar şirket düzeyinde sayılır. B'nin dönem olayları/akış bütçesi henüz yok.

**Botun sınırları:** zararlı olgun reyonu kapatır, zayıf ustayı değiştirir ve tedarikten geri iner. Zarar eden şubeyi otomatik kapatmaz, küsen marka için raf karışımını otomatik değiştirmez, işletme nakit yedeği aile dükkânı giderlerinden hesaplanır. Bu yüzden nakitsiz ağın yıllarca gider yazması iyi bir insan oyuncunun zorunlu sonucu değildir. Sayısal öneriler başlangıç denemeleridir; bu koşu C'nin tek bir sabitini suçlayan kontrollü deney değildir. C3 sonrasında şube giderlerini kapsayan bot yedeği/zararlı şube tasfiyesi ve B7 etkileriyle yeni koşu gerekir.

## C'ye ayar önerileri — öncelik sırası, henüz uygulanmadı

1. **Hiper sabit yükünü ilk denemede küçültün:** `MarketBranches::FormatInfo(hiper)` Workers **20 → 14**, Running **5 → 3**. Dengeli üç tohumda da ilk 10 yıl içinde nakitsiz kaldı; atak tohum 21 hiper sonrasında 435. günde açık verdi. Açılışın 30 günlük alışma dönemini ve bölüm çalışanlarının ek ücretini hesaba katın. Önce bu deneme + şube bazında gider/kâr dökümü; tüm sorunu bu iki sayı çözer diye varsayılmıyor.
2. **Sürekli zarar eden üç reyonu ayrı sınayın:** balık Ratio **0,04 → 0,08**, Margin **0,28 → 0,34**; elektronik Ratio **0,14 → 0,24**, Margin **0,11 → 0,20**; hiper evcil için `MarketDepartmentsLocal::Staff` **1 → 0** (mevcut kat görevlisiyle paylaşım, süperdeki gibi). Balık/evcil/elektroniğin ilgili aylık örneklerinin tamamı zarar; süper evcil %33,35 örnek kâr/ciro ile farklı. Bunlar tek seferde değil, reyon başına ayrı denenecek adaylar. Manav/kasap Margin'ini artırma önerilmedi: normal örnekler zaten olumlu.
3. **Rekabet/lig ölçeğini nakit düzeldikten sonra birlikte küçültmeyi deneyin:** ilk aday `LeagueCompression` **0,04 → 0,004** ve ulusal roster başlangıç mağazaları **×0,1** (BİN 3.500 → 350, A110 1.900 → 190, vb.; Config/zincirler.json ve yerel tablo aynı kaynaktan). Devler ile ulusal zincirler aynı ölçekte değil: Compression sadece devleri küçültür, ulusal zincirlerin dünya satırını küçültmez. Son dengeli ortak ciro 0 olduğundan bu koşudan “doğru katsayı” kestirilemez; öneri hedef ilk 3 / ilk 10 / 30. yıl için sınanacak aralıktır, doğrulanmış ayar değildir. Temkinli 10. yıl 68,8 milyon ortak ciroya karşı lider 3,36 milyar; 0,04 tek başına ulaşılabilir yarış üretmedi.
4. **Savaş yükünü azaltan deneme:** `WarDays` **45 → 21**, `WarPressure` **1,30 → 1,10**, aynı ilde yeni savaş öncesi en az **60 gün** dinlenme. Dengeli 468 savaş / 30 yıl ve şirket düzeyinde kümelerin 56 → 149 çıkması; olaylar aylık tura yığılabiliyor. B'nin akış bütçesine tek il/öncelik üzerinden bağlayın; farklı illerin bütün savaşlarını ayrı acil seçim gibi göstermeyin.
5. **Marka teklif tavanı 3 → 2:** `MaxOpenOffers`. Temkinli 266 kabul / 747 ret, dengeli 256 / 763, atak 302 / 564; dengelide teklifler yaklaşık %75 oranında reddedildi. Bot raf karışımı değiştirmediği için bu oran teklif kuralının tek başına hatalı olduğu kanıtı değildir; yine de daha az eşzamanlı düşük değerli karar denenebilir. Aylık ödeme takvimini değiştirmeyin. Tedarik asgarilerini şimdilik **15.000 / 60.000 / 200.000 başlangıç TL** bırakın: yükselişler gerçekleşti, kasa çökünce alım 0'a indi; asgariyi düşürmek bu sorunu çözmez.

Ek C3 istekleri: B7 muhasebe/dönem etkilerini bağla; iflas/kapanma nedeni sayacını kalıcı tut; M27 sürümünü yalnız bir kez artır; menü yuvasında uyumsuz sürüm cümlesi; “Gez” → StartBranchVisit ve A3 zaman bağlantıları. A5 gerçek ağla 68/68 geçti: `Saved/Screenshots/Menu/20261001-083810/index.html`, 11 şube / 3 müdür / 1 depo, örnek ağ yok. Beş menü kusuru A.md'de; menüye dokunulmadı.

---

## Ham otomatik rapor

# Otomatik oyuncunun denge raporu

3 koşu, her biri 10958 gün; toplam sure 523.7 saniye.

| Tarz | Tohum | Son kasa | Borç | Magaza | İl | Ulusal pay | Kasa eksi gün | Sıkıntı günu | Denetim hatasi |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Temkinli | 21 | -10000367720,77 TL | 4969970,98 TL | 198 | 48 | 9.0123% | 4530 | 4530 | 0 |
| Dengeli | 21 | -9571438317,00 TL | 868952,75 TL | 151 | 40 | 6.8730% | 8318 | 8318 | 0 |
| Atak | 21 | -506377882,93 TL | 3109385,95 TL | 15 | 13 | 0.6827% | 10524 | 10524 | 0 |

## Bulgular ve neye bakmalı

- 3/3 koşuda kasa eksiye düştü.
- 3/3 koşu birden cok mağazaya ulaştı.
- Satış, siparis, stok ve sayi denetimi: 0 hata.


### Temkinli / tohum 21

- İlk şube: 399. gün.
- 5 mağaza: 489. gün.
- İlk depo: 721. gün.
- İlk il müdüru: 2444. gün.
- 5 il: 519. gün.
- Ikinci ulke: bu koşuda ulaşılmadı.

En büyük 5 yuk (nakit gideri, stok yatırımi ve fire ayri anlam tasir):

- Zararli şubeler (net zarar): 9440262469,24 TL.
- Üst yönetim ücretleri: 473847377,91 TL.
- Depo ve merkez giderleri: 281384009,93 TL.
- Vergi ödemeleri: 235188290,05 TL.
- İşletme giderleri (ücret hariç): 68509527,81 TL.

Arka planin kasaya toplam net etkisi: -9560815763,32 TL. Reddedilen komut: 389.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -65880925,16 TL.
- Depo ve merkez: -281384009,93 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -475726937,42 TL.
- Internet satışi: 0,00 TL.
- Subeler: -8796642033,82 TL.

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
| 11 | 8 | 20 | 119 | 75627972 / 3439091198 |
| 12 | 8 | 20 | 130 | 83348535 / 3551332904 |
| 13 | 7 | 19 | 143 | 90731264 / 3608917786 |
| 14 | 7 | 19 | 155 | 98540860 / 3764119345 |
| 15 | 7 | 19 | 167 | 107638721 / 3906787046 |
| 16 | 7 | 19 | 179 | 112887611 / 4022490350 |
| 17 | 7 | 19 | 191 | 108715564 / 4106515797 |
| 18 | 22 | 20 | 198 | 5263375 / 4247660882 |
| 19 | 70 | 20 | 198 | 18657 / 4438563514 |
| 20 | 70 | 20 | 198 | 18731 / 4531107228 |
| 21 | 71 | 20 | 198 | 18268 / 4703714880 |
| 22 | 71 | 20 | 198 | 18662 / 4818212431 |
| 23 | 70 | 20 | 198 | 18168 / 4911777117 |
| 24 | 72 | 20 | 198 | 18242 / 5017046055 |
| 25 | 71 | 20 | 198 | 18260 / 5174495485 |
| 26 | 70 | 20 | 198 | 18095 / 5294445267 |
| 27 | 71 | 20 | 198 | 18274 / 5384598971 |
| 28 | 68 | 20 | 198 | 18947 / 5614968312 |
| 29 | 69 | 20 | 198 | 18626 / 5786449255 |
| 30 | 68 | 20 | 198 | 18397 / 5980908128 |

Rakipler: en çok 105 etkin zincir; 35 farklı satılık zincir; 35 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 457 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 9193 mağaza · senden çekildiği savaş: 82.
Markalardan toplam 23291816.31 TL; küsen farklı marka 11.

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
- İçecek: 6485. gün 3 -> 2.
- Süt ürünleri: 6485. gün 3 -> 2.
- Kuru gıda: 6485. gün 3 -> 2.
- Temizlik ve bakım: 6485. gün 3 -> 2.
- İçecek: 6541. gün 2 -> 1.
- Süt ürünleri: 6541. gün 2 -> 1.
- Kuru gıda: 6541. gün 2 -> 1.
- Temizlik ve bakım: 6541. gün 2 -> 1.
- İçecek: 6545. gün 1 -> 0.
- Süt ürünleri: 6545. gün 1 -> 0.
- Kuru gıda: 6545. gün 1 -> 0.
- Temizlik ve bakım: 6545. gün 1 -> 0.

Reyonlar: her tür/mağaza türü için görülen en düşük / en yüksek 30 günlük kâr (TL, tüm o tür şubelerin toplamı):
- Tür 1, Manav: -30191.83 / 20254.55 (zarar görüldü).
- Tür 2, Manav: -36096.40 / 255890.54 (zarar görüldü).
- Tür 2, Kasap: -107393.76 / 310246.27 (zarar görüldü).
- Tür 2, Evcil hayvan: 19.47 / 45940.79.
- Tür 2, Balık: -4558.60 / -136.23 (zarar görüldü).
- Tür 3, Manav: -748669.14 / 4117632.66 (zarar görüldü).
- Tür 3, Kasap: -411197.69 / 5899083.07 (zarar görüldü).
- Tür 3, Bebek: -258756.17 / 70535.26 (zarar görüldü).
- Tür 3, Evcil hayvan: -934376.36 / -7482.69 (zarar görüldü).
- Tür 3, Bahçe ve oto: -187085.50 / 466761.84 (zarar görüldü).
- Tür 3, Mevsimlik: -386687.30 / 196724.41 (zarar görüldü).
- Tür 3, Şarküteri: -302919.03 / 1890965.92 (zarar görüldü).
- Tür 3, Fırın ve pastane: -718332.76 / 1923461.95 (zarar görüldü).
- Tür 3, Balık: -406806.74 / -16811.81 (zarar görüldü).
- Tür 3, Elektronik ve beyaz eşya: -1092495.88 / -369.26 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -250207.79 / 1987648.33 (zarar görüldü).
- Tür 3, Ev ve mutfak: -87653.22 / 483640.47 (zarar görüldü).
- Tür 3, Oyuncak: -246156.07 / 1341972.34 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -690024.70 / 1338231.23 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 56 / 141. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 266.
- RejectBrandOffer: 747.
- ReplaceMasters: 81.
- SetDepartment: 105.
- SetDeptStance: 14.
- SetSourcing: 20.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 398.

Ertelenen kararlar (oyuncuya dönen neden, tekrar sayısı):
- Tedarik asgarisi: 398.
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

- Zararli şubeler (net zarar): 8883510475,39 TL.
- Üst yönetim ücretleri: 474912329,99 TL.
- Depo ve merkez giderleri: 227805754,15 TL.
- Vergi ödemeleri: 22078676,15 TL.
- İşletme giderleri (ücret hariç): 18418715,31 TL.

Arka planin kasaya toplam net etkisi: -9514269288,29 TL. Reddedilen komut: 296.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -17747604,88 TL.
- Depo ve merkez: -227805754,15 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -475238567,56 TL.
- Internet satışi: 0,00 TL.
- Subeler: -8802273359,81 TL.

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
| 11 | 71 | 20 | 151 | 0 / 3439091198 |
| 12 | 67 | 20 | 151 | 0 / 3551332904 |
| 13 | 65 | 20 | 151 | 0 / 3608917786 |
| 14 | 64 | 20 | 151 | 0 / 3764119345 |
| 15 | 65 | 20 | 151 | 0 / 3906787046 |
| 16 | 66 | 20 | 151 | 0 / 4022490350 |
| 17 | 66 | 20 | 151 | 0 / 4106515797 |
| 18 | 66 | 20 | 151 | 0 / 4247660882 |
| 19 | 66 | 20 | 151 | 0 / 4438563514 |
| 20 | 63 | 20 | 151 | 0 / 4531107228 |
| 21 | 61 | 20 | 151 | 0 / 4703714880 |
| 22 | 61 | 20 | 151 | 0 / 4818212431 |
| 23 | 61 | 20 | 151 | 0 / 4911777117 |
| 24 | 63 | 20 | 151 | 0 / 5017046055 |
| 25 | 63 | 20 | 151 | 0 / 5174495485 |
| 26 | 62 | 20 | 151 | 0 / 5294445267 |
| 27 | 63 | 20 | 151 | 0 / 5384598971 |
| 28 | 63 | 20 | 151 | 0 / 5614968312 |
| 29 | 61 | 20 | 151 | 0 / 5786449255 |
| 30 | 60 | 20 | 151 | 0 / 5980908128 |

Rakipler: en çok 92 etkin zincir; 29 farklı satılık zincir; 27 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 468 fiyat savaşı.
Ezeli rakip: Ezeli rakip: BİN (Haluk Sezer) · 9197 mağaza · senden çekildiği savaş: 30.
Markalardan toplam 2901902.19 TL; küsen farklı marka 11.

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

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 56 / 149. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 256.
- RejectBrandOffer: 763.
- ReplaceMasters: 81.
- SetDepartment: 118.
- SetSourcing: 24.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 1183.

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
- Tedarik asgarisi: 1183.
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

- Zararli şubeler (net zarar): 419653492,64 TL.
- Üst yönetim ücretleri: 57392058,05 TL.
- Depo ve merkez giderleri: 29404260,34 TL.
- İşletme giderleri (ücret hariç): 2585004,91 TL.
- Mal alımi (stok yatırımi): 277758,32 TL.

Arka planin kasaya toplam net etkisi: -503776962,68 TL. Reddedilen komut: 31.

Net sonucun kaynagi (kasaya girisle ayni degildir):

- Aile dükkânı: -2537914,04 TL.
- Depo ve merkez: -29404260,34 TL.
- Diğer (yönetim, banka, fire, kasa farkı): -59154619,28 TL.
- Internet satışi: 0,00 TL.
- Subeler: -418882693,39 TL.

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
| 11 | 37 | 20 | 15 | 397665 / 3439091198 |
| 12 | 40 | 20 | 15 | 336896 / 3551332904 |
| 13 | 40 | 20 | 15 | 338917 / 3608917786 |
| 14 | 38 | 20 | 15 | 350394 / 3764119345 |
| 15 | 38 | 20 | 15 | 360315 / 3906787046 |
| 16 | 40 | 20 | 15 | 367664 / 4022490350 |
| 17 | 40 | 20 | 15 | 358036 / 4106515797 |
| 18 | 41 | 20 | 15 | 355326 / 4247660882 |
| 19 | 40 | 20 | 15 | 359931 / 4438563514 |
| 20 | 40 | 20 | 15 | 358956 / 4531107228 |
| 21 | 40 | 20 | 15 | 349913 / 4703714880 |
| 22 | 40 | 20 | 15 | 373163 / 4818212431 |
| 23 | 39 | 20 | 15 | 353964 / 4911777117 |
| 24 | 48 | 20 | 15 | 11495 / 5017046055 |
| 25 | 48 | 20 | 15 | 11722 / 5174495485 |
| 26 | 48 | 20 | 15 | 11467 / 5294445267 |
| 27 | 48 | 20 | 15 | 11943 / 5384598971 |
| 28 | 48 | 20 | 15 | 12055 / 5614968312 |
| 29 | 48 | 20 | 15 | 11863 / 5786449255 |
| 30 | 46 | 20 | 15 | 11906 / 5980908128 |

Rakipler: en çok 49 etkin zincir; 5 farklı satılık zincir; 5 piyasadan çekilme (iflas veya satın alınma); 0 görünür iflas haberi; bizim 0 satın almamız; 48 fiyat savaşı.
Ezeli rakip: yok.
Markalardan toplam 2210812.64 TL; küsen farklı marka 10.

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
- Tür 2, Evcil hayvan: 5.79 / 8437.19.
- Tür 2, Fırın ve pastane: -1503.00 / 6712.94 (zarar görüldü).
- Tür 2, Balık: -636.90 / -31.96 (zarar görüldü).
- Tür 3, Manav: -188.30 / 6503.73 (zarar görüldü).
- Tür 3, Kasap: -959.43 / 19208.66 (zarar görüldü).
- Tür 3, Bebek: -281.94 / -17.29 (zarar görüldü).
- Tür 3, Evcil hayvan: -513.47 / -30.32 (zarar görüldü).
- Tür 3, Bahçe ve oto: -42.81 / 748.97 (zarar görüldü).
- Tür 3, Mevsimlik: -25.34 / 1043.00 (zarar görüldü).
- Tür 3, Şarküteri: -20.50 / 3100.45 (zarar görüldü).
- Tür 3, Fırın ve pastane: -998.76 / 11809.32 (zarar görüldü).
- Tür 3, Balık: -1426.62 / -91.31 (zarar görüldü).
- Tür 3, Giyim ve ev tekstili: -456.95 / 4368.77 (zarar görüldü).
- Tür 3, Ev ve mutfak: -90.18 / 885.94 (zarar görüldü).
- Tür 3, Oyuncak: -71.32 / -11.88 (zarar görüldü).
- Tür 3, Kırtasiye ve kitap: -559.95 / -32.33 (zarar görüldü).

Akış: mahalle karar/olay/mağaza eşiği sayımıyla 0 sıkıcı dönem; C'nin teklif/sıra/savaş/kademe hareketi de sayılınca 0. Felaket yığılması 56 / 64. Bu aynı kampanyanın iki gözlemidir, eski sürümle yeniden oynama değildir.

Başarılı yeni oyuncu komutları:
- AcceptBrandOffer: 302.
- RejectBrandOffer: 564.
- ReplaceMasters: 9.
- SetDepartment: 39.
- SetDeptStance: 13.
- SetSourcing: 10.
- Asgari alımı karşılayamadığı için ertelenen tedarik kararı: 2854.

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
- Tedarik asgarisi: 2854.
- Önce işletmenin borcunu kapat.: 1.
- Önce yerel pay %35.: 8.

Mağaza türü: 1 mahalle, 2 süpermarket, 3 hipermarket. CSV reyon numaraları MarketDepartments::EDept sırasıdır.

## Denetimin kapsamı

Satış fisindeki para, siparis bedeli, ana gün kapanisi ve mal kabul aktarimi bagimsiz hesapla kontrol edilir. Negatif stok, gecersiz sayilar ve pay sinirlari her gün denetlenir. Arka planin net kasa hareketi ayrica olculur; tek tek kalemlerin tam korunum denetimi B'nin muhasebe defteri C tarafindan baglandiginda tamamlanacak. Kasa eksisi oyun sonu degildir; sikinti günleri ayri sayilir. Ligler C bolumunde yillik olculur; B/C3 entegrasyonu oncesi bu koşu tam oyun dengesi degildir.

Fiyatlar normal oyuncunun kullandigi adimlarla degisir. Kredi, şube, depo, yonetici ve kararlar normal komutlardan gecer. Aile dükkânı PlayDay ile oynar; test modu, bedava mal veya para kullanilmaz. CSV tutarlari kurustur.
