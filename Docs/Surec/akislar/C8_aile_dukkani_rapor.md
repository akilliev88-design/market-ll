# C8 — aile dükkânı tanısı (02.10.2026, Codex)

**Sonuç:** büyümesi kapalı dükkân sekiz yıl boyunca yıllık FAVÖK bakımından kârlı. Sekizinci yıl aile FAVÖK'ü **88.812,11 TL**, patron/merkez dahil şirket FAVÖK'ü **65.278,75 TL**; kasa **232.489,63 TL**, borç ve kurtarma **0**. Normal temkinli botta ilk tam zarar yılı **6**: aile FAVÖK **−7.079,96 TL**, şirket **−28.429,23 TL**. Tek dükkânın kaçınılmaz biçimde zarar ettiği varsayımı bu koşuda doğrulanmadı.

## İlk iki sayfa: üç ana neden ve öneri

1. **İlk şube botun fiyat indirimini kaldırıyor; müşteri hacmi düşüyor.** İkinci yılın Ekim→Kasım geçişinde aynı sepet ortalama raf fiyatı **4,64934→5,48076 TL (+%17,9)**, alış maliyeti **3,32279→3,36424 TL (+%1,25)**. Yerel pay **%34,74→%14,87**, gelen müşteri **1.930→1.136**, ciro **37.097,50→18.639,60 TL**. Bot indirim koşulu `State.Branches.IsEmpty()`; kapanmış şube de bu dizide kaldığı için tekrar tek dükkâna dönmek indirimi geri getirmiyor. Bu bir **bot politikası sorunu adayı**. Kaynak: `MarketAutoPlay.cpp:190–195`; fiyat çekiciliği `MarketCompetitors.cpp:38–40`, pay `:390`, trafik `:298`, oyun bağlantısı `MarketDirector.cpp:31–37`. Şube yamyamlığı da `MarketBranches.cpp:462` üzerinden etkili; bu iki etki bu deneyde ayrı ölçülmedi. **Öneri:** aynı tohumla fiyat politikasını sabit tutan eş koşu ve açık şube sayısıyla indirim kapısını sınayan bot deneyi; oyun fiyat/talep sabitlerini değiştirme.

2. **Azalan ciroya ek İK ve aile müdürü gideri bindiriliyor.** 302. gün mali müşavir Fatma; 610. gün kasiyer Elif, İK Zeynep ve aile müdürü Esra Güler (seviye 5) kayda giriyor. Üçüncü yıl normal koşu ücret+SGK **58.089,35 TL**, tek dükkân **40.035,01 TL**: **18.054,34 TL fark**. Ayrıca aile müdürü **22.745,13 TL**, patronun şirkete maliyeti **18.770,04 TL**. Normal aile FAVÖK'ü **11.914,93 TL**, aile müdürü/patron da yüklenince **−29.600,24 TL**; tek dükkân aile FAVÖK'ü **64.037,03 TL**. Aile defteri müdür/patronu merkezde tutar; iki satır aynı kapsam değildir. Kaynak: `MarketAutoPlay.cpp:85` iki mağazada İK, `:95` uygun aile müdürü adayını atama. **Öneri:** bot işe almayı ek brüt kârın aylık toplam maliyeti karşılamasına bağlayan deney; mevcut nakit eşiği tek başına yeterli değil. Ekonomi/personel kuralı Claude alanında, burada değiştirilmedi.

3. **Nakit daralınca mal alımı kesiliyor, boş raf satışları da çökertiyor.** Altıncı yıl Eylül→Ekim: gelen **859→869** ile sabitken alışveriş yapan **566→149**; boş raf ürün isteği **414→1.699**, sipariş bedeli **3.910,02→0 TL**. Ekim ciro **1.823,45 TL**, aile FAVÖK **−5.836,83 TL**, şirket FAVÖK **−15.258,65 TL**, ay sonu kasa **−13.754,03 TL**. Kasım alan müşteri **28**, boş raf isteği **1.864**. Bu aylarda dönem `none`, bütçe/dönem bütçe çarpanı **1,0**; salgın veya kur şoku açıklaması değil. Kaynak: `MarketAutoPlay.cpp:160–179` sipariş nakit sınırı ve asgari sipariş; `MarketFinance.cpp:250` çevresindeki nakit sıkıntısı adımları. **Öneri:** sipariş için korunmuş işletme sermayesi ve stok erimesi başlamadan erken uyarı deneyi; kurtarma taksit süresini uzatmak tek başına satış döngüsünü düzeltmiyor.

**Maliyet raf fiyatını aşmıyor.** Normal yıllık brüt marj 2/3/6. yıllarda **%27,60 / %36,94 / %34,67**; raf−alış marjı **%30,40 / %37,61 / %35,24**. Üçüncü yıl müşteriler **18.957→13.027**, alan **16.632→10.607** (ikinci yıldan); hacim kaybı ana sorun. Tek dükkân sekizinci yıl **%24,61** brüt marjla daha düşük birim marjına rağmen kârlı. Fiyat, personel, şube ve kurtarma birlikte değiştiği için bu gözlemler tek başına nedensel etki büyüklüğü değildir; önerilen kontrollü koşular sonraki adımdır.

## Sekiz yıllık karşılaştırma

Tutarlar nominal TL. Yıl, 7 Mart 2011 başlangıcının yıldönümüne göre tam oyun yılı; aylık CSV takvim ayıdır. İlk/son takvim ayı kısmidir (97 satır). FAVÖK = defter neti − faiz − vergi satırları; şirket bütün mağazaları ve merkez/patron giderini kapsar. Brüt kâr fire/kayıp etkisini içerir.

| Oyun yılı | Normal aile FAVÖK | Normal şirket FAVÖK | Tek dükkân şirket FAVÖK |
|---|---:|---:|---:|
| 1 | 28.947,15 | 20.035,72 | 20.035,72 |
| 2 | 32.183,00 | 20.463,70 | 43.614,31 |
| 3 | 11.914,93 | 5.890,03 | 49.935,72 |
| 4 | 10.439,08 | 6.768,95 | 56.309,07 |
| 5 | 19.733,94 | 13.316,94 | 64.814,18 |
| 6 | −7.079,96 | −28.429,23 | 66.414,12 |
| 7 | 26.694,58 | 17.983,80 | 64.975,58 |
| 8 | 21.364,04 | 9.691,43 | 65.278,75 |

İlk negatif aile ayı dördüncü yıldaki Haziran: **−194,56 TL**; şirket **−1.424,34 TL**. Normal sekizinci yıl kasa **1.176,22 TL**, toplam borç **38.873,05 TL**, bir kurtarma (2093. gün); tek dükkânda negatif kasa günü yok. İlk 366 gün iki koşu aynı; normal sekiz yıl, on yıllık temkinli koşunun ilk 2922 günüyle aynı.

## Bot ve ölçüm teslimi

Claude C8/M36–M38 kaynağı `0ef2363`, C7 birleşimi `0426229`; kullanılmayan `MarketCredit.h/.cpp` git rm ile kaldırıldı. Claude kaynak derleme/test düzeltmesi **0**. Oyun sabitleri, ekonomi/menü kaynakları ve kayıt uyumu değiştirilmedi (CurrentVersion zaten 5).

Bot `OwnerSalary`/`Dividend` Director komutlarını kullanır: temkinli **1,5×**, dengeli 10 mağazada **3×**, atak 30 mağazada **5×**. Aktif kurtarmanın maaş düşürmesi korunur; plan bitince tarz maaşına döner. Ocakta yılda bir **%25 kâr payı**, dağıtımdan sonra üç ağ yedeği kalırsa uygulanır. Bu tohumdaki dengeli/atak ağlar üst maaş eşiklerine ulaşmadı. `NetworkReserve` kira ve patron maliyetini de içerir.

| Koşu / oyun yılı | Yıllık net maaş TL | Yıllık net kâr payı TL | Yıl sonu kişisel servet TL |
|---|---:|---:|---:|
| Normal / 8 | 13.037,49 | 0 | 0 |
| Tek dükkân / 8 | 18.775,95 | 10.721,90 | 81.701,13 |
| Temkinli / 10 | 2.817,74 | 0 | 0 |
| Dengeli / 10 | 15.497,60 | 0 | 388,11 |
| Atak / 10 | 5.635,49 | 0 | 0 |

Servet toplam ödenen maaş değildir; yaşam giderleri düşülmüş birikimdir. Ödenmeyen maaşları otomatik ödenmiş saymadık; bütün yıllar CSV'de.

`-NoGrowth -Style=0` şube/ağ büyümesini ve gönüllü krediyi kapatır; normal fiyat, stok, çalışan, olay ve günlük defter sürer. NoGrowth denetimi her gün tek mağazayı sınar. Yeni `AutoPlay.SingleShopDiagnosis`: deterministik 60 gün, günlük ölçüm, ürün sepeti, tek mağaza ve gönüllü borç sıfır testi geçti. Gerçek sipariş ürün/koli ve kadro/patron değişimleri kaydedildi; aylık ürün sepeti aktif raf ürünlerinin eşit ağırlıklı ortalamasıdır, satış ağırlıklı fiyat değildir.

Takip edilen veri [C8_veri](C8_veri/olcum.json): `normal_aylik/yillik.csv`, `tek_dukkan_aylik/yillik.csv`, iki `urunler.csv` (aynı ürün fiyat/liste/alış/stok maliyeti/hedef), iki `kadro_ve_patron.csv`, iki `siparisler.csv` (ay/ürün koli ve bedel), `uc_tarz_yillik.csv`, `C8_kurtarma.csv`. Bütün para sütunları kuruş; müdür `gunluk_maliyet` alanı aylık/yıllık tabloda günlük maliyetlerin **toplamıdır**. Patron maaşı net ödeme, ayrıca şirkete brüt toplam maliyet; servet dönem sonu stok değeridir. Ücret/SGK aile personeli, yönetici toplamı bütün ağ ve aile müdürü ayrı alanlar. Ürün isteği kayıpları ayrı ürün denemeleridir; kısmi alışverişte de oluşabilir, kaybolan kişi sayısına toplanmaz.

Ham günlük CSV ve ayrıntılı raporlar A worktree `Saved/AutoPlay/20261002-020822` (normal8), `020828` (tek8), `020915` (üç tarz10). Üretici araçlar: `Tools/c8_aile_rapor.py`, `c8_teslim.py`; kaynak **9c45f93**. Toplam 16.803 gözlenen gün (normal8 temkinli10'un tekrar öneki), stok/satış ve işaretli/mutlak defter farkı **0**.

## Kurtarma: eşleşen on yıl × üç tarz × tohum21

| Tarz | C7→C8 plan | C7→C8 yenileme | Ortanca ara gün C7→C8 | İlk plan günü C7→C8 | Kurtarma sonrası yeni şube |
|---|---:|---:|---:|---:|---:|
| Temkinli | 10→3 | 9→2 | 220→680 | 1664→2093 | 0→0 |
| Dengeli | 3→2 | 1→1 | 1463,5→2369 | 584→879 | 0→0 |
| Atak | 33→32 | 32→31 | 96,5→105 | 236→184 | 0→0 |

Atak C8 araları **82–148 gün**, temkinli **214–1146**, dengeli tek ara **2369**. C7'deki **93 gün otuz yıllık** atak örneğiydi; eş on yıllık ortanca 96,5. Kurtarma sayısı küçük ağlarda düştü ama atak döngüsü ve kurtarma sonrası büyüyememe sürüyor. M36–M38 de değiştiğinden fark yalnız C8 kurtarma formülüne yüklenemez.

Üç tarzda reklam harcaması **0**, modelin tahmini ek cirosu **0**: harcama/ciro **tanımsız**, oran 0 veya 3,84 değildir. Bu koşular yeni reklam fiyatının getirisini ölçmedi. C6'nın 3,84 sayısı **tahmini ciro/harcama** idi (ters oran yaklaşık 0,26); gerçek kâr getirisi değil. Daha büyük ağla ayrıca sınanmalı.

## Menü karşılaştırması ve kalanlar

Main'de gerçek atak22, aynı günler **1 / 2522 / 3982**; 26 hedef × iki tema × iki boy × üç dönem = **312 PNG**. Her çekim `PASSED, campaign unchanged`, boyut hatası **0**. [Galeri](../../../Saved/Screenshots/Menu/C8_20261002/index.html). Kaynak klasörleri `20261002-015509_start`, `20261002-015630_before/after`. Kaldı sütunu A.md C7 tablosunda güncellendi; tarihsel Düzeldi sütunu aynen korundu.

Bu tur geç dönemler **bir açık mağaza**, üç kapanmış şube, bir depo ve bir depo müdürü içeriyor. C7'nin 113/104 mağazalı görüntüleriyle sayfa yapısı karşılaştırılabilir; uzun il/müdür listesi ve milyonlu tutarlar yeniden sınanmadı. Seçili sorun sayfaları gözle incelendi; 312 dosyanın tamamının görsel incelemesi iddia edilmiyor. Sıfır cirolu sahte sıralama, miras/borç kapsamı, günlük aile/ağ etiketleri, ilk gün rekorları, reklam özetinin üst konumu ve tıklanabilir reklam geçişi düzelmiş görünüyor.

En önemli üç kalan:
1. Kampanya formu ilk gün bütün kapsam/tür/oran/süre/gondol seçeneklerini açıyor; beklenen masraf/kâr kararın önünde yok.
2. Boş/kilitli yönetim alanı ve ilk gün reklam kanalları fazla yer kaplıyor; beceri/etkinliğin pratik sonucu kısa açıklanmalı.
3. Haftalık grafikte negatif değerler yukarı doğru büyüyor: `MarketMenuWidget.cpp:1796` Abs(Profit), alt hizalı ortak çizim. Alt sıfır çizgisi eklense de işaretli eksen gerekli.

Çekim notu: ilk gün üst görevli özet adları kadrodan farklı. `MarketMenuPages.cpp:1547` WorkerSummary ve `MarketWorkers.cpp:101–102` yalnız ID karşılaştırması, aynı ID'yle otomasyon kadrosu değişince eski dünya adını tutabiliyor. Bu otomasyon görüntü tutarlılığı adayı; normal oyundaki isim hatası kanıtı değil. Sonraki Codex çekim düzeltmesi olarak not edildi, bu tur menü/oyun kaynağı değiştirilmedi.

## Son doğrulama ve açık iş

Son kaynak **9c45f93**, A worktree: DERLE **geçti (76,00 sn)**; TEST **149 temiz + 1 motor HTTP uyarısıyla başarılı + 1 başarısız = 151**, çalışmamış **0**; Smoke **PASSED** (02.10 02:11). `Ledger.CashAudit`, Owner.SalaryAndWealth, Promotions.EveryStore ve yeni SingleShopDiagnosis geçti.

Tek başarısız `AutoPlay.LateCarefulGrowth`: `MarketAutoPlayCTests.cpp:96–106` 600 günlük koşuda şube bekler. Yeni kira/patron/başlangıç ekonomisi ve bot yedeğiyle açılış **609. gün**; 600. gün kasa **26.472,97 TL**, botun sonraki 30 günlük büyüme sırası 601, kurulum sonrası 609. Eşik değiştirilmedi. Bu ekonomik takvim/bot beklentisi uyuşmazlığı; para korunumu hatası yok. Kira tek başına neden ilan edilemez; M36–M38 birlikte değişti. **Claude/Mustafa değerlendirmesi:** 600 gün büyüme hedefi korunacaksa fiyat/personel/nakit önerilerini kontrollü deneyle ele al; test hedefi için ayrı ürün kararı ver. Görev tam yeşil doğrulanmış olarak işaretlenmedi.
