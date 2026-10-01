# C4 finans doğrulaması — 01.10.2026

**Kod doğrulandı; toparlanma ve büyüme dengesi hedefi karşılanmadı.** DERLE geçti, TEST **138/138** (137 temiz, bir motor HTTP uyarısı), Smoke geçti. Claude tesliminde derleme/test düzeltmesi **0**. Botun 12 kampanyasında **65.751 gün**, stok/satış denetimi hatası ve açıklanamayan muhasebe farkı **0**. Hiçbir oyun sabiti değiştirilmedi.

Dengeli botun 10./20./30. yıl ulusal sırası **35 / 33 / 31**, dünya sırası **25 / 32 / 32**. 30 yıllık üç koşuda kurtarma planı temkinli **150**, dengeli **165**, atak **5** kez geldi. 10 yıllık dokuz koşuda ayrıca **230** kurtarma var; toplam **550** çağrı, fakat aynı tohumun ilk on yılı iki seride tekrar oynandığından bunlar bağımsız 550 olay değildir.

## Koşular ve yöntem

- Son kaynak: `f1b9d75` finans yedeği; menü otomasyonu `7f4dd3f`. 30 yıl: `Saved/AutoPlay/20261001-155215`, 215,9 sn. 10 yıl: `Saved/AutoPlay/20261001-160337`, 906,9 sn. Önceki 153450/153911 sonuçları son SGK/merkez gider yedeğini içermez; bu teslimin sonucu değildir.
- Türkiye/Kırklareli, Normal, tohumlar 21/22/23; üç tarz. Dünyasız PlayDay ve gerçek Director komutları; para/ürün/mağaza verilmedi. Temkinli şirket kredisi almaz; dengeli yıllık, atak altı aylık teklif fırsatını aylık incelemede değerlendirir. Olgun şubeler iki ardışık zararlı aylık incelemede kapanır.
- Yeni şubenin açılışından sonra bütün ağın bir aylık sabit gideri; yatırım kredisi planında üç aylık gider kalır. Şube giderine aile personeli/SGK, merkez yöneticileri/SGK, depo, kamyon ve karanlık mağaza eklenir. MonthlyFixedCost içindeki şube/müdür ücretleri yeniden sayılmaz.
- Ham ayrıntılar `C4_30_yil_rapor.md` ve `C4_10_yil_rapor.md`; yanlarındaki lig/reyon/tedarik/dönem/defter/banka CSV'leri. Her şubenin ilk 180 gün dökümü ayrı CSV'dedir; kurtarma günleri ve komut denemeleri ham rapordadır. Tam günlük kayıtlar Saved'de korunur.

## Şubelerin ilk 180 günü

10 yıllık üç tohum serisinde **tam 180 gün oynanan** şubelerin eşit ağırlıklı ortalaması:

| Tür | Tam örnek | Zarar eden | Ortalama net kâr |
|---|---:|---:|---:|
| Mahalle | 22 | 0 | **7.718,32 TL** |
| Süpermarket (`buyuk`) | 18 | 0 | **56.119,51 TL** |
| Hipermarket | 390 | 58 | **56.259,59 TL** |

Eksik süreli örnekler ayrıca CSV'de: mahalle 5, süper 1, hiper 125. Erken kapanan şubeler tam 180 gün ortalamasından çıkarıldığı için ortalama hayatta kalanlar lehine seçilmiştir. Açılış yılları ve enflasyon farklıdır; nominal TL ortalaması aynı tarihte yatırım karşılaştırması değildir. Faaliyet neti yatırım/ilk stok alımını içermez; fire brüt mal maliyetinde zaten sayılır, ikinci kez çıkarılmaz. Merkez/aile/banka yükü şube faaliyet kârına dağıtılmadı.

21. tohumun 30 yıllık serisinde altı tam mahalle örneğinin tümü ilk 180 günde kârlı; buna rağmen sonradan hepsi kapanıyor. Atak 21'in ilk dört şubesi 121–156 gün arasında kapanıp zarar ediyor. Bu yüzden bütün mahalle marjlarını düşürmek/artırmak yerine aile, merkez ve borç yükünü ayrı ele almak gerekir.

## Banka, kurtarma ve büyüme

| Tarz / tohum | 10. yıl mağaza | Kurtarma | Kapanan şube | Borç sınırı ihlali ay | Son kredi notu |
|---|---:|---:|---:|---:|---|
| Temkinli 21 | 1 | 32 | 1 | 0 | B+ |
| Temkinli 22 | 1 | 31 | 1 | 0 | B+ |
| Temkinli 23 | 1 | 38 | 1 | 0 | B+ |
| Dengeli 21 | 1 | 48 | 5 | 109 | D |
| Dengeli 22 | 1 | 31 | 1 | 67 | C |
| Dengeli 23 | 1 | 47 | 5 | 107 | D |
| Atak 21 | 1 | 3 | 4 | 118 | D |
| Atak 22 | 146 | 0 | 97 | 5 | A+ |
| Atak 23 | 159 | 0 | 143 | 3 | A+ |

Atak 22/23 son kasaları **6,38 / 8,30 milyon TL**, ulusal sıraları **7 / 7**, dünya **28 / 25**. Şirket borcu/FAVÖK **0,1435 / 0,0800**; ödenen şirket faizi **2.833.647,17 / 2.582.886,12 TL**. Bu iki tohumda sıfır eksi kasa günü var; diğer yedi kampanyada 1.876–2.942 eksi kasa günü.

Dengeli 21'in 10./20./30. yıl notu D/D/D; şirket borcu **131.209,25 / 369.380,96 / 990.737,89 TL**, yıllık FAVÖK **−981.108,41 / −14.005.798,82 / −87.141.201,77 TL**. CSV'deki 99 oranı negatif FAVÖK için işarettir, gerçek 99 kat borç değildir. Aile kurtarma kredileri bu şirket borcu sütununa dahil değildir. Toplam borç 30. yılda temkinli **462,52**, dengeli **528,07**, atak **489,19 milyon TL**.

Limit otomasyonu altı dengeli/atak koşuda açıldı. Dengeli 22'nin en yüksek çekilmiş limiti **28.479,55 TL**; diğerlerinde 0. Yatırım kredisi ve yapılandırma doğal oyunda gerçekleşti. RepayLine koşulu, finansman gerektiren satın alma, bağlı şirket çevirme ve satış koşulları doğal koşuda oluşmadı; bu yolların uzun oyun denemesi tamamlandı diye sunulmaz. Mevcut Banking/Chains birim testleri geçti. Bot bu komutları koşullar oluşursa kullanacak şekilde hazırdır.

## Rakipler, satın alma ve kapılar

10 yıllık atak 22: **4 teklif, 1 kabul, 3 ret**; atak 23: **6 teklif, 2 kabul, 4 ret**. Finansmanlı komut kullanıldı ancak kasa yeterli olduğundan ilave satın alma kredisi **0**. Director'ın true dönüşü teklifin işlenmesidir; kabul sayısı gerçek OurBuys artışından ölçülür. Dönüştürülen mağaza 0; yıllık örneklerde elde kalan bağlı şirket/mağazası 0. Uzun koşuda dolu bağlı şirket yaşam döngüsü yeterince sınanmadı.

Atak 22/23 fiyat savaşı **305 / 323**, ezeli rakip **BİN**; rakipçe alım **18 / 19**, bizim alım **1 / 2**, iflas/kapanma sayacı 0. Görünür iflas haberleri de 0. Otuz yılda her tarz 21 farklı dev çıkış kolu, 15 yabancı kol gördü; bu rakip kollarının teklif fırsatıdır, oyuncunun 15 yeni ülkeye girdiği anlamına gelmez.

## Reyon, tedarik ve markalar

10 yıllık seride her aylık satırın kârını o satırdaki açık reyon şube sayısına bölerek bulunan **şube başına 30 günlük kâr ortalaması**:

| En düşük | TL | Aylık örnek / zarar |
|---|---:|---:|
| Süper balık | −305,79 | 2 / 2 |
| Hiper bebek | −169,31 | 12 / 12 |
| Mahalle manav | −139,77 | 98 / 92 |

En yüksek üçü: hiper kasap **3.878,40 TL** (209 örnek, 1 zarar), hiper manav **2.520,46 TL** (206, 0), hiper elektronik **1.816,30 TL** (109, 3). Farklı açılış yıllarının nominal değerleridir; balıkta yalnız iki örnekle kesin denge kararı verilmez. Zarar eden bütün türler ham reyon raporu/CSV'de.

Tedarik çıkış/düşüş günleri hat bazında ham raporda ve CSV'de. Asgari alım koşulu botta korunur; küçük ağın alımı çökünce kademe düşer. Bu koşudan asgariyi düşürme gereği çıkmadı.

10 yıllık marka gelirleri temkinli 21/22/23 **38.976,78 / 32.999,82 / 26.119,65 TL**; dengeli **20.697,38 / 31.826,49 / 17.693,69 TL**; atak **49.439,74 / 2.290.597,47 / 3.183.108,91 TL**. Küsen farklı markalar sırasıyla **13/15/16**, **14/13/16**, **10/6/6**.

Tamamen boş raf/depo testi geçti. Ancak kaynakta `MarketBrands.cpp:68` aile dükkânında Shelf+Warehouse>0 ise bütün Capacity; şubede Units>0 ise bütün Capacity sayılıyor. Depoda bir ürün varken rafta sıfır veya kapasite 100 iken bir ürün dolu olması hâlâ tam kapasite ödemesi verebilir. C3 istismar şüphesi bütünüyle giderilmedi; mantığı değiştirmedim.

## Ritim ve dönemler

12 koşuda 31 günlük sessizlik eşiğini aşan sıkıcı dönem sayısı 0, en uzun sessizlik 19 gün. Bu eğlence kanıtı değildir: tekrar kurtarma/borç haberleri de hareket sayılabilir. 10 yıllık atak 22/23'te felaket kümeleri mahalle sayımı **0/0**, C dahil **74/83**. İl savaşlarını şirket düzeyinde toplamak sayımı artırır; iki ölçüm aynı kampanyanın farklı gözlemidir. B dönem dalgaları, net kâr ve ilk/son/en az kasa `donemler.csv` ve ham raporlarda korunur.

## C'ye beş öncelikli ayar önerisi — uygulanmadı

1. **Kurtarmayı yeniden borç üretmekten çıkar.** Dengelide 165 kurtarma, 528 milyon TL son borç. `MarketFinance.cpp:375` her eski kurtarma için yıllık +4 puan ekliyor; 165. çağrıda yalnız tekrar primi 656 puan. İlk kontrollü aday: tekrar primi toplamına **8 puan tavan** ve eski gecikmiş borçları yeni plana gerçekten birleştirme; yeni krediyi eski günlük cezaların yanında ekleme. Bu bir oyun kuralı kararıdır, C/Mustafa uygular.
2. **Kurtarma bütçesini gider/sipariş üzerinden hesapla.** Sabit 1.000 başlangıç TL yerine aile dükkânının **bir aylık sabit gideri + raf yenileme + ilk taksit**; en az bir aylık nefes. Kapanmış ağın merkez giderlerini azaltma/boş depo-kamyon satma seçeneklerini bot/oyuncuya sun. İlk 180 gün kârlı mahalleler sonradan ölürken kör marj artışı uygun değil.
3. **Kredi notu ve borç kapsamını uzlaştır.** 10. yılda temkinli 21 B+ görünürken FAVÖK −558.895 TL ve toplam borç 1,61 milyon TL; dengeli çoğu ay D'de. Aile kurtarma borcunun not/limit/ihlal hesabındaki yerini açıklaştır; pozitif faaliyet toparlanmasına yapılandırma/grace yolu tanı. Lig katsayısını bu nakit sorunu çözülmeden değiştirme: dengeli hedefe hiç yaklaşmıyor.
4. **Reyonları küçük kontrollü denemelerle düzelt.** Mahalle manav 98 örneğin 92'sinde zarar; hiper bebek 12/12. Önce gereken personeli ortak kat görevlisine bağlama veya tek reyonda ücret yükünü **bir kişi azaltma** adayını ayrı koş. Süper balıkta sadece iki örnek olduğundan mevcut oran/marjı hemen değiştirme. Kasap/manavın büyük mağaza marjını artırma; zaten pozitifler.
5. **Marka doluluğunu gerçek raf adediyle ölç.** Depo ürününü rafta doluluk kabul etme; kapasiteyi `min(raftaki adet, kapasite)` oranıyla değerlendir. Depoda 1/rafta 0 ve rafta 1/kapasite 100 testleri eklenmeli. Atak marka gelirinin milyonlara çıkması tek başına hata değil; bugünkü kaynak şartı tutarı denge kararı için şüpheli bırakıyor.

## Menü ve kalan teslim istekleri

Galeri: `Saved/Screenshots/Menu/20261001-154910/index.html`, **92/92 PNG**, iki tema/iki çözünürlük; hepsi gözle incelendi, kampanya değişmedi, boyut hatası 0. Banka, kısa gelir tablosu, satın alma kartı, harita, yönetim ve güncel kutlamalar çekildi.

En önemli üç sorun: harita katman tepsisi alt menünün arkasında; küçük/yıllık gelir tablosu tutarları kesiliyor; sınırsız eski kredi listesi şirket banka kartını ilk görünümün dışına itiyor. Dosya:satır ve çözüm istekleri A.md C4 bölümünde. Ek olarak 720p aile müdürü yer açıklaması kesiliyor, harita renk anahtarı yok. Gerçek kampanyada bağlı şirket/açık uzak şube olmadığından dolu satırların görsel doğrulaması açık kaldı.

**M27 istek:** C4 Banking/Rescues kayıt alanlarını eklediği hâlde CurrentVersion hâlâ C3'ün 3 değeri (`MarketEconomy.h:749`). Yeni kayıt biçimi için sonraki birleşimde sürümü bir kez artır ve C3 sürümünü reddetme testi yap; eski kayıt dalı yazma. Bu turdaki derleme/test düzeltmesi yetkisini mantık değişikliği için kullanmadım.

## Ortalama ilk 180 gün gider dökümü (TL)

Tam süreli 10 yıl örnekleri; her şube eşit ağırlıkta. Ayrı fire sütunu bilgi amaçlıdır, brütten tekrar çıkarılmaz.

| Tür | Ciro | Brüt | Kira | Ücret | SGK | İşletme | Lojistik | Fire | Net |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| buyuk | 341.219,05 | 128.862,77 | 6.232,75 | 51.025,24 | 11.479,85 | 5.995,85 | -1.990,44 | 5.950,03 | 56.119,51 |
| hiper | 959.332,33 | 317.244,24 | 32.383,35 | 181.967,54 | 40.940,95 | 13.137,52 | -7.444,70 | 22.787,61 | 56.259,59 |
| mahalle | 115.517,52 | 40.309,05 | 2.563,51 | 21.617,43 | 4.863,94 | 3.007,27 | 538,59 | 618,55 | 7.718,32 |

Lojistik ortalamasının negatif olması defterde maliyet indirim/iade kayıtlarının net etkisidir; eksi maliyet sütunu sıfıra kırpılmadı.
