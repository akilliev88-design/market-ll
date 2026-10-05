# Dört mağazanın yeni yerleşimleri

05.10.2026, Codex / G-116. Mevcut Blender raf kütüphanesiyle oyunda uygulanmış planlar.
Bina, depo, giriş ve sabit kolonlar korunur. Ölçüler oyun için seçilmiştir; gerçek
zincirlerin birebir kat planı veya bir erişilebilirlik mevzuatı standardı değildir.

## İncelenen birincil kaynaklar

- [REWE örnek mağaza konsepti](https://rewe-expansion.de/vertriebsformate/muster-marktgrundriss/)
  ve [yayımladığı kat planı](https://rewe-expansion.de/wp-content/uploads/2023/11/Marktgrundriss_REWE.pdf):
  girişte manav, kısa raf sıraları ve çapraz bağlantı, çevrede taze servis, kasa önü fırın.
  PDF görsel olarak incelendi; yalnız Saved/LayoutResearch altında araştırma kopyasıdır.
- [REWE To Go tasarımcısının proje açıklaması](https://www.c-e.design/en/rewe-to-go):
  150 m² kompakt formatta hemen tüketim ve ev için alışveriş alanlarının ayrılması;
  küçük alanda anlaşılır rota ve modüler ekipman.
- [Lawson Store 100 çalışma rehberi](https://crew.lawson.co.jp/lawsonstore-100/works.html):
  sınırlı alanda girişten arkaya düzenli, kolay bulunabilir ürün sunumu.
- [Walmart mağaza yönlendirmesi](https://corporate.walmart.com/news/2020/09/30/reimagining-store-design-to-help-customers-better-navigate-the-omni-shopping-experience):
  bölüm tabelaları, koridor numaraları, ayrı teknoloji/oyuncak alanları ve hızlı ödeme.
- [Walmart manav yenilemesi](https://corporate.walmart.com/news/2019/11/20/to-better-showcase-our-quality-produce-were-refreshing-our-shopping-experience):
  alçak manav teşhirleri, açık görüş ve daha geniş dolaşım.
- [Lidl Strood başvuru planı](https://strood.expansion.lidl.co.uk/wp-content/uploads/2021/08/site-layout-and-CGI.pdf):
  satış, kasa, depo, soğuk depo ve personel alanları ayrı. İkinci sayfa incelendi;
  bu çizimde ayrıntılı raf dizimi verilmediğinden iç koridor ölçüsü buradan çıkarılmadı.

## Oyundaki karşılıkları

| Mağaza | Yerleşim | Ekipman | Kasalar |
|---|---|---:|---:|
| mahalle_01 | Üç kısa günlük gıda sırası, girişte alçak manav, arkada süt/donuk; ayrı bakım/temizlik duvarı | 20 | 1 |
| kucuk_01 | Beş kompakt sıra, arka bağlantı koridoru ve süt/ekmek, kolonun etrafında açık geçiş, girişte manav | 42 | 2 |
| buyuk_01 | Manav meydanı, iki parça gıda sıraları, 2,2 m enine geçiş, ayrı bakım/ev, çevrede süt/fırın/şarküteri | 132 | 4 klasik + 2 self servis |
| hiper_01 | Üç parçalı uzun gıda sıraları, iki 2,4 m bölüm caddesi; ayrı manav, donuk, servis, bakım ve temizlik; ev/teknoloji teşhir hattı | 400 | 12 klasik + 6 self servis |

Kasa önlerinde kuyruk alanı ayrılır. Mağaza girişleri kuyruklara açılmaz.
Self servis ekranları kuyruk tarafındaki müşteriye dönüktür. Kısa mahalle kuyruğu
1,2 m; diğer planlarda ayrılan kuyruk uzunluğu 3 m'dir. Raf uçları
bağlantı koridorlarına bakar. Yapışık raf modülleri arasındaki küçük montaj aralıkları
yürüme koridoru sayılmaz. Büyük mağaza bölüm tabelaları yaklaşık 3,2 m'de okunur;
eski yüksek tavan altındaki küçük tabelalar kullanılmaz.

Yeni kütüphaneden kozmetik ada/duvar, ahşap duvar, gondol/raf başı, alçak ve iki
katlı manav, ekmek/pasta, üç kapılı içecek ve dik donuk dolapları; hipermarkette
cam eşya, küçük ev aleti, elektronik masa, ayakkabı ve hırdavat teşhirleri kullanılır.
Büyük deterjan kutusunun sığması için küçük markette yüksek açıklıklı mevcut
240 cm duvar rafı seçildi; sığ kozmetik rafına büyük ambalaj zorlanmadı.

Her raf yüzünün ayrı kategorisi olabilir. Gıda ve temizlik aynı gondolda paylaşılmaz.
Mevcut kataloğun kimlikleri korunur. Taze meyveler görsel doldurmadır; katalogda
satılabilir ürünü olmayan fırın/ev/teknoloji rafları boş kalabilir. Bu çalışma bu
kategorilere stok, fiyat veya yeni ürün eklemez; katalog ve ekonomi Claude alanıdır.

MAHALLE_MARKET_GEZI.cmd ayrıca 8×10 m sokak marketi sanat prototipini açar.
Ona da kısa raflar, manav, fırın ve bakım duvarı uyarlandı. Kampanya mağazasının
127,6 m² mahalle planıyla aynı bina değildir; MAGAZA_GEZI.cmd/F10 dört kayıtlı
planı gezer. Küçük/büyük/hiper ayrı CMD'leri yeni hazır planları açar.

## Tekrarlama ve kontrol

`Tools/design_world_store_layouts.py` mevcut bina üzerine yeni ekipman düzenini
yazar; katalog dosyasını yalnız okur. `Tools/validate_stores.py` çakışma, fiziksel
mağaza bantları, kategori ve gerçek ekipman cephe toplamlarını kontrol eder.
`Tools/check_store_routes.py` 20 cm ızgarada 35 cm yarıçaplı alışverişçi için
girişten her satış yüzünün üç noktasına bağlantıyı, kasa kuyruğunu ve tanımlı
enine koridorları kontrol eder. Oyuncunun gerçek kapsül yarıçapı 30 cm'dir.
Bu ızgara denetimi görsel modelin gerçek çarpışmasının yerine geçmez.
Hedef noktalarının açıklığı ayrıca tam koordinatında sınanır: yalnız en yakın
ızgara hücresinin boş olması yeterli değildir. Mahalle sepetliği bu ikinci
kontrolde/gerçek kapsülde yakalanıp 30 cm kaydırıldı; hipermarket araba ve
sepet alma yüzleri içeri çevrildi.

32 farklı ekipman türü dört formatta uygun reyonlarda kullanılır. Altı yeni rafın
teslimde sabit 25 cm yazılmış kat açıklıkları gerçek kat yüksekliği aralıklarından
(bir üst raf için 2 cm pay bırakılarak) hesaplandı. Üst kat, ölçülmüş dolap boyunu
aşmaz. Model/FBX geometrisi bu düzeltmede yeniden üretilmedi.

Hazır mağaza doldurması, yüz kategorilerini yeniden atayan şube planlayıcısından
ayrıldı: seçilmiş yüz kategorileri korunur, her ayrı ürüne önce yer ayrılır, sonra
tekrar teşhirleri eklenir. Aynı elle dizme geometri kuralları kullanılır. Eski yolun
yanlış yüzdeki tek bloğu sonradan silip ürünü tamamen kaybetmesi düzeltildi.
Ekonomi stok yaratılmaz, fiyatlar ve ürün kimlikleri değişmez. Genel şube yerleşim
hesabı MarketLayout değiştirilmedi.

`MarketStoreWalkAudit.cpp` ayrı gezi dünyalarında gerçek Unreal kapsülüyle aynı
satış yüzlerini kontrol eder. LargeStoreReview giriş/zemin ve sabit model kontrolünü,
StoreTourTest rastgele doldurma ve para yalıtımını; HandmadeNeighborhood prototip
giriş/cam/zemin kontrollerini çalıştırır. Görüntüler ayrıca gözle incelenir.
`Tools/LargeStoreReview.ps1 -AllFormats` mahalle planını da dahil ederek dört
formatın 2.997 raf önü kapsül noktasını ve 32 kamera görüntüsünü kontrol eder.

`Tools/sync_world_store_tours.py` dört yerel planı yüklerken var olan dosyaları
önce `Saved/StoreBackups/BeforeWorldLayouts_<tarih>` içine kopyalar. İlk yedek
20261005_165959; küçük mağaza düzeltmesi öncesi ikinci yedek 20261005_171012.
Önceki kişisel düzenleri geri almak bu yedeklerden mümkündür.

`Tools/render_store_layouts.py` gerçek koordinatlardan aşağıdaki planı üretir;
araştırma kaynağının çizimini kopyalamaz.

![Dört oyun mağazasının gerçek yerleşimleri](../Images/StoreLayouts/20261005/layouts.png)

Son doğrulama: DERLE geçti; Unreal TEST 175/175, Smoke ve dört normal StoreTour
geçti. 2.997 gerçek kapsül noktası, dört olumsuz rota regresyon testi ve dört
formatın 32 görüntüsü kontrol edildi. Mahalle prototipinde ayrıca 42 kapsül
noktası, giriş/cam/zemin ve üç görüntü geçti. Son sonuçlar
[verification.json](../Images/StoreLayouts/20261005/verification.json) içinde.

Oyun görüntüleri: [mahalle](../Images/StoreLayouts/20261005/mahalle_01_interior.png),
[küçük](../Images/StoreLayouts/20261005/kucuk_01_interior.png),
[büyük](../Images/StoreLayouts/20261005/buyuk_01_interior.png),
[hiper](../Images/StoreLayouts/20261005/hiper_01_interior.png),
[self servis yönü](../Images/StoreLayouts/20261005/buyuk_01_checkout.png),
[Blender mahalle prototipi](../Images/StoreLayouts/20261005/handmade_neighborhood_02.png).
