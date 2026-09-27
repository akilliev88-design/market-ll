# Miras Market — oyun tasarımı

> Bu dosya ilk prototipin v0.1 tasarım kaydıdır. Kullanıcının sonraki ayrıntılı planlama isteği için güncel ana belge: [Planlama v0.2](Planlama/00_OKUMA_REHBERI.md). Bu yönlendirme oyun kodunu değiştirmez.

Durum: tasarım v0.1. Bu belge hedef oyunu anlatır; uygulanmış özellikler README'de ayrıca belirtilir.

## Oyunun sözü

2011 yılında Lüleburgaz'da ailenden kalan tek şubeli marketi devralırsın. İlk gün rafı kendin doldurur, müşterinin alışverişini kasadan geçirirsin. Yıllar sonra o marketin tabelası farklı ülkelerde görünür. Her büyüme adımı önceki emeğin sonucudur: daha iyi tedarik, güven kazanan müşteriler, çalışanlarına devrettiğin işler ve rakiplerinden aldığın pazar payı.

Tek oyunculu, PC odaklı, birinci şahıs işletme ve şirket yönetimi oyunu. Oyuncunun tercihi karma deneyimdir. Çalışan sayısı arttığında günlük işleri elle yapma zorunluluğu azalır; oyuncu istediği zaman ilk marketine dönüp kasaya geçebilir.

Çalışma adı **Miras Market**. Oyuncu daha sonra şirketini yeniden adlandırabilir; ilk şubenin aile tabelasını korumak ayrı bir seçenek olur.

## Hikâye ve yer

Başlangıç yılı 2011 önerisidir. Gerçek Lüleburgaz'dan esinlenen, sokak ve işletmeleri kurgu olan bir mahalle kullanılır. İlk harita tüm ilçeyi değil, marketi, karşıdaki rakibi, teslimat yolunu ve birkaç komşu dükkânı içerir. Tarihsel birebir canlandırma iddiası yoktur.

Ailenden kalan dükkânın itibarı vardır ama raf düzeni eskimiş, nakit akışı bozulmuş ve tedarik ağı zayıflamıştır. Tapusu aileye ait olduğu için ilk işletmenin kira yükü yoktur; bakım, elektrik, personel ve tedarik ödemeleri vardır. Sonraki kiralık şubelerde kira ayrı bir giderdir.

Kurgu karakterler:

- **Nermin teyze:** Mahallenin hafızası. Veresiye ve müşterinin güveni üzerinden aile mirasını temsil eder.
- **Cem:** Aile marketini yıllardır tanıyan çalışan. İlk dönemde eğitim verir; ileride şube müdürü olabilir.
- **Selim:** Bölgedeki dağıtıcının temsilcisi. Hacim, düzenli ödeme ve görüşmelerle daha iyi koşullar açar.
- **Derya:** Büyüme sırasında katılan operasyon yöneticisi. Şube raporlarının ve görev devrinin yüzü olur.
- **Bereket Market sahibi:** İlk kurgu rakip. Fiyat, ürün çeşitliliği ve hizmet konusunda oyuncuya karşılık verir.

Açılış sahnesi: kepenk, eski anahtar, kasadaki küçük para, depoda karışık koliler, babadan/anneden kalan hesap defteri ve "Bu dükkânın müşterisini tanı" notu. Oyuncu önce bir düzen kurar, sonra dükkânın geleceğini seçer.

İlk hikâye gerilimi "satmak mı, devam etmek mi?" olur. Devam etmeyi seçtikten sonra iş kararları aile mirasını farklı biçimlerde taşır: uygun fiyatlı mahalle marketi, kaliteli yerel ürünler, hızlı büyüyen indirim zinciri. Bu tercihler farklı ama oynanabilir stratejiler olmalı; birinin her koşulda üstün olduğu tek yol tasarlanmamalı.

## Oyun döngüsü

| Ölçek | Oyuncunun kararı | Geri bildirim |
|---|---|---|
| Saniyeler | Ürün taşı, raf doldur, ödeme al | Raf dolar, sıra kısalır, nakit artar |
| Gün | Fiyat, sipariş, vardiya, kampanya | Kâr, nakit, stok kaybı, müşteri memnuniyeti |
| Hafta | Ürün karması, çalışan, donanım, tedarikçi | Tekrarlayan müşteri, daha iyi marj, daha az elle iş |
| Ay | Yeni şube, depo, finansman, bölge seçimi | Bölgesel pazar payı, yeni rakipler, yönetim yükü |
| Yıllar | Ulusal strateji, özel marka, yabancı pazar | Zincir büyüklüğü, marka değeri, sürdürülebilir kârlılık |

Her rutin işin zamanla otomasyon yolu olmalı. İlk gün öğretici olan koli taşıma, yüzüncü gün oyuncuya zorunlu tutulmaz. Yönetim ekranında verilen kararların mağazada görünür karşılığı bulunmalı: boşalan raf, yoğunlaşan kasa, değişen ürün karması ve çalışan hareketleri.

## Büyüme basamakları

| Aşama | İçerik | Açılmayı hak ettiren başarı | Yeni sorun |
|---|---|---|---|
| Aile dükkânı | Tek mağaza, az ürün, elle iş | Pozitif faaliyet sonucu ve müşteri güveni | Nakit ile stok arasında denge |
| Lüleburgaz zinciri | İkinci/üçüncü şube, müdür | Sürdürülebilir kâr, rezerv nakit, eğitimli ekip | Şubelerin birbirinin satışını azaltması |
| Kırklareli ve Trakya | Bölgesel depo ve sevkiyat | İstikrarlı hizmet, yeterli hacim | Rota, soğuk zincir, bölge farkları |
| Türkiye | Bölge müdürlükleri, merkezi satın alma, özel marka | Sağlam borç yapısı ve operasyon | Büyük rakipler, marka konumlandırması |
| Uluslararası | Ülke seçimi, yerel ortak/tedarik, para birimleri | Yönetim kapasitesi ve sermaye | Yerel alışkanlık, kur, ithalat süresi |
| Liderlik | Büyük ama dayanıklı şirket | Birkaç dönem boyunca birden fazla başarı ölçütü | Büyüklüğü verimli ve güvenilir tutmak |

Açılma koşulları yalnızca seviyeye veya paraya bağlı olmamalı. Çok para ama kötü hizmet, hızla şube açmanın riskini artırır. Kesin eşikler oyun testleriyle ayarlanır.

## Market içindeki işler

**Mal kabul:** Sipariş planla, teslimat saatini seç, koliyi indir, miktarı ve hasarı kontrol et. Son kullanma tarihi ve parti bilgisi olan malları uygun depoya yerleştir. İlk prototip bunu ertesi sabah depoya eklenen stokla temsil eder.

**Raf ve yerleşim:** Raf kategorileri, kapasite, fiyat etiketi, müşteri erişimi, koridor genişliği ve göz hizası önemlidir. Yerleşim modu çarpışmaları ve erişilemeyen rafları önceden gösterir. Kurulu düzen kaydedilip başka şubeye uygulanabilir.

**Tazelik:** Parti bazında son kullanma tarihi, önce eski partiyi satma, indirimli son gün rafı, fire ve soğutma. Farklı ürün grupları farklı depolama ister. Bozulma oranı anlaşılır olmalı; gizli kayıplarla oyuncu cezalandırılmamalı.

**Kasa:** Barkod, adet, tartılı ürün, ödeme yöntemi ve kuyruk. İlk öğreticiden sonra hızlandırma seçenekleri gelir. Fiyat kasaya ulaşmadan değişirse sepete alınırken gösterilen fiyat korunur. Kasa işini personele devretmek gerçek zaman kazandırır.

**Hizmet:** Temizlik, aydınlatma, arıza, müşteri soruları ve iade. Her olayın anlamlı seçim yaratması gerekir. Aynı basit işi sürekli tekrarlatan olay sıklığı sınırlandırılır.

**Yerel ilişkiler:** Veresiye limiti, ödeme günü, tanınan müşteri ve aile itibarı. Veresiye satışın gelir ve nakit üzerindeki etkileri ayrı gösterilir. Oyuncu tüm müşterilere sınırsız kredi vermeye zorlanmaz.

## Ürünler ve marka yapısı

Başlangıç grupları: süt ürünleri, içecek, çay/kahve, bisküvi/atıştırmalık, temel gıda, temizlik ve kişisel bakım. Genişleme sırasında manav, şarküteri, fırın ve hazır yemek açılabilir; her kategori kendine özgü maliyet ve operasyon getirir.

Ürün tanımı: sabit ürün kimliği, kategori, marka kimliği, ad, hacim/ağırlık, koli adedi, raf tipi, depolama sıcaklığı, raf ömrü, tedarikçi seçenekleri, vergi sınıfı ve görsel varlık referansları. Ürün tanımı ile şubedeki stok/parti durumu ayrı tutulur.

Kullanıcının tercihi doğrultusunda prototipte gerçek ürün adları görünür. Mevcut katalog: Sütaş, Coca-Cola, Ülker, Barilla, Çaykur ve Ariel. Şimdilik bunlar metin etiketidir; gerçek logo veya ambalaj görselleri eklenmiş değildir.

Gerçek ve kurgu isimler aynı ürün kimliğine bağlanır. F8 ile isim kümesi değişir; stok, fiyat ve kayıt etkilenmez. Gelecekte logo, ambalaj, reklam, sesli anons ve şirket isimleri de aynı marka paketinden seçilir. Gerçek şirketlere atfedilen kurgu davranışlar yerine rekabet karakterleri kurgu şirketlerle anlatılır.

2011 atmosferi için ürün boyutları, ambalajlar, araçlar, kasa cihazları ve dükkân tabelaları ayrı araştırılır. Tarihsel kaynak doğrulanana kadar fiyatlar ve ürün sürümleri kurgu olarak işaretlenir.

## Ekonomi ve finans

Para tam sayı kuruş olarak saklanır. Ticari kararlar TL biçiminde gösterilir. Ciro, kâr, nakit, borç ve stok değeri ayrı kavramlardır.

- Ciro = gerçekleşmiş satışların toplamı.
- Brüt kâr = ciro − satılan ürünlerin maliyeti.
- Faaliyet sonucu = brüt kâr − personel − kira/bakım − enerji − lojistik − diğer faaliyet giderleri.
- Nakit değişimi = tahsilatlar − ödemeler + finansman hareketleri.
- Stok almak kasadaki parayı azaltır; satılmamış stokun maliyeti aynı gün yeniden satış maliyeti sayılmaz.
- Raf ve şube yatırımları, borç anaparası ve finansman giderleri kendi hesaplarında tutulur.

Sonraki sürümlerde parti bazlı gerçek alış maliyeti, tedarikçi vadesi, kredi/faiz, vergi özeti, duran varlıklar, sigorta ve amortisman eklenir. Vergi/finans simülasyonu oyun sistemi olarak basitleştirilir; gerçek bir muhasebe ürünü olarak sunulmaz.

**Oyuncu kararları:** Düşük fiyatla müşteri kazanmak, yüksek marjlı ürüne alan ayırmak, stok bulundurmak için sermaye bağlamak, ödemeyi ertelemek için pahalı vadeli alım yapmak ve beklenen talebe göre kampanya seçmek.

**Talep:** Hane bütçesi, fiyat duyarlılığı, alışveriş sepeti, mevsim, yakınlık, ürün bulunurluğu ve hizmet geçmişi. Pahalı ürün her müşteriyi kaçırmamalı; müşterinin beklentisi fiyatı haklı çıkarabilir.

**Zorluk seçenekleri:** Rahat işletme, dengeli rekabet, zorlu ekonomi. Farklar rakip davranışı, talep belirsizliği, nakit rezervi ve hizmet toleransı üzerinden kurulur. Erişilebilirlik ile ekonomik zorluk birbirinden bağımsızdır.

**İflas ve toparlanma:** Otomatik büyük ceza yerine yaklaşan ödeme uyarısı, stok eritme, varlık satışı, küçülme ve yeniden yapılandırma. Prototipte bu sistem henüz yok; negatif nakit mümkündür ve F6 ile kampanya sıfırlanabilir.

## Rekabet ve zirve hissi

Rakiplerin farklı güçlü ve zayıf yönleri bulunur: mahalle sadakati, ucuz alım gücü, taze ürün, hızlı hizmet, geniş ürün yelpazesi. Oyuncunun seçtiği stratejiye aynı tepkiyi veren tek tip rakipler kullanılmaz.

Hedef modelde rakiplerin bütçesi, stok kapasitesi, maliyeti ve risk iştahı vardır. İndirim süresiz bedava olamaz; yeni şube ve reklamın bir bedeli olur. Oyuncunun performansına rağmen hileyle aynı büyüklükte kalan rakip sistemi kullanılmaz.

Rekabet ölçümleri:

- Mahalle, ilçe, bölge ve ülke düzeyinde ayrı pazarlar.
- Satış payı, tekrar gelen müşteri, fiyat algısı, bulunurluk ve memnuniyet.
- Karşılaştırılabilir sepet fiyatı; ucuz tek ürüne bakıp yanlış sonuç veren tablo yerine.
- Rakibin gözlemlenebilen kararları; tüm iç verileri ücretsiz gösterilmez.
- Şube açılışları, kampanyalar, tedarik anlaşmaları ve özel markalar.

Başarı görünür şekilde kutlanır: ilk kârlı hafta, aile dükkânının tadilatı, rakibi ilk kez geçmek, ilk müdür, ilk kamyon, bölge haritasında yeni tabelalar. Son hedef yalnızca en yüksek ciro değil; yüksek pazar payını, hizmeti ve sağlıklı finansı birkaç dönem korumaktır.

Prototipin "yerel müşteri payı" yalnızca basitleştirilmiş hizmet göstergesidir; gerçek Lüleburgaz pazar verisi değildir. İlk rakip kampanyası gün takvimine bağlıdır. Bütçeli, strateji seçen rakip yapay zekâsı henüz uygulanmamıştır.

## Personel ve görev devri

Roller: kasiyer, reyon görevlisi, mal kabul, temizlik, uzman tezgâhtar, şube müdürü, satın alma, depo ve bölge yöneticisi. İlk sürümlerde tek işi iyi yapan az sayıda rol tercih edilir.

Her çalışanın ücret, deneyim, hız, dikkat, enerji ve memnuniyet değerleri olur. Vardiya, mola ve eğitim bu değerleri etkiler. Görev öncelikleri oyuncu tarafından belirlenir: kasa kuyruğu kritikse destek, sonra boş temel gıda rafı, sonra depo düzeni.

Müdüre hedef verilir: stok bulunurluğu, azami personel gideri, asgari nakit rezervi, izin verilen fiyat aralığı. Yetki devri oyuncunun kararını yok etmez; günlük mikro işleri azaltır. Kritik sapmalar kısa ve eyleme dönük raporlarla bildirilir.

## Şube, depo ve uluslararası büyüme

Şube konumu; nüfus, gelir, yaya/araç erişimi, kira, rakip mesafesi ve teslimat süresi üzerinden değerlendirilir. Yeni şubenin mevcut şubeden müşteri çalma etkisi hesaplanır.

Aktif ziyaret edilen mağaza ayrıntılı fizik ve müşteri davranışı kullanır. Uzak şubeler aynı ticari kurallarla saatlik/günlük özetlenir. Her uzak müşteriyi fiziksel karakter olarak çalıştırmak yerine stok hareketi ve finans sonuçları simüle edilir. Ziyaret edildiğinde şube durumu tek veri kaynağından oluşturulur; iki kez satış yazılmaz.

Merkezi depo satın alma avantajı sağlar ama yatırım ve teslimat maliyeti getirir. Kamyon kapasitesi, teslimat penceresi, soğuk zincir, rota ve arıza zaman içinde açılır. Gereksiz erken karmaşıklık için ilk markete zorunlu depo yönetimi yüklenmez.

Uluslararası aşamada ülkeye göre ürün tercihleri, paket boyları, yerel tedarik ve para birimi bulunur. Aynı mağazayı başka haritaya kopyalamaktan öte bir karar alanı yaratılmalıdır. İlk hedef ülkeler tasarım ve içerik bütçesi netleşince seçilir.

## Zaman ve dönem atmosferi

Tasarım başlangıcı 2011; ilk prototipte yalnızca gün sayacı bulunur. Üretim sürümünde saat/gün/hafta/mevsim/yıl ve teknoloji açılmaları ayrı yönetilir. Teknolojiler uzun vadede stok yazılımı, sadakat sistemi ve teslimat kanalları gibi yeni kararlar getirir.

2010'lar Türkiye atmosferi; mahalle ilişkileri, esnaf dili, kasa fişi, promosyon broşürü, dükkân içi radyo ve döneme uygun mimariyle kurulur. Türkçe metin ve karakter desteği ürün sürümünde tam olmalıdır; prototip HUD metinleri font sorunu yaşamamak için ASCII biçimindedir.

Gerçek ekonomik olaylar birebir modellenecekse tarih ve veri kaynakları ayrıca doğrulanır. Alternatif tarih kampanyası da mümkün olabilir. Oyun dengesine göre uydurulan ekonomik değerler tarihsel veri gibi sunulmaz.

## Arayüz ve sunum

Ekranın temel soruları: Bugün ne yapmalıyım? Param neden azaldı? Müşterim neden gitti? Sonraki yatırım ne kazandıracak? Her rapor en çok etkileyen birkaç nedeni göstermeli.

Yönetim masası: ürün/stok, tedarik, fiyat, personel, nakit akışı, rakip gözlemi ve büyüme. İleride bilgisayar ekranı üzerinde fareyle kullanılabilen arayüz. İlk prototipte klavye komutları ve HUD bulunur.

Görsel hedef: sıcak, yaşanmış mahalle marketi; zamanla daha düzenli ve kurumsal mağazalar. Fotogerçekçilikten önce okunabilir raflar, akıcı hareket ve anlaşılır kasa geri bildirimi. Ses hedefi: kapı zili, raf/ambalaj, barkod, fiş, soğutucu ve sokak sesi. Bu sesler henüz eklenmemiştir.

Erişilebilirlik: tuş atama, yazı ölçeği, görüş açısı, hareket bulanıklığı seçeneği, renk dışında durum işaretleri, beklet/tıkla seçenekleri ve duraklatılabilir yönetim. İlk prototip bunların tamamını içermez.

## İlk bölüm için başarı ölçütü

Oyuncu dışarıdan açıklama almadan rafı doldurabilmeli, marketi açabilmeli, müşteriyi kasadan geçirebilmeli, fiyat ve tedarik kararlarının sonucunu görebilmeli. Gün sonundaki nakit ve kâr farkını anlayabilmeli. Kaydı yüklediğinde ürün, para ve çalışanlar aynı durumda olmalı.

İlk şube döngüsü keyifli olmadan ülke haritası ve yüzlerce ürün eklenmez. İçerik genişliği, çalışan temel deneyimin üzerine kurulur.
