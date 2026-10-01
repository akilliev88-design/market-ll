# C7 — kurtarma ve menü sadeleştirme doğrulaması

Son kaynak: c17760a, teslim 02.10.2026.

Borç şişmesi önemli ölçüde azaldı; kurtarma sonrası büyüme henüz düzelmedi. 30 yıllık üç kampanyada 204 kurtarmanın 200’ü ödenmeden yeni plana çevrildi. Kurtarma gören hiçbir kampanya sonradan yeni şube açmadı. Oyun sabitleri ve C menüsü değiştirilmedi.

DERLE, 150/150 otomasyon testi ve gerçek dünya Smoke başarılı. Claude kodunda bir derleme düzeltmesi: MarketManagersTests.cpp:1 eksik MarketStaff.h. Defterin işaretli/mutlak kasa farkı ve bağımsız satış/stok denetimi bütün kampanyalarda 0.

30 yıl × üç tarz × tohum21; 10 yıl × üç tarz × tohum21/22/23: 12 kampanya, 65.751 gün. İlk on yıllar tekrar içerir; bağımsız 12 deney değildir. Bütün para tutarları nominal TL, enflasyon içerir. Bot kurtarma sürerken şube/kredi denemez; süre bitince mevcut ağ yedeği kuralına döner.

## Mustafa için sonuçlar


| Süre/yıl | Tarz | Tohum | Plan | Ödenen | Yenilenen | Silinen kredi TL | İlk yeni şube |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 30 | Temkinli | 21 | 55 | 0 | 54 | 14.799.244,12 | yok |
| 30 | Dengeli | 21 | 39 | 1 | 37 | 12.276.665,26 | yok |
| 30 | Atak | 21 | 110 | 0 | 109 | 51.441.003,72 | yok |
| 10 | Temkinli | 21 | 10 | 0 | 9 | 348.432,57 | yok |
| 10 | Temkinli | 22 | 11 | 0 | 10 | 378.850,21 | yok |
| 10 | Temkinli | 23 | 11 | 0 | 10 | 409.627,78 | yok |
| 10 | Dengeli | 21 | 3 | 1 | 1 | 114.347,33 | yok |
| 10 | Dengeli | 22 | 8 | 0 | 7 | 402.848,90 | yok |
| 10 | Dengeli | 23 | 5 | 1 | 3 | 228.038,76 | yok |
| 10 | Atak | 21 | 33 | 0 | 32 | 2.238.972,72 | yok |
| 10 | Atak | 22 | 0 | 0 | 0 | 0,00 | uygulanamaz |
| 10 | Atak | 23 | 0 | 0 | 0 | 0,00 | uygulanamaz |


30. yıl sonunda açık son planın kalan kredi borcu: Temkinli 810.089,00 TL, Dengeli 833.295,00 TL, Atak 1.552.133,00 TL. Önceki planı yeni krediyle kapatmak ödeme sayılmadı. Plan görmeyen atak22/23 için kurtarmadan sonra şube ölçüsü uygulanamaz.

C5 30 yıllık toplam **321 → 204**, on yıllık **235 → 81** plan. C5’te planın ödendiği/yenilendiği ayrımı ölçülmedi; bu iki sütun için eski sürüme karşı fark iddiası yok. Silinen kredi nakit gelir değildir, tedarikçi/vergi cezaları bu ölçüme dahil değildir. Her planın günü, engelin sonu, anaparası, kalan borcu ve ödeme/yenileme günü kurtarma.csv’de; 0 tarih “olmadı” demektir.


| 30 yıl/tohum21 | İlk kurtarma günü | En kısa ara | Ortanca ara | 180 günden kısa | 730 günden kısa |
| --- | --- | --- | --- | --- | --- |
| Temkinli | 1664 | 131 | 161.5 | 41 | 54 |
| Dengeli | 584 | 155 | 200.0 | 10 | 37 |
| Atak | 236 | 76 | 93 | 109 | 109 |


Atak botta 109 yenileme aralığının tamamı ilk taksit için tanınan 180 günden kısa; ortanca 93 gün. Borç planı birikmiyor ama ilk taksit gelmeden yeniden kurulup iki yıllık büyüme yasağını uzatıyor. Süre dolmadan ödeme ile süre dolup yeniden açma farklıdır; son borcu sıfırlanan bir plan bile şube yasağını hemen kaldırmıyor.


| Tarz/tohum21 | Tam oyun yılı | Toplam borç TL | Aile dükkânı yıllık ciro TL |
| --- | --- | --- | --- |
| Temkinli | 10 | 42.512,00 | 161.896,48 |
| Temkinli | 20 | 262.404,00 | 923.839,30 |
| Temkinli | 30 | 818.508,33 | 2.246.646,98 |
| Dengeli | 10 | 67.344,84 | 347.052,99 |
| Dengeli | 20 | 305.099,10 | 1.341.183,45 |
| Dengeli | 30 | 1.020.647,72 | 3.640.877,80 |
| Atak | 10 | 100.761,00 | 176.160,45 |
| Atak | 20 | 580.563,83 | 1.077.913,50 |
| Atak | 30 | 1.772.826,95 | 2.813.308,40 |


Toplam borç = miras + aile/banka kredisi + açık toptancı faturası + vergi. Ciro, son günün değeri değil tam oyun yılının aile dükkânı cirosudur. C5’in 30. yıl yıllık aile cirosu temkinli 132.740,45; dengeli 120.228,95; atak 1.533.476,55 TL idi: “ciro ~0” ifadesini tam yıllık ciro olarak yorumlamıyoruz. C5 son borçları sırasıyla 461.664.261,80 / 529.332.879,72 / 490.023.079,64 TL idi.


| Dengeli21 yıl | Ulusal sıra | Dünya sırası |
| --- | --- | --- |
| 10 | 35 | 25 |
| 20 | 33 | 32 |
| 30 | 31 | 32 |


Bu sıralar C5 ve C6 ile aynı. Dengeli ilk3 / dünya ilk10 / 30. yılda liderlik hedefleri karşılanmadı. Temkinli21 bütün 30 yılda üst yönetim ücretine 7.286.057,46 TL, aile personeline 7.276.534,75 TL ödedi. Rescue aile müdürünü ve deposu varsa depo müdürünü koruyor (MarketFinance.cpp:399); A Manage atamalarını mevcut nakit yedeğine göre sürdürür. Aile/merkez gideri ve bu bot kararları ayrıştırılmadan sonuç yalnız finans sabitlerine yüklenmemeli.


| 10 yıl | Tohum | Mağaza | Kasa TL | Toplam borç TL | Aile yıllık ciro TL |
| --- | --- | --- | --- | --- | --- |
| Temkinli | 21 | 1 | 2.054,45 | 42.512,00 | 161.896,48 |
| Temkinli | 22 | 1 | 17.548,94 | 48.744,00 | 143.777,52 |
| Temkinli | 23 | 1 | 951,37 | 56.959,00 | 149.713,79 |
| Dengeli | 21 | 1 | 6.831,59 | 67.344,84 | 347.052,99 |
| Dengeli | 22 | 1 | -22.485,02 | 58.374,16 | 256.801,79 |
| Dengeli | 23 | 1 | -1.801,90 | 68.936,39 | 412.616,96 |
| Atak | 21 | 1 | 803,79 | 100.761,00 | 176.160,45 |
| Atak | 22 | 104 | 4.805.523,67 | 1.553.834,39 | 506.921,14 |
| Atak | 23 | 107 | 5.786.655,34 | 1.403.304,54 | 682.441,31 |


Atak21/22/23 mağaza **1/104/107**; C5 **1/131/138**, C6 **1/106/112**. Başarılı iki tohum genel denge kanıtı değil; C5→C7 farkı yalnız bir sabitin etkisi değildir, M33/M34 ve bot bağlantıları da değişmiştir.

## İnternet: 10. yıl


| Tarz | Tohum | Bizim ciro % | Ülke % | Sipariş | Net TL | Karanlık depo |
| --- | --- | --- | --- | --- | --- | --- |
| Temkinli | 21 | 0.2885 | 2.41 | 12 | 49,07 | 0 |
| Temkinli | 22 | 5.0203 | 8.00 | 74 | -1.166,98 | 0 |
| Temkinli | 23 | 6.8548 | 9.84 | 93 | -766,31 | 0 |
| Dengeli | 21 | 0.3615 | 2.41 | 23 | 89,45 | 0 |
| Dengeli | 22 | 2.6636 | 8.00 | 84 | -1.653,33 | 0 |
| Dengeli | 23 | 2.8239 | 9.84 | 104 | -930,30 | 0 |
| Atak | 21 | 0.0000 | 2.41 | 0 | 0,00 | 0 |
| Atak | 22 | 0.4171 | 8.00 | 19571 | 72.390,90 | 16 |
| Atak | 23 | 0.2487 | 9.84 | 15157 | -46.568,90 | 14 |


Pay = internet / (aile + fiziksel şubeler + internet), bağlı şirket toplu cirosu hariç. Net ortak gider sonrası faaliyet sonucu; kurulum yatırımı hariç. Dört kanal satırında tekrar eden ortak net/gider/mağaza ciro yıl başına bir kez alınır. Kanal katkısı ortak giderden önce; internet.csv ve ham raporda açılış/salgın uzaklığı, sipariş ve kanal katkıları ayrıntılıdır.

C5: 275 il kartı; en çok aynı tam il kimliğinde 33 tekrar (Atak23, online.area:tr|sanliurfa).

C7: 136 il kartı; en çok aynı tam il kimliğinde 10 tekrar (Atak22, online.area:tr|ordu).

Komuta açma kartı ayrıca 431; aynı ilde en çok 16. İnternet il kartıyla birleştirilmedi. Kimlikte ülke ve il birlikte sayılır.

## İnternetsiz temkinli: eşleşmiş fark

C7 talimatındaki iki asıl seri çalıştırıldı; bu tur internetsiz kontrol yeniden oynatılmadı. C5 eşleşmiş farkı eski sürümün sonucu olarak kalır; C7 internetinin nedensel etkisi diye taşınmaz.

## C4 başlıkları


| Tarz | Tohum | Kapanan şube | Limit ihlal ayı | Zincir teklif/kabul/ret | Şirket faiz TL |
| --- | --- | --- | --- | --- | --- |
| Temkinli | 21 | 1 | 0 | 0/0/0 | 0,00 |
| Temkinli | 22 | 1 | 0 | 0/0/0 | 0,00 |
| Temkinli | 23 | 1 | 0 | 0/0/0 | 0,00 |
| Dengeli | 21 | 5 | 108 | 0/0/0 | 5.040,47 |
| Dengeli | 22 | 5 | 81 | 0/0/0 | 10.556,83 |
| Dengeli | 23 | 5 | 107 | 0/0/0 | 6.497,77 |
| Atak | 21 | 4 | 118 | 0/0/0 | 1.267,52 |
| Atak | 22 | 56 | 5 | 3/2/1 | 3.476.825,18 |
| Atak | 23 | 70 | 4 | 4/2/2 | 3.295.639,65 |



| Tür | Tam180 örnek | Ortalama net TL | Zararlı | Eksik dönem |
| --- | --- | --- | --- | --- |
| mahalle | 93 | 8.528,61 | 6 | 39 |
| buyuk | 17 | 54.932,93 | 0 | 1 |
| hiper | 192 | 91.671,76 | 15 | 15 |


Kira/ücret/SGK/işletme/lojistik/fire sube180.csv’de ayrı; yatırım hariç, nominal/enflasyon ve erken kapanış seçimi ortalamayı etkiler. İki serinin c3.csv satırlarında defter farkı 0, sıkıcı dönem 0, en uzun sessizlik 19 gün. Haber/karar sıklığı keyifli oynanışın kanıtı değildir.

### Hiper reyonlar: şube başına 30 günlük kâr


| Hiper reyon | C5 ort. TL | C7 ort. TL | Zararlı grup | En düşük TL | En yüksek TL | Şube-gözlem |
| --- | --- | --- | --- | --- | --- | --- |
| Balık | -354,08 | 28,02 | 9/28 | -1.212,18 | 1.890,45 | 780 |
| Ev | 407,19 | 731,52 | 2/98 | -75,80 | 1.402,74 | 4307 |
| Kırtasiye | 24,98 | 379,59 | 5/11 | -239,64 | 1.836,43 | 303 |
| Bebek | -73,07 | -175,78 | 4/4 | -416,38 | -34,35 | 66 |


Ölçü her tür/gün grubunda toplam Last30Profit / açık reyonlu şube sayısı; ortalama şube sayısıyla ağırlıklı. Negatif grup tek tek zarar eden şube sayısı değildir; CSV grup verisi sağlıyor. Tekrarlı 30 günlük pencereler ve açılış dönemleri bağımsız olgun şube deneyi sayılmaz. 30 yıllık tohum21 hipere ulaşamadı; bu örnekler on yıllık atak22/23’ten geliyor.

### C6: reklam ve komuta


| Atak/tohum | Kanal harcama TL | Tahmini ek ciro TL | Müdür TL |
| --- | --- | --- | --- |
| 22 | 37.193.770,96 | 141.733.656,33 | 275.413,69 |
| 23 | 38.589.677,36 | 148.261.466,71 | 292.373,94 |


Tahmini ek ciro gerçek kâr/karşılaştırmalı reklam getirisi değildir; aramanın ayrı internet ek cirosu ölçülmedi. Kanal, ülke ve takvim yılı reklam.csv’de; gerçek kart/onay/ret, sokak kapanışı ve stok eritme olayları komuta_olaylar.csv’de. M33/M34 oyun sabitleri değiştirilmedi.

## C’ye en önemli beş denge isteği

1. MarketFinance.cpp:289,475: aktif planı yeniden başlatma kuralını sınayın. 30 yılda 200 yenileme; atak 109 ara <180 gün. Aktif plan içi ikinci yardım mevcut krediyi/engelin bitişini sıfırlamadan hangi koşulda yapılacak, önce karar ve test.
2. MarketFinance.cpp:440–450: Fresh tutarı Bearable kapasitesini aşabiliyor; Kept’i sıfırlamak yeni işletme parasını taşınabilir yapmıyor. Taksit sonrası aile faaliyet nakdini, ilk taksit ve 730. gün etrafında izleyen deney; yüzdeyi körlemesine değiştirmeyin.
3. Kurtarma gören hiçbir kampanya yeni şubeye dönmedi. Dengeli ulusal35/33/31, dünya25/32/32 değişmedi. Önce aile gider/satış ve plan sonrası yedek birikimini düzeltin; lig sıkıştırmasını tek başına artırmak gerçek büyüme yaratmaz.
4. İl döngüsü 33→10 iyileşti fakat 136 internet il kartı ve 431 komuta açma kartı sürüyor (komutada aynı il16). Önerinin açılış yatırımını, ağ yedeğini ve son ret/kapanışın nedenini hesaba katıp iki döngüyü birlikte sınayın.
5. Hiper bebek C7 4/4 gözlem zararlı, ortalama -175,78 TL/şube; balık +28,02’ye çıktı fakat 9/28 grup zararlı. Reyon/personel/fire yükünü olgun şubede ve başlangıç döneminde ayrı ölçün; ev/kırtasiye iyileşmesini tüm reyonlara aynı marj artışı diye genellemeyin.

## Ham raporlar

- [30 yıllık üç kampanya](../../../Saved/AutoPlay/20261002-001412/rapor.md)
- [10 yıllık dokuz kampanya](../../../Saved/AutoPlay/20261002-001737/rapor.md)
- [Üç dönem menü galerisi](../../../Saved/Screenshots/Menu/C7_20261001/index.html)

Kurtarma günlerinin tamamı ham kurtarma.csv’de; yıl10/20/30 ve diğer yıllar kurtarma_yillar.csv’de. Tüm para CSV’de kuruş. Ham çıktılar Saved altında yerel; takip edilen teslim bu rapor ve A.md.
