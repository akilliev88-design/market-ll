# C9 çekirdek denge raporu

## §5 hedef eğrileri

✓/✗ hedef karşılaştırmasıdır; test sonucu değildir. Atak bir kötü oyun senaryosu değildir; sayısal ilk şube/mağaza/sıra hedefi atanmadı. 10 yıllık koşular 21/22/23, 30 yıllık koşular 21. Tutarlar nominal TL.

| Tarz / tohum / süre | İlk şube günü | 3. yıl mağaza | 10. yıl mağaza | 10. yıl ulusal | 20. yıl ulusal | Kurtarma | Kâr eğrisi | Sıkıcı dönem |
|---|---:|---:|---:|---:|---:|---:|---|---:|
| Temkinli / 21 / 10 yıl | 399 ✓ | 7 | 1 ✗ | 34 ✗ | — | 1 (10 yıl) | ✗* | 0 ✓ |
| Temkinli / 22 / 10 yıl | 459 ✗ | 7 | 1 ✗ | 35 ✗ | — | 1 (10 yıl) | ✗* | 0 ✓ |
| Temkinli / 23 / 10 yıl | 489 ✗ | 7 | 1 ✗ | 36 ✗ | — | 1 (10 yıl) | ✗* | 0 ✓ |
| Dengeli / 21 / 10 yıl | 320 ✗ | 7 ✗ | 1 ✗ | 34 ✗ | — | 1 (10 yıl) | ✗* | 0 ✓ |
| Dengeli / 22 / 10 yıl | 306 ✗ | 7 ✗ | 1 ✗ | 35 ✗ | — | 9 (10 yıl) | ✗* | 0 ✓ |
| Dengeli / 23 / 10 yıl | 334 ✗ | 7 ✗ | 1 ✗ | 34 ✗ | — | 1 (10 yıl) | ✗* | 0 ✓ |
| Atak / 21 / 10 yıl | 187 | 21 | 139 | 8 | — | 0 (10 yıl) | ✗* | 0 ✓ |
| Atak / 22 / 10 yıl | 180 | 21 | 72 | 8 | — | 0 (10 yıl) | ✗* | 0 ✓ |
| Atak / 23 / 10 yıl | 180 | 22 | 83 | 8 | — | 0 (10 yıl) | ✗* | 0 ✓ |
| Temkinli / 21 / 30 yıl | 399 ✓ | 7 | 1 ✗ | 34 ✗ | 33 | 2 ✗ | ✗* | 0 ✓ |
| Dengeli / 21 / 30 yıl | 320 ✗ | 7 ✗ | 1 ✗ | 34 ✗ | 33 ✗ | 1 ✓ | ✗* | 0 ✓ |
| Atak / 21 / 30 yıl | 187 | 21 | 139 | 8 | 7 | 5 ✓ | ✗* | 0 ✓ |

İlk şube takvim hedeflerinin gün karşılığı yaklaşık 4–8 ay = 120–244, 8–14 ay = 240–426. Üçüncü yıl 10 mağaza alt sınır kabul edildi. Şube sayısı açık şubeler + ilk mağaza; kapanmış kayıtlar sayılmaz.

* Kâr kıyası, aşağıdaki mağaza çekirdeği üst sınırının son yılda enflasyondan arındırılmış ilk yıl düzeyini koruyup korumadığıdır. Yılın ortalama katalog liste düzeyi kullanıldı; bütün yılların monoton artması şart koşulmadı. Ağ giderleri ilk dükkân defterine karıştığı için gerçek ilk mağaza neti ayrıca doğrulanamıyor. ✓ varsa dahi gider dağıtımı düzeltilmeden hedef tamamlanmış sayılmaz.

## İlk dükkânın yıllık faaliyet kârı

Birinci dizi aile dükkânı defter FAVÖK’ü − aile müdürünün toplam işveren maliyeti. İkinci dizi brüt kâr − personel/SGK/kira/işletme/aile müdürü; ayrıştırılamayan diğer giderleri içermediği için üst sınırdır. Brüt kâr zaten fire ve stok kaybını içerir. Patron gideri merkezde; patron sonrası ayrı sütun ham yıllık CSV’de. Sütun sırası oyun yılıdır.

- Temkinli / 21 / 10 yıl, defter: 65,074.04 → 48,793.57 → 45,647.14 → 54,442.10 → 54,505.43 → 63,647.28 → 89,494.72 → 101,996.00 → 75,671.76 → 26,604.84 TL.
  Mağaza çekirdeği üst sınırı: 66,020.97 → 66,750.09 → 54,955.53 → 54,442.10 → 58,699.98 → 63,682.64 → 89,567.53 → 102,089.70 → 75,751.85 → 31,587.23 TL.
- Temkinli / 22 / 10 yıl, defter: 62,255.27 → 56,386.69 → 44,860.05 → 55,494.79 → 52,424.40 → 56,786.34 → 33,739.94 → 77,165.17 → 21,010.91 → 91,712.25 TL.
  Mağaza çekirdeği üst sınırı: 63,245.74 → 69,960.42 → 62,420.91 → 60,488.55 → 55,315.91 → 56,894.85 → 59,730.06 → 77,648.64 → 25,620.54 → 93,083.12 TL.
- Temkinli / 23 / 10 yıl, defter: 56,377.17 → 52,301.89 → 45,227.14 → 48,617.97 → 51,401.65 → 23,307.10 → 72,419.65 → 61,230.32 → 42,231.61 → 116,488.51 TL.
  Mağaza çekirdeği üst sınırı: 57,194.81 → 69,513.19 → 59,327.91 → 55,861.71 → 51,504.71 → 46,884.07 → 72,826.43 → 62,403.39 → 46,210.64 → 118,383.21 TL.
- Dengeli / 21 / 10 yıl, defter: 45,193.49 → 39,304.68 → 51,381.99 → 56,478.62 → 39,651.52 → 77,727.39 → 62,929.57 → 80,241.99 → 34,435.76 → 57,323.77 TL.
  Mağaza çekirdeği üst sınırı: 54,529.35 → 63,417.98 → 51,381.99 → 56,478.62 → 41,113.50 → 77,727.39 → 82,210.53 → 87,857.74 → 68,573.76 → 57,622.70 TL.
- Dengeli / 22 / 10 yıl, defter: 46,104.43 → 37,120.99 → 61,445.27 → 41,358.63 → 52,057.46 → 55,074.47 → 13,378.31 → -4,577.11 → -59,140.32 → -65,331.35 TL.
  Mağaza çekirdeği üst sınırı: 55,412.61 → 60,468.66 → 65,338.11 → 52,691.29 → 52,126.64 → 55,133.48 → 65,136.52 → 4,094.56 → -57,104.36 → -60,714.81 TL.
- Dengeli / 23 / 10 yıl, defter: 39,225.11 → 34,330.31 → 56,905.58 → 56,891.89 → 75,757.92 → 67,741.64 → 40,034.70 → 51,904.60 → 75,189.80 → 92,209.05 TL.
  Mağaza çekirdeği üst sınırı: 51,660.87 → 57,728.08 → 56,955.80 → 58,923.53 → 75,861.56 → 67,836.46 → 43,763.35 → 52,896.28 → 76,245.32 → 95,886.38 TL.
- Atak / 21 / 10 yıl, defter: 45,538.87 → -135,139.11 → -28,273.94 → -178,565.86 → -416,789.52 → -586,698.84 → -747,498.60 → -716,850.83 → -788,626.97 → -1,265,566.10 TL.
  Mağaza çekirdeği üst sınırı: 73,741.21 → 85,350.37 → 83,048.40 → 99,771.08 → 101,913.02 → 107,792.73 → 119,860.10 → 132,894.06 → 98,479.91 → 71,769.39 TL.
- Atak / 22 / 10 yıl, defter: 24,633.26 → -168,726.51 → 33,629.07 → -86,471.20 → -145,193.50 → -355,318.92 → -431,309.27 → -560,229.42 → -482,417.73 → -315,293.28 TL.
  Mağaza çekirdeği üst sınırı: 81,731.10 → 77,297.81 → 71,265.45 → 81,500.59 → 77,187.10 → 71,966.33 → 65,335.29 → 72,295.59 → 42,236.10 → 77,911.93 TL.
- Atak / 23 / 10 yıl, defter: 46,146.92 → -203,294.09 → -74,693.03 → -307,942.03 → -477,337.20 → -695,337.98 → -409,156.41 → -304,857.84 → -138,203.15 → 116,727.12 TL.
  Mağaza çekirdeği üst sınırı: 74,035.60 → 78,120.80 → 79,939.79 → 88,268.82 → 87,362.35 → 75,397.39 → 89,347.84 → 67,039.11 → 79,046.74 → 120,098.77 TL.
- Temkinli / 21 / 30 yıl, defter: 65,074.04 → 48,793.57 → 45,647.14 → 54,442.10 → 54,505.43 → 63,647.28 → 89,494.72 → 101,996.00 → 75,671.76 → 26,604.84 → 79,148.25 → 32,014.83 → 166,538.76 → 211,919.81 → 319,260.13 → 327,806.83 → 374,938.12 → 409,998.52 → 454,659.08 → 489,996.44 → 503,907.01 → 450,153.98 → 556,679.48 → 715,180.40 → 765,383.43 → 954,304.19 → 983,853.38 → 1,015,571.22 → 1,122,683.26 → 1,173,475.10 TL.
  Mağaza çekirdeği üst sınırı: 66,020.97 → 66,750.09 → 54,955.53 → 54,442.10 → 58,699.98 → 63,682.64 → 89,567.53 → 102,089.70 → 75,751.85 → 31,587.23 → 80,372.58 → 35,204.37 → 168,921.98 → 215,707.38 → 324,454.81 → 334,284.44 → 379,642.87 → 413,987.84 → 459,357.04 → 495,298.75 → 509,631.65 → 456,681.41 → 564,222.62 → 724,372.86 → 775,628.83 → 967,917.07 → 998,057.08 → 1,031,355.28 → 1,139,345.88 → 1,191,942.24 TL.
- Dengeli / 21 / 30 yıl, defter: 45,193.49 → 39,304.68 → 51,381.99 → 56,478.62 → 39,651.52 → 77,727.39 → 62,929.57 → 80,241.99 → 34,435.76 → 57,323.77 → 58,283.05 → 75,536.53 → 129,618.56 → 158,837.59 → 109,573.71 → 186,565.15 → 289,121.52 → 312,512.94 → 337,587.04 → 371,627.94 → 401,069.53 → 341,102.70 → 443,039.17 → 542,028.24 → 618,959.33 → 707,038.91 → 795,451.83 → 791,576.32 → 887,073.12 → 931,329.23 TL.
  Mağaza çekirdeği üst sınırı: 54,529.35 → 63,417.98 → 51,381.99 → 56,478.62 → 41,113.50 → 77,727.39 → 82,210.53 → 87,857.74 → 68,573.76 → 57,622.70 → 59,568.27 → 77,192.27 → 132,012.45 → 162,636.76 → 227,612.98 → 194,879.31 → 292,554.67 → 327,924.47 → 344,294.37 → 377,017.75 → 406,891.84 → 347,606.61 → 450,635.30 → 551,216.91 → 629,264.46 → 720,295.77 → 809,861.14 → 807,244.12 → 903,853.84 → 949,829.15 TL.
- Atak / 21 / 30 yıl, defter: 45,538.87 → -135,139.11 → -28,273.94 → -178,565.86 → -416,789.52 → -586,698.84 → -747,498.60 → -716,850.83 → -788,626.97 → -1,265,566.10 → -239,747.11 → 150,973.69 → -232,177.14 → -149,876.44 → -905,635.67 → -2,023,215.14 → -2,046,045.95 → -1,696,863.05 → -880,676.04 → -1,222,542.06 → -26,746.80 → 755,368.35 → 917,421.81 → 1,127,182.09 → 1,268,915.29 → 1,425,770.92 → 1,631,114.39 → 1,628,745.66 → 1,865,743.49 → -3,334,114.34 TL.
  Mağaza çekirdeği üst sınırı: 73,741.21 → 85,350.37 → 83,048.40 → 99,771.08 → 101,913.02 → 107,792.73 → 119,860.10 → 132,894.06 → 98,479.91 → 71,769.39 → 94,861.36 → 154,480.31 → 163,010.65 → 200,338.18 → 360,584.36 → 353,357.41 → 404,736.27 → 444,266.40 → 459,177.07 → 572,882.47 → 738,210.79 → 772,488.15 → 936,956.19 → 1,150,117.12 → 1,296,718.60 → 1,458,198.20 → 1,667,454.34 → 1,666,016.85 → 1,907,171.38 → -846,351.46 TL.

## Mal ve patron

| Tarz / tohum / süre | Acil mal | Boş raf ayı | Mal parası uyarısı | Net maaş TL | Net kâr payı TL | Son servet TL |
|---|---:|---:|---:|---:|---:|---:|
| Temkinli / 21 / 10 | 1 | 121 | 6 | 140,892.60 | 0.00 | 13,601.10 |
| Temkinli / 22 / 10 | 0 | 121 | 9 | 139,835.92 | 0.00 | 8,818.42 |
| Temkinli / 23 / 10 | 0 | 121 | 5 | 143,623.67 | 0.00 | 7,017.17 |
| Dengeli / 21 / 10 | 0 | 121 | 13 | 140,063.58 | 0.00 | 12,772.08 |
| Dengeli / 22 / 10 | 2 | 121 | 18 | 102,815.06 | 3,809.70 | 0.00 |
| Dengeli / 23 / 10 | 1 | 121 | 4 | 134,335.91 | 0.00 | 0.00 |
| Atak / 21 / 10 | 0 | 121 | 0 | 153,279.84 | 0.00 | 25,988.34 |
| Atak / 22 / 10 | 0 | 121 | 0 | 157,766.55 | 3,377.05 | 30,126.10 |
| Atak / 23 / 10 | 0 | 121 | 0 | 164,496.54 | 43,869.35 | 71,759.39 |
| Temkinli / 21 / 30 | 1 | 361 | 9 | 3,755,956.72 | 806,555.65 | 1,395,212.81 |
| Dengeli / 21 / 30 | 0 | 361 | 13 | 3,832,539.78 | 0.00 | 638,831.28 |
| Atak / 21 / 30 | 3 | 361 | 12 | 3,451,579.95 | 0.00 | 257,871.45 |

Boş raf ayı, en az bir karşılanamayan ürün isteği bulunan takvim ayıdır; bütün ay rafların boş olduğu anlamına gelmez. İlk/son takvim ayı kısmidir. Toptancı acil malı ve uyarı ilgili gün alanının değişmesinden sayılır, haber metninden tahmin edilmez. Aynı tohumun 10 yıllık koşusu 30 yılın tekrarlanan önekidir; toplamları bağımsız olay gibi toplamayın. Servet yaşam giderlerinden sonraki bakiye, maaşların toplamı değildir.

## Sonuç ve önemli beş öneri

Son kaynak **a80d925**. DERLE geçti (son kaynak 5,91 sn); TEST **153/153** (152 temiz + 1 motor HTTP uyarısı; başarısız/çalışmamış 0); Smoke geçti. Yeni `AutoPlay.CoreBalancePolicy`, `Suppliers.Lifeline` ve değiştirilmeyen `AutoPlay.LateCarefulGrowth` geçti. Temkinli21 ilk gerçek şube **399. gün** (C8 609); 600. günde 3 mağaza. Testin 600 gün ve 365 günden sonra büyüme koşulları aynen korundu.

12 kampanya / **65.751 gün**: para/stok denetimi ve defter işaretli/mutlak farkı **0**. Aynı tohum21 için 10 yıllık bütün günlük satırlar 30 yılın ilk on yılıyla birebir eşleşir. Ham klasörler `Saved/AutoPlay/20261002-092637` (10×3×3) ve `20261002-091934` (30×3×1); yıllık/aylık/karar/kurtarma kopyaları `C9_veri/`. Ara bot denemeleri teslim ölçümü değildir.

**Denge hedefi tamamlanmadı.** Temkinli ve dengeli on yılda tek mağazaya dönüyor; atak ilk on yılda 72/83/139 mağaza ve ulusal 8. sıra ile büyüyebiliyor, fakat tohum21 otuzuncu yılda çöküyor. Atak final kasa −2.529.482,88 TL, kalan plan 5.710.104,00 TL. Son beş kurtarma günleri 10666/10729/10794/10859/10923; aralar 63/65/65/64 gün. Beş plan sayısal üst sınırda olsa da borç/toparlanma hedefi sağlıklı kabul edilemez. Dengeli22 yalnız ilk on yılda 9 kurtarma gördü; üç tohumda büyüme/tökezleme farkı sürüyor.

Atak ilk dükkân defteri 2. yılda **−135.139,11 TL**, 10. yılda **−1.265.566,10 TL**. Ayrıştırılmamış diğer giderler sırasıyla **220.489,48 / 1.337.335,49 TL**; doğrudan izlenebilen mağaza çekirdeği **85.350,37 / 71.769,39 TL**. `MarketBranches::Open` tadilatı ve şubenin işe alımını `OtherCosts`'a koyuyor; `MarketLedger::BeginClose` kalan gideri mağaza belirtmeden ilk dükkânın Marketing hesabına yazıyor. **Büyük defter zararını bütünüyle ilk mağazanın satış ekonomisi diye yorumlamayın.** Kasa korunumu bu gider dağılımını sınamaz.

Temkinli/dengeli21 otuzuncu yıl doğrudan mağaza çekirdeği enflasyondan arındırıldığında ilk yılın **%57,65 / %55,62** düzeyinde. Nominal kâr artıyor, enflasyonun gerisinde kalıyor. Atak30 doğrudan çekirdek de eksiye dönüyor; bu son çöküş yalnız gider dağılımıyla açıklanamaz. Boş raf ayları her koşuda bütün takvim aylarına yayılıyor (10 yılda 121 / 30 yılda 361); bir karşılanamayan istek bile ayı saydığı için bu, ay boyu tam boşluk değildir. İstek sayıları `ek_olcum.csv`'de.

Öneriler **uygulanmadı**; sayılar tek değişkenli kontrol deneyi adaylarıdır. Birden fazla sabiti birlikte değiştirmek neden-sonuç kıyasını bozar. Atak bir kötü oyuncu testi yerine geçmez.

| Öneri | Kalem ve önerilen değer | Kaynak |
|---|---|---|
| R1 | İlk dükkân kârının kapsamı: mağaza dışı tadilat/şube işe alma giderinin ilk mağazaya dağıtımı **%100 → %0**; şube Store kimliği veya merkez hesabı. Bunu yapmadan ilk mağaza gerçek faaliyet netini hedefe göre ayarlamayın. | [MarketBranches.cpp:348](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketBranches.cpp:348); [MarketBranches.cpp:544](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketBranches.cpp:544); [MarketLedger.cpp:185](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketLedger.cpp:185) |
| R2 | İlk şube gecikmesi: açılış bedeli tamponu dengeli **1,5 → 1,0**, ayrı deneyde temkinli **2,5 → 2,0**. İki haftalık mal + bütün mevcut/yeni ağ gider yedeği korunur. Dengeli21, 244. gün %40,50 pay ve 25.603,68 TL kasaya rağmen açamıyor; ilk açılış 320. gün. Pay eşiği 35 zaten aşılmış: bu koşuda pay eşiğini düşürmek doğru müdahale değil. Bunlar bot sabitleridir. | [MarketAutoPlay.cpp:34](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketAutoPlay.cpp:34); [MarketAutoPlay.cpp:35](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketAutoPlay.cpp:35); [MarketAutoPlayFinance.cpp:61](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketAutoPlayFinance.cpp:61) |
| R3 | Mağaza sayısı / ulusal sıra: şube çekim payındaki rekabet katsayısı **3,0 → 2,5** tek deney. Ayrı karşı koşuda botun art arda zararlı ay kapanış kapısı **2 → 4**; uzun zararları büyütme riski ayrıca ölçülsün. Sırayı yapay düzeltmek yerine kârlı açık ağı koruyun. | [MarketBranches.cpp:598](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketBranches.cpp:598); [MarketAutoPlayFinance.cpp:67](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketAutoPlayFinance.cpp:67) |
| R4 | Kurtarma döngüsü: açık şubesiz ağda ücretli atıl depo/kamyon hedefi **0**; mevcut varlıklar ve iade/satış bedeli deftere işlenerek küçülsün. Planın işletme sermayesi ufku **1 → 3 ay**, kalan zorunlu merkez/patron gideri de dahil edilsin. Şu an depo müdürü kalabiliyor, kamyon/depo gideri sürüyor ve FamilyMonthCost bunların tamamını içermez. Son planların 63–65 günde yenilenmesi ana kanıt. | [MarketFinance.cpp:396](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketFinance.cpp:396); [MarketFinance.cpp:443](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketFinance.cpp:443); [MarketCompany.cpp:247](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketCompany.cpp:247) |
| R5 | Enflasyona göre ilk mağaza kârı: yıllık reel ücret artışı **%1,5 → %0,5** kontrol deneyi. Gider dağılımı düzeltildikten sonra, ücretin müşteri bütçesine etkisi de ölçülerek değerlendirilmeli; otomatik kabul değil. | [MarketPrices.cpp:26](C:/Users/mtass/Desktop/market-ll-A/Source/MirasMarket/MarketPrices.cpp:26) |

### Hedef dışındaki her satırın karşılığı

| Koşu | Hedef dışı ölçü → kalem/öneri |
|---|---|
| Temkinli/21/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; 10. yıl mağaza / ulusal sıra → şube neti/kapanışı R3; kurtarma yükü R4 |
| Temkinli/22/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; İlk şube → açılış tamponu R2; 10. yıl mağaza / ulusal sıra → şube neti/kapanışı R3; kurtarma yükü R4 |
| Temkinli/23/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; İlk şube → açılış tamponu R2; 10. yıl mağaza / ulusal sıra → şube neti/kapanışı R3; kurtarma yükü R4 |
| Dengeli/21/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; İlk şube → açılış tamponu R2; 10. yıl mağaza / ulusal sıra / 3. yıl mağaza → şube neti/kapanışı R3; kurtarma yükü R4 |
| Dengeli/22/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; İlk şube → açılış tamponu R2; 10. yıl mağaza / ulusal sıra / 3. yıl mağaza → şube neti/kapanışı R3; kurtarma yükü R4; Kurtarma → atıl merkez/yedek R4 |
| Dengeli/23/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; İlk şube → açılış tamponu R2; 10. yıl mağaza / ulusal sıra / 3. yıl mağaza → şube neti/kapanışı R3; kurtarma yükü R4 |
| Atak/21/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5 |
| Atak/22/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5 |
| Atak/23/10 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5 |
| Temkinli/21/30 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; 10. yıl mağaza / ulusal sıra → şube neti/kapanışı R3; kurtarma yükü R4; Kurtarma → atıl merkez/yedek R4 |
| Dengeli/21/30 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; İlk şube → açılış tamponu R2; 10. yıl mağaza / ulusal sıra / 3. yıl mağaza / 20. yıl sıra → şube neti/kapanışı R3; kurtarma yükü R4 |
| Atak/21/30 yıl | Kâr/enflasyon ve defter kapsamı → R1/R5; Kurtarma sonrası borç ve otuzuncu yıl çöküş → R4 |

Kaynaklar ve sayısal deney değerleri üstteki R1–R5 tablosundadır. Temkinli için üçüncü yıl, atak için ilk şube/mağaza/ulusal sıra sayısal hedef atanmadığından ✓/✗ uydurulmadı. Ulusal sıra açık ağın yetersizliğiyle birlikte okunmalı; `LeagueCompression` yalnız dünya sıralamasına etki eder, bu ulusal tabloyu düzeltmez.

## Bot değişikliği ve sınırlar

- Fiyat kapısı kapanmış kayıtları saymayan `MarketBranches::OpenCount`; ilk şube indirim kesmez, rakip/yerel pay tavanı ve haftalık hedefte en çok %3 yukarı adım. Ürün gözlem hedefi gerçek bot hedefiyle aynıdır. `OpenCount` tadilat/ruhsat aşamasındaki canlı şubeleri de sayan mevcut API'dir.
- İK/aile müdürü/ek görevli: yalnız kasa yeterliliği değil, beklenen ek brüt kâr veya gözlenen kuyruk/depoda mal varken boş raf kaybı. İşveren SGK'sı ve işe alım bedeli dahil. İK'nın açacağı bir sonraki mağazanın katkısı 90 günü geçmiş aynı tür şubenin görünen 30 günlük kârından; daha zengin ile satış bonusu yazılmaz, yeni açılış ve gideri ayrıca kasada tutulur. Bu tahmin gerçekleşmiş deney etkisi değildir.
- Korunmuş mal bütçesi `max(toptancı Volume30, son 30 günlük mal maliyeti) × 14/30`; ilk ay görünen stok maliyeti taban. Yatırım/işe alım/maaş artışı/kâr payı bunu korur, mal siparişi bu parayı kullanabilir. Bu, toptancı hacim göstergesi ve maliyetin tahminidir; geleceği okumaz. Temkinli de vade/acil mal kullanır; yeterli yedek yoksa vadeyi erken kapatmaz.
- Yeni test fiyat kapısında tadilat/açık/kapalı şube, ani artış, mal yedeği ve işe alım kârlılık/nakit sınırlarını sınar. Claude'un İK test kurulumundaki sekiz kişi, sonraki iki görevli senaryosunu bozuyordu: sekiz çalışan kilidi ayrıca sınandı, asıl senaryo iki açık şubeyle korunarak düzeltildi (cf61dae; MarketStaffTests.cpp:241). Oyun kuralı düzeltmesi yok. Kayıt sürümü/uyumu ve oyun sabitleri değişmedi.
- Doğal koşular acil malı sınadı: temkinli21 on yılda 1, dengeli22 2, dengeli23 1; otuz yıl atak21 3. Düşük kurtarma sayısını bütün tohumlar için iddia etmeyin.

## Menü: görüntü doğrulaması

[C9 galeri](../../../Saved/Screenshots/Menu/C9_20261002/index.html): **32 PNG**, iki tema × 1920×1080/1280×720 × dört hedef × iki gerçek kampanya. Boyut hatası 0; iki otomasyon `PASSED, campaign unchanged`. Başlangıç atak22 gerçek yeni oyun; işaretli hafta atak21 116. gün sonu, 1 Temmuz / 1. yıl. `MirasMenuDays=116` açıkça geçilmiştir; klasörün `three_year` etiketi otomasyonun varsayılan adıdır, üç yıllık örnek değildir. Grafik pozitif 16,00–240,03 TL ve negatif −1.187,40 TL günleri gösterir; veri/para/mağaza enjekte edilmedi.

- **Haftalık grafik düzeldi:** dört tema/çözünürlük varyantında pozitifler sıfır çizgisinin üstünde, negatif gün altta. Sayılar/alt günler sığıyor. Sıfır hattının ayrı `0` metin etiketi yok; bunun eklenmesi C'ye küçük görsel öneri.
- **Kampanya formu kaldı:** ilk gün kapsam/tür/oran/süre/gondol seçenekleri birlikte; karar öncesi beklenen masraf/kâr görünmüyor.
- **İlk gün yönetim/reklam kaldı:** boş/kilitli yönetim kartı ve bütün reklam kanalları geniş yer kaplıyor; henüz olmayan kanallar da listeleniyor.

İşaretli grafik dört varyantı ve ilk gün kampanya/yönetim/reklam seçili varyantları gözle incelendi; bütün 32 PNG'nin piksel incelemesi iddia edilmiyor. Her PNG'nin boyut ve SHA256 kaydı `C9_veri/dogrulama.json`'da. Menü ürün koduna dokunulmadı; yalnız çekim hedefi eklendi. Görsellerin Mustafa onayı ayrıca gerekir; otomatik sonuç bu onay yerine geçmez.

## Teslim

Kaynak commitleri `cf61dae` (main: İK test kurulumu), `45abc33`, `4d0ca8f`, `a80d925` (akis-a). Bot/rapor main'e birleştirilmedi; kaynak **akis-a**, ana klasörde aynı rapor/veri ve devir notları bulunur. İlk Claude teslimi `18e37bf`. GitHub gönderimi otomatik onay incelemesinde reddedildi; origin doğrulandı (`https://github.com/m07tas/market-ll`). Son gönderim durumu devir notuna yazılır. Mustafa'nın ilk `Docs/Kurgu/01_KARARLAR.md`, `06_GIDIS_YOLU.md` değişiklikleri ve `bekleyen/` korunmuştur.
